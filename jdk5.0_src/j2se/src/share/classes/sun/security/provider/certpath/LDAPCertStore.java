/*
 * @(#)LDAPCertStore.java	1.16 04/05/05
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.provider.certpath;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.util.*;
import javax.naming.Context;
import javax.naming.NamingEnumeration;
import javax.naming.NamingException;
import javax.naming.NameNotFoundException;
import javax.naming.directory.Attribute;
import javax.naming.directory.Attributes;
import javax.naming.directory.BasicAttributes;
import javax.naming.directory.DirContext;
import javax.naming.directory.InitialDirContext;

import java.security.*;
import java.security.cert.*;
import java.security.cert.Certificate;
import javax.security.auth.x500.X500Principal;

import sun.misc.HexDumpEncoder;
import sun.security.util.Cache;
import sun.security.util.Debug;
import sun.security.x509.X500Name;
import sun.security.action.GetPropertyAction;

/**
 * A <code>CertStore</code> that retrieves <code>Certificates</code> and
 * <code>CRL</code>s from an LDAP directory, using the PKIX LDAP V2 Schema
 * (RFC 2587):
 * <a href="http://www.ietf.org/rfc/rfc2587.txt">
 * http://www.ietf.org/rfc/rfc2587.txt</a>.
 * <p>
 * Before calling the {@link #engineGetCertificates engineGetCertificates} or
 * {@link #engineGetCRLs engineGetCRLs} methods, the
 * {@link #LDAPCertStore(CertStoreParameters) 
 * LDAPCertStore(CertStoreParameters)} constructor is called to create the
 * <code>CertStore</code> and establish the DNS name and port of the LDAP 
 * server from which <code>Certificate</code>s and <code>CRL</code>s will be 
 * retrieved.
 * <p>
 * <b>Concurrent Access</b>
 * <p>
 * As described in the javadoc for <code>CertStoreSpi</code>, the
 * <code>engineGetCertificates</code> and <code>engineGetCRLs</code> methods
 * must be thread-safe. That is, multiple threads may concurrently
 * invoke these methods on a single <code>LDAPCertStore</code> object
 * (or more than one) with no ill effects. This allows a
 * <code>CertPathBuilder</code> to search for a CRL while simultaneously
 * searching for further certificates, for instance.
 * <p>
 * This is achieved by adding the <code>synchronized</code> keyword to the
 * <code>engineGetCertificates</code> and <code>engineGetCRLs</code> methods.
 * <p>
 * This classes uses caching and requests multiple attributes at once to
 * minimize LDAP round trips. The cache is associated with the CertStore
 * instance. It uses soft references to hold the values to minimize impact
 * on footprint and currently has a maximum size of 750 attributes and a
 * 30 second default lifetime.
 * <p>
 * We always request CA certificates, cross certificate pairs, and ARLs in
 * a single LDAP request when any one of them is needed. The reason is that
 * we typically need all of them anyway and requesting them in one go can
 * reduce the number of requests to a third. Even if we don't need them,
 * these attributes are typically small enough not to cause a noticeable
 * overhead. In addition, when the prefetchCRLs flag is true, we also request
 * the full CRLs. It is currently false initially but set to true once any
 * request for an ARL to the server returns an null value. The reason is
 * that CRLs could be rather large but are rarely used. This implementation
 * should improve performance in most cases.
 *
 * @see java.security.cert.CertStore
 *
 * @version 	1.16, 05/05/04
 * @since	1.4
 * @author	Steve Hanna
 * @author	Andreas Sterbenz
 */
public class LDAPCertStore extends CertStoreSpi {
 
    private static final Debug debug = Debug.getInstance("certpath");
    
    private final static boolean DEBUG = false;

    /**
     * LDAP attribute identifiers.
     */
    private static final String USER_CERT = "userCertificate;binary";
    private static final String CA_CERT = "cACertificate;binary";
    private static final String CROSS_CERT = "crossCertificatePair;binary";
    private static final String CRL = "certificateRevocationList;binary";
    private static final String ARL = "authorityRevocationList;binary";
    private static final String DELTA_CRL = "deltaRevocationList;binary";
    
