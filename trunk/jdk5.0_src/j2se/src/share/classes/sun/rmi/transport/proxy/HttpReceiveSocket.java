/*
 * @(#)HttpReceiveSocket.java	1.16 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.rmi.transport.proxy;

import java.io.*;
import java.net.Socket;
import java.net.InetAddress;

/**
 * The HttpReceiveSocket class extends the WrappedSocket class
 * by removing the HTTP protocol packaging from the input stream and
 * formatting the output stream as an HTTP response.
 *
 * NOTES:
 *
 * The output stream must be explicitly closed for the output to be
 * sent, since the HttpResponseOutputStream needs to buffer the entire
 * transmission to be able to fill in the content-length field of
 * the HTTP header.  Closing this socket will do this.
 *
 * The constructor blocks until the HTTP protocol header
 * is received.  This could be fixed, but I don't think it should be a
 * problem because this object would not be created unless the
 * HttpAwareServerSocket has detected the beginning of the header
 * anyway, so the rest should be there.
 *
 * This socket can only be used to process one POST and reply to it.
 * Another message would be received on a newly accepted socket anyway.
 */
public class HttpReceiveSocket extends WrappedSocket implements RMISocketInfo {

    /** true if the HTTP header has pushed through the output stream yet */
    private boolean headerSent = false;

    /**
     * Layer on top of a pre-existing Socket object, and use specified
     * input and output streams.
     * @param socket the pre-existing socket to use
     * @param in the InputStream to use for this socket (can be null)
     * @param out the OutputStream to use for this socket (can be null)
     */
    public HttpReceiveSocket(Socket socket, InputStream in, OutputStream out)
        throws IOException
    {
	super(socket, in, out);

	this.in = new HttpInputStream(in != null ? in :
	                                           socket.getInputStream());
	this.out = (out != null ? out :
	            socket.getOutputStream());
    }

    /**
     * Indicate that this socket is not reusable.
     */
    public boolean isReusable()
    {
	return false;
    }

    /**
     * Get the address to which this socket is connected.  "null" is always
     * returned (to indicate an unknown address) because the originating
     * host's IP address cannot be reliably determined: both because the
     * request probably went through a proxy server, and because if it was
     * delivered by a local forwarder (CGI script or servlet), we do NOT
     * want it to appear as if the call is coming from the local host (in
     * case the remote object makes access control decisions based on the
     * "client host" of a remote call; see bugid 4399040).
     */
    public InetAddress getInetAddress() {
	return null;
    }

    /**
     * Get an OutputStream for this socket.
     */
    public OutputStream getOutputStream() throws IOException
    {
	if (!headerSent) { // could this be done in constructor??
	    DataOutputStream dos = new DataOutputStream(out);
	    dos.writeBytes("HTTP/1.0 200 OK\r\n");
	    dos.flush();
	    headerSent = true;
	    out = new HttpOutputStream(out);
	}
	return out;
    }

    /**
     * Close the socket.
     */
    public synchronized void close() throws IOException
    {
	getOutputStream().close(); // make sure response is sent
	socket.close();
    }

    /**
     * Return string representation of the socket.
     */
    public String toString()
    {
	return "HttpReceive" + socket.toString();
    }
}
