/*
 * @(#)CacheUtil.java	1.22 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws;

import java.io.*;
import java.net.*;
import java.util.*;
import java.text.DateFormat;

import com.sun.javaws.jnl.*;
import com.sun.javaws.ui.SplashScreen;

import com.sun.deploy.cache.Cache;
import com.sun.deploy.cache.CacheEntry;
import com.sun.deploy.cache.LocalApplicationProperties;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.config.JREInfo;
import com.sun.deploy.net.DownloadEngine;
import com.sun.deploy.Environment;
import com.sun.deploy.ui.ProgressDialog;

/**
 * CacheUtil
 *
 * Some remaining code from the original Web Start Cache implementation
 * for removing JNLP apps from the cache.
 */

public class CacheUtil {


    /**
     * Removes all the related entry for all apps installed in the cache
     */
    public static void remove() {
        Iterator it =  Cache.getJnlpCacheEntries(
        Environment.isSystemCacheMode());
	while (it.hasNext()) {
            File jnlpFile = (File) it.next();
	    LaunchDesc ld = null;
 	    try {
                ld = LaunchDescFactory.buildDescriptor(jnlpFile, null, null, null);
            } catch (Exception e) {
                Trace.ignoredException(e);
            }
            if (ld != null) {
	        remove(jnlpFile, ld);
	    }
	}
	// if shortcut more, we are done, remove will have removed only the dti
	if (Globals.isShortcutMode()) {
	    return;
	}

	// OK - now safe to remove all objects remaining in the cache
	File[] files = Cache.getCacheEntries(
		       Environment.isSystemCacheMode());
	for (int i=0; i<files.length; i++) {
	    CacheEntry ce = Cache.getCacheEntryFromFile(files[i]);
	    if (ce != null) {
                Cache.removeCacheEntry(ce);
	    } else {
		files[i].delete();
	    }
	}

	// remove muffins
	try {
	    Cache.removeAllMuffins();
	} catch (Exception e) {
	    Trace.ignored(e);
	}
    }

    /**
     * Removes all the related entrys from the cache for a jnlp cache entry
     */
    public static void remove(CacheEntry ce) {
	try {
	    LaunchDesc ld = LaunchDescFactory.buildDescriptor(
                                ce.getDataFile(), null, null, null);
	    remove(ce, ld);
	} catch (Exception e) {
	    Trace.ignored(e);
	}
    }

    /**
     * Removes all the related entrys from the cache for a jnlp file
     */
    public static void remove(File jnlpFile, LaunchDesc ld) {
        File idxFile = new File (jnlpFile.getPath() + ".idx");
	if (idxFile.exists()) {
	    CacheEntry ce = Cache.getCacheEntryFromFile(idxFile);
	    if (ce != null) {
	        remove(ce, ld);
	    } else {
		idxFile.delete();
	        jnlpFile.delete();
	    }
	} else if (jnlpFile.exists() && 
	    Cache.getCacheDir().equals(jnlpFile.getParentFile())) {
	    jnlpFile.delete();
	} else {
	    CacheEntry ce = 
		Cache.getCacheEntry(ld.getCanonicalHome(), null, null);
	    if (ce != null) {
	        remove(ce, ld);
	    }
	}
    }

    /**
     * Removes all the related entrys from the cache for a jnlp cache entry
     */
    public static void remove(CacheEntry ce, LaunchDesc ld) {
	LocalApplicationProperties lap = 
	    Cache.getLocalApplicationProperties(ce);

        InformationDesc id = ld.getInformation();
	LocalInstallHandler lih = LocalInstallHandler.getInstance();
        
        // only uninstall shortcut for application descriptor
        // lih.uninstall for installer will prevent the code
        // below to execute the uninstaller (isLocallyInstalled 
        // will return false)
        if (lih != null && ld.isApplicationDescriptor()) {
            lih.uninstall(ld, lap);
        }

	// if shortcut mode - only uninstall shortcut then return
	if (Globals.isShortcutMode()) {
	    return;
	}

	// save removed app href (for undelete)
	if (ld.isApplicationDescriptor() && (ld.getLocation() != null)) {
	    Cache.saveRemovedApp(ld.getLocation(), id.getTitle());
        }

	// first uninstall shortcuts or installers
	lap.refresh();
        if (lap.isLocallyInstalled() && ld.isInstaller()) {
            ArrayList list = new ArrayList();
            list.add((File)ce.getDataFile());
            try {
                String path = lap.getInstallDirectory();
                JnlpxArgs.executeUninstallers(list);
                JREInfo.removeJREsIn(path);
            } catch (Exception e ) {
                Trace.ignoredException(e);
            }
        }

	// remove custom splash if present
	SplashScreen.removeCustomSplash(ld);

        // next Remove images and mapped images.
        if (id != null) {
            IconDesc[] icons = id.getIcons();
            if (icons != null) {
                for (int i = 0; i < icons.length; i++) {
                    URL url = icons[i].getLocation();
                    String version = icons[i].getVersion();
                    removeEntries(url, version);
                }
            }
	    RContentDesc[] rc = id.getRelatedContent();
	    if (rc != null) for (int i=0; i<rc.length; i++) {
		URL url = rc[i].getIcon();
		if (url != null) {
		    removeEntries(url, null);
		}
	    }

        }

	// next remove resources
        ResourcesDesc cbd = ld.getResources();
        if (cbd != null) {
            JARDesc[] cps = cbd.getLocalJarDescs();
            if (cps != null) {
                for (int counter = cps.length - 1; counter >= 0; counter--) {
                    URL location = cps[counter].getLocation();
                    String version = cps[counter].getVersion();
                    removeEntries(location, version);
                }
            }
            // next remove Installer Extensions
            ExtensionDesc [] eds = cbd.getExtensionDescs();
            if (eds != null) {
                for (int counter = eds.length - 1; counter >= 0; counter--) {
                    ExtensionDesc ed = eds[counter];
                    CacheEntry insCe = Cache.getCacheEntry(
                                    ed.getLocation(), null, ed.getVersion());
                    if (insCe != null) {
                        try {
                            LaunchDesc eld = LaunchDescFactory.buildDescriptor(
                                    insCe.getDataFile(), null, null, null);
                            if (eld != null && eld.isInstaller()) {
                                remove(insCe, eld);
                            }
                        } catch (Exception e) {
                            Trace.ignored(e);
                        }
                    }
                }
            }
        }

	String urlString = ce.getURL();
        String version = ce.getVersion();
	try {
	    URL location = new URL(urlString);
            if (location != null) {
                removeEntries(location, version);
            }
	} catch (MalformedURLException mue) {
	    Trace.ignored(mue);
	}
	// fix for 5022115
	// remove LAP in hashmap	
        Cache.removeLoadedProperties(urlString);	
	
    }

    /** Removes all entries matching a given url and Version String */
    private static void removeEntries(URL location, String version) {
        if (location != null) {
            CacheEntry ce = Cache.getCacheEntry(location, null, version); 
            Cache.removeAllCacheEntries(ce);
	}
    }

}


