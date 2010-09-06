/*
 * @(#)ChunkedOutputStream.java	1.5 04/07/26
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.net.www.http;

import java.io.*;

/**
 * OutputStream that sends the output to the underlying stream using chunked
 * encoding as specified in RFC 2068. 
 *
 * @author  Alan Bateman
 * @version 1.5 07/26/04
 */
public class ChunkedOutputStream extends PrintStream {

    /* Default chunk size (including chunk header) if not specified */
    static final int DEFAULT_CHUNK_SIZE = 4096;

    /* internal buffer */
    private byte buf[];
    private int count;

    /* underlying stream */
    private PrintStream out;

    /* the chunk size we use */
    private int preferredChunkSize;

    /* return the size of the header for a particular chunk size */
    private int headerSize(int size) {
        return 2 + (Integer.toHexString(size)).length();
    }

    public ChunkedOutputStream(PrintStream o) {
        this(o, DEFAULT_CHUNK_SIZE);
    }

    public ChunkedOutputStream(PrintStream o, int size) {
	super(o);

        out = o;

	if (size <= 0) {
	    size = DEFAULT_CHUNK_SIZE;
	}
        /* Adjust the size to cater for the chunk header - eg: if the 
         * preferred chunk size is 1k this means the chunk size should
         * be 1019 bytes (differs by 5 from preferred size because of
         * 3 bytes for chunk size in hex and CRLF).
         */
        if (size > 0) {
            int adjusted_size = size - headerSize(size);
            if (adjusted_size + headerSize(adjusted_size) < size) {
                adjusted_size++;
            }
            size = adjusted_size;
        }

        if (size > 0) {
            preferredChunkSize = size;
        } else {
            preferredChunkSize = DEFAULT_CHUNK_SIZE - headerSize(DEFAULT_CHUNK_SIZE);
        }

        /* start with an initial buffer */
        buf = new byte[preferredChunkSize + 32];
    }

    /*
     * Flush any buffered data to the underlying stream. If flushAll
     * is true then all buffered data is flushed in a single chunk.
     * If false and the size of the buffer data exceeds the preferred
     * chunk size then chunks are flushed to the output stream until
     * there is sufficient data to make up a complete chunk.
     */
    private void flush(boolean flushAll) {
        int chunkSize;

	int offset = 0;

        do {
            if (count < preferredChunkSize) {
                if (!flushAll) {
                    break;
                }
                chunkSize = count; 
            } else {
                chunkSize = preferredChunkSize;
            }

	    byte[] bytes = (Integer.toHexString(chunkSize)).getBytes();

            out.write(bytes, 0, bytes.length);
            out.write((byte)'\r');
            out.write((byte)'\n');          
            if (chunkSize > 0) {
                out.write(buf, offset, chunkSize);
                out.write((byte)'\r');
                out.write((byte)'\n');
            }
            out.flush();
	    if (checkError()) {
		break;
	    }
            if (chunkSize > 0) {
                count -= chunkSize;
		offset += chunkSize;
            }
        } while (count > 0);
	
	if (!checkError()) {
	    System.arraycopy(buf, offset, buf, 0, count);
	}
    }

    public boolean checkError() {
	return out.checkError();
    }

    /* 
     * Check if we have enough data for a chunk and if so flush to the 
     * underlying output stream.
     */
    private void checkFlush() {
        if (count >= preferredChunkSize) {
            flush(false);
        }
    }

    /* Check that the output stream is still open */
    private void ensureOpen() {
        if (out == null)
            setError();
    }

    public synchronized void write(byte b[], int off, int len) {
        ensureOpen();
        if ((off < 0) || (off > b.length) || (len < 0) ||
            ((off + len) > b.length) || ((off + len) < 0)) {
            throw new IndexOutOfBoundsException();
        } else if (len == 0) {
            return;
        }
        int newcount = count + len;
        if (newcount > buf.length) {
            byte newbuf[] = new byte[Math.max(buf.length << 1, newcount)];
            System.arraycopy(buf, 0, newbuf, 0, count);
            buf = newbuf;
        }
        System.arraycopy(b, off, buf, count, len);
        count = newcount;
        checkFlush();
    }

    public synchronized void write(int b) {
        ensureOpen();
        int newcount = count + 1;
        if (newcount > buf.length) {
            byte newbuf[] = new byte[Math.max(buf.length << 1, newcount)];
            System.arraycopy(buf, 0, newbuf, 0, count);
            buf = newbuf;
        }
        buf[count] = (byte)b;
        count = newcount;
        checkFlush();
    }

    public synchronized void reset() {
        count = 0;
    }

    public int size() {
        return count;
    }

    public synchronized void close() {
        ensureOpen();

        /* if we have buffer a chunked send it */
        if (count > 0) {
            flush(true);
        }

        /* send a zero length chunk */
        flush(true);

        /* don't close the underlying stream */
        out = null;
    }

    public synchronized void flush() {
        ensureOpen();
        if (count > 0) {
            flush(true);
        }
    }

}
