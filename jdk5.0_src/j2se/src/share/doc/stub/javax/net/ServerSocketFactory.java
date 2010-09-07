/*
 * @(#)ServerSocketFactory.java	1.10 04/02/16
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

import java.io.IOException;
import java.net.InetAddress;
import java.net.ServerSocket;
import java.net.SocketException;

/** 
 * This class creates server sockets.  It may be subclassed by other
 * factories, which create particular types of server sockets.  This
 * provides a general framework for the addition of public socket-level
 * functionality.  It is the server side analogue of a socket factory,
 * and similarly provides a way to capture a variety of policies related
 * to the sockets being constructed.
 *
 * <P> Like socket factories, server Socket factory instances have
 * methods used to create sockets. There is also an environment
 * specific default server socket factory; frameworks will often use
 * their own customized factory.
 *
 * @since 1.4
 * @see SocketFactory
 *
 * @version 1.19
 * @author David Brownell
 */
public abstract class ServerSocketFactory
{

    /** 
     * Creates a server socket factory.
     */
    protected ServerSocketFactory() { }

    /** 
     * Returns a copy of the environment's default socket factory.
     *
     * @return the <code>ServerSocketFactory</code>
     */
    public static ServerSocketFactory getDefault() {
        return null;
    }

    /** 
     * Returns an unbound server socket.  The socket is configured with
     * the socket options (such as accept timeout) given to this factory.
     *
     * @return the unbound socket
     * @throws IOException if the socket cannot be created
     * @see java.net.ServerSocket#bind(java.net.SocketAddress)
     * @see java.net.ServerSocket#bind(java.net.SocketAddress, int)
     * @see java.net.ServerSocket#ServerSocket()
     */
    public ServerSocket createServerSocket() throws IOException {
        return null;
    }

    /** 
     * Returns a server socket bound to the specified port.
     * The socket is configured with the socket options
     * (such as accept timeout) given to this factory.
     *
     * @param port the port to listen to
     * @return the <code>ServerSocket</code>
     * @exception IOException for networking errors
     * @see java.net.ServerSocket#ServerSocket(int)
     */
    public abstract ServerSocket createServerSocket(int port)
        throws IOException;

    /** 
     * Returns a server socket bound to the specified port, and uses the
     * specified connection backlog.  The socket is configured with
     * the socket options (such as accept timeout) given to this factory.
     *
     * @param port the port to listen to
     * @param backlog how many connections are queued
     * @return the <code>ServerSocket</code>
     * @exception IOException for networking errors
     * @see java.net.ServerSocket#ServerSocket(int, int)
     */
    public abstract ServerSocket createServerSocket(int port, int backlog)
        throws IOException;

    /** 
     * Returns a server socket bound to the specified port,
     * with a specified listen backlog and local IP.
     * The bindAddr argument can be used on a multi-homed host for a
     * <code>ServerSocket</code> that will only accept
     * connect requests to one
     * of its addresses.  The socket is configured
     * with the socket options (such as accept timeout) given to this factory.
     *
     * @param port the port to listen to
     * @param backlog how many connections are queued
     * @param ifAddress the network interface address to use
     * @return the <code>ServerSocket</code>
     * @exception IOException for networking errors
     * @see java.net.ServerSocket#ServerSocket(int, int, java.net.InetAddress)
     */
    public abstract ServerSocket createServerSocket(int port, int backlog,
        InetAddress ifAddress) throws IOException;
}
