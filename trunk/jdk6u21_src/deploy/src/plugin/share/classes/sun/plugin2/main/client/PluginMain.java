/*
 * @(#)PluginMain.java	1.90 10/05/20 18:01:33
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.main.client;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.Dialog;
import java.awt.Frame;
import java.io.*;
import java.lang.reflect.*;
import java.net.*;
import java.security.*;
import java.util.*;
import java.net.URL;
import sun.awt.*;

import com.sun.deploy.config.Config;
import com.sun.deploy.ui.DialogHook;
import com.sun.deploy.util.*;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.ui.UIFactory;
import com.sun.deploy.ui.AppInfo;
import com.sun.deploy.perf.DeployPerfUtil;
import com.sun.deploy.perf.DefaultPerfHelper;
import com.sun.deploy.perf.PerfHelper;
import com.sun.deploy.perf.PerfLabel;
import sun.plugin2.applet.*;
import sun.plugin2.ipc.*;
import sun.plugin2.liveconnect.*;
import sun.plugin2.message.*;
import sun.plugin2.message.transport.*;
import sun.plugin2.util.*;

import com.sun.java.browser.plugin2.liveconnect.v1.*;

import com.sun.javaws.exceptions.JNLPException;
import com.sun.javaws.exceptions.ExitException;

/** This is the main class which is run in the JVM connected back to
    the web browser and which executes applets. */

public class PluginMain implements ModalityInterface {
    private static final boolean DEBUG   = (SystemUtil.getenv("JPI_PLUGIN2_DEBUG") != null);
    private static final boolean VERBOSE = (SystemUtil.getenv("JPI_PLUGIN2_VERBOSE") != null);

    // This disables the sending of heartbeat messages to make it
    // easier to debug the server-side code; the client JVM won't tear
    // itself down spontaneously
    private static final boolean NO_HEARTBEAT = (SystemUtil.getenv("JPI_PLUGIN2_NO_HEARTBEAT") != null);

    // At this point we have enough dependencies on deploy.dll /
    // libdeploy.so that we need to pull it in early
    static {
        NativeLibLoader.load(new String[] {"deploy"});

        // We also need to make sure regutils is loaded up on Windows
        // to make the UpdateCheck dialog work
        if (SystemUtil.getOSType() == SystemUtil.WINDOWS) {
            NativeLibLoader.load(new String[] {"regutils"});
        }
    }

    // NOTE: this code, and the code it depends on, needs to run on platforms as far back as 1.4.2

    // The TransportFactory which creates the Transport for us;
    // keep this around to terminate it cleanly
    private TransportFactory transportFactory;
    
    // The pipe we use to communicate with our parent process
    private Pipe         pipe;

    // This is our JVM ID, assigned by the browser process
    private int jvmID;

    // On the Windows Vista platform, these are the JVM command-line
    // arguments passed up from the JP2Launcher process via the
    // sun.plugin2.jvm.args system property
    private JVMParameters vistaJVMParams;

    // Indication that this client JVM is tainted
    // The JVM need be marked only once. Once it is tainted, it will never be clean again
    private boolean jvmTainted = false;

    // Indication that this JVM was started on behalf of a standalone
    // applet (one started with the separate_jvm parameter)
    private boolean separateJVM;

    // This is needed in conjunction with the other flag and indicates
    // whether an applet relaunch was triggered from this JVM
    private boolean anyAppletRelaunchOccurred;

    // this indicates whether the jvm was started with secure
    // command line arguments
    private boolean isSecureJVM = true;

    // The class loader cache that may or may not be used by
    // individual Applet2Manager instances depending on whether the
    // classloader_cache=false parameter was specified
    private Applet2ClassLoaderCache classLoaderCache =
        new Applet2ClassLoaderCache(Applet2Manager.getCacheEntryCreator());

    // The instance cache that provides legacy lifecycle support
    private Applet2ManagerCache instanceCache = new Applet2ManagerCache();

    // These are the running applets indexed by applet ID
    private Map/*<Integer,Plugin2Manager>*/ applets =
        Collections.synchronizedMap(new HashMap/*<Integer,Plugin2Manager>*/());

    // These are the applets which have been disconnected from the
    // parent web browser due to a drag gesture.
    private Set/*<Plugin2Manager>*/ disconnectedApplets =
        Collections.synchronizedSet(new HashSet/*<Plugin2Manager>*/());

    // Whether or not we're using the ModalityListener mechanism (not
    // available in all JREs we run on) to listen for modal dialogs
    // popping up
    private boolean usingModalityListener;

    // Whether or not we need to send manual, conservative
    // notifications to the browser side because we don't have support
    // for detecting when modal dialogs are raised on this side, but
    // we need the browser to pump input messages while any potential
    // modal dialog is up. We only need to do this on the Windows
    // platform, because this is the only platform on which we block
    // the browser while a modal dialog is up.
    private boolean sendConservativeModalNotifications;

    // A bit of a hack to avoid re-activating the modal dialog too often
    private long lastReactivationTime;
    private static final long MIN_REACTIVATION_DELAY = 500;

    // Indication that we're waiting to shut down
    private volatile boolean shouldShutdown;

    // Idle timer. Shutdown Idle client VM after timeout
    private Timer idleTimer = null;

    // Keep a handle on HeartbeatThread so to suspend/resume its activities when necessary
    private HeartbeatThread hbt = null;

    // The ThreadGroup of the main thread, which we use during
    // shutdown of the disconnected applets
    private ThreadGroup mainThreadGroup;

    private static Object  progressMonitorSync = new Object();
    private static volatile boolean progressMonitorInstalled = false;

    synchronized private boolean isJVMTainted() {
	return jvmTainted;
    }

    synchronized private void startIdleTimer() {
	if (idleTimer == null) {
            idleTimer = new Timer();
            // Note use of repeating task to handle legacy lifecycle
            // cache's asynchronous expiration
            idleTimer.schedule(new AutoShutdownTask(), IDLE_TIMEOUT, IDLE_TIMEOUT);
	}
    }

    synchronized private void stopIdleTimer() {
        if (idleTimer != null) {
            idleTimer.cancel();
            idleTimer = null;
        }
    }

