/*
 * @(#)Resource.java	1.15 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.misc;

import java.net.URL;
import java.io.IOException;
import java.io.InputStream;
import java.security.CodeSigner;
import java.util.jar.Manifest;
import java.util.jar.Attributes;
import java.nio.ByteBuffer;
import sun.nio.ByteBuffered;

/**
 * This class is used to represent a Resource that has been loaded
 * from the class path.
 *
 * @author  David Connelly
 * @version 1.15, 12/19/03
 * @since   JDK1.2
 */
public abstract class Resource {
    /**
     * Returns the name of the Resource.
     */
    public abstract String getName();

    /**
     * Returns the URL of the Resource.
     */
    public abstract URL getURL();

    /**
     * Returns the CodeSource URL for the Resource.
     */
    public abstract URL getCodeSourceURL();

    /**
     * Returns an InputStream for reading the Resource data.
     */
    public abstract InputStream getInputStream() throws IOException;

    /**
     * Returns the length of the Resource data, or -1 if unknown.
     */
    public abstract int getContentLength() throws IOException;

    private InputStream cis;

    /* Cache result in case getBytes is called after getByteBuffer. */
    private synchronized InputStream cachedInputStream() throws IOException {
	if (cis == null) {
	    cis = getInputStream();
	}
	return cis;
    }

    /**
     * Returns the Resource data as an array of bytes.
     */
    public byte[] getBytes() throws IOException {
	byte[] b;
        // Get stream before content length so that a FileNotFoundException
        // can propagate upwards without being caught too early
	InputStream in = cachedInputStream();
	int len = getContentLength();
	try {
	    if (len != -1) {
		// Read exactly len bytes from the input stream
		b = new byte[len];
		while (len > 0) {
		    int n = in.read(b, b.length - len, len);
		    if (n == -1) {
			throw new IOException("unexpected EOF");
		    }
		    len -= n;
		}
	    } else {
		// Read until end of stream is reached
		b = new byte[1024];
		int total = 0;
		while ((len = in.read(b, total, b.length - total)) != -1) {
		    total += len;
		    if (total >= b.length) {
			byte[] tmp = new byte[total * 2];
			System.arraycopy(b, 0, tmp, 0, total);
			b = tmp;
		    }
		}
		// Trim array to correct size, if necessary
		if (total != b.length) {
		    byte[] tmp = new byte[total];
		    System.arraycopy(b, 0, tmp, 0, total);
		    b = tmp;
		}
	    }
	} finally {
	    in.close();
	}
	return b;
    }

    /**
     * Returns the Resource data as a ByteBuffer, but only if the input stream
     * was implemented on top of a ByteBuffer. Return <tt>null</tt> otherwise.
     */
    public ByteBuffer getByteBuffer() throws IOException {
	InputStream in = cachedInputStream();
	if (in instanceof ByteBuffered) {
	    return ((ByteBuffered)in).getByteBuffer();
	}
	return null;
    }
	
    /**
     * Returns the Manifest for the Resource, or null if none.
     */
    public Manifest getManifest() throws IOException {
	return null;
    }

    /**
     * Returns theCertificates for the Resource, or null if none.
     */
    public java.security.cert.Certificate[] getCertificates() {
	return null;
    }

    /**
     * Returns the code signers for the Resource, or null if none.
     */
    public CodeSigner[] getCodeSigners() {
	return null;
    }
}
