/*
 * @(#)DHPrivateKeySpec.java	1.10 10/03/23
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

package javax.crypto.spec;

import java.math.BigInteger;

/** 
 * This class specifies a Diffie-Hellman private key with its associated
 * parameters.
 *
 * <p>Note that this class does not perform any validation on specified
 * parameters. Thus, the specified values are returned directly even
 * if they are null.
 *
 * @author Jan Luehe
 *
 * @version 1.15, 01/06/04
 *
 * @see DHPublicKeySpec
 * @since 1.4
 */
public class DHPrivateKeySpec implements java.security.spec.KeySpec
{

    /** 
     * Constructor that takes a private value <code>x</code>, a prime
     * modulus <code>p</code>, and a base generator <code>g</code>.
     * @param x private value x 
     * @param p prime modulus p
     * @param g base generator g
     */
    public DHPrivateKeySpec(BigInteger x, BigInteger p, BigInteger g) { }

    /** 
     * Returns the private value <code>x</code>.
     *
     * @return the private value <code>x</code>
     */
    public BigInteger getX() {
        return null;
    }

    /** 
     * Returns the prime modulus <code>p</code>.
     *
     * @return the prime modulus <code>p</code>
     */
    public BigInteger getP() {
        return null;
    }

    /** 
     * Returns the base generator <code>g</code>.
     *
     * @return the base generator <code>g</code>
     */
    public BigInteger getG() {
        return null;
    }
}