    // Constants for various empty values
    private final static String[] STRING0 = new String[0];
    
    private final static byte[][] BB0 = new byte[0][];
    
    private final static Attributes EMPTY_ATTRIBUTES = new BasicAttributes();
    
    // cache related constants
    private final static int DEFAULT_CACHE_SIZE = 750;
    private final static int DEFAULT_CACHE_LIFETIME = 30;
    
    private final static int LIFETIME;
    
    private final static String PROP_LIFETIME = 
			    "sun.security.certpath.ldap.cache.lifetime";
    
    static {
	PrivilegedAction action = new GetPropertyAction(PROP_LIFETIME);
	String s = (String)AccessController.doPrivileged(action);
	if (s != null) {
	    LIFETIME = Integer.parseInt(s); // throws NumberFormatException
	} else {
	    LIFETIME = DEFAULT_CACHE_LIFETIME;
	}
    }

    /**
     * The CertificateFactory used to decode certificates from
     * their binary stored form.
     */
    private CertificateFactory cf;
    /**
     * The JNDI directory context.
     */
    private DirContext ctx;
    
    /**
     * Flag indicating whether we should prefetch CRLs.
     */
    private boolean prefetchCRLs = false;

    private final Cache valueCache;
    
    private int cacheHits = 0;
    private int cacheMisses = 0;
    private int requests = 0;

    /**
     * Creates a <code>CertStore</code> with the specified parameters.
     * For this class, the parameters object must be an instance of
     * <code>LDAPCertStoreParameters</code>.
     *
     * @param params the algorithm parameters
     * @exception InvalidAlgorithmParameterException if params is not an
     *   instance of <code>LDAPCertStoreParameters</code>
     */
    public LDAPCertStore(CertStoreParameters params)
	    throws InvalidAlgorithmParameterException {
	super(params);
	if (!(params instanceof LDAPCertStoreParameters))
	  throw new InvalidAlgorithmParameterException(
	    "parameters must be LDAPCertStoreParameters");
    
	LDAPCertStoreParameters lparams = (LDAPCertStoreParameters) params;
    
	// Create InitialDirContext needed to communicate with the server
	createInitialDirContext(lparams.getServerName(), lparams.getPort());
    
	// Create CertificateFactory for use later on
	try {
	    cf = CertificateFactory.getInstance("X.509");
	} catch (CertificateException e) {
	    throw new InvalidAlgorithmParameterException(
		"unable to create CertificateFactory for X.509");
	}
	if (LIFETIME == 0) {
	    valueCache = Cache.newNullCache();
	} else if (LIFETIME < 0) {
	    valueCache = Cache.newSoftMemoryCache(DEFAULT_CACHE_SIZE);
	} else {
	    valueCache = Cache.newSoftMemoryCache(DEFAULT_CACHE_SIZE, LIFETIME);
	}
    }

    /**
     * Create InitialDirContext.
     *
     * @param server Server DNS name hosting LDAP service
     * @param port   Port at which server listens for requests
     * @throws InvalidAlgorithmParameterException if creation fails
     */
    private void createInitialDirContext(String server, int port) 
	    throws InvalidAlgorithmParameterException {
	String url = "ldap://" + server + ":" + port;
	Hashtable<String,Object> env = new Hashtable<String,Object>();
	env.put(Context.INITIAL_CONTEXT_FACTORY,
		"com.sun.jndi.ldap.LdapCtxFactory");
	env.put(Context.PROVIDER_URL, url);
	try {
	    ctx = new InitialDirContext(env);
	    /*
	     * By default, follow referrals unless application has
	     * overridden property in an application resource file.
	     */
	    Hashtable currentEnv = ctx.getEnvironment();
	    if (currentEnv.get(Context.REFERRAL) == null) {
		ctx.addToEnvironment(Context.REFERRAL, "follow");
	    }
	} catch (NamingException e) {
	    if (debug != null) {
		debug.println("LDAPCertStore.engineInit about to throw "
		    + "InvalidAlgorithmParameterException");
		e.printStackTrace();
	    }
	    Exception ee = new InvalidAlgorithmParameterException
	        ("unable to create InitialDirContext using supplied parameters");
	    ee.initCause(e);
	    throw (InvalidAlgorithmParameterException)ee;
	}
    }
    
