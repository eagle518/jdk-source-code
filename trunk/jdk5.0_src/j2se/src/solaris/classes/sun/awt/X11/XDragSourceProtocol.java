/*
 * @(#)XDragSourceProtocol.java	1.4 04/01/08
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.X11;

import java.awt.datatransfer.Transferable;

import java.awt.dnd.DnDConstants;
import java.awt.dnd.InvalidDnDOperationException;

import java.util.Map;

/**
 * An abstract class for drag protocols on X11 systems.
 * Contains protocol-independent drag source code.
 *
 * @since 1.5
 */
abstract class XDragSourceProtocol {
    private final XDragSourceProtocolListener listener;    

    private boolean initialized = false;

    private long targetWindow = 0;
    private long targetProxyWindow = 0;
    private int targetProtocolVersion = 0;
    private long targetWindowMask = 0;

    // Always use the XAWT root window as the drag source window.
    static long getDragSourceWindow() {
        return XWindow.getXAWTRootWindow().getWindow();
    }

    protected XDragSourceProtocol(XDragSourceProtocolListener listener) {
        if (listener == null) {
            throw new NullPointerException("Null XDragSourceProtocolListener");
        }
        this.listener = listener;
    }

    protected final XDragSourceProtocolListener getProtocolListener() {
        return listener;
    }

    /**
     * Initalizes a drag operation with the specified supported drop actions,
     * contents and data formats.
     *
     * @param actions a bitwise mask of <code>DnDConstants</code> that represent
     *                the supported drop actions.
     * @param contents the contents for the drag operation.
     * @param formats an array of Atoms that represent the supported data formats.
     * @param formats an array of Atoms that represent the supported data formats.
     * @throws InvalidDnDOperationException if a drag operation is already
     * initialized.
     * @throws IllegalArgumentException if some argument has invalid value.
     * @throws XException if some X call failed.
     */
    public final void initializeDrag(int actions, Transferable contents,
                                     Map formatMap, long[] formats) 
      throws InvalidDnDOperationException, 
             IllegalArgumentException, XException {
        synchronized (XToolkit.getAWTLock()) {
            try {
                if (initialized) {
                    throw new InvalidDnDOperationException("Already initialized");
                }
            
                initializeDragImpl(actions, contents, formatMap, formats);

                initialized = true;
            } finally {
                if (!initialized) {
                    cleanup();
                }
            }
        }
    }

    /* The caller must hold AWT_LOCK. */
    protected abstract void initializeDragImpl(int actions, 
                                               Transferable contents,  
                                               Map formatMap, long[] formats) 
      throws InvalidDnDOperationException, IllegalArgumentException, XException;

    /**
     * Terminates the current drag operation (if any) and resets the internal
     * state of this object.
     *
     * @throws XException if some X call failed.
     */
    public void cleanup() {
        initialized = false;
        cleanupTargetInfo();
    }

    /**
     * Clears the information on the current drop target.
     *
     * @throws XException if some X call failed.
     */
    public void cleanupTargetInfo() {
        targetWindow = 0;
        targetProxyWindow = 0;
        targetProtocolVersion = 0;
    }

    /**
     * Processes the specified client message event.
     *
     * @returns true if the event was successfully processed.
     */
    public abstract boolean processClientMessage(XClientMessageEvent xclient) 
      throws XException;

    /* The caller must hold AWT_LOCK. */
    public abstract boolean attachTargetWindow(long window, long time);

    /* The caller must hold AWT_LOCK. */
    public abstract void sendEnterMessage(long[] formats, int sourceAction, 
                                          int sourceActions, long time);
    /* The caller must hold AWT_LOCK. */
    public abstract void sendMoveMessage(int xRoot, int yRoot, 
                                         int sourceAction, int sourceActions,
                                         long time);
    /* The caller must hold AWT_LOCK. */
    public abstract void sendLeaveMessage(long time);

    /* The caller must hold AWT_LOCK. */
    protected abstract void sendDropMessage(int xRoot, int yRoot, 
                                            int sourceAction, int sourceActions,
                                            long time);

    public final void initiateDrop(int xRoot, int yRoot, 
                                   int sourceAction, int sourceActions,
                                   long time) {
        XWindowAttributes wattr = new XWindowAttributes();
        try {
            XToolkit.WITH_XERROR_HANDLER(XToolkit.IgnoreBadWindowHandler);
            int status = XlibWrapper.XGetWindowAttributes(XToolkit.getDisplay(),
                                                          targetWindow, wattr.pData);
                
            XToolkit.RESTORE_XERROR_HANDLER();

            if (status == 0 || 
                (XToolkit.saved_error != null && 
                 XToolkit.saved_error.get_error_code() != XlibWrapper.Success)) {
                throw new XException("XGetWindowAttributes failed");
            }
             
            targetWindowMask = wattr.get_your_event_mask();
        } finally {
            wattr.dispose();
        }
                
        XToolkit.WITH_XERROR_HANDLER(XToolkit.IgnoreBadWindowHandler);
        XlibWrapper.XSelectInput(XToolkit.getDisplay(), targetWindow,
                                 targetWindowMask | 
                                 XlibWrapper.StructureNotifyMask);
            
        XToolkit.RESTORE_XERROR_HANDLER();

        if (XToolkit.saved_error != null && 
            XToolkit.saved_error.get_error_code() != XlibWrapper.Success) {
            throw new XException("XSelectInput failed");
        }

        sendDropMessage(xRoot, yRoot, sourceAction, sourceActions, time);
    }

    protected final void finalizeDrop() {
        XToolkit.WITH_XERROR_HANDLER(XToolkit.IgnoreBadWindowHandler);
        XlibWrapper.XSelectInput(XToolkit.getDisplay(), targetWindow,
                                 targetWindowMask);
        XToolkit.RESTORE_XERROR_HANDLER();
    }

    protected final long getTargetWindow() {
        return targetWindow;
    }

    protected final long getTargetProxyWindow() {
        if (targetProxyWindow != 0) {
            return targetProxyWindow;
        } else {
            return targetWindow;
        }
    }

    protected final int getTargetProtocolVersion() {
        return targetProtocolVersion;
    }

    protected final void setTargetWindow(long window) {
        targetWindow = window;
    }
    protected final void setTargetProxyWindow(long proxy) {
        targetProxyWindow = proxy;
    }
    protected final void setTargetProtocolVersion(int version) {
        targetProtocolVersion = version;
    }
}
