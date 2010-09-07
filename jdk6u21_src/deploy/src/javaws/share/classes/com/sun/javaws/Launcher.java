/*
 * @(#)Launcher.java	1.279 10/05/13
 *
 * Copyright (c) 2010, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws;

import java.io.*;
import java.net.*;
import java.applet.*;

import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;

import java.util.ArrayList;
import java.util.Date;

import java.awt.Dimension;
import java.awt.Container;
import java.awt.BorderLayout;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

import javax.swing.JFrame;
import javax.swing.SwingUtilities;

import com.sun.jnlp.BasicServiceImpl;
import com.sun.jnlp.ExtensionInstallerServiceImpl;
import com.sun.jnlp.JNLPClassLoader;
import com.sun.jnlp.PreverificationClassLoader;
import com.sun.jnlp.AppletContainer;
import com.sun.jnlp.AppletContainerCallback;

import com.sun.javaws.jnl.*;
import com.sun.javaws.ui.AutoDownloadPrompt;
import com.sun.javaws.ui.SecureStaticVersioning;
import com.sun.javaws.ui.DownloadWindowHelper;
import com.sun.javaws.ui.LaunchErrorDialog;
import com.sun.javaws.security.AppPolicy;
import com.sun.javaws.security.JavaWebStartSecurity;
import com.sun.javaws.exceptions.*;
import com.sun.javaws.util.JavawsConsoleController;

import com.sun.deploy.util.*;
import com.sun.deploy.config.Config;
import com.sun.deploy.config.JREInfo;
import com.sun.deploy.si.SingleInstanceManager;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.cache.Cache;
import com.sun.deploy.cache.CacheEntry;
import com.sun.deploy.cache.LocalApplicationProperties;
import com.sun.deploy.net.DownloadEngine;
import com.sun.deploy.net.FailedDownloadException;
import com.sun.deploy.net.offline.DeployOfflineManager;
import com.sun.deploy.Environment;
import com.sun.deploy.ui.ComponentRef;
import com.sun.deploy.pings.Pings;
import com.sun.javaws.progress.Progress;
import com.sun.javaws.progress.CustomProgress;
import javax.jnlp.DownloadServiceListener;


/*
 * Given a LaunchDescriptor, the class takes care of launching
 * the given application. This might involve downloading JREs, and
 * optional packages
 */
public class Launcher implements Runnable {
    /**
     *  Reference to download progress window
     */
    private DownloadWindowHelper _downloadWindowHelper = null;

    /**
     * The console. This will be null if the user doesn't want to see the
     * console.
     */

    private LaunchDesc _initialLaunchDesc;
    private LaunchDesc _launchDesc;
    private String[] _args;
    private boolean _exit = true;
    private JAuthenticator _ja;
    private LocalApplicationProperties _lap = null;
    private JNLPClassLoader _jnlpClassLoader = null;
    private JREInfo _jreInfo = null;
    private boolean _isRelaunch = false;
    private boolean _isCached = false;
    private boolean _jreInstalled = false;

    private static boolean _isImport = false;
    private static boolean _isSilent = false;

    private LaunchSelection.MatchJREIf _jreMatcher;

    public Launcher(LaunchDesc ld) {
        _initialLaunchDesc = ld;

        // Create download window. This might not be shown. But it is created
        // here, so it can be used as a owner for dialogs.
        _downloadWindowHelper = new DownloadWindowHelper();

        Trace.println("new Launcher: " + ld.toString(), TraceLevel.BASIC);
    }

    /**
     *  Main entry point for launching the app.
     */
    public void launch(String[] args, boolean exit) {
        _args = args;
        _exit = exit;
        if (prepareToLaunch()) {
            (new Thread(Main.getLaunchThreadGroup(),
                this,
                "javawsApplicationMain")
            ).start();
        } else {     
            LaunchErrorDialog.show(_downloadWindowHelper.getOwner(),
                    new Exception(ResourceManager.getString("launch.error.category.unexpected")),
                    true);
        }
    }

    public void run() {
        try {
            doLaunchApp(); 
        } catch (Throwable t) {
            ExitException ee = (t instanceof ExitException) ?
                (ExitException) t : 
                new ExitException(t, ExitException.LAUNCH_ERROR);
            int exitValue =  (ee.getReason() == ExitException.OK) ? 0 : -1;
            if (ee.getReason() == ExitException.LAUNCH_ERROR) {
                //in case of background check make sure we will force update next time
                if (_launchDesc.getUpdater().isBackgroundUpdateRunning()) {
                  if (_lap != null) {
                      _lap.setForceUpdateCheck(true);
                      try {
                        _lap.store();
                      } catch(Exception e) {}
                  }
                }
                LaunchErrorDialog.show(_downloadWindowHelper.getOwner(),
                                        ee.getException(), _exit);
            }
            if (_exit) {
                try {
                    Main.systemExit(exitValue);
                } catch (ExitException ee2) { 
                    Trace.println("systemExit: "+ee2, TraceLevel.BASIC);
                    Trace.ignoredException(ee2);
                }
            }
        }
    }

    private boolean prepareToLaunch() {
        // check if there is another instance running
        if (SingleInstanceManager.isServerRunning(
                _initialLaunchDesc.getCanonicalHome().toString())) {
            String[] appArgs = Globals.getApplicationArgs();
  
            if ((appArgs != null) &&
                    (_initialLaunchDesc.getApplicationDescriptor() != null)) {
                _initialLaunchDesc.getApplicationDescriptor().setArguments(
                        appArgs);
            }

            // send the JNLP file to the server port
            if (SingleInstanceManager.connectToServer(
                    _initialLaunchDesc.toString())) {
                // if we get OK from server, we are done
                Trace.println("Exiting (launched in the other instance)",
                        TraceLevel.BASIC);
                return true;
            }
        // else continue normal launch
        }

        /*
         * Attempt to start from cache in case of background update check
         * or offline mode.
         * We also do this if this is relaunch as cache has to be updated
         * before relaunch happens.
         */
        boolean tryOffline = Cache.isCacheEnabled() &&
                (_initialLaunchDesc.getUpdate().isBackgroundCheck() 
                || DeployOfflineManager.isForcedOffline()
                || JnlpxArgs.getIsRelaunch());

          try {
            if (!tryOffline && DeployOfflineManager.isForcedOffline()) {
                throw new CacheUpdateRequiredException("Forced offline mode!");
            }
  
            boolean ret = prepareToLaunch(tryOffline);
            //if we can run from cache then make sure background check has been started
            if (Cache.isCacheEnabled() && !DeployOfflineManager.isForcedOffline()) {
                _launchDesc.getUpdater().startBackgroundUpdateOpt();
            }
            return ret;
        } catch (CacheUpdateRequiredException e) {
            Trace.println("Could not launch from cache. Will try online mode. ["+e.getMessage()+"]");
            if (tryOffline) {
                if (DeployOfflineManager.isForcedOffline()) {
                    // if application required resources are not cached and we are in
                    // forced offline mode (-offline); prompt user and see if they
                    // want to go online to start the application
                    DeployOfflineManager.setForcedOffline(false);
                    if (DeployOfflineManager.askUserGoOnline(_initialLaunchDesc.getLocation()) == false) {
                        // user choose not to go online even if
                        // application might not be able to start
                        DeployOfflineManager.setForcedOffline(true);
  
                        Trace.println("User chose not to go online and we can not not start in offline mode");
                        LaunchErrorDialog.show(_downloadWindowHelper.getOwner(),
                                new OfflineLaunchException(OfflineLaunchException.MISSING_RESOURCE),
                                _exit);
                        return false;
                    }
                  }
              }
            try {
                return prepareToLaunch(false);
            } catch (CacheUpdateRequiredException ex) {
                Trace.println("Unexpected exception: " + ex);
                return false;
            }
        }
    }
  
    class CacheUpdateRequiredException extends Exception {
        public CacheUpdateRequiredException(String message) {
            super(message);
        }
    };