    /**
     * Private class encapsulating the actual LDAP operations and cache
     * handling. Use:
     *
     *   LDAPRequest request = new LDAPRequest(dn);
     *   request.addRequestedAttribute(CROSS_CERT);
     *   request.addRequestedAttribute(CA_CERT);
     *   byte[][] crossValues = request.getValues(CROSS_CERT);
     *   byte[][] caValues = request.getValues(CA_CERT);
     *
     * At most one LDAP request is sent for each instance created. If all
     * getValues() calls can be satisfied from the cache, no request
     * is sent at all. If a request is sent, all requested attributes
     * are always added to the cache irrespective of whether the getValues()
     * method is called.
     */
    private class LDAPRequest {
	
	private final String name;
	private Map valueMap;
	private final List requestedAttributes;
	
	LDAPRequest(String name) {
	    this.name = name;
	    requestedAttributes = new ArrayList(5);
	}
	
	String getName() {
	    return name;
	}
	
	void addRequestedAttribute(String attrId) {
	    if (valueMap != null) {
		throw new IllegalStateException("Request already sent");
	    }
	    requestedAttributes.add(attrId);
	}
	
	/**
	 * Gets one or more binary values from an attribute.
	 *
	 * @param name		the location holding the attribute
	 * @param attrId		the attribute identifier
	 * @return			an array of binary values (byte arrays)
	 * @throws NamingException	if a naming exception occurs
	 */
	byte[][] getValues(String attrId) throws NamingException {
	    if (DEBUG && ((cacheHits + cacheMisses) % 50 == 0)) {
		System.out.println("Cache hits: " + cacheHits + "; misses: "
			+ cacheMisses);
	    }
	    String cacheKey = name + "|" + attrId;
	    byte[][] values = (byte[][])valueCache.get(cacheKey);
	    if (values != null) {
		cacheHits++;
		return values;
	    }
	    cacheMisses++;
	    Map attrs = getValueMap();
	    values = (byte[][])attrs.get(attrId);
	    return values;
	}
	
	/**
	 * Get a map containing the values for this request. The first time
	 * this method is called on an object, the LDAP request is sent,
	 * the results parsed and added to a private map and also to the
	 * cache of this LDAPCertStore. Subsequent calls return the private
	 * map immediately.
	 *
	 * The map contains an entry for each requested attribute. The
	 * attribute name is the key, values are byte[][]. If there are no
	 * values for that attribute, values are byte[0][].
	 *
	 * @return			the value Map
	 * @throws NamingException	if a naming exception occurs
	 */
	private Map getValueMap() throws NamingException {
	    if (valueMap != null) {
		return valueMap;
	    }
	    if (DEBUG) {
		System.out.println("Request: " + name + ":" + requestedAttributes);
		requests++;
		if (requests % 5 == 0) {
		    System.out.println("LDAP requests: " + requests);
		}
	    }
	    valueMap = new HashMap(8);
	    String[] attrIds = (String[])requestedAttributes.toArray(STRING0);
	    Attributes attrs;
	    try {
		attrs = ctx.getAttributes(name, attrIds);
	    } catch (NameNotFoundException e) {
		// name does not exist on this LDAP server
		// treat same as not attributes found
		attrs = EMPTY_ATTRIBUTES;
	    }
	    for (Iterator t = requestedAttributes.iterator(); t.hasNext(); ) {
		String attrId = (String)t.next();
		Attribute attr = attrs.get(attrId);
		byte[][] values = getAttributeValues(attr);
		cacheAttribute(attrId, values);
		valueMap.put(attrId, values);
	    }
	    return valueMap;
	}
	
	/**
	 * Add the values to the cache.
	 */
	private void cacheAttribute(String attrId, byte[][] values) {
	    String cacheKey = name + "|" + attrId;
	    valueCache.put(cacheKey, values);
	}
	
