/*
 * @(#)MEmbeddedFrame.java	1.10 01/12/03
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.X11;

import java.awt.Component;
import java.awt.peer.FramePeer;
import sun.awt.EmbeddedFrame;
import java.awt.peer.ComponentPeer;
import sun.awt.*;
import java.awt.*;

public class XEmbeddedFrame extends EmbeddedFrame {

    long handle;
    public XEmbeddedFrame() {
    }

    public XEmbeddedFrame(long handle) {
        this(handle, false);
    }

    // handle should be a valid X Window.
    public XEmbeddedFrame(long handle, boolean supportsXEmbed) {
        super(handle, supportsXEmbed);
        this.handle = handle;
        XToolkit toolkit = (XToolkit)Toolkit.getDefaultToolkit();
        setPeer(toolkit.createEmbeddedFrame(this));
        if (handle != 0) { // Has actual parent
            /*
             * addNotify() creates a LightweightDispatcher that propagates
             * SunDropTargetEvents to subcomponents. 
             * NOTE: show() doesn't call addNotify() for embedded frames.
             */
            addNotify();
            show();
            getPeer().setVisible(true);
        }
    }

    protected boolean traverseOut(boolean direction) {
        XEmbeddedFramePeer xefp = (XEmbeddedFramePeer) getPeer();
        if (direction == FORWARD) {
            xefp.traverseOutForward();
        }
        else {
            xefp.traverseOutBackward();
        }
        return true;
    }

}
