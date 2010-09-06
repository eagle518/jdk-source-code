/*
 * @(#)PKIXValidator.java	1.7, 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.validator;

import java.util.*;

import java.security.*;
import java.security.cert.*;

import javax.security.auth.x500.X500Principal;

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
 * @version 1.7, 12/19/03
 */
public final class PKIXValidator extends Validator {

    // enable use of the validator if possible
    private final static boolean TRY_VALIDATOR = true;

    private final Set trustedCerts;
    private final PKIXBuilderParameters parameterTemplate;

    // needed only for the validator
    private Set trustedSubjects;
    private CertificateFactory factory;

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
        trustedSubjects = new HashSet();
        for (Iterator t = trustedCerts.iterator(); t.hasNext(); ) {
            X509Certificate cert = (X509Certificate)t.next();
            trustedSubjects.add(cert.getSubjectX500Principal());
        }
        try {
            factory = CertificateFactory.getInstance("X.509");
        } catch (CertificateException e) {
            throw new RuntimeException("Internal error", e);
        }
    }

    public Collection getTrustedCertificates() {
        return trustedCerts;
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
	    // check if chain contains trust anchor
	    for (int i = 0; i < chain.length; i++) {
		if (trustedCerts.contains(chain[i])) {
		    if (i == 0) {
			return new X509Certificate[] {chain[0]};
		    }
		    // Remove and call validator
		    X509Certificate[] newChain = new X509Certificate[i];
		    System.arraycopy(chain, 0, newChain, 0, i);
		    return doValidate(newChain);
		}
	    }

            // not self issued and apparently issued by trust anchor?
	    X509Certificate last = chain[chain.length - 1];
            X500Principal issuer = last.getIssuerX500Principal();
            X500Principal subject = last.getSubjectX500Principal();
            if (trustedSubjects.contains(issuer) && !issuer.equals(subject)) {
                return doValidate(chain);
            }

            // otherwise, fall back to builder
        }

        return doBuild(chain, otherCerts);
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
        try {
            PKIXBuilderParameters params = 
	    	(PKIXBuilderParameters)parameterTemplate.clone();
	    setDate(params);

            // do the validation
            CertPathValidator validator = CertPathValidator.getInstance("PKIX");
            CertPath path = factory.generateCertPath(Arrays.asList(chain));
            PKIXCertPathValidatorResult result = 
	    	(PKIXCertPathValidatorResult)validator.validate(path, params);

            return toArray(path, result.getTrustAnchor());
        } catch (GeneralSecurityException e) {
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
