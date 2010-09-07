/*
 * @(#)ProxyUtils.java	1.15 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net.proxy;

import java.net.MalformedURLException;
import java.net.URL;
import java.util.List;
import java.util.StringTokenizer;
import com.sun.deploy.net.protocol.ProtocolType;
import com.sun.deploy.util.Trace;


/**
 * Utilities for proxy configuration.
 */
public final class ProxyUtils 
{
    /**
     * Parse the "host:port" format address string.  Not that the ":port"
     * part of the address is optional in the sense that the IE preferences
     * dialog will not gripe if it's not provided.
     */
    private static void parseProxyAddress(int protocolType, String adString, BrowserProxyInfo info) 
    {
        final int DEFAULT_HTTP_PORT   = 80,
                  DEFAULT_SOCKS_PORT  = 1080;

	URL u;
	String addressString = null;

	// Make sure the address string is in proper URL form
	try {
	    u = new URL(adString);
	    addressString = new String(u.getHost() + ":" + u.getPort());
	} catch (MalformedURLException mue) {
	    addressString = new String(adString);
	}

	StringTokenizer st = new StringTokenizer(addressString, ":");

	if (!st.hasMoreTokens())
	    return;

	String host = st.nextToken();
	int port = -1;

	if (st.hasMoreTokens()) 
	{
	    try {
		port = Integer.parseInt(st.nextToken());
	    }
	    catch (NumberFormatException exc) {
	    }
	}

	switch (protocolType)
	{
	    case ProtocolType.HTTP:
		info.setHttpHost(host);
                if (port == -1) port = DEFAULT_HTTP_PORT;
		info.setHttpPort(port);
		break;
	    case ProtocolType.HTTPS:
		info.setHttpsHost(host);
                // HTTPS proxy servers use HTTP tunneling on the HTTP port
                if (port == -1) port = DEFAULT_HTTP_PORT;
		info.setHttpsPort(port);
		break;
	    case ProtocolType.FTP:
		info.setFtpHost(host);
                // FTP proxy servers use HTTP tunneling on the HTTP port
                if (port == -1) port = DEFAULT_HTTP_PORT;
		info.setFtpPort(port);
		break;
	    case ProtocolType.GOPHER:
		info.setGopherHost(host);
                // GOPHER proxy servers use HTTP tunneling on the HTTP port
                if (port == -1) port = DEFAULT_HTTP_PORT;
		info.setGopherPort(port);
		break;
	    case ProtocolType.SOCKS:
		info.setSocksHost(host);
                if (port == -1) port = DEFAULT_SOCKS_PORT;
		info.setSocksPort(port);
		break;
	    default:
		throw new IllegalStateException("ProxyUtils: ProtocolType not valid");
	}
    }
    
    
    /**
     * The value of the ProxyServer registry entry at
     * <pre>
     *     Software\Microsoft\Windows\CurrentVersion\Internet Settings
     * </pre>
     * is either a ';' separated list of proxy addresses per protocol,
     * like "ftp=host:port;http=host:port" or a single address.  The latter
     * is used when the user selects the "use same address for all proxies
     * checkbox.  In all cases the ":port" part of the address is optional.
     */
    public static void parseProxyServer(String server, BrowserProxyInfo info) 
    {
	if (server.indexOf("=") != -1) 
	{
	    // Break down protocol support
	    StringTokenizer st = new StringTokenizer(server, ";");

	    while(st.hasMoreTokens()) 
	    {
		String s = st.nextToken();

		if (s.startsWith("http=")) 
		    parseProxyAddress(ProtocolType.HTTP, s.substring(5, s.length()), info);
		else if (s.startsWith("https=")) 
		    parseProxyAddress(ProtocolType.HTTPS, s.substring(6, s.length()), info);
		else if (s.startsWith("ftp=")) 
		    parseProxyAddress(ProtocolType.FTP, s.substring(4, s.length()), info);
		else if (s.startsWith("gopher=")) 
		    parseProxyAddress(ProtocolType.GOPHER, s.substring(7, s.length()), info);
		else if (s.startsWith("socks=")) 
		    parseProxyAddress(ProtocolType.SOCKS, s.substring(6, s.length()), info);
	    }
	}
	else 
	{
	    // The proxy address is specified for all proxies
	    //
	    parseProxyAddress(ProtocolType.HTTP, server, info);
	    parseProxyAddress(ProtocolType.HTTPS, server, info);
	    parseProxyAddress(ProtocolType.FTP, server, info);
	    parseProxyAddress(ProtocolType.GOPHER, server, info);
	}
    }
}



