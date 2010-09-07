/*
 * @(#)MEmbeddedFrame.java	1.18 04/01/26
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.motif;

import java.awt.Component;
import java.awt.peer.FramePeer;
import sun.awt.EmbeddedFrame;
import java.awt.peer.ComponentPeer;
import sun.awt.*;
import java.awt.*;

public class MEmbeddedFrame extends EmbeddedFrame {

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

    public void synthesizeWindowActivation(boolean b) {
        MEmbeddedFramePeer peer = (MEmbeddedFramePeer)getPeer();
        if (peer != null) {
            if (peer.supportsXEmbed()) {
                if (peer.isXEmbedActive()) {
                    // If XEmbed is active no synthetic focus events are allowed - everything
                    // should go through XEmbed
                    if (b) {
                        peer.requestXEmbedFocus();
                    }
                }
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

    // Native methods to handle widget <-> X Windows mapping
    //
    static native long getWidget(long winid);
    static native int mapWidget(long widget);
}
