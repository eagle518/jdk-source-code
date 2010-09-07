/*
 * @(#)WinBrowserSupport.java	1.17 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws;

import java.net.URL;
import com.sun.deploy.config.Config;


/** Concrete implementation of the BrowserSupport class for Windows */
public class WinBrowserSupport extends BrowserSupport  {

    /** Platform dependent */
    // no mailcap file for NS6 on win32
    public String getNS6MailCapInfo() { return null; }

    /**
     Gets an <code>OperaSupport</code> object for use on Unix/Linux platforms.

     @return An <code>OperaSupport</code> object for use on Unix/Linux
             platforms.
     */
    public OperaSupport getOperaSupport()
    {
        return (new WinOperaSupport(Config.getBooleanProperty(Config.MIME_DEFAULTS_KEY)));
    }

    public boolean isWebBrowserSupportedImpl() { return true; }
    public boolean showDocumentImpl(URL url)   {
	if (url == null) return false;
	return showDocument(url.toString());
    }

    public String getDefaultHandler(URL url) {
	return Config.getInstance().getBrowserPath();
    }

    // Native method that does all the work - now in Config:
    public boolean showDocument(String url) {
	return Config.getInstance().showDocument(url);
    }
}

