/*
 * @(#)JNLP2Manager.java	1.73 10/05/13
 *
 * Copyright (c) 2007, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.applet;

import java.applet.AppletContext;

import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.Container;
import java.io.File;
import java.io.IOException;
import java.io.FileNotFoundException;
import java.lang.reflect.Constructor;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.HashSet;
import java.util.Set;
import java.util.StringTokenizer;
import java.util.Properties;
import java.util.Date;
import java.util.Vector;
import java.util.concurrent.ExecutorService;


import javax.swing.JPanel;
import sun.awt.AppContext;

import com.sun.deploy.perf.DeployPerfUtil;
import com.sun.javaws.progress.Progress;
import com.sun.javaws.progress.CustomProgress;
import com.sun.javaws.progress.ProgressListener;

import com.sun.deploy.ui.UIFactory;

import com.sun.javaws.*;
import com.sun.javaws.jnl.*;
import com.sun.javaws.ui.AutoDownloadPrompt;
import com.sun.javaws.ui.SecureStaticVersioning;
import com.sun.javaws.security.AppPolicy;
import com.sun.javaws.exceptions.*;
import com.sun.javaws.util.JavawsDialogListener;

import com.sun.javaws.exceptions.JRESelectException;
import com.sun.javaws.exceptions.ExitException;

import javax.jnlp.ServiceManager;
import javax.jnlp.DownloadServiceListener;

import com.sun.jnlp.JnlpLookupStub;
import com.sun.jnlp.JNLPPreverifyClassLoader;
import com.sun.deploy.config.Config;
import com.sun.deploy.config.JREInfo;
import com.sun.deploy.si.SingleInstanceManager;
import com.sun.deploy.util.*;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.cache.Cache;
import com.sun.deploy.cache.LocalApplicationProperties;
import com.sun.deploy.net.DownloadEngine;
import com.sun.deploy.net.offline.DeployOfflineManager;
import com.sun.deploy.Environment;

// We need a notion of the command-line arguments that were used to
// start this JVM instance to decide whether we need to relaunch the
// applet (or, more precisely, whether to fire an event indicating the
// need to relaunch the applet)

import sun.plugin2.util.SystemUtil;
import sun.plugin2.util.ParameterNames;
import sun.plugin.javascript.JSContext;
import netscape.javascript.JSObject;

/** The main class which manages the creation and lifecycle of an
    applet. A new Plugin2Manager instance should be created for each
    applet being viewed. 

    FIXME: keep in sync with com.sun.javaws.Launcher
 */

public class JNLP2Manager extends Plugin2Manager {

    /**
    *  Reference to download progress window
    */
    private URL    _initDocumentBaseURL = null;
    private String _initJnlpFile=null;

    private boolean _initialized;

    private JREInfo homeJREInfo = null;

    private URL    _codebase = null;
    private LaunchDesc _launchDesc=null;
    private AppPolicy  _appPolicy=null;

    private LocalApplicationProperties _lap = null;

    private static boolean _environmentInitialized = false;

    private String _cachedJNLPFilePath = null;

    private JRESelectException _relaunchException = null;

    // cached sigining info for later use
    private boolean _allSigned = false;

    public void setCachedJNLPFilePath(String path) {
        _cachedJNLPFilePath = path;
    }
    
    public JNLP2Manager(String codebaseStr, URL documentBaseURL, String jnlpFile, boolean relaunched)
        throws Exception
    {
        super(relaunched);
        _initDocumentBaseURL = documentBaseURL;
        _initJnlpFile = jnlpFile;
        _initialized  = false;

        if ( _INJECT_EXCEPTION_CSTR ) {
            Exception e = new Exception("INJECT_JNLP2MANAGER_EXCEPTION_CSTR");
            throw e;
        }

        if ( codebaseStr == null ) {
	    // FIXME: if _initDocumentBaseURL is a path url without slash, 
	    // URLUtil.getBase() does not work.
	    // e.g URLUtil.getBase("https://jdk6.dev.java.net/plugin2") 
	    // returns "https://jdk.dev.java.net/"
	    // this is incorrect codebase. should be "https://jdk6.dev.java.net/plugin2".
	    // Note: _codebase need be pathURL which means end with a slash
	    // Otherwise, we are going to have trouble later.
            _codebase = URLUtil.getBase(_initDocumentBaseURL);
            if (DEBUG) {
                System.out.println("   JNLP Codebase (is documentbase): "+_codebase);
            }
        } else {
            try {
                _codebase = URLUtil.asPathURL(new URL (codebaseStr));
                if (DEBUG) {
                    System.out.println("   JNLP Codebase (absolute): "+_codebase);
                }
            } catch (Exception e) { _codebase=null; }

            if ( _codebase==null ) {
                try {
                    _codebase = URLUtil.asPathURL(new URL (URLUtil.getBase(_initDocumentBaseURL), codebaseStr));
                    if (DEBUG) {
                        System.out.println("   JNLP Codebase (documentbase+codebase): "+_codebase);
                    }
                } catch (Exception e) { _codebase=null; }
            }
            if ( DEBUG && _codebase==null ) {
                System.out.println("   JNLP Codebase (null)");
            }
        }

        if(DEBUG) {
            Trace.println("new JNLP2Manager: " + _initJnlpFile +
                          ", codebase: "+_codebase+
                          ", documentBase: "+_initDocumentBaseURL, TraceLevel.BASIC);
        }
    }

    protected Plugin2ClassLoader newClassLoader() {
        if (null == _codebase) {
            Exception e = new Exception("newClassLoader - init failed: _codebase is null");
            Trace.ignoredException(e);
            return null;
        }
        JNLPPreverifyClassLoader pcl = new JNLPPreverifyClassLoader(
                ClassLoader.getSystemClassLoader());
        JNLP2ClassLoader parent = new JNLP2ClassLoader(_codebase, pcl);

        // Only use callback if mixed code security enhancement enabled
        if (Config.getMixcodeValue() != Config.MIXCODE_DISABLE) {
            JNLP2ClassLoader child = new JNLP2ClassLoader(_codebase, parent);
            if (!Plugin2ClassLoader.setDeployURLClassPathCallbacks(parent, child)) {
                return parent;
            }
            return child;
        }
        else {
            return parent;
        }
    }

    /** Obtains a key in the form of a String which uniquely identifies an applet.
     *  The key contains document base and the JNLP filename.
     */
    public String getAppletUniqueKey() {
	String keyString = "|";

	if (_initDocumentBaseURL != null) {
	    keyString += _initDocumentBaseURL.toString();
	}
	keyString += "|";

	if (_initJnlpFile != null) {
	    keyString += _initJnlpFile;
	}
	keyString += "|";

	return keyString;
    }

