/*
 * %W% %E%
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.cache;
import java.io.*;
import java.net.*;
import java.util.*;
import java.util.zip.*;
import java.util.jar.*;
import com.sun.javaws.Globals;
import com.sun.javaws.jardiff.*;
import com.sun.javaws.util.*;
import com.sun.javaws.security.SigningInfo;
import com.sun.javaws.exceptions.*;
import com.sun.javaws.net.*;
import com.sun.javaws.JavawsFactory;
import com.sun.deploy.config.Config;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;

/**
 *  A class that implements the basic and version-based
 *  download protocol for jar files, images, and JNLP files
 *
 *  The main method is:
 *
 *  get(URL url, String version, int type)
 *    - url is the location of the resource
 *    - version is the version requested (null if no version is specified)
 *    - type is resource type as specified
 *        (JAR_DOWNLOAD, IMAGE_DOWNLOAD, JNLP_DOWNLOAD)
 *
 *  They will download the specific resource into the cache, and
 *  return a File object to where it can be found, or return
 *  an entry directly into the cache.
 *
 *  A callback can be associated with the DownloadProtocol so progress
 *  can be monitored.
 *
 *  If the Globals.isOffline() is true, this class will go directly
 *  to the cache. Otherwise, it will first go to the Web, and then
 *  to the cache.
 */
public class DownloadProtocol {
    
    /** Callback interface that can be used to track the downloading progress */
    public interface DownloadDelegate {
        public void downloading(URL rc, String version, int readSoFar, int estimatedSize, boolean willPatch);
        public void validating(URL rc, int entry, int total);
        public void patching(URL rc, String version, int percentDone);
        public void downloadFailed(URL rc, String version);
    }
    
    
    public static final int JAR_DOWNLOAD      = 0;         // JAR file containing Java resources (classes, etc.)
    public static final int NATIVE_DOWNLOAD   = 1;         // JAR file containing native resources
    public static final int IMAGE_DOWNLOAD    = 2;         // Image
    public static final int APPLICATION_JNLP_DOWNLOAD = 3; // Application-desc JNLP file
    public static final int EXTENSION_JNLP_DOWNLOAD   = 4; // Extension=-desc JNLP file
    
    // MIME types
    private static final String JNLP_MIME_TYPE     = "application/x-java-jnlp-file";
    private static final String ERROR_MIME_TYPE    = "application/x-java-jnlp-error";
    private static final String JAR_MIME_TYPE      = "application/x-java-archive";
    private static final String JARDIFF_MIME_TYPE  = "application/x-java-archive-diff";
    private static final String GIF_MIME_TYPE      = "image/gif";
    private static final String JPEG_MIME_TYPE     = "image/jpeg";
    
    
    // Arguments for the Extension Download Protocol
    private static final String ARG_ARCH                = "arch";
    private static final String ARG_OS                  = "os";
    private static final String ARG_LOCALE              = "locale";
    private static final String ARG_VERSION_ID          = "version-id";
    private static final String ARG_CURRENT_VERSION_ID  = "current-version-id";
    private static final String ARG_PLATFORM_VERSION_ID = "platform-version-id";
    private static final String ARG_KNOWN_PLATFORMS     = "known-platforms";
    
    // Reply
    private static final String REPLY_JNLP_VERSION         = "x-java-jnlp-version-id";
    
    //
    // Basic abstract on how to download a specific resource, such as a JNLP file,
    // ExtensionDescriptor, JAR file, nativelib, and images
    //
    static class DownloadInfo {
        private URL _location;
        private String  _version;
        private int     _kind;
        private boolean _isCacheOk;
        // For Extension Downloads
        private String  _knownPlatforms = null;
        private boolean _isPlatformVersion = false;
        
        public DownloadInfo(URL location, String version, int kind, boolean isCacheOk) {
	    _location = location;
	    _version = version;
	    _kind = kind;
	    _isCacheOk = isCacheOk;
        }
        
