/*
 * @(#)LocalInstallHandlerFactory.java	1.11 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws;

/**
 * LocalInstallHandlerFactory for Unix. 
 *
 * @version 1.11 03/24/10
 */

public class LocalInstallHandlerFactory {
    /**
     * Returns null, Solaris does not support an installer yet.
     */
    public static LocalInstallHandler newInstance() {
        return new UnixInstallHandler();
    }
}
