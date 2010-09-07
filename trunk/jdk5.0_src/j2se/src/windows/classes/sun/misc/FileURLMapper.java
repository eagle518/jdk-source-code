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
 * (Windows) Platform specific handling for file: URLs . In particular deals
 * with network paths mapping them to UNCs.
 *
 * @author	Michael McMahon
 * @version 	1.4, 03/12/19
 */

public class FileURLMapper {

    URL url;
    String file;

    public FileURLMapper (URL url) {
	this.url = url;
    }

    /**
     * @returns the platform specific path corresponding to the URL, and in particular
     *  returns a UNC when the authority contains a hostname
     */

    public String getPath () {
	if (file != null) {
	    return file;
	}
    	String host = url.getHost();
    	if (host != null && !host.equals("") &&
	    !"localhost".equalsIgnoreCase(host)) {
	    String rest = url.getFile();
	    String s = host + ParseUtil.decode (url.getFile());
	    file = "\\\\"+ s.replace('/', '\\');
	    return file;
	}
	String path = url.getFile().replace('/', '\\');
	file = ParseUtil.decode(path);
	return file;
    }

    public boolean exists() {
	String path = getPath();
	File f = new File (path);
	return f.exists();
    }
}