	/**
	 * Get the values for the given attribute. If the attribute is null
	 * or does not contain any values, a zero length byte array is
	 * returned. NOTE that it is assumed that all values are byte arrays.
	 */
	private byte[][] getAttributeValues(Attribute attr) 
		throws NamingException {
	    byte[][] values;
	    if (attr == null) {
		values = BB0;
	    } else {
		values = new byte[attr.size()][];
		int i = 0;
		NamingEnumeration enum_ = attr.getAll();
		while (enum_.hasMore()) {
		    Object obj = enum_.next();
		    if (debug != null) {
			if (obj instanceof String) {
			    debug.println("LDAPCertStore.getAttrValues() "
			        + "enum.next is a string!: " + obj);
			}
		    }
		    byte[] value = (byte[])obj;
		    values[i++] = value;
		}
	    }
	    return values;
	}
	
    }

    /*
     * Gets certificates from an attribute id and location in the LDAP 
     * directory. Returns a Collection containing only the Certificates that
     * match the specified CertSelector.
     *
     * @param name the location holding the attribute
     * @param id the attribute identifier
     * @param sel a CertSelector that the Certificates must match
     * @return a Collection of Certificates found
     * @throws CertStoreException	if an exception occurs
     */
    private Collection getCertificates(LDAPRequest request, String id, 
	    CertSelector sel) throws CertStoreException {
    
	/* fetch encoded certs from storage */
	byte[][] encodedCert;
	try {
	    encodedCert = request.getValues(id);
	} catch (NamingException namingEx) {
	    throw new CertStoreException(namingEx);
	}
	
	int n = encodedCert.length;
	if (n == 0) {
	    return Collections.EMPTY_LIST;
	}
    
	List certs = new ArrayList(n);
	/* decode certs and check if they satisfy selector */
	for (int i = 0; i < n; i++) {
	    ByteArrayInputStream bais = new ByteArrayInputStream(encodedCert[i]);
	    try {
		Certificate cert = cf.generateCertificate(bais);
		if (sel.match(cert)) {
		  certs.add(cert);
		}
	    } catch (CertificateException e) {
		if (debug != null) {
		    debug.println("LDAPCertStore.getCertificates() encountered "
		        + "exception while parsing cert, skipping the bad data: ");
		    HexDumpEncoder encoder = new HexDumpEncoder();
		    debug.println(
		    	"[ " + encoder.encodeBuffer(encodedCert[i]) + " ]");
		}
	    }
	}
    
	return certs;
    }

    /*
     * Gets certificate pairs from an attribute id and location in the LDAP
     * directory.
     *
     * @param name the location holding the attribute
     * @param id the attribute identifier
     * @return a Collection of X509CertificatePairs found
     * @throws CertStoreException	if an exception occurs
     */
    private Collection getCertPairs(LDAPRequest request, String id)
	    throws CertStoreException {
    
	/* fetch the encoded cert pairs from storage */
	byte[][] encodedCertPair;
	try {
	    encodedCertPair = request.getValues(id);
	} catch (NamingException namingEx) {
	    throw new CertStoreException(namingEx);
	}
	
	int n = encodedCertPair.length;
 	if (n == 0) {
	    return Collections.EMPTY_LIST;
	}
    
	List certPairs = new ArrayList(n);
	/* decode each cert pair and add it to the Collection */
	for (int i = 0; i < n; i++) {
	    try {
		X509CertificatePair certPair = 
		    X509CertificatePair.generateCertificatePair(encodedCertPair[i]);
		certPairs.add(certPair);
	    } catch (CertificateException e) {
		if (debug != null) {
		    debug.println(
		    	"LDAPCertStore.getCertPairs() encountered exception "
			+ "while parsing cert, skipping the bad data: ");
		    HexDumpEncoder encoder = new HexDumpEncoder();
		    debug.println(
		    	"[ " + encoder.encodeBuffer(encodedCertPair[i]) + " ]");
		}
	    }
	}
    
	return certPairs;
    }

