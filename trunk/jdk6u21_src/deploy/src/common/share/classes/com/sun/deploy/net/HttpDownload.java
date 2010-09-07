/*
 * @(#)HttpDownload.java	1.4 03/12/19
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.net;
import java.net.URL;
import java.net.URLConnection;
import java.io.File;
import java.io.IOException;
import java.io.ByteArrayInputStream;
import java.io.InputStream;

public interface HttpDownload {
    
    /** Download resource to the given file */
    public void download(int contentLength, URL url, InputStream in,
            String encoding, File file, HttpDownloadListener dl, int contentType) throws
            CanceledDownloadException, IOException;
}

