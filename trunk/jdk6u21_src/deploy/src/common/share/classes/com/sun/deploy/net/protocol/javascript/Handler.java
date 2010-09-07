/*
 * @(#)Handler.java	1.10 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/**
 *	JavaScript stream opener,
 */

package com.sun.deploy.net.protocol.javascript;

import java.io.IOException;
import java.net.URL;


/**
 * Open an javascript input stream given a URL 
 */
public class Handler extends java.net.URLStreamHandler {

    /*
     * <p>
     * We use our protocol handler for JDK 1.2 to open the connection for 
     * the specified URL
     * </p>
     * 
     * @param URL the url to open
     */
    protected java.net.URLConnection openConnection(URL u) throws IOException {
        return new JavaScriptURLConnection(u);
    }
}
