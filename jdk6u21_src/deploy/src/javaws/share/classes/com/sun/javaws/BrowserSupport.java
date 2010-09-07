/*
 * %W% %E%
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws;

import java.net.URL;

/**
 * The class encapsulates the methods for interacting
 * with a browser on the native platform.
 * Currently, this means getting it to show a specific URL
 *
 */

public abstract class BrowserSupport {

    private static BrowserSupport _browserSupportImplementation = null;

    public synchronized static BrowserSupport getInstance() {
        if (_browserSupportImplementation  == null) {
	    // Platform-depenendent Browser Lookup
	    _browserSupportImplementation  = BrowserSupportFactory.newInstance();
        }
        return _browserSupportImplementation;
    }

    /** All our current supported platforms supports web-browsers */
    static public boolean isWebBrowserSupported() {
	return getInstance().isWebBrowserSupportedImpl();
    }

    /** Instructs the native browser to show a specific URL
     *
     * @return If the operation succeeded
     */
    static public boolean showDocument(URL url) {
	return getInstance().showDocumentImpl(url);
    }

    /** Platform dependent */
    public abstract boolean isWebBrowserSupportedImpl();
    public abstract boolean showDocumentImpl(URL url);
    public abstract String getNS6MailCapInfo();

    /**
     Gets an <code>OperaSupport</code> object for use on the current platform.

     @return An <code>OperaSupport</code> object for use on the current
             platform.
     */
    public abstract OperaSupport getOperaSupport();
}