    private boolean prepareToLaunch(boolean offlineOnly) throws CacheUpdateRequiredException {
        try {
            Trace.println("prepareToLaunch: offlineOnly="+offlineOnly, TraceLevel.NETWORK);
            _isSilent = Globals.isSilentMode();            

            PerfLogger.setTime("Begin updateFinalLaunchDesc");
            
	        boolean isUpdated = updateFinalLaunchDesc(_initialLaunchDesc, 0,
                    offlineOnly);

            // remove temp jnlp file if it is in the cache already
            removeTempJnlpFile(_launchDesc);

            if (isUpdated) {
                File cachedJnlp = null;

                if (_launchDesc.isApplicationDescriptor()) {
                    try {
                        cachedJnlp = DownloadEngine.getCachedFile(
                                _launchDesc.getCanonicalHome());
                    } catch (IOException ioe) {
                        Trace.ignoredException(ioe);
                    }

                    if (_args != null) {
                        // if we need to relaunch, make sure we relaunch with
                        // current cached jnlp
                        _args[0] = cachedJnlp.getPath();
                    }
                }
            }

            PerfLogger.setTime("End updateFinalLaunchDesc");

            boolean isInstaller = _launchDesc.isInstaller();
            
            _isRelaunch = JnlpxArgs.getIsRelaunch();

            URL jnlpUrl = _launchDesc.getCanonicalHome();
            
            if (!isInstaller && !_launchDesc.isLibrary()) {
                _lap = Cache.getLocalApplicationProperties(jnlpUrl);
                if (offlineOnly && _lap != null && _lap.forceUpdateCheck()) {
                    throw new CacheUpdateRequiredException(
                            "Need to update: force update set in LAP");
                }
            }
            
            if (isUpdated && _lap != null && Cache.isCacheEnabled() &&
                    _lap.isLocallyInstalled() &&
                    LocalInstallHandler.getInstance().isShortcutExists(_lap)) {
                // update shortcut if cached jnlp updated and shortcut
                // already exists
		// variable isUpdated should be true to trigger shortcut update
                notifyLocalInstallHandler(_launchDesc, _lap, _isSilent, 
                        isUpdated, _downloadWindowHelper.getOwnerRef());
            }

            URL href = _launchDesc.getLocation();
            if (href != null) {
                Cache.removeRemovedApp(href.toString(), 
                    _launchDesc.getInformation().getTitle());
            }

            Trace.println("isUpdated: " + isUpdated, TraceLevel.NETWORK);

            // See if the JNLP file properties contains any debugging properties
            if (_launchDesc.getResources() != null) {
                Globals.getDebugOptionsFromProperties(
                    _launchDesc.getResources().getResourceProperties());
            }

            /**
             * We initialize the dialog to pop up for user authentication
             * to password prompted URLs.
             */
            if (Config.getBooleanProperty(Config.SEC_AUTHENTICATOR_KEY)) {
                _ja = JAuthenticator.getInstance(_downloadWindowHelper.getOwnerRef());
                Authenticator.setDefault(_ja);
            }

            _isImport = Environment.isImportMode() || 
                (_launchDesc.getLaunchType() == LaunchDesc.LIBRARY_DESC_TYPE);
            if (!_isSilent) {
                _downloadWindowHelper.initialize(_launchDesc, true, false);
            }
            prepareAllResources(_launchDesc, _args, 
                isUpdated, offlineOnly);
        } catch (CacheUpdateRequiredException e) {
            throw e;
        } catch (Throwable t) {
            ExitException ee = (t instanceof ExitException) ? (ExitException) 
                t : new ExitException(t, ExitException.LAUNCH_ERROR);
            int exitValue =  (ee.getReason() == ExitException.OK) ? 0 : -1;
            if (ee.getReason() == ExitException.LAUNCH_ERROR) {
                LaunchErrorDialog.show(_downloadWindowHelper.getOwner(),
                                        ee.getException(), _exit);
            }
            if (exitValue == 0) {
                Trace.println("Exiting", TraceLevel.BASIC);
            } else {
                Trace.ignoredException(ee);
            }
            if (_exit) {
                try {
                    Main.systemExit(exitValue);
                } catch (ExitException ee2) { 
                    Trace.println("systemExit: "+ee2, TraceLevel.BASIC);
                    Trace.ignoredException(ee2);
                }
            }
            return false;
        }
        return true;
    }


    /* updateFinalLaunchDesc()
     * This method is responsible for:
     * 1.) insuring that _launchDesc contains the final descriptor to be used.
     * 2.) insuring that any redirection occurs (a.jnlp -> b.jnlp -> c.jnlp)
     * 3.) (if caching) insuring that the final jnlp file is what is cached
     *     (including creating No Href style cached jnlp file)
     * 4.) returning true if the cached jnlp file has changed.
     *
     * Note: if useCacheOnly == true then we do not try to update cached resources.
     * If something is missing from cache - we will throw CacheUpdateRequiredException
     */
    private boolean updateFinalLaunchDesc(LaunchDesc ld, int count, boolean useCacheOnly)
        throws ExitException, CacheUpdateRequiredException {
        try {
            URL href = ld.getLocation();
            if (href == null) {
                // no href jnlp file -> no more re-direction
                _launchDesc = ld;
                return LaunchDownload.updateNoHrefLaunchDescInCache(ld);
            }

            String userCache = Config.getCacheDirectory();
            String systemCache = Config.getSystemCacheDirectory();
            boolean fromCache = ((_args[0] != null) && 
                ((userCache != null && _args[0].startsWith(userCache)) || 
                 (systemCache != null && _args[0].startsWith(systemCache))));

            File cachedFile = DownloadEngine.getCachedFile(href);

            // running from cache is slightly differant case, in that 
            // the given file might not be the latest, if there is an update
            if (!useCacheOnly && fromCache) {
                LaunchDesc newLD = 
                    LaunchDownload.getUpdatedLaunchDesc(href, null);
                if (newLD == null) {
                    _launchDesc = ld;
                    return false;  // no update
                }
                // note: even if identical to old, report update, because
                // we have downloaded new copy into cache, and we can't
                // allow a shortcut to point to the old (nonexistant) one
                URL newHref = newLD.getLocation();
                if ((newHref == null) || 
                    (!(newHref.toString().equals(href.toString()))) &&
                      (count == 0)) { 
                    Cache.removeCacheEntry(href, null, null);
                    return updateFinalLaunchDesc(newLD, ++count, false);
                }
                _launchDesc = newLD;
                return true;  // ok we updated it.
            }

            if (cachedFile != null) { // already cached:
                try {
                    _launchDesc = LaunchDescFactory.buildDescriptor(
                            cachedFile, LaunchDescFactory.getDerivedCodebase(),
                            LaunchDescFactory.getDocBase(), href);
                } catch (LaunchDescException lde) {
                    // try to build with docbase and codebase from LocalApplicationProperties
                    _launchDesc = LaunchDescFactory.buildDescriptor(cachedFile);
                    if (_launchDesc == null) {
                        throw lde;
                    }
                }

                byte[] ldContents = _launchDesc.getBytes();

                LaunchDesc newLD = null;
                /* if LaunchDesc is the same as on previous step 
                   - we do not need to do anything as there is no update.
                   If they differ we need to rebuild LD unless 
                   we are trying to run from cache.
                   In later case we raise exception to indicate presence of update. */
                if (!ld.hasIdenticalContent(ldContents)) {
                    if (count == 0 && useCacheOnly) {
                        throw new CacheUpdateRequiredException(
                                "Given JNLP is newer than cached copy!");
                    }
                    newLD = LaunchDownload.getUpdatedLaunchDesc(href, null);
                }                    

                if (newLD != null) {  // updated in cache
                    _launchDesc = newLD;
                    URL newHref = _launchDesc.getLocation();
                    if ((newHref == null) || 
                        (!(newHref.toString().equals(href.toString()))) &&
                          (count == 0)) { 
                        Cache.removeCacheEntry(href, null, null);
                        return updateFinalLaunchDesc(_launchDesc, ++count, useCacheOnly);
                    }
                    return true;	// we have updated it 
                } else { // was cached - no update unless redirect
                    Cache.removeRemovedApp(href.toString(), 
                        _launchDesc.getInformation().getTitle());
    
                    URL newHref = _launchDesc.getLocation();
                    if ((newHref == null) || 
                        (!(newHref.toString().equals(href.toString()))) &&
                          (count == 0)) { 
                        Cache.removeCacheEntry(href, null, null);
                        return updateFinalLaunchDesc(_launchDesc, ++count, useCacheOnly);
                    }
                    _launchDesc = ld;
                    return false; // was in cache, not updated or redirected
                }
            } else {  // not already in the cache
                if (useCacheOnly)
                    throw new CacheUpdateRequiredException(
                            "Missing from the cache: "+href);

                if (Cache.isCacheEnabled()) {
                    DownloadEngine.getResource(href, null, null, null, true);
                    cachedFile = DownloadEngine.getCachedFile(href);
                    if (cachedFile != null) {
                        _launchDesc = LaunchDescFactory.buildDescriptor(
                                          cachedFile, 
					  LaunchDescFactory.getDerivedCodebase(), 
					  LaunchDescFactory.getDocBase(), href);
                        URL newHref = _launchDesc.getLocation();
                        if ((newHref == null) || 
                            (!(newHref.toString().equals(href.toString()))) &&
                              (count == 0)) { 
                            Cache.removeCacheEntry(href, null, null);
                            return updateFinalLaunchDesc(_launchDesc, ++count, useCacheOnly);
                        }
                        return true;  // we have updated it 
                    } else {
                        // dosn't seem likely: wasn't in cache, cache enabled,
                        // we tried to download it, no exception, now still
                        // not in cache
                        throw new Exception("cache failed for" + href);
                    }
                } else {
                    // cache disabled case
                    _launchDesc = LaunchDescFactory.buildDescriptor(href, null);
                    
                    URL newHref = _launchDesc.getLocation();
                  
                    if ((newHref != null) &&
                            (!(newHref.toString().equals(href.toString()))) &&
                            (count == 0)) {
                        return updateFinalLaunchDesc(_launchDesc, ++count, useCacheOnly);
                    }
                    return false;
                }
            }
        } catch (CacheUpdateRequiredException e) {
            throw e;
        } catch ( Exception exception ) {
           throw new ExitException(exception, ExitException.LAUNCH_ERROR);
        }
    }


