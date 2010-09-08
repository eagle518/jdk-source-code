/*
 * @(#)WPageDialogPeer.java	1.5 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.windows;
import java.awt.print.Printable;
import java.awt.*;
import java.awt.event.*;

public class WPageDialogPeer extends WPrintDialogPeer {

    WPageDialogPeer(WPageDialog target) {
	super(target);
    }

    /**
     * Displays the page setup dialog placing the user's
     * settings into target's 'page'.
     */
    private native boolean _show();

    public void show() {
	new Thread(new Runnable() {
		public void run() {			    	    
		    // Call pageSetup even with no printer installed, this
		    // will display Windows error dialog and return false.
		    ((WPrintDialog)target).setRetVal(_show());
		   
		    ((WPrintDialog)target).hide();
		}
	    }).start();
    }   
}
