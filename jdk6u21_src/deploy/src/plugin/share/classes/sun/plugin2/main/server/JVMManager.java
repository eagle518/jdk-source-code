/*
 * @(#)JVMManager.java	1.55 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.main.server;

import java.io.*;
import java.util.*;

import com.sun.deploy.config.*;
import com.sun.deploy.util.*;
import sun.plugin2.jvm.*;
import sun.plugin2.liveconnect.*;
import sun.plugin2.message.*;
import sun.plugin2.util.*;

/** A singleton class which manages connections to multiple JVM
    instances, and within each JVM instance, multiple applets. */

public class JVMManager {
    private static final boolean DEBUG   = (SystemUtil.getenv("JPI_PLUGIN2_DEBUG") != null);
    private static final boolean VERBOSE = (SystemUtil.getenv("JPI_PLUGIN2_VERBOSE") != null);

    private static final String JPI_VM_OPTIONS = "_JPI_VM_OPTIONS";
    private static final String JAVA_EXT_DIRS = "-Djava.ext.dirs=";
    private static final String TRUSTED_DIR = File.separator+"lib"+File.separator+"trusted";
    private static final String JPI_USER_PROFILE = "javaplugin.user.profile";

    private static JVMManager soleInstance = new JVMManager();

    // The number of times we retry launching an applet if race
    // conditions such as simultaneous JVM shutdown occur
    private static final int RETRY_COUNT = 2;

    // An indication of the browser type -- this unfortunately needs to be communicated to the target JVM.
    // See sun.plugin2.util.BrowserType and sun.plugin2.main.client.ServiceDelegate.
    private static int browserType;

    // The overridden USER_JPI_PROFILE, if specified
    private String userJPIProfile;

    private JVMManager() {
        // Pick up the value of the USER_JPI_PROFILE environment
        // variable before touching the Config class to be able to
        // override from where we pick up deployment.properties and
        // therefore the list of available JREs
        userJPIProfile = SystemUtil.getenv("USER_JPI_PROFILE");
        if (userJPIProfile != null) {
            System.setProperty(JPI_USER_PROFILE, userJPIProfile);
        }
        processJREInfo();
    }

    /** Indicates to the JVMManager the kind of web browser in use;
        see {@link sun.plugin2.util.BrowserType BrowserType}. Should
        be called once during the first plugin instance's
        initialization. */
    public static void setBrowserType(int browserType) {
        JVMManager.browserType = browserType;
    }

    /** Indicates the kind of web browser in use; see {@link
        sun.plugin2.util.BrowserType BrowserType}. */
    public static int getBrowserType() {
        return browserType;
    }

    public static JVMManager getManager() {
        return soleInstance;
    }

    /** Indicates whether the JVM with the given ID has exited. */
    public boolean instanceExited(int jvmID) {
        return (getJVMInstance(jvmID) == null);
    }

    /** Indicates whether the applet with the given ID has exited. */
    public boolean appletExited(AppletID appletID) {
        for (Iterator iter = activeJVMs.values().iterator(); iter.hasNext(); ) {
            JVMInstance instance = (JVMInstance) iter.next();
            if (instance.appletRunning(appletID)) {
                return false;
            }
        }
        return true;
    }

    /** Starts an applet with the specified parameters, being hosted
        by the supplied plugin, being displayed in the given parent
        native window, and, if running on X11, whether to use the
        XEmbed protocol for the embedding. If the applet parameter
        "java_version" is specified, attempts to launch the applet in
        a JVM matching the specified version string, which is the same
        as that used in JNLP files. This is equivalent to calling
        {@link #startApplet(Map,long,String) startApplet} passing in
        the value of the "java_version" parameter extracted from the
        parameter map. The optional applet parameter "java_arguments",
        if present, is extracted from the parameter map and used to
        specify JVM arguments and system properties. <P>
    */
    public AppletID startApplet(Map/*<String,String>*/ parameters,
                                Plugin plugin,
                                long parentNativeWindow,
                                long parentConnection,
                                boolean useXEmbedOnX11Platforms) {
        return startApplet(parameters, plugin, parentNativeWindow,
                           parentConnection,
                           useXEmbedOnX11Platforms,
                           (String) parameters.get(ParameterNames.JAVA_VERSION));
    }


    /** Starts an applet with the specified parameters, being hosted
        by the specified plugin, displayed in the given parent native
        window (an HWND on Windows; an X Window on X11 platforms;
        still to be defined on Mac OS X and other platforms),
        optionally using the XEmbed protocol on X11 platforms, and
        attempting to launch it in a JVM matching the specified
        version string. The version string may be null, in which case
        the default JVM version is used (see associated comments in
        the code). The optional applet parameter "java_arguments", if
        present, is extracted from the parameter map and used to
        specify JVM arguments and system properties. <P>

        Returns an AppletID object if the applet started successfully,
        or null if for some reason it didn't (like multiple race
        conditions between JVM termination and requesting to start an
        applet).
    */
    public AppletID startApplet(Map/*<String,String>*/ parameters,
                                Plugin plugin,
                                long parentNativeWindow,
                                long parentConnection,
                                boolean useXEmbedOnX11Platforms,
                                String javaVersion) {
        return startAppletImpl(parameters, plugin,
                               parentNativeWindow,
                               parentConnection,
                               useXEmbedOnX11Platforms,
                               javaVersion, false, nextAppletID(), false);
   }
    

