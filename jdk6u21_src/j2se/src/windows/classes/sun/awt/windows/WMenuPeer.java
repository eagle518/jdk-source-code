/*
 * @(#)WMenuPeer.java	1.18 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
            this.parent = mbPeer;
	    createMenu(mbPeer);
	}
        else if (parent instanceof Menu) {
	    this.parent = (WMenuPeer) WToolkit.targetToPeer(parent);
	    createSubMenu(this.parent);
	}
        else {
	    throw new IllegalArgumentException("unknown menu container class");
	}
        // fix for 5088782: check if menu object is created successfully
        checkMenuCreation();
    }

    native void createMenu(WMenuBarPeer parent);
    native void createSubMenu(WMenuPeer parent);
}
