/*
 * @(#)DistributionPointFetcher.java	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.provider.certpath;

import java.io.*;
import java.net.*;
import java.util.*;

import java.security.*;
import java.security.cert.*;

import java.security.AccessController;
import sun.security.action.GetPropertyAction;

import sun.security.util.*;
import sun.security.x509.*;

/**
 * Class to obtain CRLs via the CRLDistributionPoints extension.
 * Note that the functionality of this class must be explicitly enabled
 * via a system property, see the USE_CRLDP variable below.
 *
 * Currently, this class has a number of limitations. CRLs are only 
 * returned if:
 *
 *  . reasonFlags in the DistributionPoint are not set
 *
 *  . crlIssuer in the DistributionPoint is not set or identical
 *    to certIssuer
 *
 *  . the DistributionPoint is identified using the fullName option
 *    containing one or more URLs
 *
 *  . CRLs are identified using non-LDAP URL
 *
 * This class also implements CRL caching. Currently, the cache is shared
 * between all applications in the VM and uses a hardcoded policy.
 * The cache has a maximum size of 185 entries, which are held by 
 * SoftReferences. A request will be satisfied from the cache if we last
 * checked for an update within CHECK_INTERVAL (last 30 seconds). Otherwise,
 * we open an URLConnection to download the CRL using an If-Modified-Since 
 * request (HTTP) if possible. Note that both positive and negative responses
 * are cached, i.e. if we are unable to open the connection or the CRL cannot
 * be parsed, we remember this result and additional calls during the
 * CHECK_INTERVAL period do not try to open another connection.
 *
 * @author Andreas Sterbenz
 * @version 1.3, 12/19/03
 * @since 1.4.2
 */
class DistributionPointFetcher {
    
    private static final Debug debug = Debug.getInstance("certpath");

    /**
     * Flag indicating whether support for the CRL distribution point
     * extension shall be enabled. Currently disabled by default for
     * compatibility and legal reasons.
     */
    private final static boolean USE_CRLDP = 
    	getBooleanProperty("com.sun.security.enableCRLDP", false);

    /**
     * Return the value of the boolean System property propName.
     */
    public static boolean getBooleanProperty(String propName,
	    boolean defaultValue) {
	// if set, require value of either true or false
	String b = (String)AccessController.doPrivileged(
		new GetPropertyAction(propName));
	if (b == null) {
	    return defaultValue;
	} else if (b.equalsIgnoreCase("false")) {
	    return false;
	} else if (b.equalsIgnoreCase("true")) {
	    return true;
	} else {
	    throw new RuntimeException("Value of " + propName
	    + " must either be 'true' or 'false'");
	}
    }
	
    // singleton instance
    private static final DistributionPointFetcher INSTANCE = 
    	new DistributionPointFetcher();
    
    // interval between checks for update of a cached CRL (30 seconds)
    private final static int CHECK_INTERVAL = 30 * 1000;
    
    // size of the cache (see Cache class for sizing recommendations)
    private final static int CACHE_SIZE = 185;
    
    // X.509 certificate factory instance
    private final CertificateFactory factory;
    
    /**
     * CRL cache mapping URI -> CacheEntry.
     */
    private final Cache cache;
    
    /** 
     * Private instantiation only.
     */
    private DistributionPointFetcher() {
	try {
	    factory = CertificateFactory.getInstance("X.509");
	} catch (CertificateException e) {
	    throw new RuntimeException();
	}
	cache = Cache.newSoftMemoryCache(CACHE_SIZE);
    }
    
    /**
     * Return a DistributionPointFetcher instance.
     */
    static DistributionPointFetcher getInstance() {
	return INSTANCE;
    }
    
    /**
     * Return the X509CRLs matching this selector. The selector must be
     * an X509CRLSelector with certificateChecking set.
     *
     * If CRLDP support is disabled, this method always returns an
     * empty set.
     */
    Collection getCRLs(CRLSelector selector) throws CertStoreException {
	if (USE_CRLDP == false) {
	    return Collections.EMPTY_SET;
	}
	if (selector instanceof X509CRLSelector == false) {
	    return Collections.EMPTY_SET;
	}
	X509CRLSelector x509Selector = (X509CRLSelector)selector;
	X509Certificate cert = x509Selector.getCertificateChecking();
	if (cert == null) {
	    return Collections.EMPTY_SET;
	}
	try {
	    X509CertImpl certImpl = X509CertImpl.toImpl(cert);
	    if (debug != null) {
		debug.println("Checking CRLDPs for "
			+ certImpl.getSubjectX500Principal());
	    }
	    CRLDistributionPointsExtension ext = 
	    	certImpl.getCRLDistributionPointsExtension();
	    if (ext == null) {
		return Collections.EMPTY_SET;
	    }
	    X500Name certIssuer = (X500Name)certImpl.getIssuerDN();
	    List points = (List)ext.get(CRLDistributionPointsExtension.POINTS);
	    Set results = new HashSet();
	    for (Iterator t = points.iterator(); t.hasNext(); ) {
		DistributionPoint point = (DistributionPoint)t.next();
		Collection crls = getCRLs(x509Selector, certIssuer, point);
		results.addAll(crls);
	    }
	    if (debug != null) {
		debug.println("Returning " + results.size() + " CRLs");
	    }
	    return results;
	} catch (CertificateException e) {
	    return Collections.EMPTY_SET;
	} catch (IOException e) {
	    return Collections.EMPTY_SET;
	}
    }
    
