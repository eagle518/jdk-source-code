/*
 * @(#)LaunchDownload.java	1.61 04/05/05
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws;

import java.io.File;
import java.io.IOException;
import java.io.DataInputStream;
import java.io.FileInputStream;
import java.net.URL;
import java.net.MalformedURLException;
import java.util.jar.JarFile;
import java.util.jar.JarEntry;
import java.util.jar.Manifest;
import java.util.Enumeration;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Arrays;
import java.security.cert.Certificate;
import java.security.Policy;
import java.security.CodeSource;
import com.sun.javaws.cache.DiskCacheEntry;
import com.sun.javaws.cache.Cache;
import com.sun.javaws.cache.DownloadProtocol;
import com.sun.javaws.util.*;
import com.sun.javaws.jnl.*;
import com.sun.javaws.security.*;
import com.sun.javaws.exceptions.*;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.config.Config;
import com.sun.deploy.config.JREInfo;
import com.sun.deploy.resources.ResourceManager;

/**
 *  The LaunchDownload methods contains methods
 *  to querery about the state of the resources that a
 *  particular LaunchDesc needs, and methods to download
 *  them.
 *
 *  The class uses the Globals.isOffline() flag to figure out
 *  if we are online or not.
 */
public class LaunchDownload {

    private static boolean updateAvailable = false;

    private static int numThread = 0;

    private static Object syncObj = new Object();

    
    /** Delegate object that can be used to monitor the progress of the downloading
     */
    public static interface DownloadProgress {
        // Downloading a JRE
        public void jreDownload(String version, URL location);
        
        // Downloading a JNLP file for an extension
        public void extensionDownload(String name, int remaining);
        
        // The percentage might not be readSoFar/total due to the validation pass
        // The total and percentage will be -1 for unknown
        public void progress  (URL url, String version, long readSoFar, long total, int overallPercent);
        public void validating(URL url, String version, long entry,     long total, int overallPercent);
        public void patching(URL url, String version, int patchPercent,
			     int overallPercent);
        public void downloadFailed(URL url, String version);
    }

    private static boolean compareByteArray(byte[] a, byte[] b) {
	if (a.length == b.length) {
	    for (int i = 0; i < a.length; i++) {
		if (a[i] != b[i]) {
		    return false;
		}
	    }
	    return true;
	}
	return false;
    }
    
    /** Tries to replace a LaunchDesc with a cached one if possible */
    static LaunchDesc getLaunchDescFromCache(LaunchDesc ld) {
        // If no href is given, put the current ld in as canonical;
	URL ref = ld.getLocation();
        if (ref == null) {
	    ref = ld.getCanonicalHome();
	    if (ref != null) try {
   	        Cache.putCanonicalLaunchDesc(ref, ld);
	    } catch (IOException ioe) {
		Trace.ignoredException(ioe);
	    }
	}

        // Look up the home in the cahce
        try {
	    DiskCacheEntry dce = DownloadProtocol.getCachedLaunchedFile(ref);
	    if (dce == null) return ld; // Not cached
	    
	    Trace.println("Loaded descriptor from cache at: " + ref, 
			  TraceLevel.BASIC);

	    File f = dce.getFile();
	    
            byte[] cachedContents = LaunchDescFactory.readBytes(new FileInputStream(f), f.length());

	    byte[] ldContents = ld.getBytes();

	    if (cachedContents != null && ldContents != null) {
		if (compareByteArray(cachedContents, ldContents)) {
		    return ld;
		}
	    }

	    return LaunchDescFactory.buildDescriptor(cachedContents);
        } catch(JNLPException jnlpe) {
	    // Fall through. Just go with the one we got
	    Trace.ignoredException(jnlpe);
        } catch(IOException ioe) {
	    // Fall through. Just go with the one we got
	    Trace.ignoredException(ioe);
        }
        // Default to the current one
        return ld;
    }
    
    /** Returns a LaunchDesc if the one on the Web is newer than the cached one,
     *  otherwise null
     */
    static LaunchDesc getUpdatedLaunchDesc(LaunchDesc ld) throws JNLPException, IOException {
        // If not home is given, the current one is fine
        if (ld.getLocation() == null) return null;
        // Newer on web?
        boolean update = DownloadProtocol.isLaunchFileUpdateAvailable(ld.getLocation());
        if (!update) return null;
        // Yes, download it and return it
       
	Trace.println("Downloading updated JNLP descriptor from: " + ld.getLocation(), TraceLevel.BASIC);
        
        DiskCacheEntry dce = DownloadProtocol.getLaunchFile(ld.getLocation(), false);
	try {
            return LaunchDescFactory.buildDescriptor(dce.getFile());
	} catch (JNLPException je) {
	    Cache.removeEntry(dce);
	    throw je;
	}
    }
    
