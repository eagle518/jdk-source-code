/*
 * @(#)WinExtensionInstallHandler.java	1.15 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws;

import com.sun.javaws.jnl.*;
import com.sun.javaws.exceptions.JNLPException;
import com.sun.deploy.util.WinRegistry;
import java.awt.*;
import java.awt.event.*;
import java.awt.image.*;
import java.io.*;
import java.net.*;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.ui.UIFactory;

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
    
    public boolean doPreRebootActions(Component owner) {
	// Ask user if they want to reboot
	int result;
	result = UIFactory.showConfirmDialog (owner, null, 
	    ResourceManager.getString("extensionInstall.rebootMessage"),
	    ResourceManager.getString("extensionInstall.rebootTitle"));
					  
	return (result == UIFactory.OK);
    }
    
    public boolean doReboot() {
	return WinRegistry.doReboot();
    }
}
