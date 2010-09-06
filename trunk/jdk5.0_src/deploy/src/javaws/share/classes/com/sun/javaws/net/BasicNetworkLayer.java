/*
 * @(#)BasicNetworkLayer.java	1.15 04/06/14
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.net;
import java.net.*;
import java.io.IOException;
import java.io.BufferedInputStream;
import java.io.InputStream;
import java.util.zip.GZIPInputStream;
import java.util.jar.JarOutputStream;
import java.io.FileOutputStream;
import java.io.FileInputStream;

import java.util.Map;
import java.util.HashMap;
import java.util.Date;
import java.io.File;
import com.sun.javaws.util.URLUtil;
import com.sun.javaws.Globals;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;

/** Implementation class for the HttpRequest and HttpResponse
 *  interfaces.
 */
public class BasicNetworkLayer implements HttpRequest {
    
    private final static String USER_AGENT_JAVA_VERSION = "UA-Java-Version";
    private final static String USER_AGENT = "User-Agent";
    
    
    /** Concrete implemenation of the response */
    static class BasicHttpResponse implements HttpResponse {
	private URL                     _request;
	private int  			_status;
	private int  			_length;
	private long 			_lastModified;
	private String  		_mimeType;
	private Map     		_headers;
	private BufferedInputStream 	_bis;
	private HttpURLConnection       _httpURLConnection;
	private String                  _contentEncoding;
	 
	
	
	BasicHttpResponse(URL request, int status, int length, long lastModified, String mimeType, Map headers, BufferedInputStream bis, HttpURLConnection httpconn, String contentEncoding) {
	    _request = request;
	    _status = status;
	    _length = length;
	    _lastModified = lastModified;
	    _mimeType = mimeType;
	    _headers = headers;
	    _bis = bis;
	    _httpURLConnection = httpconn;
	    _contentEncoding = contentEncoding;
	}
	
	// fix for 4751780: JWS Hanging on Jar update (Scanning entries at 100% for a LONG time)
	// we should disconnect connection to server after downloading
	public void disconnect() {
	    if (_httpURLConnection != null) {
		_httpURLConnection.disconnect();
		
		Trace.println("Disconnect connection to " + _request, TraceLevel.NETWORK);
		
	    }
	}
	
	/** Returns the request that generated this response */
	public URL getRequest() { return _request; }
	/** Returns the HTTP status code */
	public int getStatusCode() { return _status; }
	/** Get the length of the message, or 0 if unknown */
	public int getContentLength() { return _length; }
	/** Get last modified time, or 0 if unknown */
	public long getLastModified() { return _lastModified; }
	/** Get MIME-TYPE of content */
	public String getContentType() { return _mimeType; }
	/** Get HttpCompressionType */
	public String getContentEncoding() { return _contentEncoding; }

	/** Access a response header */
	public String getResponseHeader(String key) { return (String)_headers.get(key.toLowerCase()); }
	/** Access the bytes */
	public BufferedInputStream getInputStream() { return _bis; }
    }
    
    public HttpResponse doGetRequest(URL url) throws IOException {
	return doRequest(url, false, null, null, true);
    }
    
    public HttpResponse doGetRequest(URL url, boolean httpCompression) throws IOException {
	return doRequest(url, false, null, null, httpCompression);
    }
    
    public HttpResponse doHeadRequest(URL url) throws IOException {
	return doRequest(url, true, null, null, true);
    }
  
    public HttpResponse doHeadRequest(URL url, boolean httpCompression) throws IOException {
	return doRequest(url, true, null, null, httpCompression);
    }
    
    
    public HttpResponse doGetRequest(URL url, String[] headerKeys, String[] headerValues) throws IOException {
	return doRequest(url, false, headerKeys, headerValues, true);
    }
    
   
    public HttpResponse doGetRequest(URL url, String[] headerKeys, String[] headerValues, boolean httpCompression) throws IOException {
	return doRequest(url, false, headerKeys, headerValues, httpCompression);
    }
    
    public HttpResponse doHeadRequest(URL url, String[] headerKeys, String[] headerValues) throws IOException {
	return doRequest(url, true, headerKeys, headerValues, true);
    }
    
    public HttpResponse doHeadRequest(URL url, String[] headerKeys, String[] headerValues, boolean httpCompression) throws IOException {
	return doRequest(url, true, headerKeys, headerValues, httpCompression);
    }
    