    /** Starts a "dummy applet" -- creates an AppContext, ClassLoader,
        and ThreadGroup, but no actual applet instance. This is a
        concession to Firefox to support the Packages.* and java.*
        JavaScript keywords. See {@link
        sun.plugin2.applet.Applet2Manager#setForDummyApplet}. */
    public AppletID startDummyApplet(Map/*<String,String>*/ parameters,
                                     Plugin plugin) {
        return startAppletImpl(parameters, plugin, 0, 0, false, 
                                (String) parameters.get(ParameterNames.JAVA_VERSION), true, nextAppletID(), false);
    }

    /** Relaunch an applet with the latest installed version using the pre-assigned appletID
     *  Note that null is being passed in as the javaVersion.
     */
    public AppletID relaunchApplet(Map/*<String,String>*/ parameters,
                                   Plugin plugin,
                                   long parentNativeWindow,
                                   long parentConnection,
                                   boolean useXEmbedOnX11Platforms,
                                   String javaVersion,
                                   int intAppletID, boolean newJREInstalled) {
        if(newJREInstalled) {
            // we need to refresh the properties from the file,
            // to ensure a new installed JRE will be picked up!
            Config.refreshProps();
            processJREInfo();
        }
        return startAppletImpl(parameters, plugin,
            parentNativeWindow, parentConnection,
            useXEmbedOnX11Platforms,
            javaVersion, false, intAppletID, true);
    }

    private AppletID startAppletImpl(Map/*<String,String>*/ parameters,
                                     Plugin plugin,
                                     long parentNativeWindow,
                                     long parentConnection,
                                     boolean useXEmbedOnX11Platforms,
                                     String javaVersion,
                                     boolean isForDummyApplet,
                                     int intAppletID, boolean isRelaunch) {
        long startTimeUserClick = SystemUtils.microTime();
        VersionString verstr = null;
        if (javaVersion != null) {
	    if (isRelaunch) {
		verstr = new VersionString(javaVersion);
	    } else {
		String ssvJavaVersion = 
		    getBestJREInfo(new VersionString(javaVersion)).getProduct();
		if (SecurityBaseline.satisfiesSecurityBaseline(ssvJavaVersion)) {
		    // The best jre for the requested java version satisfies the
		    // security baseline. Go ahead and use it.
		    verstr = new VersionString(javaVersion);
		} else {
		    // start the latest available jvm for the 1st launch
		    // and pass the requested javaVersion as a parameter to applet
		    parameters.put(ParameterNames.SSV_VERSION, ssvJavaVersion);
		}
	    }
        }
	    
        JVMParameters jvmParams = new JVMParameters();
        jvmParams.parseBootClassPath(JVMParameters.getPlugInDependentJars());

        // For every attached jvm, set java.class.path to a dummy value
        // This is to avoid vm setting the property to '.' if it is empty.
        // Plugin should not include current work directoy '.' in the application
        // classpath. All applet classes need come from codebase.        
        jvmParams.addInternalArgument("-Djava.class.path="+Config.getJREHome()
                                      +File.separator+"classes");
        jvmParams.setDefault(true);

	// passing "!isRelaunch" argument to the parse method so that it'll drop all
	// non-secure jvm arguments or properties listed in the java_arguments
	// parameter in the html page if the jvm isn't a relaunch one
	jvmParams.parse((String) parameters.get(ParameterNames.JAVA_ARGUMENTS), !isRelaunch);

        int retryCount = 0;

        // obtain the "separate_jvm" parameter if any
        boolean separateJVM = false;
        String tmp = (String) parameters.get(ParameterNames.SEPARATE_JVM);
        if (tmp != null) {
            separateJVM = Boolean.valueOf(tmp).booleanValue();
        }

        do {
            JVMInstance instance = getOrCreateBestJVMInstance(startTimeUserClick, verstr, jvmParams, separateJVM);

            // Note the inherent subtle race conditions here,
            // including the JVM abruptly terminating (or having
            // already exited)
            if (!instance.exited()) {
                if (instance.startApplet(parameters, plugin, parentNativeWindow, parentConnection, useXEmbedOnX11Platforms, intAppletID, 
                                         isForDummyApplet, isRelaunch)) {
                    AppletID appletID = new AppletID(intAppletID);
                    appletToJVMMap.put(appletID, instance);
                    synchronized (appletMessageQueue) {
                        if(null == appletMessageQueue.get(appletID)) {
                            appletMessageQueue.put(appletID, new ArrayList/*<AppletMessage>*/());
                        }
                    }
                    if (DEBUG) {
                        System.out.println("JVMManager: applet launch (ID " + appletID + ") succeeded");
                    }
                    return appletID;
                }
            }

            if (DEBUG) {
                if (instance.errorOccurred()) {
                    System.out.println("Error occurred during launch of JVM");
                }
            }
        } while (++retryCount < RETRY_COUNT);

        return null;
    }

