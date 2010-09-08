/*
 * @(#)XFocusProxyWindow.java	1.6 10/03/23
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.X11;

import java.awt.*;
import java.awt.event.*;

/**
 * This class represent focus holder window implementation. When toplevel requests or receives focus
 * it instead sets focus to this proxy. This proxy is mapped but invisible(it is kept at (-1,-1))
 * and therefore X doesn't control focus after we have set it to proxy.
 */
public class XFocusProxyWindow extends XBaseWindow {
    XWindowPeer owner;

    public XFocusProxyWindow(XWindowPeer owner) {
        super(new XCreateWindowParams(new Object[] {
            BOUNDS, new Rectangle(-1, -1, 1, 1),
            PARENT_WINDOW, new Long(owner.getWindow()),
            EVENT_MASK, new Long(FocusChangeMask | KeyPressMask | KeyReleaseMask)
        }));
        this.owner = owner;
    }

    public void postInit(XCreateWindowParams params){
        super.postInit(params);
        setWMClass(getWMClass());
        xSetVisible(true);
    }

    protected String getWMName() {
        return "FocusProxy";
    }
    protected String[] getWMClass() {
        return new String[] {"Focus-Proxy-Window", "FocusProxy"};
    }

    public XWindowPeer getOwner() {
        return owner;
    }

    public void dispatchEvent(XEvent ev) {
        int type = ev.get_type();  
        switch (type)
        {
          case XlibWrapper.FocusIn:
          case XlibWrapper.FocusOut:
              handleFocusEvent(ev);
              break;
        }
        super.dispatchEvent(ev);
    }    

    public void handleFocusEvent(XEvent xev) {
        owner.handleFocusEvent(xev);
    }

    public void handleKeyPress(XEvent xev) {
        owner.handleKeyPress(xev);
    }

    public void handleKeyRelease(XEvent xev) {
        owner.handleKeyRelease(xev);
    }
}
