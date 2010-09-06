/*
 * @(#)ClipboardServiceImpl.java	1.16 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.jnlp;
import javax.jnlp.ClipboardService;
import java.awt.datatransfer.Transferable;
import java.awt.datatransfer.DataFlavor;
import java.awt.datatransfer.Clipboard;
import java.awt.Toolkit;
import java.security.AccessController;
import java.security.PrivilegedAction;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.util.Trace;
import java.io.IOException;
import java.awt.datatransfer.UnsupportedFlavorException;

public final class ClipboardServiceImpl implements ClipboardService {
    static private ClipboardServiceImpl _sharedInstance = null;
    
    private Clipboard 		_sysClipboard = null;
    private SmartSecurityDialog _readDialog   = null;
    private SmartSecurityDialog _writeDialog  = null;
    
    private ClipboardServiceImpl() {
	// Create dialog instances
	_readDialog  = new SmartSecurityDialog(ResourceManager.getString("APIImpl.clipboard.message.read"));
	_writeDialog = new SmartSecurityDialog(ResourceManager.getString("APIImpl.clipboard.message.write"));
	
	// To eliminate a redundant security dialog based on bug 4432613,
	// we no longer ask the user here if they approve of read access.
	// sysClipboard is a private variable so this is safe, we make
	// sure to ask the user in getContents and setContents.  So, this
	// is still secure.
	final Toolkit toolkit = Toolkit.getDefaultToolkit();
	if (toolkit != null) {
	    _sysClipboard = toolkit.getSystemClipboard();
	}
    }
    
    public static synchronized ClipboardServiceImpl getInstance() {
	if (_sharedInstance == null) {
	    _sharedInstance = new ClipboardServiceImpl();
	}
	return _sharedInstance;
    }
    
    public Transferable getContents() {
	if (!askUser(false)) return null;
	
	return (Transferable)AccessController.doPrivileged(new PrivilegedAction() {
		    public Object run() {
			return _sysClipboard.getContents(null);
		    }
		});
    }
    
    public void setContents(final Transferable contents) {
	if (!askUser(true)) return;
	
	AccessController.doPrivileged(new PrivilegedAction() {
		    public Object run() {
			if (contents != null) {
			    DataFlavor [] flavors = contents.getTransferDataFlavors();
			    if (flavors == null || flavors[0] == null) return null;
			    try {
				if (contents.getTransferData(flavors[0]) == null) return null;
			    } catch (IOException ioe) {
				Trace.ignoredException(ioe);
			    } catch (UnsupportedFlavorException ufe) {
				Trace.ignoredException(ufe);
			    }
			}
			_sysClipboard.setContents(contents, null);
			return null;
		    }
		});
    }
    
    private synchronized boolean askUser(final boolean isWrite) {
	if (!hasClipboard()) return false; // No clipboard - same as no access
	// If application already has permissions, just continue
	if (CheckServicePermission.hasClipboardPermissions()) return true;
	// Show dialog
	return (isWrite) ? _writeDialog.showDialog() : _readDialog.showDialog();
    }
    
    private boolean hasClipboard() { return _sysClipboard != null; }
}