        public DownloadInfo(URL location, String version, boolean isCacheOk,
			    String knownPlatforms, boolean isPlatformVersion) {
	    _location = location;
	    _version = version;
	    _kind = EXTENSION_JNLP_DOWNLOAD;
	    _isCacheOk = isCacheOk;
	    _knownPlatforms = knownPlatforms;
	    _isPlatformVersion = isPlatformVersion;
        }
        
        URL     getLocation() { return _location; }
        String  getVersion()  { return _version; }
        int     getKind()     { return _kind; }
        
        char getEntryType() {
	    switch(_kind) {
		case JAR_DOWNLOAD:              return Cache.RESOURCE_TYPE;
		case IMAGE_DOWNLOAD:            return Cache.RESOURCE_TYPE;
		case NATIVE_DOWNLOAD:           return Cache.RESOURCE_TYPE;
		case APPLICATION_JNLP_DOWNLOAD: return Cache.APPLICATION_TYPE;
		case EXTENSION_JNLP_DOWNLOAD:   return Cache.EXTENSION_TYPE;
	    }
	    return 'a';
        }
        
        /** Check if the cached element will do. The cacheDce is never null */
        boolean isCacheOk(DiskCacheEntry cachedDce, boolean exactMatch) {
	    return exactMatch && (_version != null || _isCacheOk) && cachedDce.getTimeStamp() != 0;
        }
        
        /** Returns the requst URL based on the current cache entry */
        URL getRequestURL(DiskCacheEntry cachedDce) {
	    StringBuffer args = new StringBuffer();
	    
	    // Add version information
	    if (_version != null && _kind != EXTENSION_JNLP_DOWNLOAD) {
		addURLArgument(args, ARG_VERSION_ID, _version);
		
		// Add incremental download for JAR files
		if ((_kind == JAR_DOWNLOAD || _kind == NATIVE_DOWNLOAD) &&
		    cachedDce != null && cachedDce.getVersionId() != null) {
		    addURLArgument(args, ARG_CURRENT_VERSION_ID, cachedDce.getVersionId());
		}
	    }
	    
	    if (_kind == EXTENSION_JNLP_DOWNLOAD && _version != null) {
		if (_isPlatformVersion) {
		    addURLArgument(args, ARG_PLATFORM_VERSION_ID, _version);
		} else {
		    addURLArgument(args, ARG_VERSION_ID, _version);
		}
		
		addURLArgument(args, ARG_ARCH, Config.getOSArch());
		addURLArgument(args, ARG_OS, Config.getOSName());
		addURLArgument(args, ARG_LOCALE, Globals.getDefaultLocaleString());
		if (_knownPlatforms != null) {
		    addURLArgument(args, ARG_KNOWN_PLATFORMS, _knownPlatforms);
		}
	    }
	    
	    // Remove last '&', and insert '?' for non-empty strings
	    if (args.length() > 0) args.setLength(args.length()-1);
	    if (args.length() > 0) args.insert(0, '?');
	    
	    try {
		if (Globals.getCodebaseOverride() != null && Globals.getCodebase() != null) {	
		    return new URL(Globals.getCodebaseOverride() +   _location.getFile().substring(Globals.getCodebase().getFile().length()) + args);
		}
		return new URL(_location.getProtocol(),
			       _location.getHost(),
			       _location.getPort(),
			       _location.getFile() + args);
	    } catch(MalformedURLException mue) {	
		Trace.ignoredException(mue);
		return null;
		
	    }
        }
        
        private void addURLArgument(StringBuffer sb, String key, String value) {
	    sb.append(URLEncoder.encode(key));   sb.append('=');
	    sb.append(URLEncoder.encode(value)); sb.append('&');
        }
        
        /** Is the x-jnlp-version-id header field needed */
        boolean needsReplyVersion(DiskCacheEntry dce) {
	    return (_version != null);
        }
        
        /** Is it a product request or platform request. If it is a platform request, then
	 *  the product version reply does not need to match
	 */
        boolean isPlatformRequest() {
	    return _isPlatformVersion;
        }
        
