/*
 * @(#)PKIXMasterCertPathValidator.java	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.provider.certpath;

import sun.security.util.Debug;

import java.security.cert.X509Certificate;
import java.util.Collections;
import java.util.List;
import java.util.Set;
import java.util.Iterator;
import java.security.cert.CertPath;
import java.security.cert.PKIXCertPathChecker;
import java.security.cert.CertPathValidatorException;

/** 
 * This class is initialized with a list of <code>PKIXCertPathChecker</code>s
 * and is used to verify the certificates in a <code>CertPath</code> by
 * feeding each certificate to each <code>PKIXCertPathChecker</code>.
 *
 * @version 	1.10 10/29/03
 * @since	1.4
 * @author      Yassir Elley
 */
class PKIXMasterCertPathValidator {
 
    private static final Debug debug = Debug.getInstance("certpath");
    private List certPathCheckers;

    /** 
     * Initializes the list of PKIXCertPathCheckers whose checks
     * will be performed on each certificate in the certpath.
     *
     * @param certPathCheckers a List of checkers to use
     */ 
    PKIXMasterCertPathValidator(List certPathCheckers) {
	this.certPathCheckers = certPathCheckers;
    }

    /**
     * Validates a certification path consisting exclusively of 
     * <code>X509Certificate</code>s using the
     * <code>PKIXCertPathChecker</code>s specified
     * in the constructor. It is assumed that the
     * <code>PKIXCertPathChecker</code>s
     * have been initialized with any input parameters they may need.
     *
     * @param cpOriginal the original X509 CertPath passed in by the user
     * @param reversedCertList the reversed X509 CertPath (as a List)
     * @exception CertPathValidatorException Exception thrown if cert
     * path does not validate.
     */
    void validate(CertPath cpOriginal, List reversedCertList)
	throws CertPathValidatorException
    {
	X509Certificate currCert = null;
	PKIXCertPathChecker currChecker = null;
	
	// we actually process reversedCertList, but we keep cpOriginal because
	// we need to return the original certPath when we throw an exception.
	// we will also need to modify the index appropriately when we
	// throw an exception.

	int cpSize = reversedCertList.size();

        if (debug != null) {
       	    debug.println("--------------------------------------------------"
	          + "------------");
 	    debug.println("Executing PKIX certification path validation "
	          + "algorithm.");
	}
	
	for (int i = 0; i < cpSize; i++) {
	    
	    /* The basic loop algorithm is that we get the
	     * current certificate, we verify the current certificate using
	     * information from the previous certificate and from the state, 
	     * and we modify the state for the next loop by setting the 
	     * current certificate of this loop to be the previous certificate
	     * of the next loop. The state is initialized during first loop.
	     */
            if (debug != null)
                debug.println("Checking cert" + (i+1) + " ...");
	    
	    currCert = (X509Certificate) reversedCertList.get(i);
	    Set unresolvedCritExts = currCert.getCriticalExtensionOIDs();
	    if (unresolvedCritExts == null) {
	        unresolvedCritExts = Collections.EMPTY_SET;
	    }

	    if (debug != null &&  !unresolvedCritExts.isEmpty()) {
	        debug.println("Set of critical extensions:");
		for (Iterator iter = unresolvedCritExts.iterator(); 
	                iter.hasNext();) {
	    	    debug.println((String) iter.next());
		}
	    }
	    
	    CertPathValidatorException ocspCause = null;
	    for (int j = 0; j < certPathCheckers.size(); j++) {
		
		currChecker = (PKIXCertPathChecker) certPathCheckers.get(j);
		if (debug != null) {
		    debug.println("-Using checker" + (j + 1) + " ... [" +
			currChecker.getClass().getName() + "]");
		}

		if (i == 0)
		    currChecker.init(false);
		
		try {
		    currChecker.check(currCert, unresolvedCritExts);

		    // OCSP has validated the cert so skip the CRL check
		    if (isRevocationCheck(currChecker, j, certPathCheckers)) {
			if (debug != null) {
			    debug.println("-checker" + (j + 1) +
				" validation succeeded");
			}
			j++;
			continue; // skip
		    }

		} catch (CertPathValidatorException cpve) {
		    // Throw the saved OCSP exception 
		    // (when the CRL check has also failed)
		    if (ocspCause != null && 
			currChecker instanceof CrlRevocationChecker) {
			throw ocspCause;
		    }
		    /*
		     * Handle failover from OCSP to CRLs
		     */
		    CertPathValidatorException currentCause = 
			new CertPathValidatorException(cpve.getMessage(),
			    cpve.getCause(), cpOriginal, cpSize - (i + 1));
		
		    // Check if OCSP has confirmed that the cert was revoked
		    if (cpve instanceof CertificateRevokedException) {
			throw currentCause;
		    } 
		    // Check if it is appropriate to failover
		    if (! isRevocationCheck(currChecker, j, certPathCheckers)) {
			// no failover
			throw currentCause;
		    }
		    // Save the current exception 
		    // (in case the CRL check also fails)
		    ocspCause = currentCause;

		    // Otherwise, failover to CRLs
		    if (debug != null) {
			debug.println(cpve.getMessage());
			debug.println(
			    "preparing to failover (from OCSP to CRLs)");
		    }
		}

		if (debug != null)
		    debug.println("-checker" + (j+1) + " validation succeeded");
	    }

            if (debug != null)
                debug.println("checking for unresolvedCritExts");
            if (!unresolvedCritExts.isEmpty()) {
                throw new CertPathValidatorException("unrecognized " +
                    "critical extension(s)", null, cpOriginal, cpSize-(i+1));
            }
            
	    if (debug != null)
                debug.println("\ncert" + (i+1) + " validation succeeded.\n");
	}
	
        if (debug != null) {
            debug.println("Cert path validation succeeded. (PKIX validation "
		    + "algorithm)");
            debug.println("-------------------------------------------------"
		    + "-------------");
	}
    }

    /*
     * Examines the list of PKIX cert path checkers to determine whether
     * both the current checker and the next checker are revocation checkers.
     * OCSPChecker and CrlRevocationChecker are both revocation checkers.
     */
    private boolean isRevocationCheck(PKIXCertPathChecker checker, int index,
	List checkers) {

	if (checker instanceof OCSPChecker && index + 1 < checkers.size()) {
	    Object nextChecker = checkers.get(index + 1);
	    if (nextChecker instanceof CrlRevocationChecker) {
		return true;
	    }
	}
	return false;
    }
}
