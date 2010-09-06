/*
 * @(#)JavawsFactory.java	1.7 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws;
import com.sun.javaws.net.HttpRequest;
import com.sun.javaws.net.HttpDownload;
import com.sun.javaws.net.BasicNetworkLayer;
import com.sun.javaws.net.BasicDownloadLayer;
import java.net.URL;
import java.io.File;
import java.io.IOException;

/** Main factory for instantiating and keep track of the various
 *  singleton objects that makes up Javaws
 */
public class JavawsFactory {
    private static HttpRequest _httpRequestImpl;
    private static HttpDownload _httpDownloadImpl;
    
    // Initialize all singleton objects
    static {
	_httpRequestImpl = new BasicNetworkLayer();
	_httpDownloadImpl = new BasicDownloadLayer(_httpRequestImpl);
    }
    
    /** Get implementation of the low-level communication object */
    public static HttpRequest getHttpRequestImpl() { return _httpRequestImpl; }
    /** Get implementation of the high-level communication object */
    public static HttpDownload getHttpDownloadImpl() { return _httpDownloadImpl; }
}

