/*
 * @(#)BasicHttpRequest.java	1.43 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net;
import java.net.*;
import java.io.IOException;
import java.io.BufferedInputStream;
import java.io.InputStream;
import java.util.zip.GZIPInputStream;
import java.util.jar.JarOutputStream;
import java.io.FileOutputStream;
import java.io.FileInputStream;
import java.util.Date;
import java.io.File;
import com.sun.deploy.util.URLUtil;
import com.sun.deploy.config.Config;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.net.DownloadEngine;
import com.sun.deploy.net.offline.DeployOfflineManager;
import com.sun.deploy.Environment;
import com.sun.deploy.resources.ResourceManager;

/** Implementation class for the HttpRequest
 *  interfaces.
 */
public final class BasicHttpRequest implements HttpRequest {
    
    private final static String USER_AGENT_JAVA_VERSION = "UA-Java-Version";
    private final static String USER_AGENT = "User-Agent";
    
    private static String[] fieldName = {
        "content-length",
                "last-modified",
                "expires",
                "content-type",
                "content-encoding",
                "date",
                "server",
                "x-java-jnlp-version-id",
                "pragma",
                "cache-control"
    };
    
    public static boolean isHeaderFieldCached(String name) {
        if (name == null) {
            return false;
        }
        for (int i = 0; i < fieldName.length; i++) {
            if (name.equalsIgnoreCase(fieldName[i])) {
                return true;
            }
        }
        return false;
    }

   
    public HttpResponse doGetRequestEX(URL url, long lastModified) throws IOException {
        return doRequest(url, false, null, null, true, lastModified);
    }
    
    public HttpResponse doGetRequestEX(URL url,  String[] headerKeys, 
            String[] headerValues, long lastModified) throws IOException {
        return doRequest(url, false, headerKeys, headerValues, true, 
                lastModified);
    }

    
    public HttpResponse doGetRequest(URL url) throws IOException {
        return doRequest(url, false, null, null, true);
    }
    
    public HttpResponse doGetRequest(URL url, boolean httpCompression) throws IOException {
        return doRequest(url, false, null, null, httpCompression);
    }
    
    public HttpResponse doHeadRequest(URL url) throws IOException {
        // always use GET request for Java Plugin
        return doRequest(url, Environment.isJavaPlugin() ? false : true, null, null, true);
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
    private HttpResponse doRequest(URL url, boolean isHead, 
        String[] headerKeys, String[] headerValues, 
        boolean httpCompression) throws IOException {
          
        return doRequest(url, isHead, headerKeys, headerValues, 
            httpCompression, 0);
    }
    // Low-level interface
    private HttpResponse doRequest(URL url, boolean isHead, 
        String[] headerKeys, String[] headerValues, 
        boolean httpCompression, long cacheLastModified) throws IOException {
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
                
          
                
                if (path.endsWith(".jnlp")) fileMimeType = JNLP_MIME_TYPE;
                else if (path.endsWith(".jardiff")) fileMimeType = JARDIFF_MIME_TYPE;
            } catch(Exception e) {
                // Ignore
            }
        }
        
        URLConnection connection = null;
        
        if (url.getProtocol().equals("file")) {          
            // strip off the query part of the file url
            connection = createUrlConnection(HttpUtils.removeQueryStringFromURL(url), 
                    isHead, headerKeys, headerValues, httpCompression);
        } else {                       
            // Get connection object
            connection = createUrlConnection(url, isHead, headerKeys, 
                    headerValues, httpCompression);
        }
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
        
        // need this for Java Pluing
        // otherwise we will go through ResponseCache.get twice
        connection.setUseCaches(false);
        connection.setIfModifiedSince(cacheLastModified);
        
        boolean connected = false;
        
        // follow redirects
        // do not set if codebase override
        if (Environment.getImportModeCodebase() == null &&
                Environment.getImportModeCodebaseOverride() == null) {
            if (connection instanceof HttpURLConnection) {
                connection = HttpUtils.followRedirects(connection);
                connected = true;
            }
        }
        
        // The call to followRedirects above will actually open the connection
        // to the URL, So if we are connected already, no need to connect again
        if (!connected) {
            // Do connections
            connection.connect();
        }
        
