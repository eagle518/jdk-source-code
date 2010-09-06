/*
 * @(#)LiveRef.java	1.29 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.rmi.transport;

import java.lang.ref.*;
import java.io.*;
import java.util.*;
import java.rmi.*;
import java.rmi.server.*;
import sun.rmi.transport.tcp.TCPEndpoint;

public class LiveRef implements Cloneable {
    /** wire representation for the object*/
    private final Endpoint ep;
    private final ObjID id;

    /** cached connection service for the object */
    private transient Channel ch;

    /** flag to indicate whether this ref specifies a local server or
     * is a ref for a remote object (surrogate)
     */
    private final boolean isLocal;

    /**
     * Construct a "well-known" live reference to a remote object
     * @param isLocalServer If true, indicates this ref specifies a local
     * server in this address space; if false, the ref is for a remote
     * object (hence a surrogate or proxy) in another address space.
     */
    public LiveRef(ObjID objID, Endpoint endpoint, boolean isLocal) {
	ep = endpoint;
	id = objID;
	this.isLocal = isLocal;
    }

    /**
     * Construct a new live reference for a server object in the local
     * address space.
     */
    public LiveRef(int port) {
	this((new ObjID()), port);
    }

    /**
     * Construct a new live reference for a server object in the local
     * address space, to use sockets of the specified type.
     */
    public LiveRef(int port,
		   RMIClientSocketFactory csf,
		   RMIServerSocketFactory ssf)
    {
	this((new ObjID()), port, csf, ssf);
    }

    /**
     * Construct a new live reference for a "well-known" server object
     * in the local address space.
     */
    public LiveRef(ObjID objID, int port) {
	this(objID, TCPEndpoint.getLocalEndpoint(port), true);
    }

    /**
     * Construct a new live reference for a "well-known" server object
     * in the local address space, to use sockets of the specified type.
     */
    public LiveRef(ObjID objID, int port, RMIClientSocketFactory csf,
		   RMIServerSocketFactory ssf)
    {
	this(objID, TCPEndpoint.getLocalEndpoint(port, csf, ssf), true);
    }

    /**
     * Return a shallow copy of this ref.
     */
    public Object clone() {
	try {
	    LiveRef newRef = (LiveRef) super.clone();
	    return newRef;
	} catch (CloneNotSupportedException e) {
	    throw new InternalError(e.toString());
	}
    }

    /**
     * Return the port number associated with this ref.
     */
    public int getPort() {
	return ((TCPEndpoint)ep).getPort();
    }
    
    /**
     * Return the client socket factory associated with this ref.
     */
    public RMIClientSocketFactory getClientSocketFactory() {
	return ((TCPEndpoint)ep).getClientSocketFactory();
    }
    
    /**
     * Return the server socket factory associated with this ref.
     */
    public RMIServerSocketFactory getServerSocketFactory() {
	return ((TCPEndpoint)ep).getServerSocketFactory();
    }
    
    /**
     * Export the object to accept incoming calls.
     */
    public void exportObject(Target target) throws RemoteException {
	ep.exportObject(target);
    }
    
    public Channel getChannel() throws RemoteException {
	if (ch == null) {
	    ch = ep.getChannel();
	}
	return ch;
    }

    public ObjID getObjID() {
	return id;
    }

    Endpoint getEndpoint() {
	return ep;
    }

    public String toString() {
	String type;
	
	if (isLocal)
	    type = "local";
	else
	    type = "remote";
	return "[endpoint:" + ep + "(" + type + ")," +
	    "objID:" + id + "]";
    }

    public int hashCode() {
	return id.hashCode();
    }

    public boolean equals(Object obj) {
	if (obj != null && obj instanceof LiveRef) {
	    LiveRef ref = (LiveRef)obj;

	    return (ep.equals(ref.ep) && id.equals(ref.id) &&
		    isLocal == ref.isLocal);
	} else {
	    return false;
	}
    }
    
    public boolean remoteEquals(Object obj) {
	if (obj != null && obj instanceof LiveRef) {
	    LiveRef ref = (LiveRef) obj;

	    TCPEndpoint thisEp = ((TCPEndpoint) ep);
	    TCPEndpoint refEp = ((TCPEndpoint) ref.ep);
		
	    RMIClientSocketFactory thisClientFactory =
		thisEp.getClientSocketFactory();
	    RMIClientSocketFactory refClientFactory =
		refEp.getClientSocketFactory();

	    /**
	     * Fix for 4254103: LiveRef.remoteEquals should not fail
	     * if one of the objects in the comparison has a null
	     * server socket.  Comparison should only consider the
	     * following criteria:
	     *
	     * hosts, ports, client socket factories and object IDs.
	     */
	    if (thisEp.getPort() != refEp.getPort() ||
		!thisEp.getHost().equals(refEp.getHost()))
	    {
		return false;
	    }
	    if ((thisClientFactory == null) ^ (refClientFactory == null)) {
		return false;
	    }
	    if ((thisClientFactory != null) &&
		!((thisClientFactory.getClass() == refClientFactory.getClass()) &&
		  (thisClientFactory.equals(refClientFactory))))
	    {
		return false;
	    }
	    return (id.equals(ref.id));
	} else {
	    return false;
	}
    }
    
    public void write(ObjectOutput out, boolean useNewFormat)
	throws IOException
    {
	boolean isResultStream = false;
	if (out instanceof ConnectionOutputStream) {
	    ConnectionOutputStream stream = (ConnectionOutputStream) out;
	    isResultStream = stream.isResultStream();
	    /*
	     * If this LiveRef is being marshalled as part of a return value,
	     * then we have to worry about referential integrity being broken
	     * while the reference is in transit, in case there no longer
	     * remains a strong reference in this VM after the call has
	     * completed.  Therefore, we tell the stream to save a reference
	     * until an acknowledgment has been received from the caller; for
	     * "local" LiveRefs, save a reference to the impl directly,
	     * because the impl is not reachable from this LiveRef (see
	     * 4114579); otherwise, save a reference to the LiveRef, for the
	     * client-side DGC to watch over (see 4017232 for more details).
	     */
	    if (isResultStream) {
		if (isLocal) {
		    ObjectEndpoint oe =
			new ObjectEndpoint(id, ep.getInboundTransport());
		    Target target = ObjectTable.getTarget(oe);

		    if (target != null) {
			Remote impl = target.getImpl();
			if (impl != null) {
			    stream.saveObject(impl);
			}
		    }
		} else {
		    stream.saveObject(this);
		}
	    }
	}
	// All together now write out the endpoint, id, and flag

	// (need to choose whether or not to use old JDK1.1 endpoint format)
	if (useNewFormat) {
	    ((TCPEndpoint) ep).write(out);
	} else {
	    ((TCPEndpoint) ep).writeHostPortFormat(out);
	}
	id.write(out);
	out.writeBoolean(isResultStream);
    }
    
    public static LiveRef read(ObjectInput in, boolean useNewFormat)
	throws IOException, ClassNotFoundException
    {
	Endpoint ep;
	ObjID id;

	// Now read in the endpoint, id, and result flag
	// (need to choose whether or not to read old JDK1.1 endpoint format)
	if (useNewFormat) {
	    ep = TCPEndpoint.read(in);
	} else {
	    ep = TCPEndpoint.readHostPortFormat(in);
	}
	id = ObjID.read(in);
	boolean isResultStream = in.readBoolean();

	LiveRef ref = new LiveRef(id, ep, false);
	
	if (in instanceof ConnectionInputStream) {
	    ConnectionInputStream stream = (ConnectionInputStream)in;
	    // save ref to send "dirty" call after all args/returns
	    // have been unmarshaled.
	    stream.saveRef(ref);
	    if (isResultStream) {
		// set flag in stream indicating that remote objects were
		// unmarshaled.  A DGC ack should be sent by the transport.
		stream.setAckNeeded();
	    }
	} else {
	    DGCClient.registerRefs(ep, Arrays.asList(new LiveRef[] { ref }));
	}

	return ref;
    }
}
