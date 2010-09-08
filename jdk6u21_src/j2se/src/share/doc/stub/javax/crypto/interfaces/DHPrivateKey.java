/*
 * @(#)DHPrivateKey.java	1.8 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */
  
/*
 * NOTE:
 * Because of various external restrictions (i.e. US export
 * regulations, etc.), the actual source code can not be provided
 * at this time. This file represents the skeleton of the source
 * file, so that javadocs of the API can be created.
 */

package javax.crypto.interfaces;

import java.math.BigInteger;

/** 
 * The interface to a Diffie-Hellman private key.
 *
 * @author Jan Luehe
 *
 * @version 1.12, 01/14/04
 *
 * @see DHKey
 * @see DHPublicKey
 * @since 1.4
 */
public interface DHPrivateKey extends DHKey, java.security.PrivateKey
{
    /** 
     * The class fingerprint that is set to indicate serialization
     * compatibility since J2SE 1.4. 
     */
    public static final long serialVersionUID = 2211791113380396553L;

    /** 
     * Returns the private value, <code>x</code>.
     *
     * @return the private value, <code>x</code>
     */
    public BigInteger getX();
}