    public void initialize() throws Exception 
    {
        super.initialize();

        Plugin2ClassLoader cl = getAppletClassLoader();

        if ( _INJECT_EXCEPTION_INIT ) {
            Exception e = new Exception("INJECT_JNLP2MANAGER_EXCEPTION_INIT");
            throw e;
        }

        if ( ! ( cl instanceof JNLP2ClassLoader ) ) {
            Exception e = new Exception("ClassLoader not JNLP2ClassLoader ("+cl+")");
            throw e;
        }

        if(!isAppletRelaunched()) {
            Config.refreshProps();
        }
        homeJREInfo = JREInfo.getHomeJRE(); // running javaws JRE

        if (DEBUG && VERBOSE) {
            Trace.println("JNLP2Manager.initialize(): java.home:"+Config.getJavaHome()+", RUnning JRE: "+homeJREInfo, TraceLevel.BASIC);
            Trace.println("JREInfos");
            JREInfo.traceJREs();
        }

        if (homeJREInfo == null) {
            // No running JRE  ??
            throw new ExitException(new Exception("Internal Error: no running JRE"), ExitException.LAUNCH_ERROR);
        }

        //check if we can do update check in background
	DeployPerfUtil.put("JNLP2Manager.initialize - before buildDescriptorFromCache()");
        LaunchDesc ld = LaunchDescFactory.buildDescriptorFromCache(_initJnlpFile, _codebase, _initDocumentBaseURL);
	DeployPerfUtil.put("JNLP2Manager.initialize - after buildDescriptorFromCache()");

        if (ld != null && ld.getUpdate().isBackgroundCheck()) {
  
            LocalApplicationProperties lap = Cache.getLocalApplicationProperties(ld.getCanonicalHome());
    
            if (lap == null || !lap.forceUpdateCheck()) {
                //can proceed in background mode => redirect in cache
                if (redirectLaunchDesc(ld, _initJnlpFile, _codebase,
                        _initDocumentBaseURL)) {
                    // redirectLaunchDesc will update _launchDesc if
                    // redirection occurs
                   _lap = Cache.getLocalApplicationProperties(
                           _launchDesc.getCanonicalHome());
                } else {
                    // no redirect, use ld
                    _launchDesc = ld;
                    _lap = lap;
                }
            } 
        }
        if (_launchDesc == null) {
	// FIXME: this LaunchDescFactory.buildDescriptor check if there is a newer jnlp file online than the
	// on in the cache. We need know if the jnlp is updated and update the desktop shortcut as well.
	// This is similar to what the javaws method updateFinalLaunchDesc() does.
            _launchDesc = LaunchDescFactory.buildDescriptor(
                    _initJnlpFile, _codebase, _initDocumentBaseURL, DEBUG);

            if (null != _launchDesc) {
                // update final _launchDesc if redirection occurs
                if (redirectLaunchDesc(
                        _launchDesc, _initJnlpFile, _codebase, _initDocumentBaseURL)
                        && DEBUG) {
                    Trace.println("JNLP2Manager.initialize(): JNLP redirect Ref: "
                            + _launchDesc.getLocation(), TraceLevel.BASIC);
                }
            } else {
                // cannot read jnlp file from network
                // use cache copy if we allow offline
                // use cache copy if jnlp file has no href
                if (_cachedJNLPFilePath != null) {
                    ld = LaunchDescFactory.buildDescriptor(
                            new File(_cachedJNLPFilePath), _codebase,
                            _initDocumentBaseURL, null);
                    if (ld != null && (ld.getLocation() == null ||
                            ld.getInformation().supportsOfflineOperation())) {
                        _launchDesc = ld;
                    }
                }
            }
        }

        if ( null == _launchDesc )
        {
            // we let `loadJarFiles()` throw the ExitException, to allow Plugin2Manager to show the exception properly
            Trace.println("JNLP2Manager.initialize(): JNLP not available: "+_initJnlpFile, TraceLevel.BASIC);
            return;
        } else if ( ! _launchDesc.isApplet() )
        {
            // we restrict a JNLP launch to Applet's
            // we let `loadJarFiles()` throw the ExitException, to allow Plugin2Manager to show the exception properly
            if (Trace.isTraceLevelEnabled(TraceLevel.BASIC))
                Trace.println("JNLP2Manager.initialize(): JNLP not an applet: "+_launchDesc, TraceLevel.BASIC);
            return;
        } else if (Environment.isImportMode()) 
        {
            // no import mode as an applet
            // we let `loadJarFiles()` throw the ExitException, to allow Plugin2Manager to show the exception properly
            if (Trace.isTraceLevelEnabled(TraceLevel.BASIC))
                Trace.println("JNLP2Manager.initialize(): JNLP import mode not supported: "+_launchDesc, TraceLevel.BASIC);
            return;
        }

        AppletDesc ad = _launchDesc.getAppletDescriptor();
        if( null==ad ) {
            Exception e = new Exception("initialize - init failed: AppletDesc is null"); 
            throw e;
        }

        _codebase     = _launchDesc.getCodebase();
        _initialized  = true;

        Applet2ExecutionContext exeCtx = getAppletExecutionContext();
        exeCtx.setAppletParameters(JNLP2Tag.addJNLParams2Map(exeCtx.getAppletParameters(), ad));

        // further JNLP specific initialization, ie: final LaunchDesc, _appPolicy, cache ..
        prepareToLaunch(); 

        if( null==_appPolicy ) {
            // internal check of code consitency, should never happen
            Exception e = new Exception("initialize - init failed: _appPolicy is null"); 
            throw e;
        }
    }

    //----------------------------------------------------------------------
    // Loading of jar files
    //

    // FIXME: change this to use proper localization of Trace messages
    class DefaultDownloadProgress implements ProgressListener {
        private Set/*<URL>*/ validatingJars = new HashSet();
        private Set/*<URL>*/ patchingJars   = new HashSet();

        public void jreDownload(String version, URL location) {
            Trace.println("Downloading JRE " + version + " from " + 
                          location + "...", TraceLevel.NETWORK);
        }

        public void extensionDownload(String name, int remaining) {
            // ignore
        }

        public void progress(URL url, String version, long readSoFar, 
                             long totalSize, int overallPercent) {
            if (totalSize != -1) {
                setGrayBoxProgress(overallPercent / 100.0f);
            }
        }

        public void validating(URL url, String version, long entry, 
                               long totalSize, int overallPercent) {
            if (validatingJars.add(url)) {
                Trace.println("Validating " + url + " , version " + 
                              version + "...", TraceLevel.NETWORK);
            }
        }

        public void upgradingArchive(URL url, String version, int patchPercent,
                             int overallPercent) {
            if (patchingJars.add(url)) {
                Trace.println("Patching " + url + " , version " + 
                               version + "...", TraceLevel.NETWORK);
            }
        }

        public void downloadFailed(URL url, String version) {
            Exception e =
                new RuntimeException("Download failed for URL " + url +
                                     " , version " + version + "...");
            showAppletException(e);
        }

        // ExtensionInstallerService UI part

        public void setHeading(final String text, final boolean singleLine) {
        }
        public void setStatus(String text) {
        }
        public Component getOwner() {
            return null;
        }
        public void setVisible(final boolean show) {
            if (show) {
                setGrayBoxProgress(0.0f);
            }
        }
        public void setProgressBarVisible(final boolean isVisible) {
        }
        public void setProgressBarValue(final int value) {
        }
        public void showLaunchingApplication(final String title) {
        }
    }

    private ProgressListener defaultDownloadProgress;
    private ProgressListener getDownloadProgress() {
        if (Progress.usingCustomProgress()) {
            return Progress.getCustomProgress();
        }
        if (defaultDownloadProgress == null) {
            defaultDownloadProgress = new DefaultDownloadProgress();
        }
        return defaultDownloadProgress;
    }