    /** Sets the size of the applet with the given ID. */
    public void setAppletSize(AppletID id,
                              int width,
                              int height) {
        // Figure out which instance to ask
        JVMInstance instance = getJVMInstance(id);
        if (instance == null) {
            // FIXME: not sure whether we should consider this an
            // error or not. If a JVM instance terminates abruptly
            // this might be able to happen. Probably want to "fail
            // soft" instead of hard.
            return;
        }
        int[] size = { width, height };
        instance.setAppletSize(id.getID(), width, height);
    }

    /** Sends a message to stop the applet with the specified ID. This
        method returns immediately; use {@link
        receivedStopAcknowledgment receivedStopAcknowledgment} (with
        care) to determine whether the applet shutdown appeared to be
        successful. */
    public void sendStopApplet(AppletID id) {
        // Figure out which instance to ask
        JVMInstance instance = getJVMInstance(id);
        if (instance == null) {
            // FIXME: not sure whether we should consider this an
            // error or not. If a JVM instance terminates abruptly
            // this could happen. Probably want to "fail soft" instead
            // of hard.
            return;
        }
        instance.sendStopApplet(id.getID());
    }

    /** Indicates whether we received acknowledgment of the stop
        request for the given applet. */
    public boolean receivedStopAcknowledgment(AppletID id) {
        JVMInstance instance = getJVMInstance(id);
        if (instance == null) {
            return false;
        }
        return instance.receivedStopAcknowledgment(id.getID());
    }

    /** Recycles the given applet ID for later reuse, performing
        certain resource cleanup. */
    public void recycleAppletID(AppletID id) {
        JVMInstance instance = removeJVMInstance(id);
        if (instance != null) {
            instance.recycleAppletID(id.getID());
        }
        removeAppletMessageQueue(id);
    }

    /** Requests a handle to the given applet for the browser's JavaScript engine. */
    public void sendGetApplet(AppletID appletID, int resultID) throws IOException {
        // Figure out which instance to ask
        JVMInstance instance = getJVMInstance(appletID);
        if (instance == null) {
            // NOTE: this might be able to happen if a JVM instance
            // terminates abruptly, and possibly during other termination
            // sequences. In this case we want to indicate to the caller
            // that the message send did not succeed so that the browser
            // main thread doesn't wait indefinitely for a reply.
            throw new IOException("No active JVM instance for applet ID " + appletID);
        }
        instance.sendGetApplet(appletID.getID(), resultID);
    }

    /** Requests a handle to the given portion of the Java name space
        for the browser's JavaScript engine. */
    public void sendGetNameSpace(AppletID appletID, String nameSpace, int resultID) throws IOException {
        // Figure out which instance to ask
        JVMInstance instance = getJVMInstance(appletID);
        if (instance == null) {
            // NOTE: this might be able to happen if a JVM instance
            // terminates abruptly, and possibly during other termination
            // sequences. In this case we want to indicate to the caller
            // that the message send did not succeed so that the browser
            // main thread doesn't wait indefinitely for a reply.
            throw new IOException("No active JVM instance for applet ID " + appletID);
        }
        instance.sendGetNameSpace(appletID.getID(), nameSpace, resultID);
    }

    /** Performs an operation, initiated by the browser's JavaScript engine, on a remote Java object. */
    public void sendRemoteJavaObjectOp(Conversation conversation,
                                       RemoteJavaObject object,
                                       String memberName,
                                       int operationKind,
                                       Object[] args,
                                       int resultID) throws IOException {
        JVMInstance instance = getJVMInstance(new AppletID(object.getAppletID()));
        if (instance == null) {
            // NOTE: this might be able to happen if a JVM instance
            // terminates abruptly, and possibly during other termination
            // sequences. In this case we want to indicate to the caller
            // that the message send did not succeed so that the browser
            // main thread doesn't wait indefinitely for a reply.
            throw new IOException("No active JVM instance for applet ID " + object.getAppletID() +
                                  ", JVM ID " + object.getJVMID());
        }
        instance.sendRemoteJavaObjectOp(conversation,
                                        object,
                                        memberName,
                                        operationKind,
                                        args,
                                        resultID);
    }

    /** Releases a Java object in an attached JVM instance. */
    public void releaseRemoteJavaObject(RemoteJavaObject object) {
        // We might get these notifications after the applet has disappeared
        JVMInstance instance = getJVMInstance(object.getJVMID());
        if (instance == null) {
            // NOTE: this can happen if a JVM instance terminates
            // abruptly, and possibly during other termination
            // sequences. Want to "fail soft" instead of hard.
            return;
        }
        instance.releaseRemoteJavaObject(object.getObjectID());
    }

    /** Synthesizes an activation of the window containing the given applet. */
    public void synthesizeWindowActivation(AppletID appletID, boolean active) {
        JVMInstance instance = getJVMInstance(appletID);
        if (instance == null) {
            // NOTE: this can happen if a JVM instance terminates
            // abruptly, and possibly during other termination
            // sequences. Want to "fail soft" instead of hard.
            return;
        }
        instance.synthesizeWindowActivation(appletID.getID(), active);
    }

    /** Print the applet with the given ID. */
    public boolean printApplet(AppletID id, long hdc, int x, int y, int width, int height) {
        // Figure out which instance to ask
        JVMInstance instance = getJVMInstance(id);
        if (instance == null) {
            return false;
        }
        return instance.printApplet(id.getID(), hdc, x, y, width, height);
    }


