/*
 * @(#)NullCipher.java	1.8 10/03/23
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

package javax.crypto;

/** 
 * The NullCipher class is a class that provides an
 * "identity cipher" -- one that does not tranform the plaintext.  As
 * a consequence, the ciphertext is identical to the plaintext.  All
 * initialization methods do nothing, while the blocksize is set to 1
 * byte.
 *
 * @author  Li Gong
 * @version 1.10, 01/06/04
 * @since 1.4
 */
public class NullCipher extends Cipher
{

    public NullCipher() { }
}
