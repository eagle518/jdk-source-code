/*
 * @(#)Cache.java	1.24 04/01/16
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package sun.plugin.cache;

import java.io.File;
import java.io.FileFilter;
import java.io.IOException;
import java.net.URL;
import java.net.URLConnection;
import java.security.AccessController;
import java.security.cert.Certificate;
import java.security.Permission;
import java.security.PrivilegedAction;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;
import java.text.MessageFormat;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Random;
import java.net.HttpURLConnection;
import sun.plugin.util.UserProfile;
import sun.plugin.util.Trace;
import sun.security.action.GetPropertyAction;
import com.sun.deploy.config.Config;


/**
 * This class manages the JAR file cache on the local disk
 *
 */
public class Cache 
{
    // The maximum size of the cache directory
    protected static long maxSize;
    
    // Cleanup threshould size
    protected static long threadThresholdSize;

    // Cleanup threshould size
    protected static long deleteThresholdSize;

    // Whether or not jar caching is disabled.
    protected static boolean cachingDisabled = false;

    // Random number generator used to generate cache file names
    protected static Random random = new Random();

    // Thread for doing cache cleanup
    protected static CleanupThread cleanupThread = null;
    
    //Indicates whether the cached file is ready to use
    protected static final byte INCOMPLETE = 0;
    
    //Inidcates whether the file is marked as outdated.
    protected static final byte UNUSABLE = 1;

    //Inidcates whether the file is being currently used
    protected static final byte INUSE = 2;

    // Cache file format version.  This value should be incremented
    // whenever a change is made to the format of cached jar files
    // which is not compatible with previous versions of the code.
    protected static final byte VERSION = 16;
   
    // File extensions for cache files
    protected static final String INDEX_FILE_EXT = ".idx";

    // Cleanup thread time interval in minutes
    protected static final long CLEANUP_INTERVAL = 1;

    protected static final long MINIMUM_CACHE_SIZE = 5;

    // Properties to control caching
    protected static final String ENABLED_PROP    = Config.CACHE_ENABLED_KEY;
    protected static final String SIZE_PROP        = Config.CACHE_MAX_KEY;
      
    // Default strings
    protected static String KB="{0} KB";
    protected static String MB="{0} MB";
    protected static String BYTES="{0} bytes";
    protected static String UNLIMITED="unlimited";

    static {
        // Check if the jar cache has been disabled by the user
        String prop = (String) AccessController.doPrivileged(new GetPropertyAction(ENABLED_PROP, "true"));

        // Redefined disabled as class variable cachingDisabled.
        Cache.cachingDisabled = prop.equalsIgnoreCase("false");

        // Locate the cache directory
        if (Cache.cachingDisabled) {
            // Caching is disabled.
            Cache.msgPrintln("cache.disabled", null);
        }
        else{  
            // Caching is enabled.
                    
            // Get the cache size.  Default is 50MB.
            prop = (String) AccessController.doPrivileged(new GetPropertyAction(Config.CACHE_MAX_KEY, "50m"));

            prop.trim();
            long factor;
            MessageFormat formatter;
            if (prop.endsWith("M") || prop.endsWith("m")) {
                // Size is in MegaBytes
                factor = 1024*1024;
                prop = prop.substring(0, prop.length() - 1);
                formatter = new MessageFormat(MB);
            } else if (prop.endsWith("K") || prop.endsWith("k")) {
                // Size is in KiloBytes
                factor = 1024;
                prop = prop.substring(0, prop.length() - 1);
                formatter = new MessageFormat(KB);
            } else {
                // Size is in bytes
                factor = 1;
                formatter = new MessageFormat(BYTES);
            }

            long size;
            try {
                // Parse the size
                size = Long.valueOf(prop).longValue();
                maxSize = factor * size;
            } catch (NumberFormatException e) {
                // Default to 50 MB if there is an error
                size = 50;
                maxSize = 50 * 1024* 1024;
                formatter = new MessageFormat(MB);
            }
                
            String displaySize;
            if (size > 0) {
                // The cache has a maximum size
                displaySize =
                        formatter.format(new Object[]{new Long(size)});
                        
                // Start the cache cleanup thread in the system
                // thread group.
                ThreadGroup group = 
                            Thread.currentThread().getThreadGroup();
                while (group.getParent() != null) {
                    group = group.getParent();
                }

                //Use			
                if(maxSize >= MINIMUM_CACHE_SIZE * 1024 * 1024) {
                    threadThresholdSize = maxSize - (1 * 1024 * 1024);
                    deleteThresholdSize = maxSize - (2 * 1024 * 1024);
                    cleanupThread = new CleanupThread(group, CLEANUP_INTERVAL * 60 * 1000);
                    cleanupThread.start();
                }
                else {
                    cachingDisabled = true;
                    msgPrintln("cache.minSize",
                                new Object[] {displaySize });
                    displaySize = null;
                }

            } else {
                // The cache size is unrestricted
                displaySize = UNLIMITED;                
            }

            if(displaySize != null){
                msgPrintln("cache.enabled", null);
                msgPrintln("cache.location",
                            new Object[] { UserProfile.getPluginCacheDirectory() });
                msgPrintln("cache.maxSize",
                            new Object[] {displaySize });
            }
        }//end else: caching enabled.
    }

