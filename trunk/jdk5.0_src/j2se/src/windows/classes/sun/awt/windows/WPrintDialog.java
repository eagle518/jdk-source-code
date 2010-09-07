/*
 * @(#)WPrintDialog.java	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.windows;

import java.awt.*;
import java.awt.peer.*;

import java.awt.print.PrinterJob;

public class WPrintDialog extends Dialog {
    static { 
        initIDs();
    }

    protected PrintJob job;
    protected PrinterJob pjob;

    public WPrintDialog(Frame parent, PrinterJob control) {
        super(parent, true);
	this.pjob = control;
	setLayout(null);
    }

    // Use native code to circumvent access restrictions on Component.peer
    protected native void setPeer(ComponentPeer peer);

    public void addNotify() {
        synchronized(getTreeLock()) {
	    Container parent = getParent();
            if (parent != null && parent.getPeer() == null) {
                parent.addNotify();
            }

	    if (getPeer() == null) {
	        ComponentPeer peer = ((WToolkit)Toolkit.getDefaultToolkit()).
		    createWPrintDialog(this);
		setPeer(peer);
	    }
	    super.addNotify();
	}
    }

    private boolean retval = false;
    
    public void setRetVal(boolean ret) {
	retval = ret;
    }

    public boolean getRetVal() {
	return retval;
    }

    /**
     * Initialize JNI field and method ids
     */
    private static native void initIDs();
}
