/*
 * @(#)WMenuBarPeer.java	1.16 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.windows;

import java.awt.*;
import java.awt.peer.*;

class WMenuBarPeer extends WMenuPeer implements MenuBarPeer {

    // MenuBarPeer implementation
  
    public native void addMenu(Menu m);
    public native void delMenu(int index);

    public void addHelpMenu(Menu m) {
        addMenu(m);
    }

    // Toolkit & peer internals
    WMenuBarPeer(MenuBar target) {
	this.target = target;
	WFramePeer framePeer = (WFramePeer) 
	    WToolkit.targetToPeer(target.getParent());
	create(framePeer);
    }
    native void create(WFramePeer f);
}
