/*
 * @(#)XContentWindow.java	1.17 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.X11;

import java.awt.Component;
import java.awt.Rectangle;
import java.util.logging.*;
import java.awt.Insets;
import java.awt.event.ComponentEvent;

/**
 * This class implements window which serves as content window for decorated frames.
 * Its purpose to provide correct events dispatching for the complex
 * constructs such as decorated frames.
 */
public class XContentWindow extends XWindow implements XConstants {
    private static Logger insLog = Logger.getLogger("sun.awt.X11.insets.XContentWindow");

    XDecoratedPeer parentFrame;
    XContentWindow(XDecoratedPeer parentFrame, Rectangle bounds) {
        super((Component)parentFrame.getTarget(), parentFrame.getShell(), bounds);
        this.parentFrame = parentFrame;
    }
    
    void preInit(XCreateWindowParams params) {
        super.preInit(params);        
        params.putIfNull(BIT_GRAVITY, new Integer(NorthWestGravity));
    }
    
    void initialize() {
        xSetVisible(true);
    }    
    protected String getWMName() {
        return "Content window";
    }
    protected boolean isEventDisabled(IXAnyEvent e) {
        switch (e.get_type()) {
          // Override parentFrame to receive MouseEnter/Exit
          case EnterNotify:
          case LeaveNotify:
              return false;
          // We handle ConfigureNotify specifically in XDecoratedPeer
          case ConfigureNotify:
              return true;
          // We don't want SHOWN/HIDDEN on content window since it will duplicate XDecoratedPeer
          case MapNotify:
          case UnmapNotify:
              return true;
          default:
              return super.isEventDisabled(e) || parentFrame.isEventDisabled(e);
        }
    }

    // Coordinates are that of the shell
    void setContentBounds(WindowDimensions dims) {
        XToolkit.awtLock();
        try {
            // Bounds of content window are of the same size as bounds of Java window and with
            // location as -(insets)
            Rectangle newBounds = dims.getBounds();
            Insets in = dims.getInsets();
            if (in != null) {
                newBounds.setLocation(-in.left, -in.top);
            }
            if (insLog.isLoggable(Level.FINE)) insLog.fine("Setting content bounds " + newBounds);
            boolean needHandleResize = !(newBounds.getSize().equals(getSize()));
            reshape(newBounds);
            if (needHandleResize) {
                handleResize(newBounds);
            }
        } finally {
            XToolkit.awtUnlock();
        }
        validateSurface();
    }

    // NOTE: This method may be called by privileged threads.
    //       DO NOT INVOKE CLIENT CODE ON THIS THREAD!
    public void handleResize(Rectangle bounds) {
        ComponentAccessor.setWidth((Component)target, bounds.width);
        ComponentAccessor.setHeight((Component)target, bounds.height);
        postEvent(new ComponentEvent(target, ComponentEvent.COMPONENT_RESIZED));
    }
}