    /** Checks all resources needed by the given LaunchDescriptor is cached.
     *  - this includes all library extensions are downloaded,
     *  - all installer extensions are installed
     *  - all eager JAR resources are cached
     *
     *  If throwException is true, an LaunchFileException will be thrown instead of
     *  false returned
     */
    public static boolean isInCache(LaunchDesc ld) {
        ResourcesDesc rd = ld.getResources();
        if (rd == null) return true;
        
        try {
	    // Check if LaunchDesc is cached (if a home is included)
	    if (ld.getLocation() != null) {
		DiskCacheEntry dce = DownloadProtocol.getCachedLaunchedFile(ld.getLocation());
		if (dce == null) return false;
	    }
	    
	    // Get all extensions
	    boolean success = getCachedExtensions(ld);
	    // Not all extensions were cached, or installer extension wasn't
	    // successfully installed - return false
	    if (!success) return false;
	    
	    // Get all eager JAR resources
	    JARDesc[] jars = rd.getEagerOrAllJarDescs(false);
	    for(int i = 0; i < jars.length; i++) {
		int type = jars[i].isJavaFile() ? DownloadProtocol.JAR_DOWNLOAD : DownloadProtocol.NATIVE_DOWNLOAD;
		if (!DownloadProtocol.isInCache(jars[i].getLocation(),
						jars[i].getVersion(),
						type)) {
		    return false;
		}
	    }
        } catch(JNLPException e) {
	    Trace.ignoredException(e);
	    // Something went wrong - treat it as not cached
	    return false;
        } catch(IOException ioe) {
	    Trace.ignoredException(ioe);
	    // Something went wrong - treat it as not cached
	    return false;
        }
        return true;
    }

    private static void updateCheck(final URL url, final String version, final int type, final boolean lazy) {

	synchronized (syncObj) {
	    numThread++;
	}
	new Thread(new Runnable() {
	        public void run() {		  
		  
		    try {		

			boolean update = DownloadProtocol.isUpdateAvailable(url, version, type);	
			if (update && lazy) {
			    // remove this file from cache
			   
			    File cachedFile = DownloadProtocol.getCachedVersion(url,version,type).getFile();
			    if (cachedFile != null) {
				cachedFile.delete();
			    }
			}
	
			synchronized (syncObj) {
			    if (update && !updateAvailable) {
				updateAvailable = true;	
			    }
			}
		    } catch (JNLPException jnlpe) {
			Trace.ignoredException(jnlpe);
		    } finally {
			synchronized(syncObj) {
			    numThread--;				
			}
		    }
	        }
            }).start();	
    }
    
    /** Checks if updates are available for the application described by the given LaunchDesc.
     *
     *  This LaunchDesc will already have been setup with links to the extensions.
     */
    public static boolean isUpdateAvailable(LaunchDesc ld) throws JNLPException {              
        // Check for updated JNLP files if href is specified in jnlp element
        URL homeUrl = ld.getLocation();
        if (homeUrl != null) {	  
	    boolean update = DownloadProtocol.isLaunchFileUpdateAvailable(homeUrl);	    
	    if (update) return true;
        }
	
	ResourcesDesc rd = ld.getResources();
        if (rd == null) return false;

	// fix for 4528392
	// check all extension JNLP files
	ExtensionDesc[] ed_array = rd.getExtensionDescs();

	for (int i = 0; i < ed_array.length; i++) {
	    URL edUrl = ed_array[i].getLocation();	   
	    if (edUrl != null) {
	
		updateCheck(edUrl, ed_array[i].getVersion(), DownloadProtocol.EXTENSION_JNLP_DOWNLOAD, false);
	
	    }
	}
	        
        // Check all cached resources that this application depends on (not just
	// the eager onces). We need to figure out if any of the lazy JARs have been
	// updated too
        JARDesc[] jars = rd.getEagerOrAllJarDescs(true);
        for(int i = 0; i < jars.length; i++) {
	    URL location = jars[i].getLocation();	   
	    String version = jars[i].getVersion();
	    int type = jars[i].isJavaFile() ? DownloadProtocol.JAR_DOWNLOAD : DownloadProtocol.NATIVE_DOWNLOAD;	    
	    if (DownloadProtocol.isInCache(location, version, type)) {	
		
		updateCheck(location, version, type, jars[i].isLazyDownload());
		
	    }
	}    
	IconDesc [] icons = ld.getInformation().getIcons();
	if (icons != null) for (int i=0; i<icons.length; i++) {
	    URL location = icons[i].getLocation();
	    String version = icons[i].getVersion();
	    int type = DownloadProtocol.IMAGE_DOWNLOAD;
            if (DownloadProtocol.isInCache(location, version, type)) {
	
		updateCheck(location, version, type, false);
	
            }
        }
	
	while (numThread > 0 ) {
	    synchronized (syncObj) {
		if (updateAvailable) break;
	    }
	}

	return updateAvailable;
    }
    
    private static class DownloadCallbackHelper implements DownloadProtocol.DownloadDelegate {
	DownloadProgress _downloadProgress; // Callback method
	long _totalSize;
	long _downloadedSoFar;
	long _currentTotal;
	
	public DownloadCallbackHelper(DownloadProgress dp, long totalSize) {
	    _downloadProgress = dp;
	    _totalSize = totalSize;
	    _downloadedSoFar = 0;
	}
	
