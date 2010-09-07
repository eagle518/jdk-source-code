/*
 * @(#)ExtensionInstallHandlerFactory.java	1.6 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
