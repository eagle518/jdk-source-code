/*
 * @(#)TCPTransport.java	1.55 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.rmi.transport.tcp;

import java.lang.ref.SoftReference;
import java.lang.reflect.InvocationTargetException;
import java.net.InetAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.io.*;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.WeakHashMap;
import java.util.LinkedList;
import java.rmi.RemoteException;
import java.rmi.server.ExportException;
import java.rmi.server.LogStream;
import java.rmi.server.ObjID;
import java.rmi.server.RMIFailureHandler;
import java.rmi.server.RMISocketFactory;
import java.rmi.server.RemoteCall;
import java.rmi.server.ServerNotActiveException;
import java.rmi.server.UID;
import java.security.AccessControlContext;
import java.security.AccessController;

import sun.rmi.runtime.Log;
import sun.rmi.runtime.NewThreadAction;
import sun.rmi.transport.Channel;
import sun.rmi.transport.Connection;
import sun.rmi.transport.DGCAckHandler;
import sun.rmi.transport.Endpoint;
import sun.rmi.transport.StreamRemoteCall;
import sun.rmi.transport.Target;
import sun.rmi.transport.Transport;
import sun.rmi.transport.TransportConstants;
import sun.rmi.transport.proxy.HttpReceiveSocket;

/**
 * TCPTransport is the socket-based implementation of the RMI Transport
 * abstraction.
 *
 * @author Ann Wollrath
 */
public class TCPTransport extends Transport implements Runnable {

    /** "tcp" package log level */
    static final int logLevel = LogStream.parseLevel(getLogLevel());

    private static String getLogLevel() {
	return (String) java.security.AccessController.doPrivileged(
	    new sun.security.action.GetPropertyAction(
		"sun.rmi.transport.tcp.logLevel"));
    }

    /* tcp package log */
    static final Log tcpLog =
	Log.getLog("sun.rmi.transport.tcp", "tcp", TCPTransport.logLevel);

    /** to tag threads */
    private static int threadNum = 0;

    /** client host for the current thread's connection */
    private static final ThreadLocal threadConnectionHandler =
	new ThreadLocal();
    
    /** endpoints for this transport */
    private final LinkedList epList;
    /** server socket for this transport */
    private ServerSocket server = null;
    /** table mapping endpoints to channels */
    private final Map channelTable = new HashMap(11);

    static final RMISocketFactory defaultSocketFactory =
	java.rmi.server.RMISocketFactory.getDefaultSocketFactory();

    /** number of milliseconds in accepted-connection timeout.
     * Warning: this should be greater than 15 seconds (the client-side
     * timeout), and defaults to 2 hours.
     * The maximum representable value is slightly more than 24 days
     * and 20 hours.
     */
    private final static int connectionReadTimeout = ((Integer)
	java.security.AccessController.doPrivileged(
	    new sun.security.action.GetIntegerAction(
		"sun.rmi.transport.tcp.readTimeout", 2 * 3600 * 1000)
	    )
	).intValue();

    /**
     * Constructs a TCPTransport.
     */
    TCPTransport(LinkedList epList)  {
	// assert ((epList.size() != null) && (epList.size() >= 1))
	this.epList = epList;
	if (tcpLog.isLoggable(Log.BRIEF)) {
	    tcpLog.log(Log.BRIEF, "Version = " +
		TransportConstants.Version + ", ep = " + getEndpoint());
	}
    }

    /**
     * Closes all cached connections in every channel subordinated to this
     * transport.  Currently, this only closes outgoing connections.
     */
    public void shedConnectionCaches()
    {
	TCPChannel[] ch;
	int i;

	// Build a list so that the shedCache() loop doesn't
	// need to hold a lock on channelTable.
	synchronized (channelTable) {
	    ch = new TCPChannel[channelTable.size()];
	    Iterator channelList = channelTable.values().iterator();
	    for (i = 0; channelList.hasNext(); i++) {
		ch[i] = (TCPChannel) channelList.next();
	    }
	}
	// now ch[0..i-1] are valid TCPChannels, or i==0.

	// Shed excess connections in every channel
	while (--i >= 0) {
	    ch[i].shedCache();
	}
    }

