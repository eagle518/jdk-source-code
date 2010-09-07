/*
 * @(#)IvParameterSpec.java	1.9 04/03/15
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
 * This class specifies an <i>initialization vector</i> (IV). 
 * Examples which use IVs are ciphers in feedback mode, 
 * e.g., DES in CBC mode and RSA ciphers with OAEP encoding
 * operation.
 * 
 * @author Jan Luehe
 *
 * @version 1.21, 03/15/04
 * @since 1.4
 */
public class IvParameterSpec implements AlgorithmParameterSpec
{

    /** 
     * Creates an IvParameterSpec object using the bytes in <code>iv</code> 
     * as the IV.
     *
     * @param iv the buffer with the IV. The contents of the
     * buffer are copied to protect against subsequent modification.
     * @throws NullPointerException if <code>iv</code> is <code>null</code>
     */
    public IvParameterSpec(byte[] iv) { }

    /** 
     * Creates an IvParameterSpec object using the first <code>len</code> 
     * bytes in <code>iv</code>, beginning at <code>offset</code> 
     * inclusive, as the IV.
     *
     * <p> The bytes that constitute the IV are those between
     * <code>iv[offset]</code> and <code>iv[offset+len-1]</code> inclusive.
     *
     * @param iv the buffer with the IV. The first <code>len</code>
     * bytes of the buffer beginning at <code>offset</code> inclusive
     * are copied to protect against subsequent modification.
     * @param offset the offset in <code>iv</code> where the IV
     * starts.
     * @param len the number of IV bytes.
     * @throws IllegalArgumentException if <code>iv</code> is <code>null</code>
     * or <code>(iv.length - offset < len)</code>
     * @throws ArrayIndexOutOfBoundsException is thrown if <code>offset</code>
     * or <code>len</code> index bytes outside the <code>iv</code>.
     */
    public IvParameterSpec(byte[] iv, int offset, int len) { }

    /** 
     * Returns the initialization vector (IV).
     *
     * @return the initialization vector (IV). Returns a new array 
     * each time this method is called.
     */
    public byte[] getIV() { }
}
