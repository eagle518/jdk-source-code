/*
 * @(#)ExtensionInstallHandlerFactory.java	1.2 10/03/24
 * 
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws;

/**
 * ExtensionInstallHandlerFactory for Mac OS X. Returns null as Mac OS
 * X does not yet support an install.
 */
public class ExtensionInstallHandlerFactory {
    /**
     * Returns null, Mac OS X does not support an installer yet.
     */
    public static ExtensionInstallHandler newInstance() {
        return null;
    }
}
