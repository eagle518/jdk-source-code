/*
 * @(#)LocalInstallHandlerFactory.java	1.9 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws;

/**
 * LocalInstallHandlerFactory for Unix. 
 *
 * @version 1.9 12/19/03
 */

public class LocalInstallHandlerFactory {
    /**
     * Returns null, Solaris does not support an installer yet.
     */
    public static LocalInstallHandler newInstance() {
        return new UnixInstallHandler();
    }
}
