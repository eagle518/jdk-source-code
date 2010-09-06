/*
 * @(#)FileCache.java	1.21 04/03/11
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package sun.plugin.cache;

import java.io.File;
import java.io.RandomAccessFile;
import java.io.IOException;
import java.io.InputStream;
import java.io.FileInputStream;
import java.io.BufferedInputStream;
import java.io.DataInputStream;
import java.lang.ref.Reference;
import java.lang.ref.ReferenceQueue;
import java.lang.ref.WeakReference;
import java.net.URL;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Map;
import java.util.HashMap;
import java.util.ArrayList;
import sun.plugin.util.UserProfile;
import sun.net.www.MessageHeader;

/**
 * This class manages the JAR file cache on the local disk
 *
 */
public class FileCache
{
    // The cache directory
    protected static File directory;

    // Currently loaded jar files
    protected static Hashtable loadedFiles = new Hashtable();
    
    // Reference queue to handle weak references in hashtable
    protected static ReferenceQueue refQueue = new ReferenceQueue();

    // Different extensions supported by file cache
    protected static final String[] exts = { ".class", ".gif", ".jpg", ".au", ".wav" };

    protected static long lastCacheModifyTime = 0;
    protected static Hashtable filesInCache = new Hashtable();

    static {
        AccessController.doPrivileged(new PrivilegedAction() {
            public Object run() {
		//initialize common cache
		Cache.init();

                // Locate the file cache directory
                if (!Cache.cachingDisabled) {
                    directory = null;
                    String prop = UserProfile.getFileCacheDirectory();
                    if (prop != null) {
                        // The default cache directory has been overridden
                        directory = new File(prop.trim());
                    } 

                    // Ensure that the cache directory exists and that we
                    // have access.
                    boolean success = false;
                    if (directory != null) {
                        success = directory.exists();
                        if (!success) {
                            // The cache directory does not exist.  Create it.
                            success = directory.mkdirs();
                            if (!success) {
                                Cache.msgPrintln("cache.create_warning", new Object[] {directory});
				Cache.cachingDisabled = true;
                            }
                        } else {
                            // The cache directory already exists.  Make sure
                            // we have the necessary access to it.
                            if (!directory.canRead()) {
                                success = false;
                                Cache.msgPrintln("cache.read_warning", new Object[] {directory});
                            } else if (!directory.canWrite()) {
                                success = false;
                                Cache.msgPrintln("cache.write_warning", new Object[] {directory});
                            } else if (!directory.isDirectory()) {
                                success = false;
                                Cache.msgPrintln("cache.directory_warning", new Object[] {directory});
                            }
                        }
                    }

		    if(success) {
			Cache.createTable(directory, filesInCache);
		    }
		}
		return null;
            }
        });
    }

    // This class cannot be instantiated.  All methods are static.
    public FileCache() {
    }

    /**
     * Initialize the JAR cache
     */
    public static void init() {
        // Note that this method is a no-op, it just serves to load the
        // JarCache class and cause the static initializer to be invoked.
    }

    // Get the jar file with the given url
    public static CachedFile get(URL url) throws IOException{
        CachedFile file = null;

	//If caching is disabled, simply return null
	if(Cache.cachingDisabled){
	  return null;
	}

        // Make sure caching is supported for this file
        if (isSupported(url)) {
            
            // Check if this jar file has already been loaded
            FileReference ref = (FileReference)loadedFiles.get(url);
            if (ref != null) {
                file = (CachedFile)ref.get();
            }
        
            // Otherwise, load it
            if (file == null) {
		file = (new CachedFileLoader(url)).load();
                       
                // Store the jar in the list of loaded jar files.  
                // We use weak references to allow the JarFile object to
                // be unloaded when they are no longer in use by any class
                // loaders.
                loadedFiles.put(url, new FileReference(file, refQueue, url));
                
                // Clean up the reference queue
                while ((ref = (FileReference)refQueue.poll()) != null) {
                    loadedFiles.remove(ref.getURL());
                }
            }
        }
        
	return file;
    }


