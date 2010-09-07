/*
 * @(#)BrowserSupportFactory.java	1.2 10/03/24
 * 
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws;
import java.net.URL;

/**
 * Create an instance of BrowserSupport
 */
public class BrowserSupportFactory {
    public static BrowserSupport newInstance() {
	return new MacOSXBrowserSupport();
    }
}