    protected void loadJarFiles() throws ExitException 
    {
        if ( _INJECT_EXCEPTION_LOADJARFILES ) {
            ExitException e = new ExitException(new Exception("INJECT_JNLP2MANAGER_EXCEPTION_LOADJARFILES"),
                                                ExitException.LAUNCH_ERROR);
            throw e;
        }
        getDownloadProgress().setVisible(true);

        if (!_initialized) {
            // let's handle incomplete 'initialize' errors first ..
            if ( null == _launchDesc )
            {
                ExitException ee = new ExitException(new FileNotFoundException("JNLP file error: "+_initJnlpFile+
									       ". Please make sure the file exists and check if \"codebase\" and \"href\" in the JNLP file are correct."), 
                                                     ExitException.LAUNCH_ERROR);
                throw ee;
            } else if ( ! _launchDesc.isApplet() )
            {
                ExitException ee = new ExitException(new LaunchDescException(_launchDesc, "JNLP not an applet",
                                                         new Exception("JNLP not an applet")),
                                                     ExitException.LAUNCH_ERROR);
                throw ee;
            } else if (Environment.isImportMode()) 
            {
                ExitException ee = new ExitException(new LaunchDescException(_launchDesc, "JNLP import mode not supported", 
                                                         new Exception("JNLP import mode not supported")),
                                                     ExitException.LAUNCH_ERROR);
                throw ee;
            }
            // should never happen, but double check
            ExitException ee = new ExitException(new LaunchDescException(_launchDesc, "JNLP2Manager not initialized", 
                                                     new Exception("JNLP2Manager not initialized")),
                                                 ExitException.LAUNCH_ERROR);
            throw ee;
        }

        try {
            if ( _INJECT_FOREVER_LOADJARFILES ) {
                System.out.println("INJECT_JNLP2MANAGER_FOREVER_LOADJARFILES");
                int i=999999;
                while (i>0) {
                    if(--i<999) i=999999;
                }
            }
            prepareLaunchFile(_launchDesc);
            //jars are loaded, now can start background update check without risk to load 
            //things more than once
             _launchDesc.getUpdater().startBackgroundUpdateOpt();
        } catch (Throwable t) {
            ExitException ee = (t instanceof ExitException) ? (ExitException) 
                t : new ExitException(t, ExitException.LAUNCH_ERROR);
            if (ee.getReason() != ExitException.OK) {
                throw ee; // propagate the exception, if it's not an OK one
            }
        }
    }

    // Override how we compute the codebase for JNLP-launched applets
    protected URL getCodeBase() {
        return _codebase;
    }

    // Override how we find the name of JNLP-launched applets
    protected String getCode() {
        if (  null!=_launchDesc && _launchDesc.isApplet() ) {
            // For JNLP-launched applets we get the name of the main class
            // from the main-class attribute in the applet-desc, and
            // ignore the "code" attribute, if any, in the applet tag
            return _launchDesc.getAppletDescriptor().getAppletClass();
        } else {
            return "<applet error>";
        }
    }

    protected void destroyAppContext(final AppContext fac,
                                     final Applet2StopListener stopListener,
                                     long timeToWait) {

        // this is to shutdown any thread pool we used during loading if it is not
        // shut down yet
        // FIXME: we need a general solution for hook up clean up actions during AppContext disposal

        Object obj = fac.get(LaunchDownload.APPCONTEXT_THREADPOOL_KEY);
        // for jre prior to 1.5, the obj will be null
        if (obj != null) {
            ExecutorService pool = (ExecutorService)obj;
            pool.shutdown();
        }

        super.destroyAppContext(fac, stopListener, timeToWait);
    }

    protected void appletSSVRelaunch() throws JRESelectException {
	if (_relaunchException != null)  throw _relaunchException;
    }

    protected void checkRunningJVMArgsSatisfying() throws JRESelectException {
	LaunchSelection.MatchJREIf jreMatcher = _launchDesc.getJREMatcher();
	if ( !jreMatcher.isRunningJVMArgsSatisfying(_allSigned) ) {
	    // relaunch in the current version of JRE with the selected vm args
	    if (_relaunchException != null) {
		throw new JRESelectException(null, _relaunchException.getJVMArgs());
	    } 
	}

	// continue launch in this jvm
    }

    public LaunchDesc getLaunchDesc() {
	return _launchDesc;
    }

    //----------------------------------------------------------------------
    // jnlp descriptors/files management
    //

    /**
     * updates the _launchDesc if its href has a redirection.
     *
     * @param ld the initail launch desc built from applet jnlp_href
     * @param jnlpHRef the applet jnlp_href, may be absolute, or relative
     * @param codebase the absolute applet codebase URL
     * @param documentbase the applet html documentbase. 
     * @return true if a href redirction occurs
     */
    private boolean redirectLaunchDesc (LaunchDesc ld, String jnlpHRef, 
					URL codebase ,URL documentbase)
	throws ExitException {
	try {
	    URL newHref = ld.getLocation();
	    if (newHref != null) {
		// check if the newHref is a redirection. If not, there is no
		// need to download the same jnlp again.
		
		// FIXME: most code is duplicated from LaunchDescFactory.buildDescriptor()
		// consider refactoring the code to a method to check the type
		// of jnlpHRef. 

		// 1. jnlpHRef is absolute url
		
		URL url = null;
		try {
		    url = new URL(jnlpHRef);
		} catch (Exception e) { 
		    if(DEBUG) { System.out.println(e); e.printStackTrace(); } 
		    url=null; 
		}

		if (url != null && url.toString().equals(newHref.toString())) {
		    return false;
		}
		
		// 2. try codebase + jnlp_href
		if(codebase!=null) {
		    try {
			url = new URL(codebase, jnlpHRef);
		    } catch (Exception e) { 
			if(DEBUG) { System.out.println(e); e.printStackTrace(); } 
			url=null; 
		    }
		    
		    if ( url!=null && url.toString().equals(newHref.toString())) {
			return false;
		    }
		}
		
		// 3: try documentbase + jnlp_href
		if(codebase==null && documentbase!=null) {
		    try {
			url = new URL(URLUtil.getBase(documentbase), jnlpHRef);
		    } catch (Exception e) { 
			if(DEBUG) { System.out.println(e); e.printStackTrace(); } 
			url=null; 
		    }
		    
		    if ( url!=null && url.toString().equals(newHref.toString())) {
			return false;
		    }
		    
		}

		// _launchDesc has a href redirection
		// get the new _launchDesc.
		// We only redirect one level
		
		_launchDesc = LaunchDescFactory.buildDescriptor(newHref.toString(), codebase, 
								documentbase, DEBUG);

		// FIXME: if newHref has a different codebase than the current _codebase
		// need update _codebase because this is going to be saved in LAP if dragged
		// out of browser.
		return true;
	    } else {
		// no href. unlikely to happen in plugin because there must be a jnlp_href
		// and we infer from applet tag codebase, docbase to infer an absolute jnlp
		// ref location. 
		return false;
	    }
	} catch (Exception e) {
	    if (DEBUG)  { System.out.println(e); e.printStackTrace(); } 
	    throw new ExitException(e, ExitException.LAUNCH_ERROR);
	}
    }

    private void prepareToLaunch() throws ExitException
    {
        try {
            _lap = Cache.getLocalApplicationProperties(_launchDesc.getCanonicalHome());

            URL href = _launchDesc.getLocation();
            if (href != null) {
                Cache.removeRemovedApp(href.toString(), 
                    _launchDesc.getInformation().getTitle());
            }

            // See if the JNLP file properties contains any debugging properties
            if (_launchDesc.getResources() != null) {
                Globals.getDebugOptionsFromProperties(
                    _launchDesc.getResources().getResourceProperties());
            }

            // Initialize the App policy stuff
            _appPolicy = AppPolicy.createInstance(_launchDesc.getCanonicalHome().getHost());

        } catch (Throwable t) {
            ExitException ee = (t instanceof ExitException) ? (ExitException) 
                t : new ExitException(t, ExitException.LAUNCH_ERROR);
            int eeReason = ee.getReason();
            if (eeReason == ExitException.LAUNCH_ERROR) {
                // Try to showAppletException
                // At this stage, the graybox listener is not available yet, no UI box.
                showAppletException(ee.getException());
            }
            if (eeReason != ExitException.OK) {
                throw ee; // propagate the exception, if it's not an OK one
            }

            // do not initialize the JNLP2ClassLoader yet - wait until
            // we have downloaded all the jnlp files,o we
            // we have a complete list of resources.
        }
    }

