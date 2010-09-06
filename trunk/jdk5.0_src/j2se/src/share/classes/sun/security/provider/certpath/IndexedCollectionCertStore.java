/*
 * @(#)IndexedCollectionCertStore.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.provider.certpath;

import java.util.*;

import java.security.InvalidAlgorithmParameterException;
import java.security.cert.*;

import javax.security.auth.x500.X500Principal;

/**
 * A <code>CertStore</code> that retrieves <code>Certificates</code> and
 * <code>CRL</code>s from a <code>Collection</code>.
 * <p>
 * This implementation is functionally equivalent to CollectionCertStore
 * with two differences:
 * <ol>
 * <li>Upon construction, the elements in the specified Collection are
 * partially indexed. X509Certificates are indexed by subject, X509CRLs
 * by issuer, non-X509 Certificates and CRLs are copied without indexing,
 * other objects are ignored. This increases CertStore construction time
 * but allows significant speedups for searches which specify the indexed
 * attributes, in particular for large Collections (reduction from linear
 * time to effectively constant time). Searches for non-indexed queries
 * are as fast (or marginally faster) than for the standard
 * CollectionCertStore. Certificate subjects and CRL issuers
 * were found to be specified in most searches used internally by the
 * CertPath provider. Additional attributes could indexed if there are
 * queries that justify the effort.
 *
 * <li>Changes to the specified Collection after construction time are
 * not detected and ignored. This is because there is no way to efficiently
 * detect if a Collection has been modified, a full traversal would be
 * required. That would degrade lookup performance to linear time and
 * eliminated the benefit of indexing. We may fix this via the introduction
 * of new public APIs in the future.
 * </ol>
 * <p>
 * Before calling the {@link #engineGetCertificates engineGetCertificates} or
 * {@link #engineGetCRLs engineGetCRLs} methods, the
 * {@link #CollectionCertStore(CertStoreParameters) 
 * CollectionCertStore(CertStoreParameters)} constructor is called to 
 * create the <code>CertStore</code> and establish the
 * <code>Collection</code> from which <code>Certificate</code>s and
 * <code>CRL</code>s will be retrieved. If the specified
 * <code>Collection</code> contains an object that is not a
 * <code>Certificate</code> or <code>CRL</code>, that object will be
 * ignored.
 * <p>
 * <b>Concurrent Access</b>
 * <p>
 * As described in the javadoc for <code>CertStoreSpi</code>, the
 * <code>engineGetCertificates</code> and <code>engineGetCRLs</code> methods
 * must be thread-safe. That is, multiple threads may concurrently
 * invoke these methods on a single <code>CollectionCertStore</code>
 * object (or more than one) with no ill effects.
 * <p>
 * This is achieved by requiring that the <code>Collection</code> passed to
 * the {@link #CollectionCertStore(CertStoreParameters) 
 * CollectionCertStore(CertStoreParameters)} constructor (via the
 * <code>CollectionCertStoreParameters</code> object) must have fail-fast
 * iterators. Simultaneous modifications to the <code>Collection can thus be
 * detected and certificate or CRL retrieval can be retried. The fact that
 * <code>Certificate</code>s and <code>CRL</code>s must be thread-safe is also
 * essential.
 *
 * @see java.security.cert.CertStore
 * @see CollectionCertStore
 *
 * @version 1.4, 12/19/03
 * @author Andreas Sterbenz
 */
public class IndexedCollectionCertStore extends CertStoreSpi {
    
    /**
     * Map X500Principal(subject) -> X509Certificate | List of X509Certificate
     */
    private Map certSubjects;
    /**
     * Map X500Principal(issuer) -> X509CRL | List of X509CRL
     */
    private Map crlIssuers;
    /**
     * Sets of non-X509 certificates and CRLs
     */
    private Set otherCertificates, otherCRLs;
    
    /**
     * Creates a <code>CertStore</code> with the specified parameters.
     * For this class, the parameters object must be an instance of
     * <code>CollectionCertStoreParameters</code>.
     *
     * @param params the algorithm parameters
     * @exception InvalidAlgorithmParameterException if params is not an
     *   instance of <code>CollectionCertStoreParameters</code>
     */
    public IndexedCollectionCertStore(CertStoreParameters params) 
	    throws InvalidAlgorithmParameterException {
	super(params);
        if (!(params instanceof CollectionCertStoreParameters)) {
            throw new InvalidAlgorithmParameterException(
	        "parameters must be CollectionCertStoreParameters");
	}
        Collection coll = ((CollectionCertStoreParameters)params).getCollection();
	if (coll == null) {
	    throw new InvalidAlgorithmParameterException
    					("Collection must not be null");
	}
	buildIndex(coll);
    }
    
