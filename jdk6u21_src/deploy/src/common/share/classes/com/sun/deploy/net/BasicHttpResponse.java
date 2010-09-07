/*
 * @(#)BasicHttpResponse.java	1.12 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net;

import java.net.*;
import java.util.ArrayList;
import java.io.BufferedInputStream;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.resources.ResourceManager;

/** Implementation class for the HttpResponse
 *  interfaces.
 */
final class BasicHttpResponse implements HttpResponse {
    private URL                     _request;
    private int  			_status;
    private int  			_length;
    private long 			_lastModified;
    private long			_expiration;
    private String  		_mimeType;
    private BufferedInputStream 	_bis;
    private HttpURLConnection       _httpURLConnection;
    private String                  _contentEncoding;
    private MessageHeader       _headers;
    
    BasicHttpResponse(URL request, int status, int length, long expiration, 
            long lastModified, String mimeType, MessageHeader headers, 
            BufferedInputStream bis, HttpURLConnection httpconn, 
            String contentEncoding) {
        _request = request;
        _status = status;
        _length = length;
        _expiration = expiration;
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
            if (Trace.isTraceLevelEnabled(TraceLevel.NETWORK)) {
              Trace.println(ResourceManager.getString(
                    "basicHttpResponse.disconnect", 
                    _request == null ? "" : _request.toString()), 
                    TraceLevel.NETWORK);
            }
        }
    }
    
    public MessageHeader getHeaders() { return _headers; }
    
    /** Returns the request that generated this response */
    public URL getRequest() { return _request; }
    /** Returns the HTTP status code */
    public int getStatusCode() { return _status; }
    /** Get the length of the message, or 0 if unknown */
    public int getContentLength() { return _length; }
    /** Get last modified time, or 0 if unknown */
    public long getLastModified() { return _lastModified; }
    public long getExpiration() { return _expiration; }
    /** Get MIME-TYPE of content */
    public String getContentType() { return _mimeType; }
    /** Get HttpCompressionType */
    public String getContentEncoding() { return _contentEncoding; }
    
    /** Access a response header */
    public String getResponseHeader(String key) {
        return _headers.findValue(key);
    }
    /** Access the bytes */
    public BufferedInputStream getInputStream() { return _bis; }
}
