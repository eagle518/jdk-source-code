/*
 *  @(#)Main.java	1.267 09/02/03
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws;

import java.io.*;
import java.net.*;
import java.util.*;
import java.awt.*;
import java.text.SimpleDateFormat;
import java.text.ParseException;

import javax.jnlp.ServiceManager;

import com.sun.jnlp.JnlpLookupStub;
import com.sun.jnlp.JNLPClassLoader;

import com.sun.javaws.jnl.*;
import com.sun.javaws.exceptions.*;
import com.sun.javaws.ui.*;
import com.sun.javaws.security.AppContextUtil;
import com.sun.javaws.util.JavawsDialogListener;
import com.sun.javaws.util.JavawsConsoleController;

import com.sun.deploy.util.*;
import com.sun.deploy.ui.UIFactory;
import com.sun.deploy.ui.ComponentRef;
import com.sun.deploy.net.offline.DeployOfflineManager;
import com.sun.deploy.config.Config;
import com.sun.deploy.config.JREInfo;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.services.PlatformType;
import com.sun.deploy.net.DownloadEngine;
import com.sun.deploy.Environment;
import com.sun.deploy.cache.LocalApplicationProperties;
import com.sun.deploy.cache.Cache;
import com.sun.deploy.cache.CacheEntry;
import com.sun.deploy.si.SingleInstanceManager;
import com.sun.deploy.pings.Pings;

/*
 *   Main Entry point for Java Web Start
 *
 *   @version 1.135, 06/06/03
 */
public class Main {

    /** Main entry point for Java Web Start. It passes the arguments, and 
     *  initiates either launching an application, launching the JCP, 
     *  removeing an app or the whole cache, or importing an app into the cache.
     */
    private static boolean _isViewer = false;
    private static boolean _launchingAllowed = false;
    private static boolean _environmentInitialized = false;
    private static ThreadGroup _systemTG;
    private static ThreadGroup _securityTG;
    private static ClassLoader _secureContextClassLoader;
    private static ThreadGroup _launchTG;
    private static String _tempfile = null;
    private static DataInputStream _tckStream = null;
    private static boolean _verbose = false;

    private static boolean uninstall = false;

    public static void main(final String args[]) {

        //workaround for 6792372 to enable profiling of webstart applications
        URL.setURLStreamHandlerFactory(null);
     
        PerfLogger.setBaseTimeString(System.getProperty("jnlp.start.time"));
        PerfLogger.setStartTime("Java Web Start started");
        PerfLogger.setTime("Starting Main");
        Environment.setEnvironmentType(Environment.ENV_JAVAWS);

        // awt will staticly save the ContextClassLoader in EventQueue.java,
        // so install (even though not initialized yet) our JNLPClassLoader
        // as the ContextClassLoader before any reference to an awt object
        // that may cause that static initialization. (see bug #4665132)
        _secureContextClassLoader = 
                Thread.currentThread().getContextClassLoader();

        Thread.currentThread().setContextClassLoader(
                JNLPClassLoader.createClassLoader());

        // Install protocol handler to enable the above JNLPClassLoader
        String pkgs = System.getProperty("java.protocol.handler.pkgs");
        if (pkgs != null) {
            System.setProperty("java.protocol.handler.pkgs", pkgs +
                "|com.sun.javaws.net.protocol");
        } else {
            System.setProperty("java.protocol.handler.pkgs",
                "com.sun.javaws.net.protocol");
        }

        // force static initialization of awt Toolkit:
        PerfLogger.setTime("Start Toolkit init");
        Toolkit.getDefaultToolkit();
        PerfLogger.setTime("End Toolkit init");

        initializeThreadGroups();

        (new Thread(_securityTG, new Runnable() {
            public void run() {
                AppContextUtil.createSecurityAppContext();
                Thread.currentThread().setContextClassLoader(
                    getSecureContextClassLoader());
                try {
                    continueInSecureThread(args);
                } catch (Throwable t) {
                    t.printStackTrace();
                }
                Trace.flush();
            }
        }, "Java Web Start Main Thread")).start();
    }

