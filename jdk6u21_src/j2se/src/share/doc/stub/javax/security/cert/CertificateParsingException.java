/*
 * @(#)CertificateParsingException.java	1.11 10/03/23
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

package javax.security.cert;

/** 
 * Certificate Parsing Exception. This is thrown whenever
 * invalid DER encoded certificate is parsed or unsupported DER features
 * are found in the Certificate.
 *
 * <p><em>Note: The classes in the package <code>javax.security.cert</code>
 * exist for compatibility with earlier versions of the
 * Java Secure Sockets Extension (JSSE). New applications should instead
 * use the standard Java SE certificate classes located in
 * <code>java.security.cert</code>.</em></p>
 *
 * @since 1.4
 * @author Hemma Prafullchandra
 * @version 1.9
 */
public class CertificateParsingException extends CertificateException
{

    /** 
     * Constructs a CertificateParsingException with no detail message. A
     * detail message is a String that describes this particular
     * exception.
     */
    public CertificateParsingException() { }

    /** 
     * Constructs a CertificateParsingException with the specified detail
     * message. A detail message is a String that describes this
     * particular exception.
     *   
     * @param message the detail message.
     */
    public CertificateParsingException(String message) { }
}
