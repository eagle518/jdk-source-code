/*
 * @(#)BrowserSupportFactory.java	1.8 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws;
import java.net.URL;

/**
 * Create an instance of WinBrowserSupport
 *
 * @version 1.3, 08/29/00
 */
public class BrowserSupportFactory {
    public static BrowserSupport newInstance() {
	return new WinBrowserSupport();
    }
}