    private void removeTempJnlpFile(LaunchDesc ld) {
        File cachedJnlp = null;
        
        if (ld.isApplicationDescriptor()) {
            try {
                cachedJnlp = DownloadEngine.getCachedFile(ld.getCanonicalHome());
            } catch (IOException ioe) {
                Trace.ignoredException(ioe);
            }
        }
        
        if (cachedJnlp == null) return;

        if (_args != null && cachedJnlp != null &&
                JnlpxArgs.shouldRemoveArgumentFile()) {
            //remove temp file
            new File(_args[0]).delete();

            // mark removeArgumentFile to false
            JnlpxArgs.setShouldRemoveArgumentFile(String.valueOf(false));

            //replace args[0] to point to this cached jnlp file
            _args[0] = cachedJnlp.getPath();

        }
    }

    static String getCurrentJavaFXVersion() {
        // <evar16>CurrentJavaFXVersion</evar16>
        URL javafxJnlp = null;
        try {
            javafxJnlp = new URL(Pings.JAVAFX_RT_JNLP_URL);
        } catch (MalformedURLException mue) {
            // should not happen;
        }

        // lookup javafx-rt.jnlp in cache to find out current JavaFX version
        CacheEntry ce = Cache.getLatestCacheEntry(javafxJnlp, null);

        LaunchDesc javafxLD = null;
        // current JavaFX version installed on the system
        String currentJavaFXVersion = Pings.JAVAFX_UNDEFINED_PING_FIELD;
        if (ce != null) {
            try {
                javafxLD = LaunchDescFactory.buildDescriptor(
                        ce.getDataFile(), null, null, javafxJnlp);
            } catch (Exception e) {
                // should not happen
            }

            currentJavaFXVersion = javafxLD.getVersion();
        }
        return currentJavaFXVersion;
    }

    static String getRequestedJavaFXVersion(LaunchDesc preloadLD) {
        // <evar17>RequestedJavaFXVersion</evar17>
        // requested JavaFX version is from javafx-cache.jnlp
        String requestedJavaFXVersion = Pings.JAVAFX_UNDEFINED_PING_FIELD;
        if (preloadLD != null) {
            requestedJavaFXVersion = preloadLD.getVersion();
        }
        return requestedJavaFXVersion;
    }

    private void prepareAllResources(LaunchDesc ld, String[] args,
            boolean launchFileUpdated, boolean offlineOnly)
            throws ExitException, CacheUpdateRequiredException {

        ArrayList installFiles = new ArrayList();
        
        // prepare the launch file and JRE only
        boolean skipExtensions =
                prepareLaunchFile(ld, args, offlineOnly, installFiles);

        prepareSecurity(ld);

        if (!offlineOnly) {
            // prepare ClassLoader, AppPolicy, and SecurityManager
            prepareEnvironment(ld);

            // if launch file(s) indicate a custome progress implementation
            // prepare that before other downloads
            ResourcesDesc rd = ld.getResources();
            if (rd != null && !_isSilent && !_isImport && !offlineOnly) {
                JARDesc pjd = rd.getProgressJar();
                if (pjd != null) {
                    String progressClass = ld.getProgressClassName();
                    if (progressClass != null) {
                        Trace.println("Using Custome Progress jar: " + pjd,
                                TraceLevel.NETWORK);
                        prepareCustomProgress(ld, pjd, progressClass);
                    } else {
                        Trace.println("No Custom progress class found",
                                TraceLevel.NETWORK);
                    }
                } else {
                    Trace.println("No Custom Progress jar specified",
                            TraceLevel.NETWORK);
                }
            }
        } else {
            // Need to make sure AppPolicy is initialized
            //  for signed resource verification
            // See LaunchDownload.checkSignedResourcesHelper
            AppPolicy.createInstance(ld.getCanonicalHome().getHost());
        }

        // finally prepare all the other resources
        prepareResources(ld, args, launchFileUpdated, offlineOnly,
                skipExtensions, installFiles);

        if (offlineOnly) {
            //if we got here this means we are ready to start but if we were trying to start from cache only then
            //we have not initialized enviroment yet. Do it now.
            prepareEnvironment(ld);
        }
    }


    private boolean prepareLaunchFile(LaunchDesc ld, String[] args,
            boolean offlineOnly, ArrayList installFiles)
            throws ExitException, CacheUpdateRequiredException {

        boolean loadedExtensions = false;

        // Check that at least some resources was specified
        if (ld.getResources() == null) {
            handleJnlpFileException(ld, new LaunchDescException(ld,
                ResourceManager.getString("launch.error.noappresources",
                ld.getSpecVersion()), null));
        }

        if (!_isImport && ld.isLibrary()) {
            LaunchDescException lde = new LaunchDescException(ld,
                "Internal Error: !_isImport && ld.isLibrary()", null);
            handleJnlpFileException(ld, lde);
        }

        boolean isInstaller = ld.isInstaller();

        // Initialize exception with the launch descriptor
        JNLPException.setDefaultLaunchDesc(ld);
        
        JREInfo homeJREInfo = JREInfo.getHomeJRE(); // running javaws JRE
        Trace.println("Launcher: isInstaller: "+isInstaller+", isRelaunch: "+_isRelaunch+", isImport: "+_isImport+
                      ", java.home:"+Config.getJavaHome()+", Running JRE: "+
                      homeJREInfo, TraceLevel.BASIC);
        Trace.println("JREInfos", TraceLevel.BASIC);
        JREInfo.traceJREs();
        if (homeJREInfo == null) {
            // No running JRE  ??
            LaunchDescException lde = new LaunchDescException(ld,
                "Internal Error: no running JRE", null);
            handleJnlpFileException(ld, lde);
        }

        // pop up error dialog and exit if system is in offline mode and jnlp
        // file does not allow offline operation
        if (ld.getInformation().supportsOfflineOperation() == false && 
                DeployOfflineManager.isGlobalOffline() == true) {
            throw (new ExitException(new OfflineLaunchException(
                    OfflineLaunchException.NO_OFFLINE_ALLOWED), 
                    ExitException.LAUNCH_ERROR));
        }
        
        //validate everything in cache first
        //This will build LaunchDesc objects for all jnlp files from saved xml
        //and these objects will be reused later on
        //(in particular by JRE matching code)
	PerfLogger.setTime("Begin LaunchDownload.isInCache(ld)");

        if (ld.getUpdate().isBackgroundCheck() && (_lap == null ||
                !_lap.forceUpdateCheck())) {
             //otherwise we need update check and it has no sense
             // to try to load files from cache now
            _isCached =  LaunchDownload.isInCache(ld);
        }
	PerfLogger.setTime("End LaunchDownload.isInCache(ld)");

        if (!_isCached && !offlineOnly) {
            try {
                LaunchDownload.downloadExtensions(ld, null, 0, installFiles);
                loadedExtensions = true;
            } catch (Exception ioe) {
                if (!ld.getInformation().supportsOfflineOperation() ||
                        !LaunchDownload.isInCache(ld)) {
                    throw (new ExitException(ioe, ExitException.LAUNCH_ERROR));
                }
                Trace.ignoredException(ioe);
            }
        } else {
            loadedExtensions = _isCached;
        }
        //loadedExtensions can be false only if one the following is true
        //  a) we failed to load extensions
        //  b) offline mode was requested and not everything is in cache
        //In both cases we need to check if we are allowed to start offline
        // and doublecheck cache state before we can proceed.
        if (!loadedExtensions) {
            if (!ld.getInformation().supportsOfflineOperation() ||
                    !LaunchDownload.isInCache(ld)) {
                 throw new CacheUpdateRequiredException("Some of required resources are not cached.");
            }
            loadedExtensions = true;
        }

        LaunchSelection.MatchJREIf jreMatcher = ld.getJREMatcher();
        JVMParameters jvmParams = jreMatcher.getSelectedJVMParameters();

        // Get the JREInfo of the selected and installed JRE
        _jreInfo = jreMatcher.getSelectedJREInfo();

        // Get the JREDesc of the selected JRE
        JREDesc jreDesc = jreMatcher.getSelectedJREDesc();

        // Did MatchJREIf it's job ?
        if ( (_jreInfo==null && jreDesc == null) || null==jvmParams ) {
            Trace.println(jreMatcher.toString());
            LaunchDescException lde = new LaunchDescException(ld,
                "Internal Error: Internal error, jreMatcher uninitialized", null);
            handleJnlpFileException(ld, lde);
        }

        // Did LaunchDesc and MatchJREIf select a proper JRE ?
        if (!ld.isJRESpecified()) {
            LaunchDescException lde = new LaunchDescException(ld,
                "Internal Error: !isJRESpecified()", null);
            handleJnlpFileException(ld, lde);
        }

        // Get location and LocalApplication Properties for JNLP file
        // Descriptor home is either the <jnlp href="..."> attribute
        // or the URL for the main JAR file.
        // If neither is specified, we signal an error
        URL jnlpUrl = ld.getCanonicalHome();
        if (jnlpUrl == null) {
           LaunchDescException lde = new LaunchDescException(ld,
                   ResourceManager.getString("launch.error.nomainjar"), null);
            throw (new ExitException(lde, ExitException.LAUNCH_ERROR));
        }
        if (isInstaller) {
            // An extension should always be invoked by a a file entry that
            // points to the cache. We can get the URL and version id from it.
            _lap = Cache.getLocalApplicationProperties(args[0]);

            // An extension should also always be invoked with -installer
            if (_lap == null || !Environment.isInstallMode()) {
                handleJnlpFileException(ld, new MissingFieldException(
                    ld.getSource(), "<application-desc>|<applet-desc>"));
            } 
            jnlpUrl = _lap.getLocation();
        } else if (!ld.isLibrary()) {
            _lap = Cache.getLocalApplicationProperties(jnlpUrl); 
        }

        Trace.println("LaunchDesc location: " + jnlpUrl, TraceLevel.BASIC);
     
        boolean offlineMode = _isCached && 
                DeployOfflineManager.isGlobalOffline();

        // see if a JRE shall be installed
        if (!_isImport && (_jreInfo == null)) {
            if (offlineOnly)
                throw new CacheUpdateRequiredException("Need to install JRE");

            String pref = Config.getProperty(
                Config.JAVAWS_JRE_AUTODOWNLOAD_KEY);

            if ((pref != null) && (pref.equalsIgnoreCase(
                                Config.AUTODOWNLOAD_MODE_NEVER))) {
                // config prevented downloading requested jre
                throw (new ExitException(new NoLocalJREException(ld, 
                    jreDesc.getVersion(), false), 
                        ExitException.LAUNCH_ERROR));
            }
            String source = jreDesc.getSource();
            URL location = jreDesc.getHref();

            // find out what will be downloaded.
            boolean isPlatformVersion = (location == null);
            if (isPlatformVersion) {
                String defaultURL = Config.getProperty(
                    Config.JAVAWS_JRE_INSTALL_KEY);
                try {
                    location = new URL(defaultURL);
                } catch (MalformedURLException mue) {
                    throw new ExitException(mue, 
                        ExitException.LAUNCH_ERROR);
                }
            }
            
            String replyVersion = DownloadEngine.getAvailableVersion(
                location, jreDesc.getVersion(), isPlatformVersion, 
                JREInfo.getKnownPlatforms());
        
            // if nothing available to download - exit
            if (replyVersion == null) {
                replyVersion = jreDesc.getVersion();
            }
            // Secure Static Versioning for download an older JRE
            if (SecureStaticVersioning.promptRequired(
                        ld, _lap, true, replyVersion)) {
                if (SecureStaticVersioning.promptDownload(null, ld, 
                    _lap, replyVersion, source) == false) {
                    // user chose not to download the required JRE
                    throw (new ExitException(null, ExitException.OK));
                }
            } else if ((pref != null) && (pref.equalsIgnoreCase(
                                Config.AUTODOWNLOAD_MODE_PROMPT))) {
                if (AutoDownloadPrompt.prompt(null, ld) == false) {
                    // user chose not to download the required JRE
                    throw (new ExitException(null, ExitException.OK));
                }
            }
        } else if (!_isImport) {
            // Secure Static Versioning, use an existing jre
            if (SecureStaticVersioning.promptRequired(
                    ld, _lap, false, _jreInfo.getProduct())) {
                if (SecureStaticVersioning.promptUse(
                    null, ld, _lap, _jreInfo) == false) {
                    // version not allowed
                    throw (new ExitException(null, ExitException.OK));
                }
            }
        }
        return loadedExtensions;
    }

