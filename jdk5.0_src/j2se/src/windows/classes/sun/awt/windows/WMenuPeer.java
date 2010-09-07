/*
 * @(#)WMenuPeer.java	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.windows;

import java.awt.*;
import java.awt.peer.*;

class WMenuPeer extends WMenuItemPeer implements MenuPeer {
  
    // MenuPeer implementation

    public native void addSeparator();
    public void addItem(MenuItem item) {
	WMenuItemPeer itemPeer = (WMenuItemPeer) WToolkit.targetToPeer(item);
    }
    public native void delItem(int index);

    // Toolkit & peer internals

    WMenuPeer() {}   // used by subclasses.

    WMenuPeer(Menu target) {
	this.target = target;
	MenuContainer parent = target.getParent();

	if (parent instanceof MenuBar) {
	    WMenuBarPeer mbPeer = (WMenuBarPeer) WToolkit.targetToPeer(parent);
	    createMenu(mbPeer);
	}
        else if (parent instanceof Menu) {
	    WMenuPeer mPeer = (WMenuPeer) WToolkit.targetToPeer(parent);
	    createSubMenu(mPeer);
	}
        else {
	    throw new IllegalArgumentException("unknown menu container class");
	}
    }

    native void createMenu(WMenuBarPeer parent);
    native void createSubMenu(WMenuPeer parent);
}