        int status = HttpURLConnection.HTTP_OK;
        if (httpconn != null) {
            status = httpconn.getResponseCode();
        }
        if (Trace.isTraceLevelEnabled(TraceLevel.NETWORK)) {
            Trace.println(ResourceManager.getString("basicHttpRequest.responseCode",
                url == null ? "" : url.toString(),String.valueOf(status)), 
                TraceLevel.NETWORK);
        }
        int length  	  = connection.getContentLength();
        long lastModified = (fileTimeStamp != 0) ? 
            fileTimeStamp : connection.getLastModified();
        long expiration   = connection.getExpiration();
        String mimeType   = (fileMimeType != null)? 
            fileMimeType : connection.getContentType();
        
        // The mime type might be followed by a content encoding after a ';'. 
        // Strip that if it is there
        if (mimeType != null && mimeType.indexOf(';') != -1) {
            mimeType = mimeType.substring(0, mimeType.indexOf(';')).trim();
        }
        
      
        // Get response headers         
        MessageHeader messageHeader = initializeHeaderFields(connection);
        String encoding = messageHeader.findValue(HttpRequest.CONTENT_ENCODING);
     
        if (encoding != null) {
            encoding = encoding.toLowerCase();
        }

        if (Trace.isTraceLevelEnabled(TraceLevel.NETWORK)) {
            Trace.println(ResourceManager.getString("basicHttpRequest.encoding",
                url == null ? "" : url.toString(), encoding),
                TraceLevel.NETWORK);
        }
        
        BufferedInputStream bis = null;
        if (isHead) {
            bis = null;
        } else {
            bis = new BufferedInputStream(connection.getInputStream());           
        }
        
       
        
        return new BasicHttpResponse(url,
                status,
                length,
                expiration,
                lastModified,
                mimeType,
                messageHeader,
                bis, httpconn,
                encoding);
       
    }
    
    public static MessageHeader initializeHeaderFields(URLConnection uc)throws IOException{
        MessageHeader headerFields = new MessageHeader();
        String key = uc.getHeaderFieldKey(0);
        String status = uc.getHeaderField(0);
        if(null == key && null != status)
            headerFields.add(null, status);
        
        for(int i=0;i<fieldName.length;i++) {
            String value = uc.getHeaderField(fieldName[i]);
            if(value != null) {
                if (value.equalsIgnoreCase("application/x-java-archive-diff")) {
                    value = "application/java-archive";
                }
                headerFields.add(fieldName[i], value);
            }
        }
        return headerFields;
    }
    
    
    /**
     * Helper method which actually takes care of the connection and
     * and sets the pragma = no-cache property.
     */
    
    private URLConnection createUrlConnection(URL u, boolean isHead, String[] keys, String[] values, boolean httpCompression)
    throws MalformedURLException, IOException {
        URLConnection conn = (u.openConnection());
        
        // Set HTTP connection to bypass proxy, e.g., to force the proxy server 
        // to do an update
        // comment this out for now, not sure why we need this here
        //addToRequestProperty(conn, "pragma", "no-cache");
        
        // let the server know we would prefer to use compressed
        // version to save bandwidth.
        
        // fix for 5062781: accept-encoding and content-type not set
        // when doing version-download
        // Use HttpUtils.removeQueryStringFromURL(u) to remove query portion
        // of URL
        if (httpCompression) {
            String encoding = GZIP_ENCODING;
            String urlNoQuery = 
               HttpUtils.removeQueryStringFromURL(u).toString().toLowerCase();
            if (urlNoQuery.endsWith(".jar") ||
                urlNoQuery.endsWith(".jarjar")) {
		addToRequestProperty(conn, CONTENT_TYPE,JAR_MIME_TYPE);

		if (DownloadEngine.isPack200Supported()) {
		    encoding = PACK200_GZIP_ENCODING + "," + encoding;
		}
            }
            addToRequestProperty(conn, ACCEPT_ENCODING, encoding);
        }
        
        // Add user-agent information
        if (System.getProperty("http.agent") == null) {
            conn.setRequestProperty(USER_AGENT, Environment.getUserAgent());
            conn.setRequestProperty(USER_AGENT_JAVA_VERSION, Config.getJavaVersion());
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