    private void prepareEnvironment(LaunchDesc ld) 
        throws ExitException {
        // prepare the AppPolicy object (user to create the ClassLoader)
        AppPolicy policy = AppPolicy.createInstance(
                                ld.getCanonicalHome().getHost());
        // prepare the ClassLoader
        _jnlpClassLoader = JNLPClassLoader.createClassLoader(ld, policy);

        // prepare the JNLP API
        try {
            // default is non-file codebase protocol
            String codebaseProtocol = "http";

            // use cannonicalHome because codebase maybe null
            URL canonicalHome = ld.getCanonicalHome();
            if (canonicalHome.getProtocol().equalsIgnoreCase("file")) {
                // codebaseProtocol is file only if file url does not have
                // a hostname
                if (canonicalHome.getHost().equals("")) {
                    codebaseProtocol = "file";
                }
            }

            BasicServiceImpl.initialize(ld.getCodebase(),
                                        BrowserSupport.isWebBrowserSupported(),
                                        codebaseProtocol);

            // Setup ExtensionInstallation Service if needed
            if (ld.getLaunchType() == LaunchDesc.INSTALLER_DESC_TYPE) {
                String installDir = _lap.getInstallDirectory();
                if (installDir == null) {
                    installDir = Cache.getNewExtensionInstallDirectory();
                    _lap.setInstallDirectory(installDir);
                }
                // Setup the install context
                // for sandbox application,
                // ExtensionInstallerService.getInstallPath should return
                // null
                ExtensionInstallerServiceImpl.initialize(
                        ld.isSecure() ? null : installDir, _lap,
                        _downloadWindowHelper.getProgressListener());
            }
        } catch (Throwable t) {
            throw new ExitException(t, ExitException.LAUNCH_ERROR);
        }

        // prepare and set the SecurityManager
        System.setSecurityManager(new JavaWebStartSecurity());
    }

