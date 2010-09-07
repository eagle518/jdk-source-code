/*
 * @(#)PrintServiceImpl.java	1.31 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jnlp;

import javax.jnlp.PrintService;
import java.awt.print.Pageable;
import java.awt.print.PageFormat;
import java.awt.print.Printable;
import java.awt.print.PrinterJob;
import java.awt.print.PrinterException;
import com.sun.deploy.resources.ResourceManager;
import java.security.AccessController;
import java.security.PrivilegedAction;
import com.sun.deploy.util.Trace;

public final class PrintServiceImpl implements PrintService {
    static private PrintServiceImpl _sharedInstance = null;
    private static ApiDialog _apiDialog = new ApiDialog();
    private PageFormat _pageFormat = null;
    
    /* This is run with all priviledges */
    private PrintServiceImpl() {
    }
    
    /* This is run with all priviledges */
    public static synchronized PrintServiceImpl getInstance() {
        if (_sharedInstance == null) {
	    _sharedInstance = new PrintServiceImpl();
        }
        return _sharedInstance;
    }
    
    // sets up printer job (there's no access to printer itself, so no
    // security hazard and therefore no need for security dialog popup):
    public PageFormat getDefaultPage() {
        return (PageFormat)AccessController.doPrivileged(
	    new PrivilegedAction() {
	        public Object run() {
		    PrinterJob sysPrinterJob = PrinterJob.getPrinterJob();
		    if (sysPrinterJob != null) {
		        return sysPrinterJob.defaultPage();
		    }
		    return null;
	        }
	    });
    }
    
    // sets up printer job (there's no access to printer itself, so no
    // security hazard and therefore no need for security dialog popup):
    public PageFormat showPageFormatDialog(final PageFormat page) {
        return (PageFormat)AccessController.doPrivileged(
	    new PrivilegedAction() {
	        public Object run() {
		    PrinterJob sysPrinterJob = PrinterJob.getPrinterJob();
		    if (sysPrinterJob != null) {
		        // Make sure to remember pageformat
		        _pageFormat = sysPrinterJob.pageDialog(page);
		        return _pageFormat;
		    }
		    return null;
	        }
	    });
    }
    
    // print if permission granted, i.e., if askUser() returns true:
    public synchronized boolean print(final Pageable document) {
	return doPrinting(null, document);
    }
    
    // print if permission granted, i.e., if askUser() returns true:
    public synchronized boolean print(final Printable painter) {
	return doPrinting(painter, null);
    }

    /** This is either called with a painter or a document */
    private boolean doPrinting(final Printable painter, 
			       final Pageable document) {
        if (!askUser()) {
	    return false;
	}
        Boolean result = (Boolean)AccessController.doPrivileged(
    	    new PrivilegedAction() {
	        public Object run() {
		    final PrinterJob sysPrinterJob = PrinterJob.getPrinterJob();
		    if (sysPrinterJob == null) {
		        return Boolean.FALSE;
		    }
		    // Specify document to print
		    if (document != null) {
		        sysPrinterJob.setPageable(document);
		    } else {
		        if (_pageFormat == null ) {
			    sysPrinterJob.setPrintable(painter);
		        } else {
			    sysPrinterJob.setPrintable(painter, _pageFormat);
		        }
		    }
		    // Show printing dialog
		    if (sysPrinterJob.printDialog()) {
		        // Starts a thread in the background to do 
		        // print on to avoid blocking
		        Thread t = new Thread(new Runnable() {
			    public void run() {
			        try {
				    sysPrinterJob.print();
			        } catch (PrinterException pe) {
				    Trace.ignoredException(pe);
			        }
			    }
		        });
		        t.start();
		        return Boolean.TRUE;
		    } else {
		        return Boolean.FALSE;
		    }
	        }
            });
        return result.booleanValue();
    }
    
    private synchronized boolean askUser() {
	// if user already grants unrestricted access, return true:
	if (CheckServicePermission.hasPrintAccessPermissions()) return true;
	return requestPrintPermission();
    }

    public static boolean requestPrintPermission() {
	return _apiDialog.askUser(ResourceManager.getString("api.print.title"),
                                ResourceManager.getString("api.print.message"),
                                ResourceManager.getString("api.print.always"));
    }
}

