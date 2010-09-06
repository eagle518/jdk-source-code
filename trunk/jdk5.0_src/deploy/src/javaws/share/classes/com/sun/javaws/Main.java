/*
 *  @(#)Main.java	1.188 04/04/28
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws;

import java.io.*;
import java.net.*;
import java.util.*;
import java.awt.*;
import java.security.Provider;
import java.security.Security;

import javax.swing.*;
import javax.jnlp.ServiceManager;

import com.sun.jnlp.JnlpLookupStub;
import com.sun.jnlp.JNLPClassLoader;

import com.sun.javaws.jnl.*;
import com.sun.javaws.exceptions.*;
import com.sun.javaws.ui.*;
import com.sun.javaws.cache.Cache;
import com.sun.javaws.security.AppContextUtil;
import com.sun.javaws.util.VersionString;
import com.sun.javaws.util.JavawsDialogListener;
import com.sun.javaws.util.JavawsConsoleController;

import com.sun.deploy.util.*;
import com.sun.deploy.net.proxy.BrowserProxyInfo;
import com.sun.deploy.net.proxy.ProxyType;
import com.sun.deploy.net.proxy.NSPreferences;
import com.sun.deploy.config.Config;
import com.sun.deploy.config.JREInfo;
import com.sun.deploy.resources.ResourceManager;

/*
 *   Main Entry point for Java Web Start
 *
 *   @version 1.135, 06/06/03
 */
public class Main {

    /** Main entry point for Java Web Start. It passes
     *  the arguments, and initiates either the Viewer
     *  UI or launches an application, or shows a JPDA
     *  debugging mode notification window .
     */
    private static boolean _isViewer = false;
    private static boolean _launchingAllowed = false;
    private static ThreadGroup _systemTG;
    private static ThreadGroup _securityTG;
    private static ThreadGroup _launchTG;
    private static String[] _tempfile = new String[1];
    private static DataInputStream _tckStream = null;

    private static long t0, t1, t2, t3, t4, t5;
    private static boolean _timeing = true;
    private static boolean uninstall = false;

    public static void main(String args[]) {

	PerfLogger.setStartTime("JavaWebStart main started");

        // awt will staticly save the ContextClassLoader in EventQueue.java,
        // so install (even though not initialized yet) our JNLPClassLoader
        // as the ContextClassLoader before any reference to an awt object
        // that may cause that static initialization. (see bug #4665132)
        Thread.currentThread().setContextClassLoader(
		JNLPClassLoader.createClassLoader());
	if (_timeing) { t0 = (new Date()).getTime(); }

	// force static initialization of awt Toolkit:
	Toolkit.getDefaultToolkit();

	if (_timeing) { t1 = (new Date()).getTime(); }

	// force static initialization of deploy.Config, and
	// see if valid to run apps;
	_launchingAllowed = Config.isConfigValid();

	if (_timeing) { t2 = (new Date()).getTime(); }

	// May pop up a "JPDA Notification" window:
	JPDA.setup();

        // Parse and remove debugging arguments
        // e.g arguments of the form -X<option>=<value>
        args = Globals.parseOptions(args);

	if (_timeing) { t3 = (new Date()).getTime(); }

	// setup Tracing
	initTrace();

	if (_timeing) { t4 = (new Date()).getTime(); }

	updateCache();

	// look for real args
	args = parseArgs(args);

	if (args.length > 0) {
	     _tempfile[0] = args[0];
	}

        // make sure the cache is writable
        if (Cache.canWrite()) {
            // Setup browser support if necessary
            setupBrowser();


            // Check that all properties are setup correctly
            JnlpxArgs.verify();

            // Initialize JRE with the right proxies, etc.
            initializeExecutionEnvironment();

	    // uninstall after proxy is set - we might need to build LaunchDesc
	    if (uninstall) {
		uninstallCache(args.length > 0 ? args[0] : null);
	    }

            //We need to let the TCK know we started up Java.
            if (Globals.TCKHarnessRun) {
                tckprintln(Globals.JAVA_STARTED);
            }

            if (args.length == 0) {
                _isViewer = true;
            }

            if (!_isViewer) {
                launchApp(args, true);
            }

            if (_isViewer) {
                JnlpxArgs.removeArgumentFile(args);
                try {
                    if (_timeing) {
                        t5 = (new Date()).getTime();
                        Trace.println(
                            "startup times: \n" +
                            "      toolkit: "+(t1-t0) +"\n" +
                            "       config: "+(t2-t1) +"\n" +
                            "         args: "+(t3-t2) +"\n" +
                            "        trace: "+(t4-t3) +"\n" +
                            "     the rest: "+(t5-t4) +"\n" +
                            "", TraceLevel.BASIC);
                    }
                    Trace.println("Launching Cache Viewer", TraceLevel.BASIC);


                    Trace.flush();
                    com.sun.javaws.ui.CacheViewer.main(args);
                } catch (Exception e) {
		    LaunchErrorDialog.show(null, e, true);
                }
            }
        }
        else {
            LaunchErrorDialog.show(null, new CacheAccessException(Globals.isSystemCache()), true);
        }
	Trace.flush();
    }

