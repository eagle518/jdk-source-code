/*
 * @(#)LocalInstallHandlerFactory.java	1.8 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws;

/**
 * Returns the Windows specific install handler.
 *
 * @version 1.8 12/19/03
 */
public class LocalInstallHandlerFactory {
    public static LocalInstallHandler newInstance() {
        return new WinInstallHandler();
    }
}