    private void prepareResources(LaunchDesc ld, String[] args,
            boolean launchFileUpdated, boolean offlineOnly,
            boolean skipExtensions, ArrayList installFiles)
            throws ExitException, CacheUpdateRequiredException {
   
        PerfLogger.setTime("Begin UpdateCheck");
        boolean isInstaller = ld.isInstaller();
        boolean offlineMode = _isCached && 
                DeployOfflineManager.isGlobalOffline();

        // Need to update if not offline and either:
        //  - the application is not cached
        //  - no local JRE is found
        //  - the preferences is to always do the check
        //  - the application properties says that we should do an update
        //  - Installer needs progress window
        boolean forceUpdateIfCached = (!_isImport && (_jreInfo == null)) ||
            (isInstaller);
        boolean forceUpdate =
            (!_isCached)  || forceUpdateIfCached;        
        
        if (forceUpdate && offlineMode) {
            throw (new ExitException(new OfflineLaunchException(
                    OfflineLaunchException.MISSING_RESOURCE),
                    ExitException.LAUNCH_ERROR));
        }

        boolean needUpdate = forceUpdate;

        // Need to update if we are not in the offline mode
        // and check is forced by LAP file
        // or we can check online and we found that updates are availble
        if (!offlineMode) {
            // check if any updates need checked in the FOREGROUND

            // the force update check flag is set by UpdateChecker if
            // the last update check timed out or cancelled or error happens
            if (_lap != null && _lap.forceUpdateCheck()) {
                Trace.println("Forced update check in LAP, do full update", TraceLevel.BASIC);
                needUpdate = true;
            } else {
                // If JNLP update policy is prompt-update or prompt-run, we
                // need to do update check now so we can determine if
                // we need to show update prompt
                if (!offlineOnly && !ld.getUpdate().isBackgroundCheck() &&
                        ld.getUpdate().getPolicy() == UpdateDesc.POLICY_ALWAYS) {
                    needUpdate = true;
                } else if (!offlineOnly) {
                    try {
                        needUpdate = ld.getUpdater().isUpdateAvailable();
                    } catch (Exception e) {
                        throw (new ExitException(e, ExitException.LAUNCH_ERROR));
                    }

                    if (needUpdate && ld.getUpdate().getPolicy() !=
                            UpdateDesc.POLICY_ALWAYS) {
                        // update forceUpdate value to trigger update prompt if
                        // necessary
                        _isCached = LaunchDownload.isInCache(ld);
                        if (_isCached) {
                            forceUpdate = forceUpdateIfCached;
                        }
                    }

                    if (ld.getUpdater().isCheckAborted()) {
                        throw (new ExitException(new LaunchDescException(ld, "User rejected cert - aborted", null),
                                ExitException.LAUNCH_ABORT_SILENT));
                    }
                }
            }
        }

        Trace.println(
                "Offline mode: " + offlineMode +
                "\nIsInCache: " + _isCached +
                "\nforceUpdate: " + forceUpdate +
                "\nneedUpdate: " + needUpdate +
                "\nIsInstaller: " + isInstaller, TraceLevel.BASIC);

        if (needUpdate && !forceUpdate) {
            // for updating other resources - depends on the policy
            forceUpdate = ld.getUpdater().needUpdatePerPolicy(_downloadWindowHelper);
        }

        if (forceUpdate && offlineMode) {
            throw (new ExitException(new OfflineLaunchException(
                    OfflineLaunchException.MISSING_RESOURCE),
                    ExitException.LAUNCH_ERROR));
        }

        //
        // Download all resources
        //

        // Keep track of all JNLP files for installers

        if (forceUpdate) {

            if (!_isImport && _jreInfo == null) {
                downloadJREResource(ld, installFiles);
                if (!installFiles.isEmpty()) {
                    // execute the jre installer
                    JnlpxArgs.executeInstallers(installFiles, 
                         _downloadWindowHelper.getProgressListener());
                    // if we just installed a newer jre, and the current 
                    // java web start is incapable of handling the 
                    // specification version required by the app, we try the 
                    // default version of java on this platform (which may be 
                    // the new one just installed.)
                    if (!isValidSpecificationVersion(ld)) {
                        Config.getInstance().resetJavaHome();
                    }
                }
                String jreBinPath = Config.getJavaHome() + File.separator + 
                                        "bin" + File.separator;
                Config.getInstance().notifyJREInstalled(jreBinPath);
                
                _jreInstalled = true;
            } else {
               
                // postpone downloading resource till you have the JRE
                try {
                    downloadResources(ld, installFiles, skipExtensions, isInstaller);
                } catch (ExitException ee) {
                    // something went wront during JavaFX jnlps/JARs download
                    // send JavaFX install completed ping with error information
                    // re-throw exception
                    if (Environment.isJavaFXInstallInitiated()) {
                        Throwable t = ee.getException();
                        if (t instanceof FailedDownloadException) {
                            String url =
                                    ((FailedDownloadException)t).getLocation().toString();
                            Pings.sendJFXPing(
                                    Pings.JAVAFX_INSTALL_COMPLETED_PING,
                                    getCurrentJavaFXVersion(),
                                    getRequestedJavaFXVersion(ld),
                                    Pings.JAVAFX_RETURNCODE_DOWNLOAD_FAILED_FAILURE,
                                    url);
                        } else {
                            Pings.sendJFXPing(
                                    Pings.JAVAFX_INSTALL_COMPLETED_PING,
                                    getCurrentJavaFXVersion(),
                                    getRequestedJavaFXVersion(ld),
                                    Pings.JAVAFX_RETURNCODE_UNKNOWN_FAILURE,
                                    null);
                        }
                    }
                    throw ee;
                }
            }

            // Reset force update
            if (_lap != null && _lap.forceUpdateCheck()) {
                _lap.setForceUpdateCheck(false);
                try { _lap.store(); }
                catch(IOException ioe) { Trace.ignoredException(ioe); }
            }
        } else {
            PerfLogger.setTime("End UpdateCheck - Nothing to update");
        }

        //if custom progress is used we need to wait for it to be loaded
        //before we can go and validate JNLP signing constrains
        //(progress jar == "super eager")
        CustomProgress cp = Progress.getCustomProgress();
        if (cp != null) {
            //make sure we deliver 100% completion event even if we
            // did not load anything.
            //NB: we are sending it in case of error too!
            //    should we change it to notify on load failure?
            cp.validating(ld.getLocation(), null, 1, 1, 100);

            //NB: this call will thow exception on error
            try {
                cp.waitTillLoaded();
            } catch (JNLPException je) {
                throw (new ExitException(je, ExitException.LAUNCH_ERROR));
            } catch (IOException ioe) {
                if (ld.getInformation().supportsOfflineOperation()
                        && LaunchDownload.isInCache(ld, skipExtensions)) {
                    Trace.ignoredException(ioe);
                } else {
                    throw (new ExitException(ioe, ExitException.LAUNCH_ERROR));
                }
            }
        }

        // most everything else waits till we have a JRE unless just importing
        if (_jreInfo != null || _isImport) {
        
            com.sun.javaws.ui.SplashScreen.generateCustomSplash(null, ld, 
                launchFileUpdated);

            //
            // Run all installers if any
            //
            if (!_isImport && !installFiles.isEmpty()) {
                if (isInstaller) {
                    // FIXIT: Installers should not have installers
                }
                JnlpxArgs.executeInstallers(installFiles, 
                    _downloadWindowHelper.getProgressListener());
            }

            // Let progress window show launching behavior
            if (!_isSilent) {
                String title = "progress.title.app";
                if (ld.getLaunchType() == LaunchDesc.INSTALLER_DESC_TYPE) {
                    title = "progress.title.installer";
                }
                _downloadWindowHelper.showLaunchingApplication(title);
            }
        }

        LaunchSelection.MatchJREIf jreMatcher = ld.getJREMatcher();
        JVMParameters jvmParams = jreMatcher.getSelectedJVMParameters();

        // Get the JREInfo of the selected and installed JRE
        _jreInfo = jreMatcher.getSelectedJREInfo();

        // Get the JREDesc of the selected JRE
        JREDesc jreDesc = jreMatcher.getSelectedJREDesc();

        // re-validate the JNLP 
        if (!_isImport) {
            // check that the JNLP Spec version is valid
            if (!isValidSpecificationVersion(ld)) {
                JNLPException.setDefaultLaunchDesc(ld);
                handleJnlpFileException(ld, new LaunchDescException(ld,
                    ResourceManager.getString("launch.error.badjnlversion",
                        ld.getSpecVersion()), null));
            }
        
            // Detemine what JRE to use, we have not found want yet
            if (_jreInfo == null) {
                // Reread config properties with information about new JREs
                Config.refreshProps();
                // try to re-match ..
                _jreInfo = ld.selectJRE();
                if (_jreInfo == null) {
                    Trace.println("No JREInfo(1): "+ld.getJREMatcher());
                    LaunchDescException lde = new LaunchDescException(ld,
                        ResourceManager.getString(
                                        "launch.error.missingjreversion"),
                        null);
                    throw (new ExitException(lde, ExitException.LAUNCH_ERROR));
                }
                jreMatcher = ld.getJREMatcher();
                jreDesc = jreMatcher.getSelectedJREDesc();
                jvmParams = jreMatcher.getSelectedJVMParameters();
            }

            // Running on wrong JRE or if we did an install, then relaunch JVM
            boolean homeJVMMatch = jreMatcher.isRunningJVMSatisfying(true);

            if (Trace.isTraceLevelEnabled(TraceLevel.BASIC)) {
                Trace.println("_jreInstalled:    "+_jreInstalled, TraceLevel.BASIC);
                Trace.println(jreMatcher.toString(), TraceLevel.BASIC);
            }
            if( _jreInstalled && homeJVMMatch) {
                throw new ExitException(new Exception("Internal Error: jreInstalled, but homeJVM matches"), ExitException.LAUNCH_ERROR);
            }

            // Always relaunch if 
            // a new jre installed or if the running JVM does not satisfy the desired one
            if ( ld.isSecureJVMArgs() && !homeJVMMatch && (_jreInfo != null)) {
                if(!_isRelaunch) {
                    long minHeap = jreDesc.getMinHeap();
                    long maxHeap = jreDesc.getMaxHeap();

                    // JRE is installed. Launch separate process for this JVM
                    try {
                        args = insertApplicationArgs(args);
                        JnlpxArgs.execProgram(_jreInfo, args, minHeap, maxHeap, jvmParams, false);
                    } catch(IOException ioe) {
                        throw new ExitException(new JreExecException(
                            _jreInfo.getPath(), ioe), ExitException.LAUNCH_ERROR);
                    }

                    // do not remove tmp file if it is a relaunch
                    if (JnlpxArgs.shouldRemoveArgumentFile()) {
                        JnlpxArgs.setShouldRemoveArgumentFile(
                            String.valueOf(false));
                    }
                    throw new ExitException(null, ExitException.OK);
                } else {
                    Trace.println("JAVAWS: Relaunch ignored(1): relaunched already", TraceLevel.BASIC);
                }
            }
        }

        // Remove argument file if neccesary
        JnlpxArgs.removeArgumentFile(args[0]);
        PerfLogger.setTime("End removeArgumentFile");

        if (_isImport) {
            _downloadWindowHelper.disposeWindow();
            // if importing, just do desktopIntegration and exit

            // only really updated if jnlp file is now in the cache
            boolean updated =  
                LaunchDownload.isJnlpCached(ld) && launchFileUpdated;
            notifyLocalInstallHandler(ld, _lap, _isSilent, updated, null);

            // if JavaFX download had taken place, send JavaFX install
            // completed ping
            if (Environment.isJavaFXInstallInitiated()) {
                Pings.sendJFXPing(Pings.JAVAFX_INSTALL_COMPLETED_PING,
                        getCurrentJavaFXVersion(),
                        getRequestedJavaFXVersion(ld),
                        Pings.JAVAFX_RETURNCODE_SUCCESS, null);
            }

            preverifyImportedJARs(ld);
            
            Trace.println("Exiting after import", TraceLevel.BASIC);
            throw new ExitException(null, ExitException.OK);
        }

        // Show the console if necessary.
        final String titleSuffix = " - " + ld.getInformation().getTitle();
              SwingUtilities.invokeLater(new Runnable() {
                   public void run() {
                      JavawsConsoleController.setTitle(
                              "console.caption", titleSuffix);
                      JavawsConsoleController.showConsoleIfEnable();
                   }
        });

        boolean allSigned = false;

        try {
            // Check signing of all JNLP files
            PerfLogger.setTime("Begin checkSignedLaunchDesc");

            LaunchDownload.checkSignedLaunchDesc(ld);
            PerfLogger.setTime("End checkSignedLaunchDesc");

            // Check is resources in each JNLP file is signed and prompt for
            // certificates. This will also check that each resources downloaded
            // so far is signed by the same certificate.
            allSigned = LaunchDownload.checkSignedResources(ld);

            // allSigned now means main jnlp file also
            allSigned = allSigned && ld.isSigned();
        }  catch(JNLPException je) {
            throw new ExitException(je, ExitException.LAUNCH_ERROR);
        } catch(IOException ioe) {
            // This should be very uncommon
            throw new ExitException(ioe, ExitException.LAUNCH_ERROR);
        }

        if (Trace.isTraceLevelEnabled(TraceLevel.BASIC)) {
            Trace.println("passing security checks; secureArgs:"+
              ld.isSecureJVMArgs()+", allSigned:"+allSigned, TraceLevel.BASIC);
            Trace.println("trusted app: "+!ld.isSecure()+", -secure="+
              Globals.isSecureMode(), TraceLevel.BASIC);
            Trace.println(jreMatcher.toString(), TraceLevel.BASIC);
        }
        // match jre and secure args if not signed, or all args if allSigned.
        boolean homeJVMMatch = jreMatcher.isRunningJVMSatisfying(allSigned);

        // if app is trusted, and the running JVM does not satisfy the desired 
        // one, relaunch using all the specified vm args and properties.
        if (!homeJVMMatch) {
            if(!_isRelaunch) {
                long minHeap = jreDesc.getMinHeap();
                long maxHeap = jreDesc.getMaxHeap();

                try {
                    args = insertApplicationArgs(args);
                    JnlpxArgs.execProgram(_jreInfo, args, minHeap, maxHeap, jvmParams, allSigned);
                } catch(IOException ioe) {
                    throw new ExitException(new JreExecException(
                        _jreInfo.getPath(), ioe), ExitException.LAUNCH_ERROR);
                }

                // do not remove tmp file if it is a relaunch
                if (JnlpxArgs.shouldRemoveArgumentFile()) {
                    JnlpxArgs.setShouldRemoveArgumentFile(
                        String.valueOf(false));
                }
                throw new ExitException(null, ExitException.OK);
            } else {
                Trace.println("JAVAWS: Relaunch ignored(2): relaunched already", TraceLevel.BASIC);
            }
        }

        // Check for desktop integration
        // only do desktop integration if cache enabled and shortcut does not
        // exists yet or icon image updated
        if (Cache.isCacheEnabled() &&
                (LocalInstallHandler.getInstance().isShortcutExists(_lap) ==
                false || Globals.isIconImageUpdated())) {                        
            notifyLocalInstallHandler(ld, _lap, _isSilent,
                    launchFileUpdated || Globals.isIconImageUpdated(),
                    _downloadWindowHelper.getOwnerRef());
            if (Globals.isIconImageUpdated()) {
                Globals.setIconImageUpdated(false);
            }
        }
        Trace.println("continuing launch in this VM", TraceLevel.BASIC);
    }

