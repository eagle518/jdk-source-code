/*
 * @(#)TCPChannel.java	1.42 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.rmi.transport.tcp;

import java.lang.ref.SoftReference;
import java.io.*;
import java.net.Socket;
import java.util.*;
import java.rmi.*;
import java.rmi.server.ExportException;
import java.rmi.server.RMISocketFactory;
import java.security.AccessController;
import java.security.AccessControlContext;
import sun.rmi.runtime.Log;
import sun.rmi.runtime.NewThreadAction;
import sun.rmi.transport.Channel;
import sun.rmi.transport.Connection;
import sun.rmi.transport.Endpoint;
import sun.rmi.transport.Transport;
import sun.rmi.transport.TransportConstants;

import sun.security.action.GetIntegerAction;
import sun.security.action.GetLongAction;

/**
 * TCPChannel is the socket-based implementation of the RMI Channel
 * abstraction.
 *
 * @author Ann Wollrath
 */
public class TCPChannel implements Channel {
    /** endpoint for this channel */
    private final TCPEndpoint ep;
    /** transport for this channel */
    private final TCPTransport tr;
    /** list of cached connections */
    private final List freeList = new ArrayList();
    /** frees cached connections that have expired (guarded by freeList) */
    private Reaper reaper = null;

    /** using multiplexer (for bi-directional applet communication */
    private boolean usingMultiplexer = false;
    /** connection multiplexer, if used */
    private ConnectionMultiplexer multiplexer = null;
    /** connection acceptor (should be in TCPTransport) */
    private ConnectionAcceptor acceptor;

    /** most recently authorized AccessControlContext */
    private AccessControlContext okContext;

    /** cache of authorized AccessControlContexts */
    private WeakHashMap authcache;

    /** the SecurityManager which authorized okContext and authcache */
    private SecurityManager cacheSecurityManager = null;

    /** client-side connection idle usage timeout */
    private static final long idleTimeout =		// default 15 seconds
	((Long) java.security.AccessController.doPrivileged(
	    new GetLongAction("sun.rmi.transport.connectionTimeout",
			      15000))).longValue();

    /** client-side connection handshake read timeout */
    private static final int handshakeTimeout =		// default 1 minute
	((Integer) java.security.AccessController.doPrivileged(
	    new GetIntegerAction("sun.rmi.transport.tcp.handshakeTimeout",
				 60000))).intValue();

    /** client-side connection response read timeout (after handshake) */
    private static final int responseTimeout =		// default infinity
	((Integer) java.security.AccessController.doPrivileged(
	    new GetIntegerAction("sun.rmi.transport.tcp.responseTimeout",
				 0))).intValue();

    /**
     * Create channel for endpoint.
     */
    TCPChannel(TCPTransport tr, TCPEndpoint ep) {
	this.tr = tr;
	this.ep = ep;
    }

    /**
     * Return the endpoint for this channel.
     */
    public Endpoint getEndpoint() {
	return ep;
    }

    /**
     * Checks if the current caller has sufficient privilege to make
     * a connection to the remote endpoint.
     * @exception SecurityException if caller is not allowed to use this
     * Channel.
     */
    private void checkConnectPermission() throws SecurityException {
	SecurityManager security = System.getSecurityManager();
	if (security == null)
	    return;

	if (security != cacheSecurityManager) {
	    // The security manager changed: flush the cache
	    okContext = null;
	    authcache = new WeakHashMap();
	    cacheSecurityManager = security;
	}

	AccessControlContext ctx = AccessController.getContext();

	// If ctx is the same context as last time, or if it
	// appears in the cache, bypass the checkConnect.
	if (okContext == null ||
	       !(okContext.equals(ctx) || authcache.containsKey(ctx)))
	{
	    security.checkConnect(ep.getHost(), ep.getPort());
	    authcache.put(ctx, new SoftReference(ctx));
	    // A WeakHashMap is transformed into a SoftHashSet by making
	    // each value softly refer to its own key (Peter's idea).
	}
	okContext = ctx;
    }

    /**
     * Supplies a connection to the endpoint of the address space
     * for which this is a channel.  The returned connection may
     * be one retrieved from a cache of idle connections.
     */
    public Connection newConnection() throws RemoteException {
	TCPConnection conn;

	// loop until we find a free live connection (in which case
	// we return) or until we run out of freelist (in which case
	// the loop exits)
	do {
	    conn = null;
	    // try to get a free connection 
	    synchronized (freeList) {
		int elementPos = freeList.size()-1;

		if (elementPos >= 0) {
		    // If there is a security manager, make sure
		    // the caller is allowed to connect to the
		    // requested endpoint.
		    checkConnectPermission();
		    conn = (TCPConnection)freeList.get(elementPos);
		    freeList.remove(elementPos);
		}
	    }

	    // at this point, conn is null iff the freelist is empty,
	    // and nonnull if a free connection of uncertain vitality
	    // has been found.

	    if (conn != null) {
		// check to see if the connection has closed since last use
		if (!conn.isDead()) {
		    TCPTransport.tcpLog.log(Log.BRIEF, "reuse connection");
		    return conn;
		}

		// conn is dead, and cannot be reused (reuse => false)
		this.free(conn, false);
	    }
	} while (conn != null);

	// none free, so create a new connection
	return (createConnection());
    }