    public static void continueInSecureThread(String argsin[]) {

        // force static initialization of deploy.Config, and
        // see if valid to run apps;
        _launchingAllowed = Config.isConfigValid();

        // Parse and remove debugging arguments
        // e.g arguments of the form -X<option>=<value>
        String [] args = Globals.parseOptions(argsin);

        // look for real args
        args = parseArgs(args);

        if (!Globals.isSilentMode() || Globals.isQuietMode()) {
            DeployUIManager.setLookAndFeel();
        }
        PerfLogger.setTime("End setLookAndFeel");

        if (args.length > 0) {
             _tempfile = args[args.length-1];
        } else if (!uninstall && !_isViewer &&
                   !Environment.isSystemCacheMode()) {
            LaunchErrorDialog.show(null, 
                new InvalidArgumentException(argsin), true);
        }
        
        // setup Tracing
        if (!_isViewer) {
            PerfLogger.setTime("Begin initTrace");
            initTrace();
            PerfLogger.setTime("End initTrace");
        }

        // If trying to run in system mode, system cache cannot be null, and
        // must be writable, else if silent import mode, use user cache .
        // If not silent import mode, this is an error.
        if (Environment.isSystemCacheMode()) {
            if (Config.getSystemCacheDirectory() == null ||
                !Cache.canWrite()) {
                if (Environment.isImportMode() && Globals.isSilentMode()) {
                    Environment.setSystemCacheMode(false);
                } else {
                    LaunchErrorDialog.show(null,
                        new CacheAccessException(true), true);
                }
            }
        }

        if (Environment.isImportMode() && !Cache.isCacheEnabled()) {
            if (Globals.isSilentMode()) {
                try {
                    systemExit(0);
                } catch (ExitException ee) {
                    Trace.println("systemExit: "+ee, TraceLevel.BASIC);
                    Trace.ignoredException(ee);
                }
            } else {
                LaunchErrorDialog.show(null, new CacheAccessException(
                    Environment.isSystemCacheMode(), true), true);
            }
        }
        
        // validate system cache directory
        Config.validateSystemCacheDirectory();

        // make sure the cache is writable
        if (Cache.canWrite() || _isViewer) {

            // Setup browser support if necessary
            setupBrowser();

            // Check that all properties are setup correctly
            PerfLogger.setTime("Begin JnlpxArgs.verify");
            JnlpxArgs.verify();
            PerfLogger.setTime("End JnlpxArgs.verify");
            // Initialize JRE with the right proxies, etc.
            initializeExecutionEnvironment();
            PerfLogger.setTime("End InitializeExecutionEnv");

            // uninstall after proxy is set - we need to build LaunchDesc
            if (uninstall) {
                uninstallCache(args.length > 0 ? args[0] : null);
            } else {
                // if system is in offline mode, stay offline
                if (DeployOfflineManager.isGlobalOffline()) {
                    JnlpxArgs.SetIsOffline();
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

                if (!_isViewer) {
                    // We need to let the TCK know we started up Java.
                    if (Globals.TCKHarnessRun) {
                        tckprintln(Globals.JAVA_STARTED);
                    }
                    if (args.length > 0) {
                        if (Environment.isImportMode()) {
                            // allow multiple URLS for importing.
                            for (int i=0; i<args.length; i++) {
                                // native code moves last arg to first,
                                // move back to last here
                                int argIndex;
                                if (i == 0) {
                                    argIndex = args.length - 1;
                                } else {
                                    argIndex = i - 1;
                                }
                                String newArgs[] = new String[1];
                                newArgs[0] = args[argIndex];
                                boolean exit = (i == args.length - 1);
                                PerfLogger.setTime("calling launchApp for import ...");
                                launchApp(newArgs, exit);
                                // -javafx applies only to first URL
                                Environment.setJavaFxInstallMode(
                                    Environment.JAVAFX_INSTALL_ONDEMAND);
                            }
                        } else {
                            PerfLogger.setTime("calling launchApp ...");
                            launchApp(args, true);
                        }
                    }
                } else {
                    // _isViewer - launch the Java Control Panel
                    if (args.length > 0) {
                        JnlpxArgs.removeArgumentFile(args[0]);
                    }
                    PerfLogger.setEndTime("Calling Application viewer");
                    PerfLogger.outputLog();
                    try {
                        String [] cpargs = {"-viewer"};
                        launchJavaControlPanel(cpargs);
                    } catch (Exception e) {
                        LaunchErrorDialog.show(null, e, true);
                    }
                }
            }
        } else {
            LaunchErrorDialog.show(null, new CacheAccessException(
                    Environment.isSystemCacheMode()), true);
        }
        Trace.flush();
        com.sun.javaws.ui.SplashScreen.hide();
    }

    public static void launchApp(String[] args, boolean exit) {

            // Expecting exactly one argument
            if (args.length > 1) {
                // last arg (not first) is the jnlp file:
                JnlpxArgs.removeArgumentFile(args[(args.length-1)]);
                LaunchErrorDialog.show(null,
                    new InvalidArgumentException(args), exit);
                return;
            }

            if (Environment.getJavaFxInstallMode() !=
                        Environment.JAVAFX_INSTALL_ONDEMAND) {
                if (!Environment.allowAltJavaFxRuntimeURL() &&
                        Pings.JAVAFX_CACHE_JNLP_URL.equals(args[0]) == false) {
                    LaunchErrorDialog.show(null,
                            new Exception(
                            "Incorrect URL for JavaFX preload or auto-update"),
                            exit);
                    return;
                }
            }

            // Load argument. First try to load it as a file, then as a URL
            LaunchDesc ld = null;
            try {
                JREInfo homeJREInfo = JREInfo.getHomeJRE(); 
                if (homeJREInfo == null) {
                    // No running JRE  ??
                    throw new ExitException(new Exception("Internal Error: no running JRE"), ExitException.LAUNCH_ERROR);
                }

                URL docbase = LaunchDescFactory.getDocBase();
                // use docbase to load JNLP file if available
                if (docbase != null) {
                    ld = LaunchDescFactory.buildDescriptor(args[0], (URL)null,
                            docbase, true);
                } else {
                    ld = LaunchDescFactory.buildDescriptor(args[0]);
                }
                if (ld.getLocation() == null) {
                    // if URL was passed in to a jnlp file w/o an href, 
                    // we may have an extra cached copy of the jnlp file.
                    // remove one keeping only the cannonical href.
                    try {
                        URL u = new URL(args[0]);
                        CacheEntry ce = Cache.getCacheEntry(u, null, null);
                        if (ce != null) {
                            Cache.removeCacheEntry(ce);
                        }
                    } catch (MalformedURLException e) {
                        // expected - not a URL
                    }
                }
            } catch(IOException ioe) {
                Exception e = null;
                JnlpxArgs.removeArgumentFile(args[0]);

                e = new CouldNotLoadArgumentException(args[0], ioe);

                if (Config.isJavaVersionAtLeast14()) {
                    // 1.4+ support HTTPS
                    if (ioe instanceof javax.net.ssl.SSLException || 
                        (ioe.getMessage() != null && ioe.getMessage().
                            toLowerCase().indexOf("https") != -1)) {
                        try {
                            e = new FailedDownloadingResourceException(
                                new URL(args[0]), null, ioe);
                        } catch (MalformedURLException mue) {
                            Trace.ignoredException(mue);
                        }
                    }
                }

                // for javafx preload or auto-update
                // specical case to handle initial jfx install ping
                // this send us a jfxic ping with error url to indicate
                // the javafx-cache.jnlp cannot be downloaded
                if (Environment.getJavaFxInstallMode() !=
                        Environment.JAVAFX_INSTALL_ONDEMAND) {
                    Pings.sendJFXPing(Pings.JAVAFX_INSTALL_COMPLETED_PING,
                            Launcher.getCurrentJavaFXVersion(),
                            Pings.JAVAFX_UNDEFINED_PING_FIELD,
                            Pings.JAVAFX_RETURNCODE_DOWNLOAD_FAILED_FAILURE,
                            args[0]);
                }

                // Failed to load argument. Show error message
                LaunchErrorDialog.show(null, e, exit);
                return;
	    } catch (JNLParseException jpe) {
                // The JNLP file cannot be parsed correctly
                JnlpxArgs.removeArgumentFile(args[0]);
                LaunchErrorDialog.show(null, jpe, exit);
		return;
            } catch (LaunchDescException lde) {
		Trace.println("Error parsing "+args[0]+". Try to parse again with codebase from LAP",
			      TraceLevel.BASIC);
		try {
		    // javaws is now (since 6u12) used to launch desktop
		    // shortcut of dragged applets as well. Plugin jnlp files
		    // may not have absolute url in the <jnlp codebase href>
		    // try with documentbase and codebase from LAP
		    ld = LaunchDescFactory.buildDescriptor(new File(args[0]));
		    if (ld == null) {
			// throw the original Exception
			throw lde;
		    }
		} catch (Exception e) {
		    JnlpxArgs.removeArgumentFile(args[0]);
		    LaunchErrorDialog.show(null, e, exit);
		    return;
		}
	    }catch(Exception e) {
                Trace.ignoredException(e);
                JnlpxArgs.removeArgumentFile(args[0]);
                // Missing field in launch file
                LaunchErrorDialog.show(null, e, exit);
                return;
            }

            Environment.setImportModeCodebase(ld.getCodebase());

            // Alright, we got a launch descriptor.Check for internal types,
            //  e.g., if we should launch the viewer
            if (ld.getLaunchType() == LaunchDesc.INTERNAL_TYPE) {
                JnlpxArgs.removeArgumentFile(args[0]);
                String tab = ld.getInternalCommand();
                String argv[];
                if (tab != null && !tab.equals("player") && 
                                   !tab.equals("viewer")) {
                    argv = new String[2];
                    argv[0] = "-tab";
                    argv[1] = ld.getInternalCommand();
                } else {
                    argv = new String[1];
                    argv[0] = "-viewer";
                }
                launchJavaControlPanel(argv);
            } else {
                // initialize a ThreadGroup and AppContext for security dialogs
                if (_launchingAllowed) {
                    new Launcher(ld).launch(args, exit);
                } else {
                    LaunchErrorDialog.show(null, new LaunchDescException(ld,
                        ResourceManager.getString("enterprize.cfg.mandatory",
                            Config.getEnterprizeString()), null), exit);
                }
            }
    }

    static void importApp(String app) {
        String args[] = new String[1];
        args[0] = app;
        boolean wasImport = Environment.isImportMode();
        Environment.setImportMode(true);
        boolean wasSilent = Globals.isSilentMode();
        Globals.setSilentMode(true);
        boolean wasShortcutMode = Globals.isShortcutMode();
        Globals.setCreateShortcut(true);

        launchApp(args, false);

        Environment.setImportMode(wasImport);
        Globals.setSilentMode(wasSilent);
        Globals.setCreateShortcut(wasShortcutMode);
    }        

    private static void launchJavaControlPanel(String args[]) {
        com.sun.javaws.ui.SplashScreen.hide();
        com.sun.deploy.panel.ControlPanel.main(args);
    }

    private static void uninstallCache(String path) {
        int uninstall_ret = -1;

        try {
            uninstall_ret = uninstall(path);
        } catch (Exception e) {
            LaunchErrorDialog.show(null, e, 
            (!Globals.isSilentMode() || Globals.isQuietMode()));
        }

        // don't delete <file> if called with "javaws -uninstall <file>"
        _tempfile = null;
        try {
            systemExit(uninstall_ret);
        } catch (ExitException ee) { 
            Trace.println("systemExit: "+ee, TraceLevel.BASIC);
            Trace.ignoredException(ee);
        }
    }
    
    private static Date parseDate(String dateString) {
        Date date = null;
        SimpleDateFormat formatter
                = new SimpleDateFormat("MM/dd/yy hh:mm a");
        try {
            date = formatter.parse(dateString);
        } catch (java.text.ParseException pe) {
            ParseException datePE = new ParseException(
                    pe.getMessage() + " " +
                    ResourceManager.getString("launch.error.dateformat"),
                    pe.getErrorOffset());
            LaunchErrorDialog.show(null, datePE, true);
        }
        return date;
    }

    private static String []  parseArgs(String [] args) {
        int argIndex;
        ArrayList remain = new ArrayList();

        for (argIndex = 0; argIndex < args.length; argIndex++) {
            if (!args[argIndex].startsWith("-")) {
                remain.add(args[argIndex]);
            }
            else if(args[argIndex].equals("-offline")) {
                JnlpxArgs.SetIsOffline();
                DeployOfflineManager.setForcedOffline(true);
            }
            else if(args[argIndex].equals("-online")) {
            }
            else if (args[argIndex].equals("-Xnosplash")) {
            }
            else if (args[argIndex].equals("-installer")) {
                Environment.setInstallMode(true);
            }
            else if (args[argIndex].equals("-uninstall") ||
                args[argIndex].equals("-Xclearcache")) {
                uninstall = true;
                Environment.setInstallMode(true);
                Environment.setImportMode(true);
            }
            else if (args[argIndex].equals("-import")) {
                Environment.setImportMode(true);
            }
            else if (args[argIndex].equals("-quiet")) {
                /*
                 * quiet mode is like silent mode, without running 
                 * headless (which allows necessary authentication dialogs).
                 * it also allows showing some fatal error messages if an 
                 * import or an uninstall fails.
                 */
                Globals.setQuietMode(true);
            }
            else if (args[argIndex].equals("-silent")) {
                Globals.setSilentMode(true);
            }
            else if (args[argIndex].equals("-reverse")) {
                Globals.setReverseMode(true);
            }
            else if (args[argIndex].equals("-javafx")) {
                Environment.setJavaFxInstallMode(
                        Environment.JAVAFX_INSTALL_PRELOAD_INSTALLER);
            }
            else if (args[argIndex].equals("-javafxau")) {
                Environment.setJavaFxInstallMode(
                        Environment.JAVAFX_INSTALL_AUTOUPDATE);
            }
            else if (args[argIndex].equals("-shortcut")) {
                Globals.setCreateShortcut(true);
            }
            else if (args[argIndex].equals("-association")) {
                Globals.setCreateAssoc(true);
            }
            else if (args[argIndex].equals("-prompt")) {
                Globals.setShowPrompts(true);
            }
            else if (args[argIndex].equals("-docbase")) {
                if (argIndex + 1 < args.length) {
                    String docbase = args[++argIndex];                  
                    try {
                        LaunchDescFactory.setDocBase(new URL(docbase));
                    } catch (MalformedURLException mue) {
                        // should not happen
                        Trace.ignoredException(mue);
                    }
                }
            }
            else if (args[argIndex].equals("-codebase")) {
                if (argIndex + 1 < args.length) {
                    String codebase = args[++argIndex];
                    try {
                        new URL(codebase);
                    } catch (MalformedURLException mue) {
                        LaunchErrorDialog.show(null, mue, true);
                    }
                    Environment.setImportModeCodebaseOverride(codebase);
                }
            }
            else if (args[argIndex].equals("-timestamp")) {
                if (argIndex + 1 < args.length) {
                    String dateString = args[++argIndex];
                    Date date = parseDate(dateString);
                    if (date != null) {
                        Environment.setImportModeTimestamp(date);
                    }
                }
            }
            else if (args[argIndex].equals("-expiration")) {
                if (argIndex + 1 < args.length) {
                    String dateString = args[++argIndex];
                    Date date = parseDate(dateString);
                    if (date != null) {
                        Environment.setImportModeExpiration(date);
                    }
                }
            }
            else if (args[argIndex].equals("-system")) {
                Environment.setSystemCacheMode(true);
            }
            else if (args[argIndex].equals("-secure")) {
                Globals.setSecureMode(true);
            }
            else if ((args[argIndex].equals("-open")) ||
                     (args[argIndex].equals("-print"))) {
                if (argIndex + 1 < args.length) {
                        String [] applicationArgs = new String[2];
                    applicationArgs[0] = args[argIndex++];
                    applicationArgs[1] = args[argIndex];
                    Globals.setApplicationArgs(applicationArgs);
                    // sets action and filename for single instance
                    // application
                    // this should only happens when javaws is invoked with
                    // a jnlp http url
                    // "javaws -open filename http://app.jnlp"
                    SingleInstanceManager.setActionName(applicationArgs[0]);
                    SingleInstanceManager.setOpenPrintFilePath(
                            applicationArgs[1]);
                }
            }
            else if (args[argIndex].equals("-viewer")) {
                _isViewer = true;
            } 
            else if (args[argIndex].equals("-verbose")) {
                _verbose = true;
            } 
            else {
                Trace.println("unsupported option: " + args[argIndex],
                                TraceLevel.BASIC);
            }
        }

        String[] remaining = new String[remain.size()];
        for (int i = 0; i < remaining.length; i++) {
            remaining[i] = (String)remain.get(i);
        }

        return remaining;
    }

    static private void initTrace() {

        // Redirect System.out/System.err
        com.sun.deploy.util.Trace.redirectStdioStderr();

        Trace.resetTraceLevel();

        Trace.setInitialTraceLevel();

        if (_verbose || Globals.TraceBasic) Trace.setBasicTrace(true);
        if (_verbose || Globals.TraceNetwork) Trace.setNetTrace(true);
        if (_verbose || Globals.TraceCache) Trace.setCacheTrace(true);
        if (_verbose || Globals.TraceSecurity) Trace.setSecurityTrace(true);
        if (_verbose || Globals.TraceExtensions) Trace.setExtTrace(true);
        if (_verbose || Globals.TraceTemp) Trace.setTempTrace(true);


        PerfLogger.setTime("Start setup Console");
        // setup Console tracing ...
        if (Config.getProperty(Config.CONSOLE_MODE_KEY).equals(
                        Config.CONSOLE_MODE_SHOW) && 
               !Globals.isHeadless() && !Globals.isQuietMode()) {
            JavawsConsoleController controller =
            JavawsConsoleController.getInstance();

            ConsoleTraceListener ctl = new ConsoleTraceListener(controller);

            // Create console window
            ConsoleWindow console = ConsoleWindow.create(controller);
            controller.setConsole(console);
            if (ctl != null) {
                ctl.setConsole(console);
                Trace.addTraceListener(ctl);
                ctl.print(ConsoleHelper.displayVersion() + "\n");
                ctl.print(ConsoleHelper.displayHelp());
            }
        }

        PerfLogger.setTime("End setup Console");

        // setup Socket Tracing if enabled
        SocketTraceListener stl = initSocketTrace();

        if (stl != null) {
            Trace.addTraceListener(stl);
        }

        // setup File Tracing if enabled
        FileTraceListener ftl = initFileTrace();

        if (ftl != null) {
            Trace.addTraceListener(ftl);
        }

        // Install Logger if we are running 1.4+
        if (Config.isJavaVersionAtLeast14() && Config.getBooleanProperty(
                                                Config.LOG_MODE_KEY)) {
            String logFilename = null;
            try {

                logFilename = Config.getProperty(Config.JAVAWS_LOGFILE_KEY);
                File logFileParentDir = new File(Config.getLogDirectory());
                if (logFilename != null && !"".equals(logFilename)) {
                        // valid filename
                        File logFile = new File(logFilename);
                        if (logFile.isDirectory()) {
                            logFilename = "";
                        } else {
                            logFileParentDir = logFile.getParentFile();
                            if (logFileParentDir != null) {
                                logFileParentDir.mkdirs();
                            }
                        }
                }

                if ("".equals(logFilename)) {
                    // use default log file name
                    logFileParentDir.mkdirs();
                   
                    File logFileFull = Trace.createTempFile(
                        Config.JAVAWS_OUTPUTFILE_PREFIX, 
                        Config.OUTPUTFILE_LOG_SUFFIX, logFileParentDir);
                    logFilename = logFileFull.getPath();
                }

                LoggerTraceListener ltl = null;
                
                File logFile = new File(logFilename);
                
                if ((logFile.exists() && logFile.canWrite()) || 
                    (!logFile.exists() && logFileParentDir.canWrite())) {
                    ltl = new LoggerTraceListener("com.sun.deploy", logFilename);
                }

                if (ltl != null) {
                    ltl.getLogger().setLevel(java.util.logging.Level.ALL);
                    JavawsConsoleController.getInstance().setLogger(ltl.getLogger());
                    Trace.addTraceListener(ltl);
                }
            } catch (Exception e) {
                Trace.println("can not create log file in directory: "+
                        Config.getLogDirectory(), TraceLevel.BASIC);
            }
        }


    }

    static private FileTraceListener initFileTrace() {

        if (Config.getBooleanProperty(Config.TRACE_MODE_KEY)) {
            File traceFile = null;
            String logDir = Config.getProperty(Config.LOGDIR_KEY);
            String traceFilename = Config.getProperty(Config.JAVAWS_TRACEFILE_KEY);
            try {

                if (traceFilename != null && !"".equals(traceFilename)
                        && traceFilename.compareToIgnoreCase("TEMP") != 0) {

                    // valid filename
                    traceFile = new File(traceFilename);
                    if (!traceFile.isDirectory()) {
                        int index = traceFilename.lastIndexOf(File.separator);
                        if (index != -1) {
                            logDir = traceFilename.substring(0, index);
                        }
                    } else {
                        traceFile = null;
                    }
                }

                // make sure logDir exist
                File dir = new File(logDir);
                dir.mkdirs();

                if (traceFile == null) {
                    traceFile=Trace.createTempFile(Config.JAVAWS_OUTPUTFILE_PREFIX, Config.OUTPUTFILE_TRACE_SUFFIX, dir);
                }
                if (traceFile != null) {
                    if ((traceFile.exists() && traceFile.canWrite()) || 
                        (!traceFile.exists() && dir.canWrite())) {
                        return new FileTraceListener(traceFile, true);
                    }
                }
            } catch (Exception ioe) {
                Trace.println("cannot create trace file in Directory: "+logDir,
                        TraceLevel.BASIC);
            }
        }
        return null;
    }

    static private SocketTraceListener initSocketTrace() {

         if (Globals.LogToHost != null) {

            /**
             * First we parse the options, 
             * note that IPv6 literal addresses must be
             * enclosed in [] ex: [ffff::abcd]:80
             */
            String hNamePort = Globals.LogToHost;
            String host=null;
            int port = -1;
            //Check to see if we have a literal IPv6 address
            int hostStartidx=0;
            int hostEndidx=0;
            if (hNamePort.charAt(0) == '[' && 
                        (hostEndidx = hNamePort.indexOf(1,']')) != -1) {
                hostStartidx = 1;
            } else {
                hostEndidx = hNamePort.indexOf(":");
            }
            host = hNamePort.substring(hostStartidx, hostEndidx);
            if (host == null) {

                return null;
            }
            try {
                String portS = 
                    (hNamePort.substring(hNamePort.lastIndexOf(':')+1));
                port = Integer.parseInt(portS);
            } catch (NumberFormatException nfe) {
                port = -1;
            }

            if (port < 0) {

                return null;
            }

            SocketTraceListener stl = new SocketTraceListener(host, port);

            //TCK
            if (stl != null) {
                Socket logsocket = stl.getSocket();

                if (Globals.TCKResponse && logsocket != null) {
                    try {
                        _tckStream = new DataInputStream(
                                logsocket.getInputStream());
                    } catch (IOException ioe) {
                        Trace.ignoredException(ioe);
                    }
                }
            }

            return stl;
         }

         return null;
    }
    /**
     * Uninstalls the first application named in <code>args</code>
     */
    static private int uninstall(String arg) {
        if (arg == null) {
            Trace.println("Uninstall all!", TraceLevel.BASIC);
            uninstallAll();
            if (Globals.TCKHarnessRun) {
                tckprintln(Globals.CACHE_CLEAR_OK);
            }
        } else {
            Trace.println("Uninstall: " + arg, TraceLevel.BASIC);

            LaunchDesc ld = null;
            try {
                ld = LaunchDescFactory.buildDescriptor(arg);
            } catch(IOException io) {
                Trace.ignoredException(io);
            } catch(JNLPException jnlpe) {
                Trace.ignoredException(jnlpe);
            }
            if (ld != null) {
                if (Globals.showPrompts()) {
                    String appTitle = ld.getInformation().getTitle();
                    String dlgTitle = ResourceManager
                        .getString("uninstall.app.prompt.title");
                    String message  = ResourceManager
                        .getString("uninstall.app.prompt.message", appTitle);

                    if (UIFactory.showConfirmDialog(null, ld.getAppInfo(), 
                            message, dlgTitle) != UIFactory.OK) {
                        // the user canceled the uninstall
                        Trace.println("Uninstall canceled by user.",
                                      TraceLevel.BASIC);
                        return 0;
                    }
                }

                   LocalApplicationProperties lap = null;
                if (ld.isInstaller() || ld.isLibrary()) {
                    // for extensions, arg must point to file in cache
                    // (since canonical home dosn't include version, and
                    //  may not be same place as in reffering jnlp file)
                    lap = Cache.getLocalApplicationProperties(arg);
                } else {
                    lap = Cache.getLocalApplicationProperties(
                                ld.getCanonicalHome());
                }

                if (lap != null) {
                    File jnlpFile = null;
                    try {
                        jnlpFile = new File(arg);
                        if (!jnlpFile.exists()) {
                            URL ref = new URL(arg);
                            jnlpFile = DownloadEngine.getCachedFile(ref);
                        }
                    } catch (Exception e) {
                            Trace.ignored(e);
                    }
                    if (jnlpFile != null && jnlpFile.exists()) {
                        CacheUtil.remove(jnlpFile, ld);
                    }
                    if (Globals.TCKHarnessRun) {
                        tckprintln(Globals.CACHE_CLEAR_OK);
                    }
                    return 0;
                }
            }

            // Error uninstalling

            Trace.println("Error uninstalling!", TraceLevel.BASIC);

            if (Globals.TCKHarnessRun) {
                tckprintln(Globals.CACHE_CLEAR_FAILED);
            }

            if (!Globals.isSilentMode() || Globals.isQuietMode()) {
                com.sun.javaws.ui.SplashScreen.hide();
                UIFactory.showErrorDialog(null,
                   ResourceManager.getMessage("uninstall.failedMessage"),
                   ResourceManager.getMessage("uninstall.failedMessageTitle"));
            }
            // uninstall failed
            // we should return 0 here too, since error dialog is already shown
        }
        // uninstall okay
        return 0;
    }

    /**
     * Uninstalls all the currently installed applications.
     */
    static private void uninstallAll() {
        CacheUtil.remove();
    }

    static private void setupBrowser()
    {
        if (Config.getBooleanProperty(Config.CAPTURE_MIME_KEY))
        {
            setupNS6();
            setupOpera();

            Config.setBooleanProperty(Config.CAPTURE_MIME_KEY, false);
        }
        // no else required; browsers already setup
    }

    // register mimeTypes for Opera
    static private void setupOpera()
    {
        OperaSupport opera = BrowserSupport.getInstance().getOperaSupport();

        if (opera != null) {
            if (opera.isInstalled()) {
                opera.enableJnlp(new File(
                    Config.getInstance().getSystemJavawsPath()),
                         Config.getBooleanProperty(Config.UPDATE_MIME_KEY));
            }
            // no else required; Opera isn't installed
        }
        // no else required; shouldn't need it, but maybe Opera isn't supported
        // by the current platform
    }

    // register mimeTypes for NS6
    static private void setupNS6()
    {
        String mailCapInfo = null;

        mailCapInfo = BrowserSupport.getInstance().getNS6MailCapInfo();

        String mimeInfo = "user_pref(\"browser.helperApps.neverAsk.openFile" + 
            "\", \"application%2Fx-java-jnlp-file\");\n";

        File prefs_file = null;

        // fix for 5078541: win32:fail to update mozilla prefs.js with
        // browser.helperApps.neverAsk.openFile
        String mozillaUserProfileDir = 
            Config.getInstance().getMozillaUserProfileDirectory();

        if (mozillaUserProfileDir != null) {
            prefs_file = new File(mozillaUserProfileDir + File.separator +
                                  "prefs.js");
        }

        if (prefs_file == null)
            return;

        InputStream is = null;
        // try to open the file for read
        try {
            String line = null;
            is = new FileInputStream(prefs_file);
            // read in the file
            BufferedReader in = new BufferedReader(new InputStreamReader(is));

            // check if mimeType setting already exist
            String contents = "";
            boolean addMimeInfo = true;
            boolean addMailCapInfo;
            if (mailCapInfo == null) {
                addMailCapInfo = false;
            } else {
                addMailCapInfo = true;
            }
            while (true) {
                // Get next line
                try {
                    line = in.readLine();

                    if (line == null) {
                        is.close();
                        break;
                    }
                    contents += line + "\n";
                    if (line.indexOf("x-java-jnlp-file") != -1) {
                        // already defined
                        addMimeInfo = false;
                    }
                    if (mailCapInfo != null && 
                        line.indexOf(".mime.types") != -1) {
                        addMailCapInfo = false;
                    }

                } catch (IOException ioe) {
                    Trace.ignoredException(ioe);
                }
            }

            if (!addMimeInfo && !addMailCapInfo) {
                return;
            }

            // add in new contens
            if (addMimeInfo) contents += mimeInfo;

            if (mailCapInfo != null && addMailCapInfo) {
                contents += mailCapInfo;
            }

            FileOutputStream fos = new FileOutputStream(prefs_file);
            try {
                fos.write(contents.getBytes());
                fos.close();
            } catch (IOException ioe) {
                Trace.ignoredException(ioe);
            }

        } catch (FileNotFoundException fnfe) {
            // else create a new pref.js
            Trace.ignoredException(fnfe);
            String content = "";

            // put in mailcap info (Unix only)
            if (mailCapInfo != null) content += mailCapInfo;

            // put in mimeTypes
            content += mimeInfo;

            try {
                FileOutputStream fos = new FileOutputStream(prefs_file);

                fos.write(content.getBytes());
                fos.close();
            } catch (IOException ioe) {
                Trace.ignoredException(ioe);
            }
        }
    }

    /**
     * Initializes the execution environment.
     */
    static public void initializeExecutionEnvironment()
    {
        if (_environmentInitialized) {
            return;
        }
        _environmentInitialized = true;

        boolean isWindows = (Config.getOSName().indexOf("Windows") != -1);
        boolean atLeast15 = Config.isJavaVersionAtLeast15();
        
        // setup user agent string
        Environment.setUserAgent(Globals.getUserAgent());

        // Setup services
        if (isWindows)
        {
            if (atLeast15) {
                com.sun.deploy.services.ServiceManager.setService(
                    PlatformType.STANDALONE_TIGER_WIN32);
            } else {
                com.sun.deploy.services.ServiceManager.setService(
                    PlatformType.STANDALONE_MANTIS_WIN32);
            }
        }
        else
        {
            if (atLeast15) {
                com.sun.deploy.services.ServiceManager.setService(
                    PlatformType.STANDALONE_TIGER_UNIX);
            } else {
                com.sun.deploy.services.ServiceManager.setService(
                    PlatformType.STANDALONE_MANTIS_UNIX);
            }
        }


        Properties p = System.getProperties();

        // fix for 4772298: proxy authentication dialog pop up twice 
        // even if username/password is correct
        p.put("http.auth.serializeRequests", "true");
        
     
        // Install https protocol handler if we are running 1.4+
        // Install JNLP RMI service provider if we are running 1.4+
        if (Config.isJavaVersionAtLeast14()) {
             
            if (Config.installDeployRMIClassLoaderSpi()) {
                p.put("java.rmi.server.RMIClassLoaderSpi",
                        "com.sun.jnlp.JNLPRMIClassLoaderSpi");
            }

            String pkgs = (String) p.get("java.protocol.handler.pkgs");
            if (pkgs != null) {
                p.put("java.protocol.handler.pkgs", 
                        pkgs + "|com.sun.deploy.net.protocol");
            } else {
                p.put("java.protocol.handler.pkgs", 
                        "com.sun.deploy.net.protocol");
            }
        }

        // Set Java Web Start version
        p.setProperty("javawebstart.version", Globals.getComponentName());

        // Reset proxy and cookie handlers
        try
        {
            // Reset proxy selector
            com.sun.deploy.net.proxy.DeployProxySelector.reset();

            // Reset cookie selector
            com.sun.deploy.net.cookie.DeployCookieSelector.reset();
        }
        catch(Throwable e)
        {
            // Fallback to static proxy handler
            com.sun.deploy.net.proxy.StaticProxyManager.reset();
        }

        // Reset offline manager
        com.sun.deploy.net.offline.DeployOfflineManager.reset();    

        /**
         * Because we access the net in various places this is a good starting
         * point to set the JAuthenticator callback method.
         * We initialize the dialog to pop up for user authentication
         * to password prompted URLs.
         */
        if (Config.getBooleanProperty(Config.SEC_AUTHENTICATOR_KEY))
        {
            JAuthenticator ja = JAuthenticator.getInstance((ComponentRef)null);
            /**
             * Note: Although we do this again in both Viewer and Launcher, with
             * different (non-null) parents, the ja returned is the same so the
             * extra calls to setDefault(ja) do nothing, even on merlin.
             */
            Authenticator.setDefault(ja);
        }

        // Initialize the JNLP API
        ServiceManager.setServiceManagerStub(new JnlpLookupStub());

	Config.setupPackageAccessRestriction();

        // setup JavawsDialogListener
        UIFactory.setDialogListener(new JavawsDialogListener());

        // Fix for Bug 5023701: Default handshaking protocols in HTTPS 
        // in webstart is problematic. Set default SSL handshaking protocols 
        // to SSLv3 and SSLv2Hello because some servers may not be able to 
        // handle TLS.
        //
        // Set only if https.protocols is not defined and Config.SEC_TLS_KEY
        // is false (default)
        //
        // Added options to enable/disable SSLv2/SSLv3/TLSv1 in Control Panel.
        //
        if (p.get("https.protocols") == null){
            StringBuffer protocolsStr = new StringBuffer();
            if (Config.getBooleanProperty(Config.SEC_TLS_KEY)){
                protocolsStr.append("TLSv1");
            }
            if (Config.getBooleanProperty(Config.SEC_SSLv3_KEY)){
                if (protocolsStr.length() != 0){
                    protocolsStr.append(",");
                }
                protocolsStr.append("SSLv3");
            }
            if (Config.getBooleanProperty(Config.SEC_SSLv2_KEY)){
                if (protocolsStr.length() != 0){
                    protocolsStr.append(",");
                }
                protocolsStr.append("SSLv2Hello");
            }

            p.put("https.protocols", protocolsStr.toString());
        }

        // initialize JVMParameters
        JVMParameters jvmParams = new JVMParameters();
        long heapSize = JnlpxArgs.getMaxHeapSize();
        if (heapSize<=0) heapSize=JVMParameters.getDefaultHeapSize();
        jvmParams.setMaxHeapSize(heapSize);
        heapSize = JnlpxArgs.getInitialHeapSize();
        if (heapSize>0&&heapSize!=JVMParameters.getDefaultHeapSize()) {
            jvmParams.parse("-Xms" + JVMParameters.unparseMemorySpec(heapSize));
        }
        // all command line options set with jnlpx.vmargs are trusted
        jvmParams.parseTrustedOptions(JnlpxArgs.getVMArgs());
        jvmParams.setDefault(true);
        JVMParameters.setRunningJVMParameters(jvmParams);
        if (Trace.isTraceLevelEnabled(TraceLevel.BASIC))
            Trace.println("Running JVMParams: "+jvmParams+"\n\t-> "+
                JVMParameters.getRunningJVMParameters(), TraceLevel.BASIC);
    }

    public static void systemExit(int status) throws ExitException {
        // fix for 4654173
        try {
            JnlpxArgs.removeArgumentFile(_tempfile);
            com.sun.javaws.ui.SplashScreen.hide();
        } catch (Exception e) {
            Trace.ignored(e);
        }
        Trace.flush();
        if ( Environment.isJavaPlugin() ) {
            ExitException ee = new ExitException(new RuntimeException("exit("+status+")"), ExitException.LAUNCH_ABORT_SILENT);
            throw ee;
        } else {
            System.exit(status);
        }
    }

    public static boolean isViewer() {
        return _isViewer;
    }

    public static final ThreadGroup getLaunchThreadGroup() {
        return _launchTG;
    }

    public static final ThreadGroup getSecurityThreadGroup() {
        return _securityTG;
    }

    public static final ClassLoader getSecureContextClassLoader() {
        return _secureContextClassLoader;
    }

    static private void initializeThreadGroups() {

        if (_securityTG == null) {
            for ( _systemTG = Thread.currentThread().getThreadGroup();
                  _systemTG.getParent() != null;
                  _systemTG = _systemTG.getParent()
                );

            _securityTG =
                new ThreadGroup(_systemTG, "javawsSecurityThreadGroup");

            DeploySysRun.setOverride(new JavawsSysRun());

            _launchTG =
                new ThreadGroup(_systemTG, "javawsApplicationThreadGroup");
        }
    }


    /**
     * println for tck harness only
     * "##TCKHarnessState##:%l:%d:%d:%s",datetime,pid,tid,state
     */
    public static synchronized void tckprintln(String msg) {
        final String TCK_PREAMBLE = "##TCKHarnesRun##";

        long datetime = System.currentTimeMillis();
        Trace.println(TCK_PREAMBLE+":"+datetime+":" +
                            (Runtime.getRuntime()).hashCode() + ":" +
                        Thread.currentThread() + ":" + msg );
        if (_tckStream != null) {
            try {
                   while(_tckStream.readLong()<datetime);
            } catch (java.io.IOException ioe) {
                System.err.println(
                    "Warning:Exceptions occurred, while logging to logSocket");
                ioe.printStackTrace(System.err);
            }
        }
        Trace.flush();
    }
}


