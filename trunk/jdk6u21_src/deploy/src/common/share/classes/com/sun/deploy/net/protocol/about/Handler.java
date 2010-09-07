/*
 * @(#)Handler.java	1.2 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/**
 *  Dummy handler to handle "about:blank" URLs
 */

package com.sun.deploy.net.protocol.about;

import java.io.IOException;
import java.net.URL;


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
        return new AboutURLConnection(u);
    }
}
