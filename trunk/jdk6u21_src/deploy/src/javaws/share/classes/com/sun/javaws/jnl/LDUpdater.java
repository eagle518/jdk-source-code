/*
 * @(#)LDUpdater.java	1.17 10/05/14
 * 
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.jnl;

import java.io.File;
import java.io.IOException;
import java.io.FileInputStream;
import java.net.URL;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.LinkedList;
import com.sun.deploy.Environment;
import com.sun.deploy.cache.Cache;
import com.sun.deploy.cache.LocalApplicationProperties;
import com.sun.deploy.config.Config;
import com.sun.deploy.net.DownloadEngine;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.ui.AppInfo;
import com.sun.javaws.*;
import com.sun.javaws.exceptions.*;
import com.sun.javaws.ui.DownloadWindow;
import com.sun.javaws.ui.DownloadWindowHelper;
import com.sun.javaws.ui.UpdateDialog;

import sun.awt.AppContext;

/**
 * An Object that does update check and download for a LaunchDesc.
 */

public class LDUpdater {
    private LaunchDesc _ld = null;
    private boolean _updateAvailable;
    private boolean _updateDownloaded;
    private boolean _updateChecked;
    private boolean _checkAborted;
    private boolean _checkFaulted;
    private LocalApplicationProperties _lap = null;

    private volatile Thread backgroundUpdateThread = null;
    private Exception _exception = null;
    private int _numTasks = 0;
    private int _numTasksMax = 0;
    private boolean _checkDone;
    
    private RapidUpdateCheckerQueue queue = null;
    
    private static final String APPCONTEXT_LD_KEY = Config.getAppContextKeyPrefix() +"mainlaunchdescinappcontext";

    public LDUpdater(LaunchDesc ld) {
        _ld = ld;
        setMainLaunchDescInAppContext();
    }

    /** 
     * @return true if the update check if aborted
     *         this is used when user cancel https cert dialog
     */
    public boolean isCheckAborted() {
        return _checkAborted;
    }

    /**
     * Check if any resources have update available
     * @return true is update available
     * @throws Exception extension LDUpdater throws Exception to its parent
     *         main app LDUpdater throws Exception if it is severe enough to
     *         stop execution
     */
    public boolean isUpdateAvailable() throws Exception {
        // default behavior of isUpdateAvailable if no parameters passed in
        //     - does not check icons if it is in plugin
        //     - check lazy resources
        //     - for https, do update in order
        //     - for http, do concurrent out of order check
        boolean checkIcon = Environment.isJavaWebStart();
        if (_ld.isHttps()) {
            // check in order because there may be cert pop up
            return isUpdateAvailable(checkIcon, true, false);
        } else {
            return isUpdateAvailable(checkIcon, true, true);
        }
    }

    public boolean isUpdateAvailable(boolean checkIcon, 
                                     boolean checkLazy,
                                     boolean rapidCheck) throws Exception {
        
        // if never checked, do update check, otherwise return saved result
        synchronized(this) {
            if (_updateChecked) {
                return _updateAvailable;
            }
        }

        try {

            if (_ld.isApplicationDescriptor()) {
                // doUpdaetCheck is for main app LD. It manages update check types
                startUpdateCheck(checkIcon, checkLazy, rapidCheck);
            } else {
                // extension LDs, we don't check extension based on its own
                // update check type. Type from the main app is in charge.
                // FIXME: we may want to change this in the future.
                _updateAvailable = updateCheck(checkIcon, checkLazy, rapidCheck);
            }

            synchronized(this) {
                _updateChecked = true;
                _updateDownloaded = !_updateAvailable;
            }
            // if the ld is a installer extension
            // and there is a update.
            // need force update check at next launch to install new installer
            if (_ld.isInstaller() && _updateAvailable && DownloadEngine.isBackgroundThread()) {
                setForceUpdateCheck();
            }
        } catch (Exception e) {
            Trace.ignored(e);
            throw e;
        }
        
        return _updateAvailable;
    }

    public boolean isUpdateDownloaded() {
        return _updateDownloaded;
    }
    
