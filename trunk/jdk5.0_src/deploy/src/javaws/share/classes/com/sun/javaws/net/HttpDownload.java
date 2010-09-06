/*
 * @(#)HttpDownload.java	1.4 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.net;
import java.net.URL;
import java.io.File;
import java.io.IOException;

public interface HttpDownload {
    
    /** Download resource to the given file */
    void download(HttpResponse response, File location, HttpDownloadListener dl)
	throws IOException, CanceledDownloadException;
    
    /** Download resource to the given file */
    void download(URL url, File location, HttpDownloadListener dl)
	throws IOException, CanceledDownloadException;
}