    /**
     * Returns a <I>Channel</I> that generates connections to the
     * endpoint <I>ep</I>. A Channel is an object that creates and
     * manages connections of a particular type to some particular
     * address space.
     * @param ep the endpoint to which connections will be generated.
     * @return the channel or null if the transport cannot
     * generate connections to this endpoint
     */
    public Channel getChannel(Endpoint ep) {
	Channel ch = null;

	if (ep instanceof TCPEndpoint) {
	    synchronized (channelTable) {
		ch = (Channel)channelTable.get(ep);
		if (ch == null) {
		    ch = new TCPChannel(this, (TCPEndpoint)ep);
		    channelTable.put(ep, ch);
		}
	    }
	}
	return ch;
    }

    /**
     * Removes the <I>Channel</I> that generates connections to the
     * endpoint <I>ep</I>.
     */
    public void free(Endpoint ep) {

	if (ep instanceof TCPEndpoint) {
	    synchronized (channelTable) {
		TCPChannel channel = (TCPChannel) channelTable.remove(ep);
		if (channel != null) {
		    channel.shedCache();
		}
	    }
	}
    }

    /**
     * Export the object so that it can accept incoming calls.
     */
    public void exportObject(Target target) throws RemoteException {

	listen();		// try to listen on TCP port

	/*
	 * Set the transport for which remote calls to this target
	 * are allowed (see bugid 4177266).
	 */
	target.setExportedTransport(this);

	/*
	 * Finally, allocate an ObjID only if export succeeded
	 * (see bugid 4186823).
	 */
	super.exportObject(target);
    }

    /**
     * Verify that the current access control context has permission to
     * accept the connection being dispatched by the current thread.
     */
    protected void checkAcceptPermission(AccessControlContext acc) {
	SecurityManager sm = System.getSecurityManager();
	if (sm == null) {
	    return;
	}
	ConnectionHandler h = 
	    (ConnectionHandler) threadConnectionHandler.get();
	if (h == null) {
	    throw new Error(
		"checkAcceptPermission not in ConnectionHandler thread");
	}
	h.checkAcceptPermission(sm, acc);
    }

    private TCPEndpoint getEndpoint() {
	synchronized (epList) {
	    return (TCPEndpoint) epList.getLast();
	}
    }
	
    /**
     * Listen on transport's endpoint.
     */
    private synchronized void listen() throws RemoteException {
	TCPEndpoint ep = getEndpoint();
	int port = ep.getPort();

	if (server == null) {
	    if (tcpLog.isLoggable(Log.BRIEF)) {
		tcpLog.log(Log.BRIEF,
		    "(port " + port + ") create server socket");
	    }

	    try {
		server = ep.newServerSocket();
		/*
		 * Don't retry ServerSocket if creation fails since
		 * "port in use" will cause export to hang if an
		 * RMIFailureHandler is not installed.
		 */
		Thread t = (Thread)
		    java.security.AccessController.doPrivileged(
			new NewThreadAction(TCPTransport.this,
					    "TCP Accept-" + port, true));
		t.start();
	    } catch (java.net.BindException e) {
		throw new ExportException("Port already in use: " + port, e);
	    } catch (IOException e) {
		throw new ExportException("Listen failed on port: " + port, e);
	    }

	} else {
	    // otherwise verify security access to existing server socket
	    SecurityManager sm = System.getSecurityManager();
	    if (sm != null) {
		sm.checkListen(port);
	    }
	}
    }