    /** Check if there's a newer version of jvm available. */
    public boolean isMoreRecentJVMAvailable(JREInfo jreInfo) {
	for (Iterator iter = javaPlatformList.iterator(); iter.hasNext(); ) {
	    JREInfo cur = (JREInfo) iter.next();
	    if (DEBUG) {
		System.out.println("isMoreRecentJVMAvailable considering " + cur.getProductVersion() + " JVM for relaunch");
	    }
	    if (cur.getProductVersion().isGreaterThan(jreInfo.getProductVersion())) {
		if (DEBUG) {
		    System.out.println("  isMoreRecentJVMAvailable (chosen)");
		}
		return true;
	    } 
	}
	return false;
    }

    //----------------------------------------------------------------------
    // Internals only below this point
    //

    /** This is the set of currently active JVMs attached to this one
        (i.e., attached to the browser, executing applets). We give
        each JVMInstance an integer ID to be able to more easily
        identify Java objects it exposes back to the web browser's
        JavaScript engine. */
    private Map/*<Integer, JVMInstance>*/ activeJVMs = new HashMap/*<Integer, JVMInstance>*/();

    // The current JVM ID we're on
    private int curJVMID;

    // A list of the available Java versions in the form of
    // VersionIDs, sorted in reverse order so the most recent version
    // is at the beginning of the list.
    private List/*<JREInfo>*/ javaPlatformList =
        new ArrayList/*<JREInfo>*/();

    // A map from the JREInfo objects to the JVMParameters they
    // represent
    private Map/*<JREInfo, JVMParameters>*/ javaParamMap =
        new IdentityHashMap/*<JREInfo, JVMParameters>*/();

    // The current applet ID we're on
    private int curAppletID;

    // A map indicating which applets are running in which JVM instances
    private Map/*<AppletID, JVMInstance>*/ appletToJVMMap =
        new HashMap/*<AppletID, JVMInstance>*/();

    // Maps an AppletMessage queue to the appletID, 
    // for all messages waiting to be sent, i.e. until StartAppletAck is received
    private Map/*<AppletID, List<AppletMessage>>*/ appletMessageQueue =
        new HashMap/*<AppletID, List<AppletMessage>*/();

    // if an AppletMessage queue is mapped (from startImpl until StartAppletResponse - true),
    // queue the message and return true, else false 
    protected boolean spoolAppletMessage(AppletMessage amsg) {
        synchronized (appletMessageQueue) {
            List/*Message*/ messageQueue = (List) appletMessageQueue.get(new AppletID(amsg.getAppletID()));
            if(messageQueue!=null) {
                if (DEBUG && VERBOSE) {
                    System.out.println("Spool AppletMessage: "+amsg);
                }
                messageQueue.add(amsg);

                // Don't send the message over the wire yet ..
                return true;
            }
        }
        return false;
    }

