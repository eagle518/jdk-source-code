/*
 * @(#)JARUpdater.java	1.4 10/03/24
 * 
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.jnl;

import java.io.File;
import java.io.IOException;
import java.net.URL;
import com.sun.deploy.cache.Cache;
import com.sun.deploy.cache.CacheEntry;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.util.URLUtil;
import com.sun.deploy.net.DownloadEngine;
import com.sun.javaws.exceptions.*;

/**
 * A class that does update check and download for a JARDesc.
 */

public class JARUpdater {
    private JARDesc _jar = null;
    private CacheEntry _ce = null;
    private boolean _updateChecked = false;
    private boolean _updateAvailable = false;
    private boolean _updateDownloaded = false;
    
    public JARUpdater(JARDesc jar) {
	_jar = jar;
    }
    
    public synchronized boolean isUpdateAvailable() throws Exception {
	if (!_updateChecked) {
	    Trace.println("JARUpdater: update check for "+_jar.getLocation().toString(), TraceLevel.NETWORK);
	    try {
		_updateAvailable = updateCheck();
		_updateChecked = true;
		_updateDownloaded = !_updateAvailable;
	    } catch (Exception e) {
                Trace.ignored(e);
                throw e;
	    }
	}
	
	return _updateAvailable;
    }
    
    public boolean isUpdateDownloaded() {
	return _updateDownloaded;
    }
    
    // Download the jar to a temp cache entry
    public void downloadUpdate() throws Exception {
	if (isUpdateAvailable()) {
	    download();
	}

	synchronized(this) {
	    _updateAvailable = false;
	    _updateDownloaded = true;
	} 
    }
    
    private CacheEntry getCacheEntry() {
	return _ce;
    }

    /*
     * Enable the downloaded temp cache entry and remove
     * the old (current) cache entry
     */
    public void updateCache() throws IOException {
	URL location = _jar.getLocation();
	CacheEntry newCE = getCacheEntry();

	CacheEntry currentCE = Cache.getCacheEntry(location, null,
						   _jar.getVersion(), 
						   getContentType());
	if (newCE != null) {
	    Cache.processNewCacheEntry(location, true, /*remove current ce*/
				       newCE, currentCE);
	}
	Trace.println("Background Update Thread: update cache: "+location, TraceLevel.NETWORK);
    }

    // check if a update is available
    private boolean updateCheck() throws JNLPException {
	URL location = _jar.getLocation();
	String version = _jar.getVersion();
	boolean update = false;

	// no version based update check
        if (version != null) return false;
	
	try {
	    update = DownloadEngine.isUpdateAvailable(location, version, _jar.isPack200Enabled());
	} catch  (IOException ioe) {
            ResourcesDesc parent = _jar.getParent();
            LaunchDesc ld = (parent == null)? null : parent.getParent();
	    throw new FailedDownloadingResourceException(ld, location, null, ioe);
	}
	return update;
    }


    // download the jar to a temp cache entry if there is a update
    private void download() throws JNLPException {
	int downloadType = getContentType();
	URL location = _jar.getLocation();
	String version = _jar.getVersion();
	CacheEntry ce = null;
	
	try {
	    ce = DownloadEngine.getResourceTempCacheEntry(location,
							  version,
							  downloadType);
	    
	    Trace.println("Downloaded " + location + " to "
			  + URLUtil.fileToURL(new File(ce.getResourceFilename()))+
			  "\n\t Cache Entry disabled", TraceLevel.NETWORK);
	} catch (IOException ioe) {
	    throw new FailedDownloadingResourceException(location, version, ioe);
	}
	if (Cache.isCacheEnabled() && ce == null) {
	    throw new FailedDownloadingResourceException(location, version, null);
	} else {
	    _ce = ce;
	}
    }
    
    private int getContentType() {
	int downloadType = DownloadEngine.JAR_CONTENT_BIT;
	if (_jar.isNativeLib()) {
	    downloadType = downloadType |
		DownloadEngine.NATIVE_CONTENT_BIT;
	}
	if (_jar.isPack200Enabled()) {
	    downloadType = downloadType |
		DownloadEngine.PACK200_CONTENT_BIT;
	}

	return downloadType;
    }
}
