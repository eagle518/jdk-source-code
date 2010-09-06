/*
 * @(#)Launcher.java	1.183 04/06/11
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws;
import java.util.Properties;
import java.io.*;
import java.net.*;
import java.security.*;
import java.util.*;
import java.lang.reflect.*;
import java.awt.*;
import java.awt.event.*;
import java.applet.*;
import java.lang.reflect.InvocationTargetException;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.SwingUtilities;
import javax.swing.UIManager;
import javax.swing.UnsupportedLookAndFeelException;
import java.util.jar.JarFile;
import java.util.jar.Manifest;
import com.sun.javaws.jnl.*;
import com.sun.javaws.ui.AutoDownloadPrompt;
import com.sun.javaws.ui.DownloadWindow;
import com.sun.javaws.util.VersionString;
import com.sun.javaws.util.VersionID;
import com.sun.javaws.util.URLUtil;
import com.sun.javaws.cache.Cache;
import com.sun.javaws.cache.DownloadProtocol;
import com.sun.javaws.cache.DiskCacheEntry;
import com.sun.javaws.security.AppPolicy;
import com.sun.javaws.security.JavaWebStartSecurity;
import com.sun.javaws.exceptions.*;
import com.sun.javaws.util.JavawsConsoleController;
import javax.jnlp.ServiceManager;
import javax.jnlp.UnavailableServiceException;
import com.sun.jnlp.BasicServiceImpl;
import com.sun.jnlp.ExtensionInstallerServiceImpl;
import com.sun.jnlp.JNLPClassLoader;
import com.sun.jnlp.AppletContainer;
import com.sun.jnlp.AppletContainerCallback;
import com.sun.deploy.config.Config;
import com.sun.deploy.config.JREInfo;
import com.sun.deploy.si.SingleInstanceManager;
import com.sun.deploy.util.*;
import com.sun.deploy.resources.ResourceManager;

/*
 * Given a LaunchDescriptor, the class takes care of launching
 * the given application. This might involve downloading JREs, and
 * optional packages
 */
public class Launcher implements Runnable {

    /**
     *  Reference to download progress window
     */
    private DownloadWindow _downloadWindow = null;

    /**
     * The console. This will be null if the user doesn't want to see the
     * console.
     */

    private LaunchDesc _launchDesc;
    private String[] _args;
    private boolean _exit = true;
    private JAuthenticator _ja;

    public Launcher(LaunchDesc ld) {
	_launchDesc = ld;

        // Create download window. This might not be shown. But it is created
        // here, so it can be used as a owner for dialogs.
        _downloadWindow = new DownloadWindow();

        Trace.println("new Launcher: " + ld.toString(), TraceLevel.BASIC);
    }

    /**
     *  Main entry point for launching the app.
     */
    public void launch(String[] args, boolean exit) {
        _args = args;
        _exit = exit;
        (new Thread(Main.getLaunchThreadGroup(),
		    this,
		    "javawsApplicationMain")
	).start();
    }

