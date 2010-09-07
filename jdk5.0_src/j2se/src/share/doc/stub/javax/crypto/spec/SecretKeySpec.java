/*
 * @(#)SecretKeySpec.java	1.8 04/03/31
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

import java.io.UnsupportedEncodingException;
import java.security.Key;
import java.security.spec.KeySpec;
import javax.crypto.SecretKey;

/** 
 * This class specifies a secret key in a provider-independent fashion.
 *
 * <p>It can be used to construct a <code>SecretKey</code> from a byte array,
 * without having to go through a (provider-based)
 * <code>SecretKeyFactory</code>.
 *
 * <p>This class is only useful for raw secret keys that can be represented as
 * a byte array and have no key parameters associated with them, e.g., DES or
 * Triple DES keys.
 *
 * @author Jan Luehe
 *
 * @version 1.24, 03/31/04
 *
 * @see javax.crypto.SecretKey
 * @see javax.crypto.SecretKeyFactory
 * @since 1.4
 */
public class SecretKeySpec implements KeySpec, SecretKey
{
    /** 
     * The secret key.
     *
     * @serial
     */
    private byte[] key;

    /** 
     * The name of the algorithm associated with this key.
     *
     * @serial
     */
    private String algorithm;

    /** 
     * Constructs a secret key from the given byte array.
     *
     * <p>This constructor does not check if the given bytes indeed specify a
     * secret key of the specified algorithm. For example, if the algorithm is
     * DES, this constructor does not check if <code>key</code> is 8 bytes
     * long, and also does not check for weak or semi-weak keys.
     * In order for those checks to be performed, an algorithm-specific
     * <i>key specification</i> class (in this case:
     * {@link DESKeySpec DESKeySpec})
     * should be used.
     *
     * @param key the key material of the secret key. The contents of
     * the array are copied to protect against subsequent modification.
     * @param algorithm the name of the secret-key algorithm to be associated
     * with the given key material.
     * See Appendix A in the <a href="../../../../guide/security/jce/JCERefGuide.html">
     * Java Cryptography Extension Reference Guide</a> 
     * for information about standard algorithm names.
     * @exception IllegalArgumentException if <code>algorithm</code>
     * is null or <code>key</code> is null or empty.
     */
    public SecretKeySpec(byte[] key, String algorithm) { }

    /** 
     * Constructs a secret key from the given byte array, using the first
     * <code>len</code> bytes of <code>key</code>, starting at
     * <code>offset</code> inclusive.
     *
     * <p> The bytes that constitute the secret key are
     * those between <code>key[offset]</code> and
     * <code>key[offset+len-1]</code> inclusive.
     *
     * <p>This constructor does not check if the given bytes indeed specify a
     * secret key of the specified algorithm. For example, if the algorithm is
     * DES, this constructor does not check if <code>key</code> is 8 bytes
     * long, and also does not check for weak or semi-weak keys.
     * In order for those checks to be performed, an algorithm-specific key
     * specification class (in this case:
     * {@link DESKeySpec DESKeySpec})
     * must be used.
     *
     * @param key the key material of the secret key. The first
     * <code>len</code> bytes of the array beginning at 
     * <code>offset</code> inclusive are copied to protect
     * against subsequent modification.
     * @param offset the offset in <code>key</code> where the key material
     * starts.
     * @param len the length of the key material.
     * @param algorithm the name of the secret-key algorithm to be associated
     * with the given key material.
     * See Appendix A in the <a href="../../../../guide/security/jce/JCERefGuide.html">
     * Java Cryptography Extension Reference Guide</a> 
     * for information about standard algorithm names.
     * @exception IllegalArgumentException if <code>algorithm</code>
     * is null or <code>key</code> is null, empty, or too short,
     * i.e. <code>key.length-offset<len</code>.
     * @exception ArrayIndexOutOfBoundsException is thrown if 
     * <code>offset</code> or <code>len</code> index bytes outside the 
     * <code>key</code>.
     */
    public SecretKeySpec(byte[] key, int offset, int len, String algorithm) { }

    /** 
     * Returns the name of the algorithm associated with this secret key.
     *
     * @return the secret key algorithm.
     */
    public String getAlgorithm() { }

    /** 
     * Returns the name of the encoding format for this secret key.
     *
     * @return the string "RAW".
     */
    public String getFormat() { }

    /** 
     * Returns the key material of this secret key.
     *
     * @return the key material. Returns a new array
     * each time this method is called.
     */
    public byte[] getEncoded() { }

    /** 
     * Calculates a hash code value for the object.
     * Objects that are equal will also have the same hashcode.
     */
    public int hashCode() { }

    /** 
     * Tests for equality between the specified object and this
     * object. Two SecretKeySpec objects are considered equal if 
     * they are both SecretKey instances which have the 
     * same case-insensitive algorithm name and key encoding.
     *
     * @param obj the object to test for equality with this object.
     *
     * @return true if the objects are considered equal, false if
     * <code>obj</code> is null or otherwise.
     */
    public boolean equals(Object obj) { }
}
