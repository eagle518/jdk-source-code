/*
 * @(#)MEmbeddedFramePeer.java	1.21 04/07/12
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.motif;

import sun.awt.EmbeddedFrame;
import java.util.logging.*;
import java.awt.Window;

public class MEmbeddedFramePeer extends MFramePeer {
    private static final Logger xembedLog = Logger.getLogger("sun.awt.motif.xembed.MEmbeddedFramePeer");

    public MEmbeddedFramePeer(EmbeddedFrame target) {
        super(target);
        xembedLog.fine("Creating XEmbed-enabled motif embedded frame, frame supports XEmbed:" + supportsXEmbed());
    }

    void create(MComponentPeer parent) {
	NEFcreate(parent, ((MEmbeddedFrame)target).handle);
    }
    native void NEFcreate(MComponentPeer parent, long handle);
    native void pShowImpl();
    void pShow() {
        pShowImpl();

        if (supportsXEmbed() && (target != null && ((EmbeddedFrame)target).isFocusableWindow())) {
            requestXEmbedFocus();
        }
    }

    boolean supportsXEmbed() {
        EmbeddedFrame frame = (EmbeddedFrame)target;
        if (frame != null) {
            return frame.supportsXEmbed();
        } else {
            return false;
        }
    }

    public void setVisible(boolean vis) {
        super.setVisible(vis);
        xembedLog.fine("Peer made visible");
        if (vis && !supportsXEmbed()) {
            xembedLog.fine("Synthesizing FocusIn");
            // Fix for 4878303 - generate WINDOW_GAINED_FOCUS and update if we were focused 
            // since noone will do it for us(WM does it for regular top-levels)
            synthesizeFocusInOut(true);
        }
    }
    public native void synthesizeFocusInOut(boolean b);

    native boolean isXEmbedActive();
    native boolean isXEmbedApplicationActive();
    native void requestXEmbedFocus();

    public boolean requestWindowFocus() {
        xembedLog.fine("In requestWindowFocus");
        // Should check for active state of host application
        if (isXEmbedActive()) {
            if (isXEmbedApplicationActive()) {
                xembedLog.fine("Requesting focus from embedding host");
                requestXEmbedFocus();
                return true;
            } else {
                xembedLog.fine("Host application is not active");
                return false;
            }
        } else {
            xembedLog.fine("Requesting focus from X");
            return super.requestWindowFocus();
        }
    }
    public void handleWindowFocusIn() {
        super.handleWindowFocusIn();
        xembedLog.fine("windowFocusIn");        
    }
    public void handleWindowFocusOut(Window oppositeWindow) {
        super.handleWindowFocusOut(oppositeWindow);
        xembedLog.fine("windowFocusOut, opposite is null?:" + (oppositeWindow==null));
    }
}