    /*
     * Looks at certificate pairs stored in the crossCertificatePair attribute
     * at the specified location in the LDAP directory. Returns a Collection
     * containing all Certificates stored in the forward component that match
     * the forward CertSelector and all Certificates stored in the reverse
     * component that match the reverse CertSelector.
     * <p>
     * If either forward or reverse is null, all certificates from the
     * corresponding component will be rejected.
     *
     * @param name the location to look in
     * @param forward the forward CertSelector (or null)
     * @param reverse the reverse CertSelector (or null)
     * @return a Collection of Certificates found
     * @throws CertStoreException	if an exception occurs
     */
    private Collection getMatchingCrossCerts(LDAPRequest request, 
	    CertSelector forward, CertSelector reverse) 
	    throws CertStoreException {
	// Get the cert pairs
	Collection certPairs = getCertPairs(request, CROSS_CERT);
    
	// Find Certificates that match and put them in a list
	ArrayList matchingCerts = new ArrayList();
	Iterator i = certPairs.iterator();
	while (i.hasNext()) {
	    X509CertificatePair certPair = (X509CertificatePair) i.next();
	    X509Certificate cert;
	    if (forward != null) {
		cert = certPair.getForward();
		if ((cert != null) && forward.match(cert)) {
		    matchingCerts.add(cert);
		}
	    }
	    if (reverse != null) {
		cert = certPair.getReverse();
		if ((cert != null) && reverse.match(cert)) {
		    matchingCerts.add(cert);
		}
	    }
	}
	return matchingCerts;
    }

    /**
     * Returns a <code>Collection</code> of <code>Certificate</code>s that
     * match the specified selector. If no <code>Certificate</code>s
     * match the selector, an empty <code>Collection</code> will be returned.
     * <p>
     * It is not practical to search every entry in the LDAP database for
     * matching <code>Certificate</code>s. Instead, the <code>CertSelector</code>
     * is examined in order to determine where matching <code>Certificate</code>s
     * are likely to be found (according to the PKIX LDAPv2 schema, RFC 2587).
     * If the subject is specified, its directory entry is searched. If the
     * issuer is specified, its directory entry is searched. If neither the
     * subject nor the issuer are specified (or the selector is not an
     * <code>X509CertSelector</code>), a <code>CertStoreException</code> is
     * thrown.
     *
     * @param selector a <code>CertSelector</code> used to select which
     *  <code>Certificate</code>s should be returned.
     * @return a <code>Collection</code> of <code>Certificate</code>s that
     *         match the specified selector
     * @throws CertStoreException if an exception occurs 
     */
    public synchronized Collection<X509Certificate> engineGetCertificates
	    (CertSelector selector) throws CertStoreException {
	if (debug != null) {
	    debug.println("LDAPCertStore.engineGetCertificates() selector: " 
		+ String.valueOf(selector));
	}
    
	if (selector == null) {
	    selector = new X509CertSelector();
	}
	if (!(selector instanceof X509CertSelector)) {
	    throw new CertStoreException("LDAPCertStore needs an X509CertSelector " +
					 "to find certs");
	}
	X509CertSelector xsel = (X509CertSelector) selector;
	int basicConstraints = xsel.getBasicConstraints();
	String subject = xsel.getSubjectAsString();
	String issuer = xsel.getIssuerAsString();
	HashSet certs = new HashSet();
	if (debug != null) {
	    debug.println("LDAPCertStore.engineGetCertificates() basicConstraints: "
		+ basicConstraints);
	}
    
	// basicConstraints:
	// -2: only EE certs accepted
	// -1: no check is done
	//  0: any CA certificate accepted
	// >1: certificate's basicConstraints extension pathlen must match
	if (subject != null) {
	    if (debug != null) {
		debug.println("LDAPCertStore.engineGetCertificates() "
		    + "subject is not null");
	    }
	    LDAPRequest request = new LDAPRequest(subject);
	    if (basicConstraints > -2) {
		request.addRequestedAttribute(CROSS_CERT);
		request.addRequestedAttribute(CA_CERT);
		request.addRequestedAttribute(ARL);
		if (prefetchCRLs) {
		    request.addRequestedAttribute(CRL);
		}
	    }
	    if (basicConstraints < 0) {
		request.addRequestedAttribute(USER_CERT);
	    }
	    
	    if (basicConstraints > -2) {
		certs.addAll(getMatchingCrossCerts(request, xsel, null));
		if (debug != null) {
		    debug.println("LDAPCertStore.engineGetCertificates() after "
			+ "getMatchingCrossCerts(subject,xsel,null),certs.size(): " 
			+ certs.size());
		}
		certs.addAll(getCertificates(request, CA_CERT, xsel));
		if (debug != null) {
		    debug.println("LDAPCertStore.engineGetCertificates() after "
			+ "getCertificates(subject,CA_CERT,xsel),certs.size(): " 
			+ certs.size());
		}
	    }
	    if (basicConstraints < 0) {
		certs.addAll(getCertificates(request, USER_CERT, xsel));
		if (debug != null) {
		    debug.println("LDAPCertStore.engineGetCertificates() after "
			+ "getCertificates(subject,USER_CERT, xsel),certs.size(): " 
			+ certs.size());
		}
	    }
	} else {
	    if (debug != null) {
		debug.println
		    ("LDAPCertStore.engineGetCertificates() subject is null");
	    }
	    if (basicConstraints == -2) {
		throw new CertStoreException("need subject to find EE certs");
	    }
	    if (issuer == null) {
		throw new CertStoreException("need subject or issuer to find certs");
	    }
	}
	if (debug != null) {
	    debug.println("LDAPCertStore.engineGetCertificates() about to "
		+ "getMatchingCrossCerts...");
	}
	if ((issuer != null) && (basicConstraints > -2)) {
	    LDAPRequest request = new LDAPRequest(issuer);
	    request.addRequestedAttribute(CROSS_CERT);
	    request.addRequestedAttribute(CA_CERT);
	    request.addRequestedAttribute(ARL);
	    if (prefetchCRLs) {
		request.addRequestedAttribute(CRL);
	    }
	    
	    certs.addAll(getMatchingCrossCerts(request, null, xsel));
	    if (debug != null) {
		debug.println("LDAPCertStore.engineGetCertificates() after "
		    + "getMatchingCrossCerts(issuer,null,xsel),certs.size(): " 
		    + certs.size());
	    }
	    certs.addAll(getCertificates(request, CA_CERT, xsel));
	    if (debug != null) {
		debug.println("LDAPCertStore.engineGetCertificates() after "
		    + "getCertificates(issuer,CA_CERT,xsel),certs.size(): " 
		    + certs.size());
	    }
	}
	if (debug != null) {
	    debug.println("LDAPCertStore.engineGetCertificates() returning certs");
	}
	return certs;
    }
   
