/*
 * @(#)LocalInstallHandlerFactory.java	1.10 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws;

/**
 * Returns the Windows specific install handler.
 *
 * @version 1.10 03/24/10
 */
public class LocalInstallHandlerFactory {
    public static LocalInstallHandler newInstance() {
        return new WinInstallHandler();
    }
}