    // drain all queued AppletMessages and remove the mapping
    protected void drainAppletMessages(AppletID id) {
        JVMInstance jvmInst = getJVMInstance(id);
        if(jvmInst==null) {
            // This might happen due to race conditions in applet teardown
            if (DEBUG) {
                System.out.println("JVMManager.drainAppletMessages: no JVM instance for applet ID " + id);
                return;
            }
        }
        synchronized (appletMessageQueue) {
            List/*Message*/ messageQueue = (List) appletMessageQueue.remove(id);
            if(messageQueue!=null) {
                // hold the lock to appletMessageQueue, so all messages are send in order
                for (Iterator iter = messageQueue.iterator(); iter.hasNext(); ) {
                    try {
                        AppletMessage amsg = (AppletMessage) iter.next();
                        if (DEBUG && VERBOSE) {
                            System.out.println("Drain AppletMessage: "+amsg);
                        }
                        jvmInst.sendMessageDirect(amsg);
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            }
        }
    }

    private void removeAppletMessageQueue(AppletID id) {
        synchronized (appletMessageQueue) {
            appletMessageQueue.remove(id);
        }
    }

    protected synchronized int getJVMIDForApplet(AppletID id) {
        JVMInstance i = (JVMInstance) appletToJVMMap.get(id);
        return (null!=i)?i.getID() : -1;
    }

    private synchronized JVMInstance getJVMInstance(AppletID id) {
        return (JVMInstance) appletToJVMMap.get(id);
    }

    private synchronized JVMInstance getJVMInstance(int jvmID) {
        return (JVMInstance) activeJVMs.get(new Integer(jvmID));
    }

    private synchronized JVMInstance removeJVMInstance(AppletID id) {
        return (JVMInstance) appletToJVMMap.remove(id);
    }

    // Get the next available JVM ID
    private synchronized int nextJVMID() {
        while (true) {
            int id = ++curJVMID;
            if (activeJVMs.get(new Integer(id)) == null) {
                return id;
            }
        }
    }

    // We capture a bunch of state from the Config / JREInfo
    // classes at startup, because unfortunately they
    // don't provide enough useful functionality and we require a fair
    // bit of massaging of the data in order to figure out which JRE
    // to launch a given applet with. This is factored out so that we
    // can potentially reload it later.
    private synchronized void processJREInfo() {
        javaPlatformList.clear();

        // deployment.properties is frequently out of date with
        // respect to the installed JRE instances and is therefore an
        // unreliable source of information. On Windows, the registry
        // is the definitive location of all of the installed JREs'
        // information. On Unix platforms, deployment.properties is
        // about the best we can do.

        if (DEBUG && VERBOSE) {
            System.out.println("JREInfos (1)");
            JREInfo.printJREs();
        }

        // Scan windows registry to get all installed JREs
        // And store new found JRE in the deployment.properties
        // This also cleanup non-existing JREs in the properties file
        Vector list = Config.getInstance().getInstalledJREList();
        if(list!=null) {
            Config.storeInstalledJREList(list);
        } else {
            Config.refreshIfNecessary(); // we may have done this already: JRE installed
        }
        if (DEBUG && VERBOSE) {
            System.out.println("JREInfos (2)");
            JREInfo.printJREs();
        }

        javaPlatformList.addAll(Arrays.asList(JREInfo.getAll()));
        filterJavaPlatformList(javaPlatformList);

        // Now sort the javaPlatformList in reverse order
        Collections.sort(javaPlatformList, new Comparator() {
                public int compare(Object o1, Object o2) {
                    VersionID v1 = ((JREInfo) o1).getProductVersion();
                    VersionID v2 = ((JREInfo) o2).getProductVersion();
                    // We want things going in descending order
                    return -v1.compareTo(v2);
                }

                public boolean equals(Object obj) {
                    return false;
                }
            });
        filterDisabledJREs(javaPlatformList);

        // Produce a JVMParameters for each one
        for (Iterator iter = javaPlatformList.iterator(); iter.hasNext(); ) {
            JREInfo info = (JREInfo) iter.next();
            JVMParameters params = new JVMParameters();
            // Only allow specification of secure JVM arguments via
            // deployment.properties, because otherwise the JVM
            // instance won't be allowed to execute unsigned code.
            // Developers can use the java_arguments applet parameter
            // to specify insecure JVM arguments to individual
            // applets.
            // Plugin supports use of _JPI_VM_OPTIONS env var to override
            // VM options in deployment.properties.
            // It is supported in plugin2 only for backward compatibility.
            
            params.parseTrustedOptions(getVmArgs(info));
            params.setDefault(true);
            javaParamMap.put(info, params);            
        }
    }

    // Find the best match for the specified JRE version. A null
    // VersionString passed in will return the information for the 
    // most recent JRE version.
    synchronized JREInfo getBestJREInfo(VersionString verstr) {
	boolean retry = false;
	
        if (verstr == null)
            return (JREInfo) javaPlatformList.get(0);

        // Note that we iterate down the platforms in descending
        // version order so that we match against the highest
        // available version

	while (true) {
	    if (retry) {
		// refresh the installed jre list to see if a new jre installed
		// ever since the plug-in is initialized
		Config.refreshProps();
		processJREInfo();
	    }
	    
	    for (Iterator iter = javaPlatformList.iterator(); iter.hasNext(); ) {
		JREInfo info = (JREInfo) iter.next();
		if (verstr.contains(info.getProductVersion())) {
		    return info;
		}
	    }
	    
	    // not found. If has not retried, refresh and try again
	    // otherwise, break to the next step
	    if (!retry) {
		retry = true;
	    } else {
		break;
	    } 
	}
	
        // A matching JRE can't be found. 
        // Another attempt to match with the latest JRE version 
        // in the same family.
        List versionIDs = verstr.getAllVersionIDs();
        for (Iterator iVersIDs = versionIDs.iterator(); iVersIDs.hasNext(); ) {
            VersionID id = (VersionID) iVersIDs.next();

            VersionID familyVersionID = id.getFamilyVersionID();
            if (familyVersionID != null) {

                // Since the javaPlatformList has been sorted in descending order,
                // the first one matching the version prefix should be the highest
                // JRE version in the same family.
                for (Iterator iJREInfo = javaPlatformList.iterator(); iJREInfo.hasNext(); ) {
                    JREInfo info = (JREInfo) iJREInfo.next();
                    if (familyVersionID.match(info.getProductVersion())) {
                        return info;
                    }
                }
            }
        }

        // This implements the default behavior of returning the
        // highest available version if we can't match the user's
        // request.
        return (JREInfo) javaPlatformList.get(0);
    }

    // This wraps getBestJVMInstance and createJVMInstance below, and
    // handles the case where the requested version simply can't be
    // satisfied by the available JREs on the system
    private synchronized JVMInstance getOrCreateBestJVMInstance(long startTimeUserClick,
                                                                VersionString verstr,
                                                                JVMParameters parameters,
                                                                boolean separateJVM) {
        JVMInstance instance = null;
        if (!separateJVM) {
            instance = getBestJVMInstance(verstr, parameters);
            if (instance != null) {
                if (DEBUG) {
                    System.out.println("JVMManager reusing JVMInstance for product version " + instance.getProductVersion());
                }
                return instance;
            }
            // See whether we have any hope of finding a better version
            // match, or whether we should just reuse this JVM instance
            JREInfo info = getBestJREInfo(verstr);
            instance = getBestJVMInstance(new VersionString(info.getProductVersion()), parameters);
            if (instance != null) {
                if (DEBUG) {
                    System.out.println("JVMManager reusing JVMInstance for product version " + instance.getProductVersion());
                }
                return instance;
            }
        }
        // OK, go ahead and create and start a new JVM instance
        instance = createJVMInstance(startTimeUserClick, verstr, parameters, separateJVM);

        if (DEBUG) {
            System.out.println("JVMManager starting JVMInstance for product version " + instance.getProductVersion());
            List/*<String>*/ args = instance.getParameters().getCommandLineArguments(SystemUtil.isWindowsVista(), false);
            if (args.size() > 0) {
                System.out.println("  Command-line arguments: ");
                for (int i = 0; i < args.size(); i++) {
                    System.out.println("    Argument " + i + ": " + args.get(i));
                }
            }
        }
        try {
            instance.start();
            return instance;
        } catch (IOException e) {
            e.printStackTrace();
            throw new RuntimeException(e);
        } catch (RuntimeException e) {
            e.printStackTrace();
            throw e;
        } catch (Error e) {
            e.printStackTrace();
            throw e;
        }
    }

    // Based on the set of requested JVM parameters and Java version,
    // see whether there is a running JVM instance which satisfies
    // them. A null VersionID means to match the most recent JRE
    // version. Returns null if there is no suitable JVM instance
    // currently running.
    private synchronized JVMInstance getBestJVMInstance(VersionString verstr,
                                                        JVMParameters parameters) {
        // First clear out dead JVMs
        List/*<Integer>*/ deadJVMIDs = new ArrayList();
        for (Iterator iter = activeJVMs.values().iterator(); iter.hasNext(); ) {
            JVMInstance cur = (JVMInstance) iter.next();
            if (cur.exited()) {
                deadJVMIDs.add(new Integer(cur.getID()));
            }
        }
        for (Iterator iter = deadJVMIDs.iterator(); iter.hasNext(); ) {
            activeJVMs.remove(iter.next());
        }

        // FIXME: we're going to want to expand out what comes in to
        // match against the running JVM instances -- for example, to
        // basically contain the full contents of the JNLP file
        // instead of just the parameter map.
        //
        // One can envision arbitrarily complex heuristics for
        // deciding whether to reuse a running JVM instance for
        // launching a new applet. For example, if an applet decides
        // to load native code (either by being launched from a JNLP
        // file and specifying a nativelib resource in itself or an
        // extension, or discovering this by intercepting a call to
        // System.loadLibrary() initiated by the applet via its
        // Applet2ClassLoader), we might decide to stop launching new
        // applets in that JVM in order to better isolate the native
        // code. We might add a tag to the JNLP spec which allows a
        // given applet to specify that it would prefer to run in its
        // own JVM instance. We might add tracking of the consumed
        // heap and only continue to launch applets in the same JVM if
        // the free heap percentage was greater than, say, 50% or 60%.
        //
        // For now we keep things simple. We first scan the available
        // JVM instances to see if there is a match for the requested
        // version and parameters. We then scan the available platform
        // versions for a match. If we find a suitable running
        // instance, and the match in the available platform versions
        // didn't turn up a more recent version, then we use the
        // instance. Otherwise we start a fresh JVM.

        if (DEBUG) {
            System.out.println("Seeking suitable JRE version IDs: "+verstr);
            System.out.println(activeJVMs.values().size() + " active JVM(s)");
        }
        JVMInstance bestInstance = null;
        for (Iterator iter = activeJVMs.values().iterator(); iter.hasNext(); ) {
            JVMInstance cur = (JVMInstance) iter.next();
            if (DEBUG) {
                System.out.println("Considering " + cur.getProductVersion() + " JVM for reuse");
            }
            if (!cur.isTainted() && !cur.isExclusive() &&
                (verstr == null || verstr.contains(cur.getProductVersion())) &&
                (bestInstance == null || cur.getProductVersion().isGreaterThan(bestInstance.getProductVersion())) &&
                (cur.getParameters().satisfies(parameters))) {
                bestInstance = cur;
                if (DEBUG) {
                    System.out.println("  (chosen)");
                }
            } else {
                if (DEBUG) {
                    System.out.println("  (rejected)");
                }
            }
        }
        // See whether it's okay to return this instance or whether
        // the caller is going to have to start a new one

        // FIXME: want additional checks here, such as whether the JVM
        // is considered "tainted" because we failed to cleanly
        // shutdown an earlier applet, we loaded native code, or an
        // applet "disconnected" itself from the web browser (future
        // work)

        JREInfo bestInfo = null;
        for (Iterator iter = javaPlatformList.iterator(); iter.hasNext(); ) {
            JREInfo cur = (JREInfo) iter.next();
            if (DEBUG) {
                System.out.println("Considering " + cur.getProductVersion() + " JVM for launch");
            }
            if ((verstr == null || verstr.contains(cur.getProductVersion())) &&
                (bestInfo == null || cur.getProductVersion().isGreaterThan(bestInfo.getProductVersion()))) {
                bestInfo = cur;
                if (DEBUG) {
                    System.out.println("  (chosen)");
                }
            } else {
                if (DEBUG) {
                    System.out.println("  (rejected)");
                }
            }
        }
        // At this level we can only use the running JVM instance if
        // we wouldn't otherwise use a more recent one
        if (bestInstance != null) {
            if (bestInfo == null) {
                throw new InternalError("Should not find a running JVM instance but no matching JRE platform");
            }
            if (bestInstance.getProductVersion().isGreaterThanOrEqual(bestInfo.getProductVersion())) {
                if (DEBUG) {
                    System.out.println("Reusing JVM instance with product version " +
                                       bestInstance.getProductVersion() +
                                       "; best available product version " +
                                       bestInfo.getProductVersion());                                       
                }
                return bestInstance;
            } else {
                if (DEBUG) {
                    System.out.println("NOT reusing JVM instance with product version " +
                                       bestInstance.getProductVersion() +
                                       "; best available product version " +
                                       bestInfo.getProductVersion());                                       
                }
            }
        } else {
            if (DEBUG) {
                System.out.println("No suitable JVM instance to reuse");
            }
        }
        return null;
    }

    // Creates a new JVM instance matching the given platform version
    // and parameters, adding it to the set of running instances
    // before returning it. This is factored out primarily for testing
    // purposes.
    private JVMInstance createJVMInstance(long startTimeUserClick,
                                          VersionString verstr,
                                          JVMParameters parameters,
                                          boolean separateJVM) 
    {
        // Find the best available platform
        JREInfo info = getBestJREInfo(verstr);
        // Set up the parameters. We start from the base parameters
        // specified in deployment.properties and then add in those
        // specified by the applet (or, hopefully in the near future,
        // JNLP file).
        JVMParameters params = new JVMParameters();
        if (DEBUG && VERBOSE) {
            System.out.println("    JVMManager.createJVMInstance passing along JVM parameters from deployment.properties");
        }
        params.addArguments((JVMParameters) javaParamMap.get(info));
        if (DEBUG && VERBOSE) {
            System.out.println("    JVMManager.createJVMInstance passing along JVM parameters from this applet instance");
        }
        params.addArguments(parameters);
        addJavaExtDirsOption(params);
        addXToolkitOption(params, info.getProductVersion());
        addUIElementOption(params);
        params.addInternalArgument("-Dsun.awt.warmup=true");
        if (userJPIProfile != null) {
            // Pass along the USER_JPI_PROFILE setting as the
            // javaplugin.user.profile system property so the attached
            // JVM picks up deployment.properties from the same place
            // we do
            params.addInternalArgument("-D" + JPI_USER_PROFILE + "=" + userJPIProfile);
        }
        // Now we're ready to create the JVMInstance
        if (DEBUG) {
            System.out.println("JVMManager creating JVMInstance for product version " + info.getProductVersion());
        }
        int jvmID = nextJVMID();
        JVMInstance instance = new JVMInstance(startTimeUserClick, jvmID, info, params, separateJVM);
        // Add it to our list
        synchronized(this) {
            activeJVMs.put(new Integer(jvmID), instance);
        }
        return instance;
    }

    private synchronized int nextAppletID() {
        // FIXME: need to check existing JVM instances to make sure this applet ID isn't in use
        return ++curAppletID;
    }

    private static void filterJavaPlatformList(List/*<JREInfo>*/ infoList) {
        // Filters out Java versions that obviously won't work with
        // the new plug-in.
        // On all platforms, this includes everything before 1.4.
        // On Unix platforms, this includes everything before 1.5,
        // since the XToolkit, which require, was only introduced in
        // 1.5.
        // Also filter out non matching osName and osArch
        VersionID oneFourOrLater = new VersionID("1.4+");
        VersionID oneFiveOrLater = new VersionID("1.5+");
        boolean isUnix = (SystemUtil.getOSType() == SystemUtil.UNIX);
        for (Iterator iter = infoList.iterator(); iter.hasNext(); ) {
            JREInfo info = (JREInfo) iter.next();
            if (!oneFourOrLater.match(info.getProductVersion()) ||
                (isUnix && !oneFiveOrLater.match(info.getProductVersion()))) {
                iter.remove();
		continue;
            }
            if(!info.isOsInfoMatch(Config.getOSName(), Config.getOSArch())) {
                iter.remove();
		continue;
            }
	    // if the java executable in the JREInfo doesn't exist,
	    // remove it from the list
	    File javaExecutable = new File(info.getPath());
	    if (!javaExecutable.exists()) {
		iter.remove();
	    }
        }
    }

    private static void filterDisabledJREs(List/*<JREInfo>*/ infoList) {
        // Go through the list and take out JREs that aren't enabled,
        // but if we would wind up taking out all of the JREs, then at
        // least leave the one we're currently running on because we
        // need at least one available
        String javaHome = SystemUtil.getJavaHome();
        JREInfo curInfo = null;
        for (Iterator iter = infoList.iterator(); iter.hasNext(); ) {
            JREInfo info = (JREInfo) iter.next();
            if (info.getJREPath().startsWith(javaHome)) {
                curInfo = info;
            }
            if (!info.isEnabled()) {
                iter.remove();
            }
        }
        if (infoList.isEmpty() && curInfo != null) {
            infoList.add(curInfo);
        }
    }

    private static void addXToolkitOption(JVMParameters parameters,
                                          VersionID version) {
        // The new plug-in requires the use of the AWT XToolkit, which
        // was introduced in JDK 5. On Linux the XToolkit was also the
        // default starting in JDK 5, so nothing needs to be done.
        // Unfortunately, on Solaris, the decision was made to make
        // the old Motif toolkit the default on JDK 5, so we have to
        // recognize whether we're launching a 5 family JRE on this OS
        // and manually add the command line argument to select the
        // XToolkit.
        String osName = System.getProperty("os.name").toLowerCase();
        if (osName.startsWith("sunos") &&
            new VersionID("1.5*").match(version)) {
            parameters.addInternalArgument("-Dawt.toolkit=sun.awt.X11.XToolkit");
        }
    }

    private static void addUIElementOption(JVMParameters parameters) {
        String osName = System.getProperty("os.name").toLowerCase();
        if (osName.startsWith("mac os x")) {
            parameters.addInternalArgument("-Dapple.awt.UIElement=true");
        }
    }

    /** Parses the _JPI_VM_OPTIONS environment variables that plugin supports. 
     *  This method is to be called only on the server side
     */
    private static String getVmArgs(JREInfo info) {
        // This function is added for RFE 4523267, if the user specifies
        // _JPI_VM_OPTIONS, this env will overide the jre param in the
        // control panel (deployment.properties)
        
        // Though java control limits size of the paremeter to 1024 chars (2048 in 
        // classic plugin), we set no limit in Plugin2
        
        String vmOptions = SystemUtil.getenv(JPI_VM_OPTIONS);
        if (vmOptions != null) {
            return vmOptions;
        } else {
            return info.getVmArgs();
        }
    } 

    /** Set java.ext.dirs property to the system wide repositories
     *  for each JVM instance.
     */
    private static String getJavaExtDirsProp(JVMParameters params) {
        if (params.containsPrefix(JAVA_EXT_DIRS)) {
            return null;
        }
        
        StringBuffer sb = new StringBuffer(JAVA_EXT_DIRS);
        sb.append(Config.getJREHome() + File.separator + "lib"
                  + File.separator + "ext");
        
        String defaultProp = sb.toString();
        
        boolean isMozilla = (getBrowserType() == BrowserType.MOZILLA);
        
        // Set Mozilla/Firefox jss dir
        if (isMozilla) {
            String jssPath = Config.getInstance().getBrowserHomePath() 
                + File.separator + "jss";
            
            File jssDir = new File(jssPath);
            if (jssDir.exists()){
                sb.append(File.pathSeparator);
                sb.append(jssPath);
            }
        }
        
        String trustedPath = Config.getSystemHome() + TRUSTED_DIR;
        File trustedDir = new File(trustedPath); 
        // Currently only set for windows platforms
        // It can be expanded to Unix platforms
        if (SystemUtil.getOSType() == SystemUtil.WINDOWS && trustedDir.exists()) {
            sb.append(File.pathSeparator);
            sb.append(trustedPath);
        } 

        String ret = sb.toString();
        
        if (ret.equals(defaultProp)) {
            // Do not set the property if no extra paths
            return null;
        }        
        
        return ret;
    }

    private static void addJavaExtDirsOption(JVMParameters parameters) {
        String javaExtDirsProp = getJavaExtDirsProp(parameters);
        if ( null != javaExtDirsProp) {
            parameters.addInternalArgument(javaExtDirsProp);
        }
    }
    
    //----------------------------------------------------------------------
    // Test harness for JVM selection algorithm
    //

    private void testPrintJVMs() {
        System.out.println("" + activeJVMs.size() + " active JVMs:");
        for (Iterator iter = activeJVMs.keySet().iterator(); iter.hasNext(); ) {
            Integer i = (Integer) iter.next();
            System.out.println("  JVM " + i + ": Version ID " + ((JVMInstance) activeJVMs.get(i)).getProductVersion());
        }
    }

    private void test() {
        for (int i = 0; i < javaPlatformList.size(); i++) {
            System.out.println("Available JRE " + i + ":");
            JREInfo info = (JREInfo) javaPlatformList.get(i);
            System.out.println("  Product: " + info.getProduct());
            System.out.println("  Version: " + info.getProductVersion());
            System.out.println("  Path: " + info.getPath());
        }
        // Test creation of JVM instances
        createJVMInstance(SystemUtils.microTime(), new VersionString("1.6*"), new JVMParameters(), false);
        System.out.println("After creation of 1.6* JVM:");
        testPrintJVMs();
        createJVMInstance(SystemUtils.microTime(), new VersionString("1.2 1.5+"), new JVMParameters(), false);
        System.out.println("After creation of 1.5+ JVM:");
        testPrintJVMs();
        createJVMInstance(SystemUtils.microTime(), new VersionString("1.2 1.6*"), new JVMParameters(), false);
        System.out.println("After creation of another 1.6* JVM:");
        testPrintJVMs();
    }


    /** Test harness */
    public static void main(String[] args) {
        JVMManager manager = JVMManager.getManager();
        manager.test();
    }
}
