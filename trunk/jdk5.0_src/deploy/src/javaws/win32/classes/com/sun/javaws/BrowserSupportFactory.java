/*
 * @(#)BrowserSupportFactory.java	1.6 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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


