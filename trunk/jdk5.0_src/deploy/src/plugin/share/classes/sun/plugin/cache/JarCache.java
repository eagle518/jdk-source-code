/*
 * @(#)JarCache.java	1.44 04/03/11
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package sun.plugin.cache;

import java.io.File;
import java.io.RandomAccessFile;
import java.io.IOException;
import java.lang.ref.Reference;
import java.lang.ref.ReferenceQueue;
import java.lang.ref.WeakReference;
import java.net.URL;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.util.Enumeration;
import java.util.Iterator;
import java.util.Hashtable;
import java.util.ArrayList;
import java.util.jar.JarFile;
import sun.plugin.util.UserProfile;
import com.sun.deploy.config.Config;

/**
 * This class manages the JAR file cache on the local disk
 *
 */
public class JarCache 
{
    // The cache directory
    protected static File directory;

    // Level of compression to use for cache files.
    // 0 = no compression, 9 = best compression
    protected static int compression;

    // Currently loaded jar files
    protected static Hashtable loadedJars = new Hashtable();
    
    // We test each jar to see if it's cached and up to date.  If so, we
    // store it here and use this reference later.  This saves us multiple
    // hits to the server.
    protected static Hashtable currentJars = new Hashtable();

    // If a jar file is cached and up to date, i.e. added to currentJars,
    // then store the version so that it can be used during loads &gt; 1
    // if the CachedJarFile has been gc'ed.
    protected static Hashtable jarVersions = new Hashtable();

    protected static long lastCacheModifyTime = 0;
    protected static Hashtable filesInCache = new Hashtable();
    
    // Reference queue to handle weak references in hashtable
    protected static ReferenceQueue refQueue = new ReferenceQueue();

    // File extensions for cache files
    protected static final String DATA_FILE_EXT = ".zip";
    protected static final String JAR_FILE_EXT = ".jar";
    protected static final String JARJAR_FILE_EXT = ".jarjar";
    protected static final String META_FILE_DIR = "meta-inf/";

    // Properties to control caching
    protected static final String COMPRESSION_PROP = "javaplugin.cache.compression";

    static {
        AccessController.doPrivileged(new PrivilegedAction() {
            public Object run() {
		//initialize common cache
		Cache.init();

                // Locate the jar cache directory
                if (!Cache.cachingDisabled) {
                    directory = null;
                    String prop = UserProfile.getJarCacheDirectory();
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

                    if (success) {
			Cache.createTable(directory, filesInCache);

			// Get cache compression level.  Default is 0.
			prop = Config.getProperty(Config.CACHE_COMPRESSION_KEY);
			prop.trim();
			try {
			    compression = Integer.valueOf(prop).intValue();
			    if ((compression < 0) || (compression > 9)) {
				compression = 0;
			    }
			} catch (NumberFormatException e) {
			    compression = 0;
			}

			Cache.msgPrintln("cache.compression", new Object[] {String.valueOf(compression)});
		    }
		}
		return null;
            }
        });
    }

    // This class cannot be instantiated.  All methods are static.
    public JarCache() {
    }

    /**
     * Initialize the JAR cache
     */
    public static void init() {
        // Note that this method is a no-op, it just serves to load the
        // JarCache class and cause the static initializer to be invoked.
    }

    // Get the jar file with the given url
    public static JarFile get(URL url) 
	throws IOException {
        JarFile jar = null;

	//If caching is disabled, simply return null
	if(Cache.cachingDisabled){
	  return null;
	}
        
        // Make sure caching is supported for this jar file
        if (isSupported(url)) {
	    try {
		Cache.checkPermission(url.openConnection());
	    }catch(IOException e) {
		// invalid url
		return null;
	    }
            
            // Check if this jar file has already been loaded
            JarReference ref = (JarReference)loadedJars.get(url);
            if (ref != null) {
                jar = (CachedJarFile)ref.get();
            }
        
            // Otherwise, load it
            if (jar == null) {
		// See if this jar's already been processed.
		CachedJarLoader loader =
		    (CachedJarLoader) currentJars.remove(url);
		if (loader == null) {
		    FileVersion jarVersion;
		    loader = new CachedJarLoader(url, false);
		    if ((jarVersion = (FileVersion)jarVersions.get(url)) != null) {
			if (loader.isVersionUpToDate(jarVersion) == false)
			    loader.setVersion(jarVersion);
		    } 
		}

		jar = loader.load();
                
		// Store the jar in the list of loaded jar files.  
		// We use weak references to allow the JarFile object to
		// be unloaded when they are no longer in use by any class
		// loaders.
		loadedJars.put(url, new JarReference(jar, refQueue, url));
            
		// Clean up the reference queue
		while ((ref = (JarReference)refQueue.poll()) != null) {
		    loadedJars.remove(ref.getURL());
		}
            }
        }
        return jar;
    }