    // This class cannot be instantiated.  All methods are static.
    public Cache() {
    }

    /**
     * Initialize the JAR cache
     */
    public static void init() {
        // Note that this method is a no-op, it just serves to load the
        // Cache class and cause the static initializer to be invoked.
    }

    // Checks whether caching is supported for the given URL.
    protected static final boolean isSupportedProtocol(URL url) {
        // Currently, we only support caching for HTTP and HTTPS
        String protocol = url.getProtocol();
        if ((protocol != null) &&
            (protocol.equalsIgnoreCase("http") ||
             protocol.equalsIgnoreCase("https"))) {
            return true;
        } else {
            return false;
        }
    }


    // Display a message to the console
    protected static final void msgPrintln(String resource, Object[] params) {
        Trace.msgPrintln(resource, params);
        //System.out.println(formatter.format(params));
    }

    // Gets the data file corresponding to the given index file
    protected static final File getIndexFile(File f, URL url) {
        String name = f.getName();
        name = name.substring(0, name.length() - getFileExtension(url.toString()).length());
        name += INDEX_FILE_EXT;
        return new File(f.getParentFile(), name);
    }


    // Gets the data file corresponding to the given index file
    protected static final File getDataFile(File f, String url) {
        String name = f.getName();
        name = name.substring(0, name.length() - Cache.INDEX_FILE_EXT.length());
        name += Cache.getFileExtension(url);
        return new File(f.getParentFile(), name);
    }


    // It returns the extension of the file to which URL points
    protected static final String getFileExtension(String name) {
	String ext = "";
	//get the url extension
	int extIndex = name.lastIndexOf('.');
	if(extIndex != -1) {
	    ext = name.substring(extIndex);
	}
	
	//.jar and .jarjar files are stored as .zip
	if( ext.equalsIgnoreCase(JarCache.JAR_FILE_EXT) || 
	    ext.equalsIgnoreCase(JarCache.JARJAR_FILE_EXT) ) {
	    ext = JarCache.DATA_FILE_EXT;
	} 

	return ext;
    }

    // Generate a new cache file name
    protected static String generateCacheFileName(File directory, final URL url) throws IOException {
        // Get the cache key for this URL
        String key = getKey(url);

        // Generate random filenames off of this key until a new file is found.
        // We use the atomic createNewFile() method to ensure an exclusive
        // lock on this filename.
        String filename;
	File dataFile, indexFile;
	do {
            filename = key + Integer.toString(getRandom(), 16);
	    dataFile = new File(directory, filename + getFileExtension(url.toString()));
	    indexFile = new File(directory, filename + INDEX_FILE_EXT);
        } while (indexFile.exists() || dataFile.exists());
	indexFile = null;
	dataFile = null;
        return filename;
    }

    // Returns a list of files which match the given criteria
    protected static Enumeration getMatchingFiles(File directory, URL url) {

        // Get the cache key for this URL
        final String key = getKey(url);

        // Get the list of files that match this name
        final File[] files = directory.listFiles(new FileFilter() {
            public boolean accept(File pathname) {
                return pathname.getName().startsWith(key);
            }
        });

        // Wrap the list in an enumeration
        return new Enumeration() {
            private int index = 0;
            public boolean hasMoreElements() {
                return (index < files.length);
            }
            public Object nextElement() {
                return files[index++];
            }
        };
    }

    // Touch the given file
    protected static final void touch(final File file) throws IOException {
        try {
            AccessController.doPrivileged(new Cache.CacheIOAction() {
                public Object run() throws IOException {
                    file.setLastModified(System.currentTimeMillis());
                    //return value is not used
                    return null;
                }
            });
        }// end try
        catch (PrivilegedActionException pae) {
            throw new IOException(pae.getMessage());
        }
    }
                        
    // Returns the key used to look a given url up in the cache
    protected static String getKey(URL url) {

        // Get the name of the remote file from the URL
        String file = url.getPath();
        int separator = file.lastIndexOf("/");
        if (separator != -1) {
            file = file.substring(separator + 1);
        }

        // Generate a positive hash value for this URL
        int hashCode = hashCode(url);
        if (hashCode < 0) {
            hashCode += Integer.MAX_VALUE + 1;
        }

        // Combine the file name and hash to generate the cache key
        return file + "-" + Integer.toString(hashCode, 16) + "-";
    }

    // Returns the key used to look a given url up in the cache
    protected static String parseKey(String fileName) {
	String key = null;
        int separator = fileName.lastIndexOf("-");
        if (separator != -1) {
            key = fileName.substring(0, separator+1);
        }
	return key;
    }