    // Download update to the cache if available.
    public void downloadUpdate(boolean includeLazy ) throws Exception {
        if (_updateAvailable) {
            download(includeLazy);
        }
        
        synchronized(this) {
            _updateAvailable = false;
            _updateDownloaded = true;
        }
    }

    // This method involves update check type and thread management
    // It is used only for the main app LaunchDesc
    private void startUpdateCheck(final boolean checkIcon, 
                                  final boolean checkLazy,
                                  final boolean rapidCheck) throws Exception { 
        if (_lap == null) {
            _lap = Cache.getLocalApplicationProperties(_ld.getCanonicalHome());
        }
        
        int updateCheckType = _ld.getUpdate().getCheck();
        
        if (updateCheckType == UpdateDesc.CHECK_BACKGROUND) {
            // return immediately - background update will start after jars are loaded
            return;
        }

        // non-background update check

        int timeout = Config.getIntProperty(Config.JAVAWS_UPDATE_TIMEOUT_KEY);

        // An array to hold results
        // - results[0] = available
        // - results[1] = completed
        // - results[2] = faulted
        // - results[3] = aborted
        final boolean results[] = new boolean[4]; // default false
        
        // start a thread to do update check
        // the main thread waits for the result or timeout
        new Thread(new Runnable() {

            public void run() {
                boolean available = false;
                boolean completed = false;
                boolean faulted = false;
                boolean aborted = false;
                try {
                    if (updateCheck(checkIcon, checkLazy, rapidCheck)) {
                        // updateCheck from the main LaunchDesc will recursively check 
                        // all extensions and jars
                        available = true;
                        
                        // set force update check in LAP
                        // just in case that the main thread timed out
                        // and does not update resources. The check will be
                        // reset after all updates are downloaded.
                        setForceUpdateCheck();
                    }
                    completed = true;
                } catch (FailedDownloadingResourceException fdre) {
                    if (_ld.isHttps()) {
                        Throwable thr = fdre.getWrappedException();
                        if ((thr != null) &&
                                (thr instanceof javax.net.ssl.SSLHandshakeException)) {
                            // user chose not to accept Https cert ...
                            aborted = true;
                        }
                    }
                    Trace.ignored(fdre);
                    faulted = checkException(fdre);
                } catch (Exception e) {
                    Trace.ignored(e);
                    faulted = checkException(e);
                }

                // notify the parent the check results
                synchronized (results) {
                    results[0] = available;
                    results[1] = completed;
                    results[2] = faulted;
                    results[3] = aborted;
                    results.notifyAll();
                }
            }
        }, ""+_ld.getLocation()).start();
        
        synchronized(results) {
            while (!results[1] && !results[2] && !results[3]) {
                try {
                    results.wait(timeout);
                } catch (InterruptedException e) {
                    Trace.ignored(e);
                }
                
                // if check type is time out, we don't loop forever
                if (updateCheckType == UpdateDesc.CHECK_TIMEOUT) {
                    if (!_ld.isHttps()) {
                        // https may require user interaction. e.g. accept certs
                        // It has to wait until check completed.
                        // For non https connections, break the loop
                        break;
                    }
                }
            }
            _checkFaulted = results[2];
            _checkAborted = results[3];
            _updateAvailable = results[0];
        }

        if (_checkFaulted) {
            throw _exception;
        }
    }
    
    public synchronized boolean isBackgroundUpdateRunning() {
        return null != backgroundUpdateThread;
    }

    /** 
     * Starts the background check and update thread,
     * if this feature is enabled,
     * i.e. the UpdateDesc.CHECK_BACKGROUND flag is set.
     */
    public void startBackgroundUpdateOpt() {
        if (!_ld.getUpdate().isBackgroundCheck()) {
            return; // bail out
        }
        if (null != backgroundUpdateThread) {
            return; // bail out
        }
        startBackgroundUpdate();
    }

