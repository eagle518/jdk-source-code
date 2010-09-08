/*
 * @(#)XEmbeddedFramePeer.java	1.37 10/04/07
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.X11;

import sun.awt.EmbeddedFrame;
import sun.awt.X11.XEmbeddedFrame;
import java.awt.peer.ComponentPeer;
import java.awt.*;
import java.awt.event.ComponentEvent;
import java.util.logging.*;
import java.awt.AWTKeyStroke;
import java.util.LinkedList;
import java.util.Iterator;

public class XEmbeddedFramePeer extends XFramePeer {

    private static final Logger xembedLog = Logger.getLogger("sun.awt.X11.xembed");

    LinkedList<AWTKeyStroke> strokes;

    XEmbedClientHelper embedder; // Caution - can be null if XEmbed is not supported
    public XEmbeddedFramePeer(EmbeddedFrame target) {
        // Don't specify PARENT_WINDOW param here. Instead we reparent
        // this embedded frame peer to the proper parent window after
        // an XEventDispatcher is registered to handle XEmbed events
        super(new XCreateWindowParams(new Object[] {
            TARGET, target,
            VISIBLE, Boolean.TRUE,
            EMBEDDED, Boolean.TRUE}));
    }

    public void preInit(XCreateWindowParams params) {
        super.preInit(params);
	strokes = new LinkedList<AWTKeyStroke>();
        if (supportsXEmbed()) {
            embedder = new XEmbedClientHelper();
        }
    }
    void postInit(XCreateWindowParams params) {
        super.postInit(params);
        if (embedder != null) {
            // install X11 event dispatcher
            embedder.setClient(this);
            // reparent to XEmbed server
            embedder.install();
        } else if (getParentWindowHandle() != 0) {
            XToolkit.awtLock();
            try {
                XlibWrapper.XReparentWindow(XToolkit.getDisplay(),
                                            getWindow(),
                                            getParentWindowHandle(),
                                            0, 0);
            } finally {
                XToolkit.awtUnlock();
            }
        }
    }

    @Override
    public void dispose() {
        if (embedder != null) {
            // uninstall X11 event dispatcher
            embedder.setClient(null);
        }
        super.dispose();
    }

    public void updateMinimumSize() {
    }

    protected String getWMName() {
        return "JavaEmbeddedFrame";
    }

    final long getParentWindowHandle() {
        return ((XEmbeddedFrame)target).handle;
    }

    boolean supportsXEmbed() {
        return ((EmbeddedFrame)target).supportsXEmbed();
    }

    public boolean requestWindowFocus() {
         // Should check for active state of host application
         if (embedder != null && embedder.isActive()) {
             xembedLog.fine("Requesting focus from embedding host");
             return embedder.requestFocus();
         } else {
             xembedLog.fine("Requesting focus from X");
             return super.requestWindowFocus();
         }
    }

    protected void requestInitialFocus() {
        if (embedder != null && supportsXEmbed()) {
            embedder.requestFocus();
        } else {
            super.requestInitialFocus();
        }
    }

    protected boolean isEventDisabled(XEvent e) {
        if (embedder != null && embedder.isActive()) {
            switch (e.get_type()) {
              case FocusIn:
              case FocusOut:
                  return true;
            }
        }
        return super.isEventDisabled(e);
    }

    public void handleConfigureNotifyEvent(XEvent xev)
    {
        XConfigureEvent xe = xev.get_xconfigure();
        if (xembedLog.isLoggable(Level.FINE)) {
            xembedLog.fine(xe.toString());
        }

        // fix for 5063031
        // if we use super.handleConfigureNotifyEvent() we would get wrong
        // size and position because embedded frame really is NOT a decorated one 
        checkIfOnNewScreen(toGlobal(new Rectangle(xe.get_x(),
                                                  xe.get_y(),
                                                  xe.get_width(),
                                                  xe.get_height())));

        Rectangle oldBounds = getBounds();

        synchronized (getStateLock()) {
            x = xe.get_x();
            y = xe.get_y();
            width = xe.get_width();
            height = xe.get_height();

            dimensions.setClientSize(width, height);
            dimensions.setLocation(x, y);
        }

        if (!getLocation().equals(oldBounds.getLocation())) {
            handleMoved(dimensions);
        }

        reconfigureContentWindow(dimensions);
    }

    protected void traverseOutForward() {
        if (embedder != null && embedder.isActive()) {
            if (embedder.isApplicationActive()) {  
                xembedLog.fine("Traversing out Forward");
                embedder.traverseOutForward();
            }
        }
    }
    
    protected void traverseOutBackward() {
        if (embedder != null && embedder.isActive()) {
            if (embedder.isApplicationActive()) {  
                xembedLog.fine("Traversing out Backward");
                embedder.traverseOutBackward();
            }
        }
    }

    // don't use getLocationOnScreen() inherited from XDecoratedPeer
    public Point getLocationOnScreen() {        
        XToolkit.awtLock();
        try {
            return toGlobal(0, 0);
        } finally {
            XToolkit.awtUnlock();
        }
    }

    @Override
    Rectangle constrainBounds(int x, int y, int width, int height) {
        // We don't constrain the bounds of the EmbeddedFrames
        return new Rectangle(x, y, width, height);
    }

    // don't use getBounds() inherited from XDecoratedPeer
    public Rectangle getBounds() {
        return new Rectangle(x, y, width, height);
    }

    public void setBoundsPrivate(int x, int y, int width, int height) {
        setBounds(x, y, width, height, SET_BOUNDS | NO_EMBEDDED_CHECK);
    }

    public Rectangle getBoundsPrivate() {
        int x = 0, y = 0;
        int w = 0, h = 0;
        XWindowAttributes attr = new XWindowAttributes();

        XToolkit.awtLock();
        try {
            XlibWrapper.XGetWindowAttributes(XToolkit.getDisplay(), 
                getWindow(), attr.pData);
            x = attr.get_x();
            y = attr.get_y();
            w = attr.get_width();
            h = attr.get_height();
        } finally {
            XToolkit.awtUnlock();
        }
        attr.dispose();

        return new Rectangle(x, y, w, h);
    }
    void registerAccelerator(AWTKeyStroke stroke) {
        if (stroke == null) return;
        strokes.add(stroke);
        if (embedder != null && embedder.isActive()) {
            embedder.registerAccelerator(stroke, strokes.size()-1);
        }
    }

    void unregisterAccelerator(AWTKeyStroke stroke) {
        if (stroke == null) return;
        if (embedder != null && embedder.isActive()) {
            int index = strokes.indexOf(stroke);
            embedder.unregisterAccelerator(index);
        }
    }    

    void notifyStarted() {
        // Register accelerators
        if (embedder != null && embedder.isActive()) {
            int i = 0;
            Iterator<AWTKeyStroke> iter = strokes.iterator();
            while (iter.hasNext()) {
                embedder.registerAccelerator(iter.next(), i++);
            }
        }
        // Now we know that the the embedder is an XEmbed server, so we
        // reregister the drop target to enable XDnD protocol support via
        // XEmbed.
        updateDropTarget();
    }

    void notifyStopped() {
        if (embedder != null && embedder.isActive()) {
            for (int i = strokes.size() - 1; i >= 0; i--) {
                embedder.unregisterAccelerator(i);
            }
        }
    }

    long getFocusTargetWindow() {
        return getWindow();
    }

    boolean isXEmbedActive() {
        return embedder != null && embedder.isActive();
    }

    public int getAbsoluteX() {
        XToolkit.awtLock();
        try {
            XTranslateCoordinates xtc =
                new XTranslateCoordinates(getWindow(), XToolkit.getDefaultRootWindow(), 0, 0);
            try {
                int status = xtc.execute(XToolkit.IgnoreBadWindowHandler);
                if (status == 0 ||
                    (XToolkit.saved_error != null &&
                     XToolkit.saved_error.get_error_code() != XlibWrapper.Success)) {
                    return 0;
                } else {
                    return xtc.get_dest_x();
                }
            } finally {
                xtc.dispose();
            }
        } finally {
            XToolkit.awtUnlock();
        }
    }

    public int getAbsoluteY() {
        XToolkit.awtLock();
        try {
            XTranslateCoordinates xtc =
                new XTranslateCoordinates(getWindow(), XToolkit.getDefaultRootWindow(), 0, 0);
            try {
                int status = xtc.execute(XToolkit.IgnoreBadWindowHandler);
                if (status == 0 ||
                    (XToolkit.saved_error != null &&
                     XToolkit.saved_error.get_error_code() != XlibWrapper.Success)) {
                    return 0;
                } else {
                    return xtc.get_dest_y();
                }
            } finally {
                xtc.dispose();
            }
        } finally {
            XToolkit.awtUnlock();
        }
    }

    public int getWidth() {
        return width;
    }
    public int getHeight() {
        return height;
    }

    public Dimension getSize() {
        return new Dimension(width, height);
    }

    // override XWindowPeer's method to let the embedded frame to block
    // the containing window
    public void setModalBlocked(Dialog blocker, boolean blocked) {
        super.setModalBlocked(blocker, blocked);

        EmbeddedFrame frame = (EmbeddedFrame)target;
        frame.notifyModalBlocked(blocker, blocked);
    }

    public void synthesizeFocusInOut(boolean doFocus) {
        XFocusChangeEvent xev = new XFocusChangeEvent();
        
        XToolkit.awtLock();
        try {
            xev.set_type(doFocus ? XlibWrapper.FocusIn : XlibWrapper.FocusOut);
            xev.set_window(getFocusProxy().getWindow());
            xev.set_mode(XlibWrapper.NotifyNormal);
            XlibWrapper.XSendEvent(XToolkit.getDisplay(), getFocusProxy().getWindow(), false,
                                   XlibWrapper.NoEventMask, xev.pData);
        } finally {
            XToolkit.awtUnlock();
            xev.dispose();
        }      
    }
}
