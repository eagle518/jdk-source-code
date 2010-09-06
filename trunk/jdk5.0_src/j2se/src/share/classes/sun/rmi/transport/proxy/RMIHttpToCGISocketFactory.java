/*
 * @(#)RMIHttpToCGISocketFactory.java	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.rmi.transport.proxy;

import java.io.IOException;
import java.net.Socket;
import java.net.ServerSocket;
import java.net.URL;
import java.rmi.server.RMISocketFactory;

/**
 * RMIHttpToCGISocketFactory creates a socket connection to the
 * specified host that is comminicated within an HTTP request,
 * forwarded through the default firewall proxy, to the target host's
 * normal HTTP server, to a CGI program which forwards the request to
 * the actual specified port on the socket.
 */
public class RMIHttpToCGISocketFactory extends RMISocketFactory {

    public Socket createSocket(String host, int port)
	throws IOException
    {
	return new HttpSendSocket(host, port,
	                          new URL("http", host,
	                                  "/cgi-bin/java-rmi.cgi" +
	                                  "?forward=" + port));
    }

    public ServerSocket createServerSocket(int port) throws IOException
    {
	return new HttpAwareServerSocket(port);
    }
}
