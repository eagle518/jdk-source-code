/*
 * @(#)Handler.java	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