	public void downloading(URL url, String version, int readSoFar, int total, boolean willPatch) {
	    int overallPercent = -1;
	    if (_totalSize != -1) {
		// Downloading gets 90% of an individual JAR file if patching
		// isn't required, otherwise 80%. The validating step
		// gets 10%, and patching 10%
		double multiplier = (willPatch) ? .8 : .9;
		double byteSize = ( _downloadedSoFar + multiplier *
				       (double)readSoFar);
		overallPercent = getPercent(byteSize);
		_currentTotal = total;
	    }
	    if (_downloadProgress != null) {
		_downloadProgress.progress(url, version, _downloadedSoFar + readSoFar, _totalSize, overallPercent);
	    }
	}
	
	public void patching(URL url, String version, int percentDone) {
	    int overallPercent = -1;
	    if (_totalSize != -1) {
		double byteSize = _downloadedSoFar + (double)_currentTotal *
		    (.8 + (double)percentDone / 1000);
		
		overallPercent = getPercent(byteSize);
	    }
	    if (_downloadProgress != null) {
		_downloadProgress.patching(url, version, percentDone,
					   overallPercent);
	    }
	}
	
	public void validating(URL url, int readSoFar, int total) {
	    int overallPercent = -1;
	    if (_totalSize != -1 && total != 0) {
		double byteSize = ( _downloadedSoFar + 0.9 * (double)_currentTotal) +
		    (0.1 * (double)_currentTotal * ((double)readSoFar / (double)total));
		overallPercent = getPercent(byteSize);
	    }
	    if (_downloadProgress != null) {
		_downloadProgress.validating(url, null, readSoFar, total, overallPercent);
	    }
	    
	    // Get ready for next file
	    if (readSoFar == total) {
		_downloadedSoFar += _currentTotal;
	    }
	}
	
	// A failed download is communicated directly
	public void downloadFailed(URL url, String version) {
	    if (_downloadProgress != null) {
		_downloadProgress.downloadFailed(url, version);
	    }
	}
	
	private int getPercent(double byteSize) {
	    // Check if we have downloaded more than estimated.
	    if (byteSize > _totalSize) {
		// Switch to unknown mode
		_totalSize = -1;
		return -1;
	    } else {
		double percent = ((byteSize * 100) / _totalSize);
		return (int)(percent + 0.5);
	    }
	}
    }
    
    /** Return a list of all the native directories used by this LaunchDescriptor */
    public static File[] getNativeDirectories(LaunchDesc ld) {
	ResourcesDesc rd = ld.getResources();
	if (rd == null) return new File[0];
	// Get list of all resources
	JARDesc[] jars = rd.getEagerOrAllJarDescs(true);
	ArrayList list = new ArrayList();
	for(int i = 0; i < jars.length; i++) {
	    if (jars[i].isNativeLib()) {
		URL location = jars[i].getLocation();
		String version = jars[i].getVersion();
		DiskCacheEntry dce = DownloadProtocol.getCachedVersion(location,
								       version,
								       DownloadProtocol.NATIVE_DOWNLOAD);
		if (dce != null) {
		    list.add(dce.getDirectory());
		}
	    }
	}
	File[] dirs = new File[list.size()];
	return (File[])list.toArray(dirs);
    }
    
    /** Given a LaunchDesc, it downloads all JNLP files for all extensions recursivly
     *  and add the resources to the LaunchDesc
     */
    static void downloadExtensions(LaunchDesc ld, DownloadProgress dp, int remaining, ArrayList installFiles)
	throws IOException, JNLPException {
	// Get extensions and download if neccesary
	downloadExtensionsHelper(ld, dp, remaining, false, installFiles);
    }
    