    /**
     * Private method to decide whether to continue accepting connections
     * in the wake of an accept failure.
     * @return if true, accepting should continue
     */
    synchronized private boolean continueAfterAcceptFailure(Throwable t) {
	boolean f;
	RMIFailureHandler fh = RMISocketFactory.getFailureHandler();
	if (fh != null) {
	    if (t instanceof Exception)
		f = fh.failure((Exception)t);
	    else
		f = fh.failure(new InvocationTargetException(t));
	} else {
	    /* Default behavior if no failure handler is installed:
	     * if we get a burst of NFAIL failures in NMSEC milliseconds,
	     * then wait for ten seconds.  This is to ensure that
	     * individual failures don't cause hiccups, but sustained
	     * failures don't hog the CPU in futile accept-fail-retry
	     * looping.
	     */
	    final int NFAIL = 10;
	    final int NMSEC = 5000;
	    long now = System.currentTimeMillis();
	    if (acceptFailureTime == 0L ||
		(now - acceptFailureTime) > NMSEC)
	    {
		// failure time is very old, or this is first failure
		// start a new window
		acceptFailureTime = now;
		acceptFailureCount = 0;
	    } else {
		// failure window was started recently
		acceptFailureCount++;
		if (acceptFailureCount >= NFAIL) {
		    // NFAIL failures happened in the last NMSEC ms
		    // Sleep for 10 seconds to give other parts of the
		    // system a chance to clean up
		    try {
			Thread.sleep(10000);
		    } catch (InterruptedException ign) {
		    }
		    // no need to reset counter/timer
		}
	    }
	    f = true;
	}

	return f;
    }
    private transient long acceptFailureTime = 0L;
    private transient int acceptFailureCount;

    /**
     * Accepts connections to server and spawns a thread for each accepted
     * connection that services server-side input from client.
     */
    public void run() {

	if (tcpLog.isLoggable(Log.BRIEF)) {
	    tcpLog.log(Log.BRIEF, "listening on port " +
		       getEndpoint().getPort());
	}

	// accept connection
	while (true) {
	    ServerSocket myServer = server;
	    if (myServer == null)
		// the server socket will be null if another thread
		// attempted to retry creating the server socket
		// and failed.
		return;

	    Throwable acceptFailure = null;
	    final Socket socket;
	    
	    try {
		socket = myServer.accept();
		
		/*
		 * Find client host name (or "0.0.0.0" if unknown)
		 */
		InetAddress clientAddr = socket.getInetAddress();
		String clientHost = (clientAddr != null
				     ? clientAddr.getHostAddress()
				     : "0.0.0.0");
		
		/*
		 * Spawn non-system thread to handle the connection
		 */
		Thread t = (Thread)
		    java.security.AccessController.doPrivileged (
		        new NewThreadAction(new ConnectionHandler(socket,
								  clientHost),
					    "TCP Connection(" + ++ threadNum +
					    ")-" + clientHost,
					    true, true));
		t.start();
		
	    } catch (IOException e) {
		acceptFailure = e;
	    } catch (RuntimeException e) {
		acceptFailure = e;
	    } catch (Error e) {
		acceptFailure = e;
	    } finally {
		if (acceptFailure != null) {
		    /*
		     * Also, in case we're running out of file descriptors, we
		     * should release resources held in caches.  The specific
		     * case of SecurityException is excluded to minimize the
		     * possible effects of a flood of failed checkAccepts() in
		     * a denial-of-service attack.
		     */
		    if (!(acceptFailure instanceof SecurityException)) {
			try {
			    TCPEndpoint.shedConnectionCaches();
			} catch (OutOfMemoryError mem) {
			    // don't quit if out of memory
			} catch (Exception ex) {
			    // don't quit if shed fails for some reason
			    // other than Error-scale catastrophe
			}
		    }

		    /*
		     * There are plenty of transient Errors and
		     * Exceptions which shouldn't cause the death of this
		     * thread.
		     * In particular, NoClassDefFoundError can be a symptom
		     * of an out-of-file-descriptors or out-of-memory
		     * condition which prevented the SocketException or
		     * OutOfMemoryError classes from being loaded.
		     */
		    if ((acceptFailure instanceof IOException) ||
			(acceptFailure instanceof OutOfMemoryError) ||
			(acceptFailure instanceof NoClassDefFoundError) ||
			(acceptFailure instanceof java.util.MissingResourceException) ||
			(acceptFailure instanceof SecurityException))
		    {
			if (continueAfterAcceptFailure(acceptFailure) == false)
			    return;
			    // continue to next iteration
		    } else {
		/*
		 * Otherwise, the error is probably too severe to handle here,
		 * so throw it up.  The preceding logic requires that the
		 * "acceptFailure" must be a RuntimeException or an Error,
		 * and hence unchecked, but the compiler can't tell that,
		 * so we have to convince it of that explicitly...
		 */
			if (acceptFailure instanceof RuntimeException)
			    throw (RuntimeException) acceptFailure;
			else // acceptFailure must be instance of Error...
			    throw (Error) acceptFailure;
		    }
		}
	    }

	    /*
	     * Loop to accept next connection.
	     */
	}
    }

