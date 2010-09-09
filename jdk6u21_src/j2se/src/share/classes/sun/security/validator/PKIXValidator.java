/*
 * @(#)PKIXValidator.java	1.17, 10/06/10
 *
 * Copyright (c) 2010, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.validator;

import java.util.*;

import java.security.*;
import java.security.cert.*;
import javax.security.auth.x500.X500Principal;
import sun.security.provider.certpath.OCSPResponse;

/**
 * Validator implementation built on the PKIX CertPath API. This
 * implementation will be emphasized going forward.<p>
 *
 * Note that the validate() implementation tries to use a PKIX validator
 * if that appears possible and a PKIX builder otherwise. This increases
 * performance and currently also leads to better exception messages
 * in case of failures.
 *
 * @author Andreas Sterbenz
 * @version 1.17, 06/10/10
 */
public final class PKIXValidator extends Validator {

    // enable use of the validator if possible
    private final static boolean TRY_VALIDATOR = true;

    private final Set trustedCerts;
    private final PKIXBuilderParameters parameterTemplate;
    private int certPathLength = -1;

    // needed only for the validator
    private Map<X500Principal, List<PublicKey>> trustedSubjects;
    private CertificateFactory factory;

    private boolean plugin = false;

    PKIXValidator(String variant, Collection trustedCerts) {
        super(TYPE_PKIX, variant);
	if (trustedCerts instanceof Set) {
	    this.trustedCerts = (Set)trustedCerts;
	} else {
	    this.trustedCerts = new HashSet(trustedCerts);
	}
	Set<TrustAnchor> trustAnchors = new HashSet<TrustAnchor>();
	for (Iterator t = trustedCerts.iterator(); t.hasNext(); ) {
	    X509Certificate cert = (X509Certificate)t.next();
	    trustAnchors.add(new TrustAnchor(cert, null));
	}
        try {
            parameterTemplate = new PKIXBuilderParameters(trustAnchors, null);
        } catch (InvalidAlgorithmParameterException e) {
            throw new RuntimeException("Unexpected error: " + e.toString(), e);
        }
	setDefaultParameters(variant);
        initCommon();
    }

    PKIXValidator(String variant, PKIXBuilderParameters params) {
        super(TYPE_PKIX, variant);
        trustedCerts = new HashSet();
        for (Iterator t = params.getTrustAnchors().iterator(); t.hasNext(); ) {
            TrustAnchor anchor = (TrustAnchor)t.next();
            X509Certificate cert = anchor.getTrustedCert();
            if (cert != null) {
                trustedCerts.add(cert);
            }
        }
        parameterTemplate = params;
        initCommon();
    }

    private void initCommon() {
        if (TRY_VALIDATOR == false) {
            return;
        }
        trustedSubjects = new HashMap<X500Principal, List<PublicKey>>();
        for (Iterator t = trustedCerts.iterator(); t.hasNext(); ) {
            X509Certificate cert = (X509Certificate)t.next();
	    X500Principal dn = cert.getSubjectX500Principal();
	    List<PublicKey> keys;
	    if (trustedSubjects.containsKey(dn)) {
		keys = trustedSubjects.get(dn);
	    } else {
		keys = new ArrayList<PublicKey>();
		trustedSubjects.put(dn, keys);
	    }
	    keys.add(cert.getPublicKey());
        }
        try {
            factory = CertificateFactory.getInstance("X.509");
        } catch (CertificateException e) {
            throw new RuntimeException("Internal error", e);
        }
	plugin = variant.equals(VAR_PLUGIN_CODE_SIGNING);
    }

    public Collection getTrustedCertificates() {
        return trustedCerts;
    }

    /**
     * Returns the length of the last certification path that is validated by
     * CertPathValidator. This is intended primarily as a callback mechanism
     * for PKIXCertPathCheckers to determine the length of the certification
     * path that is being validated. It is necessary since engineValidate()
     * may modify the length of the path.
     *
     * @return the length of the last certification path passed to 
     *   CertPathValidator.validate, or -1 if it has not been invoked yet 
     */ 
    public int getCertPathLength() {
	return certPathLength;
    }

    /**
     * Set J2SE global default PKIX parameters. Currently, hardcoded to disable
     * revocation checking. In the future, this should be configurable.
     */
    private void setDefaultParameters(String variant) {
        parameterTemplate.setRevocationEnabled(false);
    }

    /**
     * Return the PKIX parameters used by this instance. An application may
     * modify the parameters but must make sure not to perform any concurrent
     * validations.
     */
    public PKIXBuilderParameters getParameters() {
        return parameterTemplate;
    }