    private void run(String[] args) throws IOException {
        final long jvmLaunchTime;
        final long jvmLaunchCosts;
        final long pluginMainStartTime = SystemUtils.microTime();
        long t0=0;
        {
            long _jvmLaunchTime;
            try {
                _jvmLaunchTime = Long.parseLong(System.getProperty(ParameterNames.JVM_LAUNCH_TIME));
            } catch (NumberFormatException nfe) {
                nfe.printStackTrace();
                _jvmLaunchTime = pluginMainStartTime;
            }
            jvmLaunchTime = _jvmLaunchTime;
        }
        jvmLaunchCosts = pluginMainStartTime-jvmLaunchTime;
        Plugin2Manager.setJVMLaunchTime(jvmLaunchTime, jvmLaunchCosts);

        // initThread - let's warm-up some expensive class loading, AWT, etc.
        // This shapes the independent initialization into 2 threads.
        // About 200 ms until reaching end of handleMessageStartApplet(),
        // where DeployAWTUtil.invokeLater is needed.
        //
        // warm-up:  DeployAWTUtil and sun.net.ProgressMonitor.class
        // set-up :  sun.net.ProgressMonitor.setDefault(sun.plugin.util.ProgressMonitor())
        (new Thread(new Runnable() {
            public void run() {
                DeployAWTUtil.invokeLater(AppContext.getAppContext(), new Runnable() {
                    public void run() {
                        // nop ..
                    }
                });

                synchronized(progressMonitorSync) {
                    // Install progress monitor for progress bar support
                    try {
                        sun.net.ProgressMonitor.setDefault(new sun.plugin.util.ProgressMonitor());
                    } catch (Throwable e) {
                        // This is expected on JDK 1.4.2; don't display any dialog box
                    }
                    progressMonitorInstalled = true;
                    progressMonitorSync.notifyAll();
                }
            }
        })).start();

        if ( DeployPerfUtil.isEnabled() ) {
            System.out.println("PluginMain: JVM launch cost : "+jvmLaunchCosts+" us");
            System.out.println("            JVM launch time : "+jvmLaunchTime+" us");
            System.out.println("      pluginMainStartTimeJVM: "+pluginMainStartTime+" us");
            DeployPerfUtil.initialize(new DefaultPerfHelper(pluginMainStartTime));
            t0 = DeployPerfUtil.put(0, "PluginMain - run() - BEGIN (numbers are in unit of us)");
        }
        if (DEBUG && VERBOSE) {
            System.out.print("PluginMain.run({");
            for (int i = 0; i < args.length; i++) {
                if (i > 0)
                    System.out.print(" ");
                System.out.print(args[i]);
            }
            System.out.println("})");
        }

        // Get a handle to the main ThreadGroup
        mainThreadGroup = Thread.currentThread().getThreadGroup();
        //DeployPerfUtil.put("PluginMain - run() - post getThreadGroup");

        if (SystemUtil.isWindowsVista()) {
            // Capture our command-line arguments if they were passed up
            // from our custom Vista launcher
            vistaJVMParams = JVMParameters.parseFromSystemProperty("sun.plugin2.jvm.args");
            DeployPerfUtil.put("PluginMain - run() - vistaJVMParams");
        }

        // Set up the IPC mechanism
        transportFactory = TransportFactory.createForCurrentOS(args);
        //DeployPerfUtil.put("PluginMain - run() - post TransportFactory.createForCurrentOS");
        SerializingTransport transport = transportFactory.getTransport();
        //DeployPerfUtil.put("PluginMain - run() - post transportFactory.getTransport");
        // Register the messages we send to the other side
        PluginMessages.register(transport);
        //DeployPerfUtil.put("PluginMain - run() - post PluginMessages.register");
        // Create the pipe
        pipe = new Pipe(transport, false);
        //DeployPerfUtil.put("PluginMain - run() - post setup IPC");

        // Hook up default execution context for proxy and cookie
        // services called from the system AppContext.
        // This only has to have a working pipe
        Plugin2Manager.setDefaultAppletExecutionContext(new MessagePassingExecutionContext(null, pipe, -1, null));
        //DeployPerfUtil.put("PluginMain - run() - post Plugin2Manager.setDefaultAppletExecutionContext()");

        // Register ProxySelector
        // The ProxySelector class was only introduced in JDK 5
        // FIXME: will need to figure out how to make this work on JDK 1.4.2
        try {
            PluginProxySelector.initialize();
        } catch (Throwable t) {
            if (DEBUG) {
                System.err.println("Error initializing PluginProxySelector (this error is expected on 1.4.2):");
                t.printStackTrace();
            }
        }
        //DeployPerfUtil.put("PluginMain - run() - post PluginProxySelector.initialize()");

        // Register CookieSelector
        // The CookieHandler class was only introduced in JDK 5
        // FIXME: will need to figure out how to make this work on JDK 1.4.2
        try {
            PluginCookieSelector.initialize();
        } catch (Throwable t) {
            if (DEBUG) {
                System.err.println("Error initializing PluginCookieSelector (this error is expected on 1.4.2):");
                t.printStackTrace();
            }
        }
        DeployPerfUtil.put("PluginMain - run() - post PluginCookieSelector.initialize()");

        // NOTE: (FIXME) currently we only support this on Windows. We
        // might be able to get it working on X11 platforms by running
        // a subordinate X11 message loop on the browser's main
        // thread, but currently the behavior is that we simply block
        // the browser's repainting without having the necessary hooks
        // to detect when the modal dialog is obscured and bring it to
        // the front again.
        if (SystemUtil.getOSType() == SystemUtil.WINDOWS) {
            usingModalityListener = ModalityHelper.installModalityListener(this);
            sendConservativeModalNotifications = !usingModalityListener;
        } else {
            usingModalityListener = true;
        }
	DeployPerfUtil.put("PluginMain - run() - post installModalityListener()");

        // Before start the main loop, start the idle timer
        startIdleTimer();

        DeployPerfUtil.put(t0, "PluginMain - run() - END init - pre mainLoop()");

        // Ready to go
        try {
            mainLoop();
        } catch (RuntimeException e) {
            if (DEBUG) {
                e.printStackTrace();
            }
        } catch (Error e) {
            if (DEBUG) {
                e.printStackTrace();
            }
        } finally {
            // If the main loop exits and there aren't any applets outside the browser,
            // we should shut things down
            // Destroy cached legacy lifecycle applets
            instanceCache.clear();
            pipe.shutdown();
            transportFactory.dispose();
            if (disconnectedApplets.isEmpty()) {
                if (DEBUG) {
                    System.out.println("Exiting cleanly");
                    if (VERBOSE) {
                        try {
                            Thread.sleep(10000);
                        } catch (InterruptedException e) {
                        }
                    }
                }
                // FIXME: consider alternative exit codes for different situations
                exit(0);
            }
        }
    }

    private void unregisterApplet(Integer appletID, Plugin2Manager manager) {
        Plugin2Manager mappedManager = (Plugin2Manager) applets.get(appletID);
        if (DEBUG && VERBOSE) {
            Exception e = new Exception("PluginMain.unregisterApplet: "+appletID+" from "+manager+", "+mappedManager);
            e.printStackTrace();
        }
        long t0 = DeployPerfUtil.put(0, "PluginMain - unregisterApplet() - BEGIN");
        if(mappedManager!=null && !manager.equals(mappedManager)) {
            Exception e = new Exception("PluginMain.unregisterApplet: "+appletID+". Manager confusion: msg: "+manager+", map: "+mappedManager);
            e.printStackTrace();
        }

        if (separateJVM) {
            if (!anyAppletRelaunchOccurred) {
                // Tear down this JVM quickly to get rid of the stale
                // system tray icons more quickly -- let the browser side
                // deal with it
                if (DEBUG && VERBOSE) {
                    System.out.println("Exiting JVM because only applet in separate JVM just exited");
                    try {
                        Thread.sleep(10000);
                    } catch (InterruptedException e) {
                    }
                }

                exitJVM(false);
            }
            // Otherwise, the JVMInstance will take care of tearing us down
        }

        LiveConnectSupport.appletStopped(appletID.intValue());
        DragHelper.getInstance().unregister(manager);
        synchronized(applets) {
            applets.remove(appletID);

            // start idle timer or exit JVM if no active applets
            if (applets.isEmpty()) {
                startIdleTimer();
            }
        }
        DeployPerfUtil.put(t0, "PluginMain - unregisterApplet() - END");
    }

    private void registerApplet(Integer appletID, Plugin2Manager manager) {
        if (DEBUG && VERBOSE) {
            Plugin2Manager prevMgr = (Plugin2Manager) applets.get(appletID);
            Exception e = new Exception("PluginMain.registerApplet: "+appletID+" -> "+manager+", previous manager: "+prevMgr );
            e.printStackTrace();
        }
        applets.put(appletID, manager);
        LiveConnectSupport.appletStarted(appletID.intValue(), manager);
    }

    private void abortStartApplet(Plugin2Manager manager) {
        Integer appletID = manager.getAppletID();
        if (DEBUG) {
            System.out.println("PluginMain.abortStartApplet for applet ID " + appletID);
        }
        unregisterApplet(appletID, manager); // this applet is dead
        sendAppletAck(appletID);
    }