        /** Returns the valid mime-types for the request, based on what is current in the cache */
        boolean isValidMimeType(String mimetype, DiskCacheEntry cacheDce) {
	    if (mimetype == null) return false;
	    
	    if (_kind == JAR_DOWNLOAD || _kind == NATIVE_DOWNLOAD) {
		// No need to be pendantic. Most Web servers returns a more or less random
		// MIME type for JAR files (e.g., text/html or application/octet-stream)
		// Just make sure that if JARDIFF_MIME_TYPE is returned, then we did an
		// incremental update request.
		if (mimetype.equalsIgnoreCase(JARDIFF_MIME_TYPE)) {
		    return (cacheDce != null && cacheDce.getVersionId() != null);
		}
		return true;
	    } else if (_kind == IMAGE_DOWNLOAD) {
		return mimetype.equalsIgnoreCase(JPEG_MIME_TYPE) ||
		    mimetype.equalsIgnoreCase(GIF_MIME_TYPE);
	    } else {
		// return mimetype.equalsIgnoreCase(JNLP_MIME_TYPE);
		// no reason to complain about servers or proxys that don't
		// pass us the correct mime type ...
		return true;
	    }
        }
        
        /** Returns true if the Web version is newer. dce is guaranteed to be not-null*/
        boolean isWebNewer(DiskCacheEntry dce, long downloadSize, long lastModified, String downloadVersion) {
	    if (_version == null) {
		return ((lastModified == 0 && downloadSize > 0) || 
			(lastModified > dce.getTimeStamp()));
	    } else {
		// Version ID was used. Web is always never if we got here. Otherwise, we
		// would already have returned the cached entry.
		return true;
	    }
        }
    }
    
    //
    // Basic abstract of a download request. Specific implementation of UpdateAvailable,
    // isInCache, etc.
    //
    private static interface DownloadAction {
        /** Called if the cached copy was ok */
        public void actionInCache(DiskCacheEntry dce) throws IOException, JNLPException;
        /** Called if the cached copy was not ok, but we cannot go online */
        public void actionOffline(DiskCacheEntry dce, boolean exactMatch) throws IOException, JNLPException;
        
        /** Called to check if connection should be attempted */
        public boolean skipDownloadStep();
        
        /** Called if download is needed */
        public void actionDownload(DiskCacheEntry dce, DownloadInfo di, long lastModified, int length,
				   String responseVersion, String mimeType, HttpResponse hr) throws IOException, JNLPException;
	
	/** Called to see if we should do a HEAD request */
	public boolean useHeadRequest();
    }
    
    //
    // Implementation of UpdateAvailable Action
    //
    private static class UpdateAvailableAction implements DownloadAction {
        private boolean _result = false;
        
        /** Return result */
        public boolean getResult() { return _result; }
        
        /** Called if the cached copy was ok */
        public void actionInCache(DiskCacheEntry dce) throws IOException, JNLPException {
	    _result = false;
        }
        
        /** Called if the cached copy was not ok, but we cannot go online */
        public void actionOffline(DiskCacheEntry dce, boolean exactMatch) throws IOException, JNLPException {
	    _result = false;
        }
        
        /** Called to check if connection should be attempted */
        public boolean skipDownloadStep() { return false; }
        
        /** Called if download is needed */
        public void actionDownload(DiskCacheEntry dce, DownloadInfo di, long lastModified, int length,
				   String responseVersion, String mimeType, HttpResponse hr) throws IOException, JNLPException {
	    _result = true;
        }
	
	public boolean useHeadRequest() {
	    return true;
	}
    }
    
    //
    // Implementation of GetCachedEntry Action
    //
    private static class IsInCacheAction implements DownloadAction {
        private DiskCacheEntry _dce = null;
        
        /** Return result */
        public DiskCacheEntry getResult() { return _dce; }
        
        /** Called if the cached copy was ok */
        public void actionInCache(DiskCacheEntry dce) throws IOException, JNLPException {
	    _dce = dce;
        }
        
