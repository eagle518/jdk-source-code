/*
 * @(#)ChannelInputStream.java	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.nio.ch;

import java.io.*;
import java.nio.*;
import java.nio.channels.*;
import java.nio.channels.spi.*;


/**
 * This class is defined here rather than in java.nio.channels.Channels
 * so that code can be shared with SocketAdaptor.
 *
 * @author Mike McCloskey
 * @author Mark Reinhold
 * @version 1.6, 03/12/19
 * @since 1.4
 */

public class ChannelInputStream
    extends InputStream
{

    public static int read(ReadableByteChannel ch, ByteBuffer bb,
			   boolean block)
	throws IOException
    {
	if (ch instanceof SelectableChannel) {
	    SelectableChannel sc = (SelectableChannel)ch;
	    synchronized (sc.blockingLock()) {
		boolean bm = sc.isBlocking();
		if (!bm)
		    throw new IllegalBlockingModeException();
		if (bm != block)
		    sc.configureBlocking(block);
		int n = ch.read(bb);
		if (bm != block)
		    sc.configureBlocking(bm);
		return n;
	    }
	} else {
	    return ch.read(bb);
	}
    }

    protected final ReadableByteChannel ch;
    private ByteBuffer bb = null;
    private byte[] bs = null;		// Invoker's previous array
    private byte[] b1 = null;

    public ChannelInputStream(ReadableByteChannel ch) {
	this.ch = ch;
    }

    public synchronized int read() throws IOException {
	if (b1 == null)
	    b1 = new byte[1];
	int n = this.read(b1);
	if (n == 1)
	    return b1[0] & 0xff;
	return -1;
    }

    public synchronized int read(byte[] bs, int off, int len)
	throws IOException
    {
	if ((off < 0) || (off > bs.length) || (len < 0) ||
	    ((off + len) > bs.length) || ((off + len) < 0)) {
	    throw new IndexOutOfBoundsException();
	} else if (len == 0)
	    return 0;

	ByteBuffer bb = ((this.bs == bs)
			 ? this.bb
			 : ByteBuffer.wrap(bs));
	bb.position(off);
	bb.limit(Math.min(off + len, bb.capacity()));
	this.bb = bb;
	this.bs = bs;
	return read(bb);
    }

    protected int read(ByteBuffer bb)
	throws IOException
    {
	return ChannelInputStream.read(ch, bb, true);
    }

    public void close() throws IOException {
	ch.close();
    }

}
