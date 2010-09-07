/*
 * @(#)X11DropTargetContextPeer.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.motif;

import java.awt.Component;

import sun.awt.AppContext;
import sun.awt.SunToolkit;

import sun.awt.dnd.SunDropTargetContextPeer;
import sun.awt.dnd.SunDropTargetEvent;

/**
 * The X11DropTargetContextPeer class is the class responsible for handling
 * the interaction between the XDnD/Motif DnD subsystem and Java drop targets.
 *
 * @since 1.5
 */
final class X11DropTargetContextPeer extends SunDropTargetContextPeer {

    /*
     * A key to store a peer instance for an AppContext.
     */
    private static final Object DTCP_KEY = "DropTargetContextPeer"; 

    private X11DropTargetContextPeer() {}

    /*
     * Note: 
     * the method can be called on the toolkit thread while holding AWT_LOCK.
     */
    private static void postDropTargetEventToPeer(final Component component, 
                                                  final int x, final int y, 
                                                  final int dropAction,
                                                  final int actions, 
                                                  final long[] formats,
                                                  final long nativeCtxt,
                                                  final int eventID) {

        X11DropTargetContextPeer peer = null;
        AppContext appContext = SunToolkit.targetToAppContext(component);

        synchronized (_globalLock) {
            peer = (X11DropTargetContextPeer)appContext.get(DTCP_KEY);
            if (peer == null) {
                peer = new X11DropTargetContextPeer();
                appContext.put(DTCP_KEY, peer);
            }
        }

        peer.postDropTargetEvent(component, x, y, dropAction, actions, formats,
                                 nativeCtxt, eventID,
                                 !SunDropTargetContextPeer.DISPATCH_SYNC);
    }

    protected void eventProcessed(SunDropTargetEvent e, int returnValue, 
                                  boolean dispatcherDone) {
        /* If the event was not consumed, send a response to the source. */
        long ctxt = getNativeDragContext();
        if (ctxt != 0) {
            sendResponse(e.getID(), returnValue, ctxt, dispatcherDone,
                         e.isConsumed());
        }
    }

    protected void doDropDone(boolean success, int dropAction, 
                              boolean isLocal) {
	dropDone(getNativeDragContext(), success, dropAction);
    }

    protected Object getNativeData(long format) {
        return getData(getNativeDragContext(), format);
    }

    private native void sendResponse(int eventID, int returnValue, 
                                     long nativeCtxt, boolean dispatcherDone,
                                     boolean consumed);

    private native void dropDone(long nativeCtxt, boolean success, 
                                 int dropAction);

    private native Object getData(long nativeCtxt, long format);
}

