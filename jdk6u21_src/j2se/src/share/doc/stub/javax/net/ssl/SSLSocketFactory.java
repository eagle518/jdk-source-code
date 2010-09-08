/*
 * @(#)SSLSocketFactory.java	1.15 10/03/23
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

package javax.net.ssl;

import java.net.*;
import java.security.*;

import javax.net.SocketFactory;
import java.io.IOException;
import com.sun.net.ssl.internal.ssl.Debug;

/** 
 * <code>SSLSocketFactory</code>s create <code>SSLSocket</code>s.
 *
 * @since 1.4
 * @see SSLSocket
 * @version 1.23
 * @author David Brownell
 */
public abstract class SSLSocketFactory extends SocketFactory
{

    /** 
     * Constructor is used only by subclasses.
     */
    public SSLSocketFactory() { }

    /** 
     * Returns the default SSL socket factory.
     *
     * <p>The first time this method is called, the security property
     * "ssl.SocketFactory.provider" is examined. If it is non-null, a class by
     * that name is loaded and instantiated. If that is successful and the
     * object is an instance of SSLSocketFactory, it is made the default SSL
     * socket factory.
     *
     * <p>Otherwise, this method returns
     * <code>SSLContext.getDefault().getSocketFactory()</code>. If that
     * call fails, an inoperative factory is returned.
     *
     * @return the default <code>SocketFactory</code>
     * @see SSLContext#getDefault
     */
    public static synchronized SocketFactory getDefault() {
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
     * on an SSL connection.  Normally, only a subset of these will actually
     * be enabled by default, since this list may include cipher suites which
     * do not meet quality of service requirements for those defaults.  Such
     * cipher suites are useful in specialized applications.
     *
     * @see #getDefaultCipherSuites()
     * @return an array of cipher suite names
     */
    public abstract String[] getSupportedCipherSuites();

    /** 
     * Returns a socket layered over an existing socket connected to the named
     * host, at the given port.  This constructor can be used when tunneling SSL
     * through a proxy or when negotiating the use of SSL over an existing
     * socket. The host and port refer to the logical peer destination.
     * This socket is configured using the socket options established for
     * this factory.
     *
     * @param s the existing socket
     * @param host the server host
     * @param port the server port
     * @param autoClose close the underlying socket when this socket is closed
     * @return a socket connected to the specified host and port
     * @throws IOException if an I/O error occurs when creating the socket
     * @throws UnknownHostException if the host is not known
     */
    public abstract Socket createSocket(Socket s, String host, int port, boolean
        autoClose) throws IOException;
}