    /** Given a LaunchDesc, it gets all JNLP files for all extensions recursivly
     *  from the cache. Returns false if not all extensions could be found 
     *  in the cache, or installer extension wasn't successfully installed.
     */
    private static boolean getCachedExtensions(LaunchDesc ld)
	throws IOException, JNLPException {
	// Get extensions from cache
	return downloadExtensionsHelper(ld, null, 0, true, null);
    }
    
    
    private static boolean downloadExtensionsHelper(LaunchDesc ld, DownloadProgress dp,
						   int remaining, boolean cacheOnly, ArrayList installFiles)
	throws IOException, JNLPException {
	ResourcesDesc rd = ld.getResources();
	if (rd == null) return true;
	
	String knownPlatforms = JREInfo.getKnownPlatforms();
	
	// Get list of extensions
	final ArrayList list = new ArrayList();
	
	rd.visit(new ResourceVisitor() {
		    public void visitJARDesc(JARDesc jad) { /* ignored */ }
		    public void visitPropertyDesc(PropertyDesc prd) { /* ignored */ }
		    public void visitPackageDesc(PackageDesc pad) { /* ignored */ }
		    public void visitJREDesc(JREDesc jrd) { /* ignore */}
		    public void visitExtensionDesc(ExtensionDesc ed) {
			list.add(ed);
		    }
		});
	
	remaining += list.size();
	for(int i = 0; i < list.size(); i++) {
	    ExtensionDesc ed = (ExtensionDesc)list.get(i);
	    // Get name of extension
	    String name = ed.getName();
	    if (name == null) {
		// Create default name based on URL
		name = ed.getLocation().toString();
		int idx = name.lastIndexOf('/');
		if (idx > 0) name = name.substring(idx + 1, name.length());
	    }
	    
	    // Notify delgate
	    --remaining;
	    if (dp != null) dp.extensionDownload(name, remaining);
	    
	    // Download extension
	    DiskCacheEntry dce = null;
	    if (!cacheOnly) {
		dce = DownloadProtocol.getExtension(
		    ed.getLocation(),
		    ed.getVersion(),
		    knownPlatforms,
		    false);
	    } else {
		dce = DownloadProtocol.getCachedExtension(
		    ed.getLocation(),
		    ed.getVersion(),
		    knownPlatforms);
		
		// If not found, bail out
		if (dce == null) return false;
	    }
	    

	    Trace.println("Downloaded extension: " + ed.getLocation() + ": " + dce.getFile(), TraceLevel.NETWORK);
	    
	    
	    // Parse ExtensionDesc
	    LaunchDesc extensionLd = LaunchDescFactory.buildDescriptor(dce.getFile());
	    boolean downloadExtension = false;
	    if (extensionLd.getLaunchType() == LaunchDesc.LIBRARY_DESC_TYPE) {
		// Resources becomes part of application
		downloadExtension = true;
	    } else if (extensionLd.getLaunchType() == LaunchDesc.INSTALLER_DESC_TYPE) {
		ed.setInstaller(true);
		// If the extension is not installed already, we need to download the resources
		LocalApplicationProperties lap =
		    Cache.getLocalApplicationProperties(
			dce.getLocation(), dce.getVersionId(), ld, false);
		// This is a bit messsy, since LocallyInstalled means desktop integration for applications, and
		// really installed for extension installeds.
		downloadExtension = !lap.isLocallyInstalled();
	       		
		// It is an installer extension - store location of downloaded JNLP file in list
		// fix for 4473369, 4525544
		// only re-execute installer if new installer
		// is available on the net and gets downloaded
		// fix for 4708396
		// we should re-execute installer too if
		// it had not been run at all (downloadExtesnion == true)
		if (installFiles != null &&  (isUpdateAvailable(extensionLd) || downloadExtension)) {
		    installFiles.add(dce.getFile());
		} 

		if (cacheOnly && downloadExtension) {
		    // If just checking if installer is in cache, not being
                    // installed counts as not being in cache. (ie: reload)
		    return false;
		}
		
	    } else {
		throw new MissingFieldException(extensionLd.getSource(), "<component-desc>|<installer-desc>");
	    }
	    
	    if (downloadExtension) {
		ed.setExtensionDesc(extensionLd);
		// Do recursion.
		boolean res = downloadExtensionsHelper(extensionLd, dp, remaining, cacheOnly, installFiles);
		if (!res) return false; // Bail-out if failed (cachedOnly)
	    }
	}
	return true;
    }
    
    /** Download the selected JRE descriptor */
    static public void downloadJRE(LaunchDesc ld, DownloadProgress dp, ArrayList installFiles)
	throws JNLPException, IOException {
	JREDesc jd = ld.getResources().getSelectedJRE();
	String version = jd.getVersion();
	URL location = jd.getHref();
	
	// Platform or product download?
	boolean isPlatformVersion = (location == null);
	if (location == null) {
	    String url = Config.getProperty(Config.JAVAWS_JRE_INSTALL_KEY);
	    if (url != null) {
		try {
		    location = new URL(url);
		} catch (MalformedURLException e) {
		}
	    }
	}
	
	// Notify download window
	if (dp != null) dp.jreDownload(version, location);
	
	// Download Extension descriptor
	String knownPlatforms = JREInfo.getKnownPlatforms();
	DiskCacheEntry dce = null;
	try {
	    dce = DownloadProtocol.getJRE(location, version, isPlatformVersion, knownPlatforms);
	} catch(ErrorCodeResponseException ecre) {
	    // Provide a more sensible error message
	    ecre.setJreDownload(true);
	    throw ecre;
	}
	// dce != null, or exception is thrown
	
	// Parse ExtensionDesc for JRE installer
	LaunchDesc extensionLd = LaunchDescFactory.buildDescriptor(dce.getFile());
	if (extensionLd.getLaunchType() != LaunchDesc.INSTALLER_DESC_TYPE ) {
	    throw new MissingFieldException(extensionLd.getSource(), "<installer-desc>");
	}
	
	
	// It is an installer extension - store location of downloaded JNLP file in list
	if (installFiles != null) installFiles.add(dce.getFile());
	
	// Add LaunchDesc to ExtensionElement
	jd.setExtensionDesc(extensionLd);
	
	// Download recursivly all extensions needed by JRE installer
	downloadExtensionsHelper(extensionLd, dp, 0, false, installFiles);
    }
    
