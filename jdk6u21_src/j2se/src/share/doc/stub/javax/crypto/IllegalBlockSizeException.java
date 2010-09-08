/*
 * @(#)IllegalBlockSizeException.java	1.8 10/03/23
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
 * This exception is thrown when the length of data provided to a block
 * cipher is incorrect, i.e., does not match the block size of the cipher.
 *
 * @author Jan Luehe
 *
 * @version 1.17, 03/15/04
 * @since 1.4
 */
public class IllegalBlockSizeException
    extends java.security.GeneralSecurityException
{

    /** 
     * Constructs an IllegalBlockSizeException with no detail message.
     * A detail message is a String that describes this particular
     * exception.  
     */
    public IllegalBlockSizeException() { }

    /** 
     * Constructs an IllegalBlockSizeException with the specified
     * detail message. 
     *
     * @param msg the detail message. 
     */
    public IllegalBlockSizeException(String msg) { }
}
