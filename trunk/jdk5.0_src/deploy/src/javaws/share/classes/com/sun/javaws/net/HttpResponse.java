/*
 * @(#)HttpResponse.java	1.6 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.net;
import java.net.URL;
import java.io.BufferedInputStream;



/** Defines the low-level response from a HTTP request
 */
public interface HttpResponse {
    
    /** Returns the request that generated this response */
    URL getRequest();
    /** Returns the HTTP status code */
    int getStatusCode();
    /** Get the length of the message, or 0 if unknown */
    int getContentLength();
    /** Get last modified time, or 0 if unknown */
    long getLastModified();
    /** Get MIME-TYPE of content */
    String getContentType();
    /** Access a response header */
    String getResponseHeader(String key);
    /** Access the bytes */
    BufferedInputStream getInputStream();
    /** Close the connection to the server */
    void disconnect();
    /** HTTP compression type */
    String getContentEncoding();
 
}