    /**
     * Create a new connection to the remote endpoint of this channel.
     * The returned connection is new.  The caller must already have
     * passed a security checkConnect or equivalent.
     */
    private Connection createConnection() throws RemoteException {
	Connection conn;

	TCPTransport.tcpLog.log(Log.BRIEF, "create connection");

	if (!usingMultiplexer) {
	    Socket sock = ep.newSocket();
	    conn = new TCPConnection(this, sock);

	    try {
		DataOutputStream out =
		    new DataOutputStream(conn.getOutputStream());
		writeTransportHeader(out);

		// choose protocol (single op if not reusable socket)
		if (!conn.isReusable()) {
		    out.writeByte(TransportConstants.SingleOpProtocol);
		} else {
		    out.writeByte(TransportConstants.StreamProtocol);
		    out.flush();

		    /*
		     * Set socket read timeout to configured value for JRMP
		     * connection handshake; this also serves to guard against
		     * non-JRMP servers that do not respond (see 4322806).
		     */
		    int originalSoTimeout = 0;
		    try {
			originalSoTimeout = sock.getSoTimeout();
			sock.setSoTimeout(handshakeTimeout);
		    } catch (Exception e) {
			// if we fail to set this, ignore and proceed anyway
		    }

		    DataInputStream in =
			new DataInputStream(conn.getInputStream());
		    byte ack = in.readByte();
		    if (ack != TransportConstants.ProtocolAck) {
			throw new ConnectIOException(
			    ack == TransportConstants.ProtocolNack ?
			    "JRMP StreamProtocol not supported by server" :
			    "non-JRMP server at remote endpoint");
		    }

		    String suggestedHost = in.readUTF();
		    int    suggestedPort = in.readInt();
		    if (TCPTransport.tcpLog.isLoggable(Log.VERBOSE)) {
			TCPTransport.tcpLog.log(Log.VERBOSE,
		            "server suggested " + suggestedHost + ":" +
			    suggestedPort);
		    }

		    // set local host name, if unknown
		    TCPEndpoint.setLocalHost(suggestedHost);
		    // do NOT set the default port, because we don't
		    // know if we can't listen YET...

		    // write out default endpoint to match protocol
		    // (but it serves no purpose)
		    TCPEndpoint localEp = TCPEndpoint.getLocalEndpoint(0,
			ep.getClientSocketFactory(),
                        ep.getServerSocketFactory());
		    out.writeUTF(localEp.getHost());
		    out.writeInt(localEp.getPort());
		    if (TCPTransport.tcpLog.isLoggable(Log.VERBOSE)) {
			TCPTransport.tcpLog.log(Log.VERBOSE, "using " +
		            localEp.getHost() + ":" + localEp.getPort());
		    }

		    /*
		     * After JRMP handshake, set socket read timeout to value
		     * configured for the rest of the lifetime of the
		     * connection.  NOTE: this timeout, if configured to a
		     * finite duration, places an upper bound on the time
		     * that a remote method call is permitted to execute.
		     */
		    try {
			/*
			 * If socket factory had set a non-zero timeout on its
			 * own, then restore it instead of using the property-
			 * configured value.
			 */
			sock.setSoTimeout((originalSoTimeout != 0 ?
					   originalSoTimeout :
					   responseTimeout));
		    } catch (Exception e) {
			// if we fail to set this, ignore and proceed anyway
		    }

		    out.flush();
		}
	    } catch (IOException e) {
		if (e instanceof RemoteException)
		    throw (RemoteException) e;
		else
		    throw new ConnectIOException(
			"error during JRMP connection establishment", e);
	    }
	} else {
	    try {
		conn = multiplexer.openConnection();
	    } catch (IOException e) {
		synchronized (this) {
		    usingMultiplexer = false;
		    multiplexer = null;
		}
		throw new ConnectIOException(
		    "error opening virtual connection " +
		    "over multiplexed connection", e);
	    }
	}
	return conn;
    }

    /**
     * Free the connection generated by this channel.
     * @param conn The connection
     * @param reuse If true, the connection is in a state in which it
     *        can be reused for another method call.
     */
    public void free(Connection conn, boolean reuse) {
	if (conn == null) return;

	if (reuse && conn.isReusable()) {
	    long lastuse = System.currentTimeMillis();

	    TCPTransport.tcpLog.log(Log.BRIEF, "reuse connection");

	    /*
	     * Cache connection; if reaper for expired connection
	     * doesn't exist, then create one.
	     */
	    synchronized (freeList) {
		freeList.add(conn);
		if (reaper == null) {
		    TCPTransport.tcpLog.log(Log.BRIEF, "create reaper");

		    reaper = new Reaper();
		    Thread t = (Thread)
			java.security.AccessController.doPrivileged(
			    new NewThreadAction(reaper,
						"ConnectionExpiration-" +
						ep.toString(), true));
		    t.start();
		}
	    }
		    
	    ((TCPConnection) conn).setLastUseTime(lastuse);
	    ((TCPConnection) conn).setExpiration(lastuse + idleTimeout);
	} else { 
	    TCPTransport.tcpLog.log(Log.BRIEF, "close connection");

	    try {
		conn.close();
	    } catch (IOException ignored) {
	    }
	}
    }

