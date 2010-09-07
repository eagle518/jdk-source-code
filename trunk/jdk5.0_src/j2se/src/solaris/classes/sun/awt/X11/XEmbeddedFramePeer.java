/*
 * @(#)MEmbeddedFramePeer.java	1.14 01/12/03
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.X11;

import sun.awt.EmbeddedFrame;
import sun.awt.X11.XEmbeddedFrame;
import java.awt.peer.ComponentPeer;
import java.awt.*;
import java.util.logging.*;

public class XEmbeddedFramePeer extends XFramePeer {

    private static final Logger xembedLog = Logger.getLogger("sun.awt.X11.xembed");

    XEmbedder embedder;
    public XEmbeddedFramePeer(EmbeddedFrame target) {
        super(new XCreateWindowParams(new Object[] {
            TARGET, target,
            PARENT_WINDOW, new Long(((XEmbeddedFrame)target).handle),
            REPARENTED, Boolean.TRUE,
            VISIBLE, Boolean.TRUE,
            EMBEDDED, Boolean.TRUE}));
    }

    public void preInit(XCreateWindowParams params) {
        super.preInit(params);
        embedder = new XEmbedder();
    }
    void postInit(XCreateWindowParams params) {
        super.postInit(params);
        if (embedder != null) {
            embedder.install(this);
        }        
        
    }
    protected String getWMName() {
        return "JavaEmbeddedFrame";
    }

    public boolean requestWindowFocus() {
        // Should check for active state of host application
        if (embedder != null && embedder.isActive()) {
            if (embedder.isApplicationActive()) {            
                xembedLog.fine("Requesting focus from embedding host");
                embedder.requestFocus();
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

    protected void requestInitialFocus() {
        if (embedder != null && ((EmbeddedFrame)target).supportsXEmbed()) {
            embedder.requestFocus();
        } else {
            super.requestInitialFocus();
        }
    }

    protected boolean isEventDisabled(IXAnyEvent e) {
        if (embedder != null && embedder.isActive()) {
            switch (e.get_type()) {
              case FocusIn:
              case FocusOut:
                  return true;
            }
        }
        return super.isEventDisabled(e);
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

    public void setBoundsPrivate(int x, int y, int width, int height) {
        setBounds(x, y, width, height, SET_BOUNDS | NO_EMBEDDED_CHECK);
    }
}
