/*
 * @(#)JavaTrayIcon.java	1.3 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.ui;
import java.security.*;

public abstract class JavaTrayIcon {
    private static JavaTrayIcon soleInstance;
    protected JavaTrayIconController controller;

    public static void install(JavaTrayIconController controller) {
        if (isSupported()) {
            if (soleInstance != null) {
                throw new IllegalStateException("Already installed");
            }

            // Note assumption that this is currently Windows only
            JavaTrayIcon icon = new WindowsJavaTrayIcon(controller);
            if (icon.isEnabled()) {
                icon.installImpl();
            }
            soleInstance = icon;
        }
    }

    /** Notifies the JavaTrayIcon that the Java Console was closed by
        an outside means. */
    public static void notifyConsoleClosed() {
        if (soleInstance != null) {
            soleInstance.notifyConsoleClosedImpl();
        }
    }

    protected JavaTrayIcon(JavaTrayIconController controller) {
        this.controller = controller;
    }

    protected abstract boolean isEnabled();
    protected abstract void    installImpl();
    protected abstract void    notifyConsoleClosedImpl();

    private static boolean isSupported() {
        // For now, only supported on Windows
        // Could use Java SystemTray API (assuming it's implemented on
        // non-Windows platforms) for better portability in the future
        String osName = ((String) AccessController.doPrivileged(new PrivilegedAction() {
                public Object run() {
                    return System.getProperty("os.name");
                }
            })).toLowerCase();
        return osName.startsWith("windows");
    }
}
