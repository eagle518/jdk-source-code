/*
 * @(#)LaunchDownload.java	1.136 10/05/20
 * 
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws;

import java.io.File;
import java.io.FileNotFoundException;
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
import java.util.Iterator;
import java.util.HashSet;
import java.util.List;
import java.util.Arrays;
import java.security.CodeSource;
import com.sun.javaws.jnl.*;
import com.sun.javaws.security.*;
import com.sun.javaws.exceptions.*;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.config.Config;
import com.sun.deploy.config.JREInfo;
import com.sun.deploy.perf.DeployPerfUtil;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.net.DownloadEngine;
import com.sun.deploy.cache.Cache;
import com.sun.deploy.cache.LocalApplicationProperties;
import com.sun.deploy.Environment;
import com.sun.deploy.ui.AppInfo;
import com.sun.javaws.progress.CustomProgress;
import com.sun.javaws.progress.Progress;
import java.security.CodeSigner;
import java.util.HashMap;
import java.util.Map;
import com.sun.javaws.progress.ProgressListener;

import java.util.concurrent.*;
import sun.awt.AppContext;


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

    private static int _numThread = 0;
    private static JNLPException _exception = null;

    private static final Object syncObj = new Object();
    
    public static final int CONCURRENT_DOWNLOADS_DEF = (Config.getIntProperty(Config.JAVAWS_CONCURRENT_DOWNLOADS_KEY) == -1)? 
                        Config.JAVAWS_CONCURRENT_DOWNLOADS_DEF: Config.getIntProperty(Config.JAVAWS_CONCURRENT_DOWNLOADS_KEY);

    public static final String APPCONTEXT_THREADPOOL_KEY = Config.getAppContextKeyPrefix() +"launchdownloadthreadpoolinappcontext";
   
    // This is to insert the cache entry for no href jnlp file
    public static boolean updateNoHrefLaunchDescInCache(LaunchDesc ld) {
        if (Cache.isCacheEnabled() == false) {
            return false;
        }
        // If no href is given, put the current ld in as canonical;
        URL ref = ld.getCanonicalHome();
            
        File cachedJNLP = null;
        // Look up the home in the cahce
        try {
            cachedJNLP = DownloadEngine.getCachedFile(ref);
            
            if (cachedJNLP == null) {
                Cache.createNoHrefCacheEntry(ref, ld.getBytes());
                return true; // Not cached
            }
            Trace.println("Loaded descriptor from cache at: " + ref,
                    TraceLevel.BASIC);
            
            byte[] cachedContents = LaunchDescFactory.readBytes(
                    new FileInputStream(cachedJNLP), cachedJNLP.length());
            
            if (ld.hasIdenticalContent(cachedContents)) {
                return false; // they are the same
            }
            Cache.createNoHrefCacheEntry(ref, ld.getBytes());
            return true;
        } catch(IOException ioe) {
            Trace.ignoredException(ioe);
        }       
        return false;
    }
    
    /** Returns a LaunchDesc if the one on the Web is newer than the cached one,
     *  otherwise null
     */
    public static LaunchDesc getUpdatedLaunchDesc(URL origLocation, URL thisCodebase) throws JNLPException, 
            IOException {
        // If not home is given, the current one is fine
        if (origLocation == null) return null;
        // Newer on web?
        boolean update;
        try {
            update = DownloadEngine.isUpdateAvailable(origLocation, null);
        } catch (IOException ioe) {
            Trace.ignored(ioe);
            update = false;
        }
       
        if (!update) {
            Trace.println("Update JNLP: no update for: "+origLocation,
                    TraceLevel.BASIC);
            return null;
        }

        // Yes, download it and return it
        Trace.println("Update JNLP: "+origLocation+", thisCodebase: "+thisCodebase, TraceLevel.BASIC);
            
        File cachedFile = null;

        try {
            // force download of new JNLP
            DownloadEngine.getResource(origLocation, null, null, null, true);
            
            cachedFile = DownloadEngine.getCachedFile(origLocation);
        } catch(FileNotFoundException fnfe) {
            Trace.ignoredException(fnfe);
        }
        try {
            if (cachedFile != null) {
		LaunchDesc ld = null;
		try {
		    ld = LaunchDescFactory.buildDescriptor(
                             cachedFile, thisCodebase, origLocation, origLocation);
		    return ld;
		} catch (LaunchDescException lde) {
		    ld = LaunchDescFactory.buildDescriptor(cachedFile);
		    if (ld == null) {
			throw lde;
		    } else {
			return ld;
		    }
		}
            } else {
                return LaunchDescFactory.buildDescriptor(origLocation, origLocation);
            }
        } catch (JNLPException je) {
            throw je;
        }
    }

    /**
     * return true if a jnlp file for this ld is in the cache
     *
     */
    public static boolean isJnlpCached(LaunchDesc ld) {
        try {
            return DownloadEngine.isResourceCached(
                ld.getCanonicalHome(), null, null);
        } catch (Exception e) {
            Trace.ignored(e);
            return false;
        }
    }

    public static boolean isInCache(LaunchDesc ld) {
        return isInCache(ld, false);
    }

    /** Checks all resources needed by the given LaunchDescriptor is cached.
     *  - this includes all library extensions are downloaded,
     *  - all installer extensions are installed
     *  - all eager JAR resources are cached
     */
    public static boolean isInCache(LaunchDesc ld, boolean skipExtension) {
        ResourcesDesc rd = ld.getResources();
        if (rd == null) return true;
        
        try {
            // Check if LaunchDesc is cached (if a home is included)
            if (ld.getLocation() != null) {               
                if (DownloadEngine.isResourceCached(
                        ld.getLocation(), null, null) == false) {
                    return false;
                }              
            }

            if (skipExtension == false) {
                // Get all extensions
                boolean success = getCachedExtensions(ld);
                // Not all extensions were cached, or installer extension wasn't
                // successfully installed - return false
                if (!success) {
                    return false;
                }
            }
            
            // Get all eager JAR resources
            JARDesc[] jars = rd.getEagerOrAllJarDescs(false);
            for(int i = 0; i < jars.length; i++) {                
                if (DownloadEngine.isResourceCached(jars[i].getLocation(),
                        null, jars[i].getVersion(), 
                        jars[i].isNativeLib() ? 
                            (DownloadEngine.NATIVE_CONTENT_BIT |
                            DownloadEngine.JAR_CONTENT_BIT) :
                            DownloadEngine.NORMAL_CONTENT_BIT) == false) {
                    return false;
                }
                // make sure cached jar is not corrupted
                if (DownloadEngine.isJarFileCorrupted(jars[i].getLocation(),
                        jars[i].getVersion())) {
                    // remove corrupeted resource
                    DownloadEngine.removeCachedResource(jars[i].getLocation(),
                        null, jars[i].getVersion());
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
    
    private static void updateCheck(final URL url, final String version,
            final boolean lazy) {
        updateCheck(url, version, lazy, false);
    }
  
    private static void updateCheck(final URL url, final String version,
            final boolean lazy, final boolean isIcon) {
        updateCheck(url, version, lazy, isIcon, false);
    }

    private static void updateCheck(final URL url, final String version,
            final boolean lazy, final boolean isIcon, final boolean isPack200) {

        // no update check for versioned resource
        if (version != null) {
            return;
        }
       
        synchronized (syncObj) {
            _numThread++;
        }
        new Thread(new Runnable() {
                public void run() {                  
                    JNLPException exception = null;
                  
                    try {                

                        boolean update = DownloadEngine.isUpdateAvailable(url, 
                                version, isPack200);
 
                        if (isIcon && update) {
                            Globals.setIconImageUpdated(true);
                        }

                        synchronized (syncObj) {
                            if (update && !updateAvailable) {
                                updateAvailable = true;        
                            }
                        }
                    } catch (IOException ioe) {
                        exception = new FailedDownloadingResourceException(
                                url, version, ioe);
                    } finally {
                        synchronized(syncObj) {
                            if (_exception == null) {
                                _exception = exception;
                            }
                            _numThread--;                                
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
            try {
                boolean update = DownloadEngine.isUpdateAvailable(homeUrl, null);
                if (update) return true;
            } catch (IOException ioe) {
                throw new FailedDownloadingResourceException(homeUrl, null, ioe);
            }
        }

        ResourcesDesc rd = ld.getResources();
        if (rd == null) return false;

        // fix for 4528392
        // check all extension JNLP files
        ExtensionDesc[] ed_array = rd.getExtensionDescs();

        for (int i = 0; i < ed_array.length; i++) {
            URL edUrl = ed_array[i].getLocation();           
            if (edUrl != null) {
        
                updateCheck(edUrl, ed_array[i].getVersion(), false);
        
            }
        }
                
        // Check all cached resources that this application depends on (not just
        // the eager onces). We need to figure out if any of the lazy JARs have been
        // updated too
        JARDesc[] jars = rd.getEagerOrAllJarDescs(true);
        for(int i = 0; i < jars.length; i++) {
            URL location = jars[i].getLocation();
            String version = jars[i].getVersion();
            try {
                if (DownloadEngine.isResourceCached(location, null, version)) { 
                    
                    updateCheck(location, version, jars[i].isLazyDownload(),
                            false, jars[i].isPack200Enabled());                
                    
                }
            } catch (IOException ioe) {
                Trace.ignoredException(ioe);
            }
        }
        IconDesc [] icons = ld.getInformation().getIcons();
        if (icons != null) for (int i=0; i<icons.length; i++) {
            URL location = icons[i].getLocation();
            String version = icons[i].getVersion();
            try {
                if (DownloadEngine.getCachedFile(location, version) != null) {
                    updateCheck(location, version, false, true);
                }
            } catch (IOException ioe) {
                Trace.ignoredException(ioe);
            }
        }
        
        while (_numThread > 0 ) {          
            synchronized (syncObj) {
                if (updateAvailable) {
                    break;
                }
                if (_exception != null) {
                    throw _exception;
                }
            }
        }
        return updateAvailable;
    }    

    private static class ProgressRecord {
        private String _url;
        private String _ver;
        private int _size;
        private double _percent;

        /* indicator of complexity of this job.
         * Used it to calculate overall progress
         */
        private double _weight;

        public ProgressRecord(String url, String ver, int size) {
            _url = url;
            _ver = ver;
            _size = size;
            _weight = 1.0;
            _percent = 0;
        }

        public void setWeight(double weight) {
            _weight = weight;
        }

        public void setSize(int size) {
            _size = size;
        }

        public double getPercent() {
            return _percent;
        }

        public String getUrl() {
            return(_url);
        }

        public int hashCode() {
            int hash = 7;
            hash = 79 * hash + (this._url != null ? this._url.hashCode() : 0);
            return hash;
        }

        public boolean equals(Object o) {
            return _url.equals(((ProgressRecord)o)._url);
        }

        public double getWeight() {
            return _weight;
        }

        public void downloadProgress(int bytesDownloaded) {
            _percent = (((double)bytesDownloaded / (double)_size)) * 0.8;
        }

        public void patchProgress(int patchPercent) {
            _percent = (((double)patchPercent / 100) * 0.1) + 0.8;
        }

        public void validateProgress(int sofar, int total) {
             //Note: after validation is complete progress
             //   would be at 95%.
             // This is because some other work is done after that
             // and it is hard to characterize it in details
             // (e.g. saving cache entries to the disk)
             //We will allocate 5% for them and expect markComplete to be called
            _percent = (((double)sofar / (double)total) * 0.05) + 0.9;
        }

        private void markComplete() {
            _percent = 1.0;
        }
    }

    private static class DownloadCallbackHelper implements
                         DownloadEngine.DownloadDelegate {
        ProgressListener _progressListener; // Callback method
        long _totalSize = -1;
        final ArrayList _records;
        int _numOfJars = 1;
        int _jarsDone = 0;

        public DownloadCallbackHelper(ProgressListener dp) {
            _progressListener = dp;
            _records = new ArrayList();

        }

        /*
         * To provide better user experience we do not simply show "number of jars
         * processed". Some jars are tiny, others are large and may take way longer
         * to load.
         *
         * Unfortunatelly to find jar download size in the accurate way we need
         * to open network connections and process http headers for all of the jars.
         * This could be very slow.
         *
         * Current implementation relies on developer to provide size estimates
         * using size attribute of jar tags in the JNLP. We allocate "blocks"
         * of the progress indicator proportionally to these sizes.
         *
         * For jars that do not have sizes specified we will assume size is
         * "average", i.e. weight = 1.0
         *
         * IMPORTANT NOTE: Initialization of custom progress is performed concurrently
         *    with loading other jars. Therefore by the time progress indicator
         *    is displayed for the first time overallProgress might be
         *    significantly higher than 0. To improve user experience
         *    CustomProgress class will rescale overall progress, so visual
         *    progress will still be reported in 0 to 100 range.
         *
         * IMPORTANT NOTE: At this point of execution we do not know which jars
         *    will be actually loaded and which will be taken from cache. This may
         *    make reported progress to be not smooth. However, due to concurrent
         *    initialization of custom progress and loading jars we may find
         *    this info by the time progress is shown for the first time.
         *    Due to rescaling logic in CustomProgress this rapid increase
         *    of overall progress will be smoothed (but it is still possible
         *    that we discover jar is cached late for apps with many (4+) jars).
         */

        /*
         * Add record we will track progress for.
         * Records will be added later on on demand if needed but for on demand
         * records we will not take their weights into account.
         */
        public void register(String url, String version, int sizeEstimate, double weight) {
            ProgressRecord record = getProgressRecord(url);
            if (record == null) {
                record = new ProgressRecord(url, version, sizeEstimate);
                record.setWeight(weight);
                synchronized (_records) {
                    _records.add(record);
                }
            } else {
                record.setWeight(weight);
                record.setSize(sizeEstimate);
            }
        }

        public void setTotalSize(long size) {
            _totalSize = size;
        }

        public void setNumOfJars(int num) {
            _numOfJars = num;
        }

        public void setJarsDone(int num) {
           _jarsDone = num;
        }

        public void downloading(URL url, String version, int readSoFar,
                int total, boolean willPatch) {
            if (_progressListener != null) {
                String u = url.toString();
                ProgressRecord record = getProgressRecord(u);
                if (record == null) {
                    record = new ProgressRecord(u, version, total);
                    synchronized(_records) {
                        _records.add(record);
                    }
                } else {
                    //preregistered entry might not have accurate size
                    // => update it
                    record.setSize(total);
                }
                record.downloadProgress(readSoFar);

                int overallPercent = getOverallPercent();

                _progressListener.progress(url, version,
                    readSoFar, _totalSize, overallPercent);
            }
        }

        public void patching(URL url, String version, int percentDone) {
            if (_progressListener != null) {
                String u = url.toString();
                ProgressRecord record = getProgressRecord(u);
                if (record != null) {
                    record.patchProgress(percentDone);

                    int overallPercent = getOverallPercent();

                    _progressListener.upgradingArchive(url, version,
                        percentDone, overallPercent);
                }
            }
        }
        
        public void validating(URL url, int readSoFar, int total) {
            if (_progressListener != null) {
                String u = url.toString();
                ProgressRecord record = getProgressRecord(u);
                if (record != null) {
                    record.validateProgress(readSoFar, total);

                    int overallPercent = getOverallPercent();

                    _progressListener.validating(url, null, readSoFar,
                                                 total, overallPercent);
                }
            }
        }

        public ProgressRecord getProgressRecord(String url) {
            synchronized (_records) {
                Iterator it = _records.iterator();
                while (it.hasNext()) {
                    ProgressRecord record = (ProgressRecord) it.next();
                    if ((url != null) && (url.equals(record.getUrl()))) {
                        return record;
                    }
                }
            }
            return null;
        }

        public int getOverallPercent() {
            double percent = 0;
            double totalWeight = 0;
            synchronized(_records) {
                Iterator it = _records.iterator();
                while (it.hasNext()) {
                    //overall percent is weighted sum of progress for
                    //individual entries
                    ProgressRecord record = (ProgressRecord) it.next();
                    percent += record.getPercent()*record.getWeight();
                    totalWeight += record.getWeight();
                }
            }
            int overall = (int) (percent*100 / totalWeight);
            if (overall > 100) overall = 100;

            return overall;
        }


        // A failed download is communicated directly
        public void downloadFailed(URL url, String version) {
            if (_progressListener != null) {
                _progressListener.downloadFailed(url, version);
            }
        }

        //This is critical method for jars that are cached
        //For those jars we never get any download progress notifications
        //but we need to mark them as complete at some point.
        //
        //It is also important for normal jars as last progress notification
        //we will get after jar is validated will be 95% completion
        void jarDone(URL url) {
            if (_progressListener != null) {
                String u = url.toString();
                ProgressRecord record = getProgressRecord(u);
                if (record != null && record.getPercent() < 1.0) {
                    record.markComplete();
                    int overallPercent = getOverallPercent();

                    //Ensure progress listener was notified
                    //NB: for cached jars we do not know number of entries,
                    //  so 1 would be ok
                    //For non cached entries we may have reported meaningfull
                    //  numbers before and now we are sending 1.
                    //Might be not best idea ... (rethink later?)
                    // _progressListener.validating(url, null, 1,
                    //                            1, overallPercent);
                }
            }
        }
    }
        
    /** Given a LaunchDesc, it downloads all JNLP files for all extensions recursivly
     *  and add the resources to the LaunchDesc
     */
    public static void downloadExtensions(LaunchDesc ld, ProgressListener dp, int remaining, ArrayList installFiles)
        throws IOException, JNLPException {

        // Get extensions and download if neccesary
        downloadExtensionsHelper(ld, dp, remaining, false, installFiles);
    }
    
    /** Given a LaunchDesc, it gets all JNLP files for all extensions recursivly
     *  from the cache. Returns false if not all extensions could be found 
     *  in the cache, or installer extension wasn't successfully installed.
     */
    public static boolean getCachedExtensions(LaunchDesc ld)
        throws IOException, JNLPException {

        // Get extensions from cache
        return downloadExtensionsHelper(ld, null, 0, true, null);
    }
    
    
    private static boolean downloadExtensionsHelper(LaunchDesc ld, ProgressListener dp,
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
            // If cacheOnly is true, just use cache; otherwise connect
            // to network to do update check
            File cachedFile = DownloadEngine.getCachedFile(
                    ed.getLocation(), ed.getVersion(), !cacheOnly, 
                    false, JREInfo.getKnownPlatforms());
              
            Trace.println("Downloaded extension: " + ed.getLocation() + 
                          "\n\tcodebase: "+ed.getCodebase()+
                          "\n\tld parentCodebase: "+ld.getCodebase()+
                          "\n\tfile: " + cachedFile, TraceLevel.NETWORK);

            // Bail-out if failed without throwing exception (cacheOnly)
            if (cachedFile == null) {
                return false;
            }
            LaunchDesc extensionLd = LaunchDescFactory.buildDescriptor(
                cachedFile, ed.getCodebase(), ed.getLocation(), ed.getLocation());
      
            boolean downloadExtension = false;
            if (extensionLd.getLaunchType() == LaunchDesc.LIBRARY_DESC_TYPE) {
                // Resources becomes part of application
                downloadExtension = true;
            } else if (extensionLd.getLaunchType() == LaunchDesc.INSTALLER_DESC_TYPE) {
                ed.setInstaller(true);
                // If the extension is not installed already, we need to download the resources
                
                LocalApplicationProperties lap = Cache.getLocalApplicationProperties(
                    ed.getLocation(), ed.getVersion(), false);

                // This is a bit messsy, since LocallyInstalled means desktop integration for applications, and
                // really installed for extension installeds.
                downloadExtension = !lap.isLocallyInstalled();
                        
                // It is an installer extension - store location of downloaded 
                // JNLP file in list
                // fix for 4473369, 4525544
                // only re-execute installer if new installer
                // is available on the net and gets downloaded
                // fix for 4708396
                // we should re-execute installer too if
                // it had not been run at all (downloadExtesnion == true)
                if (installFiles != null &&  (isUpdateAvailable(extensionLd) || downloadExtension)) {
                    installFiles.add(cachedFile);
                } 

                if (cacheOnly && downloadExtension) {
                    // If just checking if installer is in cache, not being
                    // installed counts as not being in cache. (ie: reload)
                    return false;
                }
                
            } else {
                throw new MissingFieldException(extensionLd.getSource(), 
                            "<component-desc>|<installer-desc>");
            }
            
            if (downloadExtension) {
                ed.setExtensionDesc(extensionLd);
                // Do recursion.
                boolean res = downloadExtensionsHelper(extensionLd, dp, 
                            remaining, cacheOnly, installFiles);
                if (!res) return false; // Bail-out if failed (cachedOnly)
            }
        }
        return true;
    }
    
    /** Download the selected JRE descriptor */
    static public void downloadJRE(LaunchDesc ld, ProgressListener dp, 
            ArrayList installFiles)
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
  
        File cachedFile = DownloadEngine.getUpdatedFile(location, version,
                isPlatformVersion, knownPlatforms);       
        
        // Parse ExtensionDesc for JRE installer
        LaunchDesc extensionLd = LaunchDescFactory.buildDescriptor(
            cachedFile, null, null, null); // no derived codebase for JRE at all
        if (extensionLd.getLaunchType() != LaunchDesc.INSTALLER_DESC_TYPE ) {
            throw new MissingFieldException(extensionLd.getSource(), "<installer-desc>");
        }
        
        
        // It is an installer extension - store location of downloaded JNLP file in list
        if (installFiles != null) installFiles.add(cachedFile);
        
        // Add LaunchDesc to ExtensionElement
        jd.setExtensionDesc(extensionLd);
        
        // Download recursivly all extensions needed by JRE installer
        downloadExtensionsHelper(extensionLd, dp, 0, false, installFiles); // JRE ext. without derived codebase
    }
    
    /** Download all resources needed based on a single resource. A single
     *  resource can trigger several downloads. This method is used by both
     *  the DownloadService and the JNLP Classloader
     */
    static public void downloadResource(LaunchDesc ld, URL location, String version, ProgressListener dp, boolean isCacheOk)
        throws IOException, JNLPException {
        ResourcesDesc resources = ld.getResources();
        if (resources == null) return;
        int downloads = resources.getConcurrentDownloads();
        JARDesc[] jardescs = resources.getResource(location, version);
        downloadJarFiles(jardescs, dp, isCacheOk, downloads);
    }
    
    /** Download all resources needed based on a part name.
     */
    static public void downloadParts(LaunchDesc ld, String[] parts, ProgressListener dp, boolean isCacheOk)
        throws IOException, JNLPException {
        ResourcesDesc resources = ld.getResources();
        if (resources == null) return;
        int downloads = resources.getConcurrentDownloads();
        JARDesc[] jardescs = resources.getPartJars(parts);
        downloadJarFiles(jardescs, dp, isCacheOk, downloads);
    }
    
    
    /** Download all resources needed by a particular part of an specified
     *  extension
     */
    static public void downloadExtensionPart(LaunchDesc ld, URL location, String version, String parts[],
                                             ProgressListener dp, boolean isCacheOk)
        throws IOException, JNLPException {
        ResourcesDesc resources = ld.getResources();
        if (resources == null) return;
        int downloads = resources.getConcurrentDownloads();
        JARDesc[] jardescs = resources.getExtensionPart(location, version, parts);
        downloadJarFiles(jardescs, dp, isCacheOk, downloads);
    }
    
    /** Download all eager or just plain all resources */
    static public void downloadEagerorAll(LaunchDesc ld, boolean downloadAll, 
            ProgressListener dp, boolean isCacheOk)
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
                    
                    if (!hm.contains(allJarDescs[i]) && 
                            DownloadEngine.getCachedJarFile(location, 
                            version) != null) {
                        // if resource is lazy, check for update
                        if (allJarDescs[i].isLazyDownload()) {
                            boolean update = DownloadEngine.isUpdateAvailable(
                                    location, version, 
                                    allJarDescs[i].isPack200Enabled());
                            if (update) {
                                // if there is an update,
                                // remove the old lazy resource, so the
                                // updated version will be downloaded lazily
                                // when needed
                                DownloadEngine.removeCachedResource(location, 
                                        null,  version); 
                            }
                            // do not download lazy resource
                            allJarDescs[i] = null;
                        } else {
                            found++;
                        }
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
        int n = ld.getResources().getConcurrentDownloads();
        
        Trace.println("LaunchDownload: concurrent downloads from LD: "+n, TraceLevel.NETWORK);
        downloadJarFiles(jardescs, dp, isCacheOk, n);

        // also get default icon if there is one.
        IconDesc id = ld.getInformation().getIconLocation(
            AppInfo.ICON_SIZE, IconDesc.ICON_KIND_DEFAULT);
        if (id != null) {
            try {
                DownloadEngine.getResource(id.getLocation(), null, 
                    id.getVersion(), null, true, 
                    DownloadEngine.NORMAL_CONTENT_BIT);
                Trace.println("Downloaded " + id.getLocation(), 
                    TraceLevel.NETWORK);
            } catch (Exception e) {
                Trace.ignored(e);
            }
        }
    }

    public static void reverse(JARDesc[] b) {
        int left = 0;          // index of leftmost element
        int right = b.length - 1; // index of rightmost element

        while (left < right) {
            // exchange the left and right elements
            JARDesc temp = b[left];
            b[left] = b[right];
            b[right] = temp;

            // move the bounds toward the center
            left++;
            right--;
        }
    }
  
    
    /** Downloads all JAR resources specified in an array
     *
     *  The downloading happens in two steps:
     *   1. Determine size of resources that needs to be downloaded
     *   2. Download resources
     *
     *  A delegate object can be passed in that monitors the download progress
     *
     */
    private static void downloadJarFiles(JARDesc[] jars, ProgressListener dp, 
            boolean isCacheOk) throws JNLPException, IOException{
        downloadJarFiles(jars, dp, isCacheOk, CONCURRENT_DOWNLOADS_DEF);
    }

    private static int getDownloadType(JARDesc jar) {
                int downloadType = DownloadEngine.JAR_CONTENT_BIT;
                if (jar.isNativeLib()) {
                    downloadType = downloadType |
                            DownloadEngine.NATIVE_CONTENT_BIT;
                }
                if (jar.isPack200Enabled()) {
                    downloadType = downloadType |
                            DownloadEngine.PACK200_CONTENT_BIT;
                }
                if (jar.isVersionEnabled()) {
                    downloadType = downloadType |
                            DownloadEngine.VERSION_CONTENT_BIT;
                }
                return downloadType;
    }

   //NB: only called if Progress.getCustomProgress() != null
   static public void downloadProgressJars(LaunchDesc ld)
        throws IOException, JNLPException {

        ExecutorService progressThreadPool = null;
        List progressJobs = null;
        CustomProgress cp = Progress.getCustomProgress();

        ResourcesDesc resources = ld.getResources();

        if (resources == null) {
            cp.markLoaded(null);
            return;
        }

        //assume we may have default FX custom progress and app custom progress
        // (we can have more jars, it will be just not concurrent download)
        progressThreadPool = getThreadPool(2);

        //for jre 1.4 or earlier we will not be loading any jars
        //they could be loaded twice but we do not really care much
        if (progressThreadPool == null) {
            cp.markLoaded(null);
            return;
        }

        JARDesc[] jardescs = resources.getEagerOrAllJarDescs(false);

        ArrayList downloadTasks = new ArrayList(2);
        for(int i = 0; i < jardescs.length; i++) {
            JARDesc jar = jardescs[i];

            if (!jar.isProgressJar()) continue;

            DownloadTask dTask = new DownloadTask(jar.getLocation(), null,
                    jar.getVersion(), null, true, getDownloadType(jar), null,
                    null, null);
            if (downloadTasks.contains(dTask) == false) {
                downloadTasks.add(dTask);
            }
        }
        try {
            progressJobs = progressThreadPool.invokeAll(downloadTasks);
        } catch (InterruptedException e) {
            Trace.ignored(e);
            progressThreadPool.shutdownNow();
        }
        progressThreadPool.shutdown();

        try {
            validateResults(progressJobs, null);
        } catch (Exception e) {
            Trace.ignoredException(e);
            if (cp != null) {
                cp.markLoaded(e);
            }
            throw new RuntimeException(e);
        }
        cp.markLoaded(null);
    }

    /** Determins total donwload size of resources, downloads resources and notifies progress.
     *
     */
    private static void downloadJarFiles(JARDesc[] jars, ProgressListener dp, 
            boolean isCacheOk, int concurrentDownloads) 
        throws JNLPException, IOException {
        // Nothing to do?
        if (jars == null) return;
        DeployPerfUtil.put("LaunchDownload.downloadJarFiles - begin");

        // if -reverse specified during import, downloads the JARs in reverse 
        // order to prevent collision 
        if (Globals.isReverseMode()) {
            reverse(jars);
        }
        
        // Determine size of download
        long totalSize = 0;
        // Download Progress object
        DownloadCallbackHelper dch = new DownloadCallbackHelper(dp);

        /*
         * Logic overview:
         *    1) caclulate "average" jar size
         *    2) assign weights to jars (proportional to size, 1 if size unknown)
         *    3) launch downloads for everything bug progress jars
         *       (for 1.4 or earlier load everything here)
         *    4) at the end of download wait for custom progress download to be
         *        complete too and validate results
         */


        int jarsWithKnownSizes = 0;
        boolean allJarsHaveKnownSize = true;

        //first lets see how many jars needs to be loaded and
        // what info we have on total download size
        for (int i = 0; i < jars.length; i++) {
            // Returns: 0 : Unknown download size
            //         >0 : Bytes needed to be downloaded
            int sz = jars[i].getSize();
            if (Progress.getCustomProgress() == null || !jars[i].isProgressJar()) {
                if (sz > 0) {
                    jarsWithKnownSizes++;
                    totalSize += sz;
                } else {
                    allJarsHaveKnownSize = false;
                }
            }
        }

        //now register jars that needs to be loaded with proper weights
        int nonProgressJars = 0;
        for (int i = 0; i < jars.length; i++) {
            // Returns: 0 : Unknown download size
            //         >0 : Bytes needed to be downloaded
            int sz = jars[i].getSize();

            /* If there is no custom progress set then it could be that we are
               doing import operation. Then we need to include progress jars
               into download. */
            if (Progress.getCustomProgress() == null || !jars[i].isProgressJar()) {
                if (sz <= 0) {
                    //no size info => assume weight 1.0
                    dch.register(jars[i].getLocation().toString(),
                            jars[i].getVersion(), 0, 1.0);
                } else if (sz > 0) {
                    //weight is proportional to average jar size
                    //We also add some constant, so even for tiny jars
                    //there will be reasonable progress reported (to make sure
                    //connection cost is taken into account)
                    dch.register(jars[i].getLocation().toString(),
                            jars[i].getVersion(), sz,
                            0.5 + (sz * jarsWithKnownSizes / ((double) totalSize)));
                }
                nonProgressJars++;
            }
        }

        //not all sizes are really known => total estimate is not reliable?
        if (!allJarsHaveKnownSize) {
            totalSize = -1;
        }

        Trace.println("Total size to download: " + totalSize, TraceLevel.NETWORK);

        // Need to download anything?
        if (totalSize == 0) return;

        dch.setTotalSize(totalSize);        
        dch.setNumOfJars(jars.length); //for prior 1.5

        int [] jarsDoneBox = new int[1];
        jarsDoneBox[0] = 0;
        
        ExecutorService threadPool = getThreadPool(concurrentDownloads);
        if (threadPool != null) {
            // Save the thread pool in AppContext. It will be retrieved to shutdown
            // the thread pool in destroying AppContext in Plugin.
            // It is not used for java webstart. The AppContext in web start is the secure AppContext
            AppContext.getAppContext().put(APPCONTEXT_THREADPOOL_KEY, threadPool);
            dch.setNumOfJars(nonProgressJars); //progress jars are loaded separately
        }
        
        ArrayList downloadTasks = new ArrayList(jars.length);
        // Download resources
        for(int i = 0; i < jars.length; i++) {
            JARDesc jar = jars[i];
            try {
                int downloadType = getDownloadType(jar);
                
                if (threadPool == null) {
                    // This runs on jre prior to 1.5, download one by one
                    // As a matter of fact, all jars are downloaded in current jre
                    // before relaunch to 1.4.2. However, 1.4.2 may not be able to use
                    // 6.0 cache :(
                    URL cacheFileURL = DownloadEngine.getResource(
                            jar.getLocation(), null, jar.getVersion(), dch,
                            true, downloadType);
                    Trace.println("Downloaded " + jar.getLocation() + ": " +
                            cacheFileURL, TraceLevel.NETWORK);
                    jarsDoneBox[0]++;
                    dch.setJarsDone(jarsDoneBox[0]);

                    // The download progress monitor has already been notified

                    // During import mode, the jar file might not be downloaded
                    // because of the filter expiration and timestamp option
                    // so cachedFileURL can be null during import mode
                    if (Cache.isCacheEnabled() && cacheFileURL == null &&
                            Environment.isImportMode() == false) {
                        throw new FailedDownloadingResourceException(null,
                                jar.getLocation(),
                                jar.getVersion(),
                                null);
                    }

                } else {
                    // add to download list to be invoked in the pool
                    if (Progress.getCustomProgress() == null || !jar.isProgressJar()) {
                      DownloadTask dTask = new DownloadTask(jar.getLocation(), null,
                            jar.getVersion(), dch, true, downloadType, dp,
                            jarsDoneBox, dch);
                      if (downloadTasks.contains(dTask) == false) {
                        downloadTasks.add(dTask);
                      }
                    }
                }              
            }  catch(JNLPException je) {
                if (dp != null) dp.downloadFailed(jar.getLocation(), jar.getVersion());
                throw je;
            }
        }

        List tasks = null;
        try {
            if (threadPool != null) {
                tasks = threadPool.invokeAll(downloadTasks);
            }
        } catch (Exception e) {
            Trace.ignored(e);
        }
        
        if (threadPool != null) {
            AppContext.getAppContext().remove(APPCONTEXT_THREADPOOL_KEY);
            threadPool.shutdown();
            validateResults(tasks, dp);
        }
        DeployPerfUtil.put("LaunchDownload.downloadJarFiles - end");
    }

    //utility method to validate if there were any exceptions in the download
    private static void validateResults(List tasks, ProgressListener dp)
            throws IOException, JNLPException {
        if (tasks != null) {
            // Examine results. By now all tasks should be done.
            // For multiple exceptions, only the first one is thrown.
            for (Iterator iter = tasks.iterator(); iter.hasNext();) {
                Future task = (Future) iter.next();
                try {
                    task.get();
                } catch (ExecutionException ee) {
                    Throwable t = ee.getCause();
                    if (null != t) {
                        if (t instanceof IOException) {
                            throw (IOException) t;
                        }
                        if (t instanceof JNLPException) {
                            // notify progress listener. With null location and null version
                            if (dp != null) {
                                dp.downloadFailed(null, null);
                            }
                            throw (JNLPException) t;
                        }
                        throw new IOException("JNLP Jar download failure.");
                    }
                } catch (InterruptedException e) {
                    Trace.ignored(e);
                }
            }
        }        
    }

    /** progress bar notification method
     *
     * @param dch DownloadCallbackHelper
     * @param counterBox counterBox[0] holds an interger used by multi-threads
     */
    private static synchronized void notifyProgress(DownloadCallbackHelper dch, 
            int counterBox[], URL jarurl) {
        if (counterBox != null && dch != null) {
            counterBox[0]++;
            Trace.println("Download Progress: jarsDone: " + counterBox[0], TraceLevel.NETWORK);
            dch.jarDone(jarurl);
            dch.setJarsDone(counterBox[0]);
        }
    }

    /** Get a fix-sized thread pool. 
     * @param threads The size of the fixed pool
     * @return the main thread pool of default fixed size; null if not supported
     */
    private static ExecutorService getThreadPool(int threads) {
        if (Config.isJavaVersionAtLeast15()) {
            ExecutorService threadPool = Executors.newFixedThreadPool(threads,
                    new ThreadFactory() {
                        public Thread newThread(Runnable r) {
                            Thread t = new Thread(r);
                            t.setDaemon(true);
                            return t;
                        }
                    });
            return threadPool;
        }

        return null;
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
    public static void checkJNLPSecurity(LaunchDesc ld) throws MultipleHostsException, 
            NativeLibViolationException {
        final boolean[] nativeLibViolation = new boolean[1];
        final boolean[] hostViolation = new boolean[1];
        ResourcesDesc rd = ld.getResources();
        if (rd == null) return;
        JARDesc mainJar = ld.getResources().getMainJar(true);
        if (mainJar == null) return;
        checkJNLPSecurityHelper(ld, mainJar.getLocation().getHost(), 
                hostViolation, nativeLibViolation);
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
                        hostViolation[0] = hostViolation[0] || 
                                (!host.equals(thisHost));
                        nativeLibViolation[0] = nativeLibViolation[0] || 
                                jad.isNativeLib();
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
                            String thisHost = ed.getLocation().getHost();
                            if (extLd != null && extLd.getSecurityModel() == 
                                    LaunchDesc.SANDBOX_SECURITY) {
                                // check extension recursivly
                                if (!hostViolation[0]) {
                                    checkJNLPSecurityHelper(extLd, thisHost, 
                                            hostViolation, nativeLibViolation);
                                }
                                
                            }
                        }
                    }
                    
                    public void visitPropertyDesc(PropertyDesc prd) { /* ignore */}
                    public void visitPackageDesc(PackageDesc pad) { /* ignore */ }
                    public void visitJREDesc(JREDesc jrd) { /* ignore */ }
                });
    }
    
    /** 
     * getCachedSize()
     * Returns the size of all the cached jar files, and icons for this app
     */
    static public long getCachedSize(LaunchDesc ld) {
        long size = 0;
        ResourcesDesc rd = ld.getResources();
        if (rd == null) return size;
        JARDesc[] jars = rd.getEagerOrAllJarDescs(true);
        
        // All URL's must point to same host is sandboxed
        for(int i = 0; i < jars.length; i++) {
            try {
                size += DownloadEngine.getCachedSize(jars[i].getLocation(), 
                    null, jars[i].getVersion(), null, false);
            } catch (IOException ioe) {
                Trace.ignoredException(ioe);
            }
        }
        IconDesc [] icons = ld.getInformation().getIcons();
        if (icons != null) for (int i=0; i<icons.length; i++) {
            try {
                size += DownloadEngine.getCachedSize(icons[i].getLocation(),
                    null, icons[i].getVersion(), null, false);
            } catch (IOException ioe) {
                Trace.ignoredException(ioe);
            }
        }
        return size;
    }
    
    /** Returns the name of the mainclass for an application. This should be called after
     *  the main files have been downloaded - since the manifest will only be looked up
     *  in the cache.
     *
     *  This method also checks that it exist in the main class.
     */
    static String getMainClassName(LaunchDesc ld, boolean forceMain) throws 
            IOException, JNLPException, LaunchDescException  {
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
        if (mainclassname != null && mainclassname.length() == 0) {
            mainclassname = null;
        }

	// if the mainclassname is not null, return it so that 
	// Launcher will try to load it from other jar files
	if (mainclassname != null) {
	    return mainclassname;
	}

	// Make sure main class is in main file
	if (ld.getResources() == null) return null;    
	JARDesc mainJar = ld.getResources().getMainJar(forceMain);
	if (mainJar == null) return null;
	// Lookup main JAR file in cache - should already have been downloaded at this point
	// This will never return NULL - instead an exception will be thrown
    
	// pass in false to JarFile ctor so no verification will be done to
	// minimize performance impact
	JarFile jarf = null;
	try {
	    jarf = new JarFile(DownloadEngine.getCachedResourceFilePath(
		mainJar.getLocation(), mainJar.getVersion()), false);
        
	    // Lookup mainclass name in manifest if not found
	    if (jarf != null && mainclassname == null &&
		ld.getLaunchType() != LaunchDesc.APPLET_DESC_TYPE) {
		Manifest mf = jarf.getManifest();
		mainclassname = (mf != null) ?
		    mf.getMainAttributes().getValue("Main-Class") : null;
	    }
        
	    // See if a name is specified
	    if (mainclassname == null) {
		throw new LaunchDescException(ld,
		    ResourceManager.getString(
		    "launch.error.nomainclassspec"),
		    null);
	    }
        
	    // See if the class exist in main JAR file
	    String mainclasspath = mainclassname.replace('.', '/') + ".class";
	    if (jarf.getEntry(mainclasspath) == null) {
		throw new LaunchDescException(ld,
		    ResourceManager.getString(
		    "launch.error.nomainclass", mainclassname,
		    mainJar.getLocation().toString()),
		    null);
	    }
        
	    return mainclassname;
	} finally {
	    if (jarf != null) {
		jarf.close();
	    }
	}

    }
    
    /*
     * Check if a signed JNLP file exist - and if so it must match against the one
     * we are using. Note: This check the content of the signed JNLP file matches
     * the one we are using to launch with. That the JARfile is signed is checked
     * by the classloader. The main JAR file for a JNLP file is always downloaded
     * eagerly.
     */
    public static void checkSignedLaunchDesc(LaunchDesc ld) throws 
            IOException, JNLPException {
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
     *
     * @return true if all ressources are signed
     */
    public static boolean checkSignedResources(LaunchDesc ld) throws IOException, 
            JNLPException, ExitException {
        final ArrayList list = new ArrayList();
        // Find all extension resources recursivly
        addExtensions(list, ld);
        // Check the resources in each LaunchDesc
        boolean allSigned = true;
        for(int i = 0; i < list.size(); i++) {
            LaunchDesc cur = (LaunchDesc)list.get(i);
            allSigned = checkSignedResourcesHelper(cur) && allSigned;
        }
        return allSigned;
    }
    
    
    static private void addExtensions(final ArrayList list, LaunchDesc ld) {
        list.add(ld);
        ResourcesDesc rd = ld.getResources();
        if (rd != null) {
            rd.visit(new ResourceVisitor() {
                        public void visitJARDesc(JARDesc jad) { /* ignore */ }
                        public void visitPropertyDesc(PropertyDesc prd) 
                        { /* ignore */ };
                        public void visitPackageDesc(PackageDesc pad) 
                        { /* ignore */ } ;
                        public void visitJREDesc(JREDesc jrd) { /* ignore */ };
                        public void visitExtensionDesc(ExtensionDesc ed ) {
                            if (!ed.isInstaller()) {
                                addExtensions(list, ed.getExtensionDesc());
                            }
                        }
                    });
        }
    }
    
    private static void checkSignedLaunchDescHelper(LaunchDesc ld) throws 
            IOException, JNLPException  {
        boolean forceMain = ld.isApplicationDescriptor();
        
        byte[] signedJnlpFile = null;
        try {
            signedJnlpFile = getSignedJNLPFile(ld, forceMain);
            // If exist, check that is matches the one we are using
            if (signedJnlpFile != null) {
                // Parse LaunchDesc
                LaunchDesc signedLd = 
                    LaunchDescFactory.buildDescriptor(signedJnlpFile, 
                      null, ld.getLocation(), ld.getLocation()); // signedJnlpFile from cache
                // Checks if the ldNorm matches the signedLd. This will throw a
                // JNLPSigningException if the match fails

                if (Trace.isTraceLevelEnabled(TraceLevel.BASIC)) {
                    Trace.println("Signed JNLP file: ", TraceLevel.BASIC);
                    Trace.println(signedLd.toString(), TraceLevel.BASIC);
                }
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
    
    static private boolean checkSignedResourcesHelper(LaunchDesc ld)
            throws IOException, JNLPException, ExitException {
        // no need to check if we are in sandbox
        if (ld.isSecure()) {
            // sandboxed jnlp file - don't break all-signed flag
            return ld.isSecureJVMArgs();
        }

        ResourcesDesc rd = ld.getResources();
        if (rd == null) {
            // no resources - don't break all-signed flag
            return true;
        }

        /*
         * Return answer is true if all of following conditions are true:
         *    1. each jar is signed (i.e. all entries are signed
         *         and there is common certificate chain for all entries) 
         *    2. there is at least one common certificate used to sign all jars
         *    3. none of the jars are blacklisted
         *
         * To avoid wasting time on validation on each start we cache results
         * of validation and try to reuse them (if webstart cache is used!).
         *
         * To facilate that we save results of actual validation into cache
         * every time validation is performed.
         *
         * We trust JAR validation always if validation timestampt is not 0
         * (SigningInfo has helper method to check this).
         *
         * We trust JNLP validation only if validation timestampts for all jars
         * are exactly the same as cached. If any of jar timestampts saved in JNLP
         * cache entry does not match then we can not use cached results and
         * have to check presence of common certificate again.
         *
         * This is because jar update and validation may happen in background
         * and cache entry will be enabled at some time later. And we may get
         * into situation when jar validation timestampt is earlier than JNLP
         * validation timestampt but jar was updated since last validation.
         * Exact match of validation timestampts is extremely unlikely.
         */


        // Get all JARDescs local to this JNLP file
        JARDesc[] jds = rd.getLocalJarDescs();

        // Check if a certificate is used, and that the same certificate
        // is used for all
        boolean allSigned = true;
        boolean sameCert = true;
        List certChains = null;
        URL home = ld.getCanonicalHome();

        int jarsCached = 0;
        URL unsigned = null;

        //FIXME: check whether there is notion of version for JNLP!
        SigningInfo sJNLP = null; 
        Map /* <String, String> */ trustedEntries = null; 

        Trace.println("Validating signatures for " +
                ld.getLocation()+" "+ld.getSourceURL(),
                TraceLevel.SECURITY);

        if (ld.getLocation() != null) {
            //prior to introduction of caching of results of security validation
            // we never used location of main JNLP 
            // and therefore JNLP files with missing href attributes worked
            //We could throw exception but technically we can proceed after
            //revalidation of main jar. 
            //And we try to do so to preserve backward compatibility

            //Should use original URL that was used for cache lookup
            URL u = ld.getSourceURL();
            if (u == null) { //this should not ever happen but just in case
                u = ld.getLocation();
            }
            sJNLP = new SigningInfo(u, ld.getVersion());
            trustedEntries = sJNLP.getTrustedEntries();
            Trace.println("TustedSet "+(trustedEntries != null ?
                Integer.toString(trustedEntries.size()) : "null"), TraceLevel.SECURITY);
        }

        SigningInfo sInfo[] = new SigningInfo[jds.length];
        boolean canTrustJNLP;
        Map /* <String, String> */ newTrustedEntries = new HashMap();
        boolean forceFullValidation = false;

        if (trustedEntries == null) {
            canTrustJNLP = false;
            Trace.println("Empty trusted set for [" + home + "]",
                TraceLevel.SECURITY);                
        } else {
            canTrustJNLP = true;
        }

        //first try to use cached result of validation if possible
        for (int i = 0; !forceFullValidation && i < jds.length; i++) {
            JARDesc jd = jds[i];

            sInfo[i] = new SigningInfo(jd.getLocation(), jd.getVersion());

            Trace.println("Round 1 (" + i + " out of "+jds.length + "):"
                    + jd.getLocation(), TraceLevel.SECURITY);

            //skip lazy jars
            if (sInfo[i].canBeSkipped()) {
                Trace.println("    Skip: "+jd.getLocation(), TraceLevel.SECURITY);
                continue;
            }

            if (sInfo[i].isKnownToBeValidated()) {
                long tm = sInfo[i].getCachedVerificationTimestampt();
                String key = jd.getLocation().toString();
                if (sInfo[i].isKnownToBeSigned() == false) {
                    //Cached jar is unsigned => answer can not be true, 
                    //no need to check further.
                    //NB: no need to update cache or anything too
                    //on subsequent run we will have to repeat this loop anyway 
                    //and will bail out at the same time
                    throw new UnsignedAccessViolationException(ld, jd.getLocation(), true);
                }
                if (canTrustJNLP) {
                    Long l = (Long) trustedEntries.get(key);
                    if (l == null || l.longValue() != tm) {
                        //cached value does not match => need to revalidate JNLP
                        Trace.println("Entry ["+key+", " + l + 
                           "] does not match trusted set. Revert to full validation of JNLP.", 
                           TraceLevel.SECURITY);
                        canTrustJNLP = false;
                    }
                }
                newTrustedEntries.put(key, new Long(tm));
            } else {
                //check failed, have to do full check
                forceFullValidation = true;
                canTrustJNLP = false;
                Trace.println("Entry ["+ jd.getLocation().toString() +
                    "] is not prevalidated. Revert to full validation of this JAR.", 
                    TraceLevel.SECURITY);
            }
        }

        //if we get here and canTrustJNLP is null then we can not simply
        // return true. We still need to actually grant permissions!
        // But we surely want to skip full validation
        if (!canTrustJNLP) {
            for (int i = 0; i < jds.length && sameCert; i++) {
                JARDesc jd = jds[i];

            Trace.println("Round 2 (" + i + " out of " + jds.length+"):"
                    + jd.getLocation(), TraceLevel.SECURITY);

            //fill sInfo array if it has not been filled during previously
                if (sInfo[i] == null) {
                    sInfo[i] = new SigningInfo(jd.getLocation(), jd.getVersion());
                }

                if (sInfo[i].canBeSkipped()) {
                    Trace.println("    Skip " +jd.getLocation(),
                            TraceLevel.SECURITY);
                    continue;
                }

                List jarCertChains = null;
                if (sInfo[i].isKnownToBeValidated()) {
                    jarCertChains = sInfo[i].getCertificates();
                } else {
                    jarCertChains = sInfo[i].check();
                }

		// For empty jars and jars without entries outside of META-INF
		// call to SigningInfo.check() will return null
		// but these jars do not really compromise the security.
		// If this is the case then canBeSkipped() will return true here
		if (sInfo[i].canBeSkipped()) {
 		    continue;
		}

                if (jarCertChains == null) {
                    allSigned = false;
                    unsigned = jd.getLocation();
                    if (ld.getSecurityModel() != LaunchDesc.SANDBOX_SECURITY) {
                        DownloadEngine.removeCachedResource(
                                jd.getLocation(), null, jd.getVersion());
                    }
                    //no need to continue as we know the answer
                    break;
                }
                if (certChains == null) {
                    // First signed JAR file
                    certChains = jarCertChains;
                } else {
		    // We have to add the latest Certificate chain in the begining of certChains list,
		    // therefore the TrustDecider will parse the latest Certificate chain first.
		    // The reason is that
		    // 1. it is more likely that latest signature added is more relevant to the end user
		    // 2. this used to be applet behavior for long time and we don't want to change it
		    // without a reason.
                    certChains = SigningInfo.overlapChainLists(jarCertChains, certChains);
                    Trace.println("Have " + (certChains == null ? 0 : certChains.size()) +
                            " common certificates after processing " +
                            jd.getLocation(), TraceLevel.SECURITY);
                    if (certChains == null) {
                        sameCert = false;
                        if (ld.getSecurityModel() != LaunchDesc.SANDBOX_SECURITY) {
                            DownloadEngine.removeCachedResource(
                                    jd.getLocation(), null, jd.getVersion());
                        }
                    }
                }

                //if we will report error then trusted will not be saved anyway
                long tm = sInfo[i].getCachedVerificationTimestampt();
                String key = jd.getLocation().toString();
                newTrustedEntries.put(key, new Long(tm));

                jarsCached++;
            }

            // if requires signing ...
            if (!ld.isSecure()) {
                // Make sure everything is signed so far
                if (!allSigned) {
                    throw new UnsignedAccessViolationException(ld, unsigned, true);
                }
                // Throw exception is not all JAR files same certificate
                if (!sameCert) {
                    throw new LaunchDescException(ld, ResourceManager.getString(
                            "launch.error.singlecertviolation"), null);
                }
                // Check if permissions should be granted
                if (jarsCached > 0) {
                    CodeSource cs = null;
                    if (Globals.isJavaVersionAtLeast15()) {
                        cs = new CodeSource(ld.getLocation(),
                                (CodeSigner[]) certChains.toArray(new CodeSigner[0]));
                    } else {
                        cs = new CodeSource(ld.getLocation(),
                                SigningInfo.toCertificateArray(certChains));
                    }

                    /*
                     * If nothing has been changed since last start and
                     * validation was successful that time then we can
                     * trust that we do not need to verify that
                     * certificates are trusted and skip redundant validation
                     * of jar entiries by class loader.
                     */
                    if (canTrustJNLP) {
                        ld.setTrusted();
                    }
                    long tm = AppPolicy.getInstance().grantUnrestrictedAccess(ld, cs);
                    if (tm > 0) {
                        //As we got to the actual validation => JNLP entry cache was out of date
                        //Return value is >0 => all certificates were accepted permanently 
                        //Otherwise we would get exception or 0

                        //update cached jar validation status if needed
                        long checkTime = System.currentTimeMillis();
                        for (int j=0; j<sInfo.length; j++) {
                             sInfo[j].updateCacheIfNeeded(true, null, checkTime, tm);

                             //make sure that timestampts in the trusted set match
                             String key = jds[j].getLocation().toString();
                             if (newTrustedEntries.containsKey(key)) {
                                 newTrustedEntries.put(key,
                                         new Long(sInfo[j].getCachedVerificationTimestampt()));
                             }
                        }
                        if (sJNLP != null) {
                            sJNLP.updateCache(true, newTrustedEntries, System.currentTimeMillis(), tm);
                        }
                    }
                    canTrustJNLP = true; 
                } else {
                    //case of JNLP file that has no jars but only extensions
                    if (sJNLP != null) {
                        sJNLP.updateCache(true, newTrustedEntries, System.currentTimeMillis(), Long.MAX_VALUE);
                    }
                    canTrustJNLP = true;
                }
            }
        }

        /*
         * We can get here only if validation passed.
         * Mark descriptor "trusted" to avoid redundant checks in the future
         */
        if (canTrustJNLP) {
            ld.setTrusted();
        }

        Trace.println("LD - All JAR files signed: " + home, TraceLevel.BASIC);
        return allSigned;
    }
    
    private static final String SIGNED_JNLP_ENTRY = "JNLP-INF/APPLICATION.JNLP";
    
    /** Returns true if the JNLP file is propererly signed - otherwise false
     *  Only applications that request unrestricted access needs to be signed
     */
    static private byte[] getSignedJNLPFile(LaunchDesc ld, boolean forceMain) 
    throws IOException, JNLPException  {
     
        if (ld.getResources() == null) return null;
        JARDesc mainJar = ld.getResources().getMainJar(forceMain);
        if (mainJar == null) return null;
        // Lookup main JAR file in cache - should already have been downloaded 
        // at this point
        JarFile jarf = null;
        try {
            // pass in false to JarFile ctor so no verification will be done to
            // minimize performance impact
            jarf = new JarFile(DownloadEngine.getCachedResourceFilePath(
                mainJar.getLocation(), mainJar.getVersion()), false);
            
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
                return null;
            }
            
            // Read contents of signed JNLP file into bytearray
            byte[] signedJnlp = new byte[(int)sjfe.getSize()];
            DataInputStream is = new DataInputStream(jarf.getInputStream(sjfe));
            is.readFully(signedJnlp, 0, (int)sjfe.getSize());
            is.close();
            
            return signedJnlp;
        } finally {
            if (jarf != null) {
                jarf.close();
            }
        }
    }
    
    private static class DownloadTask implements Callable {
        private URL url;
        private int downloadType;
        private String resourceID;
        private String versionString;
        private DownloadEngine.DownloadDelegate dd;
        private boolean doDownload;
        private ProgressListener dp;
        private int counterBox[];
        private DownloadCallbackHelper dch;
        
        public DownloadTask(URL url, String resourceID, String versionString,
                DownloadEngine.DownloadDelegate dd, boolean doDownload, 
                int downloadType, ProgressListener dp, int counterBox [], 
                DownloadCallbackHelper dch) {
            this.url = url;
            this.downloadType = downloadType;
            this.resourceID = resourceID;
            this.versionString = versionString;
            this.dd = dd;
            this.doDownload = doDownload;
            this.dp = dp;
            this.counterBox = counterBox;
            this.dch = dch;
        }

        public URL getURL() {
            return url;
        }

        public String getVersion() {
            return versionString;
        }

        public int hashCode() {
            if (url == null) {
                return 0;
            }
            return url.hashCode();
        }

        public boolean equals(Object obj) {
            if (obj instanceof DownloadTask) {
                DownloadTask dTask = (DownloadTask) obj;
                URL u = dTask.getURL();
                String version = dTask.getVersion();
                if (url.toString().equals(u.toString())) {
                    if (versionString == null && version == null) {
                        return true;
                    }
                    if (versionString != null && version != null &
                            versionString.equals(version)) {
                        return true;
                    }
                }
            }
            return false;
        }
        
        public Object call() throws IOException, JNLPException {
            // do the download, this is blocking. Return when it is done
            // FIXME: need decouple download and write to cache in this call.
            URL cacheFileURL = DownloadEngine.getResource(url, resourceID,
                    versionString, dd, doDownload, downloadType);
            Trace.println("Downloaded " + url + ": " +
                    cacheFileURL, TraceLevel.NETWORK);
            
            // Check download failures
            if (Cache.isCacheEnabled() &&
                cacheFileURL == null &&
                Environment.isImportMode() == false) {
                throw new FailedDownloadingResourceException(null, url, versionString,null);
            }
        
            LaunchDownload.notifyProgress(dch, counterBox, url);
            return null;
        }
    } 
}




