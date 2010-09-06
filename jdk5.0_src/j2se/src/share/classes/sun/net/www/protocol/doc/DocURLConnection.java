/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)DocURLConnection.java	1.14 03/12/19
 */
/**
 * Open an file input stream given a doc URL.
 * @author	Sunita Mani	
 * @version     1.14, 12/19/03
 */

package sun.net.www.protocol.doc;

import java.net.URL;
import java.net.FileNameMap;
import java.io.*;
import sun.net.*;
import sun.net.www.*;
import java.security.Permission;

public class DocURLConnection extends URLConnection {

    InputStream is;
    String filename;
    Permission permission;

    static String installDirectory;

    static {
	installDirectory = (String)java.security.AccessController.doPrivileged(
                new sun.security.action.GetPropertyAction("hotjava.home"));
	if (installDirectory == null) {
	    installDirectory = "/usr/local/hotjava";
	}
    }

    DocURLConnection(URL u) {
	super(u);
    }

    public void connect() throws IOException {

	String fn = installDirectory + url.getFile();
	MessageHeader props = new MessageHeader();

	FileNameMap map = java.net.URLConnection.getFileNameMap();
	String contentType = map.getContentTypeFor(fn);
	if (contentType != null) {
	    props.add("content-type", contentType);
	}

	setProperties(props);

	File f = new File(fn);

	if (f.exists()) {
	    props.add("Content-length", String.valueOf(f.length()));
	}
	filename = fn.replace('/', File.separatorChar);
	is = new BufferedInputStream(new FileInputStream(filename));
		     
	connected = true;
    }

    public synchronized InputStream getInputStream()
	throws IOException
    {
	if (!connected) {
	    connect();
	}
	return is;
    }

    public Permission getPermission() throws IOException {
	if (permission == null) {
	    if (File.separatorChar == '/') {
		permission = new FilePermission(url.getFile(), "read");
	    } else {
		permission = new FilePermission(
			url.getFile().replace('/',File.separatorChar), "read");
	    }
	}
	return permission;
    }
}

