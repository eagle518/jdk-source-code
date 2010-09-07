/*
 * @(#)HostnameVerifier.java	1.8 04/02/16
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

package javax.net.ssl;

/** 
 * This class is the base interface for hostname verification.
 * <P>
 * During handshaking, if the URL's hostname and
 * the server's identification hostname mismatch, the 
 * verification mechanism can call back to implementers of this
 * interface to determine if this connection should be allowed.
 * <P>
 * The policies can be certificate-based
 * or may depend on other authentication schemes.
 * <P>
 * These callbacks are used when the default rules for URL hostname
 * verification fail.
 *
 * @author Brad R. Wetmore
 * @version 1.4, 06/24/03
 * @since 1.4
 */
public interface HostnameVerifier
{

    /** 
     * Verify that the host name is an acceptable match with
     * the server's authentication scheme.
     *
     * @param hostname the host name
     * @param session SSLSession used on the connection to host
     * @return true if the host name is acceptable
     */
    public boolean verify(String hostname, SSLSession session);
}