    // Low-level interface
    private HttpResponse doRequest(URL url, boolean isHead, String[] headerKeys, String[] headerValues, boolean httpCompression) throws IOException {
	// File URLs needs to be treated special, since the URL classes does not
        // return the right time stamp information. Try to get the timestamp if it is
        // a file
        long fileTimeStamp = 0;
        String fileMimeType = null;
        if ("file".equals(url.getProtocol()) && url.getFile() != null) {
	    try {
		// change for bug #4429806 - encoding of urls.
		String path = URLUtil.getPathFromURL(url);
		File f = new File(path);
		fileTimeStamp = f.lastModified();
		
		Trace.println("File URL discovered. Real timestamp: " + new Date(fileTimeStamp), TraceLevel.NETWORK);
		
		if (path.endsWith(".jnlp")) fileMimeType = JNLP_MIME_TYPE;
		else if (path.endsWith(".jardiff")) fileMimeType = JARDIFF_MIME_TYPE;
	    } catch(Exception e) {
		// Ignore
	    }
        }
	
	// Get connection object
	URLConnection connection = createUrlConnection(url, isHead, headerKeys, headerValues, httpCompression);
	// Get as HttpURLConnection, if possible
	HttpURLConnection httpconn = null;
	if (connection instanceof HttpURLConnection ) {
	    httpconn = (HttpURLConnection)connection;
	}
	
	// this is the work around
	// all HTTP 1.1 header must include Host header field.
	// else all HTTP 1.1 server is required to return with 400 bad request
	// see rfc2616-sec14
	URLUtil.setHostHeader(connection);
	
	
	// Do connections
	connection.connect();
	
	int status = HttpURLConnection.HTTP_OK;
	if (httpconn != null) {
	    status = httpconn.getResponseCode();	 
	}
	
	int length  	  = connection.getContentLength();
	long lastModified = (fileTimeStamp != 0) ? fileTimeStamp : connection.getLastModified();
	String mimeType   = (fileMimeType != null)? fileMimeType : connection.getContentType();
	// The mime type might be followed by a content encoding after a ';'. Strip that if
	// it is there
	if (mimeType != null && mimeType.indexOf(';') != -1) {
	    mimeType = mimeType.substring(0, mimeType.indexOf(';')).trim();
	}
	
	// Get response headers
	HashMap responsHeaders = new HashMap();
	int i = 1;
	String key = connection.getHeaderFieldKey(i);
	while(key != null) {
	    // change all the header key to lowercase - different 
	    // implementation of the servlet might change the key to
	    // different case
	    // REMEMBER to use lowercase keys when getting key header
	    responsHeaders.put(key.toLowerCase(), connection.getHeaderField(i));
	    i++;
	    key = connection.getHeaderFieldKey(i);
	}

	String encoding = (String) responsHeaders.get(HttpRequest.CONTENT_ENCODING) ;

	if (encoding != null) encoding = encoding.toLowerCase();

	Trace.println("encoding = " + encoding + " for " + url.toString(),TraceLevel.NETWORK);

	BufferedInputStream bis = null;
	if (isHead) {
	    bis = null;
	} else {
	    InputStream is = null; 
	    is = new BufferedInputStream(connection.getInputStream()); 
	    // CE must contain one string.
	    if (encoding != null && 
		    (encoding.compareTo(PACK200_GZIP_ENCODING) == 0 || 
			 encoding.compareTo(GZIP_ENCODING) == 0) ) {
		bis = new BufferedInputStream(new GZIPInputStream(is)); 	    	 
	    } else {
		bis = new BufferedInputStream(is);
	    }
	} 
	

	return new BasicHttpResponse(url,
				     status,
				     length,
				     lastModified,
				     mimeType,
				     responsHeaders,
				     bis, httpconn, 
				     encoding);

    }
    
    /**
     * Helper method which actually takes care of the connection and
     * and sets the pragma = no-cache property.
     */
    
    private URLConnection createUrlConnection(URL u, boolean isHead, String[] keys, String[] values, boolean httpCompression)
        throws MalformedURLException, IOException {
        URLConnection conn = (u.openConnection());
	
	// Set HTTP connection to bypass proxy, e.g., to force the proxy server to do an update
	addToRequestProperty(conn, "pragma", "no-cache");
	
	// let the server know we would prefer to use compressed
	// version to save bandwidth.

	// fix for 5062781: accept-encoding and content-type not set
	// when doing version-download
	// Use URL.getPath(), otherwise won't for version download protocol
	// because URL.getFile() includes the query portion of the URL
	if (httpCompression && u.getPath().endsWith(".jar")) {
	    String encoding = (Globals.havePack200()) ?	
		PACK200_GZIP_ENCODING + "," + GZIP_ENCODING :
		GZIP_ENCODING;
	    addToRequestProperty(conn, ACCEPT_ENCODING, encoding);
	    addToRequestProperty(conn, CONTENT_TYPE,JAR_MIME_TYPE);
	    Trace.println("Requesting file " + u.getFile() + " with Encoding = " + encoding,TraceLevel.NETWORK);
	}
	    
	// Add user-agent information
	if (System.getProperty("http.agent") == null) {
            conn.setRequestProperty(USER_AGENT, Globals.getUserAgent());
	    conn.setRequestProperty(USER_AGENT_JAVA_VERSION, Globals.getJavaVersion());
	}
	
	// Setup the request headers
	if (keys != null && values != null) {
	    for(int i = 0; i < keys.length; i++) {
		conn.setRequestProperty(keys[i], values[i]);
	    }
	}
	
	// Setup HEAD/GET request
	if (conn instanceof HttpURLConnection) {
	    ((HttpURLConnection)conn).setRequestMethod((isHead) ? "HEAD" : "GET");
	}
	return conn;
    }
    
    /** Add to a request header */
    private void addToRequestProperty(URLConnection conn, String key, String value) {
	String curvalue = conn.getRequestProperty(key);
	if (curvalue == null || curvalue.trim().length() == 0) {
	    curvalue = value;
	} else {
	    curvalue += "," + value;
	}
	conn.setRequestProperty(key, curvalue);
    }
}



