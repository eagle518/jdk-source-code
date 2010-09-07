/*
 * @(#)RC5ParameterSpec.java	1.8 04/06/03
 *
 * Copyright (c) 2004 Sun Microsystems, Inc. All Rights Reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */
  
/*
 * NOTE:
 * Because of various external restrictions (i.e. US export
 * regulations, etc.), the actual source code can not be provided
 * at this time. This file represents the skeleton of the source
 * file, so that javadocs of the API can be created.
 */

package javax.crypto.spec;

import java.security.spec.AlgorithmParameterSpec;

/** 
 * This class specifies the parameters used with the
 * <a href="http://www.ietf.org/rfc/rfc2040.txt"><i>RC5</i></a>
 * algorithm.
 *
 * <p> The parameters consist of a version number, a rounds count, a word
 * size, and optionally an initialization vector (IV) (only in feedback mode).
 *
 * <p> This class can be used to initialize a <code>Cipher</code> object that
 * implements the <i>RC5</i> algorithm as supplied by
 * <a href="http://www.rsasecurity.com">RSA Security Inc.</a>,
 * or any parties authorized by RSA Security.
 *
 * @author Jan Luehe
 *
 * @version 1.16, 06/03/04
 * @since 1.4
 */
public class RC5ParameterSpec implements AlgorithmParameterSpec
{

    /** 
     * Constructs a parameter set for RC5 from the given version, number of
     * rounds and word size (in bits).
     *
     * @param version the version.
     * @param rounds the number of rounds.
     * @param wordSize the word size in bits.
     */
    public RC5ParameterSpec(int version, int rounds, int wordSize) { }

    /** 
     * Constructs a parameter set for RC5 from the given version, number of
     * rounds, word size (in bits), and IV.
     *
     * <p> Note that the size of the IV (block size) must be twice the word
     * size. The bytes that constitute the IV are those between
     * <code>iv[0]</code> and <code>iv[2*(wordSize/8)-1]</code> inclusive.
     *
     * @param version the version.
     * @param rounds the number of rounds.
     * @param wordSize the word size in bits.
     * @param iv the buffer with the IV. The first <code>2*(wordSize/8)
     * </code> bytes of the buffer are copied to protect against subsequent 
     * modification.
     * @exception IllegalArgumentException if <code>iv</code> is
     * <code>null</code> or <code>(iv.length < 2 * (wordSize / 8))</code> 
     */
    public RC5ParameterSpec(int version, int rounds, int wordSize, byte[] iv)
    { }

    /** 
     * Constructs a parameter set for RC5 from the given version, number of
     * rounds, word size (in bits), and IV.
     *
     * <p> The IV is taken from <code>iv</code>, starting at
     * <code>offset</code> inclusive.
     * Note that the size of the IV (block size), starting at
     * <code>offset</code> inclusive, must be twice the word size.
     * The bytes that constitute the IV are those between
     * <code>iv[offset]</code> and <code>iv[offset+2*(wordSize/8)-1]</code>
     * inclusive.
     *
     * @param version the version.
     * @param rounds the number of rounds.
     * @param wordSize the word size in bits.
     * @param iv the buffer with the IV. The first <code>2*(wordSize/8)
     * </code> bytes of the buffer beginning at <code>offset</code> 
     * inclusive are copied to protect against subsequent modification.
     * @param offset the offset in <code>iv</code> where the IV starts.
     * @exception IllegalArgumentException if <code>iv</code> is
     * <code>null</code> or 
     * <code>(iv.length - offset < 2 * (wordSize / 8))</code> 
     */
    public RC5ParameterSpec(int version, int rounds, int wordSize, byte[] iv,
        int offset)
    { }

    /** 
     * Returns the version.
     *
     * @return the version.
     */
    public int getVersion() {
        return 0;
    }

    /** 
     * Returns the number of rounds.
     *
     * @return the number of rounds.
     */
    public int getRounds() {
        return 0;
    }

    /** 
     * Returns the word size in bits.
     *
     * @return the word size in bits.
     */
    public int getWordSize() {
        return 0;
    }

    /** 
     * Returns the IV or null if this parameter set does not contain an IV.
     *
     * @return the IV or null if this parameter set does not contain an IV.
     * Returns a new array each time this method is called.
     */
    public byte[] getIV() {
        return null;
    }

    /** 
     * Tests for equality between the specified object and this
     * object. Two RC5ParameterSpec objects are considered equal if their 
     * version numbers, number of rounds, word sizes, and IVs are equal.
     * (Two IV references are considered equal if both are <tt>null</tt>.)
     * 
     * @param obj the object to test for equality with this object.
     * 
     * @return true if the objects are considered equal, false if 
     * <code>obj</code> is null or otherwise.
     */
    public boolean equals(Object obj) {
        return false;
    }

    /** 
     * Calculates a hash code value for the object.
     * Objects that are equal will also have the same hashcode.
     */
    public int hashCode() {
        return 0;
    }
}
