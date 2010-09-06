/*
 * @(#)PKIXCertPathValidator.java	1.15 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.provider.certpath;

import java.io.IOException;
import java.security.AccessController;
import java.security.InvalidAlgorithmParameterException;
import java.security.PrivilegedAction;
import java.security.PublicKey;
import java.security.Security;
import java.security.cert.CertPath;
import java.security.cert.CertPathParameters;
import java.security.cert.CertPathValidatorException;
import java.security.cert.CertPathValidatorSpi;
import java.security.cert.CertPathValidatorResult;
import java.security.cert.PKIXCertPathChecker;
import java.security.cert.PKIXCertPathValidatorResult;
import java.security.cert.PKIXParameters;
import java.security.cert.PolicyNode;
import java.security.cert.TrustAnchor;
import java.security.cert.X509Certificate;
import java.security.cert.X509CertSelector;
import java.util.Collections;
import java.util.List;
import java.util.ArrayList;
import java.util.Date;
import java.util.Set;
import java.util.HashSet;
import javax.security.auth.x500.X500Principal;
import sun.security.util.Debug;

/** 
 * This class implements the PKIX validation algorithm for certification
 * paths consisting exclusively of <code>X509Certificates</code>. It uses
 * the specified input parameter set (which must be a
 * <code>PKIXParameters</code> object) and signature provider (if any).
 *
 * @version 	1.15 12/19/03
 * @since	1.4
 * @author      Yassir Elley
 */
public class PKIXCertPathValidator extends CertPathValidatorSpi {
 
    private static final Debug debug = Debug.getInstance("certpath");
    private Date testDate;
    private List<PKIXCertPathChecker> userCheckers;
    private String sigProvider;
    private BasicChecker basicChecker;

    /**
     * Default constructor.
     */ 
    public PKIXCertPathValidator() {}

    /**
     * Validates a certification path consisting exclusively of 
     * <code>X509Certificate</code>s using the PKIX validation algorithm,
     * which uses the specified input parameter set. 
     * The input parameter set must be a <code>PKIXParameters</code> object.
     *
     * @param cp the X509 certification path
     * @param param the input PKIX parameter set
     * @return the result
     * @exception CertPathValidatorException Exception thrown if cert path
     * does not validate.
     * @exception InvalidAlgorithmParameterException if the specified 
     * parameters are inappropriate for this certification path validator
     */
    public CertPathValidatorResult engineValidate(CertPath cp, 
        CertPathParameters param) 
	throws CertPathValidatorException, InvalidAlgorithmParameterException
    {
	if (debug != null)
	    debug.println("PKIXCertPathValidator.engineValidate()...");

	if (!(param instanceof PKIXParameters)) {
	    throw new InvalidAlgorithmParameterException("inappropriate " 
		+ "parameters, must be an instance of PKIXParameters");
	}

	if (!cp.getType().equals("X.509") && !cp.getType().equals("X509")) {
	    throw new InvalidAlgorithmParameterException("inappropriate " 
		+ "certification path type specified, must be X.509 or X509");
	}

	PKIXParameters pkixParam = (PKIXParameters) param;

        // Make sure that none of the trust anchors include name constraints 
	// (not supported).
	Set<TrustAnchor> anchors = pkixParam.getTrustAnchors();
	for (TrustAnchor anchor : anchors) {
            if (anchor.getNameConstraints() != null) {
		throw new InvalidAlgorithmParameterException
		    ("name constraints in trust anchor not supported");
            }
        }

	// the certpath which has been passed in (cp) 
	// has the target cert as the first certificate - we
	// need to keep this cp so we can return it
	// in case of an exception and for policy qualifier 
	// processing - however, for certpath validation,
	// we need to create a reversed path, where we reverse the 
	// ordering so that the target cert is the last certificate 

	// Must copy elements of certList into a new modifiable List before
	// calling Collections.reverse().
	List<X509Certificate> certList = new ArrayList<X509Certificate>
	    ((List<X509Certificate>)cp.getCertificates());
	if (debug != null) {
	    if (certList.isEmpty()) {
    	        debug.println("PKIXCertPathValidator.engineValidate() "
	            + "certList is empty");
	    }
	    debug.println("PKIXCertPathValidator.engineValidate() " 
		+ "reversing certpath...");
	}
	Collections.reverse(certList);

	// now certList has the target cert as the last cert and we
	// can proceed with normal validation

	populateVariables(pkixParam);

        // Retrieve the first certificate in the certpath
        // (to be used later in pre-screening)
        X509Certificate firstCert = null;
        if (!certList.isEmpty()) {
	    firstCert = certList.get(0);
	}

	CertPathValidatorException lastException = null;

        // We iterate through the set of trust anchors until we find
        // one that works at which time we stop iterating
	for (TrustAnchor anchor : anchors) {
	    X509Certificate trustedCert = anchor.getTrustedCert();
	    PublicKey trustedPubKey;
	    X500Principal caName;
	    if (trustedCert != null) {
	        if (debug != null) {
	    	    debug.println("PKIXCertPathValidator.engineValidate() "
		        + "anchor.getTrustedCert() != null");
		}
		// if this trust anchor is not worth trying,
		// we move on to the next one
		if (!isWorthTrying(trustedCert, firstCert)) {
		    continue;
		}

                // note that we extract the public key and subjectDN
                // from the trusted cert, but we do not validate the 
	        // trusted cert itself
		trustedPubKey = trustedCert.getPublicKey();
		caName = trustedCert.getSubjectX500Principal();
		if (debug != null) {
		    debug.println("anchor.getTrustedCert()."
		    	+ "getSubjectX500Principal() = " + caName);
		}
	    } else {
	        if (debug != null) {
	    	    debug.println("PKIXCertPathValidator.engineValidate(): " 
		        + "anchor.getTrustedCert() == null");
		}
		trustedPubKey = anchor.getCAPublicKey();
	        caName = CertPathHelper.getCA(anchor);
	    }

	    try {
                PolicyNodeImpl rootNode = new PolicyNodeImpl(null, 
	            PolicyChecker.ANY_POLICY, null, false, 
	            Collections.singleton(PolicyChecker.ANY_POLICY), false);
	        PolicyNode policyTree = doValidate(trustedPubKey, caName, cp,
		    certList, pkixParam, rootNode); 
		// if this anchor works, return success
		return new PKIXCertPathValidatorResult(anchor, policyTree, 
		    basicChecker.getPublicKey());
	    } catch (CertPathValidatorException cpe) {
		// remember this exception
		lastException = cpe;
	    }
	}
	
	// could not find a trust anchor that verified
	// (a) if we did a validation and it failed, use that exception
	if (lastException != null) {
	    throw lastException;
	}
	// (b) otherwise, generate new exception
	throw new CertPathValidatorException
			("Path does not chain with any of the trust anchors");
    }

