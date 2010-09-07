/*
 * @(#)MMenuPeer.java	1.24 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.motif;

import java.awt.*;
import java.awt.peer.*;

public class MMenuPeer extends MMenuItemPeer implements MenuPeer {
    native void createMenu(MMenuBarPeer parent);
    native void createSubMenu(MMenuPeer parent);

    void create(MMenuPeer parent) {
        if (parent.nativeCreated) {
	    createSubMenu(parent);
            nativeCreated = true;
        }
    }

    protected MMenuPeer() {
    }

    public MMenuPeer(Menu target) {
	this.target = target;
	MenuContainer parent = getParent_NoClientCode(target);

	if (parent instanceof MenuBar) {
	    MMenuBarPeer mb = (MMenuBarPeer) MToolkit.targetToPeer(parent);
	    createMenu(mb);
            nativeCreated = true;
	} else if (parent instanceof Menu) {
	    MMenuPeer m = (MMenuPeer) MToolkit.targetToPeer(parent);
            create(m);
	} else {
	    throw new IllegalArgumentException("unknown menu container class");
	}
    }

    public void addSeparator() {
    }
    public void addItem(MenuItem item) {
    }
    public void delItem(int index) {
    }

    void destroyNativeWidget() {
        // We do not need to synchronize this method because the caller
        // always holds the tree lock

	if (nativeCreated) {
	    Menu menu = (Menu) target;
	    int nitems = menu.getItemCount();
	    for (int i = 0 ; i < nitems ; i++) {
	        MMenuItemPeer mipeer = 
		    (MMenuItemPeer) MToolkit.targetToPeer(menu.getItem(i));
		mipeer.destroyNativeWidget();
	    }
	    super.destroyNativeWidget();
	}
    }
    native void pDispose();
}