    /** Download all resources needed based on a single resource. A single
     *  resource can trigger several downloads. This method is used by both
     *  the DownloadService and the JNLP Classloader
     */
    static public void downloadResource(LaunchDesc ld, URL location, String version, DownloadProgress dp, boolean isCacheOk)
	throws IOException, JNLPException {
	ResourcesDesc resources = ld.getResources();
	if (resources == null) return;
	JARDesc[] jardescs = resources.getResource(location, version);
	downloadJarFiles(jardescs, dp, isCacheOk);
    }
    
    /** Download all resources needed based on a part name.
     */
    static public void downloadParts(LaunchDesc ld, String[] parts, DownloadProgress dp, boolean isCacheOk)
	throws IOException, JNLPException {
	ResourcesDesc resources = ld.getResources();
	if (resources == null) return;
	JARDesc[] jardescs = resources.getPartJars(parts);
	downloadJarFiles(jardescs, dp, isCacheOk);
    }
    
    
    /** Download all resources needed by a particular part of an specified
     *  extension
     */
    static public void downloadExtensionPart(LaunchDesc ld, URL location, String version, String parts[],
					     DownloadProgress dp, boolean isCacheOk)
	throws IOException, JNLPException {
	ResourcesDesc resources = ld.getResources();
	if (resources == null) return;
	JARDesc[] jardescs = resources.getExtensionPart(location, version, parts);
	downloadJarFiles(jardescs, dp, isCacheOk);
    }
    
    /** Download all eager or just plain all resources */
    static public void downloadEagerorAll(LaunchDesc ld, boolean downloadAll, DownloadProgress dp, boolean isCacheOk)
	throws IOException, JNLPException {
	ResourcesDesc resources = ld.getResources();
	if (resources == null) return;
	JARDesc[] jardescs = resources.getEagerOrAllJarDescs(downloadAll);
	// We will eagerly download all lazy resources that have already been cached too.
	// Otherwise, we might not detect that an update have happend
	if (!downloadAll) {	    
	    JARDesc[] allJarDescs = resources.getEagerOrAllJarDescs(true);
	    // Make sure there are some lazy ones
	    if (allJarDescs.length != jardescs.length) {
		HashSet hm = new HashSet(Arrays.asList(jardescs));		
		int found = 0;
		for(int i = 0; i < allJarDescs.length; i++) {
		    URL location = allJarDescs[i].getLocation();
		    String version = allJarDescs[i].getVersion();
		    int type = allJarDescs[i].isJavaFile() ? DownloadProtocol.JAR_DOWNLOAD : DownloadProtocol.NATIVE_DOWNLOAD;
		    if (!hm.contains(allJarDescs[i]) && DownloadProtocol.isInCache(location, version, type)) {
			found++;			    
		    } else {
			allJarDescs[i] = null; // Entry not cached
		    }
		}
		// Add to list
		if (found > 0) {
		    JARDesc[] newjars = new JARDesc[jardescs.length + found];   
		    System.arraycopy(jardescs, 0, newjars, 0, jardescs.length);
		    int idx = jardescs.length;
		    for(int i = 0; i < allJarDescs.length; i++) {
			if (allJarDescs[i] != null) newjars[idx++] = allJarDescs[i];
		    }
		    jardescs = newjars;
		}		
	    }
	}
	
	downloadJarFiles(jardescs, dp, isCacheOk);
    }
    
    /** Downloads all JAR resources specified in an array
     *
     *  The downloading happens in two steps:
     *   1. Determine size of resources that needs to be downloaded
     *   2. Download resources
     *
     *  A delegate object can be passed in that monitors the download progress
     *
     *  Returns: true if success, otherwise false
     */
    private static void downloadJarFiles(JARDesc[] jars, DownloadProgress dp, boolean isCacheOk)
	throws JNLPException, IOException {
	// Nothing to do?
	if (jars == null) return;
	
	
	Trace.println("Contacting server for JAR file sizes", TraceLevel.NETWORK);
	
	
	// Determine size of download
	long totalSize = 0;
	for(int i = 0; i < jars.length && totalSize != -1; i++) {
	    try {
		JARDesc jar = jars[i];
		
		// Returns: 0 : Already in cache
		//         -1 : Unknown download size
		//         >0 : Bytes needed to be downloaded
		int downloadType = (jar.isNativeLib()) ?
		    DownloadProtocol.NATIVE_DOWNLOAD : DownloadProtocol.JAR_DOWNLOAD;
		long size =jar.getSize(); // Get size from JNLP file
		if (size == 0) {
		    // If not specified, query Web server
		    size = DownloadProtocol.getDownloadSize(jars[i].getLocation(),
							    jars[i].getVersion(),
							    downloadType);
		}
	
		Trace.println("Size of " + jars[i].getLocation() + ": " + size, TraceLevel.NETWORK);
		
		if (size == -1) {
		    // Unknown download size
		    totalSize = -1;
		} else {
		    totalSize += size;
		}
	    } catch(JNLPException je) {
		if (dp != null) dp.downloadFailed(jars[i].getLocation(), jars[i].getVersion());
		throw je;
	    }
	    
	}
	

	Trace.println("Total size to download: " + totalSize, TraceLevel.NETWORK);
	
	
	// Need to download anything?
	if (totalSize == 0) return;
	
	// Download Progress object
	DownloadCallbackHelper dch = new DownloadCallbackHelper(dp, totalSize);
	
	// Download resources
	for(int i = 0; i < jars.length; i++) {
	    JARDesc jar = jars[i];
	    
	    try {
		// Returns: 0 : Already in cache
		//         -1 : Unknown download size
		//         >0 : Bytes needed to be downloaded
		
		int downloadType = (jar.isNativeLib()) ?
		    DownloadProtocol.NATIVE_DOWNLOAD : DownloadProtocol.JAR_DOWNLOAD;
		DiskCacheEntry dce = DownloadProtocol.getResource(jars[i].getLocation(),
								  jars[i].getVersion(),
								  downloadType,
								  isCacheOk,
								  dch);
	
		Trace.println("Downloaded " + jars[i].getLocation() + ": " + dce, TraceLevel.NETWORK);
		
		
		// The download progress monitor has already been notified
		if (dce == null) {
		    throw new FailedDownloadingResourceException(null,
								 jars[i].getLocation(),
								 jars[i].getVersion(),
								 null);
		}
	    }  catch(JNLPException je) {
		if (dp != null) dp.downloadFailed(jar.getLocation(), jar.getVersion());
		throw je;
	    }
	}
    }
    