    private void removeTempJnlpFile(LaunchDesc ld) {
	DiskCacheEntry dce = null;

	try {
	    if (ld.isApplicationDescriptor()) {
	        dce = DownloadProtocol.getCachedLaunchedFile(
			ld.getCanonicalHome());
	    }
	} catch (JNLPException jnlpe) {
	    Trace.ignoredException(jnlpe);
	}

	if (dce == null) return;

	File cachedJnlp = dce.getFile();

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

    public void run() {

        // We never trust LaunchDesc's with <jnlp href=".."
	// unless we have downloaded them ourselves.
	// Make sure to get it from the cache, if we got one.
	LaunchDesc ld = LaunchDownload.getLaunchDescFromCache(_launchDesc);

	// remove temp jnlp file if it is in the cache already
	removeTempJnlpFile(ld);

        // See if the JNLP file properties contains any debugging properties
        if (ld.getResources() != null) {
	    Globals.getDebugOptionsFromProperties(
			ld.getResources().getResourceProperties());
        }


        /**
	 * We initialize the dialog to pop up for user authentication
	 * to password prompted URLs.
	 */
        if (Config.getBooleanProperty(Config.SEC_AUTHENTICATOR_KEY)) {
            _ja = JAuthenticator.getInstance(_downloadWindow.getFrame());
            Authenticator.setDefault(_ja);
        }

        // The handling of this LaunchDesc might result in downloading of
        // a new one, if there is an updated LaunchDesc. We don't want
        // to get into an infinite loop here - so if the Web server keeps
        // returning a newer one - we just disable caching of the JNLP
        // file and launches
        int tries = 0;
        boolean isFinalTry = false;
	boolean isSilent = Globals.isSilentMode();
	boolean isImport = Globals.isImportMode() || (ld.getLaunchType() ==
						LaunchDesc.LIBRARY_DESC_TYPE);
	try {
            do {
	        isFinalTry = (tries == 3);
	        _downloadWindow.setLaunchDesc(ld, true);
	        ld = handleLaunchFile(ld, _args, !isFinalTry,
				      isImport, isSilent);
	        tries++;
            } while(ld != null && !isFinalTry);
	} catch (ExitException ee) {
	    int exitValue =  (ee.getReason() == ExitException.OK) ? 0 : -1;
	    if (ee.getReason() == ExitException.LAUNCH_ERROR) {
	        LaunchErrorDialog.show(((_downloadWindow == null) ?
			    		null : _downloadWindow.getFrame()),
					ee.getException(), _exit);
	    }
	    if (_exit) {
	        Main.systemExit(exitValue);
	    }
	}
    }

    private LaunchDesc handleLaunchFile(LaunchDesc ld, String[] args,
	    boolean checkForJNLPUpdate, boolean isImport, boolean isSilent)
				throws ExitException {
        // Up front check on JNLP Spec version
        VersionString version = new VersionString(ld.getSpecVersion());
        VersionID supportedVersion = new VersionID("1.5");

        if (!version.contains(new VersionID("1.5"))) {
            /* now supports both 1.0 1nd 1.5 jnlp formats */
            if (!version.contains(new VersionID("1.0"))) {
                JNLPException.setDefaultLaunchDesc(ld);
		handleJnlpFileException(ld, new LaunchDescException(ld,
	            ResourceManager.getString("launch.error.badjnlversion",
	            ld.getSpecVersion()), null));
            }
        }

        // Check that at least some resources was specified
        if (ld.getResources() == null) {
	    handleJnlpFileException(ld, new LaunchDescException(ld,
	        ResourceManager.getString("launch.error.noappresources",
	        ld.getSpecVersion()), null));
	}

	// Check that a JRE is specified
	// (no need to check if it is import mode and it is a component ext)
	if (!isImport && !ld.isLibrary()) {
	    if (!ld.isJRESpecified()) {
	        LaunchDescException lde = new LaunchDescException(ld,
	            ResourceManager.getString(
			"launch.error.missingjreversion"), null);
	        handleJnlpFileException(ld, lde);
	    }
	}
	boolean isInstaller =
	    (ld.getLaunchType() == LaunchDesc.INSTALLER_DESC_TYPE);
	return handleApplicationDesc(ld, args, isInstaller,
				     checkForJNLPUpdate, isImport, isSilent);
    }

    private LaunchDesc handleApplicationDesc(LaunchDesc ld, String[] args,
	    boolean isInstaller, boolean checkForJNLPUpdate,
	    boolean isImport, boolean isSilent) throws ExitException {
        // Initialize exception with the launch descriptor
        JNLPException.setDefaultLaunchDesc(ld);

	int timeout;
        Frame owner = _downloadWindow.getFrame();

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
        LocalApplicationProperties lap = null;
        if (isInstaller) {
            // An extension should always be invoked by a a file entry that
	    // points to the cache. We can get the URL and version id from it.
	    lap = Cache.getLocalApplicationProperties(args[0], ld);

	    // An extension should also always be invoked with -installer
	    if (lap == null || !Globals.isInstallMode()) {
 		handleJnlpFileException(ld, new MissingFieldException(
 					ld.getSource(),
 					"<application-desc>|<applet-desc>"));
 	    }
	    jnlpUrl = lap.getLocation();
	} else {
            lap = Cache.getLocalApplicationProperties(jnlpUrl, ld);
	}

	Trace.println("LaunchDesc location: " + jnlpUrl + ", version: " +
					lap.getVersionId(), TraceLevel.BASIC);

        // Step One: Check if we need to download resources.

        boolean isCached = LaunchDownload.isInCache(ld);
        boolean offlineMode = isCached && Globals.isOffline();

        // Determine if a JRE is already available
	// (no need to determine JRE info for import mode
        JREInfo jreInfo = null;
	if (!isImport) {
	    jreInfo = LaunchSelection.selectJRE(ld);

            if (jreInfo == null) {
                String pref = Config.getProperty(Config.JAVAWS_JRE_AUTODOWNLOAD_KEY);

                if ((pref != null) &&
                    (pref.equalsIgnoreCase(Config.AUTODOWNLOAD_MODE_NEVER))) {
                    // configuration prevents downloading required JRE
                    String version = ld.getResources().getSelectedJRE().getVersion();

                    throw (new ExitException(new NoLocalJREException(ld, version, false),
                                             ExitException.LAUNCH_ERROR));
                }
                else if ((pref != null) &&
                         (pref.equalsIgnoreCase(Config.AUTODOWNLOAD_MODE_PROMPT))) {
                    if (AutoDownloadPrompt.prompt(owner, ld) == false)
                    {
                        // the user prevented downloading required JRE
                        String version = ld.getResources().getSelectedJRE().getVersion();

                        throw (new ExitException(new NoLocalJREException(ld, version, true),
                                                 ExitException.LAUNCH_ERROR));
                    }
                    // no else required; download accepted
                }
                // no else required; any other value is interpreted as ALWAYS
            }
            // no else required; got a local JRE
        }

	timeout = Config.getIntProperty(Config.JAVAWS_UPDATE_TIMEOUT_KEY);

        // Need to update if not offline and either:
        //  - the application is not cached
        //  - no local JRE is found
        //  - the preferences is to always do the check
        //  - the application properties says that we should do an update
        //  - Installer needs progress window
        boolean forceUpdate =
	    (!isCached)  ||
	    (!isImport && (jreInfo == null)) ||
	    (!offlineMode && (lap.forceUpdateCheck() ||
            (isInstaller) ||
            ((new RapidUpdateCheck()).doUpdateCheck(ld, lap, timeout))));

	Trace.println(
		"Offline mode: " + offlineMode +
		"\nIsInCache: " + isCached +
		"\nforceUpdate: " + forceUpdate +
		"\nInstalled JRE: " + jreInfo +
		"\nIsInstaller: " + isInstaller, TraceLevel.BASIC);

        if (forceUpdate && offlineMode) {
	    throw (new ExitException(new OfflineLaunchException(),
					ExitException.LAUNCH_ERROR));
        }

        //
        // Download all resources
        //

	// Keep track of all JNLP files for installers
        ArrayList installFiles = new ArrayList();

        if (forceUpdate) {
	    LaunchDesc newLd = downloadResources(ld,
		!isImport && jreInfo == null,
		!isInstaller && checkForJNLPUpdate,
		installFiles, isSilent);
	    // Check if we got a new LaunchDesc
	    if (newLd != null) {
		// downloaded JNLP file into cache, remove temp file
		removeTempJnlpFile(ld);
		return newLd;
	    }

	    // Reset force update
	    if (lap.forceUpdateCheck()) {
		lap.setForceUpdateCheck(false);
		try { lap.store(); }
		catch(IOException ioe) { Trace.ignoredException(ioe); }
	    }

	    if (!isSilent) {
		checkCacheMax();
	    }
	}

	// check if there is another instance running
	if (SingleInstanceManager.isServerRunning(
		ld.getCanonicalHome().toString())) {
	    String[] appArgs = Globals.getApplicationArgs();

	    if (appArgs != null) {
		ld.getApplicationDescriptor().setArguments(appArgs);
	    }

	    // send the JNLP file to the server port
	    if (SingleInstanceManager.connectToServer(
		ld.toString())) {
		// if we get OK from server, we are done
		throw new ExitException(null, ExitException.OK);
            }
	    // else continue normal launch
	}

	if (!isSilent) {
	    SplashScreen.generateCustomSplash(owner, ld, forceUpdate);
	}
        //
        // Run all installers if any
        //
        if (!isImport && !installFiles.isEmpty()) {
	    if (isInstaller) {
		// FIXIT: Installers should not have installers
	    }
	    executeInstallers(installFiles);
        }

        // Let progress window show launching behavior
        if (!isSilent && (_downloadWindow.getFrame() != null)) {
	    String title = ResourceManager.getString("launch.launchApplication");
	    if (ld.getLaunchType() == LaunchDesc.INSTALLER_DESC_TYPE) {
		title = ResourceManager.getString("launch.launchInstaller");
	    }
	    _downloadWindow.showLaunchingApplication(title);
        }

	if (!isImport) {

	    // Detemine what JRE to use, we have not found want yet
	    if (jreInfo == null) {
		// Reread config properties with information about new JREs
		Config.refreshProps();
		jreInfo = LaunchSelection.selectJRE(ld);
		if (jreInfo == null) {
		    LaunchDescException lde = new LaunchDescException(ld,
			ResourceManager.getString("launch.error.missingjreversion"),
			null);
		    throw (new ExitException(lde, ExitException.LAUNCH_ERROR));
		}
	    }

	    // Running on wrong JRE or if we did an install, then relaunch JVM
	    JREDesc selectedJRE = ld.getResources().getSelectedJRE();
	    long minHeap = selectedJRE.getMinHeap();
	    long maxHeap = selectedJRE.getMaxHeap();
	    boolean heapSizeMatches = JnlpxArgs.isCurrentRunningJREHeap(
									minHeap, maxHeap);
	    Properties props = ld.getResources().getResourceProperties();
	    String jnlp_vmargs = selectedJRE.getVmArgs();
	    boolean isAuxMatch = JnlpxArgs.isAuxArgsMatch(props, jnlp_vmargs);

	    // For debuggeeType == JWS or debuggeeType == JNL, we always force
	    // the relaunch of a new (next) JRE, even if for instance the version
	    // of the currently running JRE is the same as the version required
	    // to run the JNL app; the forced relaunch is required since there is
	    // no other way to switch from running in debugging mode to running
	    // in non-debugging mode (JWS) or vice versa (JNL).
	    boolean jpdaMotivatedForcedRelaunch =
		(JPDA.getDebuggeeType() == JPDA.JWS	||
		 JPDA.getDebuggeeType() == JPDA.JNL);

	    if ( jpdaMotivatedForcedRelaunch ||
		 !JnlpxArgs.getJVMCommand().equals(
			new File(jreInfo.getPath())) ||
		 !heapSizeMatches || !isAuxMatch) {
		// JRE is installed. Launch separate process for this JVM
		try {
		    args = insertApplicationArgs(args);
		    execProgram(jreInfo, args, minHeap, maxHeap, props, jnlp_vmargs);
		} catch(IOException ioe) {
		    throw new ExitException(new JreExecException(jreInfo.getPath(), ioe),
					    ExitException.LAUNCH_ERROR);
		}

		// do not remove tmp file if it is a relaunch
		if (JnlpxArgs.shouldRemoveArgumentFile()) {
		    JnlpxArgs.setShouldRemoveArgumentFile(String.valueOf(false));
		}
		throw new ExitException(null, ExitException.OK);
	    }
	}

        // Remove argument file if neccesary
        JnlpxArgs.removeArgumentFile(args);

	if (isImport) {
	    _downloadWindow.disposeWindow();
	    // if importing, just do desktopIntegration and exit
            notifyLocalInstallHandler(ld, lap, forceUpdate,
				      isImport, isSilent, null);
	    Trace.println("Exiting after import", TraceLevel.BASIC);
	    throw new ExitException(null, ExitException.OK);
	}

	Trace.println("continuing launch in this VM", TraceLevel.BASIC);

        continueLaunch(lap, offlineMode, jnlpUrl, ld, forceUpdate, isImport, isSilent);

        // No need to redownload
        return null;
    }

    public static void checkCacheMax() {
        long max = Config.getCacheSizeMax();
        if (max > 0) try {
            long size = Cache.getCacheSize();
            if (size > ((max * 90)/100)) {
                String name = Config.getTempDirectory() +
                    File.separator + "cachemax.timestamp";
                File file = new File(name);
                file.createNewFile();
                long time = file.lastModified();
                long now = (new Date()).getTime();
                if ((now - time) > 60000) {
                    file.setLastModified(now);
                    final String title = ResourceManager.getString(
                        "jnlp.cache.warning.title");
                    final String message = ResourceManager.getString(
                        "jnlp.cache.warning.message",
                        sizeString(size), sizeString(max));
        		SwingUtilities.invokeAndWait(new Runnable() {
            		    public void run() {
                    		DialogFactory.showMessageDialog(
                        	    DialogFactory.WARNING_MESSAGE,
				    message, title, true);
            		    }
        		});

                }
            }
        } catch (Exception e) {
            Trace.ignoredException(e);
        }
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


    private static String sizeString(long size) {
	if (size > 1024 * 1024) {
	    return "" + size/(1024*1024) + "Mb";
	}
	return "" + size + "bytes";
    }

    private static class EatInput implements Runnable {
	private InputStream _is;

	EatInput(InputStream is) {
	    _is = is;
	}

	public void run() {
	    byte[] buffer = new byte[1024];
	    try {
		int n=0;
		while(n != -1) {
		    n = _is.read(buffer);
		}
	    } catch(IOException ioe) { /* just ignore */ }
	}

	private static void eatInput(InputStream is) {
	    EatInput eater = new EatInput(is);
	    new Thread(eater).start();
	}
    }

    private void executeInstallers(ArrayList files) throws ExitException {

        // Let progress window show launching behavior
        if (_downloadWindow.getFrame() != null) {
	    String title = ResourceManager.getString("launch.launchInstaller");
	    _downloadWindow.showLaunchingApplication(title);
	    // We will just show this window a short while, and then remove
	    // it. The installer is going to pop up pretty quickly.
	    new Thread(new Runnable() {
			public void run() {
			    try {
				Thread.sleep(5000);
			    } catch(Exception ioe) { /* ignore */ };
			    _downloadWindow.setVisible(false);
			}}).start();
        }

        for(int i = 0; i < files.size(); i++) {
	    File jnlpFile = (File)files.get(i);

	    try {
		LaunchDesc ld = LaunchDescFactory.buildDescriptor(jnlpFile);
		LocalApplicationProperties lap = Cache.
		    getLocalApplicationProperties(jnlpFile.getPath(), ld);
		lap.setLocallyInstalled(false);
		lap.store();

		// proceed with normal installation

		Trace.println("Installing extension: " + jnlpFile, TraceLevel.EXTENSIONS);

		String[] args = new String[]{ "-installer",
					      jnlpFile.getAbsolutePath() };
		// Determine JRE to run on
		JREInfo jreInfo = LaunchSelection.selectJRE(ld);
		if (jreInfo == null) {
		    _downloadWindow.setVisible(true);
		    // No JRE to run application (FIXIT: Wrong exception)
		    LaunchDescException lde = new LaunchDescException(ld,
			ResourceManager.getString("launch.error.missingjreversion"), null);
		    throw new ExitException(lde, ExitException.LAUNCH_ERROR);
		}

		// remeber whether to removeArgumentFile for the application
		boolean removeArgumentFile = JnlpxArgs.shouldRemoveArgumentFile();

		// Exec installer and wait for it to complete
		// should not remove installer JNLP file in cache
		JnlpxArgs.setShouldRemoveArgumentFile("false");
                Properties props = ld.getResources().getResourceProperties();
		Process p = execProgram(jreInfo, args, -1, -1, props, null);
		EatInput.eatInput(p.getErrorStream());
		EatInput.eatInput(p.getInputStream());
		p.waitFor();

		// reset removeArgumentFile flag for this application
		JnlpxArgs.setShouldRemoveArgumentFile(String.valueOf(removeArgumentFile));

		// Validate that installation succeded
		lap.refresh();
		if (lap.isRebootNeeded()) {
		    boolean doboot = false;
		    ExtensionInstallHandler eih =
			ExtensionInstallHandler.getInstance();
		    if (eih != null &&
			eih.doPreRebootActions(_downloadWindow.getFrame())) {
		    	doboot = true;
		    }
		    // set locally installed to be true
		    lap.setLocallyInstalled(true);
		    lap.setRebootNeeded(false);
		    lap.store();
		    if (doboot && eih.doReboot()) {
			throw new ExitException(null, ExitException.REBOOT);
		    }
		}
		if (!lap.isLocallyInstalled()) {
		    _downloadWindow.setVisible(true);
		    // Installation failed
		    throw new ExitException(new LaunchDescException(ld,
			ResourceManager.getString("Launch.error.installfailed"), null),
			ExitException.LAUNCH_ERROR);
		}
	    } catch(JNLPException je) {
		_downloadWindow.setVisible(true);
		throw new ExitException(je, ExitException.LAUNCH_ERROR);
	    } catch(IOException io) {
		_downloadWindow.setVisible(true);
		throw new ExitException(io, ExitException.LAUNCH_ERROR);
	    } catch(InterruptedException iro) {
		_downloadWindow.setVisible(true);
		throw new ExitException(iro, ExitException.LAUNCH_ERROR);
	    }
        }
    }

    public static void executeUninstallers(ArrayList files)
	throws ExitException {

        for(int i = 0; i < files.size(); i++) {
	    File jnlpFile = (File)files.get(i);

	    try {
		LaunchDesc ld = LaunchDescFactory.buildDescriptor(jnlpFile);
		LocalApplicationProperties lap = Cache.
		    getLocalApplicationProperties(jnlpFile.getPath(), ld);
		// proceed with normal installation

		Trace.println("uninstalling extension: " + jnlpFile,
				TraceLevel.EXTENSIONS);

		String[] args = new String[]{ "-silent",
					      "-secure",
					      "-installer",
					      jnlpFile.getAbsolutePath() };

		// Determine JRE to run on
		JREInfo jreInfo = LaunchSelection.selectJRE(ld);
		if (jreInfo == null) {
		    // No JRE to run application (FIXIT: Wrong exception)
		    LaunchDescException lde = new LaunchDescException(ld,
			ResourceManager.getString(
			"launch.error.missingjreversion"), null);
		    throw new ExitException(lde, ExitException.LAUNCH_ERROR);
		}

                Properties props = ld.getResources().getResourceProperties();
		Process p = execProgram(jreInfo, args, -1, -1, props, null);
		EatInput.eatInput(p.getErrorStream());
		EatInput.eatInput(p.getInputStream());
		p.waitFor();

		lap.refresh();
		if (lap.isRebootNeeded()) {
		    boolean doboot = false;
		    ExtensionInstallHandler eih =
			ExtensionInstallHandler.getInstance();
		    if (eih != null && eih.doPreRebootActions(null)) {
		    	doboot = true;
		    }
		    lap.setRebootNeeded(false);
		    lap.setLocallyInstalled(false);
		    lap.store();
		    if (doboot && eih.doReboot()) {
			throw new ExitException(null, ExitException.REBOOT);
		    }
		}
	    } catch(JNLPException je) {
		throw new ExitException(je, ExitException.LAUNCH_ERROR);
	    } catch(IOException io) {
		throw new ExitException(io, ExitException.LAUNCH_ERROR);
	    } catch(InterruptedException iro) {
		throw new ExitException(iro, ExitException.LAUNCH_ERROR);
	    }
        }
    }

    static private Process execProgram(JREInfo jreInfo, String[] args,
			long minHeap, long maxHeap, Properties props,
			String jnlp_vmargs) throws IOException {
        String javacmd = null;
	String stdjavacmd = null;
	stdjavacmd = jreInfo.getPath();
	if (Config.isDebugMode() && Config.isDebugVMMode()) {
	    javacmd = jreInfo.getDebugJavaPath();
	} else {
	    javacmd = jreInfo.getPath();
	}
	if ((javacmd.length() == 0) || (stdjavacmd.length() == 0)) {
	    throw new IllegalArgumentException("must exist");
	}

        String[] jnlpxs = JnlpxArgs.getArgumentList(stdjavacmd, minHeap, maxHeap,
						    props, jnlp_vmargs);
        int cmdssize = 1 + jnlpxs.length + args.length;
        String[] cmds = new String[cmdssize];
        int pos = 0;
        cmds[pos++] = javacmd;

        for(int i = 0; i < jnlpxs.length; i++) cmds[pos++] = jnlpxs[i];

        for(int i = 0; i < args.length; i++) cmds[pos++] = args[i];

	// (possibly) insert any JPDA arguments into command line
	cmds = JPDA.JpdaSetup(cmds, jreInfo);


	Trace.println("Launching new JRE version: " + jreInfo, TraceLevel.BASIC);
	for(int i = 0; i < cmds.length; i++) {
	    Trace.println("cmd " + i + " : " + cmds[i], TraceLevel.BASIC);
	}

        if (Globals.TCKHarnessRun) {
	    Main.tckprintln(Globals.NEW_VM_STARTING);
        }
	Trace.flush();
        return Runtime.getRuntime().exec(cmds);
    }

    private void continueLaunch(LocalApplicationProperties lap,
		boolean offlineMode, URL jnlpUrl, LaunchDesc ld,
		boolean updated, boolean isImport, boolean isSilent)
		throws ExitException {

        // Initialize the App policy stuff
        AppPolicy policy = AppPolicy.createInstance(
					ld.getCanonicalHome().getHost());

        try {
	    // Check is resources in each JNLP file is signed and prompt for
	    // certificates. This will also check that each resources downloaded
	    // so far is signed by the same certificate.
	    LaunchDownload.checkSignedResources(ld);

	    // Check signing of all JNLP files
	    LaunchDownload.checkSignedLaunchDesc(ld);
        }  catch(JNLPException je) {
	    throw new ExitException(je, ExitException.LAUNCH_ERROR);
        } catch(IOException ioe) {
	    // This should be very uncommon
	    throw new ExitException(ioe, ExitException.LAUNCH_ERROR);
        }

        // Step 3: Setup netloader
        final JNLPClassLoader netLoader =
	    JNLPClassLoader.createClassLoader(ld, policy);

        // Set context classloader to the JNLP classloader, so look up of classes
        // will start in that loader. This is important since this thread will later
        // call into the main() of the application.
        Thread.currentThread().setContextClassLoader(netLoader);

        // Step 4: Setup security
	System.setSecurityManager(new JavaWebStartSecurity());

	// set the context class loader of the AWT EventQueueThread.
	try {
	    SwingUtilities.invokeAndWait(new Runnable() {
		public void run() {
		    Thread.currentThread().setContextClassLoader(netLoader);
		    // set the LookAndFeel for the app context
		    try {
			UIManager.setLookAndFeel(UIManager.getLookAndFeel());
		    } catch (UnsupportedLookAndFeelException e) {
			e.printStackTrace();
	    		Trace.ignoredException(e);
		    }
		}
	    });
	} catch (InterruptedException ignore) {
	    Trace.ignoredException(ignore);
	} catch (InvocationTargetException ignore) {
	    Trace.ignoredException(ignore);
	}

        // Step 5: Show the console if necessary.
	SwingUtilities.invokeLater(new Runnable() {
	    public void run() {
        	JavawsConsoleController.showConsoleIfEnable();
	    }
	});


        // Step 6: Load main class
        String className = null;
        Class mainClass= null;
        try {
	    className = LaunchDownload.getMainClassName(ld, true);

	    Trace.println("Main-class: " + className, TraceLevel.BASIC);

            if (className == null) {
                throw new ClassNotFoundException(className);
            }
	    // Lookup class
	    mainClass = netLoader.loadClass(className);
            if (this.getClass().getPackage().equals(mainClass.getPackage())) {
                throw new ClassNotFoundException(className);
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
	    t.printStackTrace();
	    throw new ExitException(new Exception(),ExitException.LAUNCH_ERROR);
	}


        // At this point the user will have accepted to run the code.
        // If the code requested full access, then the AppPolicy object will
	// already have put up a dialog causing the user to confirm the launch.
        //

        // If no explicit codebase is given, then the codebase URL is
	// the base URL for the JNLP file
        URL codebase =  ld.getCodebase();
        if (codebase == null) {
	    codebase = URLUtil.getBase(jnlpUrl);
        }

        // Step 7: Setup JNLP API
	try {
	    BasicServiceImpl.initialize(codebase,
					offlineMode,
					BrowserSupport.isWebBrowserSupported());


	    // Setup ExtensionInstallation Service if needed
	    if (ld.getLaunchType() == LaunchDesc.INSTALLER_DESC_TYPE) {
		String installDir = lap.getInstallDirectory();
		if (installDir == null) {
		    installDir = Cache.getNewExtensionInstallDirectory();
		    lap.setInstallDirectory(installDir);
		}
		// Setup the install context
		ExtensionInstallerServiceImpl.initialize(installDir,
							 lap,
							 _downloadWindow);
	    }
        } catch(IOException ioe) {
	    throw new ExitException(ioe, ExitException.LAUNCH_ERROR);
        }


        // Step 8: User has accepted to launch the application. GO TO IT!
        try {
	    DownloadWindow dw = _downloadWindow;
	    _downloadWindow = null; // Reset so loading delegates will be disabled

	    // Check for desktop integration
	    notifyLocalInstallHandler(ld, lap, updated,
			isImport, isSilent, dw.getFrame());

	    // Execute main class
	    if (Globals.TCKHarnessRun) {
		Main.tckprintln(Globals.JNLP_LAUNCHING);
	    }
	    executeMainClass(ld, lap, mainClass, dw);
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
	} catch (Exception e) {
	    Trace.ignoredException(e);
	}
	if (ld.getLaunchType() == LaunchDesc.INSTALLER_DESC_TYPE) {
	    throw new ExitException(null, ExitException.OK);
	}
    }

    private boolean _shownDownloadWindow = false;

    private LaunchDesc downloadResources(LaunchDesc ld,
		boolean downloadJRE, boolean downloadJNLPFile,
		ArrayList installFiles, boolean isSilent) throws ExitException {

        // Show loading progress window. We only do this the first
        // time through. If we are downloading a JNLP file, we might
        // come here twice.
        if (!_shownDownloadWindow && !isSilent) {
	    _shownDownloadWindow = true;
	    _downloadWindow.buildIntroScreen();
	    _downloadWindow.showLoadingProgressScreen();
	    _downloadWindow.setVisible(true);
	    SplashScreen.hide();
        }

        // Download all needed resources
        try {
	    // First check for updated JNLP File
	    if (downloadJNLPFile) {
		LaunchDesc updatedLaunchDesc = LaunchDownload.getUpdatedLaunchDesc(ld);
		if (updatedLaunchDesc != null) {
		    // Found updated one, return with this,
		    // so we can restart the download process
		    return updatedLaunchDesc;
		}
	    }

	    LaunchDownload.downloadExtensions(ld, _downloadWindow, 0, installFiles);
	    if (downloadJRE) {
		LaunchDownload.downloadJRE(ld, _downloadWindow, installFiles);
	    }


	    // Check that the JNLP restrictions given in the spec. Applications
	    // with sandbox security is only allowed to referer back to a specific
	    // host and not use native libraries
	    LaunchDownload.checkJNLPSecurity(ld);

	    // Download all eagerly needed resources (what is in the cache is not ok, should
	    // do a check to the server)
	    LaunchDownload.downloadEagerorAll(ld, false, _downloadWindow, false);

        } catch(SecurityException se) {
	    // This error should be pretty uncommon. Most would have already been wrapped
	    // in a JNLPException by the downloadJarFiles method.
	    throw (new ExitException(se, ExitException.LAUNCH_ERROR));
        } catch(JNLPException je) {
	    throw (new ExitException(je, ExitException.LAUNCH_ERROR));
        } catch(IOException ioe) {
	    throw (new ExitException(ioe, ExitException.LAUNCH_ERROR));
        }
        return null;
    }

    /**
     * This invokes <code>installIfNecessaryFromLaunch</code> on
     * the LocalInstallHandler. This will also update the state of
     * the <code>LocalApplicationProperties</code>.
     */
    private void notifyLocalInstallHandler(LaunchDesc ld,
		LocalApplicationProperties lap, boolean updated,
		boolean isImport, boolean isSilent, Frame owner) {
        if (lap == null) return;
        lap.setLastAccessed(new Date());
        lap.incrementLaunchCount();
        // Notify the LocalInstallHandler
        LocalInstallHandler lih = LocalInstallHandler.getInstance();

        if (ld.isApplicationDescriptor() &&
                        ((ld.getLocation() != null) ||
                         (ld.getInformation().supportsOfflineOperation()))) {
	    if (lih != null && lih.isLocalInstallSupported()) {
		AssociationDesc [] ad = lap.getAssociations();
		if (ad != null && ad.length > 0) {
		    if (updated) {
			lih.removeAssociations(ld, lap);
			lih.createAssociations(ld, lap, true, owner);
		    }
		} else {
		   lih.createAssociations(ld, lap, isSilent, owner);
		}

	        if (lap.isLocallyInstalled()) {
                    if (updated && !lap.isLocallyInstalledSystem()) {
		        lih.uninstall(ld, lap, true);
		        lih.install(ld, lap);
		    }
	        } else {
	            lih.installFromLaunch(ld, lap, isSilent, owner);
	        }
            }
	    // add into registry:
	    if (updated) {
	        String title = ld.getInformation().getTitle();
	        String url = ld.getCanonicalHome().toString();
	        Config.getInstance().addRemoveProgramsAdd(
			Config.getInstance().toExecArg(url), title, Globals.isSystemCache());
	    }
	}

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
				  Class mainclass, DownloadWindow dw)
        throws IllegalAccessException, InstantiationException,
        InvocationTargetException, NoSuchMethodException  {

        if (ld.getLaunchType() == LaunchDesc.APPLET_DESC_TYPE) {
	    executeApplet(ld, mainclass, dw);
        } else {
	    executeApplication(ld, lap, mainclass, dw);
        }
    }

    /** Execute launchDesc for application */
    private void executeApplication(LaunchDesc ld,
	LocalApplicationProperties lap, Class mainclass, DownloadWindow dw)
	    throws IllegalAccessException, InstantiationException,
		NoSuchMethodException, InvocationTargetException   {

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
	    SplashScreen.hide();

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

	PerfLogger.setEndTime("Calling Application main");
	PerfLogger.outputLog();

        // Invoke main method
        mainMethod.invoke(null, wrappedArguments);
    }

    /** Execute launchDesc for applet */
    private void executeApplet(LaunchDesc ld, Class appletClass,
		DownloadWindow dw) throws
		IllegalAccessException, InstantiationException   {
        AppletDesc ad = ld.getAppletDescriptor();
        int height = ad.getWidth();
        int width  = ad.getHeight();

        Applet applet = null;
        applet = (Applet)appletClass.newInstance();

        // Remove all content from window
        SplashScreen.hide();
        if (dw.getFrame() == null) {
	    // Build the spash screen
	    dw.buildIntroScreen();
	    dw.showLaunchingApplication(ld.getInformation().getTitle());
        }
        final JFrame mainFrame = dw.getFrame();
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

        URL codebase = BasicServiceImpl.getInstance().getCodeBase();
        URL documentbase = ad.getDocumentBase();
        // Documentbase defaults to codebase if not specified
        if (documentbase == null) documentbase = codebase;

        // Build GUI for Applet
        final AppletContainer ac = new AppletContainer(callback,
						       applet,
						       ad.getName(),
						       documentbase,
						       codebase,
						       height,
						       width,
						       ad.getParameters());



        // We want to override the default WindowListener added inside the
        // DownloadWindow object and add a new WindowListener which calls
        // applet's stop() & destroy() method whenever window is closed.

        mainFrame.removeWindowListener(dw);
        mainFrame.addWindowListener(new WindowAdapter()
        {
             public void windowClosing (WindowEvent event)
             {
              ac.stopApplet();
              }
         });

        dw.clearWindow();
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

    private void handleJnlpFileException (LaunchDesc ld,
		Exception exception) throws ExitException {
	/* purge the bad jnlp file from the cache: */
        DiskCacheEntry dce = null;
        try {
	    dce = DownloadProtocol.getCachedLaunchedFile(ld.getCanonicalHome());
	    if (dce != null) {
		Cache.removeEntry(dce);
	    }
        } catch (JNLPException jnlpe) {
	    Trace.ignoredException(jnlpe);
        }
	Frame parent = ((_downloadWindow == null) ?
			    null : _downloadWindow.getFrame());
	throw new ExitException(exception, ExitException.LAUNCH_ERROR);
    }

    private class RapidUpdateCheck extends Thread {
        private LaunchDesc _ld;
        private LocalApplicationProperties _lap;
        private boolean _updateAvailable;
        private boolean _checkCompleted;
        private Object _signalObject = null;

        public RapidUpdateCheck() {
	    _ld = null;
	    _signalObject = new Object();
        }

        private boolean doUpdateCheck(LaunchDesc ld,
				LocalApplicationProperties lap, int timeout) {
	    _ld = ld;
	    _lap = lap;
	    boolean ret = false;

	    synchronized (_signalObject) {
		_updateAvailable = false;
		_checkCompleted = false;
		start();
	        do {
		  if (ld.getInformation().supportsOfflineOperation()) {
		    try {
		        /* wait up to timeout milli's, no answer = no Update*/
			_signalObject.wait(timeout);
			ret = _updateAvailable ;
		    } catch (InterruptedException e) {
			ret = false;
		    }
		  } else {
		    try {
		        /* wait up to timeout milli's, no answer =
			 **                    show dialog and wait some more
			 */
			_signalObject.wait(timeout);
			ret = _updateAvailable || !_checkCompleted;
		    } catch (InterruptedException e) {
			ret = true;
		    }
		  }
	        } while ((_ld.isHttps() && !_checkCompleted)	||
			(_ja != null &&
		         _ja.isChallanging())); // don't cover the JAuthenticator
					        // challange window with either
					        // the DownbloadWindow or App
	    }
	    return ret;
        }


        public void run() {
	    boolean available = false;
	    try {
		available = LaunchDownload.isUpdateAvailable(_ld);
	    } catch (FailedDownloadingResourceException fdre) {
		if (_ld.isHttps()) {
		    Throwable thr = fdre.getWrappedException();
		    if ((thr != null) && (thr instanceof
				javax.net.ssl.SSLHandshakeException)) {
		        // user chose not to accept Https cert ...
		        Main.systemExit(0);
		    }
		}
		Trace.ignoredException(fdre);
	    } catch (JNLPException je) {
		// Just ignore
		Trace.ignoredException(je);
	    }

	    synchronized (_signalObject) {
		_updateAvailable = available;
		_checkCompleted = true;
		_signalObject.notify();
	    }

	    if (_updateAvailable) {
		// Store info. in local application properties,
		// so the next time we can force an update
		_lap.setForceUpdateCheck(true);
		try {
		    _lap.store();
		} catch (IOException ioe) {
		    Trace.ignoredException(ioe);
		}
	    }
        }
    }
}
