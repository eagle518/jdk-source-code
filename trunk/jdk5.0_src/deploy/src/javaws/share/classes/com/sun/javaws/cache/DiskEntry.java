/*
 * @(#)DiskEntry.java	1.6 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.cache;

import java.net.URL;
import java.util.Date;
import java.io.File;

/**
 * Wrapper class to contain a contents of an entry
 *
 * @version 1.6, 12/19/03
 */
public class DiskEntry {
    private URL    _url;
    private long   _timestamp;
    private File   _file;
    
    public DiskEntry(URL url, File file, long timestamp) {
	_url  = url;
	_timestamp = timestamp;
        _file = file;
    }

    public URL  getURL() { return _url; }
    
    public long getTimeStamp() { return _timestamp; }
    
    public File getFile() { return _file; }
}