    /** Check the URL security requirements. This recursivly check all LaunchDescriptors.
     *
     *  The requirements are:
     *        - For a LaunchDesc with sandbox security:
     *            - All JAR resources must come from the same host
     *            - No nativelibs
     *            - A sandboxed extension must be downloaded from the same host and its
     *              resources must be downloaded from the same host
     *
     **/
    static void checkJNLPSecurity(LaunchDesc ld) throws MultipleHostsException, NativeLibViolationException {
	final boolean[] nativeLibViolation = new boolean[1];
	final boolean[] hostViolation = new boolean[1];
	ResourcesDesc rd = ld.getResources();
	if (rd == null) return;
	JARDesc mainJar = ld.getResources().getMainJar(true);
	if (mainJar == null) return;
	checkJNLPSecurityHelper(ld, mainJar.getLocation().getHost(), hostViolation, nativeLibViolation);
	if (hostViolation[0]) throw new MultipleHostsException();
	if (nativeLibViolation[0]) throw new NativeLibViolationException();
    }
    
    static private void checkJNLPSecurityHelper(LaunchDesc ld,
						final String host,
						final boolean[] hostViolation,
						final boolean[] nativeLibViolation) {
	// If the application, needs unrestricted access we are done
	if (ld.getSecurityModel() != LaunchDesc.SANDBOX_SECURITY) return;
	
	ResourcesDesc rd = ld.getResources();
	if (rd == null) return;
	
	// All URL's must point to same host as the main JAR
	rd.visit(new ResourceVisitor() {
		    
		    public void visitJARDesc(JARDesc jad) {
			String thisHost = jad.getLocation().getHost();
			hostViolation[0] = hostViolation[0] || (!host.equals(thisHost));
			nativeLibViolation[0] = nativeLibViolation[0] || jad.isNativeLib();
		    }
		    
		    public void visitExtensionDesc(ExtensionDesc ed) {
			if (!hostViolation[0] && !nativeLibViolation[0]) {
			    // Check security for this exension
			    LaunchDesc extLd = ed.getExtensionDesc();
			    // fix for 4617199
			    // added null pointer check
			    // extLd will be NULL if it is already installed
			    // which cause NPE here if it tries to install
			    // again
			    if (extLd != null && extLd.getSecurityModel() == LaunchDesc.SANDBOX_SECURITY) {
				String thisHost = ed.getLocation().getHost();
				hostViolation[0] = hostViolation[0] || (!host.equals(thisHost));
				// If extension is loaded from right place, then check extension
				// recursivly
				if (!hostViolation[0]) {
				    checkJNLPSecurityHelper(extLd, host, hostViolation, nativeLibViolation);
				}
				
			    }
			}
		    }
		    
		    public void visitPropertyDesc(PropertyDesc prd) { /* ignore */}
		    public void visitPackageDesc(PackageDesc pad) { /* ignore */ }
		    public void visitJREDesc(JREDesc jrd) { /* ignore */ }
		});
    }
    
    /** Returns the size of all the cached JAR files */
    static public long getCachedSize(LaunchDesc ld) {
	long size = 0;
	ResourcesDesc rd = ld.getResources();
	if (rd == null) return size;
	JARDesc[] jars = rd.getEagerOrAllJarDescs(true);
	
	// All URL's must point to same host is sandboxed
	for(int i = 0; i < jars.length; i++) {
	    int downloadType = (jars[i].isNativeLib()) ?
		DownloadProtocol.NATIVE_DOWNLOAD : DownloadProtocol.JAR_DOWNLOAD;
	    size += DownloadProtocol.getCachedSize(jars[i].getLocation(),
						   jars[i].getVersion(),
						   DownloadProtocol.JAR_DOWNLOAD);
	    
	}
	return size;
    }
    
