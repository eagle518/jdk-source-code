/*
 * @(#)FileImageSource.java	1.19 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.image;

import java.io.InputStream;
import java.io.FileInputStream;
import java.io.BufferedInputStream;
import java.io.FileNotFoundException;

public class FileImageSource extends InputStreamImageSource {
    String imagefile;

    public FileImageSource(String filename) {
	SecurityManager security = System.getSecurityManager();
	if (security != null) {
	    security.checkRead(filename);
	}
	imagefile = filename;
    }

    final boolean checkSecurity(Object context, boolean quiet) {
	// File based images only ever need to be checked statically
	// when the image is retrieved from the cache.
	return true;
    }

    protected ImageDecoder getDecoder() {
	InputStream is;
	try {
	    is = new BufferedInputStream(new FileInputStream(imagefile));
	} catch (FileNotFoundException e) {
	    return null;
	}
	// Don't believe the file suffix - many users don't know what
	// kind of image they have and guess wrong...
	/*
	int suffixpos = imagefile.lastIndexOf('.');
	if (suffixpos >= 0) {
	    String suffix = imagefile.substring(suffixpos+1).toLowerCase();
	    if (suffix.equals("gif")) {
		return new GifImageDecoder(this, is);
	    } else if (suffix.equals("jpeg") || suffix.equals("jpg") ||
		       suffix.equals("jpe") || suffix.equals("jfif")) {
		return new JPEGImageDecoder(this, is);
	    } else if (suffix.equals("xbm")) {
		return new XbmImageDecoder(this, is);
	    }
	}
	*/
	return getDecoder(is);
    }
}
