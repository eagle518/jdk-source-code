/*
 * @(#)WPopupMenuPeer.java	1.15 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.windows;

import java.awt.*;
import java.awt.peer.*;
import java.lang.reflect.Field;

public class WPopupMenuPeer extends WMenuPeer implements PopupMenuPeer {
    // We can't use target.getParent() for TrayIcon popup
    // because this method should return null for the TrayIcon
    // popup regardless of that whether it has parent or not.
    private static Field f_parent;
    private static Field f_isTrayIconPopup;

    static {
        f_parent = WToolkit.getField(MenuComponent.class, "parent");
        f_isTrayIconPopup = WToolkit.getField(PopupMenu.class, "isTrayIconPopup");
    }

    public WPopupMenuPeer(PopupMenu target) {
	this.target = target;
        MenuContainer parent = null;
        boolean isTrayIconPopup = false;
        try {
            isTrayIconPopup = ((Boolean)f_isTrayIconPopup.get(target)).booleanValue();
            if (isTrayIconPopup) {
                parent = (MenuContainer)f_parent.get(target);
            } else {
                parent = target.getParent();
            }
        } catch (IllegalAccessException iae) {
            iae.printStackTrace();
            return;
        } 

	if (parent instanceof Component) {
	    WComponentPeer parentPeer = (WComponentPeer) WToolkit.targetToPeer(parent);
	    if (parentPeer == null) {
		// because the menu isn't a component (sigh) we first have to wait
		// for a failure to map the peer which should only happen for a 
		// lightweight container, then find the actual native parent from
		// that component.
		parent = WToolkit.getNativeContainer((Component)parent);
		parentPeer = (WComponentPeer) WToolkit.targetToPeer(parent);
	    }
	    createMenu(parentPeer);
            // fix for 5088782: check if menu object is created successfully
            checkMenuCreation();
	} else {
	    throw new IllegalArgumentException(
                "illegal popup menu container class");
	}
    }

    native void createMenu(WComponentPeer parent);

    public void show(Event e) {
	Component origin = (Component)e.target;
	WComponentPeer peer = (WComponentPeer) WToolkit.targetToPeer(origin);
	if (peer == null) {
	    // A failure to map the peer should only happen for a 
	    // lightweight component, then find the actual native parent from
	    // that component.  The event coorinates are going to have to be
	    // remapped as well.
	    Component nativeOrigin = WToolkit.getNativeContainer(origin);
	    e.target = nativeOrigin;

	    // remove the event coordinates 
	    for (Component c = origin; c != nativeOrigin; c = c.getParent()) {
		Point p = c.getLocation();
		e.x += p.x;
		e.y += p.y;
	    }
	}
	_show(e);
    }

    /*
     * This overloaded method is for TrayIcon.
     * Its popup has special parent.
     */
    void show(Component origin, Point p) {
        WComponentPeer peer = (WComponentPeer) WToolkit.targetToPeer(origin);
        Event e = new Event(origin, 0, Event.MOUSE_DOWN, p.x, p.y, 0, 0);
        if (peer == null) {
            Component nativeOrigin = WToolkit.getNativeContainer(origin);
            e.target = nativeOrigin;
        }
        e.x = p.x;
        e.y = p.y;
        _show(e); 
    }

    public native void _show(Event e); 
}