    /**
     * Prompt users whether they want to do update, launch from cache or exit
     * @param dwh DownloadWinowHelper which can get the parent of the dialog
     * @return user answers
     */
    public boolean needUpdatePerPolicy(DownloadWindowHelper dwh) throws ExitException {
        boolean forceUpdate = false;
        DownloadWindow dw = (dwh == null) ? null : dwh.get();

        // for updating other resources - depends on the policy of the main app
        // not the policy of each extension
        switch (_ld.getUpdate().getPolicy()) {
            default:
            case UpdateDesc.POLICY_ALWAYS:
                forceUpdate = true;
                break;
            case UpdateDesc.POLICY_PROMPT_UPDATE:
                forceUpdate = UpdateDialog.showUpdateDialog(_ld, dw);
                if (!forceUpdate) {
                    Trace.println("Start from cached after user chose not to update",
                            TraceLevel.BASIC);
                    // user cancelled update, use cached one
                    // update will be done in the background
                    //
                    //FIXME: this may cause redundant downloads of missing jars
                    //as background downloads may work concurrently with normal downloads
                    startBackgroundUpdate();
                }
                break;
            case UpdateDesc.POLICY_PROMPT_RUN:
                forceUpdate = UpdateDialog.showUpdateDialog(_ld, dw);

                if (!forceUpdate) {
                    Trace.println("Exiting after user chose not to update",
                            TraceLevel.BASIC);
                    // user cancelled, has to quit. Show Red X.
                    throw new ExitException(new LaunchDescException(_ld, "User cancelled mandatory update - aborted", null),
                            ExitException.LAUNCH_ERROR);
                }
                break;
        }

        return forceUpdate;
    }

    public boolean needUpdatePerPolicy() throws ExitException {
        return needUpdatePerPolicy(null);
    }

    /** 
     * Start background check and update thread
     * It does following steps:
     *   - traverse the jnlp tree, get all URLs that need check
     *   - store the list of URLs in the AppContext. The listed URLs 
     *       are not checked in DownloadEngine.isUpdateAvaiable().
     *       This is to prevent ClassLoader check for update.
     *   - the background thread is marked with a ThreadLocal "background"
     *   - if there is an update, download them
     *   - clear the background URLs from the AppContext
     *
     * We have 1 background thread per LDUpdater instance,
     * so we bail out, if one is already running for one LDUpdater object.
     */
    private synchronized void startBackgroundUpdate() {
        if ( null != backgroundUpdateThread ) {
            return; // bail out ..
        }

        Trace.println("LDUpdater: started background update check", TraceLevel.NETWORK);

        final ArrayList backgroundList = buildBackgroundDownloadList();
        storeBackgroundListInAppContext(backgroundList);

        backgroundUpdateThread = new Thread(new Runnable() {
                public void run() {
                    try {
                        // set ThreadLocal for this update checker thread
                        DownloadEngine.setBackgroundThread(true);

                        // Background update check, let other threads to start the app first
                        try {
                            Thread.sleep(5000);
                        } catch (InterruptedException e) {
                        }

                        boolean downloadError = false;

                        // background update check is not concurrent
                        if (updateCheck(false, true /*checkLazy*/, false)) {
                            try {
                                download(true /* includeLazy*/);
                            } catch (Exception e) {
                                downloadError = true;
                                Trace.println("LDUpdater: exception in " +
                                        "background update download, set force " +
                                        "update check to true", TraceLevel.NETWORK);
                                Trace.ignoredException(e);
                                // there was update and we failed during
                                // download of update, set force update
                                // check in next launch
                                setForceUpdateCheck();
                            }
                        }
                        if (!downloadError) {
                            resetForceUpdateCheck();
                        }
                    } catch (Exception e) {
                        Trace.ignoredException(e);
                    } finally {
                        removeBackgroundListInAppContext(backgroundList);
                    }
                }
            });
        
        backgroundUpdateThread.setName("Background Update Thread");
        backgroundUpdateThread.setPriority(Thread.MIN_PRIORITY);
        backgroundUpdateThread.setDaemon(true);
        backgroundUpdateThread.start();
    }

