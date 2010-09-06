/*
 * @(#)DGCAckHandler.java	1.16 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.rmi.transport;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.TimerTask;

import java.rmi.server.UID;

import sun.security.action.GetLongAction;

/**
 * A DGCAckHandler is used to hold strong references to a set of remote
 * objects, or remote references to remote objects, while they are in the
 * process of being returned from a remote call to this VM.  The purpose is
 * to prevent remote objects that are potentially unreachable in the local
 * VM from being locally garbage collected before the receiving client has
 * an opportunity to register its unmarshalled remote references via DGC.
 *
 * Each DGCAckHandler is created with a particular UID that has been sent
 * to the client VM to use to acknowledge receipt of the associated remote
 * objects.  The remote objects registered with a particular DGCAckHandler
 * are held with strong references until the static "received" method is
 * invoked with the associated UID, or after a timeout expires.
 *
 * @author	Ann Wollrath
 * @author	Peter Jones
 * @version	1.16, 03/12/19
 */
public class DGCAckHandler {

    /** timeout for holding references without receiving an acknowledgment */
    private static final long dgcAckTimeout =		// default 5 minutes
	((Long) java.security.AccessController.doPrivileged(
	    new GetLongAction("sun.rmi.dgc.ackTimeout",
			      300000))).longValue();

    /** table mapping ack ID to handler */
    private static final Map idTable =
	Collections.synchronizedMap(new HashMap(11));

    private final UID id;
    private List objList = new ArrayList();
				// null if timer task cancelled (see 4529396)
    private final TimerTask timerTask = new TimerTask() {
	public void run() {
	    received(id);
	}
    };

    /**
     * Creates a new DGCAckHandler for the specified ack ID.
     */
    DGCAckHandler(UID id) {
	// assert !idTable.contains(id);
	this.id = id;
	idTable.put(id, this);
    }

    /**
     * Adds another strong reference to this DGCAckHandler.  This method
     * should always be invoked from the same thread.
     */
    synchronized void add(Object obj) {
	if (objList != null) {
	    objList.add(obj);
	}
    }

    /**
     * Starts the timer for this DGCAckHandler; after the timeout has
     * expired, the references are dropped even if the acknowledgment has
     * not been received.
     */
    synchronized void startTimer() {
	if (objList != null) {
	    DGCImpl.timer.schedule(timerTask, dgcAckTimeout);
	}
    }

    /**
     * Informs handler that an acknowledgment is received. Forget
     * about list of remote objects.
     */
    public static void received(UID id) {
	DGCAckHandler h = (DGCAckHandler) idTable.remove(id);
	if (h != null) {
	    synchronized (h) {
		h.timerTask.cancel();
		h.objList = null;
	    }
	}
    }
}
