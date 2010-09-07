/*
 * @(#)JavaTrayIconController.java	1.2 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.ui;

/** Provides control over the Java Console to the JavaTrayIcon. */

public interface JavaTrayIconController {
    public boolean isJavaConsoleVisible();
    public void    showJavaConsole(boolean visible);
}