    private void prepareCustomProgress(final LaunchDesc ld, 
            final JARDesc progressJD, final String progressClassName) 
            throws ExitException {
   
        // start with empty implementation - set listener later.
        final CustomProgress cp = new CustomProgress();
        Progress.setCustomProgress(cp);

        Thread thread = new Thread(new Runnable() {
            public void run() {
                try {
                    /* We want to explicitly load Custom progress
                     * implementation for several reasons:
                     *    - loading on demand by classloader may cause double
                     *       loading (as URLClassloader will not save it to deploy
                     *       cache in the webstart mode)
                     *    - we do not want to report download progress for
                     *      progress jars themselves (can ot be visualized)
                     *    - Multiple progress jars will be loaded concurrently
                     */
                    LaunchDownload.downloadProgressJars(ld);
                    Class progressClass = 
                          _jnlpClassLoader.loadClass(progressClassName);
                    final Class [] noArgs = new Class [0];
                    final Constructor progressConstructor = 
                        progressClass.getConstructor(noArgs);
                    /*
                     * We need to run constructor on a new Thread in the 
                     * Applications ThreadGroup.
                     */
                    final Object[] results = new Object[1];
                    Runnable constructor = new Runnable() {
                        public void run() {
                            results[0] = null;
                            try {
                                results[0] = (Object)
                                    progressConstructor.newInstance(noArgs);
                            } catch (Exception e) {
                             // InstantiationException, IllegalAccessException, 
                             // or InvocationTargetException can be thrown here
                            }
                        }
                    };
                    ThreadGroup threadGroup = Main.getLaunchThreadGroup();
                    Thread constructorThread = (new Thread(threadGroup,
                               constructor, "progressMain"));
                    constructorThread.start();
                    try {
                        constructorThread.join(5000);
                        DownloadServiceListener dsl =
                           (DownloadServiceListener)results[0];
                        cp.setAppThreadGroup(threadGroup);
                        cp.setListener(dsl);
                        com.sun.javaws.ui.SplashScreen.hide();
                        Trace.println("Custom Progress class setup OK",
                                      TraceLevel.BASIC);
                    } catch (InterruptedException ie) {
                        Trace.println("Custom Progress class not constructed",
                                      TraceLevel.BASIC);
                        Trace.ignored(ie);
                    } catch (ClassCastException cce) {
                        Trace.println("CustomProgress class is not in " + 
                            "an  mplementation of DownloadServiceListener", 
                            TraceLevel.BASIC);
                    }
                } catch (Exception eee) {
                    Trace.println("Error generating custom progress : " + eee);
                    Trace.ignored(eee);
                } finally {
                    // if anything has gone wrong, we need to restore default 
                    // DownloadWindow as the progress listener.
                    if (cp.getListener() == null) {
                        Progress.setCustomProgress(null);
                    }
                }
            }
        });
        thread.start();
    }

    private void preverifyImportedJARs(LaunchDesc ld) {
        // Only preverify FX runtime JARs during import or au for now
        if (Environment.getJavaFxInstallMode() ==
                Environment.JAVAFX_INSTALL_PRELOAD_INSTALLER ||
                Environment.getJavaFxInstallMode() ==
                Environment.JAVAFX_INSTALL_AUTOUPDATE) {

            try {
                LaunchDownload.checkSignedResources(ld);
            } catch (JNLPException e) {
                Trace.ignoredException(e);
            } catch (ExitException e) {
                Trace.ignoredException(e);
            } catch (IOException e) {
                Trace.ignoredException(e);
            }

            // preverify classes in JavaFX runtime JARs
            PreverificationClassLoader pcl = new PreverificationClassLoader(
                    ClassLoader.getSystemClassLoader());
            pcl.initialize(ld);
            pcl.preverifyJARs();
        }
    }

    private boolean isValidSpecificationVersion(LaunchDesc ld) {
        VersionString version = new VersionString(ld.getSpecVersion());
        /* now supports 6.0.18, 6.0.10, 6.0, 1.5, and 1.0 jnlp formats */
        return (version.contains(new VersionID("6.0.18")) ||
                version.contains(new VersionID("6.0.10")) ||
                version.contains(new VersionID("6.0")) ||
                version.contains(new VersionID("1.5")) ||
                version.contains(new VersionID("1.0")));
    }

    private String [] insertApplicationArgs (String [] args) {
        String [] progArgs = Globals.getApplicationArgs();
        if (progArgs == null) {
            return args;
        }
        String [] ret = new String [progArgs.length + args.length];
        int i, j;
        for (i=0; i<progArgs.length; i++) {
            ret[i] = progArgs[i];
        }
        for (j=0; j<args.length; j++) {
            ret[i++] = args[j];
        }
        return ret;
    }