    /*
     * Gets CRLs from an attribute id and location in the LDAP directory.
     * Returns a Collection containing only the CRLs that match the
     * specified CRLSelector.
     *
     * @param name the location holding the attribute
     * @param id the attribute identifier
     * @param sel a CRLSelector that the CRLs must match
     * @return a Collection of CRLs found
     * @throws CertStoreException	if an exception occurs
     */
    private Collection getCRLs(LDAPRequest request, String id, 
	    CRLSelector sel) throws CertStoreException {
    
	/* fetch the encoded crls from storage */
	byte[][] encodedCRL;
	try {
	    encodedCRL = request.getValues(id);
	} catch (NamingException namingEx) {
	    throw new CertStoreException(namingEx);
	}
	
	int n = encodedCRL.length;
	if (n == 0) {
	    return Collections.EMPTY_LIST;
	}
    
	List crls = new ArrayList(n);
	/* decode each crl and check if it matches selector */
	for (int i = 0; i < n; i++) {
	    try {
		CRL crl = cf.generateCRL(new ByteArrayInputStream(encodedCRL[i]));
		if (sel.match(crl)) {
		    crls.add(crl);
		}
	    } catch (CRLException e) {
		if (debug != null) {
		    debug.println("LDAPCertStore.getCRLs() encountered exception"
		        + " while parsing CRL, skipping the bad data: ");
		    HexDumpEncoder encoder = new HexDumpEncoder();
		    debug.println("[ " + encoder.encodeBuffer(encodedCRL[i]) + " ]");
		}
	    }
	}
    
	return crls;
    }