        /** Called if the cached copy was not ok, but we cannot go online */
        public void actionOffline(DiskCacheEntry dce, boolean exactMatch) throws IOException, JNLPException {
	    _dce = (exactMatch) ? dce : null;
        }
        
        /** Called to check if connection should be attempted */
        public boolean skipDownloadStep() { return true; }
        
        /** Called if download is needed */
        public void actionDownload(DiskCacheEntry dce, DownloadInfo di, long lastModified, int length,
				   String responseVersion, String mimeType, HttpResponse hr) throws IOException, JNLPException {
        }
	
	public boolean useHeadRequest() {
	    return false;
	}
    }
    
    //
    // Implementation of DownloadSize Action
    // Returns: 0  : Already ready in cache
    //          -1  : Unknown or offline
    // size : No. of bytes
    //
    private static class DownloadSizeAction implements DownloadAction {
        private long _result = -1;
        
        /** Return result */
        public long getResult() { return _result; }
        
        /** Called if the cached copy was ok */
        public void actionInCache(DiskCacheEntry dce) throws IOException, JNLPException {
	    _result = 0;
        }
        
        /** Called if the cached copy was not ok, but we cannot go online */
        public void actionOffline(DiskCacheEntry dce, boolean exactMatch) throws IOException, JNLPException {
	    _result = (exactMatch) ? 0 : -1;
        }
        
        /** Called to check if connection should be attempted */
        public boolean skipDownloadStep() { return false; }
        
        /** Called if download is needed */
        public void actionDownload(DiskCacheEntry dce, DownloadInfo di, long lastModified, int length,
				   String responseVersion, String mimeType, HttpResponse hr) throws IOException, JNLPException {
	    _result = length;
        }
	
	public boolean useHeadRequest() {
	    return true;
	}
    }
    
    //
    // Implementation of Retrive Action
    //
    private static class RetrieveAction implements DownloadAction {
        private DiskCacheEntry _result = null;
        private DownloadDelegate _delegate = null;
        /** Return result */
        
        public DiskCacheEntry getResult() { return _result; }
        
        public RetrieveAction(DownloadDelegate delegate) {
	    _delegate = delegate;
        }
        
        /** Called if the cached copy was ok */
        public void actionInCache(DiskCacheEntry dce) throws IOException, JNLPException {
	    _result = dce;
        }
        
        /** Called if the cached copy was not ok, but we cannot go online */
        public void actionOffline(DiskCacheEntry dce, boolean exactMatch) throws IOException, JNLPException {
	    _result = (exactMatch) ? dce : null;
        }
        
        /** Called to check if connection should be attempted */
        public boolean skipDownloadStep() { return false; }
        