    // final act ..
    //  - no JRE verification and launch, since this is part of the client/server communication
    //
    private void prepareLaunchFile(LaunchDesc ld) throws ExitException 
    {
        boolean jreInstalled = false;

        // Check that at least some resources was specified
        if (ld.getResources() == null) {
            handleJnlpFileException(ld, new LaunchDescException(ld,
                ResourceManager.getString("launch.error.noappresources",
                ld.getSpecVersion()), null));
        }

        // Check that a JRE is specified in the root JNLP applet
        if (!ld.isJRESpecified()) {
            LaunchDescException lde = new LaunchDescException(ld,
                ResourceManager.getString(
                    "launch.error.missingjreversion"), null);
            handleJnlpFileException(ld, lde);
        }

        // Initialize exception with the launch descriptor
        JNLPException.setDefaultLaunchDesc(ld);
        
        // pop up error dialog and exit if system is in offline mode and jnlp
        // file does not allow offline operation
        if (ld.getInformation().supportsOfflineOperation() == false && 
                DeployOfflineManager.isGlobalOffline() == true) {
            throw (new ExitException(new OfflineLaunchException(
                    OfflineLaunchException.NO_OFFLINE_ALLOWED), 
                    ExitException.LAUNCH_ERROR));
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

        // We always use the JVM relaunch mechanism,
        // and don't want to set properties by com.sun.javaws.security.AppPolicy
        ld.setPropsSet(true);

        // Setup JNLPClassLoader 
        JNLP2ClassLoader cl = (JNLP2ClassLoader) getOrCreatePlugin2ClassLoader(); // implicite: _launchDesc, _appPolicy
 
        //
        // Download all nested ressources
        //

        // Keep track of all JNLP files for installers
        ArrayList installFiles = new ArrayList();
        boolean installerRelaunch = false;
        boolean skipExtensions = false;

        // In case of:
        //  - not using the background update check, and
	//  - not using "prompt" update check policy
        //  - not having relaunched
        // we perform an updateCache right away.
        // Otherwise, only missing ressources are being downloaded
        boolean updateCache = 
	    !ld.getUpdate().isBackgroundCheck() &&
	    !ld.getUpdate().isPromptPolicy() &&
	    !isAppletRelaunched();

        // we need all extension jnlp files downloaded now if not there
        boolean allInCache = false;
        if (ld.getUpdate().isBackgroundCheck() && (_lap == null ||
                !_lap.forceUpdateCheck())) {
            //otherwise we need update check and it has no sense
            // to try to load files from cache now
            allInCache = LaunchDownload.isInCache(ld);
        }

        if (!allInCache) {
            try {
                LaunchDownload.downloadExtensions(ld, null, 0, installFiles);
            } catch (Exception ioe) {
                if (ld.getInformation().supportsOfflineOperation() &&
                        LaunchDownload.isInCache(ld)) {
                    allInCache = true;
                } else {
                    throw (new ExitException(ioe, ExitException.LAUNCH_ERROR));
                }
            }
        }
        
        skipExtensions = true;

        //
        // now we have list of all resources - we can initialize ClassLoader
        //
        cl.initialize(ld, _appPolicy);

        // if launch file(s) indicate a custom progress implementation
        // prepare that before other downloads
        ResourcesDesc rd = ld.getResources();
        if (rd != null) {
            JARDesc pjd = rd.getProgressJar();
            if (pjd != null) {
                Trace.println("Custom Progress jar found", TraceLevel.NETWORK);
                String progressClass = ld.getProgressClassName();
                if (progressClass != null) {
                    Trace.println("Custom Progress class found: " +
                        progressClass, TraceLevel.NETWORK);
                    prepareCustomProgress(ld, pjd, progressClass);
                } else {
                    Trace.println("No Custom Progress classname found",
                                  TraceLevel.NETWORK);
                }
            } else {
                Trace.println("No Custom Progress jar", TraceLevel.NETWORK);
            }
        }
	
	// this boolean indicates all resources are in cache and updated
	// When the cache is empty or some resources are not in the cache,
	// a full download (implicitly update) is performed. We set allUpdated
	// to true after that. And no update check is needed.
	boolean allUpdated = false;
        allUpdated = downloadResources(cl, ld, installFiles, allInCache,
                updateCache, skipExtensions);

        ld.selectJRE(); // re-select after all is in place

        if(allUpdated) {
            // if we have performend an update check, we can clear the lap flag
            resetForceUpdateCheck();
        } 

        // passing this point, we expect all resources are in cache (either 
	// updated or not)

        Trace.println("LaunchDesc location: " + jnlpUrl, TraceLevel.BASIC);

        LaunchSelection.MatchJREIf jreMatcher = ld.getJREMatcher();
        if (DEBUG && VERBOSE) {
            Trace.println("JNLP2Manager.prepareLaunchFile: JRE matcher:");
            Trace.println(jreMatcher.toString());
        }

        // is the current JRE satisfactory (version and args) ?
        boolean homeJVMMatch = jreMatcher.isRunningJVMSatisfying(true); 

        boolean secureArgs   = jreMatcher.getSelectedJVMParameters().isSecure();

        // Get the JREInfo of the selected and installed JRE
        JREInfo jreInfo = ld.getSelectedJRE();

	// we may have installed this version since plugin was initialized
	if (jreInfo == null) {
	  
	    Vector list = Config.getInstance().getInstalledJREList();
	    if (list!=null) {
		Config.storeInstalledJREList(list);
	    }

	    if (DEBUG && VERBOSE) {
		Trace.println("JNLP2Manager.prepareLaunchFile: Refreshed JREInfo list:");
		JREInfo.traceJREs();
	    }

	    jreInfo = ld.selectJRE();

	    if (jreInfo != null) {
		if (DEBUG && VERBOSE) {
		    Trace.println("JNLP2Manager.prepareLaunchFile: Found a new match JRE:");
		    Trace.println(jreInfo.toString());
		}
		jreInstalled = true;
	    }
		
	} 

        // Get the JREDesc of the selected JRE
        JREDesc jreDesc = ld.getResources().getSelectedJRE();	

        if (jreInfo==null && jreDesc == null) {
            Trace.println(jreMatcher.toString());
            throw new ExitException(new Exception("Internal Error: Internal error, no JREDesc and no JREInfo"), 
                                    ExitException.LAUNCH_ERROR);
        }

        // Need to update if not offline and either:
        //  - no local JRE is found
        //  - the preferences is to always do the check
        //  - the application properties says that we should do an update
        boolean forceUpdate = (jreInfo==null);
        
        boolean offlineMode = DeployOfflineManager.isGlobalOffline();

        if (forceUpdate && offlineMode) {
            throw (new ExitException(new OfflineLaunchException(
                    OfflineLaunchException.MISSING_RESOURCE),
                    ExitException.LAUNCH_ERROR));
        }

        boolean needUpdate = forceUpdate;

	// No need to check updates if all resources are
	// already updated or the check is set to be in bg
	// or jre missing or applet relaunch or offline
        if(!forceUpdate && !allUpdated &&
	   !ld.getUpdate().isBackgroundCheck() && 
	   !isAppletRelaunched() && !offlineMode)
	{   
            // check if any updates need checked in the FOREGROUND

            // the force update check flag is set by UpdateChecker if 
            // the last update check timed out or cancelled or error happens
            if (_lap != null && _lap.forceUpdateCheck()) {
                if (DEBUG) {
                    Trace.println("Forced update check in LAP, do full update", TraceLevel.BASIC);
                }
                needUpdate = true;
            } else {
                try {
                    needUpdate = ld.getUpdater().isUpdateAvailable();
                } catch (Exception e) {
                    throw (new ExitException(e, ExitException.LAUNCH_ERROR));
                }
                
                if(ld.getUpdater().isCheckAborted()) {
                    throw (new ExitException(new LaunchDescException(ld, "User rejected cert - aborted", null),
                                             ExitException.LAUNCH_ABORT_SILENT));
                }
            }
        }
        
        if(DEBUG) {
            Trace.println(
                "\n\tisRelaunch: " + isAppletRelaunched() +
                "\n\tOffline mode: " + offlineMode +
                "\n\tforceUpdate: " + forceUpdate +
                "\n\tneedUpdate: " + needUpdate +
                "\n\tbgrUpdCheck: " + ld.getUpdate().isBackgroundCheck() +
                "\n\tbgrUpdThread: " + ld.getUpdater().isBackgroundUpdateRunning() +
                "\n\tRunning  JREInfo: " + homeJREInfo +
                "\n\t"+jreMatcher 
                , TraceLevel.BASIC);
        }

        if (needUpdate && !forceUpdate) {
	    // There is update available, check the policy (always, prompt, etc.)
	    // to decide whether to do update before launch
            forceUpdate = ld.getUpdater().needUpdatePerPolicy();
        }

        if (forceUpdate) {
            /**
             * JRE Autoinstall missing JRE version to cache
             */
            if (!isAppletRelaunched() && jreInfo==null ) {
                downloadJREResource(ld, jreDesc, installFiles, getDownloadProgress());
                if (!installFiles.isEmpty()) {
                    // execute the jre installer
                    try {
                        JnlpxArgs.executeInstallers(installFiles, getDownloadProgress());
                    } catch (ExitException ee) {
                        installerRelaunch= (ee.getReason() == ExitException.REBOOT);
                        if (!installerRelaunch && ee.isErrorException()) {
			    // if error happens, e.g. cancellation of install
			    // throw EE.OK to run in the default JRE
			    // This is to be consistent with cancellation of Security
			    // Warning Dialog.
			    throw (new ExitException(null, ExitException.OK));
                        }
                        Trace.ignoredException(ee); 
                    }
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

                jreInstalled = true;
            } else {
                // All ressources are guaranteed to be uptodate after this point.
                downloadResources(cl, ld, installFiles, allInCache, true /* updateCache */,
                        skipExtensions);
            }

            // Reset force update
            resetForceUpdateCheck();
        }

        //if custom progress is used we need to wait for it to be loaded
        //before we can go and validate JNLP signing constrains
        //(progress jar == "super eager")
        //(same code is replicated in the Launcher)
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

        if( jreInstalled && homeJVMMatch) {
            throw new ExitException(new Exception("Internal Error: jreInstalled, but homeJVM matches"), ExitException.LAUNCH_ERROR);
        }

        // most everything else waits till we have a JRE unless just importing
        if (homeJVMMatch) {
            if (DEBUG && VERBOSE) {
                Trace.println("JNLP2Manager.prepareLaunchFile(): SingleInstanceManager ?: "+ ld.getCanonicalHome().toString());
            }
            // check if there is another instance running
            if (SingleInstanceManager.isServerRunning(
                    ld.getCanonicalHome().toString())) {
                String[] appArgs = Globals.getApplicationArgs();

                if (DEBUG && VERBOSE) {
                    Trace.println("JNLP2Manager.prepareLaunchFile(): SingleInstanceManager: Running with appArgs: "+appArgs
                                +", thread: "+Thread.currentThread());
                }

                if (appArgs != null) {
		    ApplicationDesc applicationDesc = ld.getApplicationDescriptor();
		    // FIXME: in the context of an applet, the 
		    // applicationDesc will be null. For now, adding a null check
		    // for applicationDesc to avoid NPE. We need to investigate how to 
		    // setup AppletDesc for the applet case and whether it makes sense 
		    // to add a setArguments method to AppletDesc.
		    if (applicationDesc != null) {
			applicationDesc.setArguments(appArgs);
		    }
                }

                // send the JNLP file to the server port
                if ( SingleInstanceManager.connectToServer( ld.toString() ) ) 
                {
                    if (DEBUG && VERBOSE) {
                        Trace.println("JNLP2Manager.prepareLaunchFile(): SingleInstanceManager: OK from server");
                    }

                    // if we get OK from server, we are done
                    String msg = "Single Instance already exist: "+ ld.getCanonicalHome().toString();
                    Exception e = new Exception(msg);
                    setErrorOccurred(msg, e);
                    throw new ExitException(e, ExitException.LAUNCH_SINGLETON);
                } else if (DEBUG && VERBOSE) {
                    // else continue normal launch
                    Trace.println("JNLP2Manager.prepareLaunchFile(): SingleInstanceManager: NOK from server");
                }
            } else if (DEBUG && VERBOSE) {
                Trace.println("JNLP2Manager.prepareLaunchFile(): No SingleInstanceManager, thread: "+Thread.currentThread());
            }

            //
            // Run all installers if any
            //
            if (!installFiles.isEmpty()) {
                try {
                    JnlpxArgs.executeInstallers(installFiles, getDownloadProgress());
                } catch (ExitException ee) {
                    if (ee.getReason() != ExitException.REBOOT) {
                        throw ee; // propagate unexpected exception ..
                    }
                    installerRelaunch=true;
                }
            }
        }

        // check that the JNLP Spec version is valid
        if (!isValidSpecificationVersion(ld)) {
            JNLPException.setDefaultLaunchDesc(ld);
            handleJnlpFileException(ld, new LaunchDescException(ld,
                ResourceManager.getString("launch.error.badjnlversion",
                    ld.getSpecVersion()), null));
        }
    
	boolean homeJVMVersionMatch = jreMatcher.isRunningJVMVersionSatisfying();
	String bestJREVersion = null;
	if (!homeJVMVersionMatch) {
	    bestJREVersion = fireGetBestJREVersion(jreDesc.getVersion());
	}
	if (bestJREVersion == null) {
	    // this in unusual, server at least should return the latest version
	    bestJREVersion = jreDesc.getVersion();
	}

        /** 
         * JRE abort: C/S roundtrip to relaunch with new JRE
         */
	JREDesc bestJreDesc = null;
        if ( ld.isSecureJVMArgs() && 
	     (installerRelaunch ||      // the installer demands a relaunch
              !homeJVMMatch              // the currently running JRE doesn't satisfy (version and/or args)
	      )
	     || _INJECT_APPLET_RELAUNCH)
	{
            if(jreInstalled) {
                // earmark that exactly this has happend to PluginMain.appletJRERelaunch(),
                // so it can propagate this flag to the server.
                setParameter(ParameterNames.JRE_INSTALLED, String.valueOf(true));
            }

	    if (!homeJVMVersionMatch) {
		// earmark the best matched requested JRE version               
                setParameter(ParameterNames.SSV_VERSION, bestJREVersion);
		bestJreDesc = new JREDesc(bestJREVersion, 0, 0, jreDesc.getVmArgs(), null, null);
	    } 

	    if(DEBUG) {
		Trace.println("JRESelectException(1): installerRelaunch: "+installerRelaunch, TraceLevel.BASIC);
		Trace.println("JRESelectException(1): jreInstalled: "+jreInstalled, TraceLevel.BASIC);
		Trace.println("JRESelectException(1): running JREInfo: "+homeJREInfo, TraceLevel.BASIC);
		Trace.println("JRESelectException(1): "+jreMatcher, TraceLevel.BASIC);
	    }
	    if(!isAppletRelaunched()) {
		// no sepereator, no internals, !os-conform, !secure
		List/*String*/ paramList = jreMatcher.getSelectedJVMParameters().getCommandLineArguments(false, false, false, false, 
													 Config.getMaxCommandLineLength());
		String params = StringQuoteUtil.getStringByCommandList(paramList);
		if (fireAppletRelaunchSupported()) {
		    JRESelectException e = new JRESelectException(bestJreDesc, params);
		    if (homeJVMVersionMatch) {
			throw e; // C/S roundtrip to restart new JRE
		    } else {
			// jre version mismatch, delay relaunch to SSVValidation
			_relaunchException = e;
		    }
		} else {
		    Trace.println("JRESelectException(1): ignored - relaunch not supported", TraceLevel.BASIC);
		}
	    } else {
		Trace.println("JRESelectException(1): ignored - relaunched already", TraceLevel.BASIC);
	    }
        }

        boolean allSigned = false;

        try {
            /*
             * Check is resources in each JNLP file is signed and prompt for
             * certificates. This will also check that each resources downloaded
             * so far is signed by the same certificate.
             */
            allSigned = LaunchDownload.checkSignedResources(ld);

            // Check signing of all JNLP files
            LaunchDownload.checkSignedLaunchDesc(ld);

            // allSigned now means main jnlp file also
            allSigned = allSigned && ld.isSigned();
	    _allSigned = allSigned;
        }  catch(JNLPException je) {
            throw new ExitException(je, ExitException.LAUNCH_ERROR);
        } catch(IOException ioe) {
            // This should be very uncommon
            throw new ExitException(ioe, ExitException.LAUNCH_ERROR);
        } catch(Exception e) {
            // This should be very uncommon
            throw (new ExitException(e, ExitException.LAUNCH_ERROR));
        }

        Trace.println("passing security checks; secureArgs:"+ld.isSecureJVMArgs()+", allSigned:"+allSigned, TraceLevel.BASIC);

        // match all jvm args and parameters if allSigned, or just
        // need to match secure jvm args and parameters if not
        homeJVMMatch = jreMatcher.isRunningJVMSatisfying(allSigned);

        if (( installerRelaunch ||   // the installer demands a relaunch
             !homeJVMMatch           // the currently running JRE doesn't satisfy (version and/or args)
	       )
	    || _INJECT_APPLET_RELAUNCH)
         {
            if(jreInstalled) {
                // earmark that exactly this has happend to PluginMain.appletJRERelaunch(),
                // so it can propagate this flag to the server.
                setParameter(ParameterNames.JRE_INSTALLED, String.valueOf(true));
            }
	    if (!homeJVMVersionMatch) {
		// earmark the best matched requested JRE version               
                setParameter(ParameterNames.SSV_VERSION, bestJREVersion);
		bestJreDesc = new JREDesc(bestJREVersion, 0, 0, jreDesc.getVmArgs(), null, null);
	    } 
            if(DEBUG) {
                Trace.println("JRESelectException(2): installerRelaunch: "+installerRelaunch, TraceLevel.BASIC);
                Trace.println("JRESelectException(2): jreInstalled: "+jreInstalled, TraceLevel.BASIC);
                Trace.println("JRESelectException(2): running JREInfo: "+homeJREInfo, TraceLevel.BASIC);
                Trace.println("JRESelectException(2): "+jreMatcher, TraceLevel.BASIC);
            }
            if(!isAppletRelaunched()) {
                // no sepereator, no internals, !os-conform, include-secure:=<secureArgs>
                List/*String*/ paramList = jreMatcher.getSelectedJVMParameters().getCommandLineArguments(false, false, false, allSigned,
                                                                                                Config.getMaxCommandLineLength());
                String params = StringQuoteUtil.getStringByCommandList(paramList);
		if (fireAppletRelaunchSupported()) {
		    JRESelectException e = new JRESelectException(bestJreDesc, params);
		    if (homeJVMVersionMatch) {
			throw e; // C/S roundtrip to restart new JRE
		    } else { 
			// jre version mismatch, delay relaunch to SSVValidation
			_relaunchException = e;
		    }
		} else {
		    Trace.println("JRESelectException(2): ignored - relaunch not supported", TraceLevel.BASIC);
		}
            } else {
                Trace.println("JRESelectException(2): ignored - relaunched already", TraceLevel.BASIC);
            }
        }

        Trace.println("continuing launch in this VM", TraceLevel.BASIC);

        ResourcesDesc resources = ld.getResources();
        if (resources != null) {
            JARDesc[] allJarDescs = resources.getEagerOrAllJarDescs(true);
            for(int i = 0; i < allJarDescs.length; i++) {
                storeJarVersionMapInAppContext(allJarDescs[i]);
            }
        }
    }

    /**
     * @param updateCache   if true, we try to download updated resources, even if they are already in cache,
     *                      otherwise we only download resources, if they are not in the cache already.
     *
     * @return true if we have downloaded resources and therefor implicitly made an update check,
     *         otherwise false (all resources are in cache and no update check was performed).
     */
    private boolean downloadResources(JNLP2ClassLoader cl, LaunchDesc ld, 
            ArrayList installFiles, boolean allInCache,
            boolean updateCache, boolean skipExtensions)
        throws ExitException {
        boolean updated     = false;

        boolean doDownload = updateCache || !allInCache;

        if (DEBUG) {
            Trace.println("JNLP2Manager.downloadResources(): updateCache "+updateCache+
                          ", allInCache "+allInCache+", doDownload "+doDownload, TraceLevel.BASIC );
        }

        if(!doDownload) {
            return updated;
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
            //      (see Launcher.java)
            if (!skipExtensions)
                LaunchDownload.downloadExtensions(ld, getDownloadProgress(), 0, installFiles);
            
            // I really don't think the following call, or the method in 
            // JNLP2ClassLoader is needed anymore - FIXME - confirm and remove.
            cl.updateJarDescriptors(ld.getResources());

            // NOTE that we DO NOT enforce the so-called JNLP security
            // restrictions in this implementation
            // (LaunchDownload.checkJNLPSecurity()). They are very
            // broken and do not allow for valid use cases such as an
            // unsigned applet referring to an extension hosted on
            // another web server, which works fine with normal
            // applets. We avoid security holes by not consulting the
            // JNLP file when granting permissions to the loaded
            // classes, and only grant elevated permissions based on
            // whether the code is signed and trusted.
            
            // Download all eagerly needed resources (what is in the cache 
            // may not be ok, should do a check to the server)   
            LaunchDownload.downloadEagerorAll(ld, false, 
                                              getDownloadProgress(), false);
            updated = true;
        } catch(SecurityException se) {
            // This error should be pretty uncommon. Most would have already 
            // been wrapped in a JNLPException by the downloadJarFiles method.
            throw (new ExitException(se, ExitException.LAUNCH_ERROR));
        } catch(JNLPException je) {
            // FIXME: In LaunchDownload, IOE could be wrapped into a JNLPException
            throw (new ExitException(je, ExitException.LAUNCH_ERROR));
        } catch(IOException ioe) {
            if (ld.getInformation().supportsOfflineOperation() && allInCache) {
                Trace.ignoredException(ioe);
                return updated;
            }
            throw (new ExitException(ioe, ExitException.LAUNCH_ERROR));
        }
        return updated;
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
    
    private void handleJnlpFileException (LaunchDesc ld,
                Exception exception) throws ExitException {
        /* purge the bad jnlp file from the cache: */       
        DownloadEngine.removeCachedResource(ld.getCanonicalHome(),
                null, null);
       
        throw new ExitException(exception, ExitException.LAUNCH_ERROR);
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


    //----------------------------------------------------------------------
    // jre management
    //

    private void downloadJREResource(LaunchDesc ld, JREDesc jreDesc, 
            ArrayList installFiles, ProgressListener progress) 
            throws ExitException {

        Trace.println("downloadJREResource ...", TraceLevel.BASIC);

        // verify if it's ok to download

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
            if (SecureStaticVersioning.promptDownload(progress.getOwner(), ld,
                _lap, replyVersion, source) == false) {
                // user chose not to download the required JRE
                throw (new ExitException(null, ExitException.OK));
            }
        } else if ((pref != null) && (pref.equalsIgnoreCase(
                            Config.AUTODOWNLOAD_MODE_PROMPT))) {
            if (AutoDownloadPrompt.prompt(progress.getOwner(), ld) == false) {
                // user chose not to download the required JRE
                throw (new ExitException(null, ExitException.OK));
            }
        }

        // ok .. go ahead with the actual download 
    
        // Show loading progress window. We only do this the first time
        progress.setProgressBarVisible(true);
        progress.setVisible(true);

        // Download jre jnlp file
        try {
            if (Cache.isCacheEnabled()) {
                LaunchDownload.downloadJRE(ld, progress, installFiles);
            } else {
                throw new IOException("Cache disabled, cannot download JRE");
            }
        } catch(SecurityException se) {
            // This error should be pretty uncommon. Most would have already 
            // been wrapped in a JNLPException by the downloadJarFiles method.
            throw (new ExitException(se, ExitException.LAUNCH_ERROR));
        } catch(JNLPException je) {
            throw (new ExitException(je, ExitException.LAUNCH_ERROR));
        } catch(Exception e) {
            // if you fail to download a jre - use a differant error
            Trace.ignored(e);
            throw (new ExitException(new NoLocalJREException(ld,
                ld.getResources().getSelectedJRE().getVersion(),
                    false), ExitException.LAUNCH_ERROR));
        }
        Trace.println("downloadJREResource fin", TraceLevel.BASIC);
    }

    //----------------------------------------------------------------------
    // jar management
    //

    /*
     * initialize the jar version/preload map
     */

    private void storeJarVersionMapInAppContext(JARDesc jardesc) {
        if(null==jardesc || null==appletAppContext) return;

        URL url = jardesc.getLocation();
        
        if (url != null) {
            appletAppContext.put(Config.getAppContextKeyPrefix() +
                                 url.toString(), jardesc.getVersion());
        }
    }

    /**
     * Return the list of applicaton and extension jnlps and 
     * list of jars if specified.
     * Otherwise return null.
     */
    protected String getCodeSourceLocations() 
    {
        if (!_initialized) return null;

	ArrayList list = new ArrayList();

	list.add(_launchDesc);

        ResourcesDesc resources = _launchDesc.getResources();
	
        if (resources != null) {
	    list.addAll(Arrays.asList(resources.getEagerOrAllJarDescs(true)));
	    list.addAll(Arrays.asList(resources.getExtensionDescs()));
	}

	return buildJarList(list.toArray(new Object[list.size()]));
    }

    /**
     * Return the list of jar files if specified.
     * Otherwise return null.
     */
    protected String getJarFiles() 
    {
        if (!_initialized) return null;

        ResourcesDesc resources = _launchDesc.getResources();

        if (resources != null) {
            return buildJarList(resources.getEagerOrAllJarDescs(true));
        }
        return null;
    }

    protected static String buildJarList(Object[] resources) {
        if (resources == null) {
            return null;
        }

        StringBuffer buf = null;
        boolean needComma = false;
        for (int i = 0; i < resources.length; i++) {
	    String str;
	    if (resources[i] instanceof LaunchDesc) {
		str = ((LaunchDesc) resources[i]).getLocation().toString();
	    } else if (resources[i] instanceof JARDesc) {
		str = ((JARDesc)resources[i]).getLocation().toString();
	    } else if (resources[i] instanceof ExtensionDesc) {
		str = ((ExtensionDesc)resources[i]).getLocation().toString();
	    } else {
		continue;
	    }

            if (str != null) {
                if (buf == null)
                    buf = new StringBuffer();
                if (needComma)
                    buf.append(",");
                buf.append(str);
                needComma = true;
            }
        }

        if (buf == null)
            return null;
        return buf.toString();
    }

    /**
     * Initializes the static execution environment.
     * once in a JRE lifetime
     * 
     */
    static public void initializeExecutionEnvironment() throws JNLPException
    {
        if (_environmentInitialized) {
            return ;
        }

        long t0 = DeployPerfUtil.put(0, "JNLP2Manager - initializeExecutionEnvironment() - BEGIN");

        // make sure the cache is writable
        if (!Cache.canWrite()) {
            CacheAccessException e = new CacheAccessException(Environment.isSystemCacheMode());
            throw e;
        }

        Properties p = System.getProperties();

        // Setup browser support if necessary
        // TODO: setupBrowser();

        // We do not need to set the user agent string since we set
        // the http.agent system property in
        // Applet2Environment.initialize().

        // We do not re-initialize the ServiceManager class here -- we
        // have already installed our Applet2BrowserService in
        // Applet2BrowserService.install().

        // The http.auth.serializeRequests property has already been
        // set in Applet2Environment.initialize()

        // Do not touch the protocol handlers, which have already been
        // set up in Applet2Environment.initialize()

        // Set Java Web Start version
        p.setProperty("javawebstart.version", Globals.getComponentName());

        // Do not touch the proxy or cookie handlers, which have
        // already been set up in Applet2Environment.initialize()

        // Do not touch the offline manager, which has already been
        // set up in Applet2Environment.initialize()

        // Initialize the JNLP API
        ServiceManager.setServiceManagerStub(new JnlpLookupStub());

        Config.setupPackageAccessRestriction();

        // setup JavawsDialogListener
        UIFactory.setDialogListener(new JavawsDialogListener());

        // Don't touch the https protocols, as they've already been
        // set up in Applet2Environment.initialize()

        // if system is in offline mode, stay offline
        if (DeployOfflineManager.isGlobalOffline()) {

            DeployOfflineManager.setForcedOffline(true);

        }

        if (Environment.isSystemCacheMode()) {
            CacheUpdateHelper.systemUpdateCheck();
        } else {
            if (Config.getBooleanProperty(Config.JAVAWS_UPDATE_KEY)) {
                if (CacheUpdateHelper.updateCache()) {
                    Config.setBooleanProperty(
                        Config.JAVAWS_UPDATE_KEY, false);
                    Config.storeIfDirty();
                }
            }
        }

        _environmentInitialized = true;

        DeployPerfUtil.put(t0, "JNLP2Manager - initializeExecutionEnvironment() - END");
    }

    protected boolean useGrayBoxProgressListener() {
        // We drive the GrayBoxPainter manually in this code
        return false;
    }

    private static final boolean _INJECT_EXCEPTION_CSTR;
    private static final boolean _INJECT_EXCEPTION_INIT;
    private static final boolean _INJECT_EXCEPTION_LOADJARFILES;
    private static final boolean _INJECT_FOREVER_LOADJARFILES;
    private static final boolean _INJECT_APPLET_RELAUNCH;

    static {
        boolean exceptionCstr=false;
        boolean exceptionInit=false;
        boolean exceptionLoadJarFiles=false;
        boolean foreverLoadJarFiles=false;
	boolean injectAppletRelaunch=false;

        String env = SystemUtil.getenv("JPI_PLUGIN2_INJECT_JNLP2MANAGER");
        if(null!=env) {
            System.out.println("JPI_PLUGIN2_INJECT_JNLP2MANAGER: "+env);
            StringTokenizer tok = new StringTokenizer(env);
            try {
                while (tok.hasMoreTokens()) {
                    String tmp = new String(tok.nextToken());
                    if(null!=tmp) {
                        if(!exceptionCstr)
                            exceptionCstr = "EXCEPTION_CSTR".equals(tmp);
                        if(!exceptionInit)
                            exceptionInit = "EXCEPTION_INIT".equals(tmp);
                        if(!exceptionLoadJarFiles)
                            exceptionLoadJarFiles = "EXCEPTION_LOADJARFILES".equals(tmp);
                        if(!foreverLoadJarFiles)
                            foreverLoadJarFiles = "FOREVER_LOADJARFILES".equals(tmp);
			if (!injectAppletRelaunch)
			    injectAppletRelaunch = "APPLET_RELAUNCH".equals(tmp);
                    }
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
            System.out.println("\tEXCEPTION_CSTR: "+exceptionCstr);
            System.out.println("\tEXCEPTION_INIT: "+exceptionInit);
            System.out.println("\tEXCEPTION_LOADJARFILES: "+exceptionLoadJarFiles);
            System.out.println("\tFOREVER_LOADJARFILES: "+foreverLoadJarFiles);
	    System.out.println("\tAPPLET_RELAUNCH: " + injectAppletRelaunch);
        }
        _INJECT_EXCEPTION_CSTR = exceptionCstr;
        _INJECT_EXCEPTION_INIT = exceptionInit;
        _INJECT_EXCEPTION_LOADJARFILES = exceptionLoadJarFiles;
        _INJECT_FOREVER_LOADJARFILES = foreverLoadJarFiles;
	_INJECT_APPLET_RELAUNCH = injectAppletRelaunch;
    }

    //----------------------------------------------------------------------
    // Disconnected applet support
    //

    public void installShortcuts() {
        // Must do this in a background thread in the applet's
        // AppContext because it may pop up a dialog box. Since this
        // method is called from PluginMain's main thread it may
        // thereby block that thread indefinitely.
        startWorkerThread("Shortcut Installer Thread", new Runnable() {
                public void run() {
                    URL jnlpUrl = _launchDesc.getLocation();
                    LocalApplicationProperties lap = Cache.getLocalApplicationProperties(jnlpUrl);
                    if (lap != null) {
                        lap.setLastAccessed(new Date());
                        lap.incrementLaunchCount();
			lap.setDraggedApplet();
			if (_initDocumentBaseURL != null) {
			    lap.setDocumentBase(_initDocumentBaseURL.toString());
			}
			if (_codebase != null) {
			    lap.setCodebase(_codebase.toString());
			}
                        LocalInstallHandler lih = LocalInstallHandler.getInstance();
                        if (lih != null) {
                            if (!lih.isShortcutExists(lap)) {
                                lap.setAskedForInstall(false);
                                // try to install on the local system
                                lih.install(_launchDesc, lap, true, false, null);
                            }
                        }
                        try {
                            lap.store();
                        } catch (IOException e) {
                            Trace.ignored(e);
                        }
                    }
                }
            });
    }


    private Container progressContainer = null;
    
    private Object getSurfaceObject() {
        if (progressContainer == null) {
            final AppContext c = appletAppContext;
            final Object[] results = new Object[1];
            if (c != null) try {
                DeployAWTUtil.invokeAndWait(c, new Runnable() {
                    public void run() {
                        results[0] = (Object) (new JPanel(new BorderLayout()));
                    }
                });
                progressContainer = (Container) results[0];
            } catch (Exception e) {
                Trace.ignored(e);
            }
        }
        return (Object) progressContainer;
    }

    private Object getStubObject() {
        return (Object) getAppletStub();
    }

    private void installProgressContainer() {
        final Container parent = getAppletParentContainer();
        if (parent == null) {
            return;
        }
        if (progressContainer != null) {
            final Container child = progressContainer;

            runOnEDT(parent, new Runnable() {
                public void run() {
                    Trace.println("Adding Custom Progress container to parent",
                        TraceLevel.NETWORK);
                    parent.add(child);
                    parent.invalidate();
                    parent.validate();
                }
            });
        }
    }

    protected void removeProgressContainer(final Container parent) {
        if (parent == null) {
            return;
        }
        if (progressContainer != null) {
            Container container = progressContainer;
            progressContainer = null;
            Trace.println("Removing Custom Progress container from parent",
                TraceLevel.NETWORK);
            // we should already be on the EDT here
            try {
                parent.remove(container);
            } catch (Throwable throwable) {
                // avoid race condition where it is not yet there
                // above code should insure it is not added later
            }
        }
        //NB: reference to the custom progress is still kep in the map
        // in the Progress class. This is to allow applet to reuse it
    }

    private void prepareCustomProgress(final LaunchDesc ld,
            final JARDesc progressJD, final String progressClassName) {

        DeployPerfUtil.put("begining of prepareCustomProgress()");
        // start with empty implementation - set listener later.
        final CustomProgress cp = new CustomProgress();
        Progress.setCustomProgress(cp);
        suspendGrayBoxPainting(); //TODO: do this only when progress is ready?
        final Object obj = (Object) getAppletParentContainer();

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
                     */
                    LaunchDownload.downloadProgressJars(ld);
                    Class progressClass =
                          getAppletClassLoader().loadClass(progressClassName);
                    Class [] noArgs = new Class[0];
                    Class [] objectArgs = { (new Object()).getClass() };
                    Class [] twoArgs = { (new Object()).getClass(),
                                         (new Object()).getClass() };
                    Object [] constructorArgs = null;
                    Constructor progressConstructor = null;
                    try {
                        /* first try for a construct with two Object arg */
                        progressConstructor = 
                            progressClass.getConstructor(twoArgs);
                        constructorArgs = new Object[2];
                        constructorArgs[0] = getSurfaceObject();
                        constructorArgs[1] = getStubObject();
                    } catch (Throwable e) {
                        try {
                            /* next try for a construct with one Object arg */
                            progressConstructor = 
                                progressClass.getConstructor(objectArgs);
                            constructorArgs = new Object[1];
                            constructorArgs[0] = getSurfaceObject();
                        } catch (Exception e2) {
                            try {
                                /* if not try for a no arg constructor */
                                progressConstructor = 
                                    progressClass.getConstructor(noArgs);
                                constructorArgs = noArgs;
                            } catch (Exception e3) {
                                Trace.ignored(e3);
                            }
                        }
                    }
                    final Constructor progressC = progressConstructor;
                    final Object [] constructorA = constructorArgs;

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
                                  progressC.newInstance(constructorA);
                            } catch (Exception e) {
                               // InstantiationException, IllegalAccessException,
                               // or InvocationTargetException can be thrown here
                            }
                        }
                    };
                    ThreadGroup threadGroup = getOrCreateAppletThreadGroup();
                    Thread constructorThread = (new Thread(threadGroup,
                               constructor, "ProgressMain"));
                    constructorThread.start();
                    DeployPerfUtil.put("CustomProgress constructor started");
                    try {
                        constructorThread.join(5000);
                        final DownloadServiceListener dsl =
                           (DownloadServiceListener)results[0];
                        cp.setAppThreadGroup(threadGroup);
                        cp.setListener(dsl);
                        shutdownGrayBoxPainter();
                        installProgressContainer();
                        DeployPerfUtil.put("Custom Progress class setup OK");
                    } catch (InterruptedException ie) {
                      Trace.println("Custom Progress class not constructed",
                                  TraceLevel.NETWORK);
                        Trace.ignored(ie);
                    } catch (ClassCastException cce) {
                        Trace.println("CustomProgress class is not in " +
                            "an  implementation of DownloadServiceListener",
                            TraceLevel.NETWORK);
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


}
