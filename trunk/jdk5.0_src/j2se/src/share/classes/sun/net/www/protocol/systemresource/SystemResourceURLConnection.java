/*
 * @(#)SystemResourceURLConnection.java	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.net.www.protocol.systemresource;

import java.net.URL;
import java.net.URLConnection;
import java.net.MalformedURLException;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.security.Permission;
import sun.net.www.MessageHeader;
import sun.applet.AppletClassLoader;

/**
 * This is for backward compatibility only. It turns all systemresource
 * URLs into jar URLs, and delegate the requests.
 *
 * @author Benjamin Renaud
 * @version 1.9, 12/19/03
 */
public class SystemResourceURLConnection extends URLConnection {

    URL delegateUrl;
    URLConnection delegateConnection;

    SystemResourceURLConnection (URL url) 
    throws MalformedURLException, IOException  {
	super(url);
	delegateUrl = makeDelegateUrl(url);
	delegateConnection = delegateUrl.openConnection();
    }
    
    private URL makeDelegateUrl(URL url) throws MalformedURLException {
	boolean fileURL = false;
	String file = url.getFile();
	if (file.startsWith("/FILE")) {
	    fileURL = true;
	}
	int offset = fileURL ? 5 : 4;
	int index = file.lastIndexOf("/+/");
	if (index < 0) {
	    throw new MalformedURLException("no /+/ found in URL");
	}
	if (fileURL) {
	    file = "file:" + file.substring(offset, index) + 
		File.separatorChar + file.substring(index + 3, file.length());
	} else {
	    file = "jar:file:" + file.substring(offset, index) + "!/" +
		file.substring(index + 3, file.length());
	}
	return new URL(file);
    }

    public void connect() throws IOException {
	delegateConnection.connect();
    }

    public Object getContent() throws IOException {
	return delegateConnection.getContent();
    }

    public String getContentType() {
	return delegateConnection.getContentType();
    }

    public InputStream getInputStream() throws IOException {
	return delegateConnection.getInputStream();
    }

    public String getHeaderField(String name) {
	return delegateConnection.getHeaderField(name);
    }

    public Permission getPermission() throws IOException {
	return delegateConnection.getPermission();
    }
}
