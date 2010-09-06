/*
 * @(#)CacheFile.java	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package sun.plugin.cache;

import java.io.File;
import java.io.RandomAccessFile;
import java.io.IOException;
import java.io.EOFException;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.jar.JarFile;
import sun.plugin.util.Trace;

// Class used by the cleanup thread to represent a cache file
public class CacheFile {
    private String name;
    private File indexFile = null;
    private File dataFile = null;
    private long size = 0;
    private long date = 0;
    private byte status = Cache.UNUSABLE;
    private boolean usable = false;
    private boolean initialized = false;

    public CacheFile(String cacheName) {
	name = cacheName;
    }

    public void initialize() {
	if(dataFile != null && indexFile != null) {
	    size = dataFile.length() + indexFile.length();
	    date = indexFile.lastModified();
	} else {
	    //if one of the file is missing, both are not useful
	    if(dataFile != null) {
		size = dataFile.length();
	    }else if(indexFile != null) {
		size = indexFile.length();
	    }
	}

	//read the status if the index file exists	    
	if(indexFile != null) {
	    readStatus();
	} 

	initialized = true;
    }


    public void addFile(File file, String ext) {
	//set the files in cachefile object
	if(ext != null) {
	    if(ext.equalsIgnoreCase(Cache.INDEX_FILE_EXT)) {
		indexFile = file;
	    }else {
		dataFile = file;
	    }
	}
    }

    public long getSize() {
        return size;
    }

    public String getName() {
        return name;
    }

    public long getDate() {
        return date;
    }

    public int getFileType() {
	int type;
	String ext = Cache.getFileExtension(dataFile.getName());
	if(ext.equalsIgnoreCase(JarCache.DATA_FILE_EXT)) {
	    type = FileType.JAR;
	}else {
	    type = FileType.NONJAR;
	}
	return type;
    }

    public boolean isUsable() {
	if(!initialized) {
	    initialize();
	}
	return (status == Cache.UNUSABLE)?false:true;
    }

    public void readStatus() {
	RandomAccessFile raf = null;
	try {
	    raf = new RandomAccessFile(indexFile, "r");
	    status = raf.readByte();
	} catch (EOFException e) {
	    //the file is being written
	    //mark the status as incomplete
	    status = Cache.INCOMPLETE;		    
	} catch (IOException e) {
	    status = Cache.INCOMPLETE;		    
	} finally {
	    if (raf != null) {
		try {
		    raf.close();
		} catch (IOException e) {
		    // Ignore
		}
	    }
	}
    }

    public boolean before(CacheFile inFile) {
        // Unusable files are deleted before usable ones,
        // and less recently used files are deleted before
        // more recently used ones.
	
	//compare date only if both are usable files, otherwise
	//have the unusable files on the top regardless of date.
	if( isUsable() && inFile.isUsable() ) {
	    if(getFileType() == inFile.getFileType()) {
		return ( getDate() < inFile.getDate() ); 
	    } else {
		return ( getFileType() == FileType.NONJAR );
	    }
	} else {
	    return !isUsable();
	}
    }

    public long delete() {
        // First delete the data file, if it exists
        long deleted = 0;
	
	//before deleting data file, check whether the data file
	//is downloaded completely. This can be verified by the
	//status of the cache file
	if(status != Cache.INCOMPLETE) {
	    if (dataFile != null) {                
		long size = dataFile.length();
		Cache.msgPrintln("cache.full", new Object[]{dataFile.getName()});
		//If the file is in use, do not remove the file				
		if(isLoaded(dataFile) == true) {
		    Cache.msgPrintln("cache.inuse", new Object[] {dataFile.getName()});
		} else if (dataFile.delete()) {
		    dataFile = null;
		    deleted += size;
		} else {
		    Cache.msgPrintln("cache.notdeleted", new Object[] {dataFile.getName()});
		}
	    }
      
	    // Now delete the index file.  It is important that we only
	    // delete the index file if the data file no longer exists.
	    if (dataFile == null && indexFile != null) {
		long size = indexFile.length();
		Cache.msgPrintln("cache.full", new Object[]{indexFile.getName()});
		//If the file is in use, do not remove the file				
    		if (indexFile.delete()) {
		    indexFile = null;
		    deleted += size;
		} else {
		    Cache.msgPrintln("cache.notdeleted", new Object[] {indexFile.getName()});
		}
	    }
	}
        
        return deleted;
    }

    //Check whether the jar file has been loaded
    private boolean isLoaded(File inFile) {
	boolean retVal = false;

	if(Cache.getFileExtension(inFile.getName()).equalsIgnoreCase(JarCache.JAR_FILE_EXT)) {
	    //clone the loadedJars since we are accessing this
	    //from a different thread.
	    Hashtable cloneLoadedJars = (Hashtable)JarCache.loadedJars.clone();

	    // Check if the jar file has already been loaded
	    Iterator iter = cloneLoadedJars.values().iterator();
	    while(iter.hasNext() && retVal == false) {
		JarCache.JarReference ref = (JarCache.JarReference)iter.next();
		if (ref != null) {
		    JarFile jar = null;
		    jar = (CachedJarFile)ref.get();
		    if(jar != null) {
			//Make sure we are comparing the right filenames
			try {
			    if( new File(jar.getName()).getCanonicalPath().equals(inFile.getCanonicalPath()) ) {
				retVal = true;
			    }			    
			}catch(IOException exc) {
			    //no-op
			}
		    }
		}
	    }//while
	}else {
	    //clone the loadedFiles since we are accessing this
	    //from a different thread.
	    Hashtable cloneLoadedFiles = (Hashtable)FileCache.loadedFiles.clone();

	    // Check if the jar file has already been loaded
	    Iterator iter = cloneLoadedFiles.values().iterator();
	    while(iter.hasNext() && retVal == false) {
		FileCache.FileReference ref = (FileCache.FileReference)iter.next();
		if (ref != null) {
		    File file = null;
		    file = (File)ref.get();
		    if(file != null) {
			//Make sure we are comparing the right filenames
			try {
			    if( new File(file.getName()).getCanonicalPath().equals(inFile.getCanonicalPath()) ) {
				retVal = true;
			    }			    
			}catch(IOException exc) {
			    //no-op
			}
		    }
		}
	    }//while
	}

	return retVal;
    }

}