    /*
     * doLaunchApp - now running on the Applications Thread -
     *             - Launch The Application, Applet, or Installer.
     */
    private void doLaunchApp() throws ExitException {

        // Set context classloader to the JNLP classloader
        final ClassLoader cl = _jnlpClassLoader;
        Thread.currentThread().setContextClassLoader(cl);

        // set the context class loader of the AWT EventQueueThread.
        try {
            SwingUtilities.invokeAndWait(new Runnable() {
                public void run() {
                    try {
                        Thread.currentThread().setContextClassLoader(cl);
                    } catch (Throwable t) {
                        Trace.ignored(t);
                    }
                }
            });
        } catch (InterruptedException ignore) {
            Trace.ignoredException(ignore);
        } catch (InvocationTargetException ignore) {
            Trace.ignoredException(ignore);
        }

        // Load main class
        String className = null;
        Class mainClass= null;
        try {
            className = LaunchDownload.getMainClassName(_launchDesc, true);

            Trace.println("Main-class: " + className, TraceLevel.BASIC);

            if (className == null) {
                throw new ClassNotFoundException(className);
            }
            // Lookup class
            PerfLogger.setTime("Begin load main class ");
            mainClass = cl.loadClass(className);
            PerfLogger.setTime("End load main class ");

            if (this.getClass().getPackage().equals(mainClass.getPackage())) {
                throw new ClassNotFoundException(className);
            }
       
            ClassLoader clUsed = mainClass.getClassLoader();
  
            // make sure the main-class is loaded by the JNLPClassLoader or
            // JNLPPreverifyClassLoader. Also, if a Trusted-Library
	    // JNLPClassLoader parent is configured it will be the
	    // immediate child of JNLPPreverifyClassLoader. Otherwise,
	    // the regular JNLPClassLoader will be the immediate child.
            if (clUsed != cl && clUsed != cl.getParent() && clUsed !=
                    ((JNLPClassLoader) cl).getJNLPPreverifyClassLoader()) {
                SecurityException se = new SecurityException(
                        "Bad main-class name");
                throw new ExitException(se, ExitException.LAUNCH_ERROR);
            }
        } catch(ClassNotFoundException cnfe) {
            throw new ExitException(cnfe, ExitException.LAUNCH_ERROR);
        } catch(IOException ioe) {
            throw new ExitException(ioe, ExitException.LAUNCH_ERROR);
        } catch(JNLPException je) {
            throw new ExitException(je, ExitException.LAUNCH_ERROR);
        } catch(Exception e) {
            throw new ExitException(e, ExitException.LAUNCH_ERROR);
        } catch(Throwable t) {
            throw new ExitException(t, ExitException.LAUNCH_ERROR);
        }

        // launch the application. 
        try {
            _downloadWindowHelper.disable(); // Reset so loading delegates will be disabled

            // Execute main class
            if (Globals.TCKHarnessRun) {
                Main.tckprintln(Globals.JNLP_LAUNCHING);
            }
            PerfLogger.setTime("calling executeMainClass ...");
            executeMainClass(_launchDesc, _lap, mainClass, _downloadWindowHelper);
        
        } catch(SecurityException se) {
            // This would be an application-level security exception
            throw new ExitException(se, ExitException.LAUNCH_ERROR);
        } catch(IllegalAccessException iae) {
            throw new ExitException(iae, ExitException.LAUNCH_ERROR);
        } catch(IllegalArgumentException iae) {
            throw new ExitException(iae, ExitException.LAUNCH_ERROR);
        } catch(InstantiationException ie) {
            throw new ExitException(ie, ExitException.LAUNCH_ERROR);
        } catch(InvocationTargetException ite) {
            Exception e = ite;
            Throwable t = ite.getTargetException();
            if (t instanceof Exception) {
                e = (Exception) ite.getTargetException();
            } else {
                t.printStackTrace();
            }
            throw new ExitException(e, ExitException.LAUNCH_ERROR);
        } catch(NoSuchMethodException nsme) {
            throw new ExitException(nsme, ExitException.LAUNCH_ERROR);
        } catch (Throwable t) {
            throw new ExitException(t, ExitException.LAUNCH_ERROR);
        }
        if (_launchDesc.getLaunchType() == LaunchDesc.INSTALLER_DESC_TYPE) {
            throw new ExitException(null, ExitException.OK);
        }
    }

    private boolean _shownDownloadWindow = false;

    private void downloadJREResource(LaunchDesc ld, ArrayList installFiles)
                                     throws ExitException {

        // Show loading progress window. We only do this the first time
        if (!_shownDownloadWindow && !_isSilent) {
            _shownDownloadWindow = true;
            _downloadWindowHelper.showLoadingProgressScreen();
            _downloadWindowHelper.setAllowVisible(true);
            _downloadWindowHelper.setVisible(true);
        }
        // Download jre jnlp file
        try {
            if (Cache.isCacheEnabled()) {
                LaunchDownload.downloadJRE(ld, 
                    _downloadWindowHelper.getProgressListener(), installFiles);
            } else {
                throw new IOException("Cache disabled, cannot download JRE");
            }
        } catch(SecurityException se) {
            // This error should be pretty uncommon. Most would have already 
            // been wrapped in a JNLPException by the downloadJarFiles method.
            throw (new ExitException(se, ExitException.LAUNCH_ERROR));
        } catch(JNLPException je) {
            throw (new ExitException(je, ExitException.LAUNCH_ERROR));
        } catch(IOException ioe) {
            // if you fail to download a jre - use a differant error
	    Trace.ignored(ioe);
            throw (new ExitException(new NoLocalJREException(ld,
                ld.getResources().getSelectedJRE().getVersion(),
                    false), ExitException.LAUNCH_ERROR));
        }
    }

    private void downloadResources(LaunchDesc ld, ArrayList installFiles,
            boolean skipExtensions, boolean isInstaller) throws ExitException {

        // allow progress window to show when actual download happens
        if (!_shownDownloadWindow && !_isSilent) {
            _shownDownloadWindow = true;
            _downloadWindowHelper.showLoadingProgressScreen();
            _downloadWindowHelper.setAllowVisible(true);
            if (isInstaller) {
                _downloadWindowHelper.setVisible(true);
            }
        }

        // Download all needed resources
        try {
            //We MUST AVOID creation of multiple clones of LaunchDesc
            // after classloader is created.
            //LaucnDesc carry important info on security validation status
            //Note that while we are doing good job and updating extension descriptors
            // when we loading extensions it is not enough.
            //Security permission reguests will go through classloader to find
            // jar descriptor and jnlp descriptor it belongs too.
            //And this will return "previous" copy of LaunchDescriptor!
            //
            // NB: we need to make sure we do not loading multiple copies
            //     and might be also fix JarDesc in the classloader at the time
            //     security validation is perfromed
            // NB: exactly same problem is applicable to webstart code
            //      (see JNLP2Manager.java)
            if (!skipExtensions)
              LaunchDownload.downloadExtensions(ld,
                _downloadWindowHelper.getProgressListener(), 0, installFiles);

            // Check that the JNLP restrictions given in the spec. Applications
            // with sandbox security is only allowed to referer back to a 
            // specific host and not use native libraries
            LaunchDownload.checkJNLPSecurity(ld);

            // Download all eagerly needed resources (what is in the cache 
            // may not be ok, should do a check to the server)
            LaunchDownload.downloadEagerorAll(ld, false, 
                 _downloadWindowHelper.getProgressListener(), false);
        } catch(SecurityException se) {
            // This error should be pretty uncommon. Most would have already 
            // been wrapped in a JNLPException by the downloadJarFiles method.
            throw (new ExitException(se, ExitException.LAUNCH_ERROR));
        } catch(JNLPException je) {
            throw (new ExitException(je, ExitException.LAUNCH_ERROR));
        } catch(IOException ioe) {
            if (ld.getInformation().supportsOfflineOperation() &&
                    LaunchDownload.isInCache(ld, skipExtensions)) {
                Trace.ignoredException(ioe);
            } else {
                throw (new ExitException(ioe, ExitException.LAUNCH_ERROR));
            }
        }
    }


    /*
     * note - leaving same code above as well for now, check this up front,
     * before creating custom progress - exit if this throws exception
     */
    public void prepareSecurity(LaunchDesc ld) throws ExitException {
        try {
            // Check that the JNLP restrictions given in the spec. Applications
            // with sandbox security is only allowed to referer back to a 
            // specific host and not use native libraries
            LaunchDownload.checkJNLPSecurity(ld);
        } catch(SecurityException se) {
            // This error should be pretty uncommon. Most would have already 
            // been wrapped in a JNLPException by the downloadJarFiles method.
            throw (new ExitException(se, ExitException.LAUNCH_ERROR));
        } catch(JNLPException je) {
            throw (new ExitException(je, ExitException.LAUNCH_ERROR));
        }
    }

    /**
     * This invokes <code>installIfNecessaryFromLaunch</code> on
     * the LocalInstallHandler. This will also update the state of
     * the <code>LocalApplicationProperties</code>.
     */
    public static void notifyLocalInstallHandler(LaunchDesc ld,
                LocalApplicationProperties lap, boolean silent, 
                boolean updated, ComponentRef ownerRef) {
        if (lap == null) return;
	URL codebase = LaunchDescFactory.getDerivedCodebase();
        if (codebase != null) {
            lap.setCodebase(codebase.toString());
        }
        lap.setLastAccessed(new Date());
        // fix for 5022115
        // do not increment launch count for import mode
        if (!_isImport) {
            lap.incrementLaunchCount();
        }

        // try to install on the local system
        LocalInstallHandler lih = LocalInstallHandler.getInstance();
        if (lih != null) {
            lih.install(ld, lap, updated, silent, ownerRef);
        }
        // no else required; no local install handler

        // Save the LocalApplicationProperties state.
        try {
            lap.store();
        } catch (IOException ioe) {
            // We could warn the user
            Trace.println("Couldn't save LAP: " + ioe, TraceLevel.BASIC);
        }
    }

    /** Executes the mainclass */
    private void executeMainClass(LaunchDesc ld, LocalApplicationProperties lap,
            Class mainclass, DownloadWindowHelper dw)
            throws IllegalAccessException, InstantiationException,
            InvocationTargetException, NoSuchMethodException {

        if (ld.getLaunchType() == LaunchDesc.APPLET_DESC_TYPE) {

	    String codebase = null;
	    String documentBase = null;
	    boolean isDraggedApplet = false;

	    if (lap != null) {
		codebase = lap.getCodebase();
		documentBase = lap.getDocumentBase();
		isDraggedApplet = lap.isDraggedApplet();
	    }
	       
	    // We really don't know if this LaunchDesc is for a dragged 
	    // applet or for a Java Webstart applet, especially if the 
	    // JNLP can be launched from both plugin and webstart.

	    // The policy is if the LAP for this jnlp indicates it is
	    // a dragged applet and there are docbase and codebase in 
	    // LAP, we use the stored docbase and codebase and set 
	    // isAppletDescApplet to false even it is launched as a
	    // webstart applet.
	    boolean isAppletDescApplet = 
		(!isDraggedApplet && 
		 (codebase == null || documentBase == null));

	    // get codebase and docbase from the LaunchDesc
	    // if isAppletDescApplet is true

	    // Note: if it is an association, they need get codebase/docbase
	    // from LAP or LD in JNLP2Viewer
	    if (isAppletDescApplet) {
		AppletDesc ad = ld.getAppletDescriptor();
		
		URL codebaseURL = BasicServiceImpl.getInstance().getCodeBase();
		URL documentbaseURL = ad.getDocumentBase();
		// Documentbase defaults to codebase if not specified
		if (documentbaseURL == null) {
		    documentbaseURL = codebaseURL;
		}

		codebase = (codebaseURL != null)? codebaseURL.toString(): null;
		documentBase = (documentbaseURL != null)?
		    documentbaseURL.toString() : null;

		// if documentBase is null still, get it from ld as a last resort
		// FIXME: this may have different behavior from the java webstart
		// webstart just return null docbase. It does not use canonicalhome.
		if (documentBase == null) {
		    documentBase = ld.getCanonicalHome().toString();
		}
	    }
            executeApplet(ld, mainclass, dw, lap, codebase, documentBase, isAppletDescApplet);
        } else {
            executeApplication(ld, lap, mainclass, dw);
        }
    }

