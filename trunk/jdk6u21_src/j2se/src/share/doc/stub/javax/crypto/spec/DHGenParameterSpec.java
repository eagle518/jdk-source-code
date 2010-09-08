/*
 * @(#)DHGenParameterSpec.java	1.9 10/03/23
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
import java.security.spec.AlgorithmParameterSpec;

/** 
 * This class specifies the set of parameters used for generating
 * Diffie-Hellman (system) parameters for use in Diffie-Hellman key
 * agreement. This is typically done by a central
 * authority.
 * 
 * <p> The central authority, after computing the parameters, must send this
 * information to the parties looking to agree on a secret key.
 *
 * @author Jan Luehe
 *
 * @version 1.16, 01/06/04
 *
 * @see DHParameterSpec
 * @since 1.4
 */
public class DHGenParameterSpec implements AlgorithmParameterSpec
{

    /** 
     * Constructs a parameter set for the generation of Diffie-Hellman
     * (system) parameters. The constructed parameter set can be used to
     * initialize an
     * {@link java.security.AlgorithmParameterGenerator AlgorithmParameterGenerator}
     * object for the generation of Diffie-Hellman parameters.
     *
     * @param primeSize the size (in bits) of the prime modulus.
     * @param exponentSize the size (in bits) of the random exponent.
     */
    public DHGenParameterSpec(int primeSize, int exponentSize) { }

    /** 
     * Returns the size in bits of the prime modulus.
     *
     * @return the size in bits of the prime modulus
     */
    public int getPrimeSize() {
        return 0;
    }

    /** 
     * Returns the size in bits of the random exponent (private value).
     *
     * @return the size in bits of the random exponent (private value)
     */
    public int getExponentSize() {
        return 0;
    }
}
