/*
 * @(#)WinExtensionInstallHandler.java	1.8 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws;

import com.sun.javaws.cache.*;
import com.sun.javaws.jnl.*;
import com.sun.javaws.exceptions.JNLPException;
import com.sun.deploy.util.WinRegistry;
import java.awt.*;
import java.awt.event.*;
import java.awt.image.*;
import java.io.*;
import java.net.*;
import javax.swing.*;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.util.DialogFactory;

/**
 * Instances of ExtensionInstallHandler are used to handle platform-specific
 * installing/uninstalling actions Extension Installers.  The instance to use
 * for installing can be located via the class method <code>getInstance</code>. A
 * null return value from <code>getInstance</code> indicates the
 * current platform does not support any install options.
 * <p>
 * @version 1.0 04/07/02
 */
public class WinExtensionInstallHandler extends ExtensionInstallHandler {
    private static final String KEY_RUNONCE =
	"Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce";
    
    static {
	NativeLibrary.getInstance().load();
    }
    
    public boolean doPreRebootActions(Window owner) {
	// Ask user if they want to reboot
	int result[] = { DialogFactory.NO_OPTION };
	owner.setVisible(true);
	owner.requestFocus();
	result[0] = DialogFactory.showConfirmDialog (owner,
	    ResourceManager.getString("extensionInstall.rebootMessage"),
	    ResourceManager.getString("extensionInstall.rebootTitle"));
	owner.setVisible(false);
					  
	return (result[0] == DialogFactory.YES_OPTION);
    }
    
    public boolean doReboot() {
	return WinRegistry.doReboot();
    }
}