        /** Called if download is needed */
        public void actionDownload(DiskCacheEntry dce, DownloadInfo di, long lastModified, int length,
				   final String responseVersion, String mimeType, HttpResponse hr) throws IOException, JNLPException {
	    final URL location = di.getLocation();
	    // The mime type has already been checked at this point
	    final boolean willPatch = mimeType.equalsIgnoreCase(JARDIFF_MIME_TYPE);
	    String requestVersion = di.getVersion();
	    String storeVersion = (requestVersion != null) ? responseVersion : null;
	    
	    
	    
	    Trace.println("Doing download", TraceLevel.NETWORK);
	    
	    
	    /* Setup callback */
	    HttpDownloadListener hdl = (_delegate == null) ? null : new HttpDownloadListener() {
		public boolean downloadProgress(int read, int total) {
		    _delegate.downloading(location,
					  responseVersion,
					  read,
					  total,
					  willPatch);
		    return true;
		}
	    };
	    
	    File result = null;
	    try {
		// Get temp. entry in cache
		result = Cache.getTempCacheFile(location, storeVersion);
		JavawsFactory.getHttpDownloadImpl().download(hr, result, hdl);
	    } catch(IOException ioe) {
	
		Trace.println("Got exception while downloading resource: " + ioe, TraceLevel.NETWORK);
		
		// Notify delegate that loading failed
		if (_delegate != null) _delegate.downloadFailed(location, responseVersion);
		// Rethrow exception
		throw new FailedDownloadingResourceException(location, responseVersion, ioe);
	    } catch(com.sun.javaws.net.CanceledDownloadException cde) {
		// We never return false in the delegate
		Trace.ignoredException(cde);
	    }
	    
	    // Patch file, if necessary. The method will throw the right exceptions, if neccesary
	    if (willPatch) {
		result = applyPatch(dce.getFile(), result, location, responseVersion, _delegate);
	    }
	    
	    // Store file to disk
	    if (di.getKind() == APPLICATION_JNLP_DOWNLOAD || di.getKind() == EXTENSION_JNLP_DOWNLOAD ||
		di.getKind() == IMAGE_DOWNLOAD) {
		// Update disk cache
		Cache.insertEntry(di.getEntryType(), location, 
				       storeVersion, result, lastModified);
		result = null;	// insertEntry will delete if appropriate
	    } else if (di.getKind() == JAR_DOWNLOAD || 
		       di.getKind() == NATIVE_DOWNLOAD) {
		//
		// Downloaded JAR file. We need to scan it for a certificate
		//
		File extractDir = (di.getKind() == NATIVE_DOWNLOAD) ?
		    Cache.createNativeLibDir(location, storeVersion) :
		    null;
		
		JarFile jarfile = new JarFile(result);
		try {
		    // Check signing status of jar. This might take a while, 
		    // so show user what is going on.
		    SigningInfo.checkSigning(location, storeVersion, 
					     jarfile, _delegate, extractDir);
		    jarfile.close(); jarfile = null;
		    Cache.insertEntry(di.getEntryType(), location, 
					   storeVersion, result, lastModified);
		    result = null;  // insertEntry will delete if appropriate
		} finally {
		    if (jarfile != null) jarfile.close();
		    if (result != null) result.delete();
		}
	    }
	    
	    // Lookup the file entry in the cache
	    _result = Cache.getCacheEntry(di.getEntryType(), 
						   location, storeVersion);
        }
	
	public boolean useHeadRequest() {
	    return false;
	}
    }
    