    /**
     * Download CRLs from the given distribution point and return them.
     * See the top of the class for current limitations.
     */
    private Collection getCRLs(X509CRLSelector selector, X500Name certIssuer,
	    DistributionPoint point) {
	// ignore if reason flags are set
	if (point.getReasonFlags() != null) {
	    return Collections.EMPTY_LIST;
	}
	// if crlIssuer is set, accept only if identical to certificate issuer
	GeneralNames crlIssuer = point.getCRLIssuer();
	if (crlIssuer != null) {
	    for (Iterator t = crlIssuer.iterator(); t.hasNext(); ) {
		GeneralNameInterface name = ((GeneralName)t.next()).getName();
		if (certIssuer.equals(name) == false) {
		    return Collections.EMPTY_LIST;
		}
	    }
	}
	// must have full name
	GeneralNames fullName = point.getFullName();
	if (fullName == null) {
	    return Collections.EMPTY_LIST;
	}
	Collection crls = new ArrayList(2);
	for (Iterator t = fullName.iterator(); t.hasNext(); ) {
	    GeneralName name = (GeneralName)t.next();
	    if (name.getType() != GeneralNameInterface.NAME_URI) {
		continue;
	    }
	    URIName uriName = (URIName)name.getName();
	    String uriString = uriName.getName();

	    if (debug != null) {
		debug.println("Trying to fetch CRL from DP " + uriString);
	    }
	    try {
		URI uri = new URI(uriString);
		// do not try to fetch LDAP urls
		if (uri.getScheme().toLowerCase().equals("ldap")) {
		    continue;
		}
		CacheEntry entry = (CacheEntry)cache.get(uri);
		if (entry == null) {
		    entry = new CacheEntry();
		    cache.put(uri, entry);
		}
		X509CRL crl = entry.getCRL(factory, uri);
		if ((crl != null) && selector.match(crl)) {
		    crls.add(crl);
		}
	    } catch (URISyntaxException e) {
		if (debug != null) {
		    debug.println("Exception parsing URI:");
		    e.printStackTrace();
		}
	    }
	}
	return crls;
    }
    
    /**
     * Inner class used for cache entries.
     */
    private static class CacheEntry {
	// CRL (may be null)
	private X509CRL crl;
	
	// time we last checked for an update
	private long lastChecked;
	
	// time server returned as last modified time stamp
	// or 0 if not available
	private long lastModified;
	
	CacheEntry() {
	    // empty
	}
	
	/**
	 * Return the CRL for this entry. It returns the cached value
	 * if it is still current and fetches the CRL otherwise.
	 * For the caching details, see the top of this class.
	 */
	synchronized X509CRL getCRL(CertificateFactory factory, URI uri) {
	    long time = System.currentTimeMillis();
	    if (time - lastChecked < CHECK_INTERVAL) {
		if (debug != null) {
		    debug.println("Returning CRL from cache");
		}
		return crl;
	    }
	    lastChecked = time;
	    InputStream in = null;
	    try {
		URL url = uri.toURL();
		URLConnection connection = url.openConnection();
		if (lastModified != 0) {
		    connection.setIfModifiedSince(lastModified);
		}
		in = connection.getInputStream();
		long oldLastModified = lastModified;
		lastModified = connection.getLastModified();
		if (oldLastModified != 0) {
		    if (oldLastModified == lastModified) {
			if (debug != null) {
			    debug.println("Not modified, using cached copy");
			}
			return crl;
		    } else if (connection instanceof HttpURLConnection) {
			// some proxy servers omit last modified
			HttpURLConnection hconn = (HttpURLConnection)connection;
			if (hconn.getResponseCode()
				    == HttpURLConnection.HTTP_NOT_MODIFIED) {
			    if (debug != null) {
				debug.println("Not modified, using cached copy");
			    }
			    return crl;
			}
		    }
		}
		if (debug != null) {
		    debug.println("Downloading new CRL...");
		}
		crl = (X509CRL)factory.generateCRL(in);
		return crl;
	    } catch (IOException e) {
		if (debug != null) {
		    debug.println("Exception fetching CRLDP:");
		    e.printStackTrace();
		}
	    } catch (CRLException e) {
		if (debug != null) {
		    debug.println("Exception fetching CRLDP:");
		    e.printStackTrace();
		}
	    } finally {
		if (in != null) {
		    try {
			in.close();
		    } catch (IOException e) {
			// ignore
		    }
		}
	    }
	    // exception, forget previous values
	    lastModified = 0;
	    crl = null;
	    return null;
	}
    }
    
}
