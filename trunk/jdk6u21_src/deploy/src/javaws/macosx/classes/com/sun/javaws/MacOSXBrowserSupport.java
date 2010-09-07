/*
 * @(#)MacOSXBrowserSupport.java	1.2 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws;
import java.net.URL;
import com.sun.deploy.config.Config;
import java.io.*;

/** Concrete implementation of the BrowserSupport class for Mac OS X */
public class MacOSXBrowserSupport extends BrowserSupport  {
    // FIXME: Apple has certainly ported this before

    public boolean isWebBrowserSupportedImpl() { return false; }
    public boolean showDocumentImpl(URL url) { return false; }
    public String getNS6MailCapInfo() { return null; }
    public OperaSupport getOperaSupport() { return null; }
}
