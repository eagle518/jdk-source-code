/*
 * @(#)EndEntityChecker.java	1.4, 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.validator;

import java.util.*;

import java.security.cert.*;

import sun.security.x509.NetscapeCertTypeExtension;

/**
 * Class to check if an end entity cert is suitable for use in some 
 * context.<p>
 *
 * This class is used internally by the validator. Currently, five variants
 * are supported defined as VAR_XXX constants in the Validator class:
 * <ul>
 * <li>Generic. No additional requirements, all certificates are ok.
 *
 * <li>TLS server. Requires that a String parameter is passed to
 * validate that specifies the name of the TLS key exchange algorithm
 * in use. See the JSSE X509TrustManager spec for details.
 *
 * <li>TLS client.
 *
 * <li>Code signing.
 *
 * <li>JCE code signing. Some early JCE code signing certs issued to
 * providers had incorrect extensions. In this mode the checks
 * are relaxed compared to standard code signing checks in order to
 * allow these certificates to pass.
 *
 * </ul>
 *
 * @author Andreas Sterbenz
 * @version 1.4, 12/19/03
 */
class EndEntityChecker {

    // extended key usage OIDs for TLS server, TLS client, code signing 
    // and any usage

    private final static String OID_EXTENDED_KEY_USAGE = 
    				SimpleValidator.OID_EXTENDED_KEY_USAGE;

    private final static String OID_EKU_TLS_SERVER = "1.3.6.1.5.5.7.3.1";

    private final static String OID_EKU_TLS_CLIENT = "1.3.6.1.5.5.7.3.2";

    private final static String OID_EKU_CODE_SIGNING = "1.3.6.1.5.5.7.3.3";

    private final static String OID_EKU_ANY_USAGE = "2.5.29.37.0";

    // the Netscape Server-Gated-Cryptography EKU extension OID
    private final static String OID_EKU_NS_SGC = "2.16.840.1.113730.4.1";

    // the Microsoft Server-Gated-Cryptography EKU extension OID
    private final static String OID_EKU_MS_SGC = "1.3.6.1.4.1.311.10.3.3";

    private final static String NSCT_SSL_CLIENT = 
    				NetscapeCertTypeExtension.SSL_CLIENT;

    private final static String NSCT_SSL_SERVER = 
    				NetscapeCertTypeExtension.SSL_SERVER;

    private final static String NSCT_CODE_SIGNING = 
    				NetscapeCertTypeExtension.OBJECT_SIGNING;
				
    // bit numbers in the key usage extension
    private final static int KU_SIGNATURE = 0;

    private final static int KU_KEY_ENCIPHERMENT = 2;
    
    // TLS key exchange algorithms requiring digitalSignature key usage
    private final static Collection KU_SERVER_SIGNATURE =
        Arrays.asList(new String[] 
			{"DHE_DSS", "DHE_RSA", "RSA_EXPORT", "UNKNOWN"});

    // TLS key exchange algorithms requiring keyEncipherment key usage
    private final static Collection KU_SERVER_ENCRYPTION =
        Arrays.asList(new String[] {"RSA"});

    // variant of this end entity cert checker
    private final String variant;
    
    // type of the validator this checker belongs to
    private final String type;

    private EndEntityChecker(String type, String variant) {
	this.type = type;
        this.variant = variant;
    }

    static EndEntityChecker getInstance(String type, String variant) {
        return new EndEntityChecker(type, variant);
    }

    void check(X509Certificate cert, Object parameter) 
	    throws CertificateException {
        if (variant.equals(Validator.VAR_GENERIC)) {
            // no checks
            return;
        } else if (variant.equals(Validator.VAR_TLS_SERVER)) {
            checkTLSServer(cert, (String)parameter);
        } else if (variant.equals(Validator.VAR_TLS_CLIENT)) {
            checkTLSClient(cert);
        } else if (variant.equals(Validator.VAR_CODE_SIGNING)) {
            checkCodeSigning(cert);
        } else if (variant.equals(Validator.VAR_JCE_SIGNING)) {
            checkCodeSigning(cert);
        } else {
            throw new CertificateException("Unknown variant: " + variant);
        }
    }

    /**
     * Utility method returning the Set of critical extensions for
     * certificate cert (never null).
     */
    private Set getCriticalExtensions(X509Certificate cert) {
        Set exts = cert.getCriticalExtensionOIDs();
        if (exts == null) {
            exts = Collections.EMPTY_SET;
        }
        return exts;
    }

    /**
     * Utility method checking if there are any unresolved critical extensions.
     * @throws CertificateException if so.
     */
    private void checkRemainingExtensions(Set exts) 
	    throws CertificateException {
        // basic constraints irrelevant in EE certs
        exts.remove(SimpleValidator.OID_BASIC_CONSTRAINTS);
        if (!exts.isEmpty()) {
            throw new CertificateException("Certificate contains unsupported "
                + "critical extensions: " + exts);
        }
    }

    /**
     * Utility method checking if the extended key usage extension in
     * certificate cert allows use for expectedEKU.
     */
    private boolean checkEKU(X509Certificate cert, Set exts, 
	    String expectedEKU) throws CertificateException {
	List eku = cert.getExtendedKeyUsage();
	if (eku == null) {
	    return true;
	}
	return eku.contains(expectedEKU) || eku.contains(OID_EKU_ANY_USAGE);
    }