    /** Execute launchDesc for application */
    private void executeApplication(LaunchDesc ld,
        LocalApplicationProperties lap, Class mainclass, DownloadWindowHelper dw)
            throws IllegalAccessException, InstantiationException,
                NoSuchMethodException, InvocationTargetException   {

        PerfLogger.setTime("Begin executeApplication");

        String[] args = null;
        if (ld.getLaunchType() == LaunchDesc.INSTALLER_DESC_TYPE) {
            dw.reset();
            // Fixed argument for installer
            args = new String[1];
            args[0] = (lap.isLocallyInstalled() ? "uninstall" : "install");
            lap.setLocallyInstalled(false);
            lap.setRebootNeeded(false);
            try {
                lap.store();
            } catch (IOException ioe) {
                Trace.ignoredException(ioe);
            }
        } else {
            // Remove Splash window
            dw.disposeWindow();
            com.sun.javaws.ui.SplashScreen.hide();

            if (Globals.getApplicationArgs() != null) {
                // override args from command line
                args = Globals.getApplicationArgs();
            } else {
                // Get arguments for application from jnlp file
                args = ld.getApplicationDescriptor().getArguments();
            }
        }
	
        Object[] wrappedArguments = { args };

        // Find a static main(String[] args) method
        Class[] main_type = { (new String[0]).getClass() };
        Method mainMethod = mainclass.getMethod("main", main_type);
        // Check that method is static
        if (!Modifier.isStatic(mainMethod.getModifiers())) {
            throw new NoSuchMethodException(
                ResourceManager.getString("launch.error.nonstaticmainmethod"));
        }
        mainMethod.setAccessible(true);

        PerfLogger.setTime("End executeApplication (invoking App main)");

        // Invoke main method
        mainMethod.invoke(null, wrappedArguments);
    }

    /** Execute launchDesc for applet */
    private void executeApplet(final LaunchDesc ld, Class appletClass,
                final DownloadWindowHelper dw, LocalApplicationProperties lap,
	        String codebase, String documentBase, boolean isAppletDescApplet ) throws
		IllegalAccessException, InstantiationException,
		NoSuchMethodException, InvocationTargetException {

	// try to make use of the JNLP2Viewer to execute the applet
	// if the JNLP2Viewer class isn't found, it'll fall back to the 
	// previous code path.
	try {
	    Class jnlp2viewerClass = null;
	    
	    jnlp2viewerClass = Class.forName("sun.plugin2.applet.viewer.JNLP2Viewer");

	    // Remove all content from window
	    dw.disposeWindow();
	    com.sun.javaws.ui.SplashScreen.hide();

	    // Find a static main(String[] args) method
	    Class[] main_type = { (new String[0]).getClass() };
	    Method mainMethod = jnlp2viewerClass.getMethod("main", main_type);
	    mainMethod.setAccessible(true);

	    // use getCanonicalHome so we will also handle cases where JNLP file
	    // has no href
	    URL href = ld.getCanonicalHome();
	    
	    File cachedFile = null;
	    try {
		cachedFile = DownloadEngine.getCachedFile(href);
	    } catch (Exception ex) {
		ex.printStackTrace();
	    }
	    
	    if (cachedFile != null) {
		String args[] = null;
		if (!isAppletDescApplet) {
		    // check if "-open" or "-print" is being passed in
		    // this indicates a file association case
		    String actionName = SingleInstanceManager.getActionName();
		    if ((actionName != null) && 
			(actionName.equals("-open") || actionName.equals("-print"))) {
			args = new String[3];
			args[0] = actionName;
			args[1] = SingleInstanceManager.getOpenPrintFilePath();
			args[2] = cachedFile.toString();
		    } else if (lap.isDraggedApplet()) {
			args = new String[2];
			args[0] = "-draggedApplet";
			args[1] = cachedFile.toString();
		    } else {
			args = new String[1];
			args[0] = cachedFile.toString();
		    }
		} else {
		    args = new String[5];
		    args[0] = "-codebase";
		    args[1] = codebase;
		    args[2] = "-documentbase";
		    args[3] = documentBase;
		    args[4] = cachedFile.toString();
		}

		Object[] wrappedArgs = {args};

		// If an applet is relaunched from a shortcut which is from
		// launching an applet via the applet-desc element of a JNLP
		// file, the "-offline" argument is being passed from the javaws
		// process to the Java Web Start Main class. With the "-offline"
		// argument, the DeployOfflineManager.setForcedOffline(true) is
		// called. If the traditional AppletContainer is used to relaunch 
		// the applet from a shortcut, there's no issue because it doesn't
		// go though the DownloadEngine. However, relaunching the applet
		// via the JNLP2Viewer goes through the DownloadEngine and if the
		// forcedOffline is set in DeployOfflineManager, the DownloadEngine
		// will throw FailedDownloadException and the relaunch of the applet
		// will fail. Resetting the forcedOffline to false to workaround the
		// aforementioned issue.
		DeployOfflineManager.setForcedOffline(false);

		// call the Main method of JNLP2Viewer passing in the cached
		// JNLP file name as the last argument
		mainMethod.invoke(null, wrappedArgs);
	    }

	} catch (ClassNotFoundException cnfe) {

	    AppletDesc ad = ld.getAppletDescriptor();
	    int height = ad.getWidth();
	    int width  = ad.getHeight();

	    Applet applet = null;
	    applet = (Applet)appletClass.newInstance();

	    // Remove all content from window
	    dw.disposeWindow();
	    com.sun.javaws.ui.SplashScreen.hide();

	    final JFrame mainFrame = new JFrame();

	    // Force classloading
	    boolean check = BrowserSupport.isWebBrowserSupported();

	    // New applet container stub
	    AppletContainerCallback callback = new AppletContainerCallback() {
		/** Use BasicService to show document */
		public void showDocument(URL url) {
		    BrowserSupport.showDocument(url);
		}

		/** Resize frame */
		public void relativeResize(Dimension delta) {
		    Dimension d = mainFrame.getSize();
		    d.width  += delta.width;
		    d.height += delta.height;
		    mainFrame.setSize(d);
		}
	    };

	    URL codebaseURL = BasicServiceImpl.getInstance().getCodeBase();
	    URL documentbaseURL = ad.getDocumentBase();
	    // Documentbase defaults to codebase if not specified
	    if (documentbaseURL == null) documentbaseURL = codebaseURL;

	    // Build GUI for Applet
	    final AppletContainer ac = new AppletContainer(callback,
		      applet,
		      ad.getName(),
		      documentbaseURL,
		      codebaseURL,
		      height,
		      width,
		      ad.getParameters());



	    // We want to override the default WindowListener added inside the
	    // DownloadWindow object and add a new WindowListener which calls
	    // applet's stop() & destroy() method whenever window is closed.

	    mainFrame.addWindowListener(new WindowAdapter() {
		public void windowClosing (WindowEvent event) {
		    ac.stopApplet();
		}
	    });

	    // Update gui for frame
	    mainFrame.setTitle(ld.getInformation().getTitle());
	    Container parent = mainFrame.getContentPane();
	    parent.setLayout(new BorderLayout());
	    parent.add("Center", ac);
	    mainFrame.pack();
	    Dimension d = ac.getPreferredFrameSize(mainFrame);
	    mainFrame.setSize(d);
	    // Force repaint of frame
	    mainFrame.getRootPane().revalidate();
	    mainFrame.getRootPane().repaint();
	    mainFrame.setResizable(false);
	    if (!mainFrame.isVisible()) {
		SwingUtilities.invokeLater(new Runnable() {
		    public void run() {
			mainFrame.setVisible(true);
		    }
		});
	    }

	    // Start applet
	    ac.startApplet();
	}
    }

    private void handleJnlpFileException (LaunchDesc ld,
                Exception exception) throws ExitException {
        /* purge the bad jnlp file from the cache: */       
        DownloadEngine.removeCachedResource(ld.getCanonicalHome(),
                null, null);
       
        throw new ExitException(exception, ExitException.LAUNCH_ERROR);
    }
}
