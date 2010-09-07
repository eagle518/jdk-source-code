/*
 * @(#)ChromeURLConnection.java	1.2 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net.protocol.chrome;

import java.io.IOException;
import java.net.URL;
import java.net.ProtocolException;
import java.net.InetAddress;
import java.net.UnknownHostException;


public final class ChromeURLConnection extends java.net.URLConnection
{
    URL url = null;

    /**
     * Construct a ChromeURLConnection object.
     */
    public ChromeURLConnection(URL u) throws IOException 
    {
	super(u);
	url = u;
    }


    public void connect() throws IOException 
    {
	// Cannot connect to a chrome:// URL (yet)
	throw new IOException("Cannot connect to " + url);
    }
}
