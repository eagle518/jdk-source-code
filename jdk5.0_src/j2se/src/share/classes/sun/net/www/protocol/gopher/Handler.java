/*
 * @(#)Handler.java	1.22 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.net.www.protocol.gopher;

import java.io.*;
import java.util.*;
import sun.net.NetworkClient;
import java.net.URL;
import java.net.URLStreamHandler;
import java.net.Proxy;
import java.net.InetSocketAddress;
import java.net.SocketPermission;
import java.security.Permission;
import sun.net.www.protocol.http.HttpURLConnection;

/**
 * A class to handle the gopher protocol.
 */

public class Handler extends java.net.URLStreamHandler {

    protected int getDefaultPort() {
        return 70;
    }

    public java.net.URLConnection openConnection(URL u) 
    throws IOException {
	return openConnection(u, null);
    }

    public java.net.URLConnection openConnection(URL u, Proxy p) 
    throws IOException {
	

	/* if set for proxy usage then go through the http code to get */
	/* the url connection. */
	if (p == null && GopherClient.getUseGopherProxy()) {
	    String host = GopherClient.getGopherProxyHost();
	    if (host != null) {
		InetSocketAddress saddr = InetSocketAddress.createUnresolved(host, GopherClient.getGopherProxyPort());
		
		p = new Proxy(Proxy.Type.HTTP, saddr);
	    }
	}
	if (p != null) {
	    return new HttpURLConnection(u, p);
	}
	
	return new GopherURLConnection(u);
    }
}

class GopherURLConnection extends sun.net.www.URLConnection {

    Permission permission;
  
    GopherURLConnection(URL u) {
	super(u);
    }

    public void connect() throws IOException {
    }

    public InputStream getInputStream() throws IOException {
	return new GopherClient(this).openStream(url);
    }

    public Permission getPermission() {
	if (permission == null) {
	    int port = url.getPort();
	    port = port < 0 ? 70 : port;
	    String host = url.getHost() + ":" + url.getPort();
	    permission = new SocketPermission(host, "connect");
	}
	return permission;
    }
}

