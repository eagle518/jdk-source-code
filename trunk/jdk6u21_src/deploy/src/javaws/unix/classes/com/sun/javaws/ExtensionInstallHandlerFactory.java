/*
 * @(#)ExtensionInstallHandlerFactory.java	1.6 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws;

/**
 * ExtensionInstallHandlerFactory for Solaris. Returns null as Solaris does not
 * yet support an install.
 *
 * @version 1.0 04/07/02
 */
public class ExtensionInstallHandlerFactory {
    /**
     * Returns null, Solaris does not support an installer yet.
     */
    public static ExtensionInstallHandler newInstance() {
        return null;
    }
}
