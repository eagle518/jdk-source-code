/*
 * @(#)MEmbeddedFrame.java	1.22 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.motif;

import java.awt.Component;
import java.awt.peer.FramePeer;
import sun.awt.EmbeddedFrame;
import java.awt.peer.ComponentPeer;
import sun.awt.*;
import java.awt.*;
import java.util.logging.*;

public class MEmbeddedFrame extends EmbeddedFrame {

    private static final Logger xembedLog = Logger.getLogger("sun.awt.motif.xembed.MEmbeddedFrame");

    /**
     * Widget id of the shell widget
     */
    long handle;

    public enum IDKind {
        WIDGET,
        WINDOW
    };

    public MEmbeddedFrame() {
    }

    /**
     * Backward-compatible implementation. This constructor takes widget which represents Frame's
     * shell and uses it as top-level to build hierarchy of top-level widgets upon. It assumes that
     * no XEmbed support is provided.
     * @param widget a valid Xt widget pointer.
     */
    public MEmbeddedFrame(long widget) {
        this(widget, IDKind.WIDGET, false);
    }

    /**
     * New constructor, gets X Window id and allows to specify whether XEmbed is supported by parent
     * X window. Creates hierarchy of top-level widgets under supplied window ID.
     * @param winid a valid X window
     * @param supportsXEmbed whether the host application supports XEMBED protocol
     */
    public MEmbeddedFrame(long winid, boolean supportsXEmbed) {
        this(winid, IDKind.WINDOW, supportsXEmbed);
    }

    /**
     * Creates embedded frame using ID as parent.
     * @param ID parent ID
     * @param supportsXEmbed whether the host application supports XEMBED protocol
     * @param kind if WIDGET, ID represents a valid Xt widget pointer; if WINDOW, ID is a valid X Window
     * ID
     */
    public MEmbeddedFrame(long ID, IDKind kind, boolean supportsXEmbed) {
        super(supportsXEmbed);
        if (kind == IDKind.WIDGET) {
            this.handle = ID;
        } else {
            this.handle = getWidget(ID);
        }
        MToolkit toolkit = (MToolkit)Toolkit.getDefaultToolkit();
        setPeer(toolkit.createEmbeddedFrame(this));
        /*
         * addNotify() creates a LightweightDispatcher that propagates
         * SunDropTargetEvents to subcomponents. 
         * NOTE: show() doesn't call addNotify() for embedded frames.
         */
        addNotify();
        show();
    }

    /*
     * The method shouldn't be called in case of active XEmbed.
     */
    public void synthesizeWindowActivation(boolean b) {
        MEmbeddedFramePeer peer = (MEmbeddedFramePeer)getPeer();
        if (peer != null) {
            if (peer.supportsXEmbed() && peer.isXEmbedActive()) {
                xembedLog.fine("The method shouldn't be called when XEmbed is active!");
            } else {
                peer.synthesizeFocusInOut(b);
            }
        }
    }

    public void show() {
	if (handle != 0) {
	    mapWidget(handle);
	}
	super.show();
    }

    /*
     * The method shouldn't be called in case of active XEmbed.
     */
    public boolean traverseIn(boolean direction) {
        MEmbeddedFramePeer peer = (MEmbeddedFramePeer)getPeer();
        if (peer != null) {
            if (peer.supportsXEmbed() && peer.isXEmbedActive()) {
                xembedLog.fine("The method shouldn't be called when XEmbed is active!");
            } else {
                return super.traverseIn(direction);
            }
        }
        return false;
    }

    protected boolean traverseOut(boolean direction) {
        MEmbeddedFramePeer xefp = (MEmbeddedFramePeer) getPeer();
        xefp.traverseOut(direction);
        return true;
    }

    // Native methods to handle widget <-> X Windows mapping
    //
    static native long getWidget(long winid);
    static native int mapWidget(long widget);
    public void registerAccelerator(AWTKeyStroke stroke) {
        MEmbeddedFramePeer xefp = (MEmbeddedFramePeer) getPeer();
        if (xefp != null) {
            xefp.registerAccelerator(stroke);
        }
    }
    public void unregisterAccelerator(AWTKeyStroke stroke) {
        MEmbeddedFramePeer xefp = (MEmbeddedFramePeer) getPeer();
        if (xefp != null) {
            xefp.unregisterAccelerator(stroke);
        }
    }
}
