/*
 * @(#)SSLServerSocketFactory.java	1.9 04/02/16
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

import java.security.*;

import java.io.IOException;
import java.net.InetAddress;
import java.net.ServerSocket;
import java.net.SocketException;
import javax.net.ServerSocketFactory;
import com.sun.net.ssl.internal.ssl.Debug;
import com.sun.net.ssl.internal.ssl.ExportControl;

/** 
 * <code>SSLServerSocketFactory</code>s create
 * <code>SSLServerSocket</code>s.
 *
 * @since 1.4
 * @see SSLSocket
 * @see SSLServerSocket
 * @version 1.22
 * @author David Brownell
 */
public abstract class SSLServerSocketFactory extends ServerSocketFactory
{

    /** 
     * Constructor is used only by subclasses.
     */
    protected SSLServerSocketFactory() { }

    /** 
     * Returns the default SSL server socket factory.
     * The default implementation can be changed by setting the value of the
     * "ssl.ServerSocketFactory.provider" security property (in the Java
     * security properties file) to the desired class.
     *
     * <p>If SSL has not been
     * configured properly for this virtual machine, the factory will be
     * inoperative (use of which will report instantiation exceptions).
     *
     * @return the default <code>ServerSocketFactory</code>
     */
    public static synchronized ServerSocketFactory getDefault() {
        return null;
    }

    /** 
     * Returns the list of cipher suites which are enabled by default.
     * Unless a different list is enabled, handshaking on an SSL connection
     * will use one of these cipher suites.  The minimum quality of service
     * for these defaults requires confidentiality protection and server
     * authentication (that is, no anonymous cipher suites).
     *
     * @see #getSupportedCipherSuites()
     * @return array of the cipher suites enabled by default
     */
    public abstract String[] getDefaultCipherSuites();

    /** 
     * Returns the names of the cipher suites which could be enabled for use
     * on an SSL connection created by this factory.
     * Normally, only a subset of these will actually
     * be enabled by default, since this list may include cipher suites which
     * do not meet quality of service requirements for those defaults.  Such
     * cipher suites are useful in specialized applications.
     *
     * @return an array of cipher suite names
     * @see #getDefaultCipherSuites()
     */
    public abstract String[] getSupportedCipherSuites();
}