    // check if there is update available for this LaunchDesc
    // It check this jnlp file, its local jar resources, icons
    // and recursively all extensions
    // rapidCheck indicates whether we do in order check (serial)
    // or out of order check (parallel)
    //   - serial check is depth first tree traversal
    //   - parallel check launches one thread for each resources
    private boolean updateCheck (boolean checkIcon, boolean checkLazy, boolean rapidCheck) throws Exception {
        if (rapidCheck) {  
            if (queue == null) {
                queue = getQueue();
                if (queue == null) {
                    // for some reason, cannot get the main queue. go slow path
                    rapidCheck = false;
                }
            }
        }

        if (rapidCheck) {
            URL homeUrl = _ld.getLocation();
            if (homeUrl != null) {
                incrementTaskNum();
                queue.enqueue(new RapidUpdateChecker(homeUrl, RapidUpdateChecker.HOME_URL));
            }

            ResourcesDesc rd = _ld.getResources();
            if (rd != null) {
                // Check local jars
                JARDesc[] jars = rd.getLocalJarDescs();
                for(int i = 0; i < jars.length; i++) {
                    if (checkLazy || !jars[i].isLazyDownload()) {
                        incrementTaskNum();
                        queue.enqueue(new RapidUpdateChecker(jars[i], RapidUpdateChecker.JAR_DESC));
                    }
                }

                // Plugin does not need check icons
                // checkIcon is true for JavaWS
                if (checkIcon) {
                    IconDesc [] icons = _ld.getInformation().getIcons();
                    if (icons != null) {
                        for (int i=0; i<icons.length; i++) {
                            incrementTaskNum();
                            queue.enqueue(new RapidUpdateChecker(icons[i], RapidUpdateChecker.ICON_DESC));
                        }
                    }
                }

                // get extensions in this LaunchDesc.
                final ArrayList list = new ArrayList();
                getAllExtensions(rd, list);

                for (int i = 0; i < list.size(); i++) {
                    // The link should already be set up
                    LaunchDesc extensionLD = ((ExtensionDesc) list.get(i)).getExtensionDesc();

                    // launch a new thread to check entension. Don't queue the task in the queue since it can
                    // potentially jam the queue and cause deadlock
                    incrementTaskNum();
                    new Thread(new RapidUpdateChecker(extensionLD, RapidUpdateChecker.EXT_LD),
                               RapidUpdateCheckerQueue.RAPID_CHECK_THREAD_NAME + nextSequenceNumber()).start();
                }
            }

            // if concurrent check, the thread is waiting here for
            // _updateAvailable is updated by a thread or all
            // threads finish checking _checkDone
            // If the thread is interrupted, we stop waiting and return
            synchronized(this) {
                while (!_checkDone && !_updateAvailable) {
                    try {
                        wait();
                    } catch (InterruptedException e) {
                        Trace.ignored(e);
                        break;
                    }
                }
            }
            // suggest the queue to stop.
            // Only the main app LDUpdater can really stop the queue.
            stopQueue();
            
            // Need throw exception in rapid update check if happens
            if (!_updateAvailable && _exception != null) {
                throw _exception;
            }

            return _updateAvailable;
        }

        // slow check path
        // Check for updated JNLP files if href is specified in jnlp element
        // If the jnlp is updated, we need download the newer version and update
        // its resources.
        URL homeUrl = _ld.getLocation();
       
        if (homeUrl != null) {
            try {
                if (DownloadEngine.isUpdateAvailable(homeUrl, null)) {
                    return true;
                }
            } catch (IOException ioe) {
                throw new FailedDownloadingResourceException(homeUrl, null, ioe);
            }
        }
        
        ResourcesDesc rd = _ld.getResources();
        if (rd == null) {
            return false;
        }
        
        // Check local jars
        JARDesc[] jars = rd.getLocalJarDescs();
        for (int i = 0; i < jars.length; i++) {
            if (checkLazy || !jars[i].isLazyDownload()) {
                if (jars[i].getUpdater().isUpdateAvailable()) {
                    return true;
                }
            }
        }

        // Plugin does not need check icons
        // checkIcon is true for JavaWS
        if (checkIcon) {
            IconDesc[] icons = _ld.getInformation().getIcons();
            if (icons != null) {
                for (int i = 0; i < icons.length; i++) {
                    URL location = icons[i].getLocation();
                    String version = icons[i].getVersion();
                    try {
                        if (DownloadEngine.isUpdateAvailable(location, version)) {
                            Globals.setIconImageUpdated(true);
                        }
                    } catch (IOException ioe) {
                        throw new FailedDownloadingResourceException(location, null, ioe);
                    }
                }
            }
        }

        // get extensions in this LaunchDesc. 
        final ArrayList list = new ArrayList();
        getAllExtensions(rd, list);
        
        for (int i = 0; i < list.size(); i++) {
            // The link should already be set up
            LaunchDesc extensionLD = ((ExtensionDesc) list.get(i)).getExtensionDesc();
            try {
                // recursively check down from this ExtLD
                if (extensionLD.getUpdater().isUpdateAvailable(checkIcon,
                        checkLazy, rapidCheck)) {
                    return true;
                }
            } catch (NullPointerException npe) {
                // just in case extensionLD is null
                Trace.ignored(npe);
            }
        }

        return false;
    }