    /** Returns the name of the mainclass for an application. This should be called after
     *  the main files have been downloaded - since the manifest will only be looked up
     *  in the cache.
     *
     *  This method also checks that it exist in the main class.
     */
    static String getMainClassName(LaunchDesc ld, boolean forceMain) throws IOException, JNLPException, LaunchDescException  {
	String mainclassname = null;
	
	// Check LaunchDesc for mainclass
	ApplicationDesc ad = ld.getApplicationDescriptor();
	if (ad != null) {
	    mainclassname = ad.getMainClass();
	}
	InstallerDesc edf = ld.getInstallerDescriptor();
	if (edf != null) {
	    mainclassname = edf.getMainClass();
	}
	AppletDesc appletd = ld.getAppletDescriptor();
	if (appletd != null) {
	    mainclassname = appletd.getAppletClass();
	}
	if (mainclassname != null && mainclassname.length() == 0) mainclassname = null;
	
	// Make sure main class is in main file
	if (ld.getResources() == null) return null;
	JARDesc mainJar = ld.getResources().getMainJar(forceMain);
	if (mainJar == null) return null;
	// Lookup main JAR file in cache - should already have been downloaded at this point
	// This will never return NULL - instead an exception will be thrown
	DiskCacheEntry dce = DownloadProtocol.getResource(mainJar.getLocation(),
							  mainJar.getVersion(),
							  DownloadProtocol.JAR_DOWNLOAD,
							  true,
							  null);
	JarFile jarf = new JarFile(dce.getFile());
	
	// Lookup mainclass name in manifest if not found
	if (mainclassname == null && ld.getLaunchType() != LaunchDesc.APPLET_DESC_TYPE) {
	    Manifest mf = jarf.getManifest();
	    mainclassname = (mf != null) ? mf.getMainAttributes().getValue("Main-Class") : null;
	}
	
	// See if a name is specified
	if (mainclassname == null) {
	    throw new LaunchDescException(ld,
					  ResourceManager.getString("launch.error.nomainclassspec"),
					  null);
	}
	
	// See if the class exist in main JAR file
	String mainclasspath = mainclassname.replace('.', '/') + ".class";
	if (jarf.getEntry(mainclasspath) == null) {
	    throw new LaunchDescException(ld,
					  ResourceManager.getString("launch.error.nomainclass", mainclassname, mainJar.getLocation().toString()),
					  null);
	}
	
	return mainclassname;
    }
    
    /*
     * Check if a signed JNLP file exist - and if so it must match against the one
     * we are using. Note: This check the content of the signed JNLP file matches
     * the one we are using to launch with. That the JARfile is signed is checked
     * by the classloader. The main JAR file for a JNLP file is always downloaded
     * eagerly.
     */
    static void checkSignedLaunchDesc(LaunchDesc ld) throws IOException, JNLPException {
	final ArrayList list = new ArrayList();
	// Find all extension resources recursivly
	addExtensions(list, ld);
	// Check signing of each extension
	for(int i = 0; i < list.size(); i++) {
	    LaunchDesc cur = (LaunchDesc)list.get(i);
	    checkSignedLaunchDescHelper(cur);
	}
    }
    
    /*
     * Check if the resources of a JNLP file is signed and sets up the certificate
     * in the JNLP file. This certificate must be used for all resources that are
     * part of the same JNLP file. This check will:
     *  - prompt to accept certificates that have not already been accepted.
     * The main JAR file for a JNLP file is always downloaded
     */
    static void checkSignedResources(LaunchDesc ld) throws IOException, JNLPException {
	final ArrayList list = new ArrayList();
	// Find all extension resources recursivly
	addExtensions(list, ld);
	// Check the resources in each LaunchDesc
	for(int i = 0; i < list.size(); i++) {
	    LaunchDesc cur = (LaunchDesc)list.get(i);
	    checkSignedResourcesHelper(cur);
	}
    }
    
    
    static private void addExtensions(final ArrayList list, LaunchDesc ld) {
	list.add(ld);
	ResourcesDesc rd = ld.getResources();
	if (rd != null) {
	    rd.visit(new ResourceVisitor() {
			public void visitJARDesc(JARDesc jad) { /* ignore */ }
			public void visitPropertyDesc(PropertyDesc prd) { /* ignore */ };
			public void visitPackageDesc(PackageDesc pad) { /* ignore */ } ;
			public void visitJREDesc(JREDesc jrd) { /* ignore */ };
			public void visitExtensionDesc(ExtensionDesc ed ) {
			    if (!ed.isInstaller()) {
				addExtensions(list, ed.getExtensionDesc());
			    }
			}
		    });
	}
    }
    