    /**
     * Returns a <code>Collection</code> of <code>CRL</code>s that
     * match the specified selector. If no <code>CRL</code>s
     * match the selector, an empty <code>Collection</code> will be returned.
     * <p>
     * It is not practical to search every entry in the LDAP database for
     * matching <code>CRL</code>s. Instead, the <code>CRLSelector</code>
     * is examined in order to determine where matching <code>CRL</code>s
     * are likely to be found (according to the PKIX LDAPv2 schema, RFC 2587).
     * If issuerNames or certChecking are specified, the issuer's directory
     * entry is searched. If neither issuerNames or certChecking are specified
     * (or the selector is not an <code>X509CRLSelector</code>), a
     * <code>CertStoreException</code> is thrown.
     *
     * @param selector A <code>CRLSelector</code> used to select which
     *  <code>CRL</code>s should be returned. Specify <code>null</code>
     *  to return all <code>CRL</code>s.
     * @return A <code>Collection</code> of <code>CRL</code>s that
     *         match the specified selector
     * @throws CertStoreException if an exception occurs 
     */
    public synchronized Collection<X509CRL> engineGetCRLs(CRLSelector selector)
	    throws CertStoreException {
	if (debug != null) {
	    debug.println("LDAPCertStore.engineGetCRLs() selector: " 
		+ selector);
	}
	// Set up selector and collection to hold CRLs
	if (selector == null) {
	    selector = new X509CRLSelector();
	}
	if (!(selector instanceof X509CRLSelector)) {
	    throw new CertStoreException("need X509CRLSelector to find CRLs");
	}
	X509CRLSelector xsel = (X509CRLSelector) selector;
	HashSet crls = new HashSet();
    
	// Look in directory entry for issuer of cert we're checking.
	Collection issuerNames;
	X509Certificate certChecking = xsel.getCertificateChecking();
	if (certChecking != null) {
	    issuerNames = new HashSet();
	    X500Principal issuer = certChecking.getIssuerX500Principal();
	    issuerNames.add(issuer.getName(X500Principal.RFC2253));
	} else {
	    // But if we don't know which cert we're checking, try the directory
	    // entries of all acceptable CRL issuers
	    issuerNames = xsel.getIssuerNames();
	    if (issuerNames == null) {
		throw new CertStoreException("need issuerNames or certChecking to "
		    + "find CRLs");
	    }
	}
	Iterator i = issuerNames.iterator();
	while (i.hasNext()) {
	    Object nameObject = i.next();
	    String issuerName;
	    if (nameObject instanceof byte[]) {
		try {
		    X500Principal issuer = new X500Principal((byte[])nameObject);
		    issuerName = issuer.getName(X500Principal.RFC2253);
		} catch (IllegalArgumentException e) {
		    continue;
		}
	    } else {
		issuerName = (String)nameObject;
	    }
	    // If all we want is CA certs, try to get the (probably shorter) ARL
	    Collection entryCRLs = Collections.EMPTY_LIST;
	    if (certChecking == null || certChecking.getBasicConstraints() != -1) {
		LDAPRequest request = new LDAPRequest(issuerName);
		request.addRequestedAttribute(CROSS_CERT);
		request.addRequestedAttribute(CA_CERT);
		request.addRequestedAttribute(ARL);
		if (prefetchCRLs) {
		    request.addRequestedAttribute(CRL);
		}
		try {
		    entryCRLs = getCRLs(request, ARL, xsel);
		    if (entryCRLs.isEmpty()) {
			// no ARLs found. We assume that means that there are
			// no ARLs on this server at all and prefetch the CRLs.
			prefetchCRLs = true;
		    }
		} catch (CertStoreException e) { 
		    if (debug != null) {
			debug.println("LDAPCertStore.engineGetCRLs non-fatal error "
			    + "retrieving ARLs:" + e);
			e.printStackTrace();
		    }
		}
	    }
	    // Otherwise, get the CRL
	    // if certChecking is null, we don't know if we should look in ARL or CRL
	    // attribute, so check both for matching CRLs.
	    if (entryCRLs.isEmpty() || certChecking == null) {
		LDAPRequest request = new LDAPRequest(issuerName);
		request.addRequestedAttribute(CRL);
		entryCRLs = getCRLs(request, CRL, xsel);
	    }
	    crls.addAll(entryCRLs);
	}
	return crls;
    }
}
