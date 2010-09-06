/*
 * @(#)Validator.java	1.5, 04/05/18
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.validator;

import java.util.*;

import java.security.KeyStore;
import java.security.cert.*;

/**
 * Validator abstract base class. Concrete classes are instantiated by calling
 * one of the getInstance() methods. All methods defined in this class
 * must be safe for concurrent use by multiple threads.<p>
 *
 * The model is that a Validator instance is created specifying validation
 * settings, such as trust anchors or PKIX parameters. Then one or more
 * paths are validated using those parameters. In some cases, additional
 * information can be provided per path validation. This is independent of
 * the validation parameters and currently only used for TLS server validation.
 * <p>
 * Path validation is performed by calling one of the validate() methods. It
 * specifies a suggested path to be used for validation if available, or only
 * the end entity certificate otherwise. Optionally additional certificates can
 * be specified that the caller believes could be helpful. Implementations are
 * free to make use of this information or validate the path using other means.
 * validate() also checks that the end entity certificate is suitable for the
 * intended purpose as described below.
 *
 * <p>There are two orthogonal parameters to select the Validator
 * implementation: type and variant. Type selects the validation algorithm.
 * Currently supported are TYPE_SIMPLE and TYPE_PKIX. See SimpleValidator and
 * PKIXValidator for details.
 * <p>
 * Variant controls additional extension checks. Currently supported are
 * five variants:
 * <ul>
 * <li>VAR_GENERIC (no additional checks),
 * <li>VAR_TLS_CLIENT (TLS client specific checks)
 * <li>VAR_TLS_SERVER (TLS server specific checks), and
 * <li>VAR_CODE_SIGNING (code signing specific checks).
 * <li>VAR_JCE_SIGNING (JCE code signing specific checks).
 * </ul>
 * See EndEntityChecker for more information.
 * <p>
 * Examples:
 * <pre>
 *   // instantiate validator specifying type, variant, and trust anchors
 *   Validator validator = Validator.getInstance(Validator.TYPE_PKIX,
 *                                               Validator.VAR_TLS_CLIENT,
 *                                               trustedCerts);
 *   // validate one or more chains using the validator
 *   validator.validate(chain); // throws CertificateException if failed
 * </pre>
 *
 * @see SimpleValidator
 * @see PKIXValidator
 * @see EndEntityChecker
 *
 * @author Andreas Sterbenz
 * @version 1.5, 05/18/04
 */
public abstract class Validator {

    final static X509Certificate[] CHAIN0 = {};

    /**
     * Constant for a validator of type Simple.
     * @see #getInstance
     */
    public final static String TYPE_SIMPLE = "Simple";

    /**
     * Constant for a validator of type PKIX.
     * @see #getInstance
     */
    public final static String TYPE_PKIX = "PKIX";

    /**
     * Constant for a Generic variant of a validator.
     * @see #getInstance
     */
    public final static String VAR_GENERIC = "generic";

    /**
     * Constant for a Code Signing variant of a validator.
     * @see #getInstance
     */
    public final static String VAR_CODE_SIGNING = "code signing";

    /**
     * Constant for a JCE Code Signing variant of a validator.
     * @see #getInstance
     */
    public final static String VAR_JCE_SIGNING = "jce signing";

    /**
     * Constant for a TLS Client variant of a validator.
     * @see #getInstance
     */
    public final static String VAR_TLS_CLIENT = "tls client";

    /**
     * Constant for a TLS Server variant of a validator.
     * @see #getInstance
     */
    public final static String VAR_TLS_SERVER = "tls server";

    final EndEntityChecker endEntityChecker;
    final String variant;
    
    /**
     * @deprecated
     * @see #setValidationDate
     */
    @Deprecated
    volatile Date validationDate;

    Validator(String type, String variant) {
        this.variant = variant;
        endEntityChecker = EndEntityChecker.getInstance(type, variant);
    }

    /**
     * Get a new Validator instance using the trusted certificates from the
     * specified KeyStore as trust anchors.
     */
    public static Validator getInstance(String type, String variant, 
	    KeyStore ks) {
        return getInstance(type, variant, KeyStores.getTrustedCerts(ks));
    }

    /**
     * Get a new Validator instance using the Set of X509Certificates as trust
     * anchors.
     */
    public static Validator getInstance(String type, String variant, 
	    Collection trustedCerts) {
        if (type.equals(TYPE_SIMPLE)) {
            return new SimpleValidator(variant, trustedCerts);
        } else if (type.equals(TYPE_PKIX)) {
            return new PKIXValidator(variant, trustedCerts);
        } else {
            throw new IllegalArgumentException
	    	("Unknown validator type: " + type);
        }
    }

    /**
     * Get a new Validator instance using the provided PKIXBuilderParameters.
     * This method can only be used with the PKIX validator.
     */
    public static Validator getInstance(String type, String variant, 
	    PKIXBuilderParameters params) {
        if (type.equals(TYPE_PKIX) == false) {
            throw new IllegalArgumentException
	    	("getInstance(PKIXBuilderParameters) can only be used "
		+ "with PKIX validator");
        }
        return new PKIXValidator(variant, params);
    }

    /**
     * Validate the given certificate chain.
     */
    public final X509Certificate[] validate(X509Certificate[] chain) 
	    throws CertificateException {
        return validate(chain, null, null);
    }

    /**
     * Validate the given certificate chain. If otherCerts is non-null, it is
     * a Collection of additional X509Certificates that could be helpful for
     * path building.
     */
    public final X509Certificate[] validate(X509Certificate[] chain, 
	    Collection otherCerts) throws CertificateException {
        return validate(chain, otherCerts, null);
    }

    /**
     * Validate the given certificate chain. If otherCerts is non-null, it is
     * a Collection of additional X509Certificates that could be helpful for
     * path building.
     * <p>
     * Parameter is an additional parameter with variant specific meaning.
     * Currently, it is only defined for TLS_SERVER variant validators, where
     * it must be non null and the name of the TLS key exchange algorithm being
     * used (see JSSE X509TrustManager specification). In the future, it
     * could be used to pass in a PKCS#7 object for code signing to check time
     * stamps.
     * <p>
     * @return a non-empty chain that was used to validate the path. The
     * end entity cert is at index 0, the trust anchor at index n-1.
     */
    public final X509Certificate[] validate(X509Certificate[] chain, 
	    Collection otherCerts, Object parameter) 
	    throws CertificateException {
        chain = engineValidate(chain, otherCerts, parameter);
	// omit EE extension check if EE cert is also trust anchor
	if (chain.length > 1) {
	    endEntityChecker.check(chain[0], parameter);
	}
        return chain;
    }

    abstract X509Certificate[] engineValidate(X509Certificate[] chain, 
	Collection otherCerts, Object parameter) throws CertificateException;

    /**
     * Returns an immutable Collection of the X509Certificates this instance
     * uses as trust anchors.
     */
    public abstract Collection getTrustedCertificates();
    
    /**
     * Set the date to be used for subsequent validations. NOTE that
     * this is not a supported API, it is provided to simplify
     * writing tests only.
     *
     * @deprecated
     */
    @Deprecated
    public void setValidationDate(Date validationDate) {
	this.validationDate = validationDate;
    }
    
}
