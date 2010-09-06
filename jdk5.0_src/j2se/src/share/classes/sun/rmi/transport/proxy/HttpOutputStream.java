/*
 * @(#)HttpOutputStream.java	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.rmi.transport.proxy;

import java.io.*;

/**
 * The HttpOutputStream class assists the HttpSendSocket and HttpReceiveSocket
 * classes by providing an output stream that buffers its entire input until
 * closed, and then it sends the complete transmission prefixed by the end of
 * an HTTP header that specifies the content length.
 */
class HttpOutputStream extends ByteArrayOutputStream {

    /** the output stream to send response to */
    protected OutputStream out;

    /** true if HTTP response has been sent */
    boolean responseSent = false;

    /**
     * Begin buffering new HTTP response to be sent to a given stream.
     * @param out the OutputStream to send response to
     */
    public HttpOutputStream(OutputStream out) {
	super();
	this.out = out;
    }

    /**
     * On close, send HTTP-packaged response.
     */
    public synchronized void close() throws IOException {
	if (!responseSent) {
	    /*
	     * If response would have zero content length, then make it
	     * have some arbitrary data so that certain clients will not
	     * fail because the "document contains no data".
	     */
	    if (size() == 0)
		write(emptyData);

	    DataOutputStream dos = new DataOutputStream(out);
	    dos.writeBytes("Content-type: application/octet-stream\r\n");
	    dos.writeBytes("Content-length: " + size() + "\r\n");
	    dos.writeBytes("\r\n");
	    writeTo(dos);
	    dos.flush();
	    // Do not close the underlying stream here, because that would
	    // close the underlying socket and prevent reading a response.
	    reset(); // reset byte array
	    responseSent = true;
	}
    }

    /** data to send if the response would otherwise be empty */
    private static byte[] emptyData = { 0 };
}
