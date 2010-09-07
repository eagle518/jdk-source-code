/*
 * @(#)BadPaddingException.java	1.6 04/03/15
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

package javax.crypto;

import java.security.GeneralSecurityException;

/** 
 * This exception is thrown when a particular padding mechanism is
 * expected for the input data but the data is not padded properly.
 *
 * @author Gigi Ankney
 *
 * @version 1.11, 03/15/04
 * @since 1.4
 */
public class BadPaddingException extends GeneralSecurityException
{

    /** 
     * Constructs a BadPaddingException with no detail
     * message. A detail message is a String that describes this
     * particular exception.
     */
    public BadPaddingException() { }

    /** 
     * Constructs a BadPaddingException with the specified
     * detail message. 
     *
     * @param msg the detail message.  
     */
    public BadPaddingException(String msg) { }
}