    static private void checkSignedLaunchDescHelper(LaunchDesc ld) throws IOException, JNLPException  {
	boolean forceMain = ld.isApplicationDescriptor();
	
	byte[] signedJnlpFile = null;
	try {
	    signedJnlpFile = getSignedJNLPFile(ld, forceMain);
	    // If exist, check that is matches the one we are using
	    if (signedJnlpFile != null) {
		// Parse LaunchDesc
		LaunchDesc signedLd = LaunchDescFactory.buildDescriptor(signedJnlpFile);
		// Checks if the ldNorm matches the signedLd. This will throw a JNLPSigningException
		// if the match fails
		
		Trace.println("Signed JNLP file: ", TraceLevel.BASIC);
		Trace.println(signedLd.toString(), TraceLevel.BASIC);
		
		ld.checkSigning(signedLd);
		signedJnlpFile = null; // Make it GC'able
	    }
	} catch(LaunchDescException jse) {
	    // Tell exception object that this happended in a signed JNLP file,
	    // so it displays a better error message
	    jse.setIsSignedLaunchDesc();
	    throw jse;
	} catch(IOException ioe) {
	    // This should be very uncommon
	    throw ioe;
	}  catch(JNLPException je) {
	    throw je;
	}
    }
    
    static private void checkSignedResourcesHelper(LaunchDesc ld) 
					throws IOException, JNLPException  {
	ResourcesDesc rd = ld.getResources();
	if (rd == null) return;
	// Get all JARDescs local to this JNLP file
	JARDesc[] jds = rd.getLocalJarDescs();
	
	// Check if a certificate is used, and that the same certificate
	// is used for all
	boolean allSigned = true;
	Certificate[] certChain = null;
	CodeSource cs = null;
	URL home = ld.getCanonicalHome();
	
	int jarsCached = 0;
	URL unsigned = null;
	for(int i = 0; i < jds.length; i++) {
	    JARDesc jd = jds[i];
	    int type = jd.isJavaFile() ? DownloadProtocol.JAR_DOWNLOAD : 
					 DownloadProtocol.NATIVE_DOWNLOAD;
	    DiskCacheEntry dce = DownloadProtocol.getCachedVersion(
		jd.getLocation(), jd.getVersion(), type);

	    if (dce != null) {	
	        jarsCached++;
		JarFile jarFile = new JarFile(dce.getFile());
		CodeSource jarCs = SigningInfo.getCodeSource(home, jarFile);
		if (jarCs != null) {
		    Certificate[] jarCertChain = jarCs.getCertificates();

		    if (jarCertChain == null) {
			Trace.println("getCertChain returned null for: " + 
				      dce.getFile(), TraceLevel.BASIC);
			allSigned = false;
			unsigned = jd.getLocation();
		    }
		    if (certChain == null) {
			// First signed JAR file
			certChain = jarCertChain;
			cs = jarCs;
		    } else if (jarCertChain != null) {
			// Throw exception is not all JAR files same certificate
			if (!SigningInfo.equalChains(certChain, jarCertChain)) {
			    throw new LaunchDescException(ld, 
							  ResourceManager.getString(
										    "launch.error.singlecertviolation"), null);
			}
		    }
		}
	    }
	}

	// if requires signing ...	
	if (ld.getSecurityModel() != LaunchDesc.SANDBOX_SECURITY) {
	    // Make sure everything is signed so far
	    if (!allSigned) {
		throw new UnsignedAccessViolationException(ld, unsigned, true);
	    }
	    // Check if permissions should be granted
	    if (jarsCached > 0) {
		AppPolicy.getInstance().grantUnrestrictedAccess(ld, cs);
	    }
	}
    }
    
    private static final String SIGNED_JNLP_ENTRY = "JNLP-INF/APPLICATION.JNLP";
    
    /** Returns true if the JNLP file is propererly signed - otherwise false
     *  Only applications that request unrestricted access needs to be signed
     */
    static private byte[] getSignedJNLPFile(LaunchDesc ld, boolean forceMain) throws IOException, JNLPException  {
	if (ld.getResources() == null) return null;
	JARDesc mainJar = ld.getResources().getMainJar(forceMain);
	if (mainJar == null) return null;
	// Lookup main JAR file in cache - should already have been downloaded at this point
	DiskCacheEntry dce = DownloadProtocol.getResource(mainJar.getLocation(),
							  mainJar.getVersion(),
							  DownloadProtocol.JAR_DOWNLOAD,
							  true,
							  null);
	JarFile jarf = new JarFile(dce.getFile());
	JarEntry sjfe = jarf.getJarEntry(SIGNED_JNLP_ENTRY);
	if (sjfe == null) {
	    // Search no case sensitive
	    Enumeration allnames = jarf.entries();
	    while(allnames.hasMoreElements() && sjfe == null) {
		JarEntry jfe = (JarEntry)allnames.nextElement();
		if (jfe.getName().equalsIgnoreCase(SIGNED_JNLP_ENTRY)) {
		    sjfe = jfe;
		}
	    }
	}
	// No entry found
	if (sjfe == null) {
	    if (jarf != null) jarf.close();
	    return null;
	}
	
	// Read contents of signed JNLP file into bytearray
	byte[] signedJnlp = new byte[(int)sjfe.getSize()];
	DataInputStream is = new DataInputStream(jarf.getInputStream(sjfe));
	is.readFully(signedJnlp, 0, (int)sjfe.getSize());
	is.close();
	jarf.close();
	
	return signedJnlp;
    }
}




