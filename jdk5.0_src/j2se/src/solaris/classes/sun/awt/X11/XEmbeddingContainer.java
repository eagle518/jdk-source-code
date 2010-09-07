/*
 * @(#)XEmbeddingContainer.java	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.X11;

import java.awt.*;
import java.util.HashMap;
import java.awt.event.KeyEvent;
import java.lang.reflect.*;

public class XEmbeddingContainer extends XEmbed implements XEventDispatcher {
    HashMap children = new HashMap();

    XEmbeddingContainer() {
    }

    XWindow embedder;
    void install(XWindow embedder) {
        this.embedder = embedder;
        XToolkit.addEventDispatcher(embedder.getWindow(), this);
    }
    void deinstall() {
        XToolkit.removeEventDispatcher(embedder.getWindow(), this);        
    }

    void add(long child) {
        if (checkXEmbed(child)) {
            Component proxy = createChildProxy(child);
            ((Container)embedder.getTarget()).add("Center", proxy);
            if (proxy.getPeer() != null) {
                children.put(new Long(child), proxy.getPeer());
            }
        }
    }

    Component createChildProxy(long child) {
        return new XEmbedChildProxy(this, child);
    }
    void notifyChildEmbedded(long child) {
        sendMessage(child, XEMBED_EMBEDDED_NOTIFY, embedder.getWindow(), XEMBED_VERSION, 0);
    }

    void childResized(Component child) {
    }

    boolean checkXEmbed(long child) {
        long data = unsafe.allocateMemory(8);
        try {
            if (XEmbedInfo.getAtomData(child, data, 2)) {
                int protocol = unsafe.getInt(data);
                int flags = unsafe.getInt(data);
                return true;
            }
        } finally {
            unsafe.freeMemory(data);
        }
        return false;
    }

    void detachChild(long child) {
//  The embedder can unmap the client and reparent the client window to the root window. If the client receives an ReparentNotify event, it should check the parent field of the XReparentEvent structure. If this is the root window of the window's screen, then the protocol is finished and there is no further interaction. If it is a window other than the root window, then the protocol continues with the new parent acting as the embedder window.        
        try {
            XToolkit.awtLock();
            XlibWrapper.XUnmapWindow(XToolkit.getDisplay(), child);
            XlibWrapper.XReparentWindow(XToolkit.getDisplay(), child, XToolkit.getDefaultRootWindow(), 0, 0);
        }
        finally {
            XToolkit.awtUnlock();
        }
    }

    void focusGained(long child) {
        sendMessage(child, XEMBED_FOCUS_IN, XEMBED_FOCUS_CURRENT, 0, 0);
    }
    void focusLost(long child) {
        sendMessage(child, XEMBED_FOCUS_OUT);
    }

    XEmbedChildProxyPeer getChild(long child) {
        return (XEmbedChildProxyPeer)children.get(new Long(child));
    }
    public void handleClientMessage(long ptr) {
        XClientMessageEvent msg = new XClientMessageEvent(ptr);
        if (msg.get_message_type() == XEmbed.getAtom()) {
            switch ((int)msg.get_data(1)) {
              case XEMBED_REQUEST_FOCUS:
                  long child = msg.get_data(2); // Unspecified
                  getChild(child).requestXEmbedFocus();
                  break;
            }
        }
    }
    public void dispatchEvent(IXAnyEvent ev) {
        switch((int)ev.get_type()) {
          case (int)XlibWrapper.ClientMessage:
              handleClientMessage(ev.getPData());
              break;
        }
    }

    static Field bdata;
    byte[] getBData(KeyEvent e) {
        try {
            if (bdata == null) {
                bdata = XToolkit.getField(java.awt.AWTEvent.class, "bdata");
            }
            return (byte[])bdata.get(e);
        } catch (IllegalAccessException ex) {
            return null;
        }
    }

    void forwardKeyEvent(long child, KeyEvent e) {
        byte[] bdata = getBData(e);
        long data = Native.toData(bdata);
        if (data == 0) {
            return;
        }
        XKeyEvent ke = new XKeyEvent(data);
        ke.set_window(child);
        try {
            XToolkit.awtLock();
            XlibWrapper.XSendEvent(XToolkit.getDisplay(), child, false, XlibWrapper.NoEventMask, data);
        }
        finally {
            XToolkit.awtUnlock();
        }
        XlibWrapper.unsafe.freeMemory(data);
    }
}