    /**
     * Index the specified Collection copying all references to Certificates
     * and CRLs.
     */
    private void buildIndex(Collection coll) {
	certSubjects = new HashMap();
	crlIssuers = new HashMap();
	otherCertificates = null;
	otherCRLs = null;
	for (Iterator t = coll.iterator(); t.hasNext(); ) {
	    Object obj = t.next();
	    if (obj instanceof X509Certificate) {
		indexCertificate((X509Certificate)obj);
	    } else if (obj instanceof X509CRL) {
		indexCRL((X509CRL)obj);
	    } else if (obj instanceof Certificate) {
		if (otherCertificates == null) {
		    otherCertificates = new HashSet();
		}
		otherCertificates.add(obj);
	    } else if (obj instanceof CRL) {
		if (otherCRLs == null) {
		    otherCRLs = new HashSet();
		}
		otherCRLs.add(obj);
	    } else {
		// ignore
	    }
	}
	if (otherCertificates == null) {
	    otherCertificates = Collections.EMPTY_SET;
	}
	if (otherCRLs == null) {
	    otherCRLs = Collections.EMPTY_SET;
	}
    }
    
    /**
     * Add an X509Certificate to the index.
     */
    private void indexCertificate(X509Certificate cert) {
	X500Principal subject = cert.getSubjectX500Principal();
	Object oldEntry = certSubjects.put(subject, cert);
	if (oldEntry != null) { // assume this is unlikely
	    if (oldEntry instanceof X509Certificate) {
		if (cert.equals(oldEntry)) {
		    return;
		}
		List list = new ArrayList(2);
		list.add(cert);
		list.add(oldEntry);
		certSubjects.put(subject, list);
	    } else {
		List list = (List)oldEntry;
		if (list.contains(cert) == false) {
		    list.add(cert);
		}
		certSubjects.put(subject, list);
	    }
	}
    }
    
    /** 
     * Add an X509CRL to the index.
     */
    private void indexCRL(X509CRL crl) {
	X500Principal issuer = crl.getIssuerX500Principal();
	Object oldEntry = crlIssuers.put(issuer, crl);
	if (oldEntry != null) { // assume this is unlikely
	    if (oldEntry instanceof X509CRL) {
		if (crl.equals(oldEntry)) {
		    return;
		}
		List list = new ArrayList(2);
		list.add(crl);
		list.add(oldEntry);
		crlIssuers.put(issuer, list);
	    } else {
		List list = (List)oldEntry;
		if (list.contains(crl) == false) {
		    list.add(crl);
		}
		crlIssuers.put(issuer, list);
	    }
	}
    }

    /**
     * Returns a <code>Collection</code> of <code>Certificate</code>s that
     * match the specified selector. If no <code>Certificate</code>s
     * match the selector, an empty <code>Collection</code> will be returned.
     *
     * @param selector a <code>CertSelector</code> used to select which
     *  <code>Certificate</code>s should be returned. Specify <code>null</code>
     *  to return all <code>Certificate</code>s.
     * @return a <code>Collection</code> of <code>Certificate</code>s that
     *         match the specified selector
     * @throws CertStoreException if an exception occurs 
     */
    public Collection<? extends Certificate> engineGetCertificates(CertSelector selector)
	    throws CertStoreException {
	
	// no selector means match all
	if (selector == null) {
	    Set matches = new HashSet();
	    matchX509Certs(new X509CertSelector(), matches);
	    matches.addAll(otherCertificates);
	    return matches;
	}
		
	if (selector instanceof X509CertSelector == false) {
	    Set matches = new HashSet();
	    matchX509Certs(selector, matches);
	    for (Iterator t = otherCertificates.iterator(); t.hasNext(); ) {
		Certificate cert = (Certificate)t.next();
		if (selector.match(cert)) {
		    matches.add(cert);
		}
	    }
	    return matches;
	}
		
	if (certSubjects.isEmpty()) {
	    return Collections.EMPTY_SET;
	}
	X509CertSelector x509Selector = (X509CertSelector)selector;
	// see if the subject is specified
	X500Principal subject;
	X509Certificate matchCert = x509Selector.getCertificate();
	if (matchCert != null) {
	    subject = matchCert.getSubjectX500Principal();
	} else {
	    subject = CertPathHelper.getSubject(x509Selector);
	}
	if (subject != null) {
	    // yes, narrow down candidates to indexed possibilities
	    Object entry = certSubjects.get(subject);
	    if (entry == null) {
		return Collections.EMPTY_SET;
	    }
	    if (entry instanceof X509Certificate) {
		X509Certificate x509Entry = (X509Certificate)entry;
		if (x509Selector.match(x509Entry)) {
		    return Collections.singleton(x509Entry);
		} else {
		    return Collections.EMPTY_SET;
		}
	    } else {
		List list = (List)entry;
		Set matches = new HashSet(16);
		for (Iterator t = list.iterator(); t.hasNext(); ) {
		    X509Certificate cert = (X509Certificate)t.next();
		    if (x509Selector.match(cert)) {
			matches.add(cert);
		    }
		}
		return matches;
	    }
	}
	// cannot use index, iterate all
	Set matches = new HashSet(16);
	matchX509Certs(x509Selector, matches);
	return matches;
    }
    