    /**
     * Utility method checking if bit 'bit' is set in this certificates
     * key usage extension.
     * @throws CertificateException if not
     */
    private boolean checkKeyUsage(X509Certificate cert, int bit) 
	    throws CertificateException {
        boolean[] keyUsage = cert.getKeyUsage();
	if (keyUsage == null) {
	    return true;
	}
	return (keyUsage.length > bit) && keyUsage[bit];
    }

    /**
     * Check whether this certificate can be used for TLS client 
     * authentication.
     * @throws CertificateException if not.
     */
    private void checkTLSClient(X509Certificate cert) 
	    throws CertificateException {
        Set exts = getCriticalExtensions(cert);

        if (checkKeyUsage(cert, KU_SIGNATURE) == false) {
            throw new ValidatorException
	    	("KeyUsage does not allow digital signatures", 
		ValidatorException.T_EE_EXTENSIONS, cert);
        }

        if (checkEKU(cert, exts, OID_EKU_TLS_CLIENT) == false) {
            throw new ValidatorException("Extended key usage does not "
	    	+ "permit use for TLS client authentication",
		ValidatorException.T_EE_EXTENSIONS, cert);
        }

        if (!SimpleValidator.getNetscapeCertTypeBit(cert, NSCT_SSL_CLIENT)) {
            throw new ValidatorException
	    	("Netscape cert type does not permit use for SSL client", 
		ValidatorException.T_EE_EXTENSIONS, cert);
        }

        // remove extensions we checked
        exts.remove(SimpleValidator.OID_KEY_USAGE);
        exts.remove(SimpleValidator.OID_EXTENDED_KEY_USAGE);
        exts.remove(SimpleValidator.OID_NETSCAPE_CERT_TYPE);

        checkRemainingExtensions(exts);
    }

    /**
     * Check whether this certificate can be used for TLS server authentication
     * using the specified authentication type parameter. See X509TrustManager
     * specification for details.
     * @throws CertificateException if not.
     */
    private void checkTLSServer(X509Certificate cert, String parameter) 
	    throws CertificateException {
        Set exts = getCriticalExtensions(cert);

        if (KU_SERVER_ENCRYPTION.contains(parameter)) {
            if (checkKeyUsage(cert, KU_KEY_ENCIPHERMENT) == false) {
                throw new ValidatorException
			("KeyUsage does not allow key encipherment", 
			ValidatorException.T_EE_EXTENSIONS, cert);
            }
        } else if (KU_SERVER_SIGNATURE.contains(parameter)) {
            if (checkKeyUsage(cert, KU_SIGNATURE) == false) {
                throw new ValidatorException
			("KeyUsage does not allow digital signatures",
			ValidatorException.T_EE_EXTENSIONS, cert);
            }
        } else {
            throw new CertificateException("Unknown authType: " + parameter);
        }

        if (checkEKU(cert, exts, OID_EKU_TLS_SERVER) == false) {
	    // check for equivalent but now obsolete Server-Gated-Cryptography
	    // (aka Step-Up, 128 bit) EKU OIDs
	    if ((checkEKU(cert, exts, OID_EKU_MS_SGC) == false) &&
	        (checkEKU(cert, exts, OID_EKU_NS_SGC) == false)) {
            	throw new ValidatorException
	    	    ("Extended key usage does not permit use for TLS "
		    + "server authentication", 
		    ValidatorException.T_EE_EXTENSIONS, cert);
	    }
        }

        if (!SimpleValidator.getNetscapeCertTypeBit(cert, NSCT_SSL_SERVER)) {
            throw new ValidatorException
	    	("Netscape cert type does not permit use for SSL server", 
		ValidatorException.T_EE_EXTENSIONS, cert);
        }

        // remove extensions we checked
        exts.remove(SimpleValidator.OID_KEY_USAGE);
        exts.remove(SimpleValidator.OID_EXTENDED_KEY_USAGE);
        exts.remove(SimpleValidator.OID_NETSCAPE_CERT_TYPE);

        checkRemainingExtensions(exts);
    }

    /**
     * Check whether this certificate can be used for code signing.
     * @throws CertificateException if not.
     */
    private void checkCodeSigning(X509Certificate cert)
	    throws CertificateException {
        Set exts = getCriticalExtensions(cert);

        if (checkKeyUsage(cert, KU_SIGNATURE) == false) {
            throw new ValidatorException
	    	("KeyUsage does not allow digital signatures", 
		ValidatorException.T_EE_EXTENSIONS, cert);
        }

        if (checkEKU(cert, exts, OID_EKU_CODE_SIGNING) == false) {
            throw new ValidatorException
	    	("Extended key usage does not permit use for code signing", 
		ValidatorException.T_EE_EXTENSIONS, cert);
        }

	// do not check Netscape cert type for JCE code signing checks
	// (some certs were issued with incorrect extensions)
	if (variant.equals(Validator.VAR_JCE_SIGNING) == false) {
	    if (!SimpleValidator.getNetscapeCertTypeBit(cert, NSCT_CODE_SIGNING)) {
		throw new ValidatorException
		    ("Netscape cert type does not permit use for code signing",
		    ValidatorException.T_EE_EXTENSIONS, cert);
	    }
	    exts.remove(SimpleValidator.OID_NETSCAPE_CERT_TYPE);
	}

        // remove extensions we checked
        exts.remove(SimpleValidator.OID_KEY_USAGE);
        exts.remove(SimpleValidator.OID_EXTENDED_KEY_USAGE);

        checkRemainingExtensions(exts);
    }

}
