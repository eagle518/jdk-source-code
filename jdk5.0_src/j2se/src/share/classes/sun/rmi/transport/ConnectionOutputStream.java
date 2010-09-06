/*
 * @(#)ConnectionOutputStream.java	1.15 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.rmi.transport;

import java.io.DataOutput;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import java.rmi.server.UID;

import sun.rmi.server.MarshalOutputStream;

/**
 * Special stream to keep track of refs being marshaled as return
 * results to determine whether a special ack needs to be sent
 * to the distributed collector.
 *
 * @author Ann Wollrath
 */
class ConnectionOutputStream extends MarshalOutputStream {

    /** connection associated with ConnectionOutputStream */
    private final Connection conn;
    /** indicates whether output stream is used to marshal results */
    private final boolean resultStream;
    /** identifier for gc ack*/
    private final UID ackID = new UID();

    /** to store refs to returned remote object until DGC ack is received */
    private DGCAckHandler dgcAckHandler = null;

    /**
     * Constructs an marshal output stream using the underlying
     * stream associated with the connection, the parameter c.
     * @param c is the Connection object associated with the 
     * ConnectionOutputStream object being constructed
     * @param resultStream indicates whether this stream is used
     * to marshal return results
     */
    ConnectionOutputStream(Connection conn, boolean resultStream)
	throws IOException
    {	      
	super(conn.getOutputStream());
	this.conn = conn;
	this.resultStream = resultStream;
    }

    void writeID() throws IOException {
	ackID.write((DataOutput) this);
    }
    
    /**
     * Returns true if this output stream is used to marshal return
     * results; otherwise returns false.
     */
    boolean isResultStream() {
	return resultStream;
    }
    
    /**
     * Add reference to object list (only needs to be done if this
     * stream is an output stream for marshaling return results).
     * This hook is used by the distributed collector.
     */
    void saveObject(Object obj) {
	if (resultStream) {		// REMIND: redundant?
	    // should always be accessed from same thread
	    if (dgcAckHandler == null) {
		dgcAckHandler = new DGCAckHandler(ackID);
	    }
	    dgcAckHandler.add(obj);
	}
    }

    void done() {
	if (dgcAckHandler != null) {
	    dgcAckHandler.startTimer();
	}
    }
}