    /**
     * Internal method to do some simple checks to see if a given cert is
     * worth trying to validate in the chain.
     */
    private boolean isWorthTrying(X509Certificate trustedCert, 
			          X509Certificate firstCert) 
        throws CertPathValidatorException 
    {
	if (debug != null) {
	    debug.println("PKIXCertPathValidator.isWorthTrying() checking "
		+ "if this trusted cert is worth trying ...");
	}

	if (firstCert == null) {
	    return true;
	}

	// the subject of the trusted cert should match the
	// issuer of the first cert in the certpath
	
	X500Principal trustedSubject = trustedCert.getSubjectX500Principal();
	if (trustedSubject.equals(firstCert.getIssuerX500Principal())) {
	    if (debug != null)
	    	debug.println("YES - try this trustedCert");
	    return true;
	} else {
	    if (debug != null)
	    	debug.println("NO - don't try this trustedCert");
	    return false;
	}
    }

    /**
     * Internal method to setup the internal state
     */    
    private void populateVariables(PKIXParameters pkixParam) 
        throws CertPathValidatorException
    {
	// default value for testDate is current time
	testDate = pkixParam.getDate();
	if (testDate == null) {
	    testDate = new Date(System.currentTimeMillis());
	}
	
	userCheckers = pkixParam.getCertPathCheckers();
	sigProvider = pkixParam.getSigProvider();
    }

    /**
     * Internal method to actually validate a constructed path.
     *
     * @return the valid policy tree
     */
    private PolicyNode doValidate(
	    PublicKey trustedPubKey, X500Principal caName, 
	    CertPath cpOriginal, List<X509Certificate> certList, 
	    PKIXParameters pkixParam, PolicyNodeImpl rootNode) 
	    throws CertPathValidatorException
    {
	List<PKIXCertPathChecker> certPathCheckers = 
	    new ArrayList<PKIXCertPathChecker>();
	
	int certPathLen = certList.size();
	
        basicChecker = 
            new BasicChecker(trustedPubKey, caName,
                             testDate, sigProvider, false);
	KeyChecker keyChecker = new KeyChecker(certPathLen, 
	    pkixParam.getTargetCertConstraints());
	ConstraintsChecker constraintsChecker = 
	    new ConstraintsChecker(certPathLen);

	PolicyChecker policyChecker = 
	    new PolicyChecker(pkixParam.getInitialPolicies(), certPathLen, 
			      pkixParam.isExplicitPolicyRequired(),
			      pkixParam.isPolicyMappingInhibited(),
                              pkixParam.isAnyPolicyInhibited(),
			      pkixParam.getPolicyQualifiersRejected(),
			      rootNode);

	// add standard checkers that we will be using	
	certPathCheckers.add(keyChecker);
	certPathCheckers.add(constraintsChecker);
	certPathCheckers.add(policyChecker);
        certPathCheckers.add(basicChecker);

	// only add a revocationChecker if revocation is enabled
	if (pkixParam.isRevocationEnabled()) {

	    // Examine OCSP security property 
	    String ocspProperty = (String) AccessController.doPrivileged(
		new PrivilegedAction() {
		    public Object run() {
			return 
			    Security.getProperty(OCSPChecker.OCSP_ENABLE_PROP);
		    }
		});

	    // Use OCSP if it has been enabled
	    if ("true".equalsIgnoreCase(ocspProperty)) {
		OCSPChecker ocspChecker =
		    new OCSPChecker(cpOriginal, pkixParam);
		certPathCheckers.add(ocspChecker);
	    } 

	    // Always use CRLs
	    CrlRevocationChecker revocationChecker =
	        new CrlRevocationChecker(trustedPubKey, pkixParam, certList);
	    certPathCheckers.add(revocationChecker);
	}
		
	// add user-specified checkers	
	certPathCheckers.addAll(userCheckers);
	
	PKIXMasterCertPathValidator masterValidator = 
	    new PKIXMasterCertPathValidator(certPathCheckers);
	
	masterValidator.validate(cpOriginal, certList);

        return policyChecker.getPolicyTree();
    }
}