    public static void launchApp(String[] args, boolean exit) {

            // Expecting exactly one argument
            if (args.length > 1) {
                JnlpxArgs.removeArgumentFile(args);
                LaunchErrorDialog.show(null,
                    new TooManyArgumentsException(args), exit);
		return;
            }

            // Load argument. First try to load it as a file, then as a URL
            LaunchDesc ld = null;
            try {
                ld = LaunchDescFactory.buildDescriptor(args[0]);
            } catch(IOException ioe) {
		Exception e = null;
                JnlpxArgs.removeArgumentFile(args);

		e = new CouldNotLoadArgumentException(args[0], ioe);

		if (Globals.isJavaVersionAtLeast14()) {
		    // 1.4+ support HTTPS
		    if (ioe instanceof javax.net.ssl.SSLException || (ioe.getMessage() != null && ioe.getMessage().toLowerCase().indexOf("https") != -1)) {
			try {
			    e = new FailedDownloadingResourceException(new URL(args[0]), null, ioe);
			} catch (MalformedURLException mue) {
			    Trace.ignoredException(mue);
			}
		    }
		}

                // Failed to load argument. Show error message
		LaunchErrorDialog.show(null, e, exit);
		return;
            } catch(JNLPException jnlpe) {
                JnlpxArgs.removeArgumentFile(args);
                // Missing field in launch file
                LaunchErrorDialog.show(null, jnlpe, exit);
		return;
            }

	    Globals.setCodebase(ld.getCodebase());

            // Alright, we got a launch descriptor.Check for internal types,
            //  e.g., if we should launch the viewer
            if (ld.getLaunchType() == LaunchDesc.INTERNAL_TYPE) {
                JnlpxArgs.removeArgumentFile(args);
                String command = ld.getInternalCommand();
                if (command.equals("viewer")) {
                    _isViewer = true;
                } else if (command.equals("player")) {
                    _isViewer = true;
                } else {
                    launchJavaControlPanel(command);
                    systemExit(0);
                }
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

    public static void importApp(String app) {
	String args[] = new String[1];
	args[0] = app;
	Globals.setImportMode(true);
	Globals.setSilentMode(true);
	launchApp(args, false);
	Launcher.checkCacheMax();
    }

    public static void launchJavaControlPanel(String initialTab) {
        String [] cmds = new String[7];
	String profileString=System.getProperty("javaplugin.user.profile");
	if (profileString == null) { profileString=""; }

        cmds[0] = Config.getInstance().toExecArg(JREInfo.getDefaultJavaPath());
        cmds[1] = "-cp";
        cmds[2] = Config.getInstance().toExecArg(Config.getJavaHome() +
                  File.separator + "lib" + File.separator + "deploy.jar");
	cmds[3] = Config.getInstance().toExecArg(
		  "-Djavaplugin.user.profile="+profileString);
        cmds[4] = "com.sun.deploy.panel.ControlPanel";
        cmds[5] = "-tab";
        cmds[6] = (initialTab == null) ? "general" : initialTab;
        Trace.println("Launching Control Panel: "+cmds[0]+" "+cmds[1]+" "+
          cmds[2]+" "+cmds[3]+" "+cmds[4]+" "+cmds[5]+" ", TraceLevel.BASIC);
        try {
            Runtime.getRuntime().exec(cmds);
        } catch (java.io.IOException ioe) {
            Trace.ignoredException(ioe);
        }
    }

    private static void uninstallCache(String path) {
	int uninstall_ret = -1;
	
	try {
	    uninstall_ret = uninstall(path);
	} catch (Exception e) {
	    LaunchErrorDialog.show(null, e, !Globals.isSilentMode());
	}

	systemExit(uninstall_ret);
    }

    private static String []  parseArgs(String [] args) {
	int argIndex;
	boolean mustHaveJnlpArg = false; // some args require jnlp file also
	ArrayList remain = new ArrayList();

	for (argIndex = 0; argIndex < args.length; argIndex++) {
	    if (!args[argIndex].startsWith("-")) {
		remain.add(args[argIndex]);
	    }

            else if (args[argIndex].equals("-Xclearcache")) {
	        try {
		    Cache.remove();
		    long cacheSize = Cache.getCacheSize();
		    if (cacheSize > 0) {
		        System.err.println("Could not clean all entries " +
					   " in cache since they are in use");
		        if (Globals.TCKHarnessRun) {
			    tckprintln(Globals.CACHE_CLEAR_FAILED);
		        }
		        systemExit(-1);
		    }
	        } catch(IOException ioe) {
		    Trace.println("Clear cached failed: " + ioe.getMessage());
		    if (Globals.TCKHarnessRun) {
		        tckprintln(Globals.CACHE_CLEAR_FAILED);
		    }
		    systemExit(-1);
	        }
	        if (Globals.TCKHarnessRun) {
		    tckprintln(Globals.CACHE_CLEAR_OK);
	        }
            }

            else if(args[argIndex].equals("-offline")) {
	        JnlpxArgs.SetIsOffline();
	        Globals.setOffline(true);
            }

	    else if(args[argIndex].equals("-online")) {
	    }

            else if (args[argIndex].equals("-Xnosplash")) {
            }

            else if (args[argIndex].equals("-installer")) {
		Globals.setInstallMode(true);
            }

            else if (args[argIndex].equals("-uninstall")) {	
		uninstall = true;
		Globals.setInstallMode(true);
	    }

	    // no longer used
	    else if (args[argIndex].equals("-updateVersions")) {
		systemExit(0);
	    }

	    else if (args[argIndex].equals("-import")) {
		Globals.setImportMode(true);
		mustHaveJnlpArg = true;
	    }

	    else if (args[argIndex].equals("-silent")) {
		Globals.setSilentMode(true);
	    }

	    else if (args[argIndex].equals("-shortcut")) {
		Globals.setCreateShortcut(true);
	    }
	    else if (args[argIndex].equals("-association")) {
		Globals.setCreateAssoc(true);
	    }

	    else if (args[argIndex].equals("-codebase")) {
		if (argIndex + 1 < args.length) {
		    String codebase = args[++argIndex];
		    try {
			new URL(codebase);
		    } catch (MalformedURLException mue) {
			LaunchErrorDialog.show(null, mue, true);
		    }
                    Globals.setCodebaseOverride(codebase);
                }
		mustHaveJnlpArg = true;
	    }

	    else if (args[argIndex].equals("-system")) {
		Globals.setSystemCache(true);
		mustHaveJnlpArg = true;
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
                }
		mustHaveJnlpArg = true;
	    }

	    else if (args[argIndex].equals("-viewer")) {
		_isViewer = true;
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

	if (Globals.TraceBasic) Trace.setBasicTrace(true);
	if (Globals.TraceNetwork) Trace.setNetTrace(true);
	if (Globals.TraceCache) Trace.setCacheTrace(true);
	if (Globals.TraceSecurity) Trace.setSecurityTrace(true);
	if (Globals.TraceExtensions) Trace.setExtTrace(true);
	if (Globals.TraceTemp) Trace.setTempTrace(true);


	// setup Console tracing ...
	if (Config.getProperty(Config.CONSOLE_MODE_KEY).equals(
				Config.CONSOLE_MODE_SHOW) && !Globals.isHeadless()) {
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
	if (Globals.isJavaVersionAtLeast14() && Config.getBooleanProperty(
						Config.LOG_MODE_KEY)) {
	    String logFilename = null;
	    try {
	
		logFilename = Config.getProperty(Config.JAVAWS_LOGFILE_KEY);
		File logFileParentDir = new File(Config.getLogDirectory());
		if (logFilename != null && logFilename != "") {

		 
			
		    if (logFilename.compareToIgnoreCase("TEMP") != 0) {
			
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
		    } else {
			logFilename = "";
		    }
		 
		}

		if (logFilename == "") {
		    // use default log file name
		    logFileParentDir.mkdirs();
		    
		    logFilename = Config.getLogDirectory() + File.separator + "javaws.log";
		}
		
		LoggerTraceListener ltl = new LoggerTraceListener(
								  "com.sun.deploy", logFilename);

	
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

		if (traceFilename != null && traceFilename != "" && traceFilename.compareToIgnoreCase("TEMP") != 0) {

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
		    traceFile=File.createTempFile("javaws",".trace", dir);
		}

		return new FileTraceListener(traceFile, true);

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
             * First we parse the options, note that IPv6 literal addresses must be
             * enclosed in [] ex: [ffff::abcd]:80
             */
            String hNamePort = Globals.LogToHost;
            String host=null;
            int port = -1;
            //Check to see if we have a literal IPv6 address
            int hostStartidx=0;
            int hostEndidx=0;
            if (hNamePort.charAt(0) == '[' && (hostEndidx = hNamePort.indexOf(1,']')) != -1) {
                hostStartidx = 1;
            } else {
                hostEndidx = hNamePort.indexOf(":");
            }
            host = hNamePort.substring(hostStartidx, hostEndidx);
            if (host == null) {

                return null;
            }
            try {
                String portS = (hNamePort.substring(hNamePort.lastIndexOf(':')+1));
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
   		LocalApplicationProperties lap = null;
		if (ld.isInstaller() || ld.isLibrary()) {
		    // for extensions, arg must point to file in cache
		    // (since canonical home dosn't include version, and
		    //  may not be same place as in reffering jnlp file)
		    lap = Cache.getLocalApplicationProperties(arg, ld);
		} else {
		    lap = Cache.getLocalApplicationProperties(
			ld.getCanonicalHome(), ld);
		}

		if (lap != null) {
		    Cache.remove(arg, lap, ld);
		    Cache.clean();
		    if (Globals.TCKHarnessRun) {
		        tckprintln(Globals.CACHE_CLEAR_OK);
		    }
		    // We are done!
		    return 0;
		}
            }

            // Error uninstalling

	    Trace.println("Error uninstalling!", TraceLevel.BASIC);

	    if (Globals.TCKHarnessRun) {
	        tckprintln(Globals.CACHE_CLEAR_FAILED);
	    }

	    if (!Globals.isSilentMode()) {
		SplashScreen.hide();
		DialogFactory.showErrorDialog(null,
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
	Cache.remove();
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
		opera.enableJnlp(new File(JREInfo.getDefaultJavaPath()),
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

	String mimeInfo = "user_pref(\"browser.helperApps.neverAsk.openFile\", \"application%2Fx-java-jnlp-file\");\n";


	// check if netscape 6 is installed
	String homeDir = System.getProperty("user.home");

	File regFile = new File(homeDir + "/.mozilla/appreg");

	File prefs_file = null;
	try {
	    prefs_file = NSPreferences.getNS6PrefsFile(regFile);
	}
	catch(IOException e)
	{
	    Trace.println("cannot determine NS6 prefs.js location", TraceLevel.BASIC);
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
		    if (mailCapInfo != null && line.indexOf(".mime.types") != -1) {
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
     * Updates the cache for any version specific changes
     */
    static private void updateCache() {
	if (Config.getProperty(Config.JAVAWS_CACHE_KEY) != null) {
	    Cache.updateCache();
	    Config.setProperty(Config.JAVAWS_CACHE_KEY, null);
	    Config.storeIfDirty();
	}
    }


    /**
     * Initializes the execution environment.
     */
    static private void initializeExecutionEnvironment()
    {
	boolean isWindows = (Config.getOSName().indexOf("Windows") != -1);
	boolean atLeast15 = Globals.isJavaVersionAtLeast15();

	// Setup services
	if (isWindows)
	{
	    if (atLeast15) {
		com.sun.deploy.services.ServiceManager.setService(com.sun.deploy.services.PlatformType.STANDALONE_TIGER_WIN32);
	    } else {
		com.sun.deploy.services.ServiceManager.setService(com.sun.deploy.services.PlatformType.STANDALONE_MANTIS_WIN32);
	    }
	}
	else
	{
	    if (atLeast15) {
		com.sun.deploy.services.ServiceManager.setService(com.sun.deploy.services.PlatformType.STANDALONE_TIGER_UNIX);
	    } else {
		com.sun.deploy.services.ServiceManager.setService(com.sun.deploy.services.PlatformType.STANDALONE_MANTIS_UNIX);
	    }
	}


	Properties p = System.getProperties();

	// fix for 4772298: proxy authentication dialog pop up twice even if username/password is correct
	p.put("http.auth.serializeRequests", "true");

	// Install https protocol handler if we are running 1.4+
	if (Globals.isJavaVersionAtLeast14()) {
	    String pkgs = (String) p.get("java.protocol.handler.pkgs");
	    if (pkgs != null) {
		p.put("java.protocol.handler.pkgs", pkgs + "|com.sun.deploy.net.protocol");
	    } else {
		p.put("java.protocol.handler.pkgs", "com.sun.deploy.net.protocol");
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

        /**
         * Because we access the net in various places this is a good starting
         * point to set the JAuthenticator callback method.
         * We initialize the dialog to pop up for user authentication
         * to password prompted URLs.
         */
        if (Config.getBooleanProperty(Config.SEC_AUTHENTICATOR_KEY)) 
	{
            JAuthenticator ja = JAuthenticator.getInstance((Frame)null);
            /**
             * Note: Although we do this again in both Viewer and Launcher, with
             * different (non-null) parents, the ja returned is the same so the
             * extra calls to setDefault(ja) do nothing, even on merlin.
             */
            Authenticator.setDefault(ja);
        }

	// Initialize the JNLP API
	ServiceManager.setServiceManagerStub(new JnlpLookupStub());

	// Don't allow launched apps to load or define classes from com.sun.javaws if
	// a security manager gets installed
	addToSecurityProperty("package.access",     "com.sun.javaws");
	addToSecurityProperty("package.access",     "com.sun.deploy");
	addToSecurityProperty("package.definition", "com.sun.javaws");
	addToSecurityProperty("package.definition", "com.sun.deploy");
	addToSecurityProperty("package.definition", "com.sun.jnlp");

	// Don't allow access or define JSS classes from org.mozilla.jss
        addToSecurityProperty("package.access",     "org.mozilla.jss");
        addToSecurityProperty("package.definition", "org.mozilla.jss");

	// setup JavawsDialogListener
	DialogFactory.addDialogListener(new JavawsDialogListener());

	// Fix for Bug 5023701: Default handshaking protocols in HTTPS in webstart is problematic
	// Set default SSL handshaking protocols to SSLv3 and SSLv2Hello
        // because some servers may not be able to handle TLS.
        //
        // Set only if https.protocols is not defined and Config.SEC_TLS_KEY 
        // is false (default)
        //
	if (p.get("https.protocols") == null &&
	    Config.getBooleanProperty(Config.SEC_TLS_KEY) == false) {
	    p.put("https.protocols", "SSLv3,SSLv2Hello");
	}
    }

    static private void addToSecurityProperty(String propertyName, String newValue) {
	String value = java.security.Security.getProperty(propertyName);

	Trace.println("property " + propertyName + " value " + value, TraceLevel.SECURITY);

	if (value != null) {
	    value = value + "," + newValue;
	}
	else {
	    value = newValue;
	}
	java.security.Security.setProperty(propertyName, value);

	Trace.println("property " + propertyName + " new value " + value, TraceLevel.SECURITY);

    }

    public static void systemExit(int status) {
	// fix for 4654173
	JnlpxArgs.removeArgumentFile(_tempfile);
	SplashScreen.hide();
	Trace.flush();
	System.exit(status);
    }

    static boolean isViewer() {
	return _isViewer;
    }

    public static final ThreadGroup getLaunchThreadGroup() {
	initializeThreadGroups();
	return _launchTG;
    }

    public static final ThreadGroup getSecurityThreadGroup() {
	initializeThreadGroups();
	return _securityTG;
    }

    static private void initializeThreadGroups() {

	if (_securityTG == null) {
            for ( _systemTG = Thread.currentThread().getThreadGroup();
                  _systemTG.getParent() != null;
                  _systemTG = _systemTG.getParent()
                );

            _securityTG =
		new ThreadGroup(_systemTG, "javawsSecurityThreadGroup");

            new Thread(_securityTG,
                   new Runnable() {
                        public void run() {
                            AppContextUtil.createSecurityAppContext();
                        }
                   }).start();

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

    }
}


