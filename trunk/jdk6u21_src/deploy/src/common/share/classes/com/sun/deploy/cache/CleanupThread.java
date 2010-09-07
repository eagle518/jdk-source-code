/*
 * @(#)CleanupThread.java	1.11 10/03/24
 * 
 * Copyright (c) 2005, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.cache;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.util.SystemUtils;
import com.sun.deploy.config.Config;
import java.io.*;
import java.util.*;

class CleanupThread extends Thread {    
    private ArrayList nonJarItemRemovalList = new ArrayList();
    private ArrayList jarItemRemovalList = new ArrayList();

    // for resource just downloaded
    // This is the only structure that can be used concurrently
    // (from multiple threads using Cache as well as from Cleanup thread)
    private ArrayList loadedResourceList = new ArrayList();

    private final static double CACHE_THRESHOLD_PERCENT = 0.98;
    private final Object syncObject;
    
    private final long currentCacheMaxSize = Config.getCacheSizeMax();
    private volatile long currentCacheSize = 0;
    private boolean initCacheSize = true;
    
    CleanupThread(String name, Object syncObject) {
        super(name);
        setDaemon(true);
        //only used to sync with "cache reset"?
        this.syncObject = syncObject;
    }
    
    synchronized void addToLoadedResourceList(String url) {
        loadedResourceList.add(url);
    }
    
    void startCleanup() {
        synchronized (this) {
            this.notify();
        }
    }
    
    private File[] getCacheResourceFiles() {
        return Cache.getCacheDir().listFiles(new FileFilter() {
            public boolean accept(File pathname) {
                String filename = pathname.getName();
                // resource file do not ends with .lap or .ico or .idx
                boolean retValue = !pathname.isDirectory() &&
                        !filename.toLowerCase().endsWith(Cache.INDEX_FILE_EXT) &&
                        !filename.toLowerCase().endsWith(".lap") &&
                        !filename.toLowerCase().endsWith(".ico") &&
                        !filename.equals(Cache.LAST_ACCESS_FILE) &&
                        !filename.equals(Cache.REMOVED_APPS);
                
                return retValue;
            }
        });
    }
    
    private long getCurrentCacheSize() {
        long cacheSize = 0;
        File[] cacheResourceFiles = getCacheResourceFiles();
        for (int i = 0; i < cacheResourceFiles.length; i++) {
      
            boolean countResource = true;
            // check if idx file exists for the resource
            File idxFile = new File(cacheResourceFiles[i].getPath() + 
                    Cache.INDEX_FILE_EXT);
            if (idxFile.exists()) {
                //generate cache entry for this resource
          
                CacheEntry ce = Cache.getCacheEntryFromFile(idxFile);
                           
                if (ce != null) {
                    synchronized (this) {
                      if (ce.getURL().toLowerCase().endsWith(".jnlp") ||
                            ce.getURL().toLowerCase().endsWith(".jarjnlp") ||
                            ce.getIsShortcutImage() == 1 ||
                            MemoryCache.contains(ce.getURL()) ||
                            loadedResourceList.contains(ce.getURL()) ||
                            ce.getBusy() == 1) {
                        // should not include jnlp files
                        // should not include images used for shortcuts
                        // should not include currently loaded resource
                        // should not include resource just downloaded
                        // should not include busy resource
                        // when calculating current cache size
                        countResource = false;                       
                      }
                    }
                    
                    // add the resource item to the removal lists, according to
                    // the file type
                    if (ce.getURL().toLowerCase().endsWith(".jar") ||
                            ce.getURL().toLowerCase().endsWith(".jarjar") ||
                            ce.getURL().toLowerCase().endsWith(".zip") ) {
                        if (countResource && jarItemRemovalList.contains(
                                cacheResourceFiles[i].getPath()) == false) {
                            jarItemRemovalList.add(cacheResourceFiles[i].getPath());
                        }
                    } else {
                        if (countResource && nonJarItemRemovalList.contains(
                                cacheResourceFiles[i].getPath()) == false) {
                            nonJarItemRemovalList.add(cacheResourceFiles[i].getPath());
                        }
                    }
                } else {
                    // this index file cannot generate a valid cache entry resource
                    // remove the index file and the resource

                    boolean ret = idxFile.delete();
                    if (ret == false) {
                        idxFile.deleteOnExit();
                    }
                    File resFile = new File(cacheResourceFiles[i].getPath());
                    ret = resFile.delete();
                    if (ret == false) {
                        resFile.deleteOnExit();
                    }                

                }
            } else {
                // this resource does not have a index -> invalid resource
                // remove the resource
                // do not delete -temp resource files

                if (cacheResourceFiles[i].getPath().endsWith("-temp") == false) {
                   
                    boolean ret = cacheResourceFiles[i].delete();
                    if (ret == false) {
                        cacheResourceFiles[i].deleteOnExit();
                    }
                }
            }
            if (countResource) {
                cacheSize += cacheResourceFiles[i].length();
            }
        }
        return cacheSize;
    }
    
    private Object[] prepareRemovalList(ArrayList removalList) {
        // sort the list
        // 1. resource without index file are always at the top
        // 2. resource with index file marked incomplete will be the next target
        //    for removal
        // 3. less recently used files are deleted before more recently used 
        //    ones. the lastModified time of the resource index file indicates 
        //    when is it last accessed
        // 4. If resources has the same lastAccessed time, expired resources 
        //    are removed first.
        // 5. If we still cannot rank the resource based of lastAccessed and 
        //    expired, we will remove the resource that has the larger 
        //    total file size first
        
        ArrayList cacheEntryList = new ArrayList();
        
        // go thru the list
        Iterator it = removalList.iterator();
        while (it.hasNext()) {
            String resourceFilePath = (String)it.next();
     
            // generate cache entry for each index file for comparsion
            File indexFile = new File(resourceFilePath + Cache.INDEX_FILE_EXT);
          
            if (indexFile.exists()) {
                CacheEntry ce = Cache.getCacheEntryFromFile(indexFile);
        
                cacheEntryList.add(ce);
            }
        }
        
        Object[] cacheEntryArray = cacheEntryList.toArray();
     
        // Sort the list
        Arrays.sort(cacheEntryArray, new Comparator() {
            public int compare(Object o1, Object o2) {
                CacheEntry ce1 = (CacheEntry)o1;
                CacheEntry ce2 = (CacheEntry)o2;
                if (ce1.removeBefore(ce2)) {
                
                    return -1;
                } else if (ce2.removeBefore(ce1)) {
                   
                    return 1;
                } else {
              
                    return 0;
                }
            }
        });


        return cacheEntryArray;
    }
    
    private void removeResourceFromList(Object[] cacheEntryArray) {
      
        long cacheThreshold = (long)(currentCacheMaxSize * CACHE_THRESHOLD_PERCENT);  
        // remove resource from the array until we are back down to the threshold
        for(int i=0; i<cacheEntryArray.length && currentCacheSize >= cacheThreshold; i++) {
            CacheEntry ce = (CacheEntry)cacheEntryArray[i];
            int size = ce.getContentLength();
        
            Cache.removeCacheEntry(ce);
            
            currentCacheSize -= size;
        }
    }
    
    public void run() {        
        while (true) {
            try {
                
                synchronized (this) {
                    // wait until a clean-up is needed, i.e, a resource got
                    // written into the cache
                    this.wait();
                }
                long t0 = SystemUtils.microTime();
                synchronized (syncObject) {
                    if (initCacheSize) {
                        currentCacheSize = getCurrentCacheSize();
                        initCacheSize = false;
                    }
                    // no need to remove valid resource if cache max
                    // is -1 (infinite)
                    if (currentCacheMaxSize != -1 &&
                        currentCacheSize >= currentCacheMaxSize) {
                        //clean the cache only if the size exceeds
                        // sort the removal list for non-jar items
                        Object[] resourceList =
                        prepareRemovalList(nonJarItemRemovalList);
                        // remove files from the non-jar items removal list
                        removeResourceFromList(resourceList);
                        // sort the removal list for jar items
                        resourceList = prepareRemovalList(jarItemRemovalList);
                        // remove files from the jar items list
                        removeResourceFromList(resourceList);
                    }
                    // TODO: If there is a reliable way to tell if a ressource is in
                    //       use by this JVM or any other, activate it.
                    // currentCacheSize -= Cache.removeDuplicateEntries(false, false);
                }
                long dT = SystemUtils.microTime() - t0;
                Trace.println("CleanupThread used "+dT+" us", TraceLevel.NETWORK);
            } catch (InterruptedException ie) {
                // ignored
            }
        }//run while loop
    }
}