    static private void doDownload(DownloadInfo di, DownloadAction da) throws JNLPException {
        // Look element up in cache
        try {
	    boolean exactArray[] = new boolean[1];
	    DiskCacheEntry dce = findBestDiskCacheEntry(di.getEntryType(),
							di.getLocation(),
							di.getVersion(),
							exactArray);
	    boolean exactMatch = exactArray[0];
	    
	    // Check if we found a match in the cache
	    if (dce != null && di.isCacheOk(dce, exactMatch)) {
	
		Trace.println("Found in cache: " + dce, TraceLevel.NETWORK);
		
		da.actionInCache(dce);
		return;
	    }
	    
	    // Connect to Web Site
	    // Offline? either return cache entry or null
	    if (Globals.isOffline()) {
	
		Trace.println("Offline mode. No Web check. Cache lookup: " + dce, TraceLevel.NETWORK);
		
		da.actionOffline(dce, exactMatch);
		return;
	    }
	    
	    // Certain actions might not want to go online, like the isInCache
	    if (da.skipDownloadStep()) {
	
		Trace.println("Skipping download step", TraceLevel.NETWORK);
		
		return;
	    }
	    
	    // Build request URL
	    URL requestURL = di.getRequestURL(dce);
	   
	    Trace.println("Connection to: " + requestURL, TraceLevel.NETWORK);
	    
	    
	    // Do a HTTP request
	    HttpRequest httpreq = JavawsFactory.getHttpRequestImpl();
	    
	    HttpResponse response = null;
	    // We try download in the default mode, and retry
	    // w/o Http or Pack200 compression.
	    try {
		response = da.useHeadRequest() ?
		    httpreq.doHeadRequest(requestURL) :
		    httpreq.doGetRequest(requestURL);
	    } catch (IOException ioe) { 
		response = da.useHeadRequest() ?
		    httpreq.doHeadRequest(requestURL, false) :
		    httpreq.doGetRequest(requestURL, false);
	    }
	    
	    // Check for an HTTP 404
	    if (response.getStatusCode() == 404) {
		throw new FailedDownloadingResourceException(di.getLocation(),
							     di.getVersion(),
							     new IOException("HTTP response 404"));
	    }
	    
	    int length  = (int)response.getContentLength();
	    long lastModified = response.getLastModified();
	    String downloadVersion = response.getResponseHeader(REPLY_JNLP_VERSION);
	    String mimeType = response.getContentType();
	    
	    
	    Trace.println("Sever response: (length: " + length + ", lastModified: " +
			  new Date(lastModified) + ", downloadVersion " + downloadVersion +
			  ", mimeType: " + mimeType + ")", TraceLevel.NETWORK);
	    
	    
	    /** Check for error condition */
	    if (mimeType != null && mimeType.equalsIgnoreCase(ERROR_MIME_TYPE)) {
		BufferedInputStream is = response.getInputStream();
		// Error got returned
		BufferedReader br = new BufferedReader(new InputStreamReader(is));
		String errline = br.readLine();
		throw new ErrorCodeResponseException(di.getLocation(), di.getVersion(), errline);
	    }
	    
	    // Check that the mime-type is understood.
	    if (!di.isValidMimeType(mimeType, dce)) {
		throw new BadMimeTypeResponseException(di.getLocation(), di.getVersion(), mimeType);
	    }
	    
	    // Check that the x-java-jnlp-version-id attribute is present
	    
	    if (di.needsReplyVersion(dce)) {
		if  (downloadVersion == null) {
		    throw new MissingVersionResponseException(di.getLocation(),
							      di.getVersion());
		}
		
		// Check that the product version matches. This is needed
		// in all cases, except when we did a platform-version request
		if (!di.isPlatformRequest()) {
		    if (!(new VersionString(
			      di.getVersion()).contains(downloadVersion))) {
		        throw new BadVersionResponseException(di.getLocation(),
							      di.getVersion(), downloadVersion);
		    }
		    VersionID downloadVersionId =
			new VersionID(downloadVersion);
		    if (!downloadVersionId.isSimpleVersion()) {
		        throw new BadVersionResponseException(di.getLocation(),
							      di.getVersion(), downloadVersion);
		    }
		} else {
		}
	    }
	    
	    // Check if Web resource is newer than cached resource
	    if (dce != null && !di.isWebNewer(dce, length, lastModified, downloadVersion)) {
		da.actionInCache(dce);
		response.disconnect();
		return;
	    }
	    
	    da.actionDownload(dce, di, lastModified, (int)length, downloadVersion, mimeType, response);
	    response.disconnect();
        } catch(ZipException ze) {
	    throw new BadJARFileException(di.getLocation(), di.getVersion(), ze);
        } catch(JNLPException je) {
	    /** Make sure not to convert a JNLPexception */
	    throw je;
        } catch(Exception e) {
	    /** Wrap exception */
	    throw new FailedDownloadingResourceException(di.getLocation(), di.getVersion(), e);
        }
    }
    