    // Checks whether caching is supported for the given URL.
    protected static final boolean isSupported(URL url) {
	return Cache.isSupportedProtocol(url);
    }

    // Based upon Matt's method of the same name from JDk 1.1.8.  Used to
    // determine the size of the jar file.  Returns 0 if the jar file is
    // cached, -1 if the size is unknown, or >0 if the size is determined.
    protected static long getJarSize(URL url) throws IOException {
        long size = -1;
        
        // Check our protocol support.
        if (isSupported(url)) {
            if (Cache.cachingDisabled) {
                size = Cache.getFileSizeFromServer(url);
            } else {
                CachedJarLoader loader = new CachedJarLoader(url, true);
                size = loader.getJarSize();
                currentJars.put(url, loader);
            }
        }
        return size;
    }


    // This method is used to check whether the cached jar file is upto date.
    // If the jar file is not upto date or the file is not in cache, version 
    // of the jar file is set in CachedJarLoader. 
    //
    // Return value is true if the jar file is in memory
    protected static boolean handleVersion(URL url, FileVersion jarVersion) 
					throws IOException  {
	boolean result = false;
        // Check our protocol support.
        if (isSupported(url) && !Cache.cachingDisabled) {
	    jarVersions.put(url, jarVersion);
            CachedJarLoader loader = new CachedJarLoader(url, false);
	    JarReference ref = (JarReference)loadedJars.get(url);
	    if(loader.isVersionUpToDate(jarVersion) == false) {
		loader.setVersion(jarVersion);

		// Check if this jar file has already been loaded, 
		// if so remove it from the loaded jars
		if (ref != null) {
		    loadedJars.remove(url);
		    result = true;//bug#4638771
		}
                currentJars.put(url, loader);
	    } else {
		/* If the version check passes and the jar file is not yet loaded,
		   use the initialized jar loader to load it */
		if(ref == null) {
		    currentJars.put(url, loader);
		}
	    }
        }
	return result;
    }

    // Gets the data file corresponding to the given index file
    protected static final File getDataFile(File f) {
        String name = f.getName();
        name = name.substring(0, name.length() - Cache.INDEX_FILE_EXT.length());
        name += DATA_FILE_EXT;
        return new File(f.getParentFile(), name);
    }

    // Generate a new cache file name
    protected static String generateCacheFileName(final URL url) throws IOException {
	return Cache.generateCacheFileName(directory, url);
    }

    public static void clearLoadedJars() {
	loadedJars.clear();
	currentJars.clear();
	jarVersions.clear();
	//Make sure objects in reference queue are removed
	while(refQueue.poll() != null);
    }

    // Returns a list of files which match the given criteria
    protected static Enumeration getMatchingFiles(URL url) {
	return Cache.getMatchingFiles(directory, url);
    }

    protected static boolean getMatchingFile(CachedJarLoader loader) throws IOException{
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

    protected synchronized static boolean match(CachedJarLoader loader, String key) throws IOException{
	boolean retVal = false;
	ArrayList list = (ArrayList)filesInCache.get(key);
	if(list != null) {
	    Iterator iter = list.iterator();
	    while(iter.hasNext()) {
		File file = (File)iter.next();
		// Make sure the data file exists
		File df = FileCache.getDataFile(file, loader.getURL());
		if(file.exists() && df.exists()) {
		    retVal = verifyFile(file, loader);
		}else {
		    iter.remove();
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

    protected static boolean verifyFile(File f, CachedJarLoader loader)throws IOException {
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
		    String ver = raf.readUTF();
		    loader.setVersion(new FileVersion(ver));
		}
	    }
	} catch (IOException e) {
	    // Exception when reading the cache file.
	    // Ignore this, we'll use a different file.
        } catch (JarCacheVersionException e) {
            // Invalid jar version in the cache file
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

    // Weak reference to a jar file
    protected static class JarReference extends WeakReference {
        URL url;
        
        public JarReference(Object obj, ReferenceQueue q, URL url) {
            super(obj, q);
            this.url = url;
        }
        
        public URL getURL() {
            return url;
        }
    }
}
