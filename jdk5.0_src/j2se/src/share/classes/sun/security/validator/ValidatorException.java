/*
 * @(#)ValidatorException.java	1.4, 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.validator;

import java.security.cert.*;

/**
 * ValidatorException thrown by the Validator. It has optional fields that
 * allow better error diagnostics.
 *
 * @author Andreas Sterbenz
 * @version 1.4, 12/19/03
 */
public class ValidatorException extends CertificateException {

    private static final long serialVersionUID = -2836879718282292155L;

    public final static Object T_NO_TRUST_ANCHOR = 
    	"No trusted certificate found";

    public final static Object T_EE_EXTENSIONS = 
    	"End entity certificate extension check failed";

    public final static Object T_CA_EXTENSIONS = 
    	"CA certificate extension check failed";

    public final static Object T_CERT_EXPIRED = 
    	"Certificate expired";

    public final static Object T_SIGNATURE_ERROR = 
    	"Certificate signature validation failed";

    public final static Object T_NAME_CHAINING = 
    	"Certificate chaining error";

    private Object type;
    private X509Certificate cert;

    public ValidatorException(String msg) {
        super(msg);
    }

    public ValidatorException(String msg, Throwable cause) {
        super(msg);
        initCause(cause);
    }

    public ValidatorException(Object type) {
	this(type, null);
    }

    public ValidatorException(Object type, X509Certificate cert) {
        super((String)type);
        this.type = type;
        this.cert = cert;
    }

    public ValidatorException(Object type, X509Certificate cert, 
	    Throwable cause) {
        this(type, cert);
        initCause(cause);
    }

    public ValidatorException(String msg, Object type, X509Certificate cert) {
        super(msg);
        this.type = type;
        this.cert = cert;
    }

    public ValidatorException(String msg, Object type, X509Certificate cert, 
	    Throwable cause) {
        this(msg, type, cert);
        initCause(cause);
    }

    /**
     * Get the type of the failure (one of the T_XXX constants), if
     * available. This may be helpful when designing a user interface.
     */
    public Object getErrorType() {
        return type;
    }

    /**
     * Get the certificate causing the exception, if available.
     */
    public X509Certificate getErrorCertificate() {
        return cert;
    }

}
