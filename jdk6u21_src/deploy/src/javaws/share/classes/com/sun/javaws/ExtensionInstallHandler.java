/*
 * @(#)ExtensionInstallHandler.java	1.9 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws;

import java.awt.Component;
import com.sun.javaws.jnl.*;
import java.security.*;
import java.util.*;

/**
 * Instances of ExtensionInstallHandler are used to handle platform-specific
 * installing/uninstalling actions Extension Installers.  The instance to use
 * for installing can be located via the class method <code>getInstance</code>. A
 * null return value from <code>getInstance</code> indicates the
 * current platform does not support any install options.
 * <p>
 * @version 1.0 04/07/02
 */
public abstract class ExtensionInstallHandler {
    /** The shared instance of ExtensionInstallHandler. */
    private static ExtensionInstallHandler _installHandler;

    /**
     * Returns the LocalInstallHandler appropriate for the current platform.
     * This may return null, indicating the platform does not support
     * an installer.
     */
    public static synchronized ExtensionInstallHandler getInstance() {
        if (_installHandler == null) {
            _installHandler = ExtensionInstallHandlerFactory.newInstance();
        }
        return _installHandler;
    }

    /**
     * Perform registration actions necessary before a reboot
     */
    public abstract boolean doPreRebootActions(Component owner);
    
    /**
     * Performs reboot
     */
    public abstract boolean doReboot();
}