    /**
     * Send transport header over stream.
     */
    private void writeTransportHeader(DataOutputStream out)
	throws RemoteException
    {
	try {
	    // write out transport header
	    DataOutputStream dataOut =
		new DataOutputStream(out);
	    dataOut.writeInt(TransportConstants.Magic);
	    dataOut.writeShort(TransportConstants.Version);
	} catch (IOException e) {
	    throw new ConnectIOException(
		"error writing JRMP transport header", e);
	}
    }

    /**
     * Use given connection multiplexer object to obtain new connections
     * through this channel.
     */
    synchronized void useMultiplexer(ConnectionMultiplexer newMultiplexer) {
	// for now, always just use the last one given
	multiplexer = newMultiplexer;

	usingMultiplexer = true;
    }

    /**
     * Accept a connection provided over a multiplexed channel.
     */
    void acceptMultiplexConnection(Connection conn) {
	if (acceptor == null) {
	    acceptor = new ConnectionAcceptor(tr);
	    acceptor.startNewAcceptor();
	}
	acceptor.accept(conn);
    }

    /**
     * Closes all the connections in the cache, whether timed out or not.
     */
    public void shedCache() {
	// Build a list of connections, to avoid holding the freeList
	// lock during (potentially long-running) close() calls.
	Object conn[];
	synchronized (freeList) {
	    conn = freeList.toArray();
	    freeList.clear();
	}

	// Close all the connections that were free
	for (int i = conn.length; --i >= 0; ) {
	    Connection c = (Connection)conn[i];
	    conn[i] = null; // help gc
	    try {
		c.close();
	    } catch (java.io.IOException e) {
		// eat exception
	    }
	}
    }

    private boolean freeCachedConnections() {
	/*
	 * Remove each connection whose time out has expired.
	 */
	synchronized (freeList) {
	    int size = freeList.size();
	    
	    if (size > 0) {
		long time = System.currentTimeMillis();
		ListIterator iter = freeList.listIterator(size);

		while (iter.hasPrevious()) {
		    TCPConnection conn = (TCPConnection)iter.previous();
		    if (conn.expired(time)) {
			TCPTransport.tcpLog.log(Log.VERBOSE,
			    "connection timeout expired");

			try {
			    conn.close();
			} catch (java.io.IOException e) {
			    // eat exception
			}
			iter.remove();
		    }
		}
	    }
	    
	    if (freeList.isEmpty()) {
		reaper = null;
		return false;
	    } else {
		return true;
	    }
	}
    }

    /**
     * Reaps expired cached connections; thread exits when freelist
     * is empty.
     */
    private class Reaper implements Runnable {
	public void run()
	{
	    do {
		try {
		    Thread.sleep(idleTimeout);
		} catch (InterruptedException e) {
		}
		TCPTransport.tcpLog.log(Log.VERBOSE, "wake up");

	    } while (freeCachedConnections());
	    TCPTransport.tcpLog.log(Log.VERBOSE, "exit");
	}
    }
}

/**
 * ConnectionAcceptor manages accepting new connections and giving them
 * to TCPTransport's message handler on new threads.
 *
 * Since this object only needs to know which transport to give new
 * connections to, it doesn't need to be per-channel as currently
 * implemented.
 */
class ConnectionAcceptor implements Runnable {

    /** transport that will handle message on accepted connections */
    private TCPTransport transport;

    /** queue of connections to be accepted */
    private Vector queue = new Vector(4);

    /** thread ID counter */
    private static int threadNum = 0;

    /**
     * Create a new ConnectionAcceptor that will give connections
     * to the specified transport on a new thread.
     */
    public ConnectionAcceptor(TCPTransport transport) {
	this.transport = transport;
    }

    /**
     * Start a new thread to accept connections.
     */
    public void startNewAcceptor() {
	Thread t = (Thread) java.security.AccessController.doPrivileged(
	    new NewThreadAction(ConnectionAcceptor.this, 
				"Multiplex Accept-" + ++ threadNum,
				true));
	t.start();
    }

    /**
     * Add connection to queue of connections to be accepted.
     */
    public void accept(Connection conn) {
	synchronized (queue) {
	    queue.addElement(conn);
	    queue.notify();
	}
    }

    /**
     * Give transport next accepted conection, when available.
     */
    public void run() {
	Connection conn;

	synchronized (queue) {
	    while (queue.size() == 0) {
		try {
		    queue.wait();
		} catch (InterruptedException e) {
		}
	    }
	    startNewAcceptor();
	    conn = (Connection) queue.elementAt(0);
	    queue.removeElementAt(0);
	}

	transport.handleMessages(conn, true);
    }
}
