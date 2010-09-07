/*
 * @(#)Cache.java	1.107 10/03/24
 *
 * Copyright (c) 2005, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.cache;

import java.io.*;
import java.net.*;
import java.security.SecureRandom;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.lang.reflect.InvocationTargetException;
import java.util.*;
import java.util.jar.JarFile;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;
import com.sun.deploy.util.VersionString;
import com.sun.deploy.config.Config;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.services.ServiceManager;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.net.*;
import com.sun.deploy.net.DownloadEngine.DownloadDelegate;
import com.sun.deploy.Environment;

public class Cache {
    
    private static boolean doIPLookup = true;
    
    // use 0.0.0.0 to indicate the hostname cannot be resolved
    private final static String IP_ADDR_CANNOT_RESOLVE = "0.0.0.0";
    
    private final static int VERSION_INT = 603;
    private final static String VERSION_STRING = "6.0";
    private final static String DASH = "-";
    final static String INDEX_FILE_EXT = ".idx";
    private final static String MUFFIN_FILE_EXT = ".muf";
    private final static String HOST_FILE_EXT = ".hst";
    private final static int NUM_OF_CACHE_SUBDIR = 64;
    
    final static char APPLICATION_TYPE  = 'A';  // Application-Desc
    final static char EXTENSION_TYPE    = 'E';  // Extension-Desc

    // Get the platform dependent Random generator
    private static SecureRandom random = null;
        
    private static String cachePath;
    
    private static File cacheDir;
    private static File sysCacheDir;
    private static File muffinDir;
    private static File hostDir;
    
    private final static int BUF_SIZE = 32 * 1024;
    
    /**
     * Maps from path (either URL, or path on file system) to
     * LocalApplicationProperties.
     */
    private final static Map loadedProperties = new HashMap();
    private static CleanupThread ct;
    
    private final static Object syncObject = new Object();
    
    private static boolean cleanupEnabled = true;
    
    // time to wait before starting cached JAR preverification in ms
    public final static long TIME_WAIT_BEFORE_JAR_PERVERIFY = 30000;
    
    public static void setCleanupEnabled(boolean enabled) {
        cleanupEnabled = enabled;
    }
    
    static {
        reset();
	
        // only start clean-up thread if cache enabled
        if (isCacheEnabled()) {
            ct = new CleanupThread("CacheCleanUpThread", syncObject);
            ct.start();
        }
     }
    
    public static void setDoIPLookup(boolean b) {
        doIPLookup = b;
    }
    
    private static void createCacheBucketDirectories(String cacheDirPath) {
        for (int i = 0; i < NUM_OF_CACHE_SUBDIR; i++) {
            File cacheSubDir = new File(cacheDirPath + File.separator + i);
            cacheSubDir.mkdir();
        }
    }
    
    public static void reset() {
        synchronized (syncObject) {
            MemoryCache.reset();
            synchronized (loadedProperties) {
                loadedProperties.clear();
            }

            // initialize cache/sysCache/muffin directory path
            cachePath = Config.getCacheDirectory() +
                    File.separator + VERSION_STRING;
            
            String muffinPath = cachePath + File.separator + "muffin";
            
            String hostPath = cachePath + File.separator + "host";
            
            cacheDir = new File(cachePath);
            muffinDir = new File(muffinPath);
            hostDir = new File(hostPath);
            
            cacheDir.mkdirs();
            
            hostDir.mkdirs();
            
            // generate directory for each cache bucket
            createCacheBucketDirectories(cachePath);
            
            muffinDir.mkdirs();
            
            if (Config.getSystemCacheDirectory() != null &&
                    Config.getSystemCacheDirectory().length() != 0) {
                String sysCachePath = Config.getSystemCacheDirectory() +
                        File.separator + VERSION_STRING;
                sysCacheDir = new File(sysCachePath);
                if (Environment.isSystemCacheMode()) {
                    sysCacheDir.mkdirs();

                    // generate directory for each cache bucket
                    createCacheBucketDirectories(sysCachePath);
                }
            } else {
                sysCacheDir = null;
            }
            
            long cacheMax = Config.getCacheSizeMax();
            // make the max 5Mb if size is less then 5MB and not infinte (-1)
            if (cacheMax > 0 && cacheMax < 5 * 1024 * 1024) {
                cacheMax = 5 * 1024 * 1024;
            }
        }
    }
    
    static void addToCleanupThreadLoadedResourceList(String url) {
        if (ct != null && cleanupEnabled) {
	    ct.addToLoadedResourceList(url); 
	}
    }
    
    static void cleanup() {
	if (ct != null && cleanupEnabled) {
	    ct.startCleanup();	    
	}
    }
    
    private Cache() {
    }
    
    static void markResourceIncomplete(CacheEntry ce) {
        if (ce != null) {
            synchronized (ce) {
                ce.setIncomplete(1);
                try {
                    ce.updateIndexHeaderOnDisk();
                } catch (IOException ioe) {
                    Trace.ignoredException(ioe);
                }
            }
        }
    }

    static boolean isSystemCacheEntry(CacheEntry ce) {
        if (ce != null && sysCacheDir != null) {
            File idx = ce.getIndexFile();
            if (idx != null && idx.getParentFile() != null) {
                //generateCacheFileName() will always create index 2 levels
                // below cache directory
                return sysCacheDir.equals(idx.getParentFile().getParentFile());
            }
        }
        return false;
    }

    public final static CacheEntry getSystemCacheEntry(URL url, String version) {

        if (url == null) {
            return null;
        }

        CacheEntry ce = (CacheEntry) MemoryCache.getLoadedResource(url.toString());
        if (isSystemCacheEntry(ce)) {
            String currentVersion = ce.getVersion();
            if ((version == null && currentVersion == null )||
                    (version != null && currentVersion != null &&
                    currentVersion.compareTo(version) >= 0)) {
                return ce;
            }
        }
        
        ce = Cache.getCacheEntry(url, null, version, sysCacheDir);

        if (ce != null) {
            MemoryCache.addLoadedResource(url.toString(), ce);
        }

        return ce;
    }

    public final static boolean isSupportedProtocol(URL url) {
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
    
    
    public static boolean isCacheEnabled() {
        return Config.getBooleanProperty(Config.CACHE_ENABLED_KEY);
    }
    
    public static void removeLoadedProperties(String key) {
         synchronized(loadedProperties) {
            loadedProperties.remove(key);
        }
    }
    
    /**
     * Returns the LocalApplciationProperties for the launch descriptor
     * loaded from the diskCacheEntry
     */
    public static LocalApplicationProperties getLocalApplicationProperties(
            CacheEntry ce) {
        URL jnlpURL = null;
        try {
            jnlpURL = new URL(ce.getURL());
        } catch (MalformedURLException mue) {
            Trace.ignoredException(mue);
        }
        return getLocalApplicationProperties(jnlpURL, ce.getVersion(), true);
    }
    
    /**
     * Returns the LocalApplciationProperties for the launch descriptor
     * loaded from the file pointed into the cache
     */
    public static LocalApplicationProperties getLocalApplicationProperties(
            String path) {
        if (Cache.isCacheEnabled() == false) {
            return null;
        }
        CacheEntry ce = com.sun.deploy.cache.Cache.getCacheEntryFromFile(
                new File(path + INDEX_FILE_EXT));
        if (ce == null) {
            return null;
        }
        URL u = null;
        try {
            u = new URL(ce.getURL());
        } catch (MalformedURLException mue) {
            Trace.ignoredException(mue);
            return null;
        }
        return getLocalApplicationProperties(u, ce.getVersion(), true);
    }
    
    /**
     * Returns the LocalApplicationProperties for the launch descriptor loaded
     * from <code>jnlpUrl</code>. Unless home is null, this will always return
     * a non-null value.
     */
    public static LocalApplicationProperties getLocalApplicationProperties(
            URL jnlpUrl) {
        return getLocalApplicationProperties(jnlpUrl, null, true);
    }
    
    /**
     * Returns the LocalApplicationProperties for the launch descriptor loaded
     * from <code>jnlpUrl</code>. Unless url is null, this will always return
     * a non-null value.
     */
    public static LocalApplicationProperties getLocalApplicationProperties(
            URL jnlpUrl, String version, boolean isApplicationDesc) {

        LocalApplicationProperties props = null;
        
        if (isCacheEnabled() && jnlpUrl != null) { 
        
            // Generating the hashcode for a URL requires a nameserver lookup,
            // i.e., we need to be online. This should fix the problem.
            String key = jnlpUrl.toString() + "?" + version;
        
            synchronized(loadedProperties) {
                props = (LocalApplicationProperties)loadedProperties.get(key);
                if (props == null) {
                    props = new DefaultLocalApplicationProperties(
                        jnlpUrl, version, isApplicationDesc);
                    loadedProperties.put(key, props);
                } else {
                     props.refreshIfNecessary();
                }
            }
        }
        return props;
    }

    public static String getLapFileName(URL url, String version) {
        String key = getKey(url);
        return getBucket(key) + File.separator + key + VERSION_STRING + 
                getVersionTag(version) + ".lap";
    }

    
    public static void putLapData(char type, URL url, String version,
            byte[] data) throws IOException  {
        
        File dir = getActiveCacheDir();
        File lapFile = new File(dir, getLapFileName(url, version));

        InputStream is = new ByteArrayInputStream(data);
        BufferedOutputStream bof = new BufferedOutputStream(
            new FileOutputStream(lapFile));
        byte[] buffer = new byte[BUF_SIZE];
        try {
            int n = is.read(buffer);
            while(n >= 0) {
                bof.write(buffer, 0, n);
                n = is.read(buffer);
            }
        } finally {
            bof.close();
            is.close();
        }
    }
    
    public static byte[] getLapData(char type, URL url, String version,
            boolean fromSystemCache) throws IOException  {

        File dir = (fromSystemCache) ? sysCacheDir : cacheDir;

	if (dir == null) {
	    return null;
	}
	
        File lapFile = new File(dir, getLapFileName(url, version));

        long size = lapFile.length();
        if (size > 0 && size < 1024 * 1024) {
            BufferedInputStream is = 
                new BufferedInputStream(new FileInputStream(lapFile));
            ByteArrayOutputStream baos = 
                new ByteArrayOutputStream((int)size);
        
            byte[] buffer = new byte[BUF_SIZE];
            try {
                int n = is.read(buffer);
                while(n >= 0) {
                    baos.write(buffer, 0, n);
                    n = is.read(buffer);
                }
            } finally {
                baos.close();
                is.close();
            }
            return baos.toByteArray();
        }
        return null;
    }
    
    /** Returns a new directory where to install an extension */
    public static String getNewExtensionInstallDirectory() throws IOException {
        String dir = cacheDir.getAbsolutePath() +
                File.separator + "ext";
        String tempname = null;
        int count = 0;
        do {
            tempname =  dir + File.separator + "E" +
                    (new Date().getTime()) + File.separator;
            // Create path
            File cacheDir = new File(tempname);
            if (!cacheDir.mkdirs()) {
                tempname = null;
            }
            Thread.yield(); // Just to improve scheduling.
        } while(tempname == null && ++count < 50); // max 50 attempts
        if (tempname == null) {
            throw new IOException("Unable to create temp. dir for extension");
        }
        return tempname;
    }
    
   
    
    public static String getCacheEntryVersion(final URL url, 
            final String resourceID) {
        String currentVersion = null;
        CacheEntry ce = Cache.getLatestCacheEntry(url, resourceID);
        if (ce != null) {
            currentVersion = ce.getVersion();
        }
        return currentVersion;
    }
    
    public static int getCacheEntryContentLength(URL href, String version) {
        CacheEntry ce = Cache.getCacheEntry(href, null, version);
        if (ce != null) {
            return ce.getContentLength();
        }
        return 0;
    }
    
    public static long getCacheEntryLastModified(URL href, String version) {
        CacheEntry ce = Cache.getCacheEntry(href, null, version);
        if (ce != null) {
            return ce.getLastModified();
        }
        return 0;
    }
    
    static CacheEntry downloadResourceToCache(final URL href, final String downloadVersion,
            final URLConnection conn, final URL requestURL, final boolean applyJarDiff, 
            final int contentType, 
            final InputStream is) throws
            IOException, CanceledDownloadException {
        CacheEntry ce = null;
        try {
            ce = (CacheEntry)AccessController.doPrivileged(new PrivilegedExceptionAction() {
                public Object run() throws IOException, CanceledDownloadException {
                 
                    // download the resource into the cache
                    String cacheFileName = generateCacheFileName(href, downloadVersion);
                    File dir = getActiveCacheDir();
                    
                    // Generate the index filename for this resource
                    File indexFile = new File(dir, cacheFileName + getIndexFileExtension());
                    
                    // create a new cache entry for this url and mark it busy
                    CacheEntry ce = new CacheEntry(indexFile);
                    ce.writeFileToDisk();
                    
                    String encoding = conn.getContentEncoding();
                    
                    if (DownloadEngine.isPackContentType(contentType)) {
                        // set encoding to pack200
                        encoding = HttpRequest.PACK200_GZIP_ENCODING;
                    }

                    DownloadEngine.getHttpDownloadImpl().download(
                            conn.getContentLength(), conn.getURL(), is,
                            encoding, ce.getTempDataFile(), null, contentType);
                    
                    // process the downloaded file
                    if (ce.processTempDataFile(applyJarDiff, null, href, requestURL,
                            downloadVersion)){
                        ce.setBusy(0);
                        ce.setIncomplete(0);
                        ce.setURL(downloadVersion == null ? 
                            requestURL.toString() : href.toString());                     
                        if (applyJarDiff) {
                            ce.setContentLength(
                                    (int)new File(ce.getResourceFilename()).length());
                        } else {
                            ce.setContentLength(conn.getContentLength());
                        }
                        ce.setLastModified(conn.getLastModified());
                        ce.setExpirationDate(conn.getExpiration());
                        if (downloadVersion != null) {
                            ce.setVersion(downloadVersion);
                        }
                        MessageHeader headerFields =
                                BasicHttpRequest.initializeHeaderFields(conn);
                        // workaround for java plugin
                        // use the request property header to help determine the
                        // JAR content type
                        if (conn instanceof HttpURLConnection) {
                            // make sure we disconnect first, otherwise
                            // getRequestProperty might throw
                            // illegalStateException
                            ((HttpURLConnection)conn).disconnect();
                            
                            String requestContentType =
                                    conn.getRequestProperty(HttpRequest.CONTENT_TYPE);
                       
                            if (requestContentType != null &&
                                    headerFields != null) {
                                headerFields.add(
                                        HttpRequest.DEPLOY_REQUEST_CONTENT_TYPE,
                                        requestContentType);
                            }
                        }
                        ce.setHeaders(headerFields);
                        // update the cache entry for it, resource is ready to use
                        ce.writeFileToDisk(contentType, null);
                        
                        recordLastAccessed();
                        
                        return ce;
                    }
                    return null;
                }
            });
        } catch (PrivilegedActionException pae) {
            if (pae.getException() instanceof IOException) {
                throw (IOException)pae.getException();
            } else if (pae.getException() instanceof CanceledDownloadException) {
                throw (CanceledDownloadException)pae.getException();
            }
        }
        return ce;
    }
    
    public static CacheEntry downloadResourceToCache(URL href, String downloadVersion,
            HttpResponse response, HttpDownloadListener hdl, 
            DownloadDelegate dd, boolean removeCurrentCE, URL requestURL,
            CacheEntry currentCE, boolean applyJarDiff, int contentType) 
	throws IOException, CanceledDownloadException {

	CacheEntry tempCE = downloadResourceToTempFile(href, downloadVersion, response, hdl,
						       dd, removeCurrentCE, requestURL,
						       currentCE, applyJarDiff, contentType);

	if (tempCE != null) {
	    return processNewCacheEntry(href, removeCurrentCE,
					tempCE, currentCE);
	}
	return null;
    }


    // Download resource to a temp cache entry
    // The temp cache entry is set to busy and incomplete. 
    // It is not supposed to be usable cache entry for this href
    // If a user inquiry Cache for the href, it should still get old resource
    public static CacheEntry downloadResourceToTempFile(URL href, String downloadVersion,
							HttpResponse response, HttpDownloadListener hdl,
							DownloadDelegate dd, boolean removeCurrentCE,
							URL requestURL, CacheEntry currentCE,
							boolean applyJarDiff, int contentType) 
	throws IOException, CanceledDownloadException {	 
       
        // download the resource into the cache
        String cacheFileName = generateCacheFileName(href, downloadVersion);
        File dir = getActiveCacheDir();
        
        // Generate the index filename for this resource
        File indexFile = new File(dir, cacheFileName + getIndexFileExtension());
        
        // create a new cache entry for this url and mark it busy
        CacheEntry ce = new CacheEntry(indexFile);
        ce.writeFileToDisk();
       
        String encoding = response.getContentEncoding();
        if (DownloadEngine.isPackContentType(contentType)) {
            // set encoding to pack200
            encoding = HttpRequest.PACK200_GZIP_ENCODING;
        }
        DownloadEngine.getHttpDownloadImpl().download(response.getContentLength(),
                response.getRequest(), response.getInputStream(),
                encoding, ce.getTempDataFile(), hdl, contentType);
        
        response.disconnect();
        
        // process the downloaded file
        if (ce.processTempDataFile(applyJarDiff, dd, href, requestURL,
				   downloadVersion)){
	    // if we are doing import, should always use the original
	    // codebase url from jnlp file.  requestURL will be substituted
	    // with the alternate codebase.
	    ce.setURL((downloadVersion == null && 
		       Environment.isImportMode() == false) ?
		      requestURL.toString() : href.toString());
	    MessageHeader headerFields = response.getHeaders();
	    if (headerFields != null && 
		DownloadEngine.isJarContentType(contentType)) {
		headerFields.add(HttpRequest.DEPLOY_REQUEST_CONTENT_TYPE,
				 HttpRequest.JAR_MIME_TYPE);              
	    }
	    ce.setHeaders(headerFields);
	    ce.setContentLength(response.getContentLength());
	    ce.setLastModified(response.getLastModified());
	    ce.setExpirationDate(response.getExpiration());
	    if (downloadVersion != null) {
		ce.setVersion(downloadVersion);
	    }
	    ce.writeFileToDisk(contentType, dd);
	    return ce;
	}
	
	return null;
    }

    public static boolean isBackgroundVerificationEnabled() {
        if (isCacheEnabled() == false) {
            return false;
        }
        String s = System.getProperty("jnlp.disableBackgroundVerification");
        if (s != null && s.equalsIgnoreCase("true")) {
            Trace.println("Cached JAR background verification disabled",
                    TraceLevel.CACHE);
            return false;
        }
        return true;
    }
    
    // set the new cache entry to be usable. and remove old current cache entry
    // For non-versioned resource, there should be only one cache entry
    public static CacheEntry processNewCacheEntry (URL href, boolean removeCurrentCE, 
						   CacheEntry ce,CacheEntry currentCE)
        throws IOException {
	
	Trace.println("Cache: Enable a new CacheEntry: "+href.toString(), 
                TraceLevel.NETWORK);

	ce.setBusy(0);
	ce.setIncomplete(0);
    ce.updateIndexHeaderOnDisk();
	
	if (removeCurrentCE && currentCE != null) {
	    if (removeCacheEntry(currentCE, false) == false) {
		// mark the entry as incomplete and let the
		// clean up take care of it
		currentCE.setIncomplete(1);
        currentCE.updateIndexHeaderOnDisk();
	    }
	    currentCE = null;
	} 

	recordLastAccessed();
	return ce;
    }

    public static int getCacheVersion() {
        return VERSION_INT;
    }

    public static String getCacheVersionString() {
	return VERSION_STRING;
    }
    
    // return size of user or system cache    
    public static long getCacheSize(boolean system) {
        long size = 0;
        File [] idxFiles = getCacheEntries(system);
        for (int i=0; i<idxFiles.length; i++) {
            size += idxFiles[i].length();
            CacheEntry ce = Cache.getCacheEntryFromFile(idxFiles[i]);
            if (ce != null) {
                size += ce.getContentLength();
                size += getTotalSize(new File(ce.getNativeLibPath()));
            }
        }
        return size;
    }

    private static long getTotalSize(File f) {
        long size = 0;
        if (f != null && f.exists()) {
            if (f.isDirectory()) {
                File[] files = f.listFiles();
                for (int i = 0; i < files.length; i++) {
                    size += getTotalSize(files[i]);
                }
            } else {
                size += f.length();
            }
        }
        return size;
    }
    
    private static SecureRandom getSecureRandom() {
        // Get the platform dependent Random generator
    
        if (random == null) {
            random = ServiceManager.getService().getSecureRandom();
            random.nextInt();
        }
        return random;
    }
    
    public static boolean exists() {
        if (Environment.isSystemCacheMode()) {
            return ((sysCacheDir != null) && (sysCacheDir.exists()));
        } else {
	    return cacheDir.exists();
	}
    }
    
    public static boolean canWrite() {
              
        if (Environment.isSystemCacheMode()) {
            return ((sysCacheDir != null) && (sysCacheDir.canWrite()));
        }
        
        return cacheDir.canWrite();
    }
    
    public static CacheEntry addLoadedResource(URL url, String resourceID, 
            String version) {
        if (url == null) {
            return null;
        }
        
        CacheEntry ce = (CacheEntry) MemoryCache.getLoadedResource(url.toString());
        if (ce != null)
            return ce;
        
        ce = getCacheEntry(url, resourceID, version);
        if (ce != null) {
            return (CacheEntry) MemoryCache.addLoadedResource(url.toString(), ce);
        }
        return null;
    }
   public static void createNoHrefCacheEntry(URL href, byte[] jnlpBytes)
    throws IOException {

        File dir = getActiveCacheDir();
        // we only want to replace the entry from the right cache
        CacheEntry currentCE = getCacheEntry(href, null, null, dir);
        String cacheFileName = generateCacheFileName(href, null);

        // Generate the index filename for this resource
        File indexFile = new File(dir,
            cacheFileName + Cache.getIndexFileExtension());

        // Generate the cache filename for this resource
        File cacheFile = new File(dir, cacheFileName);

        // create a new cache entry for this url and mark it busy
        CacheEntry ce = new CacheEntry(indexFile);
        ce.writeFileToDisk();

        BufferedOutputStream bos = new BufferedOutputStream(
                new FileOutputStream(ce.getTempDataFile()));

        try {
            bos.write(jnlpBytes);
        } finally {
            bos.close();
        }

        // process the downloaded file
        if (ce.processTempDataFile(false, null, href, href, null)){
            ce.setBusy(0);
            ce.setIncomplete(0);
            ce.setURL(href.toString());
            ce.setNoHref(1);

            // update the cache entry for it, resource is ready to use
            ce.writeFileToDisk();

            // remove the old CE
            if (currentCE != null) {
                // do not remove LAP
                removeCacheEntry(currentCE, false);
            }
            recordLastAccessed();
        }
    }
        
    public static String getIndexFileExtension() {
        return INDEX_FILE_EXT;
    }
    
    private static String getVersionTag(final String version) {
        return version == null ? "" : DASH + version + DASH ;
    }
    
    public static File getCacheDir() {
        return cacheDir;
    }
    
    public static void setSystemCacheDir(String path) {
	if (path == null || path.length() == 0) {
	    sysCacheDir = null;
	} else {
	    String sysCachePath = path + File.separator + VERSION_STRING;
	    sysCacheDir = new File(sysCachePath);
	}
    }

    public static File getActiveCacheDir() {
        return (Environment.isSystemCacheMode()) ? sysCacheDir : cacheDir;
    }
      /**
     * Returns an array of all index files in the cache
     */
    public static File [] getCacheEntries(boolean system) {
        File currentCacheDir = (system || Environment.isSystemCacheMode())  ?
                                sysCacheDir : cacheDir;
	if (currentCacheDir == null) {
	    return new File[0];
	}
      
        ArrayList fileList = new ArrayList();
        
        // go thru all the cache bucket directories
        for (int i = 0; i < NUM_OF_CACHE_SUBDIR; i++) {
            File cacheSubDir = new File(currentCacheDir.getPath() + 
                    File.separator + i);
            // system cache may be configured but not yet exist
            if (cacheSubDir.exists()) {
                // get a list of all the idx file in the cache
                File[] files = cacheSubDir.listFiles(new FileFilter() {
                    public boolean accept(File pathname) {
                        
                        String filename = pathname.getName();
                        boolean ret = filename.endsWith(INDEX_FILE_EXT);
                        return ret;
                    }
                });
                for (int j = 0; j < files.length; j++) {
                    fileList.add(files[j]);
                }
            }
        }
        return (File[])fileList.toArray(new File[fileList.size()]);
    }

    /**
     * Returns an iteration of the JNLP Applications, Applets, Librarys, and 
     * Installers in the cache. The elements of the array are of type File.
     */
    public static Iterator getJnlpCacheEntries(boolean system) {
        final ArrayList al = new ArrayList();
     
        // get a list of all the idx file in the cache
        File[] files = getCacheEntries(system);
   
        if (files != null) {
            for (int i = 0; i < files.length; i++) {
                // add to list if it is a jnlp entry
                String url = getURLFromIndexFile(files[i]);
                
                if (url != null) {
                    // remove query string from url
                    int queryIndex = url.lastIndexOf('?');
                    if (queryIndex != -1) {
                        url = url.substring(0, queryIndex);
                    }
                    if (url.endsWith(".jnlp") || url.endsWith(".jarjnlp")) {
                        String idxpath = files[i].getPath();
                        String jnlppath = 
                            idxpath.substring(0, idxpath.length() - 4);
                        al.add(new File(jnlppath));
                    }
                }
            }
        }
        
        return al.iterator();
    }

    public static void removeRemovedApp(String urlString, String title) {
        Properties p = getRemovedApps();
	String app = p.getProperty(urlString);
        if (app != null && app.equals(title)) {
            p.remove(urlString);
            setRemovedApps(p);
        }
    }
    
    public static void saveRemovedApp(URL href, String title) {
	// skip saving this as a removed app if you are uninstalling 
	// the application from one cache while it is still in the other.
	if (Environment.isSystemCacheMode()) {
            if (getCacheEntry(href, null, null, cacheDir) != null) {
		return;
            }
	} else {
            if (getCacheEntry(href, null, null, sysCacheDir) != null) {
		return;
            }
	}
        Properties p = getRemovedApps();
        p.setProperty(href.toString(), title);
        setRemovedApps(p);
    }

    public static void setRemovedApps(Properties props) {
        try {
            FileOutputStream fos = new FileOutputStream(getRemovePath());
            props.store(fos, "Removed JNLP Applications");
            fos.close();
        } catch (IOException ioe) { }
    }

    public static Properties getRemovedApps() {
        Properties props = new Properties();
        try {
            InputStream is = new FileInputStream(getRemovePath());
            props.load(is);
            is.close();
        } catch (IOException ioe) { }
        return props;
    }
    
    static final String LAST_ACCESS_FILE = "lastAccessed";
    
    /** Returns the time the cache was last accessed */
    public static long getLastAccessed(boolean system) {
        File f = new File(system ? sysCacheDir : cacheDir, LAST_ACCESS_FILE);
        return f.lastModified();
    }


    /** Private method to update the last accessed time stamp */
    private static void recordLastAccessed() {
        File f = new File(getActiveCacheDir(), LAST_ACCESS_FILE);
        try {
            OutputStream os = new FileOutputStream(f);
            os.write((int) '.');
            os.close();
        } catch(IOException io) { /* ignore */ }
    }

    public static String getRemovePath() {
        return cachePath + File.separator + REMOVED_APPS;
    }
    
    final static String REMOVED_APPS = "removed.apps";

    public static boolean removeCacheEntry(final URL resourceURL, 
            final String resourceID, final String version) {

        // only remove cache entries from the proper cache (system or user)
        CacheEntry ce = getCacheEntry(resourceURL, resourceID, 
                                      version, getActiveCacheDir());
        return removeCacheEntry(ce);
    }
    
    
    // Touch the given file
    public static final void touch(final File file) {
        try {
            AccessController.doPrivileged(new PrivilegedExceptionAction() {
                public Object run() throws IOException {
                    file.setLastModified(System.currentTimeMillis());
                    //return value is not used
                    return null;
                }
            });
        } catch (PrivilegedActionException pae) {
            Trace.ignoredException(pae);
        }
    }
    
    // remove the cache entry, as well as other instance of the same
    // cache entry with different IP
    public static void removeAllCacheEntries(CacheEntry ce) {
        if (ce == null) {
            return;
        }
        URL u = null;
        try {
            u = new URL(ce.getURL());
        } catch (MalformedURLException mue) {
            // should not happen
        }
        String version = ce.getVersion();
        // remove the cache entry
        removeCacheEntry(ce);
        if (u == null) {
            return;
        }
        // look for other instance of the same cache entry using the
        // cache entry url
        File[] idxFiles = getMatchingIndexFiles(getActiveCacheDir(), u);
        for (int i = 0; i < idxFiles.length; i++) {

            CacheEntry c = new CacheEntry(idxFiles[i]);
            // entry is a match if it's a non-versioned resource, or the
            // version matches
            String cVers = c.getVersion();
            if ( ( version == null && cVers==null )  || 
                 ( version != null && version.equals(cVers) )) {
                // entry matches, remove it
                removeCacheEntry(c);
            }
        }
    }
    
    /**
     * Remove all duplicate cache entries (by URL).
     * The latest entry will be kept.
     * Versioning is respected.
     *
     * @param system if true, using the system cache entries, 
     *               otherwise use the assigned cacheDir
     * @param ignoreIP ignore the IP while matching
     *
     * @return the size of all removed entries
     */
    public static int removeDuplicateEntries(boolean system, boolean ignoreIP) {
        int totalSize = 0;
        File [] files = getCacheEntries(system);
        for (int i=0; i<files.length; i++) {
            CacheEntry ce = Cache.getCacheEntryFromFile(files[i]);
            totalSize += removeDuplicateEntries(ce, ignoreIP);
        }
        if(totalSize>0) {
            Trace.println("Remove All Duplicates: "+totalSize+" bytes", 
                TraceLevel.NETWORK);
        }
        return totalSize;
    }

    /**
     * Remove cache entries, duplicate to 'ce' (by URL).
     * The latest entry will be kept.
     * Versioning is respected.
     *
     * @param ce the to be matched entry
     * @param ignoreIP ignore the IP while matching
     *
     * @return the size of all removed entries
     */
    public static int removeDuplicateEntries(CacheEntry ce, boolean ignoreIP) {
        int totalSize = 0;
        String urlString = (null!=ce)?ce.getURL():null;
        if (ce == null || urlString == null || 
            ce.getIncomplete()!=0 || MemoryCache.contains(urlString))
        {
            return totalSize;
        }
        URL u = null;
        try {
            u = new URL(urlString);
        } catch (MalformedURLException mue) {
            // should not happen
        }
        if (u == null) {
            return totalSize;
        }

        File[] idxFiles = getMatchingIndexFiles(cacheDir, u);
        CacheEntry lc = ce;

        // lookup the last modified one, which we like to keep
        for (int i = 0; i < idxFiles.length; i++) {
            CacheEntry c = new CacheEntry(idxFiles[i]);
            String version = lc.getVersion();
            String cVers = c.getVersion();
            if ( ( version == null && cVers==null )  || 
                 ( version != null && version.equals(cVers) )) {
                String ip  = ignoreIP?null:lc.getCodebaseIP();
                String cip = ignoreIP?null:c.getCodebaseIP();
                if( ip == cip || ( ip!=null && ip.equals(cip) )) {
                    if(lc.removeBefore(c)) {
                        // lc shall be removed before c, so take c
                        lc=c;
                    }
                }
            }
        }
        String version = lc.getVersion();
        int num = 0;

        // look for other instance of the same cache entry using the
        // cache entry url
        for (int i = 0; i < idxFiles.length; i++) {
            CacheEntry c = new CacheEntry(idxFiles[i]);
            // entry is a match if it's a non-versioned resource, or the
            // version matches
            String cVers = c.getVersion();
            if ( ( version == null && cVers==null )  || 
                 ( version != null && version.equals(cVers) )) {
                String ip  = ignoreIP?null:lc.getCodebaseIP();
                String cip = ignoreIP?null:c.getCodebaseIP();
                if(ip == cip || ( ip!=null && ip.equals(cip) )) {
                    if(!c.getIndexFile().equals(lc.getIndexFile()))
                    {
                        // entry matches, remove it
                        // should not remove LAP here, since we are only
                        // removing duplicates
                        if (removeCacheEntry(c, false)) {
                            totalSize += c.getContentLength();
                            num++;
                        }
                    }
                }
            }
        }

        if(totalSize>0) {
            Trace.println("Remove "+num+" Duplicates of: ["+lc.getURL()+", ",
                TraceLevel.NETWORK);
            Trace.println("\tidx: "+lc.getIndexFile()+"], "+totalSize+" bytes",
                TraceLevel.NETWORK);
        }
        return totalSize;
    }

    public static boolean removeCacheEntry(final CacheEntry ce, 
            final boolean removeLAP) {
        Boolean ret = (Boolean) AccessController.doPrivileged(
                new PrivilegedAction() {
            public Object run() {
                boolean ret = removeCacheEntryImpl(ce, removeLAP);
                return Boolean.valueOf(String.valueOf(ret));
            }
        });
        
        return ret.booleanValue();
    }

    private static boolean removeCacheEntryImpl(CacheEntry ce, boolean removeLAP) {
        if (Trace.isTraceLevelEnabled(TraceLevel.NETWORK)) {
            Trace.println(ResourceManager.getString("cache.removeCacheEntry",
                ce == null ? "" : ce.getURL()), TraceLevel.NETWORK);
        }
        // don't delete anything from the wrong cache:
        if (!ce.getIndexFile().getParentFile().getParentFile().equals(getActiveCacheDir())) {
            // we also don't want to return false here because that would
            // make the caller think delete failed, and mark it for clean up.
            return true;
        }

        File jarFile = new File(ce.getResourceFilename());
        String urlString = ce.getURL();
        String version = ce.getVersion();
        boolean ret = false;
        /* avoid deleting used resources */
        if (!MemoryCache.contains(urlString)) {
            if (urlString != null &&
                    (urlString.toLowerCase().endsWith(".jar") ||
                    urlString.toLowerCase().endsWith(".jarjar"))) {
                JarFile jf = ce.getJarFile();
                if (jf != null) {
                    try {
                        jf.close();
                    } catch (IOException ioe) {
                        Trace.ignoredException(ioe);
                    }
                }
            }
            ret = jarFile.delete();
        } else {
            jarFile.deleteOnExit();
        }
     
        clobber(new File(ce.getNativeLibPath()));
        boolean ret2 = ce.getIndexFile().delete();
        try {
            String name = getLapFileName(new URL(urlString), version);
            File lapFile = new File(getActiveCacheDir(), name);
            if (lapFile.exists() && removeLAP) {
                lapFile.delete();
            }
        } catch (MalformedURLException mfue) {
        }
        File icoFile = new File(ce.getResourceFilename() + ".ico");
        if (icoFile.exists()) {
            icoFile.delete();
        }
        recordLastAccessed();
        MemoryCache.removeLoadedResource(ce.getURL());
        return ret && ret2;      
    }

    private static void clobber(File f) {
        if (f.exists()) {
            if (f.isDirectory()) {
                File[] files = f.listFiles();
                for (int i=0; i<files.length; i++) {
                    clobber(files[i]);
                }
                // remove the directory itself
                f.delete();
            } else {
                f.delete();
            }
        }
    }
    
    public static boolean removeCacheEntry(CacheEntry ce) {
        // always remove LAP by default
        return removeCacheEntry(ce, true);
    }
    
    // Returns a list of index files which match the given criteria
    private static File[] getMatchingIndexFiles(final File cacheDirectory, URL url) {
        
        // Get the cache key for this URL
        final String key = getKey(url);
        // Get the cache bucket for this key
        final File directory = new File(cacheDirectory.getPath() + File.separator +
                getBucket(key));

        File[] idxFiles = (File[]) AccessController.doPrivileged(
                new PrivilegedAction() {
            public Object run() {
                // Get the list of files that match this name
                File[] files = directory.listFiles(new FileFilter() {
                    public boolean accept(File pathname) {
                        String filename = pathname.getName();
                        return filename.startsWith(key) &&
                                filename.endsWith(INDEX_FILE_EXT);
                    }
                });
                return files;
            }
        });
        return idxFiles;
    }
    
    public static CacheEntry getCacheEntryFromFile(File idxFile) {
        CacheEntry ce = new CacheEntry(idxFile);
        
        if (ce != null && ce.getIncomplete() == 0 && isCacheEntryIPValid(ce)) {
            return ce;
        }
        return null;
    }
    
    public static String getVersionFromFilename(String filename) {
    
        StringTokenizer st = new StringTokenizer(filename, "-");
     
        if (st.countTokens() != 4) {
            return null;
        }
        
        st.nextToken();
        st.nextToken();
        return st.nextToken();
    }
    
    public static CacheEntry getLatestCacheEntry(final URL url, 
            final String resourceID) {
        File latestCacheFile = null;
        String latestVersion = null;
        File[] idxFiles = getMatchingIndexFiles(cacheDir, url);
        // read the index and find out the matching index file with the url
        for (int i = 0 ; i < idxFiles.length; i++) {         
            if (latestCacheFile == null) {
                latestCacheFile = idxFiles[i];
            } else {
                String curVer = getVersionFromFilename(idxFiles[i].getName());
                String latestVer = 
                        getVersionFromFilename(latestCacheFile.getName());
                if (curVer != null && (latestVer == null || 
                        curVer.compareTo(latestVer) > 0)) {
                    latestCacheFile = idxFiles[i];
                }
            }
        }
        if (latestCacheFile != null) {
   
            return getCacheEntryFromFile(latestCacheFile);
            
        }
        return null;
    }
    
    public static String getURLFromIndexFile(File idxFile) {
        CacheEntry ce = new CacheEntry(idxFile);
        return ce.getURL();
    }

    public static boolean isEntryIncomplete(File idxFile) {
         CacheEntry ce = new CacheEntry(idxFile);
         return (ce.getIncomplete() == 0);
    }
    
    private static CacheEntry getCacheEntryFromIdxFiles(File[] idxFiles, 
            URL url, String version, int contentType) {
        CacheEntry ce = null;
        CacheEntry found = null;
        if (idxFiles != null) {
            // read the index and find out the matching index file with the url
            for (int i = 0 ; i < idxFiles.length; i++) {
                ce = new CacheEntry(idxFiles[i]);
                if (ce.getIncomplete() == 0) {
                    if (ce.getURL().equals(url.toString())) {
                       if (isCacheEntryIPValid(ce)) {
                            if (version == null && ce.getVersion() == null) {
                                found = ce;
                                break;
                            }
                            if (version != null &&
                                    new VersionString(version).contains(
                                    ce.getVersion())) {
                                if (found == null) {
                                    found = ce;
                                } else {
                                    if (ce.getVersion() != null) {
                                        if (ce.getVersion().compareTo(
                                                found.getVersion()) > 0) {
                                            found = ce;
                                        }
                                    }
                                }
                            }
                       }  // cache entry does not match current IP, keep looking
                    } else {
                        // url in index file does not match, remove the
                        // cache entry
                        // this is the case where the url has a query
                        // string, and we encounter a new query string
                        // for the same entry
                        if (ce.getURL().indexOf('?') != -1) {
                            // Keep the LAP file, we don't want to prompt
                            // the user for shortcut whenever there is a new
                            // query string for the same jnlp url
                            removeCacheEntry(ce, false);
                        }
                    }
                } else {
                    // wake up clean-up thread to remove incomplete entry
                    cleanup();
                }
            }
        }
                
        // if native content entry, make sure native directory exists
        if (found != null && DownloadEngine.isNativeContentType(contentType)) {
            File nativeDir = new File(found.getNativeLibPath());
            if (nativeDir.isDirectory() == false) {
                // native directory is missing
                // cache entry is corrupted, remove it
                removeCacheEntry(found);
                found = null;
            }
        }
        
        return found;
    }
    
    // Update the cache host IP file for the specific host
    // If host file is not avaiable, create one with the current IP address
    // If host file is avaiable and current IP lookup is different from the
    // host file IP, update host file IP
    public static void updateHostIPFile(String host) {
        String cachedIP = getCachedHostIP(host);
        if (cachedIP == null) {
            // first time lookup
            createHostEntry(host);
        } else {
            // call new API get the current host IP
            String currentIP = getCurrentIP(host, cachedIP);
  
            // update cached host IP if different
            if (currentIP != null && currentIP.equals(cachedIP) == false) {
                updateHostEntry(host);
            }
        }
    }
    
    private static URL getHostURL(String host) {
        URL u = null;
        try {
            u = new URL("http://" + host);
        } catch (MalformedURLException mue) {
            // should not happen
        }
        return u;
    }
    
    private static void updateHostEntry(String host) {
        URL u = getHostURL(host);
        
        // delete all files for this host
        final File[] hostFiles = getMatchingHostFiles(hostDir, u);
        
        // this needs to go inside a doPriviledged block, as it might call
        // from applet code thru DeployCacheHandler to look up a cache
        // entry
        AccessController.doPrivileged(
                new PrivilegedAction() {
            public Object run() {
                for (int i = 0; i < hostFiles.length; i++) {                   
                    hostFiles[i].delete();
                }
                return null;
            }
        });
        
        // create new host entry
        createHostEntry(host);
    }
    
    // Get the current hostIP using private InetAddress API
    // getByName(String, InetAddress)
    // Returns the IP address string if lookup can be performed
    // otherwise return null;
    private static String getCurrentIP(String host, String cachedIP) {

        if (host == null || cachedIP == null) {
            return null;
        }

        if (!doIPLookup) {
            return null;
        }
          
        String hostIP = null;
       
        // lookup new InetAddress API and do the dns lookup
        // if ipaddress is valid then continue
        // otherwise, remove cache entry and force download again
        Class inetClass = null;
        try {
            inetClass = Class.forName("java.net.InetAddress");
        } catch (ClassNotFoundException cnfe) {
            // should not happen
            Trace.ignoredException(cnfe);
            return null;
        }
        
        InetAddress cacheIna = null;
        
        try {
            cacheIna = InetAddress.getByName(cachedIP);
        } catch (UnknownHostException uhe) {
            // should not happen
            Trace.ignoredException(uhe);
        }
        
        Object[] arguments = { host, cacheIna };
        
        // Find getByName(String host) method
        Class type[] = new Class[2];
        try {
            type[0] = Class.forName("java.lang.String");
            type[1] = inetClass;
        } catch (ClassNotFoundException cnfe) {
            // should not happen
            Trace.ignoredException(cnfe);
            return null;
        }
        
        final Method lookupMethod;
        try {
            lookupMethod = inetClass.getDeclaredMethod("getByName", type);
        } catch (NoSuchMethodException nsme) {
            // method not available,  skip cached ip check
            //Trace.ignoredException(nsme);
            // assume cache entry IP still valid
            return null;
        }
        
        // this needs to go inside a doPriviledged block, as it might call
        // from applet code thru DeployCacheHandler to look up a cache
        // entry   
        AccessController.doPrivileged(
                new PrivilegedAction() {
            public Object run() {
                lookupMethod.setAccessible(true);
                return null;
            }
        });
                
        // Check that method is static
        if (!Modifier.isStatic(lookupMethod.getModifiers())) {
            // should not happen
            return null;
        }
        
        InetAddress ina = null;
        try {
            // Invoke InetAddress.getByName method
            ina = (InetAddress) (lookupMethod.invoke((Object)null,
                    arguments));
        } catch (IllegalAccessException iae) {
            Trace.ignoredException(iae);
        } catch (InvocationTargetException ite) {
            // failed to resolve hostname (UnkownHostException)
        }
        
        if (ina != null) {
            hostIP = ina.getHostAddress();
        }

        return hostIP;
    }
    
    // Check if the Cache Entry cached IP is still vaild
    // returns true if the IP is still valid; false otherwise
    // Method will returns true if the entry has no cached IP; or
    // if we fail to perform a new dns lookup of the entry codebase
    // (We assume the cache entry is still valid for these cases)
    private static boolean isCacheEntryIPValid(CacheEntry ce) {
        boolean result = true;

	if (ce != null && ce.isKnownToBeSigned()) {
    	    // skip cache entry IP check for pre-validated cache entry
    	    return result;
	}
    
        String cacheEntryCodebaseIP = ce.getCodebaseIP();
        
        if (cacheEntryCodebaseIP == null) {
            // cache entry does not has IP entry, nothing to check
            return result;
        }
        
        URL u = null;
        try {
            u = new URL(ce.getURL());
        } catch (MalformedURLException mue) {
            // should not happen
            Trace.ignoredException(mue);
            return false;
        }
        String host = u.getHost();

        String currentHostIP = getCurrentIP(host, cacheEntryCodebaseIP);

        if (currentHostIP == null) {
            // cannot resolve IP address -> application/applet cannot make
            // socket connections anyway without IP address, so no need to
            // continue the checck
            return true;
        }
                
        if (currentHostIP.equals(cacheEntryCodebaseIP) == false) {
            // IP mismatch, cache entry mismatch
            result = false;
        }
    
        return result; 
    }
    
    public static CacheEntry getCacheEntry(final URL url,
            final String resourceID, final String version) {
        return getCacheEntry(url, resourceID, version,
                DownloadEngine.NORMAL_CONTENT_BIT);
    }
    
    public static CacheEntry getCacheEntry(final URL url,
            final String resourceID, final String version,
            final int contentType) {
        
        CacheEntry ce = (CacheEntry) MemoryCache.getLoadedResource(url.toString());
        if (ce != null) {
            String currentVersion = ce.getVersion();
            if ((version == null && currentVersion == null )||
                    (version != null && currentVersion != null &&
                    currentVersion.compareTo(version) >= 0)) {
                return ce;
            }
        }
   
        CacheEntry found = getCacheEntry(url, resourceID, version, cacheDir,
                contentType);
        CacheEntry sysFound = getCacheEntry(url, resourceID, 
                                            version, sysCacheDir, contentType);
     
        // if -system is specified, ignore user cache contents
        if (Environment.isSystemCacheMode()) {
            if (sysFound != null) {
               MemoryCache.addLoadedResource(url.toString(), sysFound);
                Trace.println("System Cache: " +
                    ResourceManager.getString(
                    "cache.getCacheEntry.return.found",
                    url == null ? "" : url.toString(), version) +
                    " prevalidated=" + sysFound.isKnownToBeSigned() +
                    "/" + sysFound.getClassesVerificationStatus(),
                    TraceLevel.NETWORK);
            }
            return sysFound;
        }
        
        if (sysFound != null) {
             if (found != null) {
                 // use system cache entry if both entry is the same
                 if (found.getLastModified() <= sysFound.getLastModified()) {
                     found = sysFound;
                 }
             } else {
                 found = sysFound;
             }
        }
        
        if (found == null) {
            if (Trace.isTraceLevelEnabled(TraceLevel.NETWORK)) {
                Trace.println(ResourceManager.getString(
                    "cache.getCacheEntry.return.notfound",
                    url == null ? "" : url.toString(), version),
                    TraceLevel.NETWORK);
            }
        } else {
            if (Trace.isTraceLevelEnabled(TraceLevel.NETWORK)) {
                Trace.println(ResourceManager.getString(
                    "cache.getCacheEntry.return.found",
                    url == null ? "" : url.toString(), version) +
                    " prevalidated=" + found.isKnownToBeSigned() +
                    "/" + found.getClassesVerificationStatus(),
                    TraceLevel.NETWORK);
            }

            MemoryCache.addLoadedResource(url.toString(), found);
        }
        
        // return the cache entry
        return found;
    }
    
    public static CacheEntry getCacheEntry(final URL url,
            final String resourceID, final String version, final File dir) {
        return getCacheEntry(url, resourceID, version, dir, 
                DownloadEngine.NORMAL_CONTENT_BIT);
    }

    private static CacheEntry getCacheEntry(final URL url,
            final String resourceID, final String version, final File dir,
            final int contentType) {
        if (dir == null) {
            return null;
        }
        File [] idxFiles = getMatchingIndexFiles(dir, url);
        CacheEntry ce = getCacheEntryFromIdxFiles(idxFiles, url, version,
                contentType);

        /* Do not read manifest or certificates greedy as
         * they might be not needed soon and then this will be waste of time.
         * If cache entry is corrupt then we will try to handle
         * this in the runtime. */

        return ce;
    }

    
    // Returns a list of muffin files which match the given criteria
    private static File[] getMatchingMuffinFiles(File directory, URL url) {
        
        // Get the cache key for this URL
        final String key = getKey(url);
        
        // Get the list of files that match this name
        final File[] files = directory.listFiles(new FileFilter() {
            public boolean accept(File pathname) {
                String filename = pathname.getName();
                return filename.startsWith(key) && 
                        !filename.endsWith(MUFFIN_FILE_EXT);
            }
        });
        
        return files;
    }
    
     // Returns a list of muffin files which match the given criteria
    private static File[] getMatchingMuffinAttributeFiles(File directory, 
            URL url) {
        
        // Get the cache key for this URL
        final String key = getKey(url);
        
        // Get the list of files that match this name
        final File[] files = directory.listFiles(new FileFilter() {
            public boolean accept(File pathname) {
                String filename = pathname.getName();
                return filename.startsWith(key) && 
                        filename.endsWith(MUFFIN_FILE_EXT);
            }
        });
        
        return files;
    }
    
    public static File getMuffinFile(URL url) {
        String key = getKey(url);
        File[] files = getMatchingMuffinFiles(muffinDir, url);
        
        if (files == null || files.length == 0) {
            return null;
        }
    
        return files[0];
    }
    
    public static File getMuffinAttributeFile(URL url) {
        String key = getKey(url);
        File[] files = getMatchingMuffinAttributeFiles(muffinDir, url);
        
        if (files == null || files.length == 0) {
            return null;
        }
       
        return files[0];
    }
    
    public static long [] getMuffinAttributes(URL url) throws IOException {
        BufferedReader br = null;
        long tag = -1;
        long maxsize = -1;
        try {
            File muffinFile = getMuffinAttributeFile(url);
            if (muffinFile == null) {
                throw new FileNotFoundException("Muffin not found for " + url);
            }
            InputStream is = new FileInputStream(muffinFile);
            br = new BufferedReader(new InputStreamReader(is));
            String line = br.readLine();
            try {
                tag = (long) Integer.parseInt(line);
            } catch (NumberFormatException nfe) {
                throw new IOException(nfe.getMessage());
            }
            line = br.readLine();
            try {
                maxsize = Long.parseLong(line);
            } catch (NumberFormatException nfe) {
                throw new IOException(nfe.getMessage());
            }
        } finally {
            if (br != null) br.close();
        }
        return new long [] {tag, maxsize};
    }

    
    public static void removeMuffinEntry(URL url) throws IOException {
        
        File muffinFile = getMuffinFile(url);
        if (muffinFile != null) {
          
            if (!muffinFile.delete()) throw 
                    new IOException("delete failed for muffin: " + url);
            File maFile = new File(muffinFile.getPath() + MUFFIN_FILE_EXT);
            if (!maFile.delete()) throw 
                    new IOException("delete failed for muffin: " + url);
        } else {
            throw new FileNotFoundException(
                    "Muffin for " + url + " does not exists");
        }
    }
    
    // Lookup cached IP
    // Return cached IP address if available
    // Otherwise, return null
    private static String getCachedHostIP(String host) {
        if (host == null) return null;
        
        URL u = getHostURL(host);
        
        String cachedIP = null;
   
        final File cachedHostFile = getHostFile(u);
    
        if (cachedHostFile != null) {
            // this needs to go inside a doPriviledged block, as it might call
            // from applet code thru DeployCacheHandler to look up a cache
            // entry
            cachedIP = (String) AccessController.doPrivileged(
                    new PrivilegedAction() {
                public Object run() {
                    String ip = null;
                    try {
                        BufferedReader br = new BufferedReader(new FileReader(
                                cachedHostFile));
                        ip = br.readLine();
                        br.close();
                    } catch (IOException ioe) {
                        Trace.ignoredException(ioe);
                    }
                    return ip;
                }
            });
        }
        return cachedIP;
    }
    
    private static File getHostFile(URL url) {
 
        File[] files = getMatchingHostFiles(hostDir, url);
        
        if (files == null || files.length == 0) {
            return null;
        }
        
        return files[0];
    }
    
    // Returns a list of Host files which match the given criteria
    private static File[] getMatchingHostFiles(final File directory, URL url) {
        
        // Get the cache key for this URL
        final String key = getKey(url);
        
        // Get the list of files that match this name
        // this needs to go inside a doPriviledged block, as it might call
        // from applet code thru DeployCacheHandler to look up a cache
        // entry      
        File[] hostFiles = (File[]) AccessController.doPrivileged(
                new PrivilegedAction() {
            public Object run() {
                final File[] files = directory.listFiles(new FileFilter() {
                    public boolean accept(File pathname) {
                        String filename = pathname.getName();
                        return filename.startsWith(key) &&
                                filename.endsWith(HOST_FILE_EXT);
                    }
                });
                return files;
            }
        });
              
        return hostFiles;
    }
    
    static InetAddress getHostIP(String host) {
        InetAddress ina = null;
        try {
            ina = InetAddress.getByName(host);
        } catch (UnknownHostException uhe) {
            // use 0.0.0.0 to indicate the hostname cannot be
            // resolve, e.g when we download via proxy
            try {
                ina = InetAddress.getByName(IP_ADDR_CANNOT_RESOLVE);
            } catch (UnknownHostException uhe2) {
                // should not happen
            }
        }
        return ina;
    }
    
    private static void createHostEntry(String host) {
        URL u = getHostURL(host);
    
        String key = getKey(u);

        // Generate random filenames off of this key until a new file is found.     
        String filename;
        final File hostFile;
   
        filename = key + Integer.toString(getRandom(), 16);
        hostFile = new File(hostDir, filename + HOST_FILE_EXT);
      
        InetAddress ina = getHostIP(host);
      
        if (ina != null) {        
            final String hostAddr = ina.getHostAddress();
            // this needs to go inside a doPriviledged block, as it might call
            // from applet code thru DeployCacheHandler to look up a cache
            // entry  
            AccessController.doPrivileged(
                    new PrivilegedAction() {
                public Object run() {
                    // write IP address into host file
                    try {
                        BufferedWriter bw = new BufferedWriter(
                                new FileWriter(hostFile));
                        bw.write(hostAddr);
                        bw.close();
                    } catch (IOException ioe) {
                        Trace.ignoredException(ioe);
                    }
                    return null;
                }
            });
        }
    }
    
    public static void createMuffinEntry(URL url, int tag, long maxSize) 
    throws IOException {
        String key = getKey(url);
        // make sure entry does not exists
        File[] files = getMatchingMuffinAttributeFiles(muffinDir, url);
        
        if (files.length != 0) {
            throw new IOException(
                    "insert failed in cache: target already exixts");
        }      
               
        // Generate random filenames off of this key until a new file is found.
        // We use the atomic createNewFile() method to ensure an exclusive
        // lock on this filename.
        String filename;
        File muffinFile;
        File muffinAttributeFile;
       
        filename = key + Integer.toString(getRandom(), 16);
        muffinAttributeFile = new File(muffinDir, filename + MUFFIN_FILE_EXT);
        muffinFile = new File(muffinDir, filename);
    
        putMuffinAttributes(muffinAttributeFile, url, tag, maxSize);
        muffinFile.createNewFile();
        
    }
    
    // get all muffin entries under this url directory
    public static String[] getMuffinNames(URL url) {
        Vector v = new Vector();
          // Get the list of files that match this name
        final File[] files = muffinDir.listFiles(new FileFilter() {
            public boolean accept(File pathname) {
                String filename = pathname.getName();
                return filename.endsWith(MUFFIN_FILE_EXT);
            }
        });
        URL u = null;
        String entryName;
        for (int i = 0; i < files.length; i++) {
          
            try {
                u = getCachedMuffinURL(files[i]);
            } catch (IOException ioe) {
                Trace.ignoredException(ioe);
            }
 
            if (u != null) {
                URL urlNoQuery = HttpUtils.removeQueryStringFromURL(u);
                entryName = urlNoQuery.getFile().substring(1 + 
                        urlNoQuery.getFile().lastIndexOf('/'));
           
                if (u.toString().equals(url.toString() + entryName)) {
                    v.add(entryName);
                }
            }
        }
        return (String [])v.toArray(new String[0]);
    }
    
    public static URL[] getAccessibleMuffins(URL url) throws IOException {
        ArrayList list = new ArrayList();

        // Get the list of files that match this name
        final File[] files = muffinDir.listFiles(new FileFilter() {
            public boolean accept(File pathname) {
                String filename = pathname.getName();
                return filename.endsWith(MUFFIN_FILE_EXT);
            }
        });
        URL u;
        int urlCount = 0;
        for (int i = 0; i < files.length; i++) {
            u = getCachedMuffinURL(files[i]);
            if (u.getHost().equals(url.getHost())) {
                list.add(u);
            }
        }
        return (URL [])list.toArray(new URL[0]);
    }
    
    static URL getCachedMuffinURL(File muffinAttributeFile) throws IOException {
        BufferedReader br = null;
        long tag = -1;
        long maxsize = -1;
        String line = null;
        try {
            InputStream is = new FileInputStream(muffinAttributeFile);
            
            br = new BufferedReader(new InputStreamReader(is));
            line = br.readLine();
          
            line = br.readLine();
            line = br.readLine();
        } catch (Exception e) {
            Trace.ignoredException(e);
        } finally {
            if (br != null) br.close();
        }
        URL url = null;
        try {
            url = new URL(line);
        } catch (MalformedURLException  mue) {
            Trace.ignoredException(mue);
        }
        return url;
    }
    
    public static long getMuffinSize(URL url) throws IOException {
        // For now, not counting attribute file size as part of muffin size
        long size = 0;
        File muffinFile = getMuffinFile(url);

        if (muffinFile != null && muffinFile.exists()) {
            size += muffinFile.length();
        }
        return size;
    }
    
    private static void putMuffinAttributes(File muffinAttributeFile,
            URL url, int tag, long maxsize) throws IOException {
       
        PrintStream ps = new PrintStream(new FileOutputStream(
                muffinAttributeFile));
        try {
            ps.println(tag);
            ps.println(maxsize);
            ps.println(url.toString());
        } finally {
            if (ps != null) ps.close();
        }
    }
    
    public static void putMuffinAttributes(URL url, int tag, 
            long maxsize) throws IOException {
        File muffinAttributeFile = getMuffinAttributeFile(url);
        PrintStream ps = new PrintStream(new FileOutputStream(
                muffinAttributeFile));
        try {
            ps.println(tag);
            ps.println(maxsize);
            ps.println(url.toString());
        } finally {
            if (ps != null) ps.close();
        }
    }
    
    // Generate a new cache file name base on the URL
    // FIXME: not good - 2 threads may pick same nmae!
    static String generateCacheFileName(final URL url,
            final String version) throws IOException {
        String filename = null;
        try {
            filename = (String)AccessController.doPrivileged(new PrivilegedExceptionAction() {
                public Object run() throws IOException {
                    // Get the cache key for this URL
                    String key = getKey(url);
                    
                    // Generate random filenames off of this key until a new file is found.
                    // We use the atomic createNewFile() method to ensure an exclusive
                    // lock on this filename.
                    String filename;
                    File dataFile, indexFile;
                    do {
                        filename = getBucket(key) + File.separator + 
                                key + Integer.toString(getRandom(), 16) +
                                getVersionTag(version);
                        dataFile = new File(cacheDir, filename);
                        indexFile = new File(cacheDir, filename + INDEX_FILE_EXT);
                    } while (indexFile.exists() || dataFile.exists());
                    indexFile = null;
                    dataFile = null;
                    return filename;
                }
            });
        } catch (PrivilegedActionException pae) {
            if (pae.getException() instanceof IOException) {
                throw (IOException)pae.getException();
            }
        }
        return filename;
    }
    
    /**
     * Returns index for hash code h.
     */
    public static int getBucket(String key) {
        return Integer.valueOf(key.substring(0, key.length()-1), 16).intValue()
            & (NUM_OF_CACHE_SUBDIR-1);
    }
    
    // Returns the key used to look a given url up in the cache
    protected static String getKey(URL url) {
        
        // Generate a positive hash value for this URL
        // Use no query string URL
        int hashCode = hashCode(HttpUtils.removeQueryStringFromURL(url));
        if (hashCode < 0) {
            hashCode += Integer.MAX_VALUE + 1;
        }
        String key = Integer.toString(hashCode, 16);
        
        // Combine the file name and hash to generate the cache key
        return key + "-";
    }
    
    /**
     * From java.util.HashMap
     *
     * Returns a hash value for the specified object.  In addition to
     * the object's own hashCode, this method applies a "supplemental
     * hash function," which defends against poor quality hash functions.
     * This is critical because HashMap uses power-of two length
     * hash tables.<p>
     *
     * The shift distances in this function were chosen as the result
     * of an automated search over the entire four-dimensional search space.
     *
     * Instead of using URL.hashCode, we convert the URL to a String first.
     * This is to prevent the URL address lookup which takes place if you
     * do URL.hashCode.
     */
    protected static int hashCode(URL x) {               
        int h = x.toString().hashCode();

        h += ~(h << 9);
        h ^=  (h >>> 14);
        h +=  (h << 4);
        h ^=  (h >>> 10);
        return h;
    }
    
    // It returns the extension of the file to which URL points
    protected static final String getFileExtension(String name) {
        String ext = "";
        //get the url extension
        int extIndex = name.lastIndexOf('.');
        if(extIndex != -1) {
            ext = name.substring(extIndex);
        }
        
        return ext;
    }
    
    // Returns a postive, eight digit hexadecimal random number
    protected static final int getRandom() {
        return 0x10000000 + getSecureRandom().nextInt(Integer.MAX_VALUE - 0x10000000);
    }

    // methods used to update old format cache to new

    /**
     * insertFile - for putting old cache entry into new cache 
     "              (used for normal and nativelib jars, and jnlp files) 
     */
    public static void insertFile(File dataFile, int contentType,
		URL url, String version, long ts, long expiration) 
	throws IOException {

	String cacheFileName = generateCacheFileName(url, version);
	File dir = getActiveCacheDir();

	// Generate the index filename for this resource
	File indexFile = new File(dir, cacheFileName + getIndexFileExtension());

	// create a new cache entry for this content and url 
	CacheEntry ce = new CacheEntry(indexFile);
	ce.writeFileToDisk();

	try {
            // copy the data file from the old cache
            copyFile(dataFile, new File(dir, cacheFileName));
	} catch (IOException e) {
	    // if there is an error, clean up index file before continuing
	    removeCacheEntry(ce);
	    throw e;
	}

	ce.setBusy(0);
	ce.setIncomplete(0);
	ce.setURL(url.toString());
	ce.setContentLength((int) dataFile.length());
	ce.setLastModified(ts);
	ce.setExpirationDate(expiration);
	if (version != null) {
            ce.setVersion(version);
	}
        if (DownloadEngine.isJarContentType(contentType)) {
            MessageHeader headerFields = new MessageHeader();
            headerFields.add(HttpRequest.DEPLOY_REQUEST_CONTENT_TYPE,
                    HttpRequest.JAR_MIME_TYPE);
            ce.setHeaders(headerFields);
        }
        // update the cache entry for it, resource is ready to use
	ce.writeFileToDisk(contentType, null);

	recordLastAccessed();

    }

    public static void insertMuffin(URL url, File muffin, 
				    int tag, long maxSize) throws IOException {
	// make sure entry does not exists
	File[] files = getMatchingMuffinAttributeFiles(muffinDir, url);

	if (files.length != 0) {
            throw new IOException(
                    "insert failed in cache: target already exixts");
	}

        // Generate random filenames off of this key until a new file is found.
	// We use the atomic createNewFile() method to ensure an exclusive
	// lock on this filename.
	String filename;
	File muffinAttributeFile;

	String key = getKey(url);
	filename = key + Integer.toString(getRandom(), 16);
	muffinAttributeFile = new File(muffinDir, filename + MUFFIN_FILE_EXT);
	putMuffinAttributes(muffinAttributeFile, url, tag, maxSize);
	copyFile(muffin, new File(muffinDir, filename));
    }

    public static void copyFile(File src, File dst) throws IOException {
	byte b[] = new byte[10240];
	BufferedOutputStream bos = null;
	BufferedInputStream bis = null;
	try {
            bos = new BufferedOutputStream(new FileOutputStream(dst));
            bis = new BufferedInputStream(new FileInputStream(src));
            int n = bis.read(b);
            while (n >= 0) {
	        bos.write(b, 0, n);
	        n = bis.read(b);
            }
	} finally {
            try {
                if (bos != null) { 
		    bos.close(); 
		}
            } catch (Exception e) {}
            try {
                if (bis != null) { 
		    bis.close(); 
	        }
            } catch (Exception e) {}
	}
    }

    public static void removeAllMuffins() {
	File[] children = muffinDir.listFiles();
	for (int i=0; i<children.length; i++) {
            children[i].delete();
	}
    }
}
