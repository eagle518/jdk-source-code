/*
 * @(#)CacheUpdateHelper.java	1.8 10/03/24
 *
 * Copyright (c) 2005, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.cache;
import java.util.LinkedList;
import java.util.ListIterator;
import java.io.File;
import java.io.IOException;
import com.sun.deploy.config.Config;
import com.sun.deploy.net.DownloadEngine;
import com.sun.deploy.cache.Cache;
import com.sun.deploy.util.Trace;
import com.sun.deploy.ui.ProgressDialog;
import com.sun.deploy.ui.CacheUpdateProgressDialog;

public class CacheUpdateHelper {

    private static final String JAVAPI = "javapi";
    private static final String CACHE_VERSION = "v1.0";

    private static String getCacheDirectorySubStructure() {
        return JAVAPI + File.separator + CACHE_VERSION;
    }


    static String getOldCacheDirectoryPath() {
	return Config.getCacheDirectory() + File.separator + getCacheDirectorySubStructure();
    }

    public static boolean updateCache() {
	String oldPath = getOldCacheDirectoryPath();
	File oldCache = new File(oldPath);
	File newCache = Cache.getCacheDir();

	if (oldCache.exists() == false || oldCache.isDirectory() == false || 
	    oldCache.equals(newCache)) {
	    // no need to update
	    return true;
	}

	LinkedList list = OldCacheEntry.getEntries();
	ListIterator li = list.listIterator(0);
	int total = list.size();
	int done = 0;

	try {
	    if (total > 0) {
		CacheUpdateProgressDialog.showProgress(0, 100);
	    }
            Cache.setCleanupEnabled(false);
	    while (li.hasNext()) {
		OldCacheEntry oce = (OldCacheEntry)(li.next());
		try {
		    Cache.insertFile(oce.getDataFile(), 
				     oce.isJarEntry() ? 
                                         DownloadEngine.JAR_CONTENT_BIT : 
                                         DownloadEngine.NORMAL_CONTENT_BIT,
				     oce.getURL(), 
				     oce.getVersion(), oce.getLastModified(),
				     oce.getExpiration());
		} catch (IOException ioe) {
		    Trace.ignored(ioe);
		}
		CacheUpdateProgressDialog.showProgress(++done, total);
	    }
	} catch (CacheUpdateProgressDialog.CanceledException ce) {
            // user choose to cancel cache upgrade, we should not do it
            // again anymore
	} finally {
	    CacheUpdateProgressDialog.dismiss();
            Cache.setCleanupEnabled(true);
	}

	return true;
    }
}
