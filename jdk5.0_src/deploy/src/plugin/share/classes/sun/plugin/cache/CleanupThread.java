/*
 * @(#)CleanupThread.java	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package sun.plugin.cache;

import java.io.File;
import java.util.HashMap;
import java.util.Comparator;
import java.util.Arrays;
import sun.plugin.util.UserProfile;


// Thread to do cache cleanup.  Note that we use a lazy cache cleanup
// mechanism.  That is, we don't try to guarantee that the cache is
// always below the maximum size.  Instead, we wait until it is over
// the maximum size and then delete files until it falls back under.
// So the maximum cache size is more of a guideline than a rule.
public class CleanupThread extends Thread {

    // Whether a cache cleanup has been requested
    private boolean cleanupRequested = false;
    private Thread thisThread = null;
    private long timeToWait = 0;
    private long prevCacheSize;

    public CleanupThread(ThreadGroup group) {
	this(group, 10 * 60 * 1000);//default time out is 10 minutes
    }

    public CleanupThread(ThreadGroup group, long timeOut) {
        super(group, "Cache Cleanup Thread");
        setPriority(Thread.NORM_PRIORITY - 1);
	timeToWait = timeOut;
	setCacheSize();
    }

    public void addedFile(long size) {
	synchronized(this) {
	    prevCacheSize += size;		
	}
    }

    public void addedJar(long size) {
	synchronized(this) {
	    prevCacheSize += size;
	}
    }
    
    private void setCacheSize() {
	long fileCacheSize=0, jarCacheSize=0;
	File[] files;
	File jarDir = new File(UserProfile.getJarCacheDirectory());
	if(jarDir.exists()) {
	    files = jarDir.listFiles();
	    // Get current jar cache size
	    jarCacheSize = 0;
	    for (int i = 0; i < files.length; i++) {
		jarCacheSize += files[i].length();
	    }
	}

	File fileDir = new File(UserProfile.getFileCacheDirectory());
	if(fileDir.exists()) {
	    files = fileDir.listFiles();
	    // Get current jar cache size
	    fileCacheSize = 0;
	    for (int i = 0; i < files.length; i++) {
		fileCacheSize += files[i].length();
	    }
	}

	//set the cache sizes
	prevCacheSize = fileCacheSize + jarCacheSize;
    }


    public void run() {
        thisThread = Thread.currentThread();
	try {
	    while (true) {
		synchronized (this) {
		    if (cleanupRequested) {
			cleanupRequested = false;

			//clean the cache only if the size exceeds, otherwise
			//wait for some more time.
			if(prevCacheSize > Cache.threadThresholdSize) {
                            Cache.msgPrintln("cache.cleanup", new Object[] {new Long(prevCacheSize).toString()});
			    cleanCache();
			}
		    } else {
			// Wait for a certain time interval
			wait(timeToWait);
			cleanupRequested = true;
		    }
		}
	    }//run while loop
	} catch (InterruptedException e) {
	    // Ignore
	}

	thisThread = null;
    }

    // Clean up the cache if it is over the size limit
    protected void trigger() {
        if (thisThread != null) {
            synchronized (this) {
                cleanupRequested = true;
                notifyAll();
            }
        }
    }

    private void cleanCache() {
	// Get the list of cache files in the order they 
	// should be cleaned up
	Object[] fileList = getFilesInCache();
	int numOfFiles = fileList.length;
	long allowedSize = 0;

	// Get current file cache size
	long totalCacheSize = 0;
	for (int i = 0; i < numOfFiles; i++) {
	    totalCacheSize += ((CacheFile)fileList[i]).getSize();
	}

	//jar cache allowed size;
	allowedSize = Cache.deleteThresholdSize;

	// If the cache is larger than the maximum size,
	// delete files until it falls back under.
	int count = 0;
	while ((totalCacheSize > allowedSize) && (count < numOfFiles)) {
	    Cache.msgPrintln("cache.size", new Object[] {new Long(totalCacheSize)});		    	    			
	    totalCacheSize -= ((CacheFile)fileList[count++]).delete();
	}

	//set the previous cache values
	synchronized(this) {
	    prevCacheSize = totalCacheSize;
	}
    }


    // Updates our list of current cache files.  This list determines the
    // order in which files will be deleted.  Items at the head of the list
    // are deleted first.  We don't need to keep this list continuously up
    // to date becaues least recently used files are deleted first.  If
    // newer files appear in the cache, they would go at the end of the list.
    // So we don't need to check for new files until we have exhausted the
    // list of older files.
    private Object[] getFilesInCache() {
	HashMap cacheMap = new HashMap();

	//Get all the files in jar cache directory
        File[] files = null;
	//Get all the files in jar cache directory
	files = FileCache.directory.listFiles();

	//We construct a list of name strings. Each of the name string represent
	//either a combination of .idx and .zip, or only .zip or only .idx
        for (int i = 0; i < files.length; i++) {
	    String name = files[i].getName();
	    String ext = Cache.getFileExtension(name);
	    String cacheEntry = name.substring(0, name.length()-ext.length()).toLowerCase();

	    CacheFile cacheFile = (CacheFile)cacheMap.get(cacheEntry);
	    if(cacheFile == null) {
		cacheFile = new CacheFile(cacheEntry);
		cacheMap.put(cacheEntry, cacheFile);
	    }
	    cacheFile.addFile(files[i], ext);
        }

	files = JarCache.directory.listFiles();

	//We construct a list of name strings. Each of the name string represent
	//either a combination of .idx and .zip, or only .zip or only .idx
        for (int i = 0; i < files.length; i++) {
	    String name = files[i].getName();
	    String ext = Cache.getFileExtension(name);
	    String cacheEntry = name.substring(0, name.length()-ext.length()).toLowerCase();

	    CacheFile cacheFile = (CacheFile)cacheMap.get(cacheEntry);
	    if(cacheFile == null) {
		cacheFile = new CacheFile(cacheEntry);
		cacheMap.put(cacheEntry, cacheFile);
	    }
	    cacheFile.addFile(files[i], ext);
        }

	Object[] CacheFileArray = cacheMap.values().toArray();

        // Sort the list
        Arrays.sort(CacheFileArray, new Comparator() {
            public int compare(Object o1, Object o2) {
                CacheFile f1 = (CacheFile)o1;
                CacheFile f2 = (CacheFile)o2;
                if (f1.before(f2)) {
                    return -1;
                } else if (f2.before(f1)) {
                    return 1;
                } else {
                    return 0;
                }
            }
        });

	//For debug purpose
	//for(int i=0;i<CacheFileArray.length;i++) {
	//    System.out.println(((CacheFile)CacheFileArray[i]).getName());
	//}

	return CacheFileArray;
    }
}