    /**
     * Iterate through all the X509Certificates and add matches to the
     * collection.
     */
    private void matchX509Certs(CertSelector selector, Collection matches) {
	for (Iterator t = certSubjects.values().iterator(); t.hasNext(); ) {
	    Object obj = t.next();
	    if (obj instanceof X509Certificate) {
		X509Certificate cert = (X509Certificate)obj;
		if (selector.match(cert)) {
		    matches.add(cert);
		}
	    } else {
		List list = (List)obj;
		for (Iterator u = list.iterator(); u.hasNext(); ) {
		    X509Certificate cert = (X509Certificate)u.next();
		    if (selector.match(cert)) {
			matches.add(cert);
		    }
		}
	    }
	}
    }
   
    /**
     * Returns a <code>Collection</code> of <code>CRL</code>s that
     * match the specified selector. If no <code>CRL</code>s
     * match the selector, an empty <code>Collection</code> will be returned.
     *
     * @param selector a <code>CRLSelector</code> used to select which
     *  <code>CRL</code>s should be returned. Specify <code>null</code>
     *  to return all <code>CRL</code>s.
     * @return a <code>Collection</code> of <code>CRL</code>s that
     *         match the specified selector
     * @throws CertStoreException if an exception occurs 
     */
    public Collection<CRL> engineGetCRLs(CRLSelector selector)
	    throws CertStoreException {
		
	if (selector == null) {
	    Set matches = new HashSet();
	    matchX509CRLs(new X509CRLSelector(), matches);
	    matches.addAll(otherCRLs);
	    return matches;
	}
		
	if (selector instanceof X509CRLSelector == false) {
	    Set matches = new HashSet();
	    matchX509CRLs(selector, matches);
	    for (Iterator t = otherCRLs.iterator(); t.hasNext(); ) {
		CRL crl = (CRL)t.next();
		if (selector.match(crl)) {
		    matches.add(crl);
		}
	    }
	    return matches;
	}
		
	if (crlIssuers.isEmpty()) {
	    return Collections.EMPTY_SET;
	}
	X509CRLSelector x509Selector = (X509CRLSelector)selector;
	// see if the issuer is specified
	Collection issuers = CertPathHelper.getIssuers(x509Selector);
	if (issuers != null) {
	    HashSet matches = new HashSet(16);
	    for (Iterator t = issuers.iterator(); t.hasNext(); ) {
		X500Principal issuer = (X500Principal)t.next();
		Object entry = crlIssuers.get(issuer);
		if (entry == null) {
		    // empty
		} else if (entry instanceof X509CRL) {
		    X509CRL crl = (X509CRL)entry;
		    if (x509Selector.match(crl)) {
			matches.add(crl);
		    }
		} else { // List
		    List list = (List)entry;
		    for (Iterator u = list.iterator(); u.hasNext(); ) {
			X509CRL crl = (X509CRL)u.next();
			if (x509Selector.match(crl)) {
			    matches.add(crl);
			}
		    }
		}
	    }
	    return matches;
	}
	// cannot use index, iterate all
	Set matches = new HashSet(16);
	matchX509CRLs(x509Selector, matches);
	return matches;
    }
    
    /**
     * Iterate through all the X509CRLs and add matches to the
     * collection.
     */
    private void matchX509CRLs(CRLSelector selector, Collection matches) {
	for (Iterator t = crlIssuers.values().iterator(); t.hasNext(); ) {
	    Object obj = t.next();
	    if (obj instanceof X509CRL) {
		X509CRL crl = (X509CRL)obj;
		if (selector.match(crl)) {
		    matches.add(crl);
		}
	    } else {
		List list = (List)obj;
		for (Iterator u = list.iterator(); u.hasNext(); ) {
		    X509CRL crl = (X509CRL)u.next();
		    if (selector.match(crl)) {
			matches.add(crl);
		    }
		}
	    }
	}
    }
    
}
