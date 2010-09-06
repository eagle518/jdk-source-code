/*
 * @(#)GssKrb5Base.java	1.6 04/02/03
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package com.sun.security.sasl.gsskerb; 

import java.io.IOException;
import java.util.Map;
import java.util.logging.Logger;
import java.util.logging.Level;
import javax.security.sasl.*;
import com.sun.security.sasl.util.AbstractSaslImpl;
import org.ietf.jgss.*;

abstract class GssKrb5Base extends AbstractSaslImpl {

    private static final String KRB5_OID_STR = "1.2.840.113554.1.2.2";
    protected static Oid KRB5_OID;
    protected static final byte[] EMPTY = new byte[0];

    static {
	try {
	    KRB5_OID = new Oid(KRB5_OID_STR);
	} catch (GSSException ignore) {}
    }

    protected GSSContext secCtx = null;
    protected MessageProp msgProp;              // QOP and privacy for unwrap
    protected static final int JGSS_QOP = 0;	// unrelated to SASL QOP mask

    protected GssKrb5Base(Map props, String className) throws SaslException {
	super(props, className);
    }

    /**
     * Retrieves this mechanism's name.
     *
     * @return  The string "GSSAPI".
     */
    public String getMechanismName() {
	return "GSSAPI";
    }

    public byte[] unwrap(byte[] incoming, int start, int len) 
	throws SaslException {
	if (!completed) {
	    throw new IllegalStateException("GSSAPI authentication not completed");
	}

	// integrity will be true if either privacy or integrity negotiated
	if (!integrity) {
	    throw new IllegalStateException("No security layer negotiated");
	}

	try {
	    byte[] answer = secCtx.unwrap(incoming, start, len, msgProp);
	    if (logger.isLoggable(Level.FINEST)) {
		traceOutput(myClassName, "KRB501:Unwrap", "incoming: ", 
		    incoming, start, len);
		traceOutput(myClassName, "KRB502:Unwrap", "unwrapped: ", 
		    answer, 0, answer.length);
	    }
	    return answer;
	} catch (GSSException e) {
	    throw new SaslException("Problems unwrapping SASL buffer", e);
	}
    }

    public byte[] wrap(byte[] outgoing, int start, int len) throws SaslException {
	if (!completed) {
	    throw new IllegalStateException("GSSAPI authentication not completed");
	}

	// integrity will be true if either privacy or integrity negotiated
	if (!integrity) {
	    throw new IllegalStateException("No security layer negotiated");
	}

	// Generate GSS token 
	try {
	    byte[] answer = secCtx.wrap(outgoing, start, len, msgProp);
	    if (logger.isLoggable(Level.FINEST)) {
		traceOutput(myClassName, "KRB503:Wrap", "outgoing: ", 
		    outgoing, start, len);
		traceOutput(myClassName, "KRB504:Wrap", "wrapped: ", 
		    answer, 0, answer.length);
	    }
	    return answer;

	} catch (GSSException e) {
	    throw new SaslException("Problem performing GSS wrap", e);
	}
    }

    public void dispose() throws SaslException {
	if (secCtx != null) {
	    try {
		secCtx.dispose();
	    } catch (GSSException e) {
		throw new SaslException("Problem disposing GSS context", e);
	    }
	    secCtx = null;
	}
    }

    protected void finalize() throws Throwable {
	dispose();
    }
}