    /**
     * Applies the delta at <code>deltaPath</code> to <code>dce</code>,
     * returning a File to the resulting file. <code>diffMimeType</code>
     * gives the mime type for the delta.
     */
    static private File applyPatch(File baseFile, File jardiffFile,
				   final URL location, final String newVersionId,
				   final DownloadDelegate delegate) throws JNLPException {
        
        // Get patcher
        Patcher differ = new JarDiffPatcher();
        
        File result = null;
        OutputStream out = null;
        boolean done = false;
        try {
	    result = Cache.getTempCacheFile(location, newVersionId);
	    out = new FileOutputStream(result);
	    Patcher.PatchDelegate pd = null;
	    
	    if (delegate != null) {
		delegate.patching(location, newVersionId, 0);
		pd = new Patcher.PatchDelegate() {
		    public void patching(int percentDone) {
			delegate.patching(location, newVersionId, percentDone);
		    }
		};
	    }
	    try {
		differ.applyPatch(pd, baseFile.getPath(), jardiffFile.getPath(), out);
	    } catch(IOException ioe) {
		throw new InvalidJarDiffException(location, newVersionId, ioe);
	    }
	    done = true;
        } catch(IOException ioe) {
	    
	    Trace.println("Got exception while patching: " + ioe, TraceLevel.NETWORK);
	    
	    throw new FailedDownloadingResourceException(location, newVersionId, ioe);
        } finally {
	    try {
		if (out != null) out.close();
	    } catch(IOException ioe) { Trace.ignoredException(ioe); }
	    if (!done) result.delete();
	    jardiffFile.delete();
	    if (delegate != null && !done) delegate.downloadFailed(location, newVersionId);
        }
        return result;
    }
    
    /** Downloads a given JRE */
    static public DiskCacheEntry getJRE(URL url, String version, boolean isPlatformVersion, String knownPlatforms) throws JNLPException {
        // Extension Download constructor
        DownloadInfo di = new DownloadInfo(url, version, false, knownPlatforms, isPlatformVersion);
        RetrieveAction ra = new RetrieveAction(null);
        doDownload(di, ra);
        DiskCacheEntry dce = ra.getResult();
        if (dce == null) {
	    throw new FailedDownloadingResourceException(url, version, null);
        }
        return dce;
    }
    
    
    /** Returns the given extension descriptor */
    static public DiskCacheEntry getLaunchFile(URL url, boolean isCacheOk) throws JNLPException {
        // Extension Download constructor
        DownloadInfo di = new DownloadInfo(url, null, DownloadProtocol.APPLICATION_JNLP_DOWNLOAD, false);
        RetrieveAction ra = new RetrieveAction(null);
        doDownload(di, ra);
        DiskCacheEntry dce = ra.getResult();
        if (dce == null) {
	    throw new FailedDownloadingResourceException(url, null, null);
        }
        return dce;
    }
    
    
    /** Returns the given extension if it is cached */
    static public DiskCacheEntry getCachedLaunchedFile(URL url) throws JNLPException {
        // Extension Download constructor
        DownloadInfo di = new DownloadInfo(url, null, DownloadProtocol.APPLICATION_JNLP_DOWNLOAD, true);
        IsInCacheAction ra = new IsInCacheAction();
        doDownload(di, ra);
        DiskCacheEntry dce = ra.getResult();
        return dce;
    }
    
    /** Returns true if an update is available of the Application Descriptor (JNLP file) at the
     *  given URL
     */
    static public boolean isLaunchFileUpdateAvailable(URL url) throws JNLPException {
        // If we are offline, no update is available
        if (Globals.isOffline()) return false;
        // Check Web server for update
        DownloadInfo di = new DownloadInfo(url, null, DownloadProtocol.APPLICATION_JNLP_DOWNLOAD, false);
        UpdateAvailableAction uaa = new UpdateAvailableAction();
        doDownload(di, uaa);
        
        return uaa.getResult();
    }
    
    
    /** Returns the given extension descriptor */
    static public DiskCacheEntry getExtension(URL url, String version, String knownPlatforms, boolean isCacheOk) throws JNLPException {
        // Extension Download constructor
        DownloadInfo di = new DownloadInfo(url, version, isCacheOk, knownPlatforms, false);
        RetrieveAction ra = new RetrieveAction(null);
        doDownload(di, ra);
        DiskCacheEntry dce = ra.getResult();
        if (dce == null) {
	    throw new FailedDownloadingResourceException(url, version, null);
        }
        return dce;
    }
    