    private void getAllExtensions(final ResourcesDesc rd, final ArrayList list) {
        rd.visit(new ResourceVisitor() {
            public void visitJARDesc(JARDesc jad) {}
            public void visitPropertyDesc(PropertyDesc prd) {}
            public void visitPackageDesc(PackageDesc pad) {}
            public void visitJREDesc(JREDesc jrd) {}
            public void visitExtensionDesc(ExtensionDesc ed) {
                list.add(ed);
            }
        });
    }  
    
    // download resources if there is a update 
    // currently used only in background. And download is serial not parallel
    private void download (boolean includeLazy)
        throws Exception, JNLPException {
        
        // Download the jnlp, if update available and installer type, we need force update check 
        // at next launch. 
        URL location = _ld.getLocation();

        // download new jnlp file if there is one.
        // We know there is a update. but we don't know if the update is 
        // jnlp itself or its resources

        // If you download a main jnlp file, you might have a shortcut 
        // for the old one, which will now be invalid, so we must know if a 
        // download is necessary first, then call notify LIH to fix.
        boolean jnlpUpdate = DownloadEngine.isUpdateAvailable(location, null);

        if (jnlpUpdate) {
            File newJnlpFile = DownloadEngine.getUpdatedFile(location, null);
        
            byte[] newContents = LaunchDescFactory.readBytes(
                   new FileInputStream(newJnlpFile), newJnlpFile.length());

            // compare the content of two LaunchDesc
            // if the new LD is different from the old LD
            // We need rebuilt LD from the jnlp file since it
            // may have updated resource. e.g. a new jar resource
            if (!_ld.hasIdenticalContent(newContents)) {
                _ld = LaunchDescFactory.buildDescriptor(newJnlpFile,
                        _ld.getCodebase(), location, location);
            }

            // Whether it is differant or not, if it was downloaded, 
            // we may need to notify the shortcut handler.
	    // this update shortcut for both webstart application and
	    // dragged-out applets
            LocalApplicationProperties lap = 
                Cache.getLocalApplicationProperties(location);
            if (lap != null) {
                Launcher.notifyLocalInstallHandler(_ld, lap, 
                                                   true, true, null);
            }
        }
        ResourcesDesc rd = _ld.getResources();
        if (rd == null) return;

        JARDesc [] jardescs = rd.getLocalJarDescs();
        ArrayList jars = new ArrayList();
        
        // build a list of Jars that need update
        if (includeLazy) {
            jars.addAll(Arrays.asList(jardescs));
        } else {
            for(int i = 0; i < jardescs.length; i++) {
                // if resource is lazy and !includeLazy, don't add to the list
                if (!jardescs[i].isLazyDownload()) {
                    jars.add(jardescs[i]);
                }
            }
        }
        
        // if the jar has update, download it to a temp cache entry
        for (int i = 0; i < jars.size(); i++) {
            JARDesc jar =(JARDesc) jars.get(i);
            if (jar.getUpdater().isUpdateAvailable()) {
                jar.getUpdater().downloadUpdate();
            }
        }
        
        // All jar downloaded, make downloaded entries active
        // This is not really atomic cache update, but fast enough
        // since all jars should have downloaded.
        for (int i = 0; i < jars.size(); i++) {
            JARDesc jar =(JARDesc) jars.get(i);
            if (jar.getUpdater().isUpdateDownloaded()) {
                jar.getUpdater().updateCache();
            }
        }

        // also get default icon if there is one.
        // This may not needed for jnlp plugin
        IconDesc id = _ld.getInformation().getIconLocation(
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
        
        // now recursively download extensions
        ExtensionDesc[] extensions = rd.getExtensionDescs();
        
        for (int i = 0; i < extensions.length; i++) {
            LaunchDesc extensionLD = extensions[i].getExtensionDesc();
            URL extLocation = extensionLD.getLocation();
            
            try {                
                if (extensionLD.getUpdater().isUpdateAvailable(false, includeLazy, false)) {
                    extensionLD.getUpdater().downloadUpdate(includeLazy);
                }
            } catch (NullPointerException npe) {
                Trace.ignored(npe);
            } catch (IOException ioe) {
                throw new FailedDownloadingResourceException(extLocation, null, ioe);
            }
        }
        return;
    }

    // store force update check in LAP
    // it will force a full non-background check at next launch
    private void setForceUpdateCheck() {
        // Store info. in local application properties,
        // so the next time we can force an update
        if (_lap != null && !_lap.forceUpdateCheck()) {
            _lap.setForceUpdateCheck(true);
            try {
                _lap.store();
            } catch (IOException ioe) {
                Trace.ignoredException(ioe);
            }
        }
    }

    private void resetForceUpdateCheck() {
        if (_lap != null && _lap.forceUpdateCheck()) {
            _lap.setForceUpdateCheck(false);
            try {
                _lap.store();
            }catch(IOException ioe) {
                Trace.ignoredException(ioe);
            }
        }
    }

    // Take a main launch descriptor and traverse all extensinons 
    // And build a list of files that requires background update check
    // One assumption is that All resources (though may be outdated) are in cache
    private ArrayList buildBackgroundDownloadList() {
        ArrayList list = new ArrayList();
        
        try {
            URL homeUrl = _ld.getLocation();
            
            if (homeUrl != null) {
                list.add(homeUrl.toString());
            }
            
            // add all resources
            ResourcesDesc rd = _ld.getResources();
            if (rd != null) {
                ExtensionDesc[] extensions = rd.getExtensionDescs();
                
                for (int i = 0; i < extensions.length; i++) {
                    URL extensionUrl = extensions[i].getLocation();
                    if (extensionUrl != null) { 
                        list.add(extensionUrl.toString());
                    }
                }
                
                // Include lazy resources
                JARDesc[] jars = rd.getEagerOrAllJarDescs(true);
                if (jars != null) {
                    for (int i = 0; i < jars.length; i++) {
                        URL location = jars[i].getLocation();
                        if (location != null) {
                            list.add(location.toString());
                        }
                    }
                }
                
                IconDesc[] icons = _ld.getInformation().getIcons();
                if (icons != null) {
                    for (int i = 0; i < icons.length; i++) {
                        URL location = icons[i].getLocation();
                        if (location != null) {
                            list.add(location.toString());
                        }
                    }
                }
            }
        } catch (Exception e) {
            Trace.ignored(e);
        }
        
        return list;
    }

    // store the list in the AppContext so it can be retrieved easily
    private void storeBackgroundListInAppContext(ArrayList list) {
        if (list == null) return;

        for (int i = 0; i < list.size(); i++) {
            String href = (String)list.get(i);
            
            if (href != null) {
                AppContext.getAppContext().put(DownloadEngine.APPCONTEXT_BG_KEY+href, 
                                               DownloadEngine.BACKGROUND_STRING);
            }
        }
    }
    
    private void removeBackgroundListInAppContext(ArrayList list) {
        if (list == null) return;

        for (int i = 0; i < list.size(); i++) {
            String href = (String)list.get(i);
            
            if (href != null) {
                AppContext.getAppContext().remove(DownloadEngine.APPCONTEXT_BG_KEY+href);
            }
        }
    }

    // use a sequence number to identify rapid update checker thread
    // maybe we can use the location in the thread name for debugging purpose.
    private static int sequenceNumber = 0;
    private static int nextSequenceNumber() {
        synchronized (LDUpdater.class) {
            return ++sequenceNumber;
        }
    }

    private LDUpdater getMainLDUpdater() {
        if (_ld.isApplicationDescriptor()) {
            return _ld.getUpdater();
        }

        LaunchDesc mainLd = getMainLaunchDescFromAppContext();
        if (null != mainLd) {
            return mainLd.getUpdater();
        }
        return null;
    }

    // return the queue in the main LDUpdater
    // queue in extension LDUpdater points to the queue in the main.
    private RapidUpdateCheckerQueue getQueue() {
        if (!(_ld.isApplicationDescriptor())) {
            LDUpdater updater = getMainLDUpdater();
            if (updater != null) {
                return updater.getQueue();
            }

            return null;
        }

        if (queue == null) {
            int n = getNumTasksMax();
            queue = new RapidUpdateCheckerQueue(n);
            new Thread(queue, "Rapid Update Checker Queue").start();
        }
        return queue;
    }

    private void stopQueue() {
        if (!(_ld.isApplicationDescriptor())) {
            return;
        }
        // only the main app LDUpdater can stop the queue
        if (queue != null) {
            queue.stop();
        }
        return;
    }


    // return the max conncurrent downloads setting in
    // main app LaunchDesc
    private int getNumTasksMax() {
        if (_numTasksMax != 0) {
            return _numTasksMax;
        }

        if (_ld.isApplicationDescriptor()) {
            _numTasksMax = _ld.getResources().getConcurrentDownloads();
            return _numTasksMax;
        }

        LaunchDesc mainAppLd = getMainLaunchDescFromAppContext();
        if (mainAppLd != null) {
            return mainAppLd.getUpdater().getNumTasksMax();
        }

        return Config.JAVAWS_CONCURRENT_DOWNLOADS_DEF;
    }

    private void setMainLaunchDescInAppContext() {
        if (_ld.isApplicationDescriptor()) {
            AppContext.getAppContext().put(APPCONTEXT_LD_KEY, _ld);
        }
    }

    private LaunchDesc getMainLaunchDescFromAppContext() {
        Object o = AppContext.getAppContext().get(APPCONTEXT_LD_KEY);
        if (null != o) {
            return (LaunchDesc)o;
        }
        return null;
    }

    private synchronized void incrementTaskNum() {
        _numTasks++;
    }

    private synchronized void notifyUpdate(boolean update) {
        if (update) {
            _updateAvailable = true;
            notifyAll();
        } else {
            if (--_numTasks == 0) {
                _checkDone = true;
                notifyAll();
            }
        }
    }

    private synchronized void notifyException(Exception e) {
        if (_ld != null && _ld.getInformation().supportsOfflineOperation()) {
            // application has offline-allowed set in JNLP file
            // ignore update check exception and continue
            Trace.ignoredException(e);
            return;
        }
        // we keep only the first exception

        if (_exception == null) {
            _exception = e;
        }
    }

    // based on exception and LD, decide whether we want to stop or continue
    // if exception happens during update check
    private boolean checkException(Exception exception) {
        // We know update check is important but not critical. If the type is timeout and the exception
        // is a IOE wrapped in a JNLPException and the LD support offline
        // we don't want to throw exception and let the app run from cache.
        if (_ld.getUpdate().getCheck() == UpdateDesc.CHECK_TIMEOUT &&
                _ld.getInformation().supportsOfflineOperation()) {
            if (exception instanceof JNLPException) {
                Throwable t = ((JNLPException) exception).getWrappedException();
                if (t instanceof IOException) {
                    return false;
                }
            }
        }
        return true;
    }

    // manage a FIFO queue of tasks and only start a new checker thread when the max
    // number is not reached.
    private class RapidUpdateCheckerQueue implements Runnable {

        private static final String RAPID_CHECK_THREAD_NAME = "Rapid Update Checker- ";
        private volatile boolean shouldStop;
        private final Object lock = new Object();
        private LinkedList/*<Runnable>*/ workQueue = new LinkedList();
        private int nThreads = 0;
        private int nThreadsMax;

        private RapidUpdateCheckerQueue(int max) {
            nThreadsMax = max;
        }

        private void enqueue(Runnable task) {
            synchronized (lock) {
                workQueue.add(task);
                lock.notifyAll();
            }
        }

        private void stop() {
            shouldStop = true;
            synchronized (lock) {
                lock.notifyAll();
            }
        }

        public void run() {
            try {
                while (!shouldStop) {
                    synchronized (lock) {
                        while (!shouldStop && workQueue.isEmpty()) {
                            try {
                                lock.wait();
                            } catch (InterruptedException e) {
                                e.printStackTrace();
                            }
                        }
                    }
                    while (!shouldStop && !workQueue.isEmpty()) {
                        Runnable task = null;
                        synchronized (lock) {
                            task = (Runnable) workQueue.removeFirst();
                        }
                        synchronized(this) {
                            while (nThreads >= nThreadsMax) {
                                try {
                                    wait();
                                } catch (InterruptedException e) {
                                    e.printStackTrace();
                                }
                            }
                            nThreads++;
                        }
                        new Thread(task, RAPID_CHECK_THREAD_NAME + nextSequenceNumber()).start();
                    }
                }
            } catch (Exception e) {
                e.printStackTrace();
            } 
        }
        
        private synchronized void decreaseNumTasks() {
            nThreads--;
            notifyAll();
        }
    }

    // Each thread checks an object. Once a update is found. The thread
    // updates the main thread immediately
    private class RapidUpdateChecker implements Runnable {
        private static final int HOME_URL = 0;
        private static final int JAR_DESC = 1;
        private static final int ICON_DESC = 2;
        private static final int EXT_LD = 3;

        private Object target;
        private int type;

        private boolean update = false;

        private RapidUpdateChecker(Object target, int type) {
            this.target = target;
            this.type = type;
        }

        public void run() {
            try {
                switch (type) {
                    case HOME_URL :
                        URL homeUrl = (URL)target;
                        Trace.println("LDUpdater: update check for " + homeUrl, TraceLevel.NETWORK);
                        try {
                            update = DownloadEngine.isUpdateAvailable(homeUrl, null);
                        } catch (IOException ioe) {
                            throw new FailedDownloadingResourceException(homeUrl, null, ioe);
                        }
                        break;
                    case JAR_DESC :
                        JARDesc jar = (JARDesc)target;
                        Trace.println("LDUpdater: update check for " + jar.getLocation(), TraceLevel.NETWORK);
                        update = jar.getUpdater().isUpdateAvailable();
                        break;
                    case ICON_DESC :
                        IconDesc icon = (IconDesc)target;
                        URL location = icon.getLocation();
                        String version = icon.getVersion();
                        try {
                            if (DownloadEngine.isUpdateAvailable(location, version)) {
                                Globals.setIconImageUpdated(true);
                            }
                        } catch (IOException ioe) {
                            throw new FailedDownloadingResourceException(location, null, ioe);
                        }
                        break;
                    case EXT_LD :
                        LaunchDesc ext = (LaunchDesc)target;
                        Trace.println("LDUpdater: update check for "+ext.getLocation(), TraceLevel.NETWORK);
                        update = ext.getUpdater().isUpdateAvailable();
                        break;
                    default:
                        break;
                }
            } catch (Exception e) {
                notifyException(e);
            } finally {
                notifyUpdate(update);
                // decrease the number of running threads in the queue
                // notify the queue if it is waiting
                queue.decreaseNumTasks();
            }
        }
    }
}