    X509Certificate[] engineValidate(X509Certificate[] chain, 
	    Collection otherCerts, Object parameter) 
	    throws CertificateException {
        if ((chain == null) || (chain.length == 0)) {
            throw new CertificateException
	    	("null or zero-length certificate chain");
        }
        if (TRY_VALIDATOR) {
	    // check that chain is in correct order and check if chain contains 
            // trust anchor
            X500Principal prevIssuer = null;
	    for (int i = 0; i < chain.length; i++) {
                X509Certificate cert = chain[i];
                X500Principal dn = cert.getSubjectX500Principal();
                if (i != 0 && 
                    !dn.equals(prevIssuer)) {
                    // chain is not ordered correctly, call builder instead
                    return doBuild(chain, otherCerts);
                }

                // Check if chain[i] is already trusted. It may be inside
                // trustedCerts, or has the same dn and public key as a cert
                // inside trustedCerts. The latter happens when a CA has
                // updated its cert with a stronger signature algorithm in JRE
                // but the weak one is still in circulation.

                if (trustedCerts.contains(cert) ||          // trusted cert
			(trustedSubjects.containsKey(dn) && // replacing ...
                         trustedSubjects.get(dn).contains(  // ... weak cert
				cert.getPublicKey()))) {
		    if (i == 0) {
			return new X509Certificate[] {chain[0]};
		    }
		    // Remove and call validator on partial chain [0 .. i-1]
		    X509Certificate[] newChain = new X509Certificate[i];
		    System.arraycopy(chain, 0, newChain, 0, i);
		    return doValidate(newChain);
		}
                prevIssuer = cert.getIssuerX500Principal();
	    }

            // apparently issued by trust anchor?
	    X509Certificate last = chain[chain.length - 1];
            X500Principal issuer = last.getIssuerX500Principal();
            X500Principal subject = last.getSubjectX500Principal();
            if (trustedSubjects.containsKey(issuer) &&
                    isSignatureValid(trustedSubjects.get(issuer), last)) {
                return doValidate(chain);
            }

	    // don't fallback to builder if called from plugin/webstart
	    if (plugin) {
		// Validate chain even if no trust anchor is found. This
		// allows plugin/webstart to make sure the chain is 
		// otherwise valid 
		if (chain.length > 1) {
		    X509Certificate[] newChain = 
			new X509Certificate[chain.length-1];
		    System.arraycopy(chain, 0, newChain, 0, newChain.length);
		    // temporarily set last cert as sole trust anchor
        	    PKIXBuilderParameters params = 
    	    		(PKIXBuilderParameters) parameterTemplate.clone();
		    try {
		        params.setTrustAnchors
			    (Collections.singleton(new TrustAnchor
			        (chain[chain.length-1], null)));
		    } catch (InvalidAlgorithmParameterException iape) {
			// should never occur, but ...
		 	throw new CertificateException(iape);
		    }
		    doValidate(newChain, params);
		}
		// if the rest of the chain is valid, throw exception 
		// indicating no trust anchor was found
	        throw new ValidatorException
	            (ValidatorException.T_NO_TRUST_ANCHOR);
	    }
            // otherwise, fall back to builder
        }

        return doBuild(chain, otherCerts);
    }

    private boolean isSignatureValid(List<PublicKey> keys, X509Certificate sub) {
	if (plugin) {
	    for (PublicKey key: keys) {
	    	try {
		    sub.verify(key);
		    return true;
	    	} catch (Exception ex) {
		    continue;
		}
	    }
	    return false;
	}
	return true; // only check if PLUGIN is set
    }

    private static X509Certificate[] toArray(CertPath path, TrustAnchor anchor)
	    throws CertificateException {
        List list = path.getCertificates();
        X509Certificate[] chain = new X509Certificate[list.size() + 1];
        list.toArray(chain);
        X509Certificate trustedCert = anchor.getTrustedCert();
        if (trustedCert == null) {
            throw new ValidatorException
	    	("TrustAnchor must be specified as certificate");
        }
        chain[chain.length - 1] = trustedCert;
        return chain;
    }
    
    /**
     * Set the check date (for debugging).
     */
    private void setDate(PKIXBuilderParameters params) {
	Date date = validationDate;
	if (date != null) {
	    params.setDate(date);
	}
    }

    private X509Certificate[] doValidate(X509Certificate[] chain)
	    throws CertificateException {
        PKIXBuilderParameters params = 
    	    (PKIXBuilderParameters)parameterTemplate.clone();
	return doValidate(chain, params);
    }

    private X509Certificate[] doValidate(X509Certificate[] chain, 
	    PKIXBuilderParameters params) throws CertificateException {
	try {
	    setDate(params);

            // do the validation
            CertPathValidator validator = CertPathValidator.getInstance("PKIX");
            CertPath path = factory.generateCertPath(Arrays.asList(chain));
	    certPathLength = chain.length;
            PKIXCertPathValidatorResult result = 
	    	(PKIXCertPathValidatorResult)validator.validate(path, params);

            return toArray(path, result.getTrustAnchor());
        } catch (GeneralSecurityException e) {
            if (e instanceof CertPathValidatorException) {
                // check cause
                Throwable cause = e.getCause();
                if (cause != null &&
                    cause instanceof OCSPResponse.UnreliableException) {
                    throw new ValidatorException
                        (ValidatorException.T_OCSP_RESPONSE_UNRELIABLE);
                }
            }
            throw new ValidatorException
	    	("PKIX path validation failed: " + e.toString(), e);
        }
    }

    private X509Certificate[] doBuild(X509Certificate[] chain, 
	    Collection otherCerts) throws CertificateException {
        try {
            PKIXBuilderParameters params = 
	    	(PKIXBuilderParameters)parameterTemplate.clone();
	    setDate(params);

            // setup target constraints
            X509CertSelector selector = new X509CertSelector();
            selector.setCertificate(chain[0]);
            params.setTargetCertConstraints(selector);

            // setup CertStores
            Collection certs = new ArrayList();
            certs.addAll(Arrays.asList(chain));
            if (otherCerts != null) {
                certs.addAll(otherCerts);
            }
            CertStore store = CertStore.getInstance("Collection", 
	    			new CollectionCertStoreParameters(certs));
            params.addCertStore(store);

            // do the build
            CertPathBuilder builder = CertPathBuilder.getInstance("PKIX");
            PKIXCertPathBuilderResult result = 
	    	(PKIXCertPathBuilderResult)builder.build(params);

            return toArray(result.getCertPath(), result.getTrustAnchor());
        } catch (GeneralSecurityException e) {
            throw new ValidatorException
	    	("PKIX path building failed: " + e.toString(), e);
        }
    }
}