    /** Returns the given extension if it is cached */
    static public DiskCacheEntry getCachedExtension(URL url, String version, String knownPlatforms) throws JNLPException {
        // Extension Download constructor
        DownloadInfo di = new DownloadInfo(url, version, true, knownPlatforms, false);
        IsInCacheAction ra = new IsInCacheAction();
        doDownload(di, ra);
        DiskCacheEntry dce = ra.getResult();
        return dce;
    }
    
    /** Returns true if an update is available of the given Extension Descriptor */
    static public boolean isExtensionUpdateAvailable(URL url, String version, String knownPlatforms) throws JNLPException {
        // If we are offline, no update is available
        if (Globals.isOffline()) return false;
        DownloadInfo di = new DownloadInfo(url, version, false, knownPlatforms, false);
        UpdateAvailableAction uaa = new UpdateAvailableAction();
        doDownload(di, uaa);
        return uaa.getResult();
    }
    
    /** Returns true if the given resource is cached. */
    static public DiskCacheEntry getResource(URL url, String version, int type, boolean isCacheOk, DownloadDelegate dd) throws JNLPException {
        DownloadInfo di = new DownloadInfo(url, version, type, isCacheOk);
        RetrieveAction ra = new RetrieveAction(dd);
        doDownload(di, ra);
        DiskCacheEntry dce = ra.getResult();
        if (dce == null) {
	    throw new FailedDownloadingResourceException(url, version, null);
        }
        return dce;
    }
    
    /** Returns true if the given resource is cached. */
    static public boolean isInCache(URL url, String version, int type) {
        return getCachedVersion(url, version, type) != null;
    }
    
    /** Returns true if the given resource is cached. */
    static public long getCachedSize(URL url, String version, int type) {
        DiskCacheEntry dce = getCachedVersion(url, version, type);
        return (dce != null) ? dce.getSize() : 0;
    }
    
    /** Returns the cached resource, if it exists. */
    static public DiskCacheEntry getCachedVersion(URL url, String version, int type)  {
        try {
	    DownloadInfo di = new DownloadInfo(url, version, type, true);
	    IsInCacheAction iica = new IsInCacheAction();
	    doDownload(di, iica);
	    // Make sure certificate info. is intack
	    DiskCacheEntry dce = iica.getResult();
	    return dce;
        } catch(JNLPException je) {
	    Trace.ignoredException(je);
	    return null;
        }
    }
    
    /** Returns true if an update is available for the given resource */
    static public boolean isUpdateAvailable(URL url, String version, int type) throws JNLPException {
        // If we are offline, no update is available
        if (Globals.isOffline()) return false;
        
        DownloadInfo di = new DownloadInfo(url, version, type, false);
        UpdateAvailableAction uaa = new UpdateAvailableAction();
        doDownload(di, uaa);
        return uaa.getResult();
    }
    
    /** Returns the size of bytes needed to be downloaded for this particular resource.
     *  Returns: 0  : Already ready in cache
     *          -1  : Unknown or offline
     *         size : No. of bytes
     */
    static public long getDownloadSize(URL url, String version, int type) throws JNLPException {
        DownloadInfo di = new DownloadInfo(url, version, type, false);
        DownloadSizeAction dsa = new DownloadSizeAction();
        doDownload(di, dsa);
        return dsa.getResult();
    }
    
    /** Returns the best DiskCache entry for a given url and version string. It might return
     *  an exact match. The exact should be a an one-elment boolean array. The first element
     *  will be set to true if an exact match was found, otherwise false
     */
    static private DiskCacheEntry findBestDiskCacheEntry(char diskCacheType, URL url, String version, boolean[] exact) throws IOException {
        DiskCacheEntry dces[] =
	    Cache.getCacheEntries(diskCacheType, url, version, false);
        
        // If we did not find anything
        if (dces.length == 0) {
	    exact[0] = false;
	    return null;
        }
        
        // Get best match
        DiskCacheEntry dce = dces[0];
        
        if (version == null) {
	    exact[0] = true;
        } else {
	    VersionString vs = new VersionString(version);
	    exact[0] = vs.contains(dce.getVersionId());
        }
        return dce;
    }
}


