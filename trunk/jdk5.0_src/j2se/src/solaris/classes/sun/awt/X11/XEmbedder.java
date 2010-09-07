/*
 * @(#)XEmbedder.java	1.14 04/01/21
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.X11;

import sun.misc.Unsafe;

import java.util.logging.*;

/**
 * Helper class implementing XEmbed protocol handling routines(client side)
 * Window which wants to participate in a protocol should create an instance,
 * call install and forward all XClientMessageEvents to it.
 */
public class XEmbedder extends XEmbed implements XEventDispatcher {
    private static final Logger xembedLog = Logger.getLogger("sun.awt.X11.xembed");
    
    private XWindowPeer embedded;
    private boolean active;
    private long server;
    private boolean applicationActive;

    XEmbedder() {
        super();
    }

    void install(XWindowPeer embedded) {
        this.embedded = embedded;

        if (xembedLog.isLoggable(Level.FINE)) xembedLog.fine("Installing xembedder on " + embedded);
        XToolkit.addEventDispatcher(embedded.getWindow(), this);
        long[] info = new long[] { XEMBED_VERSION, XEMBED_MAPPED };
        long data = Native.card32ToData(info);        
        try {
            XEmbedInfo.setAtomData(embedded.getWindow(), data, 2);
        } finally {
            unsafe.freeMemory(data);
        }
    }
    
    void handleClientMessage(long ptr) {
        XClientMessageEvent msg = new XClientMessageEvent(ptr);
        if (xembedLog.isLoggable(Level.FINE)) xembedLog.fine(msg.toString());
        if (msg.get_message_type() == XEmbed.getAtom()) {
            if (xembedLog.isLoggable(Level.FINE)) xembedLog.fine("Embedded message: " + msgidToString((int)msg.get_data(1)));
            switch ((int)msg.get_data(1)) {
              case XEMBED_EMBEDDED_NOTIFY: // Notification about embedding protocol start
                  active = true;
                  server = getEmbedder(embedded, msg);
                  // Check if window is reparented. If not - it was created with
                  // parent and so we should update it here.
                  if (!embedded.reparented) {
                      embedded.reparented = true;
                      embedded.updateSizeHints();
                  }
                  break;
              case XEMBED_WINDOW_ACTIVATE:
                  applicationActive = true;
                  break;
              case XEMBED_WINDOW_DEACTIVATE:
                  applicationActive = false;
                  break;
              case XEMBED_FOCUS_IN: // We got focus!
                  embedded.handleWindowFocusInSync(0);
                  break;
              case XEMBED_FOCUS_OUT:
                  embedded.handleWindowFocusOutSync(null, 0);
                  break;
            }
        }
    }
    public void dispatchEvent(IXAnyEvent ev) {
        switch((int)ev.get_type()) {
          case (int)XlibWrapper.ClientMessage:
              handleClientMessage(ev.getPData());
              break;
          case (int)XlibWrapper.ReparentNotify:
              handleReparentNotify(ev.getPData());
              break;
        }
    }
    public void handleReparentNotify(long ptr) {
        XReparentEvent re = new XReparentEvent(ptr);
        server = re.get_parent();
    }
    void requestFocus() {
        if (active) {
            sendMessage(server, XEMBED_REQUEST_FOCUS);
        }
    }

    long getEmbedder(XWindowPeer embedded, XClientMessageEvent info) {
        // Embedder is the parent of embedded.
        XQueryTree qt = new XQueryTree(embedded.getWindow());
        try {
            qt.execute();
            return qt.get_parent();
        } finally {
            qt.dispose();
        }
    }

    boolean isApplicationActive() {
        return applicationActive;
    }

    boolean isActive() {
        return active;
    }

    void traverseOutForward() {
        if (active) {
            sendMessage(server, XEMBED_FOCUS_NEXT);
        }
    }
    
    void traverseOutBackward() {
        if (active) {
            sendMessage(server, XEMBED_FOCUS_PREV);
        }
    }

    
}
