/*
 * @(#)WPopupMenuPeer.java	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.windows;

import java.awt.*;
import java.awt.peer.*;

public class WPopupMenuPeer extends WMenuPeer implements PopupMenuPeer {

    public WPopupMenuPeer(PopupMenu target) {
	this.target = target;

	if (target.getParent() instanceof Component) {
	    Component parent = (Component)target.getParent();	    
	    WComponentPeer parentPeer = (WComponentPeer) WToolkit.targetToPeer(parent);
	    if (parentPeer == null) {
		// because the menu isn't a component (sigh) we first have to wait
		// for a failure to map the peer which should only happen for a 
		// lightweight container, then find the actual native parent from
		// that component.
		parent = WToolkit.getNativeContainer(parent);
		parentPeer = (WComponentPeer) WToolkit.targetToPeer(parent);
	    }
	    createMenu(parentPeer);
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

    public native void _show(Event e); 
}
