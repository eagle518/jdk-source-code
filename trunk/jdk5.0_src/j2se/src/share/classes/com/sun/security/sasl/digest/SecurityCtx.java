/*
 * @(#)SecurityCtx.java	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.security.sasl.digest;

import javax.security.sasl.SaslException;

/**
  * Interface used for classes implementing integrity checking and privacy
  * for DIGEST-MD5 SASL mechanism implementation.
  * 
  * @see <a href="http://www.ietf.org/rfc/rfc2831.txt">RFC 2831</a>
  * - Using Digest Authentication as a SASL Mechanism
  *
  * @author Jonathan Bruce
  */

interface SecurityCtx {

    /**
     * Wrap out-going message and return wrapped message
     *
     * @throws SaslException
     */
    byte[] wrap(byte[] dest, int start, int len) 
	throws SaslException;

    /**
     * Unwrap incoming message and return original message
     *
     * @throws SaslException
     */
    byte[] unwrap(byte[] outgoing, int start, int len) 
	throws SaslException;
}