    // Checks whether caching is supported for the given URL.
    protected static final boolean isSupported(URL url) {
	boolean supported = false;

	//Check for file extension support
	if(Cache.isSupportedProtocol(url)) {
	    //If jar file, we should not handle it;JarCache handles it
	    //Cache only if the jar files are of the 
	    String ext = Cache.getFileExtension(url.toString());
	    if(ext != null) {
		for(int i=0;i<exts.length;i++ ) {
		    if(ext.equalsIgnoreCase(exts[i])) {
			supported = true;
		    }
		}
	    }
	}

	return supported;
    }

    // Gets the data file corresponding to the given index file
    protected static final File getDataFile(File f, URL url) {
        String name = f.getName();
        name = name.substring(0, name.length() - Cache.INDEX_FILE_EXT.length());
        name += Cache.getFileExtension(url.toString());
        return new File(f.getParentFile(), name);
    }

    // Generate a new cache file name
    protected static String generateCacheFileName(final URL url) throws IOException {
	return Cache.generateCacheFileName(directory, url);
    }

    // Returns a list of files which match the given criteria
    protected static Enumeration getMatchingFiles(URL url) {
	return Cache.getMatchingFiles(directory, url);
    }

    public static void clearLoadedFiles() {
	loadedFiles.clear();
	//Make sure objects in reference queue are removed
	while(refQueue.poll() != null);
    }

    // Weak reference to a jar file
    protected static class FileReference extends WeakReference {
        URL url;
        
        public FileReference(Object obj, ReferenceQueue q, URL url) {
            super(obj, q);
            this.url = url;
        }
        
        public URL getURL() {
            return url;
        }
    }

    protected static boolean getMatchingFile(CachedFileLoader loader) throws IOException{
	boolean retVal = false;
	String key = Cache.getKey(loader.getURL());
	retVal = match(loader, key);
	if(!retVal) {	    
	    if(Cache.updateTable(directory, filesInCache, key)){
		retVal = match(loader, key);
	    }
	}
	return retVal;
    }

    protected synchronized static boolean match(CachedFileLoader loader, String key) throws IOException{
	boolean retVal = false;
	ArrayList list = (ArrayList)filesInCache.get(key);
	if(list != null) {
	    for(int i=0;i<list.size();i++) {
		File file = (File)list.get(i);
		// Make sure the data file exists
		File df = FileCache.getDataFile(file, loader.getURL());
		if(file.exists() && df.exists()) {
		    retVal = verifyFile(file, loader);
		}else {
		    list.remove(i);
		    if(list.size() == 0)
			filesInCache.remove(key);
		}
		//break if match is found
		if(retVal) {
		    loader.setDataFile(df);
		    break;
		}
	    }
	}
	return retVal;
    }

    protected static boolean verifyFile(File f, CachedFileLoader loader)throws IOException {
        // Check each file to see if it is a match
        boolean found = false;
	RandomAccessFile raf = new RandomAccessFile(f, "r");
	try {
	    // Check the version of the file to make sure
	    // we know how to read it.
	    if (raf.readByte() == Cache.VERSION) {
		// Check the URL to see if it is a match
		String fileURL = raf.readUTF();
		if (fileURL.equals(loader.getURL().toString())) {
		    // We've found a matching file
		    found = true;
		    loader.setIndexFile(f);
		    loader.setLastModify(raf.readLong());
		    loader.setExpiration(raf.readLong());
		    raf.readInt();//ignore file type 
		    loader.setHeaders(readHeaderFields(raf));			
		}
	    }
	} catch (IOException e) {
	    // Exception when reading the cache file.
	    // Ignore this, we'll use a different file.
	} finally {
	    try {
		raf.close();
		raf = null;
	    } catch (IOException e) {
		// Ignore
	    }
	}
        return found;
    }


    public static MessageHeader readHeaderFields(RandomAccessFile raf) throws IOException{
	MessageHeader headers = new MessageHeader(); 

        // Create an input stream
	BufferedInputStream in = new BufferedInputStream(new FileInputStream(raf.getFD()) );
	DataInputStream dis = new DataInputStream(in);

	//Read the header fields as name-value paris
        try {
	    for(int count = dis.readInt();count > 0; count--) {
			String name = dis.readUTF();
			if(name.equals("<null>"))
				name = null;
			headers.add(name, dis.readUTF());
	    }
	} finally {
	    //do not close the stream since raf could be used
	    //by the calling method
	}

	return headers;
    }
}