    /** close socket and eat exception */
    private static void closeSocket(Socket sock) {
	try {
	    sock.close();
	} catch (IOException ex) {
	    // eat exception
	}
    }
    
    /**
     * handleMessages decodes transport operations and handles messages
     * appropriately.  If an exception occurs during message handling,
     * the socket is closed.
     */
    void handleMessages(Connection conn, boolean persistent) {
	int port = getEndpoint().getPort();
	
	try {
	    DataInputStream in = new DataInputStream(conn.getInputStream());
	    do {
		int op = in.read();	// transport op
		if (op == -1) {
		    if (tcpLog.isLoggable(Log.BRIEF)) {
			tcpLog.log(Log.BRIEF, "(port " +
                            port + ") connection closed");
		    }
		    break;
		}

		if (tcpLog.isLoggable(Log.BRIEF)) {
		    tcpLog.log(Log.BRIEF, "(port " + port +
                        ") op = " + op);
		}
		    
		switch (op) {
		case TransportConstants.Call:
		    // service incoming RMI call
		    RemoteCall call = new StreamRemoteCall(conn);
		    if (serviceCall(call) == false)
			return;
		    break;

		case TransportConstants.Ping:
		    // send ack for ping
		    DataOutputStream out =
			new DataOutputStream(conn.getOutputStream());
		    out.writeByte(TransportConstants.PingAck);
		    conn.releaseOutputStream();
		    break;

		case TransportConstants.DGCAck:
		    DGCAckHandler.received(UID.read((DataInput) in));
		    break;
		    
		default:
		    throw new IOException("unknown transport op " + op);
		}
	    } while (persistent);
	    
	} catch (IOException e) {
	    // exception during processing causes connection to close (below)
	    if (tcpLog.isLoggable(Log.BRIEF)) {
		tcpLog.log(Log.BRIEF, "(port " + port +
                    ") exception: ", e);
	    }
	} finally {
	    try {
		conn.close();
	    } catch (IOException ex) {
		// eat exception
	    }
	}
    }

    /**
     * Returns the client host for the current thread's connection.  Throws
     * ServerNotActiveException if no connection is active for this thread.
     */
    public static String getClientHost() throws ServerNotActiveException {
	ConnectionHandler h =
	    (ConnectionHandler) threadConnectionHandler.get();
	if (h != null) {
	    return h.getClientHost();
	} else {
	    throw new ServerNotActiveException("not in a remote call");
	}
    }
	
    /**
     * Services messages on accepted connection
     */
    private class ConnectionHandler implements Runnable {

	/** int value of "POST" in ASCII (Java's specified data formats
	 *  make this once-reviled tactic again socially acceptable) */
	private final static int POST = 0x504f5354;
	
	/** most recently accept-authorized AccessControlContext */
	private AccessControlContext okContext;
	/** cache of accept-authorized AccessControlContexts */
	private WeakHashMap authCache;
	/** security manager which authorized contexts in authCache */
	private SecurityManager cacheSecurityManager = null;

