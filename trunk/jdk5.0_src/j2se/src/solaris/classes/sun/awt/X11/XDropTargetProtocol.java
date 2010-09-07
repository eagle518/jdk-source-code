/*
 * @(#)XDropTargetProtocol.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.X11;

import java.io.IOException;

import java.util.HashMap;

/**
 * An abstract class for drop protocols on X11 systems.
 * Contains protocol-independent drop target code.
 *
 * @since 1.5
 */
abstract class XDropTargetProtocol {
    private final XDropTargetProtocolListener listener;    

    public static final int EMBEDDER_ALREADY_REGISTERED = 0;

    public static final int UNKNOWN_MESSAGE = 0;
    public static final int ENTER_MESSAGE   = 1;
    public static final int MOTION_MESSAGE  = 2;
    public static final int LEAVE_MESSAGE   = 3;
    public static final int DROP_MESSAGE    = 4;

    protected XDropTargetProtocol(XDropTargetProtocolListener listener) {
        if (listener == null) {
            throw new NullPointerException("Null XDropTargetProtocolListener");
        }
        this.listener = listener;
    }

    protected final XDropTargetProtocolListener getProtocolListener() {
        return listener;
    }

    /* The caller must hold AWT_LOCK. */
    public abstract void registerDropTarget(long window);

    /* The caller must hold AWT_LOCK. */
    public abstract void unregisterDropTarget(long window);

    /* The caller must hold AWT_LOCK. */
    public abstract void registerEmbedderDropSite(long window);

    /* The caller must hold AWT_LOCK. */
    public abstract void unregisterEmbedderDropSite(long window);

    /* The caller must hold AWT_LOCK. */
    public abstract boolean isProtocolSupported(long window);

    public abstract int getMessageType(XClientMessageEvent xclient);

    /* The caller must hold AWT_LOCK. */
    public final boolean processClientMessage(XClientMessageEvent xclient) {
        int type = getMessageType(xclient);
        boolean processed = processClientMessageImpl(xclient);

        postProcessClientMessage(xclient, processed, type);

        return processed;
    }

    /* The caller must hold AWT_LOCK. */
    protected abstract boolean processClientMessageImpl(XClientMessageEvent xclient);

    /*
     * Forwards a drag notification to the embedding toplevel modifying the event 
     * to match the protocol version supported by the toplevel. 
     * The caller must hold AWT_LOCK.
     * Returns True if the event is sent, False otherwise.
     */
    protected final boolean forwardClientMessageToToplevel(long toplevel,
                                                           XClientMessageEvent xclient) {
        assert Thread.holdsLock(XToolkit.getAWTLock());

        EmbedderRegistryEntry entry = getEmbedderRegistryEntry(toplevel);
      
        // Window not registered as an embedder for this protocol.
        if (entry == null) {
            return false;
        }

        // Window didn't have an associated drop site, so there is no need 
        // to forward the message.
        if (!entry.isOverriden()) {
            return false;
        }
        
        adjustEventForForwarding(xclient, entry);

        long proxy = entry.getProxy();

        if (proxy == 0) {
            proxy = toplevel;
        }

        XlibWrapper.XSendEvent(XToolkit.getDisplay(), proxy, false,
                               XlibWrapper.NoEventMask, xclient.pData);

        return true;
    }


    /* True iff the previous notification was MotionEvent and it was
       forwarded to the browser. */
    private boolean motionPassedAlong = false;

    protected abstract void sendEnterMessageToToplevel(long toplevel, 
                                                       XClientMessageEvent xclient); 

    protected abstract void sendLeaveMessageToToplevel(long toplevel, 
                                                       XClientMessageEvent xclient); 

