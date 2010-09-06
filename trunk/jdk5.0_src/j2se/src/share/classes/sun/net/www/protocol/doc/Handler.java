/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)Handler.java	1.30 03/12/19
 */

/**
 *	doc urls point either into the local filesystem or externally
 *      through an http url.
 */

package sun.net.www.protocol.doc;

import java.net.URL;
import java.net.URLConnection;
import java.net.MalformedURLException;
import java.net.URLStreamHandler;
import java.io.InputStream;
import java.io.IOException;
import java.io.File;

public class Handler extends URLStreamHandler {

    static String base;
    static String installDirectory;

    static {

	base = (String) java.security.AccessController.doPrivileged(
                new sun.security.action.GetPropertyAction("doc.url"));
	if (base == null) {
	    base = "http://java.sun.com/HotJava/";
	} else if (base.charAt(base.length() - 1) != '/') {
	    base += "/";
	}

	installDirectory = (String)java.security.AccessController.doPrivileged(
                new sun.security.action.GetPropertyAction("hotjava.home"));
	if (installDirectory == null) {
	    installDirectory = "/usr/local/hotjava";
	}
    }
    /*
     * Attempt to find a load the given url using the local
     * filesystem. If that fails, then try opening using http.
     */
    public synchronized URLConnection openConnection(URL u)
	throws IOException
    {
	String file = u.getFile();
	file = installDirectory+file;
	File f = new File(file);

	if (f != null && f.exists() && !f.isDirectory()) {
	    return new DocURLConnection(u);
	} else {
	    URLConnection uc = null;
	    URL ru;
	    try {
		file = u.getFile();
		if (file.charAt(0) == '/') {
		    file = file.substring(1);
		}
		ru = new URL(base + file);
	    } catch (MalformedURLException e) {
		ru = null;
	    }
	    if (ru != null) {
		uc = ru.openConnection();
	    }
	    if (uc == null) {
		throw new IOException("Can't find file for URL: "
				      +u.toExternalForm());
	    }
	    return uc;
	}
    }
}

