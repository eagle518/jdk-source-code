/*
 * @(#)DHPublicKeySpec.java	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
 * This class specifies a Diffie-Hellman public key with its associated
 * parameters.
 *
 * <p>Note that this class does not perform any validation on specified
 * parameters. Thus, the specified values are returned directly even
 * if they are null.
 *
 * @author Jan Luehe
 *
 * @version 1.14, 10/29/03
 *
 * @see DHPrivateKeySpec
 * @since 1.4
 */
public class DHPublicKeySpec implements java.security.spec.KeySpec
{

    /** 
     * Constructor that takes a public value <code>y</code>, a prime
     * modulus <code>p</code>, and a base generator <code>g</code>.
     * @param y  public value y
     * @param p  prime modulus p
     * @param g  base generator g
     */
    public DHPublicKeySpec(BigInteger y, BigInteger p, BigInteger g) { }

    /** 
     * Returns the public value <code>y</code>.
     *
     * @return the public value <code>y</code>
     */
    public BigInteger getY() { }

    /** 
     * Returns the prime modulus <code>p</code>.
     *
     * @return the prime modulus <code>p</code>
     */
    public BigInteger getP() { }

    /** 
     * Returns the base generator <code>g</code>.
     *
     * @return the base generator <code>g</code>
     */
    public BigInteger getG() { }
}
