/*
 * @(#)OldCacheEntry.java	1.5 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.cache;

import java.util.Date;
import java.util.LinkedList;
import java.io.File;
import java.io.FileFilter;
import java.io.RandomAccessFile;
import java.io.IOException;
import java.net.URL;
import java.net.MalformedURLException;

public class OldCacheEntry {
    // File extensions for old cache files
    private static final String INDEX_FILE_EXT = ".idx";
    private static final String DATA_FILE_EXT = ".zip";
    private static final String JAR_FILE_EXT = ".jar";
    private static final String JARJAR_FILE_EXT = ".jarjar";

    // JAR file
    private static final int JAR = 0x0001;

    // 1.4.2 and 5.0 have the same cache version
    private static final byte OLD_CACHE_VERSION = 16;


    private String url;
    private long expiration = 0;
    private long lastModified = 0;
    private String version = null;
    private File indexFile = null;
    private File dataFile = null;
    private boolean isJarEntry = false;

    private OldCacheEntry() {}
    
    boolean isJarEntry() {
        return isJarEntry;
    }

    File getDataFile() { return dataFile; }

    URL getURL() {
	try {
	    return new URL(url);
	} catch (MalformedURLException mue) {
	    // ignored
	}
	return null;
    }
    long getExpiration() { return expiration; }
    long getLastModified() { return lastModified; }

    String getVersion() { 
	if(version == null || version.equals(FileVersion.defStrVersion))
	    version = null;
	return version; 
    }

    static LinkedList getEntries(){
	LinkedList list = new LinkedList();
	String sep = File.separator;
	File jarCachePath = new File(CacheUpdateHelper.getOldCacheDirectoryPath() 
                                     + sep + "jar");
	getFileEntries(jarCachePath, list);
	File fileCachePath = new File(CacheUpdateHelper.getOldCacheDirectoryPath()
                                      + sep + "file");
	getFileEntries(fileCachePath, list);
	return list;
    }

    // loop through the files and call getDetails() to find out whether it is
    // a usable cache file
    private static void getFileEntries(File cachePath, LinkedList list){
	if(cachePath.exists()) {
	    //get only the .idx files
	    File[] cacheFiles = cachePath.listFiles(new FileFilter() {
		public boolean accept(File pathname) {
		    String name = pathname.getName();
		    return name.toLowerCase().endsWith(INDEX_FILE_EXT);
		}
	    });

	    for(int i=0;i<cacheFiles.length;i++) {
		Object entry = null;
		try {
		    entry = getDetails(cacheFiles[i]);
		} catch (IOException exc) {
		    entry = null;
		}
		if(entry != null ) {                   
		    list.add(entry);
		}
	    }
	}
    }

    //Check whether the cache file is usable, if so, construct OldCacheEntry
    //and return it
    private static OldCacheEntry getDetails(final File f) throws IOException {
       
	OldCacheEntry entry = null;
	RandomAccessFile raf = new RandomAccessFile(f, "r");
	try {
	    // Check the version of the file to make sure
	    // we know how to read it.
	    if (raf.readByte() == OLD_CACHE_VERSION) {
		//make sure that the data file exists
		entry = new OldCacheEntry();
		entry.indexFile = f;
		entry.url = raf.readUTF();
		entry.lastModified = raf.readLong();
		entry.expiration = raf.readLong();
		int type = raf.readInt();
		if(type == JAR) {
                    entry.isJarEntry = true;
		    entry.version = raf.readUTF();
		}
		//make sure that the data file exists
		File dataFile = getDataFileFromIndex(f, entry.url);
		if (dataFile.exists()) {
		    entry.dataFile = dataFile;
		} else {
		    entry = null;
		}
	    }
	} finally {
	    try {
		raf.close();
		raf = null;
	    } catch (IOException e) {
		// Ignore
	    }
	}
	return entry;
    }

      // Gets the data file corresponding to the given index file
    private static final File getDataFileFromIndex(File f, String url) {
        String name = f.getName();
        name = name.substring(0, name.length() - INDEX_FILE_EXT.length());
        name += getFileExtension(url);
        return new File(f.getParentFile(), name);
    }


    // It returns the extension of the file to which URL points
    private static final String getFileExtension(String name) {
	String ext = "";
	//get the url extension
	int extIndex = name.lastIndexOf('.');
	if(extIndex != -1) {
	    ext = name.substring(extIndex);
	}
	
	//.jar and .jarjar files are stored as .zip
	if( ext.equalsIgnoreCase(JAR_FILE_EXT) || 
	    ext.equalsIgnoreCase(JARJAR_FILE_EXT) ) {
	    ext = DATA_FILE_EXT;
	} 

	return ext;
    }


    public String toString() {
	return "url: " + getURL() + "\n" +
	    "dataFile: " + getDataFile() + "\n" +
	    "expiration: " + getExpiration() + "\n" +
	    "lastModified: " + getLastModified() + "\n" +
	    "version: " + getVersion() + "\n";
    }

}
