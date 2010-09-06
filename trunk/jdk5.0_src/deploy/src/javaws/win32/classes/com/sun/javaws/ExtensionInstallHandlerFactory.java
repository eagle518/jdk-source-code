/*
 * @(#)ExtensionInstallHandlerFactory.java	1.4 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws;

/**
 * Returns the Windows specific install handler.
 *
 * @version 1.5 02/14/01
 */
public class ExtensionInstallHandlerFactory {
    public static ExtensionInstallHandler newInstance() {
        return new WinExtensionInstallHandler();
    }
}
