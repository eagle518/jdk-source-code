/*
 * @(#)CertificateException.java	1.8 04/02/16
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

package javax.security.cert;

/** 
 * This exception indicates one of a variety of certificate problems.
 *
 * <p><em>Note: The classes in the package <code>javax.security.cert</code>
 * exist for compatibility with earlier versions of the
 * Java Secure Sockets Extension (JSSE). New applications should instead
 * use the standard J2SE certificate classes located in
 * <code>java.security.cert</code>.</em></p>
 *
 * @author Hemma Prafullchandra
 * @version 1.29
 * @since 1.4
 * @see Certificate
 */
public class CertificateException extends Exception
{

    /** 
     * Constructs a certificate exception with no detail message. A detail
     * message is a String that describes this particular exception. 
     */
    public CertificateException() { }

    /** 
     * Constructs a certificate exception with the given detail
     * message. A detail message is a String that describes this
     * particular exception.
     *
     * @param msg the detail message.  
     */
    public CertificateException(String msg) { }
}
