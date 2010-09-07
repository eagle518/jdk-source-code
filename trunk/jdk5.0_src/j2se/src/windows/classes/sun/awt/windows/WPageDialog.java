/*
 * @(#)WPageDialog.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.windows;

import java.awt.Frame;
import java.awt.Toolkit;
import java.awt.peer.ComponentPeer;
import java.awt.print.PrinterJob;
import java.awt.print.PageFormat;
import java.awt.print.Printable;

public class WPageDialog extends WPrintDialog {	
    PageFormat page;
    Printable painter;

    WPageDialog(Frame parent, PrinterJob control, PageFormat page, Printable painter) {
	super(parent, control);
	this.page = page;
	this.painter = painter;
    }

    public void addNotify() {
	synchronized(getTreeLock()) {
	    if (getPeer() == null) {
		ComponentPeer peer = ((WToolkit)Toolkit.getDefaultToolkit()).
		    createWPageDialog(this);
		setPeer(peer);
	    }
	    super.addNotify();
	}
    }
}
