/*
 * @(#)JarCacheEntry.java	1.20 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package sun.plugin.cache;

import java.util.Date;
import java.util.LinkedList;
import java.io.File;
import java.io.FileFilter;
import java.io.RandomAccessFile;
import java.io.IOException;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;
import java.security.AccessController;
import java.text.SimpleDateFormat;
import java.awt.Panel;
import java.awt.Font;
import sun.plugin.resources.ResourceHandler;
import sun.plugin.util.UserProfile;

class JarCacheEntry {
    final public static String NAME = ResourceHandler.getMessage("cache_viewer.name");
    final public static String TYPE = ResourceHandler.getMessage("cache_viewer.type");
    final public static String SIZE = ResourceHandler.getMessage("cache_viewer.size");
    final public static String MODIFY_DATE = ResourceHandler.getMessage("cache_viewer.modify_date");
    final public static String EXPIRY_DATE = ResourceHandler.getMessage("cache_viewer.expiry_date");
    final public static String VERSION = ResourceHandler.getMessage("cache_viewer.version");
    final public static String URL = ResourceHandler.getMessage("cache_viewer.url");

    final public static String NAME_HELP = ResourceHandler.getMessage("cache_viewer.help.name");
    final public static String TYPE_HELP = ResourceHandler.getMessage("cache_viewer.help.type");
    final public static String SIZE_HELP = ResourceHandler.getMessage("cache_viewer.help.size");
    final public static String MODIFY_DATE_HELP = ResourceHandler.getMessage("cache_viewer.help.modify_date");
    final public static String EXPIRY_DATE_HELP = ResourceHandler.getMessage("cache_viewer.help.expiry_date");
    final public static String VERSION_HELP = ResourceHandler.getMessage("cache_viewer.help.version");
    final public static String URL_HELP = ResourceHandler.getMessage("cache_viewer.help.url");

    private String url;
    private long expiration = 0;
    private long lastModified = 0;
    String version = null;
    private File indexFile = null;
    private File dataFile = null;

    public String getURL() { return url; }
    public String getExpiration() { return formatDate(expiration); }
    public String getLastModifyDate() { return formatDate(lastModified); }

    public String getVersion() { 
	if(version == null || version.equals(FileVersion.defStrVersion))
	    version = "N/A";
	return version; 
    }

    public String getSize() { 
	long lengthInKb = 0;
	long length = dataFile.length();
	lengthInKb = length/1024;
	if((length - lengthInKb*1024) > 0 )
	    lengthInKb++;
	return new Long(lengthInKb).toString(); 
    }

    public String getName() { return dataFile.getName(); }

    public String getFileExtenstion() {
	return FileType.getFileDescription(Cache.getFileExtension(url));
    }

    public Object getDisplayValue(String column) {
	Object retVal = null;
	if(column.equals(URL)) {
	    retVal = getURL();
	} else if(column.equals(EXPIRY_DATE)) {
	    retVal =  getExpiration();
	} else if(column.equals(MODIFY_DATE)) {
	    retVal =  getLastModifyDate();
	} else if(column.equals(VERSION)) {
	    retVal =  getVersion();
	} else if(column.equals(SIZE)) {
	    retVal =  getSize() + "KB";
	} else if(column.equals(NAME)) {
	    retVal =  getName();
	} else if(column.equals(TYPE)) {
	    retVal =  getFileExtenstion();
	}
	
	return retVal;		
    }

    public Object getValue(String column) {
	Object retVal = null;
	if(column.equals(URL)) {
	    retVal = getURL();
	} else if(column.equals(EXPIRY_DATE)) {
	    retVal =  new Date(expiration);
	} else if(column.equals(MODIFY_DATE)) {
	    retVal =  new Date(lastModified);
	} else if(column.equals(VERSION)) {
	    retVal =  getVersion();
	} else if(column.equals(SIZE)) {
	    retVal =  new Long(dataFile.length());
	} else if(column.equals(NAME)) {
	    retVal =  getName();
	} else if(column.equals(TYPE)) {
	    retVal =  getFileExtenstion();
	}
	
	return retVal;		
    }

    private String formatDate(long time) {
	SimpleDateFormat formatter
	    = new SimpleDateFormat ("MM/dd/yy hh:mm a");
	Date date = new Date(time);
	return formatter.format(date);
    }

    public static int getTextWidth(Font face, String column) {
        Panel p = new Panel();
	String text = "";

	if(column.equals(URL)) {
	    text = "http://java.sun.com/javaweb/products/plugin/swingapplet.html";
	} else if( column.equals(EXPIRY_DATE) || column.equals(MODIFY_DATE) ) {
	    text =  "12/13/69 04:00 PM  ";
	} else if(column.equals(VERSION)) {
	    text =  "Version    ";
	} else if(column.equals(SIZE)) {
	    text =  "999,999KB ";
	} else if(column.equals(NAME)) {
	    text =  "test-images.jar-2bc62790-5ad1049d";
	} else if(column.equals(TYPE)) {
	    text =  "Jpeg Image  ";
	}
	
	return p.getFontMetrics(face).stringWidth(text);    
    }

    //Try to delete the selected cached jar file	
    public boolean delete() {
	boolean result = dataFile.delete();
	if(result == true){
	    dataFile = null;
	    if(indexFile.delete() == true) {
		indexFile = null;
	    }
	}
	return result;
    }

    public static LinkedList getEntries(){
	LinkedList list = new LinkedList();
	String sep = File.separator;
	File jarCachePath = new File(JarCacheViewer.getPluginCacheLocation() 
                                     + sep + "jar");
	getFileEntries(jarCachePath, list);
	File fileCachePath = new File(JarCacheViewer.getPluginCacheLocation()
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
		    return name.toLowerCase().endsWith(Cache.INDEX_FILE_EXT);
		}
	    });

	    for(int i=0;i<cacheFiles.length;i++) {
		Object entry = null;
		try {
		    entry = getDetails(cacheFiles[i]);
		}catch(IOException exc) {
		    exc.printStackTrace();
		    entry = null;
		}
		if(entry != null ) {
		    list.add(entry);
		}
	    }
	}
    }

    //Check whether the cache file is usable, if so, construct JarCacheEntry
    //and return it
    private static JarCacheEntry getDetails(final File f) throws IOException{
        return (JarCacheEntry)privileged(new CacheIOAction() {
            public Object run() throws IOException {
		JarCacheEntry entry = null;
                RandomAccessFile raf = new RandomAccessFile(f, "r");
		try {
                    // Check the version of the file to make sure
                    // we know how to read it.
                    if (raf.readByte() == Cache.VERSION) {
			//make sure that the data file exists
			entry = new JarCacheEntry();
			entry.indexFile = f;
			entry.url = raf.readUTF();
			entry.lastModified = raf.readLong();
			entry.expiration = raf.readLong();
			int type = raf.readInt();
			if(type == FileType.JAR) {
			    entry.version = raf.readUTF();
			}
			//make sure that the data file exists
			File dataFile = Cache.getDataFile(f, entry.url);
			if(dataFile.exists()) {
			    entry.dataFile = dataFile;
			}else {
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
        });
    }

    private static final Object privileged(CacheIOAction action)
    throws IOException {
	try {
	    return AccessController.doPrivileged(action);
	} catch (PrivilegedActionException pae) {
	    throw (IOException)pae.getException();
	}
    }

    // Class used for performing cache IO operations
    private static class CacheIOAction implements PrivilegedExceptionAction {
	public Object run() throws IOException {
	    return null;
	}
    }
}



