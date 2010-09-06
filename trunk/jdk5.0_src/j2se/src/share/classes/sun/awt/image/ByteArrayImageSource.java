/*
 * @(#)ByteArrayImageSource.java	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.image;

import java.io.InputStream;
import java.io.ByteArrayInputStream;
import java.io.BufferedInputStream;

public class ByteArrayImageSource extends InputStreamImageSource {
    byte[] imagedata;
    int imageoffset;
    int imagelength;

    public ByteArrayImageSource(byte[] data) {
	this(data, 0, data.length);
    }

    public ByteArrayImageSource(byte[] data, int offset, int length) {
	imagedata = data;
	imageoffset = offset;
	imagelength = length;
    }

    final boolean checkSecurity(Object context, boolean quiet) {
	// No need to check security.  Applets and downloaded code can
	// only make byte array image once they already have a handle
	// on the image data anyway...
	return true;
    }

    protected ImageDecoder getDecoder() {
	InputStream is =
	    new BufferedInputStream(new ByteArrayInputStream(imagedata,
							     imageoffset,
							     imagelength));
	return getDecoder(is);
    }
}
