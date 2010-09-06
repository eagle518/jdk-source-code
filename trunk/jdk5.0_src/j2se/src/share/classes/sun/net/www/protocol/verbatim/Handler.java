/*
 * @(#)Handler.java	1.18 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.net.www.protocol.verbatim;

import java.io.*;
import java.net.*;


public class Handler extends URLStreamHandler {
    public URLConnection openConnection(URL u) throws IOException {
	return new VerbatimConnection(u);
    }
}

class VerbatimConnection extends URLConnection {
    URLConnection sub;
    protected VerbatimConnection (URL url)
		throws MalformedURLException, IOException
    {
	super(url);
	String file = url.getFile();
	if (file.startsWith("/"))
	    file = file.substring(1);
	sub = new URL(null, file).openConnection();
    }
    public void connect() throws IOException {
	sub.connect();
    }

    public java.security.Permission getPermission() throws IOException {
	return sub.getPermission();
    }

    public String getContentType() {
	return "text/plain";
    }

    public String getHeaderField(String name) {
	if (name.equalsIgnoreCase("content-type"))
	    return getContentType();
	return sub.getHeaderField(name);
    }

    public String getHeaderFieldKey(int n) {
	return sub.getHeaderFieldKey(n);
    }

    public String getHeaderField(int n) {
	if ("content-type".equalsIgnoreCase(getHeaderFieldKey(n)))
	    return getContentType();
	return sub.getHeaderField(n);
    }

    public Object getContent() throws IOException {
	return new sun.net.www.content.text.plain().getContent(sub);
    }

    public InputStream getInputStream() throws IOException {
	return sub.getInputStream();
    }

    public OutputStream getOutputStream() throws IOException {
	return sub.getOutputStream();
    }

    public String toString() {
	return "verbatim:"+sub.toString();
    }

    public void setDoInput(boolean doinput) {
	sub.setDoInput(doinput);
    }
    public boolean getDoInput() {
	return sub.getDoInput();
    }
    public void setDoOutput(boolean dooutput) {
	sub.setDoOutput(dooutput);
    }
    public boolean getDoOutput() {
	return sub.getDoOutput();
    }
    public void setAllowUserInteraction(boolean allowuserinteraction) {
	sub.setAllowUserInteraction(allowuserinteraction);
    }
    public boolean getAllowUserInteraction() {
	return sub.getAllowUserInteraction();
    }
    public void setUseCaches(boolean usecaches) {
	sub.setUseCaches(usecaches);
    }
    public boolean getUseCaches() {
	return sub.getUseCaches();
    }
    public void setIfModifiedSince(long ifmodifiedsince) {
	sub.setIfModifiedSince(ifmodifiedsince);
    }
    public long getIfModifiedSince() {
	return sub.getIfModifiedSince();
    }
    public void setRequestProperty(String key, String value) {
	sub.setRequestProperty(key, value);
    }
    public String getRequestProperty(String key) {
	return sub.getRequestProperty(key);
    }

}
