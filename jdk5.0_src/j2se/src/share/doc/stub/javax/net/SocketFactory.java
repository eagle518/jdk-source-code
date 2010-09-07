/*
 * @(#)SocketFactory.java	1.9 04/02/16
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

package javax.net;

import java.net.*;

import java.io.IOException;

/** 
 * This class creates sockets.  It may be subclassed by other factories,
 * which create particular subclasses of sockets and thus provide a general
 * framework for the addition of public socket-level functionality.
 *
 * <P> Socket factories are a simple way to capture a variety of policies
 * related to the sockets being constructed, producing such sockets in
 * a way which does not require special configuration of the code which
 * asks for the sockets:  <UL>
 *
 *	<LI> Due to polymorphism of both factories and sockets, different
 *	kinds of sockets can be used by the same application code just
 *	by passing it different kinds of factories.
 *
 *	<LI> Factories can themselves be customized with parameters used
 *	in socket construction.  So for example, factories could be
 *	customized to return sockets with different networking timeouts
 *      or security parameters already configured.
 *
 *	<LI> The sockets returned to the application can be subclasses
 *	of java.net.Socket, so that they can directly expose new APIs
 *	for features such as compression, security, record marking,
 *	statistics collection, or firewall tunneling.
 *
 *	</UL>
 *
 * <P> Factory classes are specified by environment-specific configuration
 * mechanisms.  For example, the <em>getDefault</em> method could return
 * a factory that was appropriate for a particular user or applet, and a
 * framework could use a factory customized to its own purposes.
 *
 * @since 1.4
 * @see ServerSocketFactory
 *
 * @version 1.17
 * @author David Brownell
 */
public abstract class SocketFactory
{

    /** 
     * Creates a <code>SocketFactory</code>.
     */
    protected SocketFactory() { }

    /** 
     * Returns a copy of the environment's default socket factory.
     *
     * @return the default <code>SocketFactory</code>
     */
    public static SocketFactory getDefault() {
        return null;
    }

    /** 
     * Creates an unconnected socket.
     *
     * @return the unconnected socket
     * @throws IOException if the socket cannot be created
     * @see java.net.Socket#connect(java.net.SocketAddress)
     * @see java.net.Socket#connect(java.net.SocketAddress, int)
     * @see java.net.Socket#Socket()
     */
    public Socket createSocket() throws IOException {
        return null;
    }

    /** 
     * Creates a socket and connects it to the specified remote host
     * at the specified remote port.  This socket is configured using
     * the socket options established for this factory.
     *
     * @param host the server host
     * @param port the server port
     * @return the <code>Socket</code>
     * @throws IOException if an I/O error occurs when creating the socket
     * @throws UnknownHostException if the host is not known
     * @see java.net.Socket#Socket(String, int)
     */
    public abstract Socket createSocket(String host, int port)
        throws IOException, UnknownHostException;

    /** 
     * Creates a socket and connects it to the specified remote host
     * on the specified remote port.
     * The socket will also be bound to the local address and port supplied.
     * This socket is configured using
     * the socket options established for this factory.
     *
     * @param host the server host
     * @param port the server port
     * @param localHost the local address the socket is bound to
     * @param localPort the local port the socket is bound to
     * @return the <code>Socket</code>
     * @throws IOException if an I/O error occurs when creating the socket
     * @throws UnknownHostException if the host is not known
     * @see java.net.Socket#Socket(String, int, java.net.InetAddress, int)
     */
    public abstract Socket createSocket(String host, int port, InetAddress
        localHost, int localPort) throws IOException, UnknownHostException;

    /** 
     * Creates a socket and connects it to the specified port number
     * at the specified address.  This socket is configured using
     * the socket options established for this factory.
     *
     * @param host the server host
     * @param port the server port
     * @return the <code>Socket</code>
     * @throws IOException if an I/O error occurs when creating the socket
     * @see java.net.Socket#Socket(java.net.InetAddress, int)
     */
    public abstract Socket createSocket(InetAddress host, int port)
        throws IOException;

    /** 
     * Creates a socket and connect it to the specified remote address
     * on the specified remote port.  The socket will also be bound
     * to the local address and port suplied.  The socket is configured using
     * the socket options established for this factory.
     *
     * @param address the server network address
     * @param port the server port
     * @param localAddress the client network address
     * @param localPort the client port
     * @return the <code>Socket</code>
     * @throws IOException if an I/O error occurs when creating the socket
     * @see java.net.Socket#Socket(java.net.InetAddress, int,
     *     java.net.InetAddress, int)
     */
    public abstract Socket createSocket(InetAddress address, int port,
        InetAddress localAddress, int localPort) throws IOException;
}