    // Returns a postive, eight digit hexadecimal random number
    protected static final int getRandom() {
        return 0x10000000 + random.nextInt(Integer.MAX_VALUE - 0x10000000);
    }

    // Check that the caller has permission to make the given URL connection
    protected static void checkPermission(URLConnection uc)
        throws IOException {

        Permission perm = uc.getPermission();
        URL url = uc.getURL();
        if (perm != null) {
            SecurityManager sm = System.getSecurityManager();
            if (sm != null) {
                try {
                    sm.checkPermission(perm);
                } catch (SecurityException se) {
                    // fallback to checkRead/checkConnect for pre 1.2
                    // security managers
                    if ((perm instanceof java.io.FilePermission) &&
                        perm.getActions().indexOf("read") != -1) {
                        sm.checkRead(perm.getName());
                    } else if ((perm instanceof
                        java.net.SocketPermission) &&
                        perm.getActions().indexOf("connect") != -1) {
                        sm.checkConnect(url.getHost(), url.getPort());
                    } else {
                        throw se;
                    }
                }
            }
        }
    }

    // Used to snag the size of the jar file from the server.
    protected static long getFileSizeFromServer(URL url) throws IOException {
        long size = -1;
        
        HttpURLConnection c = (HttpURLConnection) url.openConnection();
        c.setUseCaches(false);
        c.setRequestMethod("HEAD");
        size = c.getContentLength();
        c.disconnect();
        
        return size;
    }

    //adds a file into cache table
    protected static void addToTable(File indexFile, Hashtable table) {
	//update the files in cache table
	String key = parseKey(indexFile.getName());
	synchronized (table) {
	    ArrayList list = (ArrayList)table.get(key);
	    if(list == null) {
	        list = new ArrayList();
	        list.add(indexFile);
	        table.put(key, list);
	    } else {
	        if(list.indexOf(indexFile) == -1) {
		    list.add(indexFile);
	        }
	    }
	}
    }

    //removes a file from cache table
    protected static void removeFromTable(File indexFile, Hashtable table) {
	//update the files in cache table
	String key = parseKey(indexFile.getName());
	synchronized (table) {
	    ArrayList list = (ArrayList)table.get(key);
	    if(list != null) {
	        int index = list.indexOf(indexFile);
	        if(index != -1)
		    list.remove(index);
	        if(list.size() == 0)
		    table.remove(key);
	    }
	}
    }

    //adds any files starting with a given key into the cache table
    protected static boolean updateTable(File directory, Hashtable table, final String key) {
	boolean retVal = false;
	File []files = directory.listFiles(new FileFilter() {
	    public boolean accept(File file) {
		return ( file.getName().endsWith(Cache.INDEX_FILE_EXT) &&
			 file.getName().startsWith(key) );
	    }
	});
	
	//update the files in cache table
	for(int i=0;i<files.length;i++) {
	    synchronized (table) {
	        ArrayList list = (ArrayList)table.get(key);
	        if(list == null) {
		        list = new ArrayList();
		        list.add(files[i]);
		        table.put(key, list);
		        retVal = true;
	        } else {
		        if(list.indexOf(files[i]) == -1) {
		        list.add(files[i]);
		        retVal = true;
		        }
	        }
	    }
	}
	return retVal;
    }

    //Creates the list of files in cache
    protected static void createTable(File directory, Hashtable table) {
	//long begin = System.currentTimeMillis();
	File []files = directory.listFiles(new FileFilter() {
	    public boolean accept(File file) {
		return file.getName().endsWith(Cache.INDEX_FILE_EXT);
	    }
	});
	
	//update the files in cache table
	for(int i=0;i<files.length;i++) {
	    String key = parseKey(files[i].getName());
	    ArrayList list = (ArrayList)table.get(key);
	    if(list == null) {
		list = new ArrayList();
		list.add(files[i]);
		table.put(key, list);
	    } else {
		list.add(files[i]);
	    }
	}
    }

    // Provides the hashing algorithm for urls to be used in the cache filename
    // Independant of the host address
    protected static int hashCode(URL u) {
	int h = 0;

	// Generate the protocol part.
	String protocol = u.getProtocol();
	if (protocol != null)
	    h = protocol.hashCode();

	// Generate the host part.
	String host = u.getHost();
	if (host != null)
	    h = host.toLowerCase().hashCode();

	// Generate the file part.
	String file = u.getFile();
	if (file != null)
	    h += file.hashCode();

	// Generate the port part.
	if (u.getPort() == -1)
	    h += u.getDefaultPort();
	else
	    h += u.getPort();

	// Generate the ref part.
	String ref = u.getRef();
	if (ref != null)
	    h += ref.hashCode();

	return h;
    }

    // Class used for performing cache IO operations
    protected static class CacheIOAction implements PrivilegedExceptionAction {
        public Object run() throws IOException {
            return null;
        }
    }
}




