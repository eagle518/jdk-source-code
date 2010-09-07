/*
 * @(#)WPageDialogPeer.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.windows;

public class WPageDialogPeer extends WPrintDialogPeer {

    WPageDialogPeer(WPageDialog target) {
	super(target);
    }

    public void show() {
	new Thread(new Runnable() {
		public void run() {
		    WPageDialog pdlg = (WPageDialog)target;
		    WPrinterJob pjob = (WPrinterJob)pdlg.pjob;

		    // Call pageSetup even with no printer installed, this
		    // will display Windows error dialog and return false.
		    ((WPrintDialog)target).setRetVal(pjob.pageSetup(pdlg.page, pdlg.painter));
		    ((WPrintDialog)target).hide();
		}
	    }).start();
    }
}
