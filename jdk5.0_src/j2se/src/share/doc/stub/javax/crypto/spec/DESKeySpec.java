/*
 * @(#)DESKeySpec.java	1.9 04/01/14
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

import java.security.InvalidKeyException;

/** 
 * This class specifies a DES key.
 *
 * @author Jan Luehe
 *
 * @version 1.21, 01/07/04
 * @since 1.4
 */
public class DESKeySpec implements java.security.spec.KeySpec
{
    /** 
     * The constant which defines the length of a DES key in bytes.
     */
    public static final int DES_KEY_LEN = 8;

    /** 
     * Creates a DESKeySpec object using the first 8 bytes in 
     * <code>key</code> as the key material for the DES key.
     *
     * <p> The bytes that constitute the DES key are those between
     * <code>key[0]</code> and <code>key[7]</code> inclusive.
     *
     * @param key the buffer with the DES key material. The first 8 bytes
     * of the buffer are copied to protect against subsequent modification.
     *
     * @exception NullPointerException if the given key material is 
     * <code>null</code>
     * @exception InvalidKeyException if the given key material is shorter
     * than 8 bytes.
     */
    public DESKeySpec(byte[] key) throws InvalidKeyException { }

    /** 
     * Creates a DESKeySpec object using the first 8 bytes in 
     * <code>key</code>, beginning at <code>offset</code> inclusive, 
     * as the key material for the DES key.
     *
     * <p> The bytes that constitute the DES key are those between
     * <code>key[offset]</code> and <code>key[offset+7]</code> inclusive.
     *
     * @param key the buffer with the DES key material. The first 8 bytes
     * of the buffer beginning at <code>offset</code> inclusive are copied 
     * to protect against subsequent modification.
     * @param offset the offset in <code>key</code>, where the DES key
     * material starts.
     *
     * @exception NullPointerException if the given key material is 
     * <code>null</code>
     * @exception InvalidKeyException if the given key material, starting at
     * <code>offset</code> inclusive, is shorter than 8 bytes.
     */
    public DESKeySpec(byte[] key, int offset) throws InvalidKeyException { }

    /** 
     * Returns the DES key material.
     *
     * @return the DES key material. Returns a new array
     * each time this method is called.
     */
    public byte[] getKey() { }

    /** 
     * Checks if the given DES key material, starting at <code>offset</code>
     * inclusive, is parity-adjusted.
     *
     * @param key the buffer with the DES key material.
     * @param offset the offset in <code>key</code>, where the DES key
     * material starts.
     *
     * @return true if the given DES key material is parity-adjusted, false
     * otherwise.
     *
     * @exception InvalidKeyException if the given key material is 
     * <code>null</code>, or starting at <code>offset</code> inclusive, is 
     * shorter than 8 bytes.
     */
    public static boolean isParityAdjusted(byte[] key, int offset)
        throws InvalidKeyException
    { }

    /** 
     * Checks if the given DES key material is weak or semi-weak.
     *
     * @param key the buffer with the DES key material.
     * @param offset the offset in <code>key</code>, where the DES key
     * material starts.
     *
     * @return true if the given DES key material is weak or semi-weak, false
     * otherwise.
     *
     * @exception InvalidKeyException if the given key material is
     * <code>null</code>, or starting at <code>offset</code> inclusive, is 
     * shorter than 8 bytes.
     */
    public static boolean isWeak(byte[] key, int offset)
        throws InvalidKeyException
    { }
}