    private void postProcessClientMessage(XClientMessageEvent xclient, 
                                          boolean processed, 
                                          int type) {
        long toplevel = xclient.get_window();

        if (getEmbedderRegistryEntry(toplevel) != null) {
            /*
             * This code forwards drag notifications to the browser according to the
             * following rules:
             *  - the messages that we failed to process are always forwarded to the
             *    browser;
             *  - MotionEvents and DropEvents are forwarded if and only if the drag
             *    is not over a plugin window; 
             *  - XDnD: EnterEvents and LeaveEvents are never forwarded, instead, we
             *    send synthesized EnterEvents or LeaveEvents when the drag 
             *    respectively exits or enters plugin windows;
             *  - Motif DnD: EnterEvents and LeaveEvents are always forwarded.
             * Synthetic EnterEvents and LeaveEvents are needed, because the XDnD drop
             * site implemented Netscape 6.2 has a nice feature: when it receives
             * the first XdndPosition it continuously sends XdndStatus messages to
             * the source (every 100ms) until the drag terminates or leaves the drop
             * site. When the mouse is dragged over plugin window embedded in the
             * browser frame, these XdndStatus messages are mixed with the XdndStatus
             * messages sent from the plugin.
             * For Motif DnD, synthetic events cause Motif warnings being displayed,
             * so these events are always forwarded. However, Motif DnD drop site in
             * Netscape 6.2 is implemented in the same way, so there could be similar
             * problems if the drag source choose Motif DnD for communication.
             */
            if (!processed) {
                forwardClientMessageToToplevel(toplevel, xclient);
            } else {
                boolean motifProtocol = 
                    xclient.get_message_type() == 
                    MotifDnDConstants.XA_MOTIF_DRAG_AND_DROP_MESSAGE.getAtom();

                switch (type) {
                case XDropTargetProtocol.MOTION_MESSAGE:
                    if (!isDragOverComponent()) {
                        if (!motionPassedAlong && !motifProtocol) {
                            sendEnterMessageToToplevel(toplevel, xclient); 
                        }
                        forwardClientMessageToToplevel(toplevel, xclient);
                        motionPassedAlong = true;
                    } else {
                        if (motionPassedAlong && !motifProtocol) {
                            sendLeaveMessageToToplevel(toplevel, xclient); 
                        }
                        motionPassedAlong = false;
                    }
                    break;
                case XDropTargetProtocol.DROP_MESSAGE:
                    if (!isDragOverComponent()) {
                        forwardClientMessageToToplevel(toplevel, xclient);
                    }
                    motionPassedAlong = false;
                    break;
                case XDropTargetProtocol.ENTER_MESSAGE:
                case XDropTargetProtocol.LEAVE_MESSAGE:
                    if (motifProtocol) {
                        forwardClientMessageToToplevel(toplevel, xclient);
                    }
                    motionPassedAlong = false;
                    break;
                }
            }
	}
    }

    public abstract boolean sendResponse(long ctxt, int eventID, int action);

    /*
     * Retrieves the data from the drag source in the specified format.
     *
     * @param ctxt a pointer to the XClientMessageEvent structure for this
     *             protocol's drop message. 
     * @param format the format in which the data should be retrieved.
     *
     * @throws IllegalArgumentException if ctxt doesn't point to the
     *         XClientMessageEvent structure for this protocol's drop message.
     * @throws IOException if data retrieval failed.
     */
    public abstract Object getData(long ctxt, long format)
      throws IllegalArgumentException, IOException; 

    public abstract boolean sendDropDone(long ctxt, boolean success, 
                                         int dropAction); 

    public abstract long getSourceWindow();

    public abstract void cleanup();

    public abstract boolean isDragOverComponent();

    public void adjustEventForForwarding(XClientMessageEvent xclient, 
        EmbedderRegistryEntry entry) {}

    protected static final class EmbedderRegistryEntry {
        private final boolean overriden;
        private final int version;
        private final long proxy;
        EmbedderRegistryEntry(boolean overriden, int version, long proxy) {
            this.overriden = overriden;  
            this.version = version; 
            this.proxy = proxy;
        }
        public boolean isOverriden() {
            return overriden;
        }
        public int getVersion() {
            return version;
        }
        public long getProxy() {
            return proxy;
        }
    }

    /* Access to HashMap is synchronized on this XDropTargetProtocol instance. */
    private final HashMap embedderRegistry = new HashMap();
        
    protected final void putEmbedderRegistryEntry(long embedder, 
                                                  boolean overriden,  
                                                  int version, 
                                                  long proxy) {
        synchronized (this) {
            embedderRegistry.put(new Long(embedder),
                                 new EmbedderRegistryEntry(overriden, version,
                                                           proxy));
        }
    }

    protected final EmbedderRegistryEntry getEmbedderRegistryEntry(long embedder) {
        synchronized (this) {
            return 
                (EmbedderRegistryEntry)embedderRegistry.get(new Long(embedder));
        }
    }
}