    private void sendAppletAck(Integer appletID) {
        try {
            pipe.send(new StartAppletAckMessage(null, appletID.intValue()));
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void mainLoop() {
        if (!NO_HEARTBEAT) {
            // Start up a heartbeat thread so we can tell if the parent died
            hbt = new HeartbeatThread(); 
            hbt.start();
        }

	try {
	    while (true) {
		Message msg = null;
		try {
		    msg = pipe.receive(0);
		} catch (InterruptedException e) {
		    if (DEBUG) {
			e.printStackTrace();
		    }
		}
		if (msg != null) {
		    switch (msg.getID()) {
			case SetJVMIDMessage.ID:
			    handleMessageSetJVMID((SetJVMIDMessage)msg);
			    break;

			case StartAppletMessage.ID:
			    handleMessageStartApplet((StartAppletMessage)msg);
			    break;

			case SetAppletSizeMessage.ID: 
			    handleMessageSetAppletSize((SetAppletSizeMessage)msg);
			    break;

			case SynthesizeWindowActivationMessage.ID: 
			    handleMessageSynthesizeWindowActivation((SynthesizeWindowActivationMessage)msg);
			    break;

			case PrintAppletMessage.ID:
			    handleMessagePrintApplet((PrintAppletMessage)msg);
			    break;

			case StopAppletMessage.ID: 
			    handleMessageStopApplet((StopAppletMessage)msg);
			    break;

			case GetAppletMessage.ID: 
			    handleMessageGetApplet((GetAppletMessage)msg);
			    break;

			case GetNameSpaceMessage.ID: 
			    handleMessageGetNameSpace((GetNameSpaceMessage)msg);
			    break;

			case JavaObjectOpMessage.ID: {
			    if (DEBUG && VERBOSE) {
				System.out.println("PluginMain: processing JavaObjectOpMessage");
			    }
			    LiveConnectSupport.doObjectOp((JavaObjectOpMessage) msg);
			    break;
			}

			case ReleaseRemoteObjectMessage.ID: {
			    if (DEBUG && VERBOSE) {
				System.out.println("PluginMain: processing ReleaseRemoteObjectMessage");
			    }
			    // We fake up a RemoteJavaObject instance because we know that the only thing
			    // the LiveConnectSupport will ask for is its object ID
			    RemoteJavaObject remote =
				new RemoteJavaObject(-1, -1,
				((ReleaseRemoteObjectMessage) msg).getObjectID(),
				false);
			    LiveConnectSupport.releaseRemoteObject(remote);
			    break;
			}

			case HeartbeatMessage.ID: {
			    if (DEBUG && VERBOSE) {
				System.out.println("PluginMain: processing HeartbeatMessage");
			    }
			    // Prevent livelock caused by pingponging heartbeat messages during shutdown
			    if (!shouldShutdown) {
				pipe.send(msg);
			    }
			    break;
			}

			case ShutdownJVMMessage.ID: {
			    if (DEBUG && VERBOSE) {
				System.out.println("PluginMain: processing ShutdownJVMMessage");
			    }
			    // Assume if the user set JPI_PLUGIN2_NO_HEARTBEAT that they want
			    // even tainted JVM instances to stick around for debugging purposes
			    if (!NO_HEARTBEAT) {
				if (disconnectedApplets.isEmpty()) {
				    // Destroy cached legacy lifecycle applets
				    instanceCache.clear();
				    exitJVM(true);
				}
			    }
			    // Normally not reached
			    break;
			}

			default:
			    // FIXME: need more messages
			    // Consider throwing an InternalError here since this would be entirely our fault
			    System.err.println("sun.plugin2.main.client.PluginMain: unrecognized message ID " + msg.getID());
		    }
		}
	    }
	} catch (IOException e) {
	    if (DEBUG) {
		e.printStackTrace();
	    }
	} catch (JNLPException e) {
	    if (DEBUG) {
		e.printStackTrace();
	    }
	}
    }

    private void exitJVM(boolean cleanly) {
        try {
            if (cleanly) {
                pipe.shutdown();
                transportFactory.dispose();
            }
        } catch (Exception e) {
        } finally {
            exit(0);
        }
    }

    private static void exit(int statusCode) {
        try {
            System.exit(statusCode);
        } catch (IllegalThreadStateException e) {
            // This only happens if a ShutdownHook is used improperly
            // on pre-1.6 JREs
            Runtime.getRuntime().halt(statusCode);
        }
    }

    private void handleMessageSetJVMID(SetJVMIDMessage jvmMsg) throws IOException, JNLPException
    {
        long t0 = DeployPerfUtil.put(0, "PluginMain - handleMessageSetJVMID() - BEGIN");
        if (DEBUG && VERBOSE) {
            System.out.println("PluginMain: processing SetJVMIDMessage, params:");
        }
        //DeployPerfUtil.put("PluginMain - handleMessageSetJVMID() - BEGIN");

        // Get a notion of whether this JVM was started on behalf of a
        // single applet passing the separate_jvm parameter. We do
        // this rather than looking at the StartAppletMessage because
        // we don't want to do the wrong thing if for some reason in
        // the future the browser-side heuristics are changed and an
        // applet which requested separate_jvm actually had to share a
        // JVM instance with another applet.
        separateJVM = jvmMsg.isSeparateJVM();

        JVMParameters params = null;

        // On non-Vista platforms, we get the client VM java arguments from this 
        // message. On Vista, they are retrieved from a system property. The messages
        // coming over from the browser must be considered untrusted, and we make
        // security decisions based on the arguments. Our trusted jp2launcher on
        // Vista passes us the command line arguments via this system property.
        if (SystemUtil.isWindowsVista()) {
            params = vistaJVMParams;
        } else {
            String[][] paramsArrays = jvmMsg.getParameters();
            params = new JVMParameters();
            params.getFromStringArrays(paramsArrays);
        }

        if (DEBUG && VERBOSE) {
            List/*<String>*/ subordinateArgs = params.getCommandLineArguments(false, false);
            for (Iterator iter = subordinateArgs.iterator(); iter.hasNext(); ) {
                System.out.println("\t<"+(String) iter.next()+">");
            }
        }

        isSecureJVM = params.isSecure();

        if (DEBUG) {
            System.out.println("PluginMain: The running JVM is " +
                               (isSecureJVM ? "" : "NOT ") + "secure+\n\tJVMParameters: "+params);
        }

        // make this JVM's parameter accessible deployment wide
        JVMParameters.setRunningJVMParameters(params);
        if (DEBUG && VERBOSE) {
            System.out.println("Running JVMParams: "+params+"\n\t-> "+JVMParameters.getRunningJVMParameters());
        }
        //DeployPerfUtil.put("PluginMain - handleMessageSetJVMID() - post setRunningJVMParameters()");

        // Set up the ServiceDelegate class based on the type of browser
        // the server indicated is in use
        ServiceDelegate.initialize(jvmMsg.getBrowserType());
        //DeployPerfUtil.put("PluginMain - handleMessageSetJVMID() - post ServiceDelegate.initialize()");

        // Set things up for executing applets

        // For backward compatibility with the old plugin, we're setting
        // the "user.home" property to the value of the USERPROFILE env. 
        // variable on windows platforms. The value is being sent from 
        // the server side via the SetJVMIDMessage.

        String userHome = jvmMsg.getUserHome();
        // On Unix, jvmMsg.getUserHome() returns null.
        if (userHome != null) {
            Config.getInstance().setUserHomeOverride(userHome);
        }

        try {
            // Note: if you're debugging the bootstrapping sequence
            // (before the Java Console shows up), and aren't seeing
            // any output, try temporarily disabling the redirectStdio
            // flag below
            Applet2Environment.initialize(params.getCommandLineArgumentsAsString(false),
                                          true, false,
                                          new Plugin2ConsoleController(classLoaderCache,
                                                                       instanceCache),
                                          new MessagePassingExecutionContext(null, pipe, -1, null),
                                          getDialogHook());

            // static JNLP environment initialization once in a JRE lifetime
            JNLP2Manager.initializeExecutionEnvironment();
        } catch (Error e) {
            e.printStackTrace();
            throw(e);
        } catch (RuntimeException e) {
            e.printStackTrace();
            throw(e);
        } catch (JNLPException e) {
            e.printStackTrace();
            throw(e);
        }

        // Initialize LiveConnect support
        LiveConnectSupport.initialize(pipe, jvmMsg.getJVMID());

        // Tell the browser we have successfully started up
        pipe.send(new JVMStartedMessage(null));

        t0 = DeployPerfUtil.put(t0, "PluginMain - handleMessageSetJVMID() - END");
        DeployPerfUtil.put("PluginMain - Init - END ;  Total Time: "+
            (Plugin2Manager.getJVMLaunchCosts()+t0)+" us");
    }

    private void handleMessageSetAppletSize(SetAppletSizeMessage setSizeMessage)
    {
        if (DEBUG && VERBOSE) {
            System.out.println("PluginMain: processing SetAppletSizeMessage");
        }
        setAppletSize(new Integer(setSizeMessage.getAppletID()), 
                     setSizeMessage.getWidth(), setSizeMessage.getHeight());
    }

    private void setAppletSize(Integer appletID, int width, int height)
    {
        Plugin2Manager manager =
            (Plugin2Manager) applets.get(appletID);
        if (manager != null) {
            if (DEBUG) {
                System.out.println("PluginMain: setting size of applet " +
                                   appletID + " to (" + width + ", " + height + ")");
            }
            manager.setAppletSize(width, height);
        }
    }

    /**
     * handleMessageStartApplet 
     * handles the task:
     *  - of starting the applet in it's own thread
     *  - while having an applet2listener to intercept results and events
     *    which may lead to send back a modified start message.
     * 
     * because of the start message modification,
     * we cannot use implicit interface implementation using inner classes
     */
    private void handleMessageStartApplet(final StartAppletMessage startMessage)
    {
        long t0 = DeployPerfUtil.put(0, "PluginMain - handleMessageStartApplet() - BEGIN");
        if (DEBUG && VERBOSE) {
            System.out.println("PluginMain: processing StartAppletMessage");
        }

        // stop the idle timer
        stopIdleTimer();

        Map params = startMessage.getParameters();
	final boolean isForDummyApplet = startMessage.isForDummyApplet();

        Plugin2Manager _mgr = null;
        String tmpDocBase = startMessage.getDocumentBase();

        try {
            // Canonicalize URL in case the URL is in some
            // weird form only recognized by the browsers
            //
            tmpDocBase = URLUtil.canonicalize(tmpDocBase);
            tmpDocBase = URLUtil.canonicalizeDocumentBaseURL(tmpDocBase);
            // Need to "normalize" the document base by creating a temporary URL for it
            tmpDocBase = new URL(tmpDocBase).toString();
        } catch (MalformedURLException e) {
	} catch (NullPointerException npe) {
	    // continue, don't kill the PluginMain thread
        }

        final String documentBase = tmpDocBase;
        String jnlpFile = null;
        try {
            jnlpFile = (String) params.get(JNLP2Tag.JNLP_HREF);
        } catch (Exception e) {
        }

        boolean relaunched = false;
        {
            String s1 = (String) params.get(ParameterNames.APPLET_RELAUNCHED);
            try {
                relaunched = Boolean.valueOf(s1).booleanValue();
            } catch (Exception e) { }
        }

        if ( null == jnlpFile ) {
            // See if the legacy applet lifecycle is in use
            _mgr = instanceCache.get(documentBase, params);
            if (_mgr == null) {
                // Need to create a new one
		Applet2ClassLoaderCache loaderCache = isForDummyApplet? null : getClassLoaderCacheForManager(params);
                _mgr = new Applet2Manager(loaderCache, instanceCache, relaunched);
            }
        } else {
            // we assume the jnlp applet is choosen
            jnlpFile = URLUtil.canonicalize(jnlpFile);
            String codebase = null;

            // optional codebase within applet tag
            try {
                codebase = (String) params.get("java_codebase");
                if (codebase == null) {
                    codebase = (String) params.get("codebase");
                }
            } catch (Exception e) {  }

            try {
                URL documentBaseURL = new URL(documentBase); 

                _mgr = new JNLP2Manager(codebase, documentBaseURL, jnlpFile, relaunched);

            } catch (Exception e) { 
                System.out.println("PluginMain: JNLP2Manager creation: "+e);
                e.printStackTrace();
            }
        }
        final Integer appletID = new Integer(startMessage.getAppletID());

        if (null == _mgr ) {
            // if we don't have a Plugin2Manager at this point,
            // we have to abort our mission ..
            if (DEBUG && VERBOSE) {
                System.out.println("PluginMain: Couldn't deduce a Plugin2Manager - bail out");
            }
            sendAppletAck(appletID);
            return;
        }

        final Plugin2Manager manager = _mgr;
        manager.setAppletID(appletID);
        if (isForDummyApplet) {
            manager.setForDummyApplet(true);
        }

        // set secure vm flag in the manager
        manager.setSecureFlag(isSecureJVM);

        if (DEBUG) {
            System.out.println("PluginMain: starting applet ID " + appletID + " in parent window 0x" +
                               Long.toHexString(startMessage.getParentNativeWindowHandle()) + " with parameters:");
            System.out.println("    Document base = " + documentBase);
            for (Iterator iter = params.keySet().iterator(); iter.hasNext(); ) {
                String key = (String) iter.next();
                System.out.println("    " + key + "=" + (String) params.get(key));
            }
        }
        manager.setAppletExecutionContext(new MessagePassingExecutionContext(params, 
                                                                             pipe, 
                                                                             appletID.intValue(),
                                                                             documentBase));
        DeployPerfUtil.put("PluginMain - setAppletExecutionContext()");
        final AppContext appContext = manager.getAppletAppContext();
        DeployPerfUtil.put("PluginMain - getAppletAppContext()");
        registerApplet(appletID, manager);
        DeployPerfUtil.put(t0, "PluginMain - handleMessageStartApplet() - END");

	new Thread(appContext.getThreadGroup(), new Runnable() {
		public void run() {
		    if (!initManager(manager)) {
			return;
		    }

		    Runnable runner = 
			new StartAppletRunner( manager, startMessage,
					       (usingModalityListener)? null : PluginMain.this);
		    if (!isForDummyApplet) {
			DeployAWTUtil.invokeLater(appContext, runner);
		    } else {
			// Break AWT-level deadlocks with applets that call
			// JavaScript from the EDT and apparently prevent new EDT
			// threads from starting in some situations
			manager.startWorkerThread("Applet " + appletID + " start thread", runner);
		    }
		}
	    }).start();
    }

    class PluginMainDragListener implements DragListener {
        public void appletDroppedOntoDesktop(Plugin2Manager manager) {
            // Move this applet to the disconnected set. This has the
            // semantics that when we switch away from the web page
            // from which the applet came, we will cut its ties to the
            // browser by swapping out its Applet2ExecutionContext.
            applets.remove(manager.getAppletID());
            disconnectedApplets.add(manager);

	    // install desktop shortcut if "eager_install" is specified
	    if (manager.isEagerInstall()) {
		manager.installShortcuts();
	    }
        }

        public void appletExternalWindowClosed(final Plugin2Manager manager) {
            if (!manager.isDisconnected()) {
                // The web page where the applet originated from wasn't closed yet.
                // Put back the applet where it came
                DragHelper.getInstance().restore(manager);
                applets.put(manager.getAppletID(), manager);
                disconnectedApplets.remove(manager);
            } else {
                // Shut down the applet
                // Need to do this work outside of the applet's
                // ThreadGroup, which is being torn down
                // Need doPrivileged because there might be user code on the stack
                AccessController.doPrivileged(new PrivilegedAction() {
                        public Object run() {
                            new Thread(mainThreadGroup, new Runnable() {
                                    public void run() {
                                        // Shut down LiveConnect if it hasn't been already
                                        LiveConnectSupport.appletStopped(manager.getAppletID().intValue());
                                        // FIXME: the call to stop here also kills the
                                        // Java logo we put in place of the applet
                                        manager.stop(null, null);
                                        disconnectedApplets.remove(manager);
                                        if (shouldShutdown && disconnectedApplets.isEmpty()) {
                                            // Destroy cached legacy lifecycle applets (just to
                                            // call destoy() against them)
                                            instanceCache.clear();
                                            // Exit this JVM instance
                                            exit(0);
                                        }
                                    }
                                }).start();
                            return null;
                        }
                    });
            }
        }
    }
    private DragListener pluginMainDragListener;
    private DragListener getDragListener() {
        if (pluginMainDragListener == null) {
            pluginMainDragListener = new PluginMainDragListener();
        }
        return pluginMainDragListener;
    }

    private boolean initManager(Plugin2Manager manager) {
	try {
	    manager.initialize(); 
	} catch (Exception e) {
	    e.printStackTrace();
	    String msg = "Error while initializing manager: "+e;
	    System.err.println(msg+", bail out");
	    abortStartApplet(manager);
	    return false; // bail out.
	}
	DeployPerfUtil.put("PluginMain.StartAppletRunner - post manager.initialize()");
	return true;
    }
    
    class StartAppletRunner implements Runnable
    {
        final ModalityInterface modalityInterface;
        final Plugin2Manager manager;
        StartAppletMessage startMessage;

        StartAppletRunner (final Plugin2Manager manager, StartAppletMessage startMessage, 
                           final ModalityInterface modalityInterface) {
            this.manager = manager;
            this.startMessage = startMessage;
            this.modalityInterface = modalityInterface; // may be null
        }

        public void run() {
            long t0 = DeployPerfUtil.put(0, "PluginMain.StartAppletRunner - BEGIN");
	    Integer appletID = new Integer(startMessage.getAppletID());
	    Map params = startMessage.getParameters();
	    boolean isForDummyApplet = startMessage.isForDummyApplet();
	    
	    DeployPerfUtil.put("PluginMain.StartAppletRunner - post startMessage.get*()");

            PluginEmbeddedFrame f = null;
            if (!isForDummyApplet) {
                try {
                    f = new PluginEmbeddedFrame(startMessage.getParentNativeWindowHandle(),
                                                startMessage.getParentConnection(),
                                                startMessage.useXEmbed(), modalityInterface,
                                                pipe, appletID.intValue());
                } catch (Exception e) {
                    // We want to be robust in case this occurs
                    e.printStackTrace();
                }
                if (f == null) {
                    // If we failed to create the embedded frame for the applet
                    // (which is a very deep failure), don't even consider it
                    // to have started
                    System.out.println("PluginMain: could not create embedded frame");
                    abortStartApplet(manager);

                    return; // bail out ..
                }
            }
            DeployPerfUtil.put("PluginMain.StartAppletRunner - post createEmbeddedFrame()");

            if (!isForDummyApplet) {
                f.setLayout(new BorderLayout());
                manager.setAppletParentContainer(f);
                // Parse the width and height out of the applet tag, but don't let a
                // malformed applet tag prevent us from showing anything on the screen
                int defaultWidth  = 256;
                int defaultHeight = 256;
                int width  = defaultWidth;
                int height = defaultHeight;
                String w = (String) params.get("width");
                String h = (String) params.get("height");
                try {
                    width = Integer.parseInt(w);
                    height = Integer.parseInt(h);
                } catch (Exception e) {
                    width  = defaultWidth;
                    height = defaultHeight;
                    System.err.println("Error parsing width (\"" + w + "\") or height (\"" + h + "\")");
                    System.err.println("Defaulting to (" + width + ", " + height + ")");
                }

		// Parse the boxbgcolor out of the applet tag. Set the embeddedframe
		// to use the background color
		String bgColorStr = (String) params.get(ParameterNames.BOX_BG_COLOR);
		Color bgColor = Color.lightGray; // default
		if (bgColorStr != null) {
		    bgColor = ColorUtil.createColor(ParameterNames.BOX_BG_COLOR, bgColorStr);
		}

		if (bgColor != null) {
		    f.setBackground(bgColor);
		}
		
                f.setVisible(true);
                setAppletSize(appletID, width, height);

                if (DEBUG) {
                    System.out.println("Made EmbeddedFrame for applet " +
                                       startMessage.getAppletID() + " visible");
                }

		// If the applet has requested it, set up the support
		// for dragging and dropping it to the desktop
		if (Boolean.valueOf((String) params.get("draggable")).booleanValue() ||
		    // FIXME: hack for testing
		    SystemUtil.getenv("JPI_PLUGIN2_FORCE_DRAGGABLE") != null) {
		    DragHelper.getInstance().register(manager, getDragListener());
		}
            }
            //DeployPerfUtil.put("PluginMain.StartAppletRunner - post embeddedFrame setup");

	    manager.addAppletListener(new StartAppletListener(manager, startMessage));
	    
	    // Wait until progress monitor for progress bar support is installed
	    int progressMonitorSynced=0;
	    if ( !progressMonitorInstalled ) {
		progressMonitorSynced++;
		synchronized (progressMonitorSync) {
		    if ( !progressMonitorInstalled )
			{
			    progressMonitorSynced++;
			    try {
				progressMonitorSync.wait();
			    } catch (InterruptedException ie) {}
			}
		}
	    }
	    //DeployPerfUtil.put("PluginMain.StartAppletRunner - post sun.plugin.util.ProgressMonitor(), sync:"+progressMonitorSynced);
            DeployPerfUtil.put(t0, "PluginMain.StartAppletRunner - END");
            manager.start();
        }
    }

    class StartAppletListener implements Applet2Listener 
    {
        final Plugin2Manager manager;
        StartAppletMessage startMessage;
        boolean heartbeatSuspended;
	boolean _ssvValidated = false;

        StartAppletListener (final Plugin2Manager manager, StartAppletMessage startMessage) {
            this.manager = manager;
            this.startMessage = startMessage;
            if (hbt != null) {
                heartbeatSuspended=true;
                hbt.suspendHeartbeat();
            } else {
                heartbeatSuspended=false;
            }
        }

        private void resumeHeartbeat() {
            if (hbt != null && heartbeatSuspended) {
                heartbeatSuspended=false;
                hbt.resumeHeartbeat();
            }
        }

        public void released(Plugin2Manager hostingManager) {
            resumeHeartbeat();
        }

	public String getBestJREVersion(Plugin2Manager bostingManager, String javaVersionStr) {
	    String bestJREVersion = null;
	    if (javaVersionStr != null) {
		Conversation c = pipe.beginConversation();
		BestJREAvailableMessage bestJREMessage = 
		    new BestJREAvailableMessage(c, BestJREAvailableMessage.ASK, javaVersionStr);
		try {
		    pipe.send(bestJREMessage);
		    BestJREAvailableMessage replyMessage = 
			(BestJREAvailableMessage)pipe.receive(0,c);
		    if (replyMessage.isReply()) {
			bestJREVersion = replyMessage.getJavaVersion();
		    }
		} catch (Exception e) {
		    e.printStackTrace();
		}
	    }
	    return bestJREVersion;
	}

        /** This method checks if the requested java version satisfies
         *  the security baseline. If not, it pops up a security dialog
         *  asking the user's permission to run the applet with the insecure
         *  JRE. If the user doesn't grant the permission, it'll check if the
         *  running JRE also satisfies the security baseline and pops up a 
         *  security dialog if necessary.
         * 
         *  @param hostingManager Plugin2Manager for this applet
         *  @return true if it's OK to relaunch the applet with the requested 
         *  version of JRE, false to continue in current jvm.
         *  @exception ExitException if there's no JRE to relaunch the applet
         */
        public boolean appletSSVValidation(Plugin2Manager hostingManager) throws ExitException {
	    boolean SSVValidated = Boolean.valueOf(hostingManager.getParameter(ParameterNames.SSV_VALIDATED)).booleanValue();

	    if (SSVValidated) {
		return false;
	    }
	    String javaVersion = hostingManager.getParameter(ParameterNames.SSV_VERSION);
	    String runningJavaVersion = System.getProperty("java.version");


            // if the requested javaVersion satisfies the security baseline
            // but not equal to the running version, we return true to 
            // indicate relaunching the applet with the requested version of JRE
            if (javaVersion != null &&
               !javaVersion.equals(runningJavaVersion) &&
               SecurityBaseline.satisfiesSecurityBaseline(javaVersion)) {
                   _ssvValidated = true;
                   return true;
            }
	   
	    boolean ssvRelaunch = false;

	    if (javaVersion != null && 
		!javaVersion.equals(runningJavaVersion) &&
		!SecurityBaseline.satisfiesSecurityBaseline(javaVersion) &&
		!hostingManager.getAppletClassLoader().getSSVDialogShown()) {
                ssvRelaunch = showSSVDialog(javaVersion);
		if (ssvRelaunch == false) {
		    // remember cancel in classloader
		    hostingManager.getAppletClassLoader().setSSVDialogShown(true);
		}
            } else {
                ssvRelaunch = false;
            }
	    // earmark in relaunch parameter that it has been SSV validated
	    _ssvValidated = true;

	    // if user cancelled, continue to run in the current vm
	    // need check if the current vm satisfies the security baseline
	    // this is for the scenario that the latest available jre does not
	    // satisfiy the security baseline.
	    if (ssvRelaunch == false) {
		boolean okToRun = true;
		if (!SecurityBaseline.satisfiesSecurityBaseline(runningJavaVersion)) {
		    okToRun = showSSVDialog(runningJavaVersion);
		}

		if (okToRun) {
                    return false;
		} else {
		    // no jvm is available for relaunch, shows the "Red X" error icon
		    throw new ExitException(new Exception("No JVM to relaunch applet"),
					    ExitException.JRE_MISMATCH);
		}
	    }
	    return ssvRelaunch;
        }

	private boolean showSSVDialog(String javaVersion) {
	    String title = ResourceManager.getString("javaws.ssv.title");

	    String message = ResourceManager.getString(
		"javaws.ssv.runold.masthead");
	    
	    String bullet = ResourceManager.getString(
		"javaws.ssv.runold.bullet", javaVersion);
	    
	    String run = ResourceManager.getString("javaws.ssv.run.button");
	    String cancel = ResourceManager.getString("common.cancel_btn");
	    
	    final URL docBase = manager.getDocumentBase();
	    
	    AppInfo ai = new AppInfo();
	    ai.setFrom(docBase);
            
	    int result = UIFactory.ERROR;
            
	    // the AppInfo (ai) is required so that the UIFactory won't 
	    // insert unnecessary string (such as "Name:") in the center
	    // pane of the dialog
	    result = UIFactory.showConfirmDialog(null, ai,
						 message, bullet, title, run, cancel, true);
	    return ((result == UIFactory.OK) ? true : false);
	}

	public boolean isAppletRelaunchSupported() {
	    return true;
	}

        public void appletJRERelaunch(Plugin2Manager hostingManager, String javaVersion, String jvmArgs) {
                anyAppletRelaunchOccurred = true;

                // make sure the Conversation object associated with the 
                // StartAppletMessage is null because Conversation is only used
                // for a 1-to-1 type of communications between the client JVM 
                // and the browser such as in the case of a liveconnect call
                Conversation conversation = startMessage.getConversation();
                assert(conversation == null);

                Integer appletID = new Integer(startMessage.getAppletID());
                unregisterApplet(appletID, hostingManager);

                // check, if we are in a relaunch loop, ie already were here;
                // but allow relaunch attempts to the latest javaVersion, ie null!
                if(null!=javaVersion && hostingManager.isAppletRelaunched()) {
                    // FIXME: show appropriate message
                    throw new InternalError("appletJRERelaunch: incorrectly looped in relaunch code");
                }

                Map appletParams = startMessage.getParameters();

                // Pass back the "__applet_ssv_version" for the next launch
                if(null!=javaVersion) {
                    appletParams.put(ParameterNames.SSV_VERSION, javaVersion);
                } else {
                    appletParams.remove(ParameterNames.SSV_VERSION);
                }

                // Pass back the command-line arguments for the next launch
                appletParams.put(ParameterNames.JAVA_ARGUMENTS, jvmArgs);

                // Indicate in the parameter map that a relaunch has occurred
                appletParams.put(ParameterNames.APPLET_RELAUNCHED, String.valueOf(true));

                // Pass the JRE_INSTALLED parameter of the manager to the server,
                // indicating that we just have installed a new JRE
                String tmp = hostingManager.getParameter(ParameterNames.JRE_INSTALLED);
                if (null!=tmp) {
                    appletParams.put(ParameterNames.JRE_INSTALLED, tmp);
                }

		// Pass the SSV_VALIDATED parameter to the server
		// indicating that we no longer need check ssv validation
		if (_ssvValidated) {
		    appletParams.put(ParameterNames.SSV_VALIDATED, "true");
		}

                startMessage.setParameters(appletParams);

                if (DEBUG) {
                    System.out.println("PluginMain.StartAppletListener: appletJRERelaunch for applet ID " + hostingManager.getAppletID());
                    System.out.println("\t javaVersion: "+javaVersion);
                    System.out.println("\t jvmArgs: "+jvmArgs);
                }

                // sending back a StartAppletMessage indicates to the browser to do a re-launch 
                // of the applet on the given available JRE version.
                // if no version is specified, the latest shall be used
                try {
                    pipe.send(startMessage);
                } catch (IOException e) {
                    if (DEBUG) {
                        e.printStackTrace();
                    }
                }
                resumeHeartbeat();

        }

        public void appletLoaded(Plugin2Manager hostingManager) {
            if (DEBUG) {
                System.out.println("PluginMain.StartAppletListener: appletLoaded for applet ID " + hostingManager.getAppletID());
            }
            sendAppletAck(new Integer(startMessage.getAppletID()));
            resumeHeartbeat();
        }

        public void appletReady(Plugin2Manager hostingManager) {
            // We ignore this notification
        }

        public void appletErrorOccurred(Plugin2Manager hostingManager) {
            if (DEBUG) {
                System.out.println("PluginMain.StartAppletListener: appletErrorOccurred for applet ID " + hostingManager.getAppletID());
            }
            sendAppletAck(hostingManager.getAppletID());
            resumeHeartbeat();
        }
    }

    // This is used by the some of the message handlers below that we
    // want to support for applets dragged out of the browser but not
    // yet fully disconnected from the browser
    private Plugin2Manager getActiveOrDisconnectedApplet(int appletID) {
        Integer key = new Integer(appletID);
        Plugin2Manager manager = (Plugin2Manager) applets.get(key);
        if (manager != null) {
            return manager;
        }
        // Try the disconnected applets, which can still be called
        // from JavaScript and printed, for example
        synchronized(disconnectedApplets) {
            for (Iterator iter = disconnectedApplets.iterator(); iter.hasNext(); ) {
                manager = (Plugin2Manager) iter.next();
                if (key.equals(manager.getAppletID())) {
                    return manager;
                }
            }
        }
        return null;
    }

    private void handleMessageSynthesizeWindowActivation(final SynthesizeWindowActivationMessage message)
    {
        if (DEBUG && VERBOSE) {
            System.out.println("PluginMain: processing SynthesizeWindowActivationMessage");
        }
        final Integer appletID = new Integer(message.getAppletID());
        Plugin2Manager manager =
            (Plugin2Manager) applets.get(appletID);
        if (manager != null) {
            final PluginEmbeddedFrame container = (PluginEmbeddedFrame) manager.getAppletParentContainer();
            DeployAWTUtil.invokeLater(manager.getAppletAppContext(), new Runnable() {
                    public void run() {
                        if (DEBUG) {
                            System.out.println("Calling synthesizeWindowActivation(" +
                                               message.getActive() + ") for applet " + message.getAppletID());
                        }
                        if ((container != null) && !modalDialogHasPopped) {
                            try {
                                container.synthesizeWindowActivation(message.getActive());
                            } catch (NoSuchMethodError e) {
                                // This method doesn't exist on some earlier JDKs
                            }
                        }
			if (modalDialogHasPopped) {
			    modalDialogHasPopped = false;
			}
                        if (message.getActive()) {
                            // See whether there's a modal dialog blocking this applet and, if so,
                            // raise the blocker to the front
                            Dialog blocker = getModalDialogForApplet(appletID);
                            if (blocker != null) {
                                blocker.toFront();
                                blocker.requestFocus();
                                if (DEBUG) {
                                    System.out.println("  Called Dialog.toFront() / requestFocus() for blocker of applet ID " + appletID);
                                }
                                if (lastReactivationTime == 0 ||
                                    System.currentTimeMillis() > lastReactivationTime + MIN_REACTIVATION_DELAY) {
                                    ModalityHelper.reactivateDialog(blocker);
                                    lastReactivationTime = System.currentTimeMillis();
                                }
                            } else {
                                if (container != null) {
                                    container.requestFocus();
                                    if (DEBUG) {
                                        System.out.println("  Called PluginEmbeddedFrame.requestFocus()");
                                    }
                                }
                            }
                        }
                    }
                });
        }
    }

    private void handleMessagePrintApplet(final PrintAppletMessage printAppletMessage)
    {
        if (DEBUG && VERBOSE) {
            System.out.println("PluginMain: processing PrintAppletMessage");
        }
        final Plugin2Manager manager = getActiveOrDisconnectedApplet(printAppletMessage.getAppletID());

        if (manager != null) {
            if (DEBUG) {
                System.out.println("PluginMain: printing applet " +
                                   printAppletMessage.getAppletID() + " isPrinterDC = " +
                                   printAppletMessage.getIsPrinterDC());
            }

            DeployAWTUtil.invokeLater(manager.getAppletAppContext(), new Runnable() {
                public void run() {
                    try {
                        if (hbt != null) {
                            hbt.suspendHeartbeat();
                        }

                        boolean res = ClientPrintHelper.print(manager, 
                                                              printAppletMessage.getAppletID(), 
                                                              printAppletMessage.getHDC(), 
                                                              printAppletMessage.getIsPrinterDC(),
                                                              printAppletMessage.getX(), 
                                                              printAppletMessage.getY(),
                                                              printAppletMessage.getWidth(), 
                                                              printAppletMessage.getHeight(), 
                                                              pipe);

                        final PrintAppletReplyMessage reply = 
                            new PrintAppletReplyMessage(printAppletMessage.getConversation(),
                                                        printAppletMessage.getAppletID(), 
                                                        res);
                        pipe.send(reply);
                    } catch (IOException e) {
                        if (DEBUG) {
                            e.printStackTrace();
                        }
                    } finally {
                        if (hbt != null) {
                            hbt.resumeHeartbeat();
                        }
                    }                                                                                       
                }
            });
        }   
    }

    private void handleMessageStopApplet(final StopAppletMessage stopMessage) throws IOException
    {
        DeployPerfUtil.setInitTime(SystemUtils.microTime());
        long t0 = DeployPerfUtil.put(0, "PluginMain - handleMessageStopApplet() - BEGIN (time reset)");
        if (DEBUG) {
            System.out.println("PluginMain: processing StopAppletMessage, applet ID " + stopMessage.getAppletID());
        }

        final Integer key = new Integer(stopMessage.getAppletID());
        Plugin2Manager manager = (Plugin2Manager) applets.get(key);

        if (manager != null) {
            final Plugin2Manager f_manager = manager;
            Runnable afterStopRunnable = new Runnable() {
                public void run() {
                    unregisterApplet(key, f_manager);
                }
            };

            if (!isJVMTainted()) {
                        // We are using a new MarkTaintedMessage to indicate Applet stop
                // failure. To make it simple, we don't piggyback StopAppletpAckMessage with
                // return status. StopAppletAckMessage acknowleges the reception of 
                // StopAppletMessage. It does not return status of applet stop.
                Applet2StopListener listener = new Applet2StopListener() {
                    public void stopFailed() {
                        synchronized (PluginMain.this) {
                            if (jvmTainted) {
                            //need mark only once
                            return;
                            }
                            
                            jvmTainted = true;
                        }

                        //send MarkTaintedMessage to the browser side
                        final MarkTaintedMessage message = new MarkTaintedMessage(null);
                        try {
                            pipe.send(message);
                        } catch (IOException e) {
                            if (DEBUG) {
                            e.printStackTrace();
                            }
                        }
                    }
                };

                DeployPerfUtil.put("PluginMain - handleMessageStopApplet() - 1 - manager.stop() - START");
                manager.stop(afterStopRunnable, listener);
                DeployPerfUtil.put("PluginMain - handleMessageStopApplet() - 1 - manager.stop() - END");
            } else {
                DeployPerfUtil.put("PluginMain - handleMessageStopApplet() - 2 - manager.stop() - START");
                manager.stop(afterStopRunnable);
                DeployPerfUtil.put("PluginMain - handleMessageStopApplet() - 2 - manager.stop() - END");
            }
        } else {
            // See if this is one of the disconnected applets (those
            // dragged and dropped to the desktop), and if so, fully
            // disconnect it from the web browser
            // FIXME: eventually need to provide a more complete
            // context for the disconnected applet to work with
            synchronized(disconnectedApplets) {
                for (Iterator iter = disconnectedApplets.iterator(); iter.hasNext(); ) {
                    manager = (Plugin2Manager) iter.next();
                    if (key.equals(manager.getAppletID())) {
                        String documentBase = 
                            manager.getAppletExecutionContext().getDocumentBase(manager);
                        Map/*<String,String>*/ params =
                            manager.getAppletExecutionContext().getAppletParameters();
                        manager.setDisconnected();
                        manager.setAppletExecutionContext(
                            new DisconnectedExecutionContext(params, documentBase));
                        // Create a shortcut for this applet if we can
			if (!manager.isEagerInstall()) {
			    manager.installShortcuts();
			}
                        // Prevent this applet from doing further LiveConnect operations
                        LiveConnectSupport.appletStopped(key.intValue());
                        break;
                    }
                }
            }
            DeployPerfUtil.put("PluginMain - handleMessageStopApplet() - 3 - disconnectedApplets - POST");
        }
        StopAppletAckMessage reply = new StopAppletAckMessage(stopMessage.getConversation(),
                                                              stopMessage.getAppletID());
        pipe.send(reply);

        DeployPerfUtil.put(t0, "PluginMain - handleMessageStopApplet() - END");
    }

    private void handleMessageGetApplet(final GetAppletMessage getMsg) throws IOException
    {
        if (DEBUG && VERBOSE) {
            System.out.println("PluginMain: processing GetAppletMessage");
        }
        Plugin2Manager manager = getActiveOrDisconnectedApplet(getMsg.getAppletID());

        if (manager != null) {
            manager.waitUntilAppletStartDone();
        }

        // If the manager exists, but the applet is null, do not call 
        // getAppletStatus() - this can happen after dragging out applet with
        // custom progress using liveconnect - and will create error state on 
        // the call to JSObject.getWindow(null)
        if (manager == null || manager.getApplet() == null) {
            pipe.send(new JavaReplyMessage(getMsg.getConversation(),
                                           getMsg.getResultID(),
                                           null,
                                           false,
                                           "Applet ID " + getMsg.getAppletID() + " is not currently running"));
        } else {
            Applet2Status status = manager.getAppletStatus();

            if (status != null) {
                if (status.getApplet() != null) {
                    pipe.send(new JavaReplyMessage(getMsg.getConversation(),
                                                   getMsg.getResultID(),
                                                   LiveConnectSupport.exportObject(status.getApplet(),
                                                                                   getMsg.getAppletID(),
                                                                                   false,
                                                                                   true),
                                                   false,
                                                   null));
                } else {
                    String errorMessage = status.getErrorMessage();
                    if (errorMessage == null) {
                        errorMessage = "Unspecified error while fetching applet";
                    }
                    pipe.send(new JavaReplyMessage(getMsg.getConversation(),
                                                   getMsg.getResultID(),
                                                   null,
                                                   false,
                                                   errorMessage));
                }
            }
        }
    }

    private void handleMessageGetNameSpace(final GetNameSpaceMessage getMsg) throws IOException
    {
        if (DEBUG && VERBOSE) {
            System.out.println("PluginMain: processing GetNameSpaceMessage");
        }
        Plugin2Manager manager = getActiveOrDisconnectedApplet(getMsg.getAppletID());
        if (manager == null) {
            pipe.send(new JavaReplyMessage(getMsg.getConversation(),
                                           getMsg.getResultID(),
                                           null,
                                           false,
                                           "Applet ID " + getMsg.getAppletID() + " is not currently running"));
        } else {
            // It doesn't matter in which context we create the JavaNameSpace object;
            // we just have to make sure to export it via LiveConnectSupport into the
            // right applet's context since it's the subsequent operations upon it
            // that actually do work.
            pipe.send(new JavaReplyMessage(getMsg.getConversation(),
                                           getMsg.getResultID(),
                                           LiveConnectSupport.exportObject(new JavaNameSpace(getMsg.getNameSpace()),
                                                                           getMsg.getAppletID(),
                                                                           false,
                                                                           false),
                                           false,
                                           null));
        }
    }

    private static final int HEARTBEAT_TIMEOUT = 10000; // ms
    private class HeartbeatThread extends Thread {
        private volatile int suspendThreadCnt = 0;
        private boolean ignoreOneShotTO = false;

        public HeartbeatThread() {
            super("Java Plug-In Heartbeat Thread");
        }

        public void run() {
            boolean dead = false;
            long t0=0;
            Conversation c = pipe.beginConversation();
            try {
                while (!dead) {
                    synchronized (this) {
                        while(suspendThreadCnt>0) {
                            try {
                                if (DEBUG && VERBOSE) {
                                    t0 = SystemUtils.microTime();
                                    System.out.println("JVM heartbeat .. suspended, ts: "+t0);
                                }
                                this.wait();
                            } catch (InterruptedException ie) {
                            } finally {
                                if (DEBUG && VERBOSE) {
                                    t0 = SystemUtils.microTime();
                                    System.out.println("JVM heartbeat .. resumed, ts: "+t0);
                                }
                            }
                            ignoreOneShotTO = false;
                        }
                    }
                    pipe.send(new HeartbeatMessage(c));
                    t0 = SystemUtils.microTime();
                    Message m = pipe.receive(HEARTBEAT_TIMEOUT, c);
                    if (suspendThreadCnt==0) {
                        if (m == null && !ignoreOneShotTO) {
                            dead = true;
                            long t1 = SystemUtils.microTime();
                            System.out.println("JVM heartbeat .. dead, send ts: "+t0+", now ts: "+t1+", dT "+(t1-t0));
                        } else {
                            try {
                                Thread.sleep(HEARTBEAT_TIMEOUT);
                            } catch (InterruptedException e) {}
                        }
                    }
                    ignoreOneShotTO = false;
                }
            } catch (Exception e) {
                long t1 = SystemUtils.microTime();
                System.out.println("JVM heartbeat .. Exception, send ts: "+t0+", now ts: "+t1+", dT "+(t1-t0));
                if (DEBUG) {
                    e.printStackTrace();
                }
            } finally {
                pipe.endConversation(c);
            }
            shouldShutdown = true;
            if (disconnectedApplets.isEmpty()) {
                // Destroy cached legacy lifecycle applets
                instanceCache.clear();
                if (DEBUG) {
                    System.out.println("JVM instance exiting due to no heartbeat reply");
                    if (VERBOSE) {
                        try {
                            Thread.sleep(10000);
                        } catch (InterruptedException e) {
                        }
                    }
                }
                exit(0);
            }
        }

        public synchronized void suspendHeartbeat() {
            suspendThreadCnt++;
            ignoreOneShotTO=true;
            if (DEBUG && VERBOSE) {
                long t0 = SystemUtils.microTime();
                System.out.println("JVM heartbeat.suspend-counter incr: "+suspendThreadCnt+", ts: "+t0);
            }
        }

        public synchronized void resumeHeartbeat() {
            if(suspendThreadCnt>0) {
                suspendThreadCnt--;
                if (DEBUG && VERBOSE) {
                    long t0 = SystemUtils.microTime();
                    System.out.println("JVM heartbeat.suspend-counter decr: "+suspendThreadCnt+", ts: "+t0);
                }
            }
            if(suspendThreadCnt==0) {
                this.notifyAll();
            }
        }

    }


    private static final long IDLE_TIMEOUT = 60000; // 1 minute
    private class AutoShutdownTask extends TimerTask {
        public void run() {
            if (disconnectedApplets.isEmpty() && applets.isEmpty() && instanceCache.isEmpty()) {
                if (DEBUG) {
                    System.out.println("JVM instance exiting due to no applets running");
                }
                exit(0);
            }
        }
    }
    
    //----------------------------------------------------------------------
    // Implementation of ModalityInterface
    //

    // We need this to cope better with the situation where the class
    // loader cache is enabled and modal dialogs are in use. The main
    // problem is that Plugin2Manager.getCurrentManager() returns
    // imprecise answers in this case; we can't figure out which of
    // the multiple applets loaded by that class loader actually
    // pushed the dialog, so we also can't figure out precisely which
    // plugin instance on the browser side to notify of the modal
    // dialog's push. This could lead to lots of problems including
    // potentially blocking the wrong browser window if multiple
    // windows are open and all looking at the same applet, and one
    // applet pops up a modal dialog. It appears so far there is
    // really no good way to solve this problem aside from not using
    // class loader caching. For this among other reasons this is the
    // model we are advocating going forward. We need to rethink the
    // entire applet programming model.
    private static class ModalityLevel {
        private int level;
        private int appletID;
        private Plugin2Manager manager;

        public ModalityLevel(int appletID, Plugin2Manager manager) {
            this.appletID = appletID;
            this.manager = manager;
        }

        public int getAppletID() {
            return appletID;
        }

        public Plugin2Manager getManager() {
            return manager;
        }

        public synchronized void push() {
            ++level;
        }

        public synchronized int pop() {
            return --level;
        }

    }

    // Map from Dialogs to a bunch of information, including which
    // applet popped it up and what the modality level is
    private Map/*<Dialog, ModalityLevel>*/ modalityMap = new HashMap();

    // Map from applet IDs to lists of modal Dialogs these applets have popped up.
    // First Dialog in the list is the most recent Dialog raised.
    private Map/*<Integer, List<Dialog>>*/ appletToDialogMap = new HashMap();

    private boolean modalDialogHasPopped = false;

    // Indicates the given applet has raised the given dialog
    private synchronized void pushDialogForApplet(Integer appletID,
                                                  Plugin2Manager manager,
                                                  Dialog dialog) {
        ModalityLevel level = (ModalityLevel) modalityMap.get(dialog);
        if (level == null) {
            level = new ModalityLevel(appletID.intValue(), manager);
            modalityMap.put(dialog, level);
	    manager.increaseModalityLevel();
        }
        level.push();
    }

    // Returns the most recently raised, visible dialog for the given applet,
    // or null if no such dialog was available
    private synchronized Dialog getModalDialogForApplet(Integer appletID) {
        Plugin2Manager manager = (Plugin2Manager) applets.get(appletID);
        if (manager == null) {
            // Probably shouldn't happen
            return null;
        }
        for (Iterator iter = modalityMap.keySet().iterator(); iter.hasNext(); ) {
            Dialog d = (Dialog) iter.next();
            ModalityLevel level = (ModalityLevel) modalityMap.get(d);
            if (manager.isInSameAppContext(level.getManager())) {
                return d;
            }
        }
        return null;
    }

    // Reduces the modality level for the given Dialog
    private synchronized ModalityLevel popDialog(Dialog d) {
        ModalityLevel level = (ModalityLevel) modalityMap.get(d);
        if (level == null)
            return null;
        if (level.pop() == 0) {
            modalityMap.remove(d);
	    modalDialogHasPopped = true;
	    level.getManager().decreaseModalityLevel();
        }
        return level;
    }

    private boolean skipManagerForModalOperation(Plugin2Manager manager) {
        // If the applet has been dragged out of the browser (even if
        // it hasn't been fully separated from the browser -- i.e.,
        // we're still on the web page containing the applet), it does
        // not seem to be a good idea to block the web browser with
        // modal dialogs.
        return (manager.isDisconnected() ||
                disconnectedApplets.contains(manager));
    }

    public void modalityPushed(Dialog source) {
        Plugin2Manager manager = Plugin2Manager.getCurrentManager();
        if (manager == null) {
            manager = ModalityHelper.getManagerShowingSystemDialog();
            if (manager == null) {
                // Probably shouldn't happen
                return;
            }
        }

        if (skipManagerForModalOperation(manager)) {
            // Do not try to block the browser
            return;
        }

        Integer appletID = manager.getAppletID();
        if (appletID == null) {
            // Probably shouldn't happen
            return;
        }

        pushDialogForApplet(appletID, manager, source);

        if (DEBUG) {
            System.out.println("modalityPushed for applet ID " + appletID + " with dialog " + source);
        }
        try {
            pipe.send(new ModalityChangeMessage(null, appletID.intValue(), true));
        } catch (IOException e) {
        }
    }

    public void modalityPopped(Dialog source) {
        ModalityLevel level = popDialog(source);
        if (level == null) {
            // Probably shouldn't happen
            return;
        }
        int appletID = level.getAppletID();
        if (DEBUG) {
            System.out.println("modalityPopped for applet ID " + appletID);
        }
        try {
            pipe.send(new ModalityChangeMessage(null, appletID, false));
        } catch (IOException e) {
        }
    }        

    private void sendConservativeModalPush() {
        if (sendConservativeModalNotifications) {
            Plugin2Manager manager = Plugin2Manager.getCurrentManager();
            if (manager != null) {
                Integer appletID = manager.getAppletID();
                try {
                    pipe.send(new ModalityChangeMessage(null, appletID.intValue(), true));
                } catch (IOException e) {
                }
            }
        }
    }

    private void sendConservativeModalPop() {
        if (sendConservativeModalNotifications) {
            Plugin2Manager manager = Plugin2Manager.getCurrentManager();
            if (manager != null) {
                Integer appletID = manager.getAppletID();
                try {
                    pipe.send(new ModalityChangeMessage(null, appletID.intValue(), false));
                } catch (IOException e) {
                }
            }
        }
    }

    // Dialog hook to block the browser while system dialogs are active
    private DialogHook getDialogHook() {
        return new DialogHook() {
                public Component beforeDialog(Component owner) {
                    Plugin2Manager manager = Plugin2Manager.getCurrentManager();
                    if (manager != null && !skipManagerForModalOperation(manager)) {
                        sendConservativeModalPush();
                        ModalityHelper.pushManagerShowingSystemDialog();
                        return manager.getAppletParentContainer();
                    }
                    return null;
                }

                public void afterDialog() {
                    ModalityHelper.popManagerShowingSystemDialog();
                    sendConservativeModalPop();
                }

                public boolean ignoreOwnerVisibility() {
                    // This improves the placement of dialogs
                    return true;
                }
            };
    }

    //----------------------------------------------------------------------
    // Entry point
    //

    public static void main(String[] args) throws IOException {
        try {
            new PluginMain().run(args);
        } catch (IOException e) {
            e.printStackTrace();
            try {
                Thread.sleep(10000);
            } catch (InterruptedException e2) {
            }
            exit(1);
        } catch (RuntimeException e) {
            e.printStackTrace();
            try {
                Thread.sleep(10000);
            } catch (InterruptedException e2) {
            }
            exit(1);
        } catch (Error e) {
            e.printStackTrace();
            try {
                Thread.sleep(10000);
            } catch (InterruptedException e2) {
            }
            exit(1);
        }
    }

    //----------------------------------------------------------------------
    // Internals only below this point
    //

    private Applet2ClassLoaderCache getClassLoaderCacheForManager(Map params) {
        // The default behavior is to use the class loader cache
        // unless the applet specified the following parameter in its
        // applet tag:
        //   <PARAM name="classloader_cache" value="false"></PARAM>
        // or if the JVM had the system property
        // -Djavaplugin.classloader.cache.enabled=false specified on
        // its command line.
        String value = (String) params.get("classloader_cache");
        if (value != null && value.equalsIgnoreCase("false")) {
            return null;
        }
        if (!classLoaderCache.isInUse()) {
            return null;
        }
        return classLoaderCache;
    }
}
