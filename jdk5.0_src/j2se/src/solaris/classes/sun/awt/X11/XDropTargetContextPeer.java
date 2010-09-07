/*
 * @(#)XDropTargetContextPeer.java	1.3 04/01/08
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.X11;

import java.awt.Component;

import java.io.IOException;

import java.util.Iterator;

import sun.awt.AppContext;
import sun.awt.SunToolkit;

import sun.awt.dnd.SunDropTargetContextPeer;
import sun.awt.dnd.SunDropTargetEvent;

import sun.misc.Unsafe;

/**
 * The XDropTargetContextPeer is the class responsible for handling
 * the interaction between the XDnD/Motif DnD subsystem and Java drop targets.
 *
 * @since 1.5
 */
final class XDropTargetContextPeer extends SunDropTargetContextPeer {
    private static final Unsafe unsafe = XlibWrapper.unsafe;

    /*
     * A key to store a peer instance for an AppContext.
     */
    private static final Object DTCP_KEY = "DropTargetContextPeer"; 

    private XDropTargetContextPeer() {}

    static XDropTargetContextPeer getPeer(AppContext appContext) {
        synchronized (_globalLock) {
            XDropTargetContextPeer peer = 
                (XDropTargetContextPeer)appContext.get(DTCP_KEY);
            if (peer == null) {
                peer = new XDropTargetContextPeer();
                appContext.put(DTCP_KEY, peer);
            }
            
            return peer;
        }
    }

    static XDropTargetProtocolListener getXDropTargetProtocolListener() {
        return XDropTargetProtocolListenerImpl.getInstance();
    }

    /*
     * @param returnValue the drop action selected by the Java drop target.
     */
    protected void eventProcessed(SunDropTargetEvent e, int returnValue, 
                                  boolean dispatcherDone) {
        /* The native context is the pointer to the XClientMessageEvent
           structure. */
        long ctxt = getNativeDragContext();
        /* If the event was not consumed, send a response to the source. */
        if (ctxt != 0 && !e.isConsumed()) {
            try {
                Iterator dropTargetProtocols = 
                    XDragAndDropProtocols.getDropTargetProtocols();
        
                while (dropTargetProtocols.hasNext()) {
                    XDropTargetProtocol dropTargetProtocol = 
                        (XDropTargetProtocol)dropTargetProtocols.next();
                    if (dropTargetProtocol.sendResponse(ctxt, e.getID(),
                                                        returnValue)) { 
                        break;
                    }
                }
            } finally {
                if (dispatcherDone) {
                    unsafe.freeMemory(ctxt);
                }
            }
        }
    }

    protected void doDropDone(boolean success, int dropAction, 
                              boolean isLocal) {
        /* The native context is the pointer to the XClientMessageEvent
           structure. */
        long ctxt = getNativeDragContext();

        if (ctxt != 0) {
            try {
                Iterator dropTargetProtocols = 
                    XDragAndDropProtocols.getDropTargetProtocols();
        
                while (dropTargetProtocols.hasNext()) {
                    XDropTargetProtocol dropTargetProtocol = 
                        (XDropTargetProtocol)dropTargetProtocols.next();
                    if (dropTargetProtocol.sendDropDone(ctxt, success,
                                                        dropAction)) { 
                        break;
                    }
                }
            } finally {
                unsafe.freeMemory(ctxt);
            }
        }
    }

    protected Object getNativeData(long format) 
      throws IOException {
        /* The native context is the pointer to the XClientMessageEvent
           structure. */
        long ctxt = getNativeDragContext();

        if (ctxt != 0) {
            Iterator dropTargetProtocols = 
                XDragAndDropProtocols.getDropTargetProtocols();
        
            while (dropTargetProtocols.hasNext()) {
                XDropTargetProtocol dropTargetProtocol = 
                    (XDropTargetProtocol)dropTargetProtocols.next();
                // getData throws IAE if ctxt is not for this protocol.
                try {
                    return dropTargetProtocol.getData(ctxt, format);
                } catch (IllegalArgumentException iae) {
                }
            }
        }
        
        return null;
    }

    private void cleanup() {
    }

    static final class XDropTargetProtocolListenerImpl 
        implements XDropTargetProtocolListener {

        private final static XDropTargetProtocolListener theInstance = 
            new XDropTargetProtocolListenerImpl();

        private XDropTargetProtocolListenerImpl() {}

        static XDropTargetProtocolListener getInstance() {
            return theInstance;
        }

        public void handleDropTargetNotification(XWindow xwindow, int x, int y, 
                                                 int dropAction, int actions, 
                                                 long[] formats, long nativeCtxt, 
                                                 int eventID) { 
            Object target = xwindow.getTarget();

            // The Every component is associated with some AppContext.
            assert target instanceof Component;

            Component component = (Component)target;

            AppContext appContext = SunToolkit.targetToAppContext(target);

            // Every component is associated with some AppContext.
            assert appContext != null;

            XDropTargetContextPeer peer = XDropTargetContextPeer.getPeer(appContext);

            // Make a copy of the passed XClientMessageEvent structure, since
            // the original structure can be freed before this
            // SunDropTargetEvent is dispatched.
            if (nativeCtxt != 0) {
                int size = new XClientMessageEvent(nativeCtxt).getSize();
                
                long copy = unsafe.allocateMemory(size);
                
                unsafe.copyMemory(nativeCtxt, copy, size);
                
                nativeCtxt = copy;
            }

            peer.postDropTargetEvent(component, x, y, dropAction, actions, formats,
                                     nativeCtxt, eventID,
                                     !SunDropTargetContextPeer.DISPATCH_SYNC);
        }
    }
}

