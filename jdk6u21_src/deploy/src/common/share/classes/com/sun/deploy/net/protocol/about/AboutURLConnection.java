/*
 * @(#)AboutURLConnection.java	1.2 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net.protocol.about;

import java.io.IOException;
import java.net.URL;
import java.net.ProtocolException;
import java.net.InetAddress;
import java.net.UnknownHostException;


public final class AboutURLConnection extends java.net.URLConnection
{
    URL url = null;

    /**
     * Construct an AboutURLConnection object.
     */
    public AboutURLConnection(URL u) throws IOException 
    {
	super(u);
	url = u;
    }


    public void connect() throws IOException 
    {
	// Cannot connect to an about: URL
	throw new IOException("Cannot connect to " + url);
    }
}
