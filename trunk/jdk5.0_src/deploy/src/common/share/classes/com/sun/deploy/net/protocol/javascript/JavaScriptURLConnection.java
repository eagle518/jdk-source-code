/*
 * @(#)JavaScriptURLConnection.java	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net.protocol.javascript;

import java.io.IOException;
import java.net.URL;
import java.net.ProtocolException;
import java.net.InetAddress;
import java.net.UnknownHostException;


/**
 * A class to represent an JavaScript connection.
 *
 * @author  Stanley Man-Kit Ho
 */

public final class JavaScriptURLConnection extends java.net.URLConnection
{
    URL url = null;

    /**
     * Construct a JavaScriptURLConnection object.
     */
    public JavaScriptURLConnection(URL u) throws IOException 
    {
	super(u);
	url = u;
    }


    public void connect() throws IOException 
    {
	// Cannot connect to a JavaScript
	throw new IOException("Cannot connect to " + url);
    }
}


