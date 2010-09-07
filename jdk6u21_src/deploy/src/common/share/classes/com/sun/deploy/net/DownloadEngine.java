/*
 *  @(#)DownloadEngine.java	1.73 10/04/29
 *
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net;
import java.net.URL;
import java.net.MalformedURLException;
import java.net.HttpURLConnection;
import java.net.URLConnection;
import java.io.InputStream;
import java.io.BufferedInputStream;
import java.io.IOException;
import java.io.FileNotFoundException;
import java.io.File;
import java.io.OutputStream;
import java.io.FileOutputStream;
import java.io.UnsupportedEncodingException;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.jar.*;
import java.util.*;
import java.util.zip.GZIPInputStream;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.util.VersionString;
import com.sun.deploy.util.VersionID;
import com.sun.deploy.util.URLUtil;
import com.sun.deploy.util.BlackList;
import com.sun.deploy.config.Config;
import com.sun.deploy.cache.CacheEntry;
import com.sun.deploy.cache.CachedJarFile;
import com.sun.deploy.cache.Cache;
import com.sun.deploy.Environment;
import com.sun.deploy.cache.MemoryCache;
import com.sun.deploy.jardiff.Patcher;
import com.sun.deploy.jardiff.JarDiffPatcher;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.net.offline.DeployOfflineManager;

import sun.awt.AppContext;

public class DownloadEngine {
    // Arguments for the Extension Download Protocol
    private static final String ARG_ARCH                 = "arch";
    private static final String ARG_OS                   = "os";
    private static final String ARG_LOCALE               = "locale";
    private static final String ARG_VERSION_ID           = "version-id";
    public static final String ARG_CURRENT_VERSION_ID
            = "current-version-id";
    private static final String ARG_PLATFORM_VERSION_ID
            = "platform-version-id";
    private static final String ARG_KNOWN_PLATFORMS      = "known-platforms";
    private static final String REPLY_JNLP_VERSION
            = "x-java-jnlp-version-id";
    private static final String ERROR_MIME_TYPE
            = "application/x-java-jnlp-error";
    private static final String JARDIFF_MIME_TYPE
            = "application/x-java-archive-diff";
    private static final String JNLP_MIME_TYPE
            = "application/x-java-jnlp-file";
    private static final int BASIC_DOWNLOAD_PROTOCOL     = 1;
    private static final int VERSION_DOWNLOAD_PROTOCOL   = 2;
    private static final int EXTENSION_DOWNLOAD_PROTOCOL = 3;
 
    public final static int NORMAL_CONTENT_BIT  = 0x00001;
    public final static int NATIVE_CONTENT_BIT  = 0x00010;
    public final static int JAR_CONTENT_BIT     = 0x00100;
    // used for client-side Pack200 and version download selection
    public final static int PACK200_CONTENT_BIT = 0x01000;
    public final static int VERSION_CONTENT_BIT = 0x10000;
    
    private static int BUF_SIZE = 8192;
    
    private static final String defaultLocaleString =
            Locale.getDefault().toString();
    
    private static HttpRequest _httpRequestImpl;
    private static HttpDownload _httpDownloadImpl;

    public final static String BACKGROUND_STRING = "background";
    public final static String APPCONTEXT_BG_KEY = Config.getAppContextKeyPrefix() + "bg-";

    // This ThreadLocal marks a thread "background" download thread. 
    // It is used to avoid passing down a parameter deep down in a call stack.
    // When a "background" thread calls isUpdateAvailable(), the files
    // in the background blocked list will be checked online.
    // For all other threads, those files are not checked.
    private static InheritableThreadLocal backgroundUpdateThreadLocal = 
        new InheritableThreadLocal() {
            protected Object initialValue() {
                return Boolean.FALSE;
            }
    };

    public static boolean isBackgroundThread() {
        return ((Boolean)backgroundUpdateThreadLocal.get()).booleanValue();
    }

    public static void setBackgroundThread(boolean background) {
        backgroundUpdateThreadLocal.set(new Boolean(background));
    }

    private static boolean isNormalContentType(int type) {
        return ((type & NORMAL_CONTENT_BIT) == NORMAL_CONTENT_BIT);
    }        

    public static boolean isNativeContentType(int type) {
        return ((type & NATIVE_CONTENT_BIT) == NATIVE_CONTENT_BIT);
    }

    public static boolean isJarContentType(int type) {
        return ((type & JAR_CONTENT_BIT) == JAR_CONTENT_BIT);
    }

    public static boolean isPackContentType(int type) {
        return ((type & PACK200_CONTENT_BIT) == PACK200_CONTENT_BIT);
    }

    private static boolean isVersionContentType(int type) {
        return ((type & VERSION_CONTENT_BIT) == VERSION_CONTENT_BIT);
    }
    
    static boolean isPack200Supported() {
        return Config.isJavaVersionAtLeast15();
    }
    
    // Level of compression to use for cache files.
    // 0 = no compression, 9 = best compression
    private static int jarCompression = -1;
    
    public static void setJarCompressionLevel(int level) {
        jarCompression = level;
    }
    
    public static int getJarCompressionLevel() {
        // Get cache compression level.  Default is 0.
        // lazy init
        if (jarCompression == -1) {
            String prop = Config.getProperty(Config.CACHE_COMPRESSION_KEY);
            prop = prop.trim();
            try {
                jarCompression = Integer.valueOf(prop).intValue();
                if ((jarCompression < 0) || (jarCompression > 9)) {
                    jarCompression = 0;
                }
            } catch (NumberFormatException e) {
                jarCompression = 0;
            }
        }
        return jarCompression;
    }
    
    public static String getCachedResourceFilePath(URL resourceURL,
        String versionString) throws IOException {
        if (Cache.isCacheEnabled()) {
            CacheEntry ce = Cache.getCacheEntry(resourceURL, null, versionString);
            if (ce != null) {
                return ce.getResourceFilename();
            }
        } else {
            Object o = MemoryCache.getLoadedResource(resourceURL.toString());
            if (o != null) {
                if (o instanceof JarFile) {
                    return ((JarFile) o).getName();
                } 
                //this is possible if getCacheEntryTemp() is used!
                if (o instanceof CacheEntry) {
                    return ((CacheEntry) o).getResourceFilename();
                }
            }
        }
        throw new IOException("Cannot find cached resource for URL: " + 
            resourceURL.toString());
    }
    
    public static File downloadJarWithoutCache(URL resourceURL, 
            String resourceID, String versionString, DownloadDelegate dd, 
            int contentType) throws IOException {
     
        File f = null;
    
        try {
            // download with client side pack200 or version content type
            f = getJarFileWithoutCache(resourceURL, resourceID, versionString, 
                    dd, contentType);
        } catch (IOException ioe) {
            // re-try with normal jar content type
            f = getJarFileWithoutCache(resourceURL, resourceID, versionString, 
                    dd);
        }

        return f;
    }
    
    public static URL getResource(URL resourceURL, String resourceID,
            String versionString, DownloadDelegate dd, boolean doDownload)
            throws IOException {
        return getResource(resourceURL, resourceID, versionString, dd,
                doDownload, NORMAL_CONTENT_BIT);
    }
    
    public static URL getResource(URL resourceURL, String resourceID,
            String versionString, DownloadDelegate dd, boolean doDownload,
            int contentType)
            throws IOException {

        URL url = null;
        
        // remove resource entry from loaded hashmap to force download
        MemoryCache.removeLoadedResource(resourceURL.toString());
        
        if (Cache.isCacheEnabled()) {
            
            CacheEntry ce = getResourceCacheEntry(resourceURL, resourceID,
                    versionString, dd, doDownload, contentType);            
            
            if (ce != null) {
                url = URLUtil.fileToURL(new File(ce.getResourceFilename()));
            }
        } else {
            if (isJarContentType(contentType) || 
                    isPackContentType(contentType) ||
                    isAlwaysCached(resourceURL.toString().toLowerCase())) {
                
                File f = downloadJarWithoutCache(resourceURL, resourceID, 
                        versionString, dd, contentType);
             
                if (f != null) {
                    JarFile jf = new JarFile(f);
                    // add the jar file to the loaded map
                    MemoryCache.addLoadedResource(resourceURL.toString(), jf);
                }
            }
        }
        return url;
    }
    
    static boolean isZipFile(String filename) {
        return filename.toLowerCase().endsWith(".zip");
    }
    
    // check if the file should be always cached
    public static boolean isAlwaysCached(String filename) {
        return filename.toLowerCase().endsWith(".jar") ||
                filename.toLowerCase().endsWith(".jarjar") ||
                filename.toLowerCase().endsWith(".zip");
    }
    
    public static long getCachedLastModified(final URL href,
            final String resourceID, final String version) throws IOException {
        CacheEntry currentCE = getResourceCacheEntry(href, resourceID,
                version, null, false);
        if (currentCE != null) {
            return currentCE.getLastModified();
        }
        return 0;
    }
    
    public static long getCachedSize(URL resourceURL, String
            resourceID, String versionString, DownloadDelegate dd,
            boolean doDownload) throws IOException {
        CacheEntry ce = null;
        
        ce = getResourceCacheEntry(resourceURL, resourceID, versionString, dd,
                doDownload);
        
        if (ce != null) {
            return ce.getSize();
        }
        return 0;
    }
    
    private static Hashtable noCacheJarFileList = new Hashtable();
    
    public static void clearNoCacheJarFileList() {
        noCacheJarFileList.clear();
    }
    
    private static File getJarFileWithoutCache(final URL resourceURL, 
            String resourceID, final String versionString, 
            final DownloadDelegate dd) throws IOException {
        return getJarFileWithoutCache(resourceURL, resourceID, versionString,
                dd, JAR_CONTENT_BIT);
    }

    // This method simply download the resource into a temp file location
    // and verify the jar file
    private static File getJarFileWithoutCache(final URL resourceURL, String
            resourceID, final String versionString, final DownloadDelegate dd,
            final int contentType) throws IOException {
      
        File cachedFile = (File)noCacheJarFileList.get(resourceURL.toString());
        if (cachedFile != null) {
            return cachedFile;
        }

        byte[] buffer = new byte[8192];
        URL requestURL = getRequestURL(resourceURL, resourceID,
                versionString, null, null, null, null, false, null, 
                versionString == null ?
                    BASIC_DOWNLOAD_PROTOCOL : VERSION_DOWNLOAD_PROTOCOL);
        
        boolean clientSidePack200 = false;
        if (isVersionContentType(contentType)) {
            requestURL = getEmbeddedVersionUrl(requestURL, versionString);
        }
        if (isPackContentType(contentType)) {
            requestURL = getPack200Url(requestURL);
            clientSidePack200 = true;
        }
    
        URLConnection urlConn = requestURL.openConnection();
        if (isPack200Supported()) {
            urlConn.setRequestProperty(HttpRequest.ACCEPT_ENCODING, 
                    HttpRequest.PACK200_GZIP_ENCODING + "," + 
                    HttpRequest.GZIP_ENCODING);
        } else {
            urlConn.setRequestProperty(HttpRequest.ACCEPT_ENCODING,
                    HttpRequest.GZIP_ENCODING);
        }
        InputStream in =  urlConn.getInputStream();
        int length = urlConn.getContentLength();
        String contentEncoding = urlConn.getContentEncoding();
        
        int totalRead = 0;
        if (dd != null) {
            dd.setTotalSize(length);
        }
        /* Setup callback */
        HttpDownloadListener hdl = (dd == null) ? null :
            new HttpDownloadListener() {
            public boolean downloadProgress(int read, int total) {
                dd.downloading(resourceURL,
                        versionString,
                        read,
                        total,
                        false);
                return true;
            }
        };                  
        
        OutputStream out = null;
        File tmpFile = null;
        JarFile result = null;
        if (Trace.isTraceLevelEnabled(TraceLevel.NETWORK)) {
            Trace.println(ResourceManager.getString(
                "httpDownloadHelper.doingDownload",
                requestURL.toString(),
                length,
                contentEncoding),
                TraceLevel.NETWORK);
        }
        try {
            tmpFile = File.createTempFile("jar_cache", null);
            tmpFile.deleteOnExit();
            out = new FileOutputStream(tmpFile);
            int read = 0;
            if (clientSidePack200 || (contentEncoding != null &&
                    contentEncoding.equals(
                    HttpRequest.PACK200_GZIP_ENCODING))) {
                // pack200-gzip encoding
                JarOutputStream jout = new JarOutputStream(out);
                Pack200.Unpacker upkr200  = Pack200.newUnpacker();
                upkr200.unpack(new GZIPInputStream(in, BUF_SIZE), jout);
                jout.close();
            } else {
                if (contentEncoding != null &&
                        contentEncoding.equals(HttpRequest.GZIP_ENCODING)) {
                    // gzip encoding
                    in = new GZIPInputStream(in, BUF_SIZE);
                }
	    	int i = 0;
                while ((read = in.read(buffer)) != -1) {
		    // check first 4 byte for magic number of jar file
	    	    if ( i==0 && !isJarHeaderValid(buffer)) {
			throw new IOException("Invalid jar file");
		    }

                    out.write(buffer, 0, read);
                    // Notify delegate
                    totalRead += read;
                    if (totalRead > length && length != 0) {
                        totalRead = length;
                    }
                    if (hdl != null) {
                        hdl.downloadProgress(totalRead, length);
                    }
		    i++;
                }
            }
            result = new JarFile(tmpFile);
      
        } catch (IOException e) {
            if (tmpFile != null) {
                tmpFile.delete();
            }
            throw e;
        } finally {
            if (in != null) {
                   in.close();
            }
            if (out != null) {
                  out.close();
            }
        }
        if (result != null) {
            // number of entries in jar
            int total = result.size();
            int count = 0;
            if (dd != null) {
                dd.validating(resourceURL, 0, total);
            }
            for (Enumeration e = result.entries() ; e.hasMoreElements() ;) {
                count++;
                JarEntry entry = (JarEntry)e.nextElement();          
                int read = 0;
                try {
                    BufferedInputStream bis =
                        new BufferedInputStream(result.getInputStream(entry));
                    while((read = bis.read(buffer, 0, buffer.length)) != -1) {
                        // Do nothing
                        // Just read. This will throw a security exception if a
                        // signature fails.
                    }
                } catch (java.util.zip.ZipException ze) {
                    throw new JARSigningException(resourceURL, versionString,
                                 JARSigningException.BAD_SIGNING, ze);
                } catch (SecurityException se) {
                    throw new JARSigningException(resourceURL, versionString,
                                 JARSigningException.BAD_SIGNING, se);
                }
                if (dd != null) {
                    // is overkill to track progress of every class validating
                    if (((count % 10) == 0) || count >= total) {
                        dd.validating(resourceURL, count, total);
                    }
                }
            }

	    // Check blacklist
	    if (BlackList.checkJarFile(result)) {
		return null;
	    }

            // we are done with the jar file, close it
             result.close();
        }
        noCacheJarFileList.put(resourceURL.toString(), tmpFile);
        return tmpFile;
    }
    
    // get the cached jar file without update check 
    public static JarFile getCachedJarFile(URL resourceURL, 
        String versionString) throws IOException {
        return getCachedJarFile(resourceURL, versionString, false, 
                NORMAL_CONTENT_BIT);
    }

    // if jar file (location, version) contains library in question 
    // then return name of the directory otherwise return null
    // Note that jar is taken from the cache if available,
    // if it is not then we try to download it
    public static String getLibraryDirForJar(String libname,
            URL location, String version) throws IOException {
        File f = DownloadEngine.getCachedFile(location, version);
        if (f == null) { //not in the cache
            //try to download
            CacheEntry ce = null;
            ce = getResourceCacheEntry(location, null, version, null,
                    true, false, null, (DownloadEngine.NATIVE_CONTENT_BIT |
                    DownloadEngine.JAR_CONTENT_BIT));
            if (ce != null) {
                f = ce.getDataFile();
            }
        }
        if (f != null) {
            String dir = f.getPath() + "-n";
            File ff = new File(dir, libname);
            Trace.println("Looking up native library in: " + ff, 
                    TraceLevel.NETWORK);
            if (ff.exists()) {
                return dir;
            }
        }
        return null;
    }

    // get the cached jar file with update check
    public static JarFile getUpdatedJarFile(URL resourceURL, 
        String versionString) throws IOException {
        return getCachedJarFile(resourceURL, versionString, true, 
                NORMAL_CONTENT_BIT);
    }
    
     // get the cached jar file with update check 
    public static JarFile getUpdatedJarFile(URL resourceURL, 
        String versionString, int contentType) throws IOException {
        return getCachedJarFile(resourceURL, versionString, true, contentType);
    }
    
    private static JarFile getCachedJarFile(URL resourceURL, 
        String versionString, boolean doDownload, int contentType) throws IOException {
        if (Cache.isCacheEnabled()) {
            
            CacheEntry ce = null;
            
            ce = getResourceCacheEntry(resourceURL, null,
                    versionString, null, doDownload, contentType);
            
            if (ce != null) {          
                return ce.getJarFile();
            }

        } else {
            // use jar file from loaded map if available
            Object result = MemoryCache.getLoadedResource(
                                               resourceURL.toString());
            if (result != null && result instanceof JarFile) {
                return ((JarFile) result);
            }

            // otherwise download file into temp location (no-cache)
            File f = getJarFileWithoutCache(resourceURL, null, 
                                                versionString, null);
            if (f != null) {
                JarFile jf = new JarFile(f);

		if (jf != null && BlackList.checkJarFile(jf)) {
		    return null;
		}

                // add jar file to loaded map
                MemoryCache.addLoadedResource(resourceURL.toString(), jf);
                return jf;
            }            
        }
        return null;
    }
    
    // get the cached shortcut image without update check 
    public static File getCachedShortcutImage(URL resourceURL, 
        String versionString) throws IOException {
        return getShortcutImage(resourceURL, versionString, false);
    }
    
    // get the shortcut image with update check 
    public static File getUpdatedShortcutImage(URL resourceURL,
        String versionString) throws IOException {
        return getShortcutImage(resourceURL, versionString, true);
    }
    
    private static File getShortcutImage(URL resourceURL, 
    String versionString, boolean doDownload) throws IOException {
        CacheEntry ce = getResourceCacheEntry(resourceURL, null, versionString, 
            null, doDownload);
        if (ce != null) {            
            ce.generateShortcutImage();    
            return ce.getDataFile();
        }
        return null;
    }

    public static File getCachedFileNative(URL url) throws IOException {

        if (url.getProtocol().equals("jar")) {
            String path = url.getPath();
            int bang = path.indexOf("!/");
            if (bang > 0) try {
                String entry = path.substring(bang+2);
                URL lib = new URL(path.substring(0,bang));
                // NOTE: if version based resource won't work ...
                CacheEntry ce = Cache.getCacheEntry(lib, null, null);
                if (ce != null) {
                    String dir = ce.getNativeLibPath();
                    if (dir != null) {
                        return new File(dir, entry);
                    }
                }
            } catch (MalformedURLException mue) {
                Trace.ignored(mue);
            } catch (IOException ioe) {
                Trace.ignored(ioe);
            }
            return null;
        } else  {
            return getCachedFile(url);
        }
    }
    
    public static File getCachedFile(URL resourceURL) throws IOException {
        return getCachedFile(resourceURL, null);
    }
    
    public static File getCachedFile(URL resourceURL, String versionString) 
        throws IOException {
        return getCachedFile(resourceURL, versionString, false, false, null);
    }
    
    public static File getUpdatedFile(URL resourceURL, String versionString) 
        throws IOException {
        return getUpdatedFile(resourceURL, versionString, false, null);
    }
    
    public static File getUpdatedFile(URL resourceURL, String versionString,
        boolean isPlatformVersion, String knownPlatforms) throws IOException {
        return getCachedFile(resourceURL, versionString, true,
                        isPlatformVersion, knownPlatforms);
    }
    
    public static File getCachedFile(URL resourceURL, String versionString,
        boolean doDownload, boolean isPlatformRequest, String knownPlatforms) 
        throws IOException {
        CacheEntry ce = null;
        ce = getResourceCacheEntry(resourceURL, null, versionString, null,
            doDownload, isPlatformRequest, knownPlatforms, NORMAL_CONTENT_BIT);
        if (ce != null) {
            return ce.getDataFile();
        }
        return null;
    }    
    
    public static boolean isJarFileCorrupted(URL resourceURL, 
            String versionString) {
        JarFile jf = null;
        try {
            CacheEntry ce = null;
            ce = getResourceCacheEntry(resourceURL, null, versionString, null,
                    false);
            // do not use ce.getJarFile, because it will create the WeakReference
            // for the jar file object, which will be cleared immediately when
            // this method is done, because it only needs the jarfile object
            // in this method.  When the weakRef got cleared, the cache entry
            // will be removed from the loaded resource table, which will
            // slow down the loading of the cache entry later.
            jf = new JarFile(ce.getResourceFilename(), false);
            if (jf != null) {
                return false;
            }
        } catch (IOException ioe) {
            Trace.ignoredException(ioe);
        } finally {
            try {
                if (jf != null) {
                    jf.close();
                }
            } catch (IOException ioe) {
                Trace.ignoredException(ioe);
            }
        }
        return true;
    }
    
    public static boolean isResourceCached(URL resourceURL, String
            resourceID, String versionString) throws IOException {
        return isResourceCached(resourceURL, resourceID, versionString,
                NORMAL_CONTENT_BIT);
    }

    public static boolean isResourceCached(URL resourceURL, String
            resourceID, String versionString, int contentType)
            throws IOException {
        CacheEntry ce = null;
        ce = getResourceCacheEntry(resourceURL, resourceID, versionString, null,
                false, contentType);
        if (ce != null) {
            return true;
        }
        return false;
    }
    
   
    
    public static void removeCachedResource(URL resourceURL, String resourceID,
            String versionString) {
        CacheEntry ce = null;
        try {
            ce = getResourceCacheEntry(resourceURL, resourceID, versionString,
                    null, false);
        } catch (IOException ioe) {
            Trace.ignoredException(ioe);
        }
        if (ce != null) {
            Cache.removeCacheEntry(ce);
        }
    }
    
    public static Map getCachedHeaders(URL resourceURL, String
            resourceID, String versionString, DownloadDelegate dd,
            boolean doDownload) throws IOException {
        CacheEntry ce = null;
        ce = getResourceCacheEntry(resourceURL, resourceID, versionString, dd,
                doDownload);
        if (ce != null) {
            return ce.getHeaders();
        }
        return null;
    }
    
    // Initialize all singleton objects
    static {
        _httpRequestImpl = new BasicHttpRequest();
        _httpDownloadImpl = new HttpDownloadHelper(_httpRequestImpl);
    }
    
    /** Get implementation of the low-level communication object */
    public static HttpRequest getHttpRequestImpl() {
        return _httpRequestImpl;
    }
    public static HttpDownload getHttpDownloadImpl() {
        return _httpDownloadImpl;
    }
    
    /* Callback interface that can be used to track the downloading progress */
    public interface DownloadDelegate {
        public void setTotalSize(long size);
        public void downloading(URL rc, String version, int readSoFar,
                int estimatedSize, boolean willPatch);
        public void validating(URL rc, int entry, int total);
        public void patching(URL rc, String version, int percentDone);
        public void downloadFailed(URL rc, String version);
    }
    
    /**
     * Applies the delta at <code>deltaPath</code> to <code>dce</code>,
     * returning a File to the resulting file. <code>diffMimeType</code>
     * gives the mime type for the delta.
     */
    static public File applyPatch(File baseFile, File jardiffFile,
            final URL location, final String newVersion,
            final DownloadDelegate delegate, String resultFilename)
            throws FailedDownloadException {

        
        // Get patcher
        Patcher differ = new JarDiffPatcher();
        
        File result = new File(resultFilename);
        OutputStream out = null;
        boolean done = false;
        try {
            out = new FileOutputStream(result);
            Patcher.PatchDelegate pd = null;
            
            if (delegate != null) {
                delegate.patching(location, newVersion, 0);
                pd = new Patcher.PatchDelegate() {
                    public void patching(int percentDone) {
                        delegate.patching(location, newVersion, percentDone);
                    }
                };
            }
            try {
                differ.applyPatch(pd, baseFile.getPath(),
                        jardiffFile.getPath(), out);
            } catch(IOException ioe) {
                throw new FailedDownloadException(location,
                        newVersion, ioe);
            }
            done = true;
        } catch(IOException ioe) {
            
            
            
            throw new FailedDownloadException(location,
                    newVersion, ioe);
        } finally {
            try {
                if (out != null) out.close();
            } catch (IOException ioe) {
                Trace.ignoredException(ioe);
            }
            if (!done) {
                result.delete();
            }
            jardiffFile.delete();
            if (delegate != null && !done) {
                delegate.downloadFailed(location, newVersion);
            }
        }
        return result;
    }

    public static boolean isJnlpURL(URL url) {
        try {
            HttpResponse response = getHttpRequestImpl().doHeadRequest(url);
            return (response.getContentType().equals(JNLP_MIME_TYPE));
        } catch (IOException ioe) {
            return false;
        }
    }
             
    private static ArrayList updateCheckDoneList = new ArrayList();   
    private final static Object syncObject = new Object();
    
    public static void addToUpdateCheckDoneList(String url) {
        if (url != null) {
            synchronized(syncObject) {
                updateCheckDoneList.add(url);
            }
        }
    }

    public static void removeFromUpdateCheckDoneList(String url) {
        synchronized(syncObject) {
            int i = -1;
            // in case there are duplicate entries, remove them all
            while ( (i = updateCheckDoneList.indexOf(url)) > -1 ) {
                updateCheckDoneList.remove(i);
            }
        }
    }
    
    public static void clearUpdateCheckDoneList() {
        synchronized(syncObject) {
            updateCheckDoneList.clear();
        }
    }

    // Helper method
    public static boolean isUrlInAppContext(URL href) {
        String background = (String)AppContext.getAppContext().get(APPCONTEXT_BG_KEY +href);
        if ((background != null) && background.equals(BACKGROUND_STRING)) {
            return true;
        }
        return false;
    }

    public static boolean isUpdateAvailable(URL href, String version) throws
            IOException {
        return isUpdateAvailable(href, version, false, null);
    }
    
    public static boolean isUpdateAvailable(URL href, String version, 
            boolean clientSidePack200) throws IOException {
        return isUpdateAvailable(href, version, clientSidePack200, null);
    }

    public static boolean isUpdateAvailable(URL href, String version, 
                       boolean clientSidePack200, Map requestHeaders) throws 
                       IOException {
        
        synchronized(syncObject) {
            if (updateCheckDoneList.contains(href.toString())) {
                return false;
            }          
        }

	// no need to do update check for versioned resource, if it is
        // found in cache already
        if (version != null) {
            return false;
        }

        URL urlNoQuery = HttpUtils.removeQueryStringFromURL(href);

        // return false if this resource will be checked on 
        // background thread
        if (!isBackgroundThread() 
            && isUrlInAppContext(href)) {
            return false;
        }
        
        CacheEntry ce = null;

        if (Cache.isCacheEnabled()) {
            ce = Cache.getCacheEntry(
                version == null ? href: urlNoQuery, null, version);
        }
     
     
        // look at expire and no-cache http header to determine whether
        // we need a revalidation of the cache entry
        if (isValidationRequired(ce) == false) {
            
            return false;
        }      
        
        // check if the system is online; prompt user to go online if
        // we are not connected to the network
        if (DeployOfflineManager.promptUserGoOnline(href) == false) {
            throw new FailedDownloadException(href, null, null, true);
        }     
        
        if (DeployOfflineManager.isGlobalOffline()) {
            throw new FailedDownloadException(href, null, null, true);
        }

        // if no cache entry, return true since we must download
        if (ce == null) {
            return true;
        }
     
        URL requestURL = getRequestURL(href, null, version,
                null, null, null, null, false, null,
                version == null ?
                    BASIC_DOWNLOAD_PROTOCOL : VERSION_DOWNLOAD_PROTOCOL);
  
        // Do a HTTP request
        HttpRequest httpreq = getHttpRequestImpl();
        HttpResponse response = null;
        long cacheEntryLastModified = -1;
        
        cacheEntryLastModified = ce.getLastModified();
 
        URL pack200RequestURL = null;
       
        if (clientSidePack200) {
            pack200RequestURL = getPack200Url(requestURL);
        }
        
        String[] keys = null;
        String[] values = null;
        
        if (requestHeaders != null) {
            
            keys = (String[])(requestHeaders.keySet().toArray(new String[0]));
            
            values = new String[keys.length];
            
            for (int i = 0; i < keys.length; i++) {
                Object o = requestHeaders.get(keys[i]);
                if (o != null && o instanceof List) {
                    values[i] = (String) (((List) o).get(0));
                }
            }
        }
        
        try {
            // use pack200 url if pack200 enabled
            response = httpreq.doGetRequestEX(pack200RequestURL != null ?
                pack200RequestURL : requestURL, keys, values, 
                cacheEntryLastModified);
        } catch (FileNotFoundException fnfe) {
            if (pack200RequestURL == null) {
                throw fnfe;
            }
            // retry with non pack200 url
            response = httpreq.doGetRequestEX(requestURL, 
                    cacheEntryLastModified);
        }

        if (response == null) {          
            // cannot complete request from server, so we
            // must download again to make sure resource is update to date
            return true;
        }       
        
        int statusCode = response.getStatusCode();
        response.disconnect();        
      
        boolean updateAvail = true;
  
        if (statusCode == HttpURLConnection.HTTP_NOT_MODIFIED) {
            updateAvail = false;
        } else if (statusCode == HttpURLConnection.HTTP_OK) {
            // for server that do not understand the if-modified-since
            // header, it will always return HTTP_OK (200), even if
            // there is no update for the resoruce
            // this applies to file URL as well
            // manually check for content-length and lastModified to see
            // if there is an update
            int length  = response.getContentLength();
            long lastModified = response.getLastModified();    
            
            if (lastModified == cacheEntryLastModified && 
                    length == ce.getContentLength()) {
                updateAvail = false;
            }
        }
        
        if (updateAvail == false) {
            synchronized (syncObject) {
                updateCheckDoneList.add(href.toString());
            }
            long expiration = response.getExpiration();

            if (expiration != 0) {
                // write new expiration to cache index file
                ce.updateExpirationInIndexFile(expiration);
            }
            
        }
        return updateAvail;
    }
    
    private static String getVersionJarPath(String path, String version) {
        String filename = path.substring(path.lastIndexOf("/") + 1);
        path = path.substring(0, path.lastIndexOf("/") + 1);
        String name = filename;
        String ext = null;

        if (filename.lastIndexOf(".") != -1) {
            ext = filename.substring(filename.lastIndexOf(".") + 1);

            filename = filename.substring(0, filename.lastIndexOf("."));

        }
        StringBuffer filenameSB = new StringBuffer(filename);
        if (version != null) {
            filenameSB.append("__V");
            filenameSB.append(version);
        }

        if (ext != null) {
            filenameSB.append(".");
            filenameSB.append(ext);
        }

        path += filenameSB.toString();

        return path;
    }
    
    private static URL getEmbeddedVersionUrl(URL u, String versionString) {
        if (versionString == null || versionString.indexOf("*") != -1 ||
                versionString.indexOf("+") != -1) {
            // do not support * or + in version string
            return u;
        }
        URL versionURL = null;
            
        String protocol = u.getProtocol();
       
        String host = u.getHost();
      
        int port = u.getPort();
      
        String path = u.getPath();
        
        path = getVersionJarPath(path, versionString);
        
        try {
            versionURL = new URL(protocol, host, port, path);
        } catch (MalformedURLException mue) {
            // should not happen
            Trace.ignoredException(mue);
        }
        
        return versionURL;
    }
    
    // append .pack.gz to end of url path
    public static URL getPack200Url(URL u) {
        URL pack200RequestURL = null;
        
        if (u == null) {
            return null;
        }
                
        // add .pack.gz to end of url       
        String protocol = u.getProtocol();
       
        String host = u.getHost();
      
        int port = u.getPort();
      
        String path = u.getPath();
   
        String query = u.getQuery();
      
        StringBuffer file = new StringBuffer(path);
        file.append(".pack.gz");
        if (query != null) {
            file.append("?");
            file.append(query);
        }
        
        try {
            pack200RequestURL = new URL(protocol, host, port,
                    file.toString());
        } catch (MalformedURLException mue) {
            // should not happen
            Trace.ignoredException(mue);
        }
        
        return pack200RequestURL;
    }
    
    private static CacheEntry actionDownload(CacheEntry currentCE,
            final URL href, URL requestURL, String resourceID,
            String versionString, final DownloadDelegate dd,
            final int contentType, boolean isPlatformRequest, boolean enable) 
            throws IOException {
        boolean retry = false;
        int retryContentType = -1;
        
        boolean removeCurrentCE = false;
        String currentVersion = null;
        if (currentCE != null && versionString != null) {
            // found matching versioned resource in cache
            // since it's a versioned resource, not need to check for
            // timestamp on server, return resource directly
            if (currentCE.getVersion() != null) {
                if (new VersionString(versionString).contains(
                        new VersionID(currentCE.getVersion()))) {
                    return currentCE;
                }
            }
            
            if (Environment.isJavaPlugin() == false) {
                // java web start, do nothing
            } else {
                // For java plugin, need to check expiration date of cached
                // resource
                long expiration = currentCE.getExpirationDate();
                if (expiration != 0) {
                    if ((new Date()).after(new Date(expiration))) {
                        // expired entry, should download again
                        currentCE = null;
                    }
                } 
            }          
        }
        if (currentCE != null) {
            currentVersion = currentCE.getVersion();
        }
        CacheEntry ce = null;
        try {
            // Do a HTTP request
            HttpRequest httpreq = getHttpRequestImpl();
            HttpResponse response = null;
            // this should be initialized to 0 (instead of -1)
            // otherwise we will send bogus date for ifModifiedSince header
            // for first download of resource
            // this value is used for the ifModifiedSince header, and the
            // default should be 0 (see URLConnection.ifModifiedSince)
            long cacheEntryLastModified = 0;
            if (currentCE != null) {
                cacheEntryLastModified = currentCE.getLastModified();
            }
            // We try download in the default mode, and retry
            // w/o Http or Pack200 compression.
            URL realRequestURL = requestURL;

            if (isVersionContentType(contentType)) {
                realRequestURL = getEmbeddedVersionUrl(realRequestURL,
                        versionString);
            }

            if (isPackContentType(contentType)) {
                realRequestURL = getPack200Url(realRequestURL);
            }
            
            try {
                try {
                    // use pack200 url if pack200 enabled
                    response =
                            httpreq.doGetRequestEX(realRequestURL,
                                cacheEntryLastModified);
                } catch (FileNotFoundException fnfe) {
                    if (requestURL.toString().equals(
                            realRequestURL.toString())) {
                        throw fnfe;
                    }
                    // retry with normal url if .pack.gz file not found
                    response =
                            httpreq.doGetRequestEX(requestURL,
                            cacheEntryLastModified);

                    retry = true;
                    retryContentType = contentType;
                    // reset to normal content type
                    if (isPackContentType(contentType) && 
                            (isNativeContentType(contentType) || 
                            isJarContentType(contentType))) {
                        retryContentType = retryContentType & 
                                ~PACK200_CONTENT_BIT;
                    }  
                    if (isVersionContentType(contentType)) {
                        retryContentType = retryContentType & 
                                ~VERSION_CONTENT_BIT;
                    }
             
                }
            } catch (FailedDownloadException fde) {
                throw fde;
            } catch (IOException ioe) {
                Trace.ignoredException(ioe);             
                response = httpreq.doGetRequest(requestURL, false);
            }
            
            int statusCode = response.getStatusCode();
            
            // Check for an HTTP 404
            if (statusCode == HttpURLConnection.HTTP_NOT_FOUND) {
                throw new FailedDownloadException(href,
                        versionString,
                        new IOException("HTTP response 404"));
            } else if (statusCode == HttpURLConnection.HTTP_NOT_MODIFIED) {
                response.disconnect();
                return currentCE;
            }
            
            int length  = (int)response.getContentLength();
            long lastModified = response.getLastModified();          
            long expiration = response.getExpiration();
            
            boolean isJnlp = JNLP_MIME_TYPE.equals(response.getContentType());

            if (!isJnlp && Environment.isImportMode() &&
                    Environment.getImportModeExpiration() != null &&
                    expiration != 0) {
                Date httpExpirationDate = new Date(expiration);
    
                if (httpExpirationDate.before(
                        Environment.getImportModeExpiration())) {
                    response.disconnect();
                    return null;
                }
            } else if (!isJnlp && Environment.isImportMode() &&
                    Environment.getImportModeTimestamp() != null &&
                    lastModified != 0) {
                // check last modified if expiration is not specified
                Date httpLastModifiedDate = new Date(lastModified);

                if (httpLastModifiedDate.before(
                        Environment.getImportModeTimestamp())) {
                    response.disconnect();
                    return null;
                }
            }         
            
            String downloadVersion = response.getResponseHeader(
                    REPLY_JNLP_VERSION);
      
            if ((versionString != null) && (downloadVersion == null) &&
                    (Environment.getImportModeCodebaseOverride() != null) &&
                    ((new VersionID(versionString)).isSimpleVersion())) {
                downloadVersion = versionString;
            }           

            if (currentCE != null) {
                
                if (downloadVersion != null &&
                        new VersionString(downloadVersion).contains(
                        currentVersion)) {
                    response.disconnect();
                    return currentCE;
                }
                
                // compare contentLength and lastModified for non-versioned
                // resource
                if (length == currentCE.getContentLength() &&
                        lastModified == cacheEntryLastModified &&
                        currentVersion == null) {
                    response.disconnect();
                    return currentCE;
                }
                
                if (currentVersion == null) {
                    // non-versioned current cache entry is out of date,
                    // remove it
                    removeCurrentCE = true;
                }
                
            }
            
            if (dd != null) {
                dd.setTotalSize(length);
            }
            
            if (downloadVersion == null && (
                    Environment.isJavaPlugin() || 
                    isVersionContentType(retry ? 
                        retryContentType : contentType))) {
                downloadVersion = versionString;
            }
            
            final String responseVersion = downloadVersion;
            
            
            String mimeType = response.getContentType();
            
            boolean applyJarDiff = (mimeType != null) &&
                    mimeType.equalsIgnoreCase(JARDIFF_MIME_TYPE);

            if (Trace.isTraceLevelEnabled(TraceLevel.NETWORK)) {
                Trace.println(ResourceManager.getString(
                    "downloadEngine.serverResponse", String.valueOf(length),
                    (new Date(lastModified)).toString(), downloadVersion,
                    mimeType), TraceLevel.NETWORK);
            }
            
            /** Check for error condition */
            if (mimeType != null &&
                    mimeType.equalsIgnoreCase(ERROR_MIME_TYPE)) {
                BufferedInputStream is = response.getInputStream();
                // Error got returned
                BufferedReader br =
                        new BufferedReader(new InputStreamReader(is));
                String errline = br.readLine();
                br.close();
                throw new FailedDownloadException(requestURL,
                        versionString,
                        new IOException("Error returned: " + errline));
            }
            
            if  (downloadVersion == null && versionString != null &&
                    isVersionContentType(retry ? 
                        retryContentType : contentType) == false) {
                throw new FailedDownloadException(href,
                        versionString,
                        new IOException("missing version response from server"));
            } else if (downloadVersion != null && versionString != null) {
                // Check that the product version matches. This is needed
                // in all cases, except when we did a platform-version request
                if (!isPlatformRequest) {
                    
                    if (!(new VersionString(versionString)).contains(
                            downloadVersion)) {
                        throw new FailedDownloadException(href,
                            versionString,
                            new IOException("bad version response from server:" +
                            downloadVersion));
                        
                    }
                    
                    VersionID downloadVersionId =
                            new VersionID(downloadVersion);
                    if (!downloadVersionId.isSimpleVersion()) {
                        throw new FailedDownloadException(href,
                            versionString,
                            new IOException("bad version response from server:" +
                            downloadVersion));
                    }
                }
            }
            
            /* Setup callback */
            HttpDownloadListener hdl = (dd == null) ? null :
                new HttpDownloadListener() {
                public boolean downloadProgress(int read, int total) {
                    dd.downloading(href,
                            responseVersion,
                            read,
                            total,
                            false);
                    return true;
                }
            };
          
            // download the resource into the cache
            if (enable && allowCaching(response)) {
                return Cache.downloadResourceToCache(href, downloadVersion, 
                       response, hdl, dd, removeCurrentCE, requestURL, currentCE,
                       applyJarDiff, retry ? retryContentType : contentType);
            } else {
                // download to a temp cache entry. The entry is not enabled yet.
                CacheEntry tce = Cache.downloadResourceToTempFile(href, 
                    downloadVersion, response, hdl, dd, removeCurrentCE, 
                    requestURL, currentCE, applyJarDiff,
                    retry ? retryContentType : contentType);
                tce.setBusy(0);
                tce.setIncomplete(1);
                tce.updateIndexHeaderOnDisk();
                return tce;
            }
        } catch(Exception e) {
            if (ce != null) {
                Cache.removeCacheEntry(ce);
            }
            if (e instanceof JARSigningException) {
                throw (JARSigningException) e;
            }
            if (e instanceof FailedDownloadException) {
                throw (FailedDownloadException)e;
            }            
            /* Wrap exception */
            throw new FailedDownloadException(requestURL,
                    versionString, e);
        }
        
    }

    private static boolean allowCaching(HttpResponse response) {
        if (response != null) {
            String control = response.getResponseHeader("cache-control");
            if (control != null && 
                           control.toLowerCase().indexOf("no-store") != -1) {
                Trace.println("Not caching resource due to response header: " +
                    "cache-control: no-store", TraceLevel.NETWORK);
                return false;
            }
        }
        return true;
    }

    private static void addURLArgument(StringBuffer sb, String key,
            String value) {
        try {
            sb.append(URLEncoder.encode(key, "UTF-8"));
            sb.append('=');
            sb.append(URLEncoder.encode(value, "UTF-8"));
            sb.append('&');
        } catch (UnsupportedEncodingException uee) {
            Trace.ignoredException(uee);
        }
    }
    
    /** Returns the requst URL based on the current cache entry */
    private static URL getRequestURL(URL href, String resourceID,
            String versionString, String currentVersion, String arch,
            String os, String locale, boolean isPlatformRequest,
            String knownPlatforms, int kind) {
        
        StringBuffer args = new StringBuffer();
        if (currentVersion == null && versionString != null) {
            
            currentVersion = Cache.getCacheEntryVersion(href, resourceID);
            
        }
        
        // version download protocol
        if (versionString != null && kind == VERSION_DOWNLOAD_PROTOCOL) {
            addURLArgument(args, ARG_VERSION_ID, versionString);
            
            // Add incremental download for JAR files, jardiff support
            if (currentVersion != null) {
                addURLArgument(args, ARG_CURRENT_VERSION_ID, currentVersion);
            }
        }
        
        // extension download protocol
        if (versionString != null && kind == EXTENSION_DOWNLOAD_PROTOCOL) {
            if (isPlatformRequest) {
                addURLArgument(args, ARG_PLATFORM_VERSION_ID, versionString);
            } else {
                addURLArgument(args, ARG_VERSION_ID, versionString);
            }
            addURLArgument(args, ARG_ARCH, Config.getOSArch());
            addURLArgument(args, ARG_OS, Config.getOSName());
            addURLArgument(args, ARG_LOCALE, defaultLocaleString);
            if (knownPlatforms != null) {
                addURLArgument(args, ARG_KNOWN_PLATFORMS, knownPlatforms);
            }
        }
        // Remove last '&', and insert '?' for non-empty strings
        if (args.length() > 0) args.setLength(args.length()-1);
        if (args.length() > 0) args.insert(0, '?');
        
        try {
            
            if (Environment.getImportModeCodebaseOverride() != null &&
                    Environment.getImportModeCodebase() != null) {            
                return new URL(
                        Environment.getImportModeCodebaseOverride() +
                        href.getFile().substring(
                        Environment.getImportModeCodebase().getFile().length()) +
                        args);
            }            
            return new URL(href.getProtocol(),
                    href.getHost(),
                    href.getPort(),
                    href.getFile() + args);
        } catch(MalformedURLException mue) {
            Trace.ignoredException(mue);
            return null;
        }
    }
    

    // Download a resource to a inactive and temporary cache entry
    // the cache entry will be activated when all resources are downloaded
    public static CacheEntry getResourceTempCacheEntry(URL resourceURL,
                                                       String versionString,
                                                       int contentType)
        throws IOException {
        // get current Cache Entry
        CacheEntry ce = Cache.getCacheEntry(resourceURL, null,
                                            versionString, contentType);

        // download update to temp cache entry
        CacheEntry newCE = getCacheEntry(ce, resourceURL, null, versionString, null,
                             false, null, contentType, false /* disable ce*/);

        return newCE;
    }

    
    // Download a resource to a inactive and temporary cache entry,
    // which is allowed even cache is disabled, as long it won't get activated
    public static CacheEntry getCacheEntryTemp( URL resourceURL,
                                                String resourceID, String versionString,
                                                DownloadDelegate dd,
                                                boolean isPlatformRequest, String knownPlatforms,
                                                int contentType)
        throws IOException {

        return getCacheEntry(null, resourceURL, resourceID, versionString, dd, 
                             isPlatformRequest, knownPlatforms, contentType, false);
    }

    private static CacheEntry getCacheEntry(CacheEntry ce, URL resourceURL,
                                            String resourceID, String versionString, 
                                            DownloadDelegate dd,
                                            boolean isPlatformRequest, String knownPlatforms, 
                                            int contentType)
        throws IOException {

        return getCacheEntry(ce, resourceURL, resourceID, versionString, dd, 
                             isPlatformRequest, knownPlatforms, contentType, true);
    }
        
    private static CacheEntry getCacheEntry(CacheEntry ce, URL resourceURL,
            String resourceID, String versionString, DownloadDelegate dd,
            boolean isPlatformRequest, String knownPlatforms, int contentType, 
                                            boolean enable)
            throws IOException {
        
        int type = BASIC_DOWNLOAD_PROTOCOL;
        
        if (knownPlatforms != null) {
            type = EXTENSION_DOWNLOAD_PROTOCOL;
        } else if (versionString != null) {
            type = VERSION_DOWNLOAD_PROTOCOL;
        }
        
        URL requestURL = getRequestURL(resourceURL, resourceID, versionString,
                null, null, null, null, isPlatformRequest, knownPlatforms,
                type);
        
        return actionDownload(ce, resourceURL, requestURL, resourceID,
                              versionString, dd, contentType, isPlatformRequest, enable);
    }
    
    private static CacheEntry getResourceCacheEntry(URL resourceURL, String
            resourceID, String versionString, DownloadDelegate dd,
            boolean doDownload, int contentType)
            throws IOException {
        
        return getResourceCacheEntry(resourceURL, resourceID, versionString,
                dd, doDownload, false, null, contentType);
    }
    
    private static CacheEntry getResourceCacheEntry(URL resourceURL, String
            resourceID, String versionString, DownloadDelegate dd,
            boolean doDownload) throws IOException {
        
        return getResourceCacheEntry(resourceURL, resourceID, versionString,
                dd, doDownload, false, null, DownloadEngine.NORMAL_CONTENT_BIT);
    }
    
    private static boolean isValidationRequired(CacheEntry ce) {
        if (ce == null) return true;
        boolean revalidate = true;
        // no need to download/revalidate if cache entry is not exipred
        if (ce.isExpired() == false) {
            revalidate = false;
        }
        // however, if no-cache http cache control directive is set,
        // need to revalidate anyway
        if (ce.isHttpNoCacheEnabled()) {
            revalidate = true;
        }
        return revalidate;
    }
    
    private static CacheEntry getResourceCacheEntry(URL resourceURL, String
            resourceID, String versionString, DownloadDelegate dd,
            boolean doDownload, boolean isPlatformRequest,
            String knownPlatforms, int contentType) throws IOException {

        // should not return (temp) cached resource if cache is disabled and no download allowed
        if ( !Cache.isCacheEnabled() && !doDownload ) {
            return null;
        }

        CacheEntry ce = null;

        // update, if !cacheEnable || ( doDownload && updateAvailable )
        boolean doUpdate = !Cache.isCacheEnabled() ||
                           ( doDownload && 
                             isUpdateAvailable(resourceURL, 
                                             versionString, isPackContentType(contentType)) );

        if(!doUpdate) {
            // look in loaded memory hash map
            ce = (CacheEntry) MemoryCache.getLoadedResource(resourceURL.toString());

            if (ce != null) {
                String currentVersion = ce.getVersion();
                // make sure we do not return versioned resource for
                // non-vesrion request
                if ((versionString == null && currentVersion == null )||
                        (versionString != null && currentVersion != null &&
                        currentVersion.compareTo(versionString) >= 0)) {
                    return ce;
                }
            }
            // CacheEntry from loaded memory map is not what we want            
        }
        
        if ( Cache.isCacheEnabled() ) {
            // check if resource is cached in file cache
            ce = Cache.getCacheEntry(resourceURL, resourceID,
                                     versionString, contentType);
        }
        
        // fetch the cache resource
        if ( doDownload && ( ce==null || isValidationRequired(ce) ) ) {
            // this will trigger a JavaFX ping if we are in JavaFX preload/au
            // mode
            Environment.setDownloadInitiated(true);
            // actually download the resource
            if ( Cache.isCacheEnabled() ) {
                ce = getCacheEntry(ce, resourceURL, resourceID,
                        versionString, dd, isPlatformRequest,
                        knownPlatforms, contentType); // with pack200
            } else {
                ce = getCacheEntryTemp(resourceURL, resourceID,
                        versionString, dd, isPlatformRequest,
                        knownPlatforms, contentType); // with pack200
            }
            
            // add to loaded hash table
            // only add to loaded resource if resource is validated
            MemoryCache.addLoadedResource(resourceURL.toString(), ce);
            
            if ( ce != null && Cache.isCacheEnabled() ) {
                // activate/touch the cache entry index file
                Cache.touch(new File(ce.getResourceFilename() +
                        Cache.getIndexFileExtension()));
            }
        }
        
        return ce;
    }
        
    public static String getAvailableVersion(URL url, String requestVersion,
            boolean isPlatform, String platforms) {

        int type =  (platforms != null) ?
            EXTENSION_DOWNLOAD_PROTOCOL : VERSION_DOWNLOAD_PROTOCOL;
       
        URL requestURL = getRequestURL(url, null, requestVersion,
                null, null, null, null, isPlatform, platforms, type);
       
        // Do a HTTP request
        HttpRequest httpreq = getHttpRequestImpl();
        HttpResponse response = null;
        String replyVersion = null;
   
        try {
            response = httpreq.doGetRequest(requestURL);
            if (response != null) {
                replyVersion = response.getResponseHeader(REPLY_JNLP_VERSION);
                response.disconnect();
            }
        } catch (Throwable t) {
            Trace.ignored(t);
        }
        return replyVersion;
    }    

    /* Check the magic number of this jar file */
    static boolean isJarHeaderValid(byte[] buf) {
	if (get32(buf,0) == java.util.jar.JarEntry.LOCSIG) {
	    return true;
	}
	return false;
    }

    /*
     * Fetches unsigned 16-bit value from byte array at specified offset.
     * The bytes are assumed to be in Intel (little-endian) byte order.
     */
    private static final int get16(byte b[], int off) {
        return (b[off] & 0xff) | ((b[off+1] & 0xff) << 8);
    }

    /*
     * Fetches unsigned 32-bit value from byte array at specified offset.
     * The bytes are assumed to be in Intel (little-endian) byte order.
     */
    private static final long get32(byte b[], int off) {
        return get16(b, off) | ((long)get16(b, off+2) << 16);
    }
}
