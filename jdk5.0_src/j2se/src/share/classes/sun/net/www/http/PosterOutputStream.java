/*
 * @(#)PosterOutputStream.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.net.www.http;

import java.io.*;
import java.net.*;

/**
 * Instances of this class are returned to applications for the purpose of
 * sending user data for a HTTP POST or PUT request. This class is used
 * when the content-length will be specified in the header of the request.
 * The semantics of ByteArrayOutputStream are extended so that 
 * when close() is called, it is no longer possible to write 
 * additional data to the stream. From this point the content length of 
 * the request is fixed and cannot change.
 *
 * @version 1.4, 12/19/03
 * @author Michael McMahon
 */

public class PosterOutputStream extends ByteArrayOutputStream {

    private boolean closed;

    /**
     * Creates a new output stream for POST user data
     */
    public PosterOutputStream () {
	super (256);
    }

    /**
     * Writes the specified byte to this output stream.
     *
     * @param   b   the byte to be written.
     */
    public synchronized void write(int b) {
	if (closed) {
	    return;
	}
	super.write (b);
    }

    /**
     * Writes <code>len</code> bytes from the specified byte array
     * starting at offset <code>off</code> to this output stream.
     *
     * @param   b     the data.
     * @param   off   the start offset in the data.
     * @param   len   the number of bytes to write.
     */
    public synchronized void write(byte b[], int off, int len) {
	if (closed) {
	    return;
	}
	super.write (b, off, len);
    }

    /**
     * Resets the <code>count</code> field of this output
     * stream to zero, so that all currently accumulated output in the
     * ouput stream is discarded. The output stream can be used again,
     * reusing the already allocated buffer space. If the output stream
     * has been closed, then this method has no effect.
     *
     * @see     java.io.ByteArrayInputStream#count
     */
    public synchronized void reset() {
	if (closed) {
	    return;
	}
	super.reset ();
    }

    /**
     * After close() has been called, it is no longer possible to write
     * to this stream. Further calls to write will have no effect.
     */
    public synchronized void close() throws IOException {
	closed = true;
	super.close ();
    }
}
