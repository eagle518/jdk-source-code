/*
 * @(#)FileURLMapper.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.misc;

import java.net.URL;
import java.io.File;
import sun.net.www.ParseUtil;

/**
 * (Solaris) platform specific handling for file: URLs . 
 * urls must not contain a hostname in the authority field
 * other than "localhost". 
 *
 * This implementation could be updated to map such URLs
 * on to /net/host/...
 *
 * @author	Michael McMahon
 * @version 	1.4, 03/12/19
 */

public class FileURLMapper {

    URL url;
    String path;

    public FileURLMapper (URL url) {
	this.url = url;
    }

    /**
     * @returns the platform specific path corresponding to the URL
     *  so long as the URL does not contain a hostname in the authority field.
     */

    public String getPath () {
	if (path != null) {
	    return path;
	}
	String host = url.getHost();
	if (host == null || "".equals(host) || "localhost".equalsIgnoreCase (host)) {
	    path = url.getFile();
	    path = ParseUtil.decode (path);
	} 
	return path;
    }

    /**
     * Checks whether the file identified by the URL exists.
     */
    public boolean exists () {
	String s = getPath ();
	if (s == null) {
	    return false;
	} else {
	    File f = new File (s);
	    return f.exists();
	}
    }
}