	private Socket socket;
	private String remoteHost;

	ConnectionHandler(Socket socket, String remoteHost) {
	    this.socket = socket;
	    this.remoteHost = remoteHost;
	}

	String getClientHost() {
	    return remoteHost;
	}

	/**
	 * Verify that the given AccessControlContext has permission to
	 * accept this connection.
	 */
	void checkAcceptPermission(SecurityManager sm,
				   AccessControlContext acc) 
	{
	    /*
	     * Note: no need to synchronize on cache-related fields, since this
	     * method only gets called from the ConnectionHandler's thread.
	     */
	    if (sm != cacheSecurityManager) {
		okContext = null;
		authCache = new WeakHashMap();
		cacheSecurityManager = sm;
	    }
	    if (acc.equals(okContext) || authCache.containsKey(acc)) {
		return;
	    }
	    InetAddress addr = socket.getInetAddress();
	    String host = (addr != null) ? addr.getHostAddress() : "*";

	    sm.checkAccept(host, socket.getPort());

	    authCache.put(acc, new SoftReference(acc));
	    okContext = acc;
	}

	public void run() {
	    TCPEndpoint endpoint = getEndpoint();
	    int port = endpoint.getPort();
	    
	    threadConnectionHandler.set(this);
	    
	    // set socket to disable Nagle's algorithm (always send
	    // immediately)
	    // TBD: should this be left up to socket factory instead?
	    try {
		socket.setTcpNoDelay(true);
	    } catch (Exception e) {
		// if we fail to set this, ignore and proceed anyway
	    }
	    // set socket to timeout after excessive idle time
	    try {
		if (connectionReadTimeout > 0)
		    socket.setSoTimeout(connectionReadTimeout);
	    } catch (Exception e) {
		// too bad, continue anyway
	    }

	    try {
		InputStream sockIn = socket.getInputStream();
		InputStream bufIn = sockIn.markSupported()
			? sockIn
			: new BufferedInputStream(sockIn);

		// Read magic (or HTTP wrapper)
		bufIn.mark(4);
		DataInputStream in = new DataInputStream(bufIn);
		int magic = in.readInt();

		if (magic == POST) {
		    tcpLog.log(Log.BRIEF, "decoding HTTP-wrapped call");

		    // It's really a HTTP-wrapped request.  Repackage
		    // the socket in a HttpReceiveSocket, reinitialize
		    // sockIn and in, and reread magic.
		    bufIn.reset();	// unread "POST"

		    try {
			socket = new HttpReceiveSocket(socket, bufIn, null);
			remoteHost = "0.0.0.0";
			sockIn = socket.getInputStream();
			bufIn = new BufferedInputStream(sockIn);
			in = new DataInputStream(bufIn);
			magic = in.readInt();

		    } catch (IOException e) {
			throw new RemoteException("Error HTTP-unwrapping call",
						  e);
		    }
		}
		// bufIn's mark will invalidate itself when it overflows
		// so it doesn't have to be turned off

		// read and verify transport header
		short version = in.readShort();
		if (magic != TransportConstants.Magic ||
		    version != TransportConstants.Version) {
		    // protocol mismatch detected...
		    // just close socket: this would recurse if we marshal an
		    // exception to the client and the protocol at other end
		    // doesn't match.
		    closeSocket(socket);
		    return;
		}

		OutputStream sockOut = socket.getOutputStream();
		BufferedOutputStream bufOut = 
		    new BufferedOutputStream(sockOut);  
		DataOutputStream out = new DataOutputStream(bufOut);

		int remotePort = socket.getPort();

		if (tcpLog.isLoggable(Log.BRIEF)) {
		    tcpLog.log(Log.BRIEF, "accepted socket from [" +
				     remoteHost + ":" + remotePort + "]");
		}

		TCPEndpoint ep;
		TCPChannel ch;
		TCPConnection conn;

		// send ack (or nack) for protocol
		byte protocol = in.readByte();
		switch (protocol) {
		case TransportConstants.SingleOpProtocol:
		    // no ack for protocol

		    // create dummy channel for receiving messages
		    ep = new TCPEndpoint(remoteHost, socket.getLocalPort(),
					 endpoint.getClientSocketFactory(),
					 endpoint.getServerSocketFactory());
		    ch = new TCPChannel(TCPTransport.this, ep);
		    conn = new TCPConnection(ch, socket, bufIn, bufOut);

		    // read input messages
		    handleMessages(conn, false);
		    break;

		case TransportConstants.StreamProtocol:
		    // send ack
		    out.writeByte(TransportConstants.ProtocolAck);

		    // suggest endpoint (in case client doesn't know host name)
		    if (tcpLog.isLoggable(Log.VERBOSE)) {
			tcpLog.log(Log.VERBOSE, "(port " + port +
			    ") " + "suggesting " + remoteHost + ":" +
			    remotePort);
		    }

		    out.writeUTF(remoteHost);
		    out.writeInt(remotePort);
		    out.flush();

		    // read and discard (possibly bogus) endpoint
		    // REMIND: would be faster to read 2 bytes then skip N+4
		    String clientHost = in.readUTF();
		    int    clientPort = in.readInt();
		    if (tcpLog.isLoggable(Log.VERBOSE)) {
			tcpLog.log(Log.VERBOSE, "(port " + port +
			    ") client using " + clientHost + ":" + clientPort);
		    }

		    // create dummy channel for receiving messages
		    // (why not use clientHost and clientPort?)
		    ep = new TCPEndpoint(remoteHost, socket.getLocalPort(),
					 endpoint.getClientSocketFactory(),
					 endpoint.getServerSocketFactory());
		    ch = new TCPChannel(TCPTransport.this, ep);
		    conn = new TCPConnection(ch, socket, bufIn, bufOut);

		    // read input messages
		    handleMessages(conn, true);
		    break;

		case TransportConstants.MultiplexProtocol:
		    if (tcpLog.isLoggable(Log.VERBOSE)) {
			tcpLog.log(Log.VERBOSE, "(port " + port +
			    ") accepting multiplex protocol");
		    }

		    // send ack
		    out.writeByte(TransportConstants.ProtocolAck);

		    // suggest endpoint (in case client doesn't already have one)
		    if (tcpLog.isLoggable(Log.VERBOSE)) {
			tcpLog.log(Log.VERBOSE, "(port " + port +
			    ") suggesting " + remoteHost + ":" + remotePort);
		    }

		    out.writeUTF(remoteHost);
		    out.writeInt(remotePort);
		    out.flush();

		    // read endpoint client has decided to use
		    ep = new TCPEndpoint(in.readUTF(), in.readInt(),
					 endpoint.getClientSocketFactory(),
					 endpoint.getServerSocketFactory());
		    if (tcpLog.isLoggable(Log.VERBOSE)) {
			tcpLog.log(Log.VERBOSE, "(port " +
			    port + ") client using " +
			    ep.getHost() + ":" + ep.getPort());
		    }

		    ConnectionMultiplexer multiplexer;
		    synchronized (channelTable) {
			// create or find channel for this endpoint
			ch = (TCPChannel) getChannel(ep);
			multiplexer =
			    new ConnectionMultiplexer(ch, bufIn, sockOut,
						      false);
			ch.useMultiplexer(multiplexer);
		    }
		    multiplexer.run();
		    break;

		default:
		    // protocol not understood, send nack and close socket
		    out.writeByte(TransportConstants.ProtocolNack);
		    out.flush();
		    break;
		}

	    } catch (IOException e) {
		// socket in unknown state: destroy socket
		tcpLog.log(Log.BRIEF, "terminated with exception:", e);
	    } finally {
		closeSocket(socket);
	    }
	}
    }
}
