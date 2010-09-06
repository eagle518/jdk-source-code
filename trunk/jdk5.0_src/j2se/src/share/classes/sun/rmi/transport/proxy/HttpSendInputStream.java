/*
 * @(#)HttpSendInputStream.java	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.rmi.transport.proxy;

import java.io.*;

/**
 * The HttpSendInputStream class is used by the HttpSendSocket class as
 * a layer on the top of the InputStream it returns so that it can be
 * notified of attempts to read from it.  This allows the HttpSendSocket
 * to know when it should push across its output message.
 */
class HttpSendInputStream extends FilterInputStream {

    /** the HttpSendSocket object that is providing this stream */
    HttpSendSocket owner;

    /**
     * Create new filter on a given input stream.
     * @param in the InputStream to filter from
     * @param owner the HttpSendSocket that is providing this stream
     */
    public HttpSendInputStream(InputStream in, HttpSendSocket owner)
	throws IOException
    {
	super(in);

	this.owner = owner;
    }

    /**
     * Mark this stream as inactive for its owner socket, so the next time
     * a read is attempted, the owner will be notified and a new underlying
     * input stream obtained.
     */
    public void deactivate()
    {
	in = null;
    }

    /**
     * Read a byte of data from the stream.
     */
    public int read() throws IOException
    {
	if (in == null)
	    in = owner.readNotify();
	return in.read();
    }

    /**
     * Read into an array of bytes.
     * @param b the buffer into which the data is to be read
     * @param off the start offset of the data
     * @param len the maximum number of bytes to read
     */
    public int read(byte b[], int off, int len) throws IOException
    {
	if (len == 0)
	    return 0;
	if (in == null)
	    in = owner.readNotify();
	return in.read(b, off, len);
    }

    /**
     * Skip bytes of input.
     * @param n the number of bytes to be skipped
     */
    public long skip(long n) throws IOException
    {
	if (n == 0)
	    return 0;
	if (in == null)
	    in = owner.readNotify();
	return in.skip(n);
    }

    /**
     * Return the number of bytes that can be read without blocking.
     */
    public int available() throws IOException
    {
	if (in == null)
	    in = owner.readNotify();
	return in.available();
    }

    /**
     * Close the stream.
     */
    public void close() throws IOException
    {
	owner.close();
    }

    /**
     * Mark the current position in the stream.
     * @param readlimit how many bytes can be read before mark becomes invalid
     */
    public synchronized void mark(int readlimit)
    {
	if (in == null) {
	    try {
		in = owner.readNotify();
	    }
	    catch (IOException e) {
		return;
	    }
	}
	in.mark(readlimit);
    }

    /**
     * Reposition the stream to the last marked position.
     */
    public synchronized void reset() throws IOException
    {
	if (in == null)
	    in = owner.readNotify();
	in.reset();
    }

    /**
     * Return true if this stream type supports mark/reset.
     */
    public boolean markSupported()
    {
	if (in == null) {
	    try {
		in = owner.readNotify();
	    }
	    catch (IOException e) {
		return false;
	    }
	}
	return in.markSupported();
    }
}
