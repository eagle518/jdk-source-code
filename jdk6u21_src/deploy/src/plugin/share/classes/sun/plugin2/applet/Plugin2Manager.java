/*
 * @(#)Plugin2Manager.java	1.75 10/05/21
 *
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.applet;

import java.applet.Applet;
import java.applet.AppletContext;
import java.applet.AppletStub;
import java.applet.AudioClip;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.EventQueue;
import java.awt.Font;
import java.awt.Frame;
import java.awt.Image;
import java.awt.KeyboardFocusManager;
import java.awt.MenuItem;
import java.awt.PopupMenu;
import java.awt.Window;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.lang.ref.SoftReference;
import java.lang.ref.WeakReference;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.net.MalformedURLException;
import java.net.SocketPermission;
import java.net.URL;
import java.security.AccessController;
import java.security.CodeSource;
import java.security.PrivilegedAction;
import java.security.PrivilegedActionException;
import java.security.PrivilegedExceptionAction;
import java.security.AccessControlContext;
import java.text.MessageFormat;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Enumeration;
import java.util.Iterator;
import java.util.List;
import java.util.HashMap;
import java.util.Map;
import java.util.HashSet;
import java.util.Set;
import java.util.StringTokenizer;

import sun.awt.AppContext;
import sun.awt.EmbeddedFrame;
import sun.awt.SunToolkit;

import sun.plugin2.util.ColorUtil;
import sun.plugin2.util.SystemUtil;
import sun.plugin2.util.ParameterNames;

import com.sun.javaws.jnl.JREDesc;
import com.sun.javaws.exceptions.JRESelectException;
import com.sun.javaws.exceptions.ExitException;

import com.sun.deploy.cache.Cache;
import com.sun.deploy.config.Config;
import com.sun.deploy.security.TrustDecider;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.ui.UIFactory;
import com.sun.deploy.util.DeployAWTUtil;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.util.URLUtil;
import com.sun.deploy.util.SystemUtils;
import com.sun.deploy.perf.DeployPerfUtil;
import sun.plugin.perf.PluginRollup;

// We need a notion of the command-line arguments that were used to
// start this JVM instance to decide whether we need to relaunch the
// applet (or, more precisely, whether to fire an event indicating the
// need to relaunch the applet)
import com.sun.deploy.util.JVMParameters;

// Should break the following dependencies
import sun.plugin.JavaRunTime;
import sun.plugin.util.ErrorDelegate;
import sun.plugin.util.GrayBoxPainter;

// Probably can't break the following dependency without completely
// replacing the JSObject implementation
import sun.plugin.javascript.JSContext;

/** The main class which manages the creation and lifecycle of an
    applet. A new Plugin2Manager instance should be created for each
    applet being viewed. */

public abstract class Plugin2Manager {
    protected static final boolean DEBUG = (SystemUtil.getenv("JPI_PLUGIN2_DEBUG") != null);
    protected static final boolean VERBOSE = (SystemUtil.getenv("JPI_PLUGIN2_VERBOSE") != null);


    // NOTE: there is an invariant that we MUST NOT perform ANY
    // operations where we block waiting for any EDT in the thread
    // calling the public methods of this class. Any such operations
    // must be done with invokeLater(), optionally with a timeout
    // waiting for the operation to complete.

    public static final String APPCONTEXT_APPLETCONTEXT_KEY  = "AppletContextKey";

    // We store ourselves in the AppContext so that the BrowserService
    // can delegate back through us to the execution context.
    public static final String APPCONTEXT_APPLET2MANAGER_LIST_KEY = "Plugin2ManagerListKey";

    // This is a workaround for a bootstrapping issue. The
    // construction of the URL associated with the codebase may
    // require one of the deployment protocol handlers to be loaded
    // and the Applet2BrowserService to be queried before we've fully
    // created the AppContext. We use a thread-local variable as a
    // fallback for this case.
    private static final ThreadLocal currentThreadManager = new ThreadLocal();

    // This is a workaround for the getCurrentManager() issue when classloader cacheing
    // is enabled.
    private static final InheritableThreadLocal currentManagerThreadLocal
            = new InheritableThreadLocal();

    private static long jvmLaunchTime  = -1;
    private static long jvmLaunchCosts = -1;

    // to track if there's a modal dialog has been 
    // popped up from the applet
    private static int modalityLevel = 0;

    // the following 2 methods are being used by PluginMain
    // to indicate the modalityLevel (number of modal dialogs)
    // w.r.t. the applet
    public void increaseModalityLevel() {
	modalityLevel++;
    }

    public void decreaseModalityLevel() {
	modalityLevel--;
    }

    // the following method is being used by DragHelper to check
    // whether there's a modal dialog being displayed
    public int getModalityLevel() {
	return modalityLevel;
    }

    public static void setJVMLaunchTime(long t0, long costs) {
        if(jvmLaunchTime==-1) {
            jvmLaunchTime = t0;
            jvmLaunchCosts= costs;
        }
    }
    public static long getJVMLaunchCosts() {
        return jvmLaunchCosts;
    }
    public static long getJVMLaunchTime() {
        return jvmLaunchTime;
    }

    /** Fetches the Plugin2Manager instance associated with the
        current AppContext. <P>

        NOTE: (FIXME) this method may return imprecise answers, in
        particular in the case where the old "class loader caching"
        mechanism is being used and we have multiple applets all
        running in the same AppContext. This definitely isn't the
        execution model we want to support long term going forward; it
        causes all sorts of encapsulation and termination problems. In
        the meantime, callers of this method should implicitly assume
        that it might return a "random" Plugin2Manager if multiple
        applets are running in the same AppContext. If more precise
        answers are required, such as for Java-to-JavaScript calls,
        different mechanisms should be used.
    */
    public static Plugin2Manager getCurrentManager() {
        Plugin2Manager manager = getFromThreadLocal();
        if (manager == null) {
            manager = getFromAppContext();
        }
        
        if (manager != null)
            return manager;
        // Try the fallback path for bootstrapping
        return (Plugin2Manager) currentThreadManager.get();
    }

    public static void setCurrentManagerThreadLocal(Plugin2Manager manager) {
        currentManagerThreadLocal.set(manager);
    }

    /** Creates a new Plugin2Manager.
        */
    public Plugin2Manager(boolean relaunched) {
        _appletRelaunched=relaunched;
    }

    /**
     * Process all expensive initialisations to finalize the manager,
     * e.g. networking , etc ..
     *
     * The pending finalization is necessary to not block the main thread.
     *
     * The applet is complete at this point, e.g parameter, etc ..
     */
    public void initialize() throws Exception {
        // make sure these applet parameters are set all the time (even at no relaunch)
        setParameter(ParameterNames.APPLET_RELAUNCHED, String.valueOf(_appletRelaunched));
        setParameter(ParameterNames.JAVA_ARGUMENTS, JVMParameters.getRunningJVMParameters().getCommandLineArgumentsAsString(false));
    }

    /** This must be called before calling any of the other methods on
        this class. The intent is that it can be changed at run-time,
        with the caveat that proper implementations of some of the
        methods (like the fetching of the applet's parameters) must be
        available at various times, and that some of the methods might
        be expected to fail in certain situations (like Java ->
        JavaScript calls in some advanced scenarios). */
    public void setAppletExecutionContext(Applet2ExecutionContext context) {
        this.appletExecutionContext = context;
    }

    /** Fetches the Applet2ExecutionContext associated with this
        Plugin2Manager. */
    public Applet2ExecutionContext getAppletExecutionContext() {
        return appletExecutionContext;
    }

    /** Sets the "default" Applet2ExecutionContext. This is a
        workaround for the fact that certain services like proxy
        queries are made in the system AppContext, in which no applet
        is running. */
    public static void setDefaultAppletExecutionContext(Applet2ExecutionContext context) {
        defaultAppletExecutionContext = context;
    }

    /** Returns the "default" Applet2ExecutionContext. */
    public static Applet2ExecutionContext getDefaultAppletExecutionContext() {
        return defaultAppletExecutionContext;
    }

    /** Returns the current manager's Applet2ExecutionContext, or the
        default if no applet is running in this AppContext. */
    public static Applet2ExecutionContext getCurrentAppletExecutionContext() {
        Plugin2Manager manager = getCurrentManager();
        if (manager != null) {
            return manager.getAppletExecutionContext();
        }
        return defaultAppletExecutionContext;
    }

    /**
     * Gets an applet parameter.
     */
    public String getParameter(String name) {
        name = name.toLowerCase(java.util.Locale.ENGLISH);
        Map/*<String,String>*/ parms = getAppletParameters();
        synchronized (parms) {
            return trimWhitespace((String) parms.get(name));
        }
    }

    /**
     * Sets an applet parameter.
     */
    public void setParameter(String name, Object value) {
        name = name.toLowerCase(java.util.Locale.ENGLISH);
        Map/*<String,String>*/ parms = getAppletParameters();
        synchronized(parms) {
            parms.put(name, trimWhitespace(value.toString()));
        }
    }

    /**
     * Returns the document base in which this applet is running. This
     * needs to be public to properly create the AccessControlContexts
     * and ProtectionDomains associated with sandboxed applet
     * execution and LiveConnect calls.
     */
    public URL getDocumentBase() {
        String docBase = appletExecutionContext.getDocumentBase(this);
        if (docBase == null) {
            // FIXME: need to specify more clearly under what conditions this may return null
            return null;
        }
        try {
            return new URL(docBase);
        } catch (MalformedURLException e) {
            // FIXME: fail more gracefully
            throw new RuntimeException(e);
        }
    }

    /** Fetch the ClassLoader which will be used to load the applet.
        This may return null if an error condition such as a null
        codebase prevents the class loader object from being created.

        FIXME: consider delegating the implementation to the
        AppletExecutionContext object. */
    public Plugin2ClassLoader getAppletClassLoader() {
        synchronized (this) {
            if (loader == null) {
                // Not using class loader caching -- create a new loader
                loader = getOrCreatePlugin2ClassLoader();
            }
            
            return loader;
        }
    }

    protected void setupClassLoaderCodebaseRecursiveRead(Plugin2ClassLoader loader) {
        URL documentBase = getDocumentBase();

        if (documentBase == null ||
                documentBase.getProtocol().equalsIgnoreCase("file") == false ||
                URLUtil.isUNCFileURL(documentBase)) {
            // do not allow recursive codebase direcotry read if:
            // 1.  documentBase is null
            // 2.  documentBase is not file protocol url
            // 3.  documentBase url is UNC file url
            loader.disableRecursiveDirectoryRead();
        }
    }

    /** Fetch and/or create the ThreadGroup associated with this
        applet's execution. */
    public ThreadGroup getAppletThreadGroup() {
        synchronized(this) {
            if (appletThreadGroup == null) {
                appletThreadGroup = getOrCreateAppletThreadGroup();
            }

            return appletThreadGroup;
        }
    }

    /** Fetch and/or create the AppContext associated with this applet's
        execution. */
    public AppContext getAppletAppContext() {
        synchronized(this) {
            if (appletAppContext == null) {
                appletAppContext = getOrCreateAppletAppContext();

                // Immediately put ourselves into the AppContext
                registerInAppContext(appletAppContext);
            }

            return appletAppContext;
        }
    }

    public synchronized AppletContext getAppletContext() 
    {
        if (appletContext == null) {
            appletContext = new AppletContextImpl(this);
        }

        return appletContext;
    }

    /** Sets the parent Container the applet will be shown in. This is
        required before initiating the loading of the applet. Note
        that the Plugin2Manager assumes responsibility for the
        Container, including resizing it when needed. If the Container
        is actually a Window, the Plugin2Manager also assumes
        responsibility for disposing it when the applet is stopped via
        {@link #stop stop()}. */
    public synchronized void setAppletParentContainer(Container parent) {
        this.appletParentContainer = parent;
    }

    /** Returns the parent Container that was set for the applet, or
        null if none has been set yet. */
    public Container getAppletParentContainer() {
        return appletParentContainer;
    }

    /** Returns the Applet, if loaded and instantiated yet, or null if
        not. Note that querying this for a non-null value to see
        whether the applet is ready or not is not a good idea because
        of the case of a so-called "dummy applet". Instead call {@link
        #isStarted isStarted}. */
    public Applet getApplet() {
        return applet;
    }

    /** Indicates whether the applet has reached the "started"
        state. This returns correct answers even for so-called "dummy"
        applets (see {@link #setForDummyApplet
        setForDummyApplet}. Note that there are some inherent race
        conditions in querying this value. */
    public boolean isAppletStarted() {
        return appletIsActive;
    }

    /** In one synchronous operation, fetches the applet's state,
        registering the given Applet2Listener if the applet is both
        not yet created and is not yet in an error state (meaning that
        it is likely still in the process of loading). Returns null if
        the applet's state is not yet determined. */
    public synchronized Applet2Status getAppletStatus() {
        if (applet == null && !hasErrorOccurred()) {
            showAppletException(new Exception("InternalError: LiveConnect GetApplet issued before appletLoaded"));
            fireAppletErrorOccurred();
        }
        return new Applet2Status(getApplet(), hasErrorOccurred(), getErrorMessage(), getErrorException());
    }

    /** Extensibility mechanism for allowing third-party code (such as
        LiveConnect support) to easily start a worker thread with the
        AppContext, ThreadGroup, etc. properly set up. */
    public void startWorkerThread(String threadName, Runnable runnable) {
        final Thread worker = new Thread(getAppletThreadGroup(),
                                         runnable,
                                         threadName);
        final ClassLoader workerLoader = getAppletClassLoader();
        // set the context class loader for this thread
        AccessController.doPrivileged(new PrivilegedAction() {
                public Object run() {
                    worker.setContextClassLoader(workerLoader);
                    return null;
                }
            });
        worker.start();
    }

    /** Sets whether this Plugin2Manager is being created only to set
        up an AppContext, ClassLoader, and ThreadGroup, but no actual
        applet. This is a concession to Firefox where they create a
        dummy plugin instance in order to support the Packages.* and
        java.* keywords in the JavaScript engine; there is no
        associated applet in this case. Note that it is still
        necessary to call {@link #start start} and {@link #stop
        stop}. */
    public void setForDummyApplet(boolean isForDummyApplet) {
        this.isForDummyApplet = isForDummyApplet;
    }

    /** Indicates whether this Plugin2Manager is not really being
        created to execute an applet, but only to set up the
        AppContext, etc. See {@link #setForDummyApplet
        setForDummyApplet}. */
    public boolean isForDummyApplet() {
        return isForDummyApplet;
    }

    /** Indicates that the applet corresponding to this manager has
        been disconnected from the browser. This disqualifies this
        manager instance from certain queries in the face of class
        loader caching. */
    public void setDisconnected() {
        disconnected = true;
        // Take the opportunity to dispose of the applet's parent window
        disposeParentWindow(null, 0);
    }

    /** Returns whether the applet corresponding to this manager has
        been disconnected from the browser. */
    public boolean isDisconnected() {
        return disconnected;
    }

    /** Causes the (disconnected) applet that this Plugin2Manager is
        visualizing to have desktop shortcuts and/or start menu
        entries to be installed. This only works for JNLP-launched
        applets. */
    public void installShortcuts() {
    }

    /** Checks the "eager_install" parameter
     * @return true if "eager_install" parameter is set to true
     */
    public boolean isEagerInstall() {
	String param = getParameter(ParameterNames.EAGER_INSTALL);
	return Boolean.valueOf(param).booleanValue();
    }

    /** Obtains a key which uniquely identifies an applet.
     */
    public abstract String getAppletUniqueKey();

    //----------------------------------------------------------------------
    // Lifecycle interaction
    //

    /** Starts the applet, performing all necessary applet lifecycle
        management such as installing it into its parent container and
        calling init() and start(). */
    public void start() throws IllegalStateException {
        synchronized (this) {
            long t0 = DeployPerfUtil.put(0, "Plugin2Manager - start() - BEGIN");

            if (appletExecutionThread != null) {
                throw new IllegalStateException("Plugin2Manager already started");
            }

            if (appletParentContainer == null && !isForDummyApplet) {
                throw new IllegalStateException("Applet's parent container not set up");
            }

            //Check whether the exception dialogboxes are allowed
            // FIXME: break this dependency
            // FIXME: unclear whether this really works -- in the original
            // code it's probing the system properties, which is probably
            // not what was intended
            if (!fShowExceptionInitialized) {
                String showExceptionProp = SystemUtil.getSystemProperty(Config.SHOW_EXCEPTIONS_KEY);
                if ("true".equalsIgnoreCase(showExceptionProp)) {
                    fShowException = true;
                }
            }
        
            final Plugin2ClassLoader loader = getAppletClassLoader();
            ThreadGroup group = getAppletThreadGroup();
            String name = "thread applet-" + getCode() + "-" + nextSequenceNumber();
            appletExecutionThread = new Thread(group,
                                               new AppletExecutionRunnable(),
                                               name);
                                               
            // set the context class loader for this thread
            AccessController.doPrivileged(new PrivilegedAction() {
                    public Object run() {
                        appletExecutionThread.setContextClassLoader(loader);
                        return null;
                    }
                });

            appletExecutionThread.start();
            DeployPerfUtil.put(t0, "Plugin2Manager - start() - END");
        }
    }

    /** Stops the applet, performing all necessary lifecycle
        interaction such as calling stop() and destroy() and cleaning
        up the applet's AppContext. Between the time stop() and
        destroy() are called, will run the optional afterStopRunnable
        if supplied. Returns true to indicate that the applet stopped
        cooperatively, false if it didn't and may require more drastic
        cleanup measures(such as shutting down this JVM instance). */
    public boolean stop(Runnable afterStopRunnable) {
        return stop(afterStopRunnable, null);
    }

    /** Stops the applet, performing all necessary lifecycle
        interaction such as calling stop() and destroy(), cleaning
        up the applet's AppContext and calling Applet2StopListener.stopFailed()
        to notify stop failure.        Between the time stop() and
        destroy() are called, will run the optional afterStopRunnable
        if supplied. Returns true to indicate that the applet stopped
        cooperatively, false if it didn't and may require more drastic
        cleanup measures. Calls stopListener's 
        {@link sun.plugin2.applet.Applet2StopListener#stopFailed stopFailed()} when 
        stop is not completely successful. 
        * @param afterStopRunnable optional code to run after applet stop
        * @param stopListener listener to notify Plugin Main stop failures
        * @return boolean applet stopped successfully or not

        * @see sun.awt.AppContext#dispose
        * @see sun.plugin2.applet.Applet2StopListener#stopFailed
        * @see #stop(Runnable)
     */
    public boolean stop(Runnable afterStopRunnable, Applet2StopListener stopListener) {
        long stopTimeout = getAppletStopTimeout();
        long t0 = DeployPerfUtil.put(0, "Plugin2Manager - stop() - BEGIN");
        removeAllAppletListeners();
        if (hasErrorOccurred()) {
            // Assume that this means we "successfully" stopped this applet
            if (afterStopRunnable != null) {
                try {
                    afterStopRunnable.run();
                } catch (Throwable t) {
                    // FIXME: deal better with this
                    t.printStackTrace();
                }
            }
            
            unregisterFromAppContext(appletAppContext);
            
            // Clean up AppContext and ThreadGroup
            cleanupAppContext(System.currentTimeMillis(), stopTimeout, stopListener);

            DeployPerfUtil.put(t0, "Plugin2Manager - stop() - END(1)");
            return true;
        }

        // Determine whether the applet is a JApplet and, if so,
        // enable some additional cleanup to be done while disposing
        // the AppContext
        if (isSubclassOf(applet, "javax.swing.JApplet")) {
            AppContext ac = appletAppContext;
            if (ac != null) {
                ac.put(APPCONTEXT_JAPPLET_USED_KEY, Boolean.TRUE);
            }
        }

        // FIXME: figure out how to send notification to abort applet
        // loading if it's in progress; suspect this logic needs to be in
        // the Plugin2ClassLoader

        boolean ret = false;

        DeployPerfUtil.put("Plugin2Manager - stop() - stopLock - pre ");
        long startTime = System.currentTimeMillis();
        synchronized (stopLock) {
            shouldStop = true;
            stopLock.notifyAll();

            try {
                stopLock.wait(stopTimeout);
                ret = stopSuccessful;
            } catch (InterruptedException e) {
            }
        }
        long timeRemaining = stopTimeout - (System.currentTimeMillis() - startTime);

        DeployPerfUtil.put("Plugin2Manager - stop() - afterStopRunnable.run() - START");

        if (afterStopRunnable != null) {
            try {
                afterStopRunnable.run();
            } catch (Throwable t) {
                // FIXME: deal better with this
                t.printStackTrace();
            }
        }
        DeployPerfUtil.put("Plugin2Manager - stop() - afterStopRunnable.run() - END");

        // Regardless of whether the applet terminated cooperatively,
        // terminate things hard at this point
        disposeParentWindow(stopListener, timeRemaining);
        DeployPerfUtil.put("Plugin2Manager - stop() - AWT disposal - post");

        shutdownAppContext(appletAppContext, startTime, stopTimeout, stopListener, ret);

        DeployPerfUtil.put(t0, "Plugin2Manager - stop() - END(2)");
        return ret;
    }

    /** Returns whether an error occurred somewhere during the loading
        of the applet. */
    public boolean hasErrorOccurred() {
        return errorOccurred;
    }

    /** Returns a detail message about the error which occurred, or null
        if none was available. */
    public String getErrorMessage() {
        return errorMessage;
    }

    /** Returns a Throwable indicating the root cause of the error which
        occurred, or null if none was available. */
    public Throwable getErrorException() {
        return errorException;
    }

    /** Returns whether we have sent a response to the StartApplet message or not.
        This could be one of the following client responses:
            fireAppletSSVValidation() 
                  Applet2Listener.appletSSVValidation==false   no message
            fireAppletJRERelaunch
                  Applet2Listener.appletJRERelaunch         -> StartMessage
                                                               (will eventually result in StartAppletAckMessage being
                                                                sent by newly launched JVM instance)
            fireAppletLoaded()
                  Applet2Listener.appletLoaded              -> StartAppletAckMessage
            fireAppletErrorOccurred()
                  Applet2Listener.appletErrorOccurred       -> StartAppletAckMessage

                  
        We rely on the Applet2Listener impl. to actually send out the message
        over the wire; therefore we call 'setAppletStartResponseSent()'
        within the fireApplet* methods, after the actual method has been called
     */
    private boolean hasAppletStartResponseBeenSent() {
        return appletStartResponseSent;
    }

    /** An indication that we have sent a response to the StartApplet
        message: applet was loaded successfully, error occurred
        starting applet, or applet needs to be relaunched in a new
        JVM. */
    private void setAppletStartResponseSent() {
        appletStartResponseSent = true;
    }

    protected long getAppletStopTimeout() {
        long stopTimeout;
        try {
            stopTimeout = Integer.parseInt(getParameter(ParameterNames.APPLET_STOP_TIMEOUT));
            if (stopTimeout > Config.JAVAPI_STOP_TIMEOUT_MAX) {
                stopTimeout = Config.JAVAPI_STOP_TIMEOUT_MAX;
            }
        } catch (NumberFormatException nfe) {
            stopTimeout = STOP_TIMEOUT_CONFIG;
        }
        return stopTimeout;
    }
    
    protected void disposeParentWindow(final Applet2StopListener stopListener, long millisToWait) {
        Container parent = null;
        synchronized (this) {
            parent = getAppletParentContainer();
            setAppletParentContainer(null);
        }
        if (parent != null && (parent instanceof Window)) {
            // Manually dispose the parent window. In the case
            // where we are running in the browser, the parent
            // window is an EmbeddedFrame (on Windows, a
            // WEmbeddedFrame), and it causes significant problems
            // if we don't dispose the EmbeddedFrame before
            // destroying the containing window in the browser.
            // See 6592751.
            // FIXME: perhaps should make this configurable
            final Window parentWindow = (Window) parent;
            final Object disposeLock = new Object();
            // NOTE: in the context of the browser, this is called
            // from the main thread of the PluginMain class; we can
            // *not* do any work here using a blocking invokeAndWait
            // since that might never return
            synchronized (disposeLock) {
                DeployAWTUtil.invokeLater(parentWindow, new Runnable() {
                        public void run() {
                            try {
                                parentWindow.dispose();
                            } catch (Exception ex) {
                                //the parent window(EmbeddedFrame) is not disposed correctly
                                if (DEBUG) {
                                    ex.printStackTrace();
                                }
                                if (stopListener != null) {
                                    if (DEBUG) {
                                        System.out.println("Plugin2Manager calling stopFailed() because of exception during EmbeddedFrame.dispose()");
                                    }
                                    stopListener.stopFailed();
                                }
                            }
                            synchronized (disposeLock) {
                                disposeLock.notifyAll();
                            }
                        }
                    });
                if (millisToWait > 0) {
                    try {
                        disposeLock.wait(millisToWait);
                    } catch (InterruptedException e) {
                    }
                }
            }
        }
    }

    protected void shutdownAppContext(AppContext context,
                                      long startTime,
                                      long timeout,
                                      Applet2StopListener stopListener,
                                      boolean stopSuccessfulSoFar) {
        long t0 = DeployPerfUtil.put(0, "Plugin2Manager - shutdownAppContext() - BEGIN");

        // Take ourselves out of the AppContext, regardless of whether
        // we actually dispose it at this point
        unregisterFromAppContext(appletAppContext);
        DeployPerfUtil.put("Plugin2Manager - shutdownAppContext() - unregisterFromAppContext() - post");

        cleanupAppContext(startTime, timeout, stopListener);
        DeployPerfUtil.put("Plugin2Manager - shutdownAppContext() - cleanupAppContext() - post");

        DeployPerfUtil.put(t0, "Plugin2Manager - shutdownAppContext() - END");
    }

    protected void cleanupAppContext(long startTime, long timeout, Applet2StopListener stopListener) {
        AppContext ac = null;
        synchronized (this) {
            ac = appletAppContext;
            appletAppContext = null;
        }

        destroyAppContext(ac, stopListener, timeout - (System.currentTimeMillis() - startTime));
    }

    public synchronized void addAppletListener(Applet2Listener listener) {
        listeners.add(listener);
    }

    private synchronized void removeAllAppletListeners() {
        while ( ! listeners.isEmpty() ) {
            Applet2Listener a2l = (Applet2Listener) listeners.get(0);
            if(listeners.remove(a2l)) {
                a2l.released(this);
            }
        }
    }

    public synchronized void removeAppletListener(Applet2Listener listener) {
        if(listeners.remove(listener)) {
            listener.released(this);
        }
    }

    //----------------------------------------------------------------------
    // Dynamic resizing of applets
    //

    public void setAppletSize(final int width, final int height) {
        // Handle the parent container automatically
        final Container p = appletParentContainer;
        if (p != null && width > 0 && height > 0) {
            final AppContext c = appletAppContext;
            if (c != null) {
                DeployAWTUtil.invokeLater(c, new Runnable() {
                        public void run() {
                            p.setSize(width, height);
                        }
                    });
            }
        }
        // Resize the applet separately
        setSize(width, height);
    }

    //----------------------------------------------------------------------
    // Querying needed for modal dialog support
    //

    /** Indicates whether this Plugin2Manager is hosted in the same
        AppContext as the given one. This is currently used for
        handling of modal dialogs in the face of class loader caching.
        It isn't perfect (see 6621874) and we may have to come up with
        another mechanism. */
    public boolean isInSameAppContext(Plugin2Manager manager) {
        return isInSameAppContextImpl(manager);
    }

    /** For support of modal dialogs, drag-and-drop of applets to the
        desktop, and debugging purposes */
    public void setAppletID(Integer appletID) {
        this.appletID = appletID;
    }

    /** For support of modal dialogs, drag-and-drop of applets to the
        desktop, and debugging purposes */
    public Integer getAppletID() {
        return appletID;
    }

    public boolean isAppletRelaunched() {
        return  _appletRelaunched;
    }

    //----------------------------------------------------------------------
    // Internals only below this point
    //

    /*
     * The AppContextCreator class is used to create an AppContext from within
     * a Thread belonging to the new AppContext's ThreadGroup.  To wait for
     * this operation to complete before continuing, wait for the notifyAll()
     * operation on the syncObject to occur.
     */
    static class AppContextCreator extends Thread {
        Object syncObject = new Object();
        AppContext appContext = null;

        AppContextCreator(ThreadGroup group) {
            super(group, "AppContextCreator");
        }

        public void run() {
            synchronized(syncObject)  {
                appContext = SunToolkit.createNewAppContext();
                syncObject.notifyAll();
            }
        } // run()
    } // class AppContextCreator

    private volatile boolean appletStartResponseSent;
    private volatile boolean errorOccurred;
    private volatile Throwable errorException;
    private volatile String errorMessage;
    private static volatile boolean fShowException;
    private static volatile boolean fShowExceptionInitialized;

    // The execution context in which the applet is executing. Provides
    // services like communication with the outside world.
    private Applet2ExecutionContext appletExecutionContext;

    // The "default" execution context, which is a workaround for the
    // fact that certain services like proxy queries are made from the
    // system AppContext
    private static Applet2ExecutionContext defaultAppletExecutionContext;

    // The parameters for the applet, fetched once from the
    // AppletExecutionContext and cached
    private Map/*<String,String>*/ appletParameters;

    // Codebase for the applet, computed lazily and cached
    private URL codebase;

    // Size of the applet
    private boolean gotSize = false;
    private int width = 10;
    private int height = 10;

    // The thread which is responsible for calling the applet's control
    // methods -- init(), start(), stop(), destroy() -- as well as
    // downloading its data and running some other control code
    private volatile Thread appletExecutionThread;

    protected volatile ThreadGroup appletThreadGroup;

    protected volatile AppContext appletAppContext;

    protected volatile Plugin2ClassLoader loader;

    protected Object   stopLock = new Object();
    protected volatile boolean shouldStop = false;
    protected volatile boolean stopSuccessful = false;
    // Don't wait for more than this period of time for the applet to terminate cooperatively
    // Experience shows that until AppContext.dispose() only 10ms were consumed.
    // Therefor the phenomenon of a  slow applet stop() is related to the AppContext destruction,
    // for which we don't wait longer than STOP_TIMEOUT - 10ms.
    // However, applet may need more time to execute its stop() and destroy() methods.
    // We make this time configurable through deployment.properties and applet's parameter.
    // Until this time passed, we don't start shutdown AppContext and ThreadGroup.
    // get configured stop timeout. We don't want to wait too long, an integer is sufficient.
    
    protected static final int STOP_TIMEOUT_CONFIG = (Config.getIntProperty(Config.JAVAPI_STOP_TIMEOUT_KEY) == -1)? 
                                                          Config.JAVAPI_STOP_TIMEOUT_DEF: Config.getIntProperty(Config.JAVAPI_STOP_TIMEOUT_KEY);
    // The time we wait before checking whether all threads die in the ThreadGroup
    protected static final long THREADDIE_TIMEOUT = 3000; // ms

    private volatile Applet applet;
    // Used largely to provide answers to the AppletStub
    private volatile boolean appletIsActive;

    private volatile Container appletParentContainer;

    private GrayBoxPainter grayBoxPainter;
    private GrayBoxListener grayBoxListener;
    // Flag to indicate if applet is targeted for JDK 1.1.
    private boolean jdk11Applet = false;
     
    // Flag to indicate if applet is targeted for JDK 1.2.
    private boolean jdk12Applet = false;

    // Whether we should call init() on the newly-created applet. Set
    // to false in certain circumstances, in particular
    // deserialization of a serialized applet, and if the legacy
    // applet lifecycle is in use and we have already previously
    // called init().
    protected boolean doInit = true;

    private List/*<Applet2Listener>*/ listeners = new ArrayList/*<Applet2Listener>*/();

    protected static Applet2MessageHandler amh = new Applet2MessageHandler("appletpanel");

    // For modal dialog support as well as debugging
    protected Integer appletID;

    // To provide unique thread names to the applet execution threads
    private static int sequenceNumber = 0;
    private static int nextSequenceNumber() {
        synchronized (Plugin2Manager.class) {
            return ++sequenceNumber;
        }
    }

    // Indicates whether this manager is created only to set up an
    // AppContext, etc. and not an actual applet
    private boolean isForDummyApplet;

    // Indicates whether this applet has been disconnected from the browser
    private boolean disconnected;

    // Indicates whether this applet is running in a secure VM (VM launched
    // with secure properties)
    protected boolean isSecureVM = true;

    // this indicates, if the applet is relaunched
    private boolean _appletRelaunched = false;

    // Called by the exectution context to indicate the running VM
    // is secure or not.
    // PluginMain calls this method if the VM is insecure
    // If the manager is in standalone mode, isSecureVM is default value (true)
    public void setSecureFlag(boolean isSecure) {
        isSecureVM = isSecure;
    }

    // LiveconnectSupport calls this method to check if the VM is secure
    public boolean isVMSecure() {
	return isSecureVM;
    }

    // To avoid deadlocks with the outside world
    private synchronized List/*<Applet2Listener>*/ copyListeners() {
        return (List) ((ArrayList) listeners).clone();
    }

    protected boolean fireAppletSSVValidation() throws ExitException {
        for (Iterator iter = copyListeners().iterator(); iter.hasNext(); ) {
            if (!((Applet2Listener) iter.next()).appletSSVValidation(this)) {
                return false;
            }
        }
        return true;
    }

    protected String fireGetBestJREVersion(String javaVersionStr) {
	String bestVersion = null;
        for (Iterator iter = copyListeners().iterator(); iter.hasNext(); ) {
            bestVersion = ((Applet2Listener) iter.next()).getBestJREVersion(this, javaVersionStr);
	    if (bestVersion != null) {
		return bestVersion;
            }
        }
        return null;
    }

    protected boolean fireAppletRelaunchSupported() {
	boolean appletRelaunchSupported = true;
	for (Iterator iter = copyListeners().iterator(); iter.hasNext(); ) {
	    if (!((Applet2Listener) iter.next()).isAppletRelaunchSupported()) {
		appletRelaunchSupported = false;
	    }
	}
	return appletRelaunchSupported;
    }

    protected void fireAppletJRERelaunch(String javaVersion, String jvmArgs) {
        for (Iterator iter = copyListeners().iterator(); iter.hasNext(); ) {
            ((Applet2Listener) iter.next()).appletJRERelaunch(this, javaVersion, jvmArgs);
            setAppletStartResponseSent();
        }
    }

    private void fireAppletLoaded() {
        for (Iterator iter = copyListeners().iterator(); iter.hasNext(); ) {
            ((Applet2Listener) iter.next()).appletLoaded(this);
            setAppletStartResponseSent();
        }
    }

    private void fireAppletReady() {
        for (Iterator iter = copyListeners().iterator(); iter.hasNext(); ) {
            ((Applet2Listener) iter.next()).appletReady(this);
        }
    }

    private void fireAppletErrorOccurred() {
        for (Iterator iter = copyListeners().iterator(); iter.hasNext(); ) {
            ((Applet2Listener) iter.next()).appletErrorOccurred(this);
            setAppletStartResponseSent();
        }
    }

    //----------------------------------------------------------------------
    // Implementation methods for the creation of the class loader,
    // AppContext, and ThreadGroup, used when class loader caching is
    // both enabled and disabled
    // 

    // Fetches an already-created Plugin2ClassLoader, or creates a new
    // one, without consulting the cache
    protected Plugin2ClassLoader getOrCreatePlugin2ClassLoader() {
        synchronized (this) {
            if (loader == null) {
                currentThreadManager.set(Plugin2Manager.this);
                try {
                    AccessController.doPrivileged(new PrivilegedAction() {
                        public Object run() {
                            loader = newClassLoader();
                            return null;
                        }
                    });

                    // set whether we allow codebase recursive read
                    setupClassLoaderCodebaseRecursiveRead(loader);

                    // Set ClassLoader securityCheck flag
                    loader.setSecurityCheck(!isSecureVM);

                    // NOTE: it is essential for bootstrapping purposes when
                    // the class loader cache is in use that we do not call
                    // the "public" methods for fetching these, which will
                    // result in incorrect (null) answers
                    loader.setThreadGroup(getOrCreateAppletThreadGroup());
                    loader.setAppContext(getOrCreateAppletAppContext());
                } finally {
                    currentThreadManager.set(null);
                }
            }

            // 4668479: Option to turn off codebase lookup in AppletClassLoader 
            // during resource requests. [stanley.ho]
            String param = getParameter("codebase_lookup");
        
            if (param != null) {
                if (param.equals("false")) {
                    loader.setCodebaseLookup(false);
                } else {
                    loader.setCodebaseLookup(true);
                }
            }

            return loader;
        }
    }

    protected abstract Plugin2ClassLoader newClassLoader();

    protected ThreadGroup getOrCreateAppletThreadGroup() {
        synchronized (this) {
            if (appletThreadGroup == null) {
                final URL codebase = getCodeBase();

                // Code comes from AppletClassLoader.getThreadGroup()
                AccessController.doPrivileged(new PrivilegedAction() {
                        public Object run() {
                            appletThreadGroup = new Applet2ThreadGroup(codebase + "-threadGroup");
                            return null;
                        }
                    });
            }

            return appletThreadGroup;
        }
    }

    protected AppContext getOrCreateAppletAppContext() {
        synchronized(this) {
            if (appletAppContext == null) {
                // NOTE: it is essential for bootstrapping purposes
                // when the class loader cache is in use that we do
                // not call the "public" methods for fetching these,
                // which will result in incorrect (null) answers
                final Plugin2ClassLoader loader = getOrCreatePlugin2ClassLoader();
                if (appletAppContext != null) {
                    //attempt to get loader above will end up calling this method
                    //again. To avoid creation of extra useless appcontext we
                    //check if any work is needed again.
                    return appletAppContext;
                }
                final ThreadGroup tg = getOrCreateAppletThreadGroup();
                if (loader == null || tg == null) {
                    throw new InternalError("Error during bootstrapping of AppContext");
                }
                // Code comes from AppletClassLoader.getThreadGroup()
                AccessController.doPrivileged(new PrivilegedAction() {
                        public Object run() {
                            // Create the new AppContext from within a Thread belonging
                            // to the newly created ThreadGroup, and wait for the
                            // creation to complete before returning from this method.
                            AppContextCreator creatorThread = new AppContextCreator(tg);

                            // Since this thread will later be used to launch the
                            // applet's AWT-event dispatch thread and we want the applet 
                            // code executing the AWT callbacks to use their own class
                            // loader rather than the system class loader, explicitly
                            // set the context class loader to the AppletClassLoader.
                            creatorThread.setContextClassLoader(loader);

                            synchronized(creatorThread.syncObject)  {
                                creatorThread.start();
                                try {
                                    creatorThread.syncObject.wait();
                                } catch (InterruptedException e) { }
                                appletAppContext = creatorThread.appContext;
                            }
                            return null;
                        }
                    });
            }

            return appletAppContext;
        }
    }

    protected void destroyAppContext(final AppContext fac,
                                     final Applet2StopListener stopListener,
                                     long timeToWait) {
        long t0 = DeployPerfUtil.put(0, "Plugin2Manager - destroyAppContext() - BEGIN");
        
        // Even stop timeout is used up, we still want to give AppContextDisposer some 
        // time to run before check. Otherwise, we may mark the vm as "tainted" too eager
        if (timeToWait <= 0) {
            timeToWait = 10;
        } // give about 10 ms
        
        if (fac != null) {
            // Get a list of top level frame and the ThreadGroup first before dispose
            final ThreadGroup ftg = fac.getThreadGroup();
            final Window[][] windowBox = new Window[1][];
            final Object enumeratorLock = new Object();
            Thread windowEnumerator = new Thread(ftg, new Runnable() {
                    public void run() {
                        windowBox[0] = (Config.isJavaVersionAtLeast16()?
                                        Window.getOwnerlessWindows() 
                                        : Frame.getFrames());

                        // If we have used a Swing-based applet in
                        // this AppContext, also clear out the
                        // RepaintManager before disposing the
                        // AppContext as a workaround for temporary
                        // memory leaks due to strong references to
                        // Components held in the RepaintManager (see
                        // 6703401); note that this has to be done in
                        // the applet's AppContext
                        if (fac.get(APPCONTEXT_JAPPLET_USED_KEY) != null) {
                            javax.swing.RepaintManager.setCurrentManager(null);
                        }

                        synchronized(enumeratorLock) {
                            enumeratorLock.notifyAll();
                        }
                    }
                }, "Window enumerator");
            synchronized(enumeratorLock) {
                windowEnumerator.start();
                try {
                    enumeratorLock.wait(timeToWait);
                } catch (InterruptedException e) {
                }
            }
            DeployPerfUtil.put("Plugin2Manager - destroyAppContext() - windowEnumerator - post");
            final Window [] windows = windowBox[0];

            // Run this in a separate thread so we don't potentially hang things up
            // NOTE that the parent thread group at this point must
            // not be the applet's thread group!
            new Thread(new AppContextDisposer(fac, stopListener, stopLock)).start();

            // Wait a little longer for the AppContext to be fully destroyed
            synchronized (stopLock) {
                try {
                    stopLock.wait(timeToWait);
                } catch (InterruptedException e) {
                    DeployPerfUtil.put("Plugin2Manager - destroyAppContext() - stopLock - interrupted ..");
                }
            }
            DeployPerfUtil.put("Plugin2Manager - destroyAppContext() - stopLock - post");

            // null stopListener indicates the jvm is marked tained already
            // we don't check Component and Thread Leak in the AppContext
            if (null == stopListener) {
                DeployPerfUtil.put("Plugin2Manager - destroyAppContext() - END (3)");
                return;
            }

            // Check whether the AppContext (Components and Threads) has been disposed correctly
            if (windows != null) {
                for (int i = 0; i < windows.length; i++) {
                    if (windows[i].isDisplayable()) {
                        //the window is not disposed;
                        if (DEBUG) {
                            System.out.println("Plugin2Manager calling stopFailed() because of displayable window " + windows[i]);
                        }
                        stopListener.stopFailed();
                        DeployPerfUtil.put(t0, "Plugin2Manager - destroyAppContext() - END (2)");
                        return;
                    }
                }
            }
            DeployPerfUtil.put("Plugin2Manager - destroyAppContext() - windows/displayable - post");

            // Spawn a background thread to check whether there are active threads left in 
            // the ThreadGroup. It might take a little longer than the STOP_TIMEOUT for threads
            // to die, especially when they are stopped by AppContext.dispose(). On this background 
            // thread, a little more time is given for them to die before we check.
            new Thread(new ShutdownChecker(ftg, stopListener)).start();
        }
        DeployPerfUtil.put(t0, "Plugin2Manager - destroyAppContext() - END (1)");
    }

    // Make this a separate static inner class to avoid implicit
    // strong references to the Plugin2Manager during AppContext
    // destruction, which can take a long time in some situations
    private static class AppContextDisposer implements Runnable {
        private AppContext appContext;
        private Applet2StopListener stopListener;
        private Object stopLock;

        private AppContextDisposer(AppContext appContext,
                                   Applet2StopListener stopListener,
                                   Object stopLock) {
            this.appContext = appContext;
            this.stopListener = stopListener;
            this.stopLock = stopLock;
        }

        public void run() {
            DeployPerfUtil.put("Plugin2Manager - destroyAppContext() - appContext.dispose() - START");

            //Awt AppContext dispose() looks for ownerless windows first
            //and dispose them if exist.
            //frame disposal can potentially throw null pointer exception if native peer 
            //has been destroyed by the browser
            try {
                appContext.dispose();
            } catch (Exception ex) {
                if (stopListener != null) {
                    if (DEBUG) {
                        System.out.println("Plugin2Manager calling stopFailed() because of exception during AppContext.dispose()");
                    }
                    stopListener.stopFailed();
                }
            } catch (Throwable t) {
                Trace.ignored(t);
            }
            DeployPerfUtil.put("Plugin2Manager - destroyAppContext() - appContext.dispose() - END");
            synchronized (stopLock) {
                stopLock.notifyAll();
            }
        }
    }

    // This static helper class avoids an implicit strong reference
    // back to this Plugin2Manager during the shutdown sequence,
    // allowing things to get GCd more eagerly
    static class ShutdownChecker implements Runnable {
        private ThreadGroup group;
        private Applet2StopListener listener;

        ShutdownChecker(ThreadGroup group, Applet2StopListener listener) {
            this.group = group;
            this.listener = listener;
        }

        public void run() {
            try {
                Thread.sleep(THREADDIE_TIMEOUT);
            } catch (InterruptedException e) {
            }

            if ((group != null) && group.activeCount() > 0) {
                if (DEBUG) {
                    System.out.println("Plugin2Manager calling stopFailed() because of lingering threads");
                    group.list();
                }
                listener.stopFailed();
            }
        }
    }

    private volatile Object appletStartLock = null;

    // counter to track the number of java to javascript calls
    // during applet initialization phase.
    // A value of 0 indicates that a javascript to java call
    // is allowed. It is initialized to 1 because a java to javascript
    // call initiated from an applet will decrement it.
    private volatile int java2JSCounter = 1;

    public synchronized void increaseJava2JSCounter() {
        if (java2JSCounter != 0) {
            java2JSCounter++;
        }
    }

    public synchronized void decreaseJava2JSCounter() {
        if (java2JSCounter >= 1) {
            java2JSCounter--;
        }
    }

    public synchronized void unblockJS2Java() {
        java2JSCounter = 0;
    }

    public synchronized int getJava2JSCounter() {
        return java2JSCounter;
    }

    // Wait until applet.start() has been passed
    // It is not legal to call this method before appletLoaded 
    // has been signaled!
    public void waitUntilAppletStartDone() {
        Object llock = appletStartLock; // assure only one field access to appletStartLock
        if(llock!=null) {
            synchronized(llock) {
                // Make sure start() hasn't already passed
                if (appletStartLock != null && getJava2JSCounter() != 0 ) {
                    // Wait for start() to complete, or for
                    // notification to come in from the outside that
                    // we should allow this barrier to be passed
                    try {
                        llock.wait();
                    } catch (InterruptedException e) {
                    }
                }
            }
        } else {
            // appletStartLock hasn't been created yet
            // need to wait until java2JSCounter becomes 0
            while (getJava2JSCounter() != 0) {
                try {
                    Thread.sleep(10);
                } catch (java.lang.InterruptedException ie) {
                }
            }
        }
    }

    // Allow applet.start() barrier to be passed; this is needed if an
    // applet makes a Java-to-JavaScript call in init() and start()
    public void stopWaitingForAppletStart() {
        unblockJS2Java();
        Object llock = appletStartLock;
        if (llock != null) {
            synchronized(llock) {
                appletStartLock = null;
                llock.notifyAll();
            }
        }
    }

    //----------------------------------------------------------------------
    // The worker Runnable which manages the applet's lifecycle
    //
    // Note that the model in the face of the legacy applet lifecycle
    // (only implemented in Applet2Manager) is that we might start
    // multiple threads in succession (not in parallel) all running
    // this same Runnable.

    class AppletExecutionRunnable implements Runnable {

        public void run() {
            JRESelectException _jreSelectException = null;

            long t0 = DeployPerfUtil.put(0, "AppletExecutionRunnable - BEGIN");

            // store the current Plugin2Manager in a ThreadLocal
            currentManagerThreadLocal.set(Plugin2Manager.this);

            boolean doCreationAndInitialization =
                (!usingLegacyLifeCycle() || (applet == null));

	    boolean appletStartTerminated = false;

            try {
                // check if valid config before launch applets
                // we do here in order to be able to show the error dialog (red X)
                if (!Config.isConfigValid()) {
                    UIFactory.showErrorDialog(null,
                                              ResourceManager.getString("launcherrordialog.brief.message.applet"),
                                              ResourceManager.getString("enterprize.cfg.mandatory.applet",
                                                                        Config.getEnterprizeString()),
                                              ResourceManager.getString("error.default.title.applet"));
                    throw new ExitException(new Exception("Invalid Deployment Configuration"),
                                            ExitException.LAUNCH_ABORT_SILENT);
                }

                if (doCreationAndInitialization) {
                    if (shouldStop) return; // early bail out ..
                    initJarVersionMap();
                    if (shouldStop) return; // early bail out ..
                    setupAppletAppContext();
                }

                if (!isForDummyApplet) {
                    String code = getCode();
                    try {
                        // NOTE: we can't do the call to
                        // setupGrayBoxPainter() on the EDT because at
                        // this point there is apparently no EDT for us to
                        // use.
                        setupGrayBoxPainter();
                        if (doCreationAndInitialization) {
                            loadJarFiles();
                            DeployPerfUtil.put("AppletExecutionRunnable - post loadJarFiles()");
                            applet = createApplet();
                            DeployPerfUtil.put("AppletExecutionRunnable - post createApplet()");
                        }
                    } catch (ExitException ee) {
                        // just propagate the exception to skip the following code
                        // and use the later catch
                        throw ee; 
                    } catch (ClassNotFoundException e) {
                        setErrorOccurred("notfound "+code, e);
                        showAppletStatus("notfound", code);
                        showAppletLog("notfound", code);
                        showAppletException(e);
                        return;
                    } catch (InstantiationException e) {
                        setErrorOccurred("nocreate "+code, e);
                        showAppletStatus("nocreate", code);
                        showAppletLog("nocreate", code);
                        showAppletException(e);
                        return;
                    } catch (IllegalAccessException e) {
                        setErrorOccurred("noconstruct "+code, e);
                        showAppletStatus("noconstruct", code);
                        showAppletLog("noconstruct", code);
                        showAppletException(e);
                        // sbb -- I added a return here
                        return;
                    } catch (Exception e) {
                        setErrorOccurred(e);
                        showAppletStatus("exception", e.getMessage());
                        showAppletException(e);
                        return;
                    } catch (ThreadDeath e) {
                        setErrorOccurred("Applet ID " + appletID + " killed");
                        if (DEBUG) {
                            showStatusText("Applet ID " + appletID + " killed");
                        }
                        showAppletStatus("death");
                        return;
                    } catch (Error e) {
                        setErrorOccurred(e);
                        showAppletStatus("error", e.getMessage());
                        showAppletException(e);
                        return;
                    }

                    if (applet == null) {
                        if (!hasErrorOccurred()) {
                            String msg = "nocode";
                            setErrorOccurred(msg);
                            showAppletStatus(msg);
                            showAppletLog(msg);
                        }
                        return; // will reach finally
                    }

                    // Make this applet accessible to the AppletContext implementation
                    setActiveStatus(Plugin2Manager.this, true);

                    applet.setStub(stub);
                    showAppletStatus("loaded");

                    DeployPerfUtil.put("Switching from progress to Applet");
                    runOnEDT(appletParentContainer, new Runnable() {
                            public void run() {
                                if (shouldStop) return;
                                // Put the applet into the parent container automatically
                                // FIXME: think about what to do if the parent container is null

                                // Note that we deliberately make the applet initially invisible
                                // to prevent painting it before we call init() -- some applets
                                // out there rely on this behavior
                                applet.setVisible(false);
                                // Some applets out there expect their size to be set before
                                // init() is called
                                applet.resize(new Dimension(getWidth(), getHeight()));

                                try {
                                    removeProgressContainer(
                                        appletParentContainer);
                                    appletParentContainer.add(applet, BorderLayout.CENTER);
                                } catch (Error e) {
                                    // need to use JFrame.getContentPane().add
                                    // for 1.4.2 or earlier
                                    if (Config.isJavaVersionAtLeast15() == false &&
                                            appletParentContainer instanceof javax.swing.JFrame) {
                                        ((javax.swing.JFrame) appletParentContainer).getContentPane().add(applet, BorderLayout.CENTER);
                                    } else {
                                        throw e;
                                    }
                                } finally {
                                    DeployPerfUtil.put(
                                      "Done switching from progress to Applet");
                                }
                            }
                        });

                    DeployPerfUtil.put("AppletExecutionRunnable - post applet container");
                    showStatusText("Applet resized and added to parent container");

		    if (!shouldStop) {
			if(_INJECT_DELAY_APPLETLOADED) {
			    Trace.msgPrintln("PERF: AppletExecutionRunnable - fireAppletLoaded SLEEP");
			    try {
				Thread.sleep(5000);
			    } catch (InterruptedException e) { }
			}
			if (_INJECT_NEVER_APPLETLOADED) {
			    Trace.msgPrintln("PERF: AppletExecutionRunnable - fireAppletLoaded NEVER");
			    int i=999999;
			    while (i>0) {
				if(--i<999) i=999999;
				try {
				    Thread.sleep(1000);
				} catch (InterruptedException e) { }
			    }
			}	
		    }
		}

		// sync on the appletStartLock, as waitUntilAppletStartDone() does
		// Need be created before fireAppletLoaded()
		appletStartLock = new Object();

		// signal that the applet is loaded and we are in pre-init state
		// this drains and releases the LC queue on the browser side vm
		if (isForDummyApplet || !shouldStop) {
		    fireAppletLoaded();	
		}
		
		try {
		    if (!isForDummyApplet) {
			// Initialize the applet
			if (doInit) {
			    long total = SystemUtils.microTime()-Plugin2Manager.getJVMLaunchTime();
			    Trace.msgPrintln("PERF: AppletExecutionRunnable - applet.init() BEGIN ; jvmLaunch dt "+
					     Plugin2Manager.getJVMLaunchCosts()+" us, pluginInit dt "+
					     (total-Plugin2Manager.getJVMLaunchCosts())+" us, TotalTime: "+ total+" us");
			    DeployPerfUtil.put(t0, "AppletExecutionRunnable - applet.init() BEGIN");
			    try {
				applet.init();
			    } catch (Exception e) {
				final Exception ex = e;
                                suspendGrayBoxPainting();
				runOnEDT(appletParentContainer, new Runnable() {
					public void run() {
					    if (shouldStop) return;
					    shutdownGrayBoxPainter();
					    appletParentContainer.remove(applet);
					    showAppletException(ex);
					}
				    });
			    } finally {
				DeployPerfUtil.put(t0, "AppletExecutionRunnable - applet.init() END");
			    }
			    doInit = false; // Note that this handles the legacy lifecycle case too
			    showStatusText("Applet initialized");
			}   
		    
			// FIXME: deleted code related to setting up of fallback font as
			// this class is no longer a Component; should we do that for
			// the parent container?
			
			if (!hasErrorOccurred() && !shouldStop) {
                            suspendGrayBoxPainting();

			    // Validate and show the applet in the Event Dispatch Thread
			    runOnEDT(appletParentContainer, new Runnable() {
				    public void run() {
					if (shouldStop) return;
					shutdownGrayBoxPainter();
					appletParentContainer.validate();
					applet.validate();
					applet.setVisible(true);
					
					// Fix for BugTraq ID 4041703.
					// Set the default focus for an applet.
					if (hasInitialFocus())
					    setDefaultFocus();
				    }
				});
			    
			    showStatusText("Applet made visible");
			}
		    }
		    
		    if (!shouldStop) {
                        unblockJS2Java();
			// Start the applet
			appletIsActive = true;
			if (!isForDummyApplet) {
			    showStatusText("Starting applet");
			    try {
				if (!(DeployPerfUtil.isDeployFirstframePerfEnabled())) {
				    DeployPerfUtil.write(new PluginRollup(), true);
				    Trace.println("completed perf rollup", TraceLevel.BASIC);
				}
			    } catch (IOException ioe) {
				// ignore exception
			    }
			    try {
				applet.start();
			    } catch (Exception e) {
				final Exception ex = e;
				runOnEDT(appletParentContainer, new Runnable() {
					public void run() {
					    if (shouldStop) return;
					    appletParentContainer.remove(applet);
					    showAppletException(ex);
					}
				    });
			    }
			    showStatusText("Applet started");
			}
		    } else {
			appletStartTerminated = true;
		    }
		    
		} finally {
		    // release appletStartLock
		    Object o = appletStartLock;
		    if (o != null) {
			synchronized(o) {
			    appletStartLock = null;
			    o.notifyAll();
			}
		    }
                    unblockJS2Java();
		}

                if (!shouldStop) {
                    // At this point we can consider the applet ready
                    fireAppletReady();
                    showStatusText("Told clients applet is started");
                } else {
                    appletStartTerminated = true;
                }

                if (!shouldStop) {
                    if (!isForDummyApplet) {
                        // Force re-layout in case applet added components in start() method
                        runOnEDT(appletParentContainer, new Runnable() {
                                public void run() {
                                    if (shouldStop) return;
                                    appletParentContainer.invalidate();
                                    appletParentContainer.validate();
                                }
                            });
                    }
                } else {
                    appletStartTerminated = true;
                } 

                if(appletStartTerminated) {
                    showStatusText("Skipped starting applet -- terminated abruptly");
                }

                synchronized (stopLock) {
                    while (!shouldStop) {
                        try {
                            stopLock.wait();
                        } catch (InterruptedException e) {
                        }
                    }
                }

                // OK, at this point we're supposed to terminate the applet.
                showStatusText("Starting applet teardown");
      
                if (!isForDummyApplet) {
                    // Hide the applet in the event dispatch thread to avoid
                    // deadlock.
                    runOnEDT(applet, new Runnable() {
                            public void run() {
                                applet.setVisible(false);
                            }
                        });
                }

                appletIsActive = false;
                if (!isForDummyApplet) {
                    // Certain exceptions like AccessControlExceptions during stop() and destroy() apparently
                    // remain persistent in the class loader's "memory" and will show up again if the same
                    // class loader is reused. Watch for exceptions during stop() / destroy() and avoid reusing
                    // the class loader instance if any show up. Note we don't propagate these as doing so in
                    // this thread is pointless.

                    // We also won't try to cache the applet instance
                    // for legacy lifecycle purposes if we get an
                    // exception during stop().
                    try {
                        applet.stop();
                    } catch (Throwable t) {
                        invalidateClassLoaderCacheEntry();
                        clearUsingLegacyLifeCycle();
                        t.printStackTrace();
                    }

                    // FIXME: for correctness, should actually run the afterStopRunnable at this point;
                    // however, don't want to rely on it being run on this thread, since everything
                    // here is "opportunistic" / "cooperative". Should introduce an additional synchronization
                    // point with the thread running Plugin2Manager.stop().

                    if (!usingLegacyLifeCycle()) {
                        // Destroy the applet
                        try {
                            applet.destroy();
                        } catch (Throwable t) {
                            invalidateClassLoaderCacheEntry();
                            t.printStackTrace();
                        }
                    }

                    // Remove the applet from the parent container -- this is the
                    // "dispose" step
                    // Note that if the applet isn't terminating
                    // cooperatively, the parent container might have been
                    // disposed by the thread doing the shutdown at this point
                    runOnEDT(applet, new Runnable() {
                            public void run() {
                                Container c = getAppletParentContainer();
                                if (c != null) {
                                    c.remove(applet);
                                }
                            }
                        });

                    // That's it.
                }

                showStatusText("Finished applet teardown");

                synchronized (stopLock) {
                    stopSuccessful = true;
                    stopLock.notifyAll();
                }

                // FIXME: need to pull in code from
                // AppletClassLoader.release() here to properly
                // dispose the AppContext -- need a watchdog thread
                // and asynchronous listener mechanism on this class
                // so that if the disposal of the AppContext hangs, we
                // mark this JVM instance as tainted

            } catch (JRESelectException jreSelectException) {
                // The running JRE is not suitable; C/S roundtrip, restart JRE from server
                // We have to fire the JRERelaunch _after_ we have properly teared this one down.
                _jreSelectException = jreSelectException;
            } catch (ExitException ee) {
                if (ee.isErrorException())
                {
                    setErrorOccurred(ee);
                    showAppletStatus("exception", ee.getMessage());
                    showAppletLog("exception", ee.getMessage());
                    showAppletException(ee.getException(), ee.isSilentException());
                } else if (ExitException.LAUNCH_SINGLETON == ee.getReason()) {
                    showAppletStatus(getErrorMessage());
                    showAppletLog(getErrorMessage());
                    showAppletException(ee.getException(), ee.isSilentException());
                }
            } finally {
                Container c = getAppletParentContainer();

                if(shouldStop) {
                    // nobody expect us to deliver this signal any more
                    setAppletStartResponseSent();

                    // no relaunch ..
                    _jreSelectException=null;
                }

                if(hasErrorOccurred())
                {
                    fireAppletErrorOccurred();

                } else if (hasAppletStartResponseBeenSent() || null!=_jreSelectException) {
                    // Only remove the gray box painter if no error
                    // occurred since it is needed for displaying
                    // an error notification such as the red X,
                    // and if the response is send incl. the future JRERelaunch.
                    if ( grayBoxPainter != null && c != null) {
                        runOnEDT(c, new Runnable() {
                                public void run() {
                                    shutdownGrayBoxPainter();
                                }
                            });
                    }
                }
                appletIsActive = false;
                setActiveStatus(Plugin2Manager.this, false);
                appletExecutionThread = null;

                if(null!=_jreSelectException) {
                    // tear down the EmbeddedFrame properly ..
                    setAppletParentContainer(null);
                    if (c != null && (c instanceof Window)) {
                        final Window parentWindow = (Window) c;
                        c=null;
                        parentWindow.dispose();
                    }

                    // pending JRE Relaunch ..
                    JREDesc jreDesc = _jreSelectException.getJREDesc();
                    fireAppletJRERelaunch((null!=jreDesc)?jreDesc.getVersion():null, _jreSelectException.getJVMArgs());
                    _jreSelectException=null;
                }

                if (!hasAppletStartResponseBeenSent()) {
                    showAppletException(new Exception("AppletLifecycle interrupted"));
                    fireAppletErrorOccurred(); 
                }
                if (!hasAppletStartResponseBeenSent()) {
                    // This means, that Applet2Listener.appletErrorOccurred()
                    // has not send a message.
                    // This is a serious implementation error!
                    showAppletException(new Exception("Applet2Listener.appletErrorOccurred() has not sent a message"));
                }
                removeAllAppletListeners();
            }
        }
    }

    protected void initJarVersionMap() {
    }

    protected synchronized void invalidateClassLoaderCacheEntry() {
    }

    protected boolean usingLegacyLifeCycle() {
        return false;
    }

    protected void clearUsingLegacyLifeCycle() {
    }

    //----------------------------------------------------------------------
    // Fetching of applet information
    //

    private Map/*<String,String>*/ getAppletParameters() {
        if (appletParameters == null) {
            if (appletExecutionContext == null) {
                throw new IllegalStateException("Requires AppletExecutionContext to be set by now");
            }
            appletParameters = appletExecutionContext.getAppletParameters();
            if (appletParameters == null) {
                throw new IllegalStateException("AppletExecutionContext illegally returned a null parameter map");
            }
        }
        return appletParameters;
    }


    /**
     * Returns the codebase of the applet.
     */
    protected URL getCodeBase() {
        if (codebase == null) {
            // Comes from original plug-in AppletViewer.getCodeBase()
            String attr = getParameter("java_codebase");
            if (attr == null)
                attr = getParameter("codebase");

            if (attr != null) {
                if (!attr.equals(".") && !attr.endsWith("/")) {
                    attr += "/";
                }

                // Canonicalize URL in case the URL is in some
                // weird form only recognized by the browser
                //
                attr = URLUtil.canonicalize(attr);
            }

            // NOTE: (FIXME) this is a change from the original code
            // and is an attempt to avoid unneeded calls back to the
            // web browser. If the codebase is an absolute URL
            // (including a protocol) rather than a relative one, then
            // we can avoid fetching the document base, which requires
            // a round trip back to the browser.
            if (attr != null) {
                try {
                    URL tmp = new URL(attr);
                    // If this succeeds, I don't think we need the document base
                    codebase = tmp;
                    return codebase;
                } catch (MalformedURLException e) {
                }
            }

            URL docBase = getDocumentBase();
            if (docBase == null) {
                // FIXME: don't like this behavior; need to better specify
                return null;
            }
            URL baseURL = null;
            if (attr != null) {
                try {
                    baseURL = new URL(docBase, attr);
                } catch (MalformedURLException e) {
                    // ignore exception
                }
            }      
      
            if (baseURL == null) {
                String urlString = docBase.toString();
                int i = urlString.indexOf('?'); 
                if (i > 0) { 
                    urlString = urlString.substring(0, i); 
                } 
                i = urlString.lastIndexOf('/');
                if (i > -1 && i < urlString.length() - 1) {
                    try {
                        // Canonicalize URL to ensure it is well-formed
                        baseURL = new URL(URLUtil.canonicalize(urlString.substring(0, i + 1)));
                    } catch (MalformedURLException e) {
                        // ignore exception
                    }
                }

                // when all is said & done, baseURL shouldn't be null
                if (baseURL == null)
                    baseURL = docBase;
            }

            codebase = baseURL;
        }

        return codebase;
    }

    /**
     * Get the code parameter.
     */
    protected String getCode() {
        // Support HTML 4.0 style of OBJECT tag.
        //
        // <OBJECT classid=java:sun.plugin.MyClass .....>
        // <PARAM ....>
        // </OBJECT>
        //
        // In this case, the CODE will be inside the classid
        // attribute.
        //
        String moniker = getParameter("classid");
        String code = null;
        
        if (moniker != null) {
            int index = moniker.indexOf("java:");
            
            if (index > -1) {
                code = moniker.substring(5 + index);
                
                if (code != null || !code.equals(""))
                    return code;
            }
        }
        
        code = getParameter("java_code");
        if (code == null)
            code = getParameter("code");
        return code;
    }

    /*
     * @return the applet name
     */
    public String getName() {
        String name = getParameter("name");
        if (name != null)
            return name;
        
        // Remove .class extension
        name = getCode();
        if (name != null) {
            int index = name.lastIndexOf(".class");
            if (index != -1)
                name = name.substring(0, index);
        } else {
            // Remove .object extension
            name = getSerializedObject();
            
            if (name != null) {
                int index = name.lastIndexOf(".ser");
                if (index != -1)
                    name = name.substring(0, index);
            }
        }
        
        return name;
    }

    /**
     * Return the value of the object param
     */
    protected String getSerializedObject() {
        String object = getParameter("java_object");
        if (object == null)
            object = getParameter("object"); // another name?
        return object;
    }

    private synchronized void setSize(int width, int height) {
        // Note that in similar fashion to the previous
        // implementation, we change the width and height in the
        // parameters as we receive a new width and height from the
        // outside
        if (width > 0 && height > 0) {
            // Filter out bogus inputs
            String w = Integer.toString(width);
            String h = Integer.toString(height);
            setParameter("width",  w);
            setParameter("height", h);
            // If the AppletExecutionRunnable hasn't reached the point
            // where it's queried the applet's size, it will do so
            // automatically later; otherwise we can call the applet's
            // stub to resize the applet
            if (gotSize && applet != null && stub != null) {
                stub.appletResize(width, height);
            }
        }
    }

    private synchronized void getSize() {
        if (!gotSize) {
            String w = getParameter("width");
            if (w != null) {
                width = Integer.parseInt(w);
            }
            String h = getParameter("height");
            if (h != null) {
                height = Integer.parseInt(h);
            }
            gotSize = true;
        }
    }

    /**
     * Returns the width of the applet.
     */
    public int getWidth() {
        getSize();
        return width;
    }

    /**
     * Returns the height of the applet.
     */
    public int getHeight() {
        getSize();
        return height;
    }

    /**
     * Get initial_focus
     */
    private boolean hasInitialFocus() {    
        // 6234219: Do not set initial focus on an applet 
        // during startup if applet is targeted for 
        // JDK 1.1/1.2. [stanley.ho]
        if (isJDK11Applet() || isJDK12Applet())
            return false;
                
        String initialFocus = getParameter("initial_focus");

        if (initialFocus != null) {
            if (initialFocus.toLowerCase().equals("false")) 
                return false;
        }

        // FIXME: must break this dependency on the Config class
        if (Config.getInstance().isNativeModalDialogUp())
            return false;

        return true;
    }

    //----------------------------------------------------------------------
    // Setup of applet resources
    //

    protected void setupAppletAppContext() {
        appletAppContext.put(Config.APPCONTEXT_APP_NAME_KEY, getName());
        appletAppContext.put(APPCONTEXT_APPLETCONTEXT_KEY, getAppletContext());
    }

    /**
     * Return the list of jar files if specified.
     * Otherwise return null.
     */
    protected abstract String getJarFiles();

    // This method works the same as getJarFiles() in Applet2Manager
    // It returns both jar and jnlp locations in JNLP2Manager
    protected abstract String getCodeSourceLocations();

    //----------------------------------------------------------------------
    // Gray box painting
    //

    private void setupGrayBoxPainter() {
        long t0 = DeployPerfUtil.put(0, "Plugin2Manager.setupGrayBoxPainter() - BEGIN");

        final Container c = getAppletParentContainer();
        if (c != null) {
            grayBoxPainter = new GrayBoxPainter(c);

            // Set up the error delegate
            grayBoxPainter.setErrorDelegate(new ErrorDelegate() {
                    public void handleReloadApplet() {
                        invalidateClassLoaderCacheEntry();
                        getAppletContext().showDocument(getDocumentBase());
                    }

                    public String getCodeBase() {
                        return Plugin2Manager.this.getCodeBase().toString();
                    }

                    public void addJarFilesToSet(Set/*<String>*/ jarSet) {
                        String jars = getCodeSourceLocations();
                        if (jars != null){
                            // Jar files are separated by "," 
                            StringTokenizer jarTok = 
                                new StringTokenizer(jars, ",", false);
                            int jarCount = jarTok.countTokens();

                            for(int i=0; i<jarCount; i++) {
                                String jarFileName = jarTok.nextToken().trim();
                                jarSet.add(jarFileName);                            
                            }
                        }
                    }
                });

            // See if user specified special image to be shown 
            String userImage = getParameter("image");
            if (userImage != null) {
                try {
                    URL customImageURL = new URL(getCodeBase(), userImage);
                    boolean centerImage = Boolean.valueOf(getParameter("centerimage")).booleanValue();
                    grayBoxPainter.setBoxBorder(getParameter("boxborder"));
                    grayBoxPainter.setCustomImageURL(customImageURL, centerImage);
                } catch (MalformedURLException e) {
                    e.printStackTrace();
                }             
            }     

            String customBoxMessage = getParameter("boxmessage");

            // Set up the background color of the applet's parent container as
            // well as the gray box painter
            setupColorAndText(customBoxMessage);

            // Get a list of jar files
            if (useGrayBoxProgressListener()) {
                grayBoxPainter.setProgressFilter(getCodeBase(), getJarFiles());
            } else {
                grayBoxPainter.setUsingProgressListener(false);
            }
            grayBoxPainter.beginPainting(appletExecutionThread.getThreadGroup());
            //DeployPerfUtil.put("Plugin2Manager.setupGrayBoxPainter() - post beginPainting");

            // Add mouse listener
            grayBoxListener = new GrayBoxListener(c, customBoxMessage);

            runOnEDT(c, new Runnable() {
                    public void run() {
                        c.addMouseListener(grayBoxListener);
                    }
                });
        }
        DeployPerfUtil.put(t0, "Plugin2Manager.setupGrayBoxPainter() - END");
    }

    protected boolean useGrayBoxProgressListener() {
        // Use this by default
        return true;
    }

    protected void setGrayBoxProgress(float progress) {
        GrayBoxPainter painter = grayBoxPainter;
        if (painter != null) {
            painter.setProgress(progress);
        }
    }

    protected void setGrayBoxError() {
        GrayBoxPainter painter = grayBoxPainter;
        if (painter != null) {
            painter.showLoadingError();
        }
    }

    protected void suspendGrayBoxPainting() {
        if (grayBoxPainter != null) {
            grayBoxPainter.suspendPainting();
        }
    }

    protected void shutdownGrayBoxPainter() {

        // Destroy gray box painter
        if (grayBoxPainter != null) {
            grayBoxPainter.finishPainting();
            grayBoxPainter = null;
        }

        // Remove graybox listener
        if (grayBoxListener != null) {
            Container c = getAppletParentContainer();
            if (c != null) {
                c.removeMouseListener(grayBoxListener);
            }
            grayBoxListener = null;
        }
    }

    // override in JNLP2Manager
    protected void removeProgressContainer(final Container parent) {
        // do nothing if not overwritten
    }

    /*
     * See if user specified the background color for the applet's gray box
     * or a foreground color and set these values for the applet.
     */
    private void setupColorAndText(String customBoxMessage) {
        Color val = null;
        /*
         * See if user specified any colors
         *      BOXBGCOLOR - background color for the gray box
         *      BOXFGCOLOR - foreground color for the gray box
         *      BOXMESSAGE - user-defined message for the applet viewer.
         * If not - use defaults - light gray for background, black for foreground,
         * and purple for the progress bar color.
         */
        String boxBGColorStr = getParameter(ParameterNames.BOX_BG_COLOR);
        if (boxBGColorStr != null) {
            val = ColorUtil.createColor(ParameterNames.BOX_BG_COLOR, boxBGColorStr);
            
            /*
             * If user specified valid color, set background color.
             */
            if (val != null)
                grayBoxPainter.setBoxBGColor(val);
        }

        String boxFGColorStr = getParameter(ParameterNames.BOX_FG_COLOR);
        if (boxFGColorStr != null) {
            val = ColorUtil.createColor(ParameterNames.BOX_FG_COLOR, boxFGColorStr);
            
            /*
             * If user specified valid color, set foreground color.
             */
            if (val != null)
                grayBoxPainter.setBoxFGColor(val);
        }
        
        final Container c = getAppletParentContainer();
        if (c != null) {
            runOnEDT(c, new Runnable() {
                public void run() {
                    c.setBackground(grayBoxPainter.getBoxBGColor());
                    c.setForeground(grayBoxPainter.getBoxFGColor());
                }
            });
        }
        
        if (customBoxMessage != null)
            grayBoxPainter.setWaitingMessage(customBoxMessage);
        else
            grayBoxPainter.setWaitingMessage(getWaitingMessage());
    }

    private class GrayBoxListener implements MouseListener, ActionListener {
        private PopupMenu popup;
        private MenuItem open_console, about_java;
        private String msg = null;
        private Container parent;

        GrayBoxListener(Container parent, String msg) {
            this.msg = msg;
            this.parent = parent;
        }

        private PopupMenu getPopupMenu()
        {
            if (popup == null)
                {
                    Font f = parent.getFont();

                    // Derive font to make it looks like Swing
                    Font menuFont = f.deriveFont(11.0f);

                    // Create popup menu
                    popup = new PopupMenu();
                    open_console = new MenuItem(ResourceManager.getMessage("dialogfactory.menu.open_console")); 
                    open_console.setFont(menuFont);
                    about_java = new MenuItem(ResourceManager.getMessage("dialogfactory.menu.about_java"));
                    about_java.setFont(menuFont);

                    open_console.addActionListener(this);
                    about_java.addActionListener(this);

                    popup.add(open_console);
                    popup.add("-");
                    popup.add(about_java);

                    parent.add(popup);                      
                }

            return popup;
        }

        /*
         * When pointing at the applet - display status message (or user-defined
         * message) in browser status field.
         */
        public void mouseEntered(MouseEvent e) {
            if (msg != null)
                showStatusText(msg);
            else
                showStatusText(getWaitingMessage());
        }
    
        public void mouseExited(MouseEvent e) {}
        public void mousePressed(MouseEvent e) {
            if (e.isPopupTrigger() && hasErrorOccurred()) {
                //Pop up menu with option to display Java Console if an error occured
                //while loading applet.  This will be called on unix.
                getPopupMenu().show(e.getComponent(), e.getX(), e.getY());
            
            }
        }
        public void mouseReleased(MouseEvent e) {
            if (e.isPopupTrigger() && hasErrorOccurred()) {
                //Pop up menu with option to display Java Console if an error occured
                //while loading applet.  This will be called on windows.
                getPopupMenu().show(e.getComponent(), e.getX(), e.getY());            
            }
        }   
        public void mouseClicked(MouseEvent e) {}

        /**
         * Invoked when an action occurs.
         */
        public void actionPerformed(ActionEvent e) {
            if (e.getSource() == open_console){ 
                // Popup java console and print exceptions to it from trace file.        
                JavaRunTime.showJavaConsoleLater(true);
                JavaRunTime.installConsoleTraceListener();
            }
            else if (e.getSource() == about_java){
                // Show about java dialog
                UIFactory.showAboutJavaDialog();
            }
        }
    }

    //----------------------------------------------------------------------
    // Implementation of AppletStub which forwards to this manager or
    // the Applet2ExecutionContext where necessary
    //

    private AppletStubImpl stub = new AppletStubImpl();
    protected AppletStub getAppletStub() {
        return (AppletStub) stub;
    }
    private class AppletStubImpl implements AppletStub {
        /**
         * Determines if the applet is active. An applet is active just
         * before its <code>start</code> method is called. It becomes
         * inactive just before its <code>stop</code> method is called.
         *
         * @return  <code>true</code> if the applet is active;
         *          <code>false</code> otherwise.
         */
        public boolean isActive() {
            return appletIsActive;
        }

        /**
         * Gets the URL of the document in which the applet is embedded.
         * For example, suppose an applet is contained
         * within the document:
         * <blockquote><pre>
         *    http://java.sun.com/products/jdk/1.2/index.html
         * </pre></blockquote>
         * The document base is:
         * <blockquote><pre>
         *    http://java.sun.com/products/jdk/1.2/index.html
         * </pre></blockquote>
         *
         * @return  the {@link java.net.URL} of the document that contains the
         *          applet.
         * @see     java.applet.AppletStub#getCodeBase()
         */
        public URL getDocumentBase() {
            return Plugin2Manager.this.getDocumentBase();
        }
    
        /**
         * Gets the base URL. This is the URL of the directory which contains the applet.
         *
         * @return  the base {@link java.net.URL} of
         *          the directory which contains the applet.
         * @see     java.applet.AppletStub#getDocumentBase()
         */
        public URL getCodeBase() {
            return Plugin2Manager.this.getCodeBase();
        }

        /**
         * Returns the value of the named parameter in the HTML tag. For
         * example, if an applet is specified as
         * <blockquote><pre>
         * &lt;applet code="Clock" width=50 height=50&gt;
         * &lt;param name=Color value="blue"&gt;
         * &lt;/applet&gt;
         * </pre></blockquote>
         * <p>
         * then a call to <code>getParameter("Color")</code> returns the
         * value <code>"blue"</code>.
         *
         * @param   name   a parameter name.
         * @return  the value of the named parameter,
         * or <tt>null</tt> if not set.
         */
        public String getParameter(String name) {
            return Plugin2Manager.this.getParameter(name);
        }

        /**
         * Returns the applet's context.
         *
         * @return  the applet's context.
         */
        public AppletContext getAppletContext() {
            return Plugin2Manager.this.getAppletContext();
        }

        /**
         * Called when the applet wants to be resized.
         *
         * @param   width    the new requested width for the applet.
         * @param   height   the new requested height for the applet.
         */
        public void appletResize(final int width, final int height) {
            // Note that unlike the previous implementations, we don't
            // need to worry about resizing the "AppletPanel" or
            // "AppletViewer" since we've eliminated that component
            // from the hierarchy in this implementation.
            final Applet a = applet;
            final AppContext c = appletAppContext;
            if (a != null && c != null) {
                Runnable r = new Runnable() {
                        public void run() {
			    a.resize(width, height);
                            a.validate();
                        }
                    };		
                if (EventQueue.isDispatchThread()) {
                    // Assuming we're on the applet's EDT in this case
                    r.run();
                } else {
                    // It is very important to invoke this later instead of now
                    // to avoid potential deadlocks
                    DeployAWTUtil.invokeLater(a, r);
                }
            }
        }
    }

    //----------------------------------------------------------------------
    // Implementation of AppletContext which forwards to the
    // Applet2ExecutionContext where necessary. For those methods which
    // do not require communication with the outside world, a default
    // and currently non-overridable implementation is provided.
    //

    // This Set shouldn't be accessed directly, only through the
    // setActiveStatus method
    private static Set/*<WeakReference<Plugin2Manager>>*/ activeManagers =
        Collections.synchronizedSet(new HashSet/*<WeakReference<Plugin2Manager>>*/());
  
    private static void setActiveStatus(Plugin2Manager manager, boolean active) {
        if (active) {
            activeManagers.add(new WeakReference(manager));
        } else {
            List/*<WeakReference<Plugin2Manager>>*/ deadRefs = null;
            synchronized (activeManagers) {
                for (Iterator iter = activeManagers.iterator(); iter.hasNext(); ) {
                    WeakReference/*<Plugin2Manager>*/ ref = (WeakReference) iter.next();
                    Plugin2Manager referent = (Plugin2Manager) ref.get();
                    if (referent == null) {
                        if (deadRefs == null) {
                            deadRefs = new ArrayList/*<WeakReference<Plugin2Manager>>*/();
                        }
                        deadRefs.add(ref);
                    } else if (referent == manager) {
                        activeManagers.remove(ref);
                        break;
                    }
                }
            }
            if (deadRefs != null) {
                for (Iterator iter = deadRefs.iterator(); iter.hasNext(); ) {
                    activeManagers.remove(iter.next());
                }
            }
        }
    }

    private static List/*<Plugin2Manager>*/ getActiveManagers() {
        List/*<Plugin2Manager>*/ managers = new ArrayList/*<Plugin2Manager>*/();
        synchronized (activeManagers) {
            for (Iterator iter = activeManagers.iterator(); iter.hasNext(); ) {
                WeakReference ref = (WeakReference) iter.next();
                Plugin2Manager mgr = (Plugin2Manager) ref.get();
                if (mgr != null) {
                    managers.add(mgr);
                }
            }
        }
        return managers;
    }

    private AppletContextImpl appletContext;

    // FIXME: should rethink the caching strategy here; at a minimum
    // this implementation leaks the storage for the URLs used as keys
    // and the HashMaps
    private static Map/*<URL,Image>*/ imageRefs = new HashMap();
    private static Map/*<URL,Map<URL,AudioClip>>*/ audioClipStore = new HashMap();

    private static final int PERSIST_STREAM_MAX_SIZE = 65536;
    private static Map streamStore = new HashMap();

    // Shared code in this class was derived from the original DefaultPluginAppletContext
    private static class AppletContextImpl implements AppletContext, JSContext {
        // We use a weak reference to refer to our manager to support clean teardown
        private WeakReference ref;

        private AppletContextImpl(Plugin2Manager mgr) {
            ref = new WeakReference(mgr);
        }

        /*
         * Get an audio clip.
         *
         * @param url url of the desired audio clip
         */
        public AudioClip getAudioClip(final URL url) 
        {
            // Fixed #4508940: MSVM takes null URL and return null
            //
            if (url == null)
                return null;

            System.getSecurityManager().checkConnect(url.getHost(), url.getPort());

            SoftReference ref = null;

            synchronized (audioClipStore)
                {
                    // Obtain audio list according to applet context
                    HashMap audioClips = (HashMap) audioClipStore.get(getCodeBase());
            
                    if (audioClips == null)
                        {
                            audioClips = new HashMap();
                            audioClipStore.put(getCodeBase(), audioClips);
                        }

                    ref = (SoftReference) audioClips.get(url);

                    if (ref == null || ref.get() == null)
                        {
                            ref = new SoftReference(Applet2AudioClipFactory.createAudioClip(url));
                            audioClips.put(url, ref);
                        }
                }

            // Get audio clip outside sync block
            AudioClip clip = (AudioClip) ref.get();

            Trace.msgPrintln("appletcontext.audio.loaded", new Object[] {url});

            return clip;
        }

        /*
         * Get an image.
         * 
         * @param url of the desired image
         */
        public Image getImage(URL url) 
        {
            // Fixed #4508940: MSVM takes null URL and return null
            //
            if (url == null)
                return null;

            System.getSecurityManager().checkConnect(url.getHost(), url.getPort());

            SoftReference ref = null;

            synchronized (imageRefs) 
                {
                    ref = (SoftReference) imageRefs.get(url);

                    if (ref == null || ref.get() == null) 
                        {
                            ref = new SoftReference(Applet2ImageFactory.createImage(url));
                
                            imageRefs.put(url, ref);
                        }

                }

            Image image = (Image) ref.get();

            Trace.msgPrintln("appletcontext.image.loaded", new Object[] {url});

            return image;
        }

        public Applet getApplet(String name) {
            List/*<Plugin2Manager>*/ managers = getActiveManagers();

            name = name.toLowerCase();

            for (Iterator iter = managers.iterator(); iter.hasNext(); ) {
                Plugin2Manager mgr = (Plugin2Manager) iter.next();
                // NOTE: removed the check from the original code regarding
                // identity of the applet's container -- which I think was a bug
                Applet applet = mgr.getApplet();
                if (applet != null) {
                    String param = mgr.getParameter("name");
                    if (param != null) {
                        param = param.toLowerCase();
                    }
                    if (name.equals(param) && 
                        mgr.getDocumentBase().equals(getDocumentBase())) {
                        try {
                            if (!checkConnect(getCodeBase().getHost(),
                                              mgr.getCodeBase().getHost()))
                                return null;
                        } catch (InvocationTargetException ee) {
                            showStatusText(ee.getTargetException().getMessage());
                            return null;
                        } catch (Exception ee) {
                            showStatusText(ee.getMessage());
                            return null;
                        }
                        return mgr.getApplet();
                    }
                }
            }

            return null;
        }

        public Enumeration/*<Applet>*/ getApplets() {
            List/*<Plugin2Manager>*/ managers = getActiveManagers();
            List/*<Applet>*/ applets = new ArrayList/*<Applet>*/();
            for (Iterator iter = managers.iterator(); iter.hasNext(); ) {
                Plugin2Manager mgr = (Plugin2Manager) iter.next();

                if (mgr.getDocumentBase().equals(getDocumentBase())) {
                    try {
                        if (checkConnect(getCodeBase().getHost(), mgr.getCodeBase().getHost())) {
                            Applet a = mgr.getApplet();
                            if (a != null) {
                                applets.add(a);
                            }
                        }
                    } catch (InvocationTargetException ee) {
                        showStatusText(ee.getTargetException().getMessage());
                    } catch (Exception ee) {
                        showStatusText(ee.getMessage());
                    }
                }
            }

            // Applet should always get itself
            Applet a = getThisApplet();
            if (a != null && !applets.contains(a)) {
                applets.add(a);
            }
            return Collections.enumeration(applets);
        }

        public void setStream(String name, InputStream is) throws IOException {
            HashMap streamMap = (HashMap) streamStore.get(getCodeBase());

            if (streamMap == null) {
                streamMap = new HashMap();
                streamStore.put(getCodeBase(), streamMap);
            }

            synchronized(streamMap) {
                if (is != null) {
                    byte[] data = (byte[]) streamMap.get(name);

                    if (data == null) {
                        int streamSize = is.available();
                        if(streamSize < PERSIST_STREAM_MAX_SIZE) {
                            data = new byte[streamSize];
                            is.read(data, 0, streamSize);
                            streamMap.put(name, data);
                        } else {
                            throw new IOException("Stream size exceeds the maximum limit");
                        }
                    } else {
                        //If PeristStream already exists with the same name,
                        //replace the older byte stream by the newer one
                        streamMap.remove(name);
                        setStream(name, is);
                    }
                } else {
                    //remove the persist stream from the map
                    streamMap.remove(name);
                }
            }
        }

        public InputStream getStream(String name) {
            ByteArrayInputStream bAIS = null;
            HashMap streamMap = (HashMap) streamStore.get(getCodeBase());

            if (streamMap != null) {
                synchronized(streamMap) {
                    byte[] data = (byte[]) streamMap.get(name);

                    if (data != null)
                        bAIS = new ByteArrayInputStream(data);
                }
            }
            return bAIS;
        }

        public Iterator/*<String>*/ getStreamKeys() {
            Iterator iter = null;
            HashMap streamMap = (HashMap) streamStore.get(getCodeBase());
            if (streamMap != null) {
                synchronized(streamMap) {
                    iter = streamMap.keySet().iterator();
                }
            }
            return iter;
        }

        public void showDocument(URL url) {
            getAppletExecutionContext().showDocument(url);
        }

        public void showDocument(URL url, String target) {
            getAppletExecutionContext().showDocument(url, target);
        }

        public void showStatus(String status) {
            getAppletExecutionContext().showStatus(status);
        }

        public netscape.javascript.JSObject getJSObject() {
            return getAppletExecutionContext().getJSObject(getManager());
        }

        public netscape.javascript.JSObject getOneWayJSObject() {
            return getAppletExecutionContext().getOneWayJSObject(getManager());
        }

        //----------------------------------------------------------------------
        // Trampoline routines allowing the Plugin2Manager to be more eagerly GCd
        //

        private Plugin2Manager getManager() {
            return (Plugin2Manager) ref.get();
        }

        private URL getCodeBase() {
            Plugin2Manager mgr = getManager();
            if (mgr == null) {
                return null;
            }
            return mgr.getCodeBase();
        }

        private URL getDocumentBase() {
            Plugin2Manager mgr = getManager();
            if (mgr == null) {
                return null;
            }
            return mgr.getDocumentBase();
        }

        private Applet2ExecutionContext getAppletExecutionContext() {
            Plugin2Manager mgr = getManager();
            if (mgr == null) {
                return null;
            }
            return mgr.getAppletExecutionContext();
        }

        private void showStatusText(String msg) {
            Plugin2Manager mgr = getManager();
            if (mgr != null) {
                mgr.showStatusText(msg);
            }
        }

        private boolean checkConnect(String sourceHostName, String targetHostName)
            throws Exception {
            Plugin2Manager mgr = getManager();
            if (mgr != null) {
                return mgr.checkConnect(sourceHostName, targetHostName);
            }
            return false;
        }

        private Applet getThisApplet() {
            Plugin2Manager mgr = getManager();
            if (mgr == null) {
                return null;
            }
            return mgr.getApplet();
        }
    }

    /*
     * <p>
     * Check applet connection rights.
     * </p>
     * @param sourceHostName host originating the applet
     * @param targetHostName host the applet requested a connection to
     * 
     * @return true if the applet originating from sourceHostName can make
     * a connection to targetHostName
     *
     */
    private boolean checkConnect(String sourceHostName, String targetHostName)
        throws Exception
    {
        SocketPermission panelSp = new SocketPermission(sourceHostName,
                                                        "connect");
        SocketPermission sp = new SocketPermission(targetHostName,
                                                   "connect");
        if (panelSp.implies(sp)) {
            return true;
        }
        return false;
    }

    //----------------------------------------------------------------------
    // Loading of jar files
    //

    protected abstract void loadJarFiles() throws ExitException ;

    //----------------------------------------------------------------------
    // Relauch the applet in an earlier jvm version requested
    //

    protected abstract void appletSSVRelaunch() throws JRESelectException ;

    //----------------------------------------------------------------------
    // Continue to lauch the applet in current jvm if vm args satisfies
    //

    protected abstract void checkRunningJVMArgsSatisfying() throws JRESelectException ;

    //----------------------------------------------------------------------
    // Creation of the applet
    //

    protected Applet createApplet() throws ClassNotFoundException, IllegalAccessException, ExitException,
                                           JRESelectException, IOException, InstantiationException {
        long t0 = DeployPerfUtil.put(0, "Plugin2Manager.createApplet() - BEGIN");
        final String serName = getSerializedObject();
        final String code = getCode();

        final Plugin2ClassLoader loader = getAppletClassLoader();

        DeployPerfUtil.put("Plugin2Manager.createApplet() - post getAppletClassLoader()");

        if ( _INJECT_EXCEPTION_CREATEAPPLET ) {
            IOException e = new IOException("INJECT_PLUGIN2MANAGER_EXCEPTION_CREATEAPPLET");
            throw e;
        }

        if ( _INJECT_CREATEAPPLET_NULL ) {
            System.out.println("INJECT_PLUGIN2MANAGER_CREATEAPPLET_NULL");
            return null;
        }

        if (code != null && serName != null) {
            System.err.println(amh.getMessage("runloader.err"));
            throw new InstantiationException("Either \"code\" or \"object\" should be specified, but not both.");
        }
        if (code == null && serName == null) {
            return null; // callee sets the error state
        }

        Applet applet = null;

        final Error[] errorBox = new Error[1];
        errorBox[0] = null;

        if (code != null) {
            final Class cls = loader.loadCode(code);
            DeployPerfUtil.put("Plugin2Manager.createApplet() - post loader.loadCode()");
            if (cls != null) {
		if (fireAppletSSVValidation()) {
		    appletSSVRelaunch();		   
		} else {
		    // check if the current running jvm args satisfying
		    // note: this only applies to JNLP applets that can add more args in JNLP files
		    checkRunningJVMArgsSatisfying();
		    
                    // Move the instantiation of the applet on to the EDT to avoid deadlocks
                    final Applet[] appletBox = new Applet[1];
                    Runnable r = new Runnable() {
                            public void run() {
                                try {
                                    appletBox[0] = (Applet) cls.newInstance();
                                } catch (InstantiationException e) {
                                    throw new RuntimeException(e);
                                } catch (IllegalAccessException e) {
                                    throw new RuntimeException(e);
                                } catch (Error err) {
                                    errorBox[0] = err;
                                }
                            }
                        };
                    Component parent = getAppletParentContainer();		    
                    if (parent != null) {
                        runOnEDT(parent, r);
                    } else {
                        r.run();
                    }

                    applet = appletBox[0];
		    DeployPerfUtil.put("Plugin2Manager.createApplet() - created applet instance");
		}
            }

        } else {
            // Don't use this code path if the vm is insecure
            if (!isSecureVM) return applet;

            // serName is not null;
            // Move the instantiation of the applet on to the EDT to avoid deadlocks
            final Applet[] appletBox = new Applet[1];
            Runnable r = new Runnable() {
                    public void run() {
                        try {
			    appletBox[0] = createSerialApplet(loader, serName);
                            doInit = false; // skip over the first init
                        } catch (ClassNotFoundException e) {
                            throw new RuntimeException(e);
                        } catch (IOException e) {
                            throw new RuntimeException(e);
                        } catch (Error err) {
                            errorBox[0] = err;
                        }
                    }
                };
            Component parent = getAppletParentContainer();
            if (parent != null) {
                runOnEDT(parent, r);
            } else {
                r.run();
            }
            applet = appletBox[0];
            
            DeployPerfUtil.put("Plugin2Manager.createApplet() - post: secureVM .. serialized .. ");
        }

        // if an error (such as NoClassDefFoundError) has occurred,
        // throws a RuntimeException because the applet instance
        // is likely to be null and we can't proceed further
        if (errorBox[0] != null) {
            throw new RuntimeException(errorBox[0]);
        }

        // null check on the applet instance
        if (applet == null) {
            // caller handles null applet instance
            return null;
        }

        // Determine the JDK level that the applet targets.
        // This is critical for enabling certain backward
        // compatibility switch if an applet is a JDK 1.1 
        // applet. [stanley.ho]
        findAppletJDKLevel(applet);        
        
        if (shouldStop) {
            setErrorOccurred("death"); // aborted
            applet = null;
            if (DEBUG) {
                showStatusText("Applet ID " + appletID + " killed during creation");
            }
            showAppletStatus("death");
            synchronized (stopLock) {
                stopSuccessful = true;
                stopLock.notifyAll();
            }
            return null;
        }

        DeployPerfUtil.put(t0, "Plugin2Manager.createApplet() - END");
        return applet;
    }

    // Helper method to check if applet class is signed
    // We check whether All Permission is granted for this applet
    // to determine if it is signed
    protected boolean isAppletSigned(Class cls) {
        boolean ret = false;
	CodeSource cs = cls.getProtectionDomain().getCodeSource();
	if ((cs != null) && (cs.getCertificates() != null)) {
	    try {
		ret = TrustDecider.isAllPermissionGranted(cs) != TrustDecider.PERMISSION_DENIED;
	    } catch (Exception e) {
	    }
	}
        return ret;
    }

    //----------------------------------------------------------------------
    // Backward compatibility for older applets

    /**
     * Determine JDK level of an applet.
     */
    protected void findAppletJDKLevel(Applet applet) {  
        // To determine the JDK level of an applet, the
        // most reliable way is to check the major version
        // of the applet class file.

        // synchronized on applet class object, so calling from
        // different instances of the same applet will be 
        // serialized.
        Class appletClass = applet.getClass();
        
        synchronized(appletClass)  {        
            // Determine if the JDK level of an applet has been 
            // checked before.
            Boolean jdk11Target = loader.isJDK11Target(appletClass);
            Boolean jdk12Target = loader.isJDK12Target(appletClass);

            // if applet JDK level has been checked before, retrieve
            // value and return.
            if (jdk11Target != null || jdk12Target != null) {
                jdk11Applet = (jdk11Target == null) ? false : jdk11Target.booleanValue();
                jdk12Applet = (jdk12Target == null) ? false : jdk12Target.booleanValue();
                return;
            }
        
            String name = appletClass.getName();

            // first convert any '.' to '/'
            name = name.replace('.', '/');
        
            // append .class
            final String resourceName = name + ".class";
        
            InputStream is = null;
            byte[] classHeader = new byte[8];    
        
            try {
                is = (InputStream) java.security.AccessController.doPrivileged(
                                                                               new java.security.PrivilegedAction() {
                                                                                   public Object run() {
                                                                                       return loader.getResourceAsStream(resourceName);
                                                                                   }
                                                                               });     

                // Read the first 8 bytes of the class file
                int byteRead = is.read(classHeader, 0, 8);
                is.close();

                // return if the header is not read in entirely 
                // for some reasons. 
                if (byteRead != 8)
                    return;
            } catch (IOException e) {
                return;
            } catch (NullPointerException e) {
                return;
            }

            // Check major version in class file header
            int major_version = readShort(classHeader, 6);
        
            // Major version in class file is as follows:
            //   45 - JDK 1.1
            //   46 - JDK 1.2
            //   47 - JDK 1.3
            //   48 - JDK 1.4
            //   49 - JDK 1.5    
            if (major_version < 46) 
                jdk11Applet = true;
            else if (major_version == 46)
                jdk12Applet = true;         

            // Store applet JDK level in AppContext for later lookup,
            // e.g. page switch.
            loader.setJDK11Target(appletClass, jdk11Applet);
            loader.setJDK12Target(appletClass, jdk12Applet);
        }
    }

    /**
     * Return true if applet is targeted to JDK 1.1.
     */
    protected boolean isJDK11Applet() {
        return jdk11Applet;
    }
    
    /**
     * Return true if applet is targeted to JDK1.2.
     */
    protected boolean isJDK12Applet() {
        return jdk12Applet;
    }

    /**
     * Read short from byte array.
     */
    protected static int readShort(byte[] b, int off) {
        int hi = readByte(b[off]);
        int lo = readByte(b[off + 1]);
        return (hi << 8) | lo;
    }
    
    protected static int readByte(byte b) {
        return ((int)b) & 0xFF;
    }   

    //----------------------------------------------------------------------
    // Focus-related helper routines
    //

    /**
     * Gets most recent focus owner component associated with the given window.
     * It does that without calling Window.getMostRecentFocusOwner since it
     * provides its own logic contradicting with setDefaultFocus. Instead, it 
     * calls KeyboardFocusManager directly.
     */
    private Component getMostRecentFocusOwnerForWindow(Window w) {
        Method meth = (Method)AccessController.doPrivileged(new PrivilegedAction() {
                public Object run() {
                    Method meth = null;
                    try {
                        meth = KeyboardFocusManager.class.getDeclaredMethod("getMostRecentFocusOwner", new Class[] {Window.class});
                        meth.setAccessible(true);
                    } catch (Exception e) {
                        // Must never happen
                        e.printStackTrace();
                    }
                    return meth;
                }
            });
        if (meth != null) {
            // Meth refers to static method
            try {
                return (Component)meth.invoke(null, new Object[] {w});
            } catch (Exception e) {
                // Must never happen
                e.printStackTrace();
            }
        }
        // Will get here if exception was thrown or meth is null
        return w.getMostRecentFocusOwner();
    }

    /*
     * Fix for BugTraq ID 4041703.
     * Set the focus to a reasonable default for an Applet.
     */
    private void setDefaultFocus() {
        Component toFocus = null;
        Container parent = appletParentContainer;

        if (parent != null) {
            if (parent instanceof Window) {
                toFocus = getMostRecentFocusOwnerForWindow((Window)parent);
                if (toFocus == parent || toFocus == null) {
                    toFocus = parent.getFocusTraversalPolicy().
                        getInitialComponent((Window)parent);
                }
            } else if (parent.isFocusCycleRoot()) {
                toFocus = parent.getFocusTraversalPolicy().
                    getDefaultComponent(parent);
            }
        }

        if (toFocus != null) {
            if (parent instanceof EmbeddedFrame) {
                ((EmbeddedFrame)parent).synthesizeWindowActivation(true);
            }
            // EmbeddedFrame might have focus before the applet was added.
            // Thus after its activation the most recent focus owner will be
            // restored. We need the applet's initial focusabled component to
            // be focused here.
            toFocus.requestFocusInWindow();
        }
    }

    //----------------------------------------------------------------------
    // Displaying of applet status information
    //

    protected void showStatusText(String msg) {
        showStatusText(msg, false);
    }

    private void showStatusText(String msg, boolean isError) {
        // Message should NOT be displayed in the browser status bar since 
        // it is "unprofessional"
        // Display in the java console instead
        Trace.msgPrintln(msg, null, TraceLevel.BASIC);
        if (isError) {
            setErrorOccurred(msg);
        }
    }

    /**
     * Status line. Called by the AppletPanel to provide
     * feedback on the Applet's state.
     */
    protected void showAppletStatus(String status) {
        try {
            showStatusText(amh.getMessage(status));
        } catch (Exception e) {
            Trace.ignoredException(e);
            showStatusText(status);
        }
    }

    protected void showAppletStatus(String status, Object arg) {
        try {
            showStatusText(amh.getMessage(status, arg));
        } catch (Exception e) {
            Trace.ignoredException(e);
            showStatusText(status);
        }
    }
    protected void showAppletStatus(String status, Object arg1, Object arg2) {
        try {
            showStatusText(amh.getMessage(status, arg1, arg2));
        } catch (Exception e) {
            Trace.ignoredException(e);
            showStatusText(status);
        }
    }

    protected void showAppletErrorStatus(String status) {
        try {
            showStatusText(amh.getMessage(status), true);
        } catch (Exception e) {
            Trace.ignoredException(e);
            showStatusText(status, true);
        }
    }

    protected void showAppletErrorStatus(String status, Object arg) {
        try {
            showStatusText(amh.getMessage(status, arg), true);
        } catch (Exception e) {
            Trace.ignoredException(e);
            showStatusText(status, true);
        }
    }

    /**
     * Called by the AppletPanel to print to the log.
     */
    protected void showAppletLog(String msg) {
        try {
            System.out.println(amh.getMessage(msg));
        } catch (Exception e) {
            Trace.ignoredException(e);
            System.out.println(msg);
        }
    }

    protected void showAppletLog(String msg, Object arg) {
        try {
            System.out.println(amh.getMessage(msg, arg));
        } catch (Exception e) {
            Trace.ignoredException(e);
            System.out.println(msg);
        }
    }

    /**
     * Called to provide feedback when an exception has happened.
     *
     * This will call 'setErrorOccurred(Throwable)' as well,
     * and therefor set the error state to true;
     */
    protected void showAppletException(Throwable t) {
        showAppletException(t, false);
    }

    protected void showAppletException(Throwable t, boolean forceNoUI) {
        setErrorOccurred(t);
        t.printStackTrace();
        Trace.msgPrintln("exception", new Object[]{t.toString()});
        if (fShowException && !forceNoUI) {
            try {
                Trace.printException(t);
            } catch(Exception e) {}
        }

        if (grayBoxPainter == null) {
            Container c = getAppletParentContainer();
            if (c != null) {
                Trace.println("setting up a new GraBoxPainter for Error", 
                    TraceLevel.BASIC);
                setupGrayBoxPainter();
            }
        }
        if (grayBoxPainter != null) {
            grayBoxPainter.showLoadingError();
        }
    }

    /** An indication that a deep error has
        occurred while starting an applet, i.e. init(), start(), ... 
        No notification is send here.
    */
    protected void setErrorOccurred(String message, Throwable t) {
        errorOccurred = true;
        errorMessage  = message;
        errorException = t;
    }

    protected void setErrorOccurred(String message) {
        errorOccurred = true;
        errorMessage  = message;
        errorException = null;
    }

    protected void setErrorOccurred(Throwable t) {
        errorOccurred = true;
        errorMessage  = t.toString();
        errorException = t;
    }

    /**
     * Method to get an internationalized string from the Activator resource.
     */
    protected static String getMessage(String key) {
        return ResourceManager.getMessage(key);
    }

    protected String getWaitingMessage() {
        if (hasErrorOccurred()) {
            return getMessage("failed");
        } else {
            MessageFormat mf = new MessageFormat(getMessage("loading"));
            return mf.format(new Object[] {getHandledType()});
        }
    }

    protected String getHandledType() {
        return getMessage("java_applet");
    }

    //----------------------------------------------------------------------
    // AWT helper routines
    //

    protected void runOnEDT(Component comp, Runnable runnable) {
        if (comp == null) {
            // Bug in above code; should not happen
            Exception e = new Exception("comp is null");
            e.printStackTrace();
            return;
        }

        try {
            DeployAWTUtil.invokeAndWait(comp, runnable);
        } catch(InterruptedException ie) {
            Trace.ignoredException(ie);
        } catch(InvocationTargetException ite) {
            showAppletException(ite);
        }

    }

    //----------------------------------------------------------------------
    // AppContext-related helper routines
    //

    private static synchronized List/*<WeakReference<Plugin2Manager>>*/ getPlugin2ManagerList(AppContext context) {
        List/*<WeakReference<Plugin2Manager>>*/ managerList =
            (List) context.get(APPCONTEXT_APPLET2MANAGER_LIST_KEY);
        if (managerList == null) {
            managerList = new ArrayList();
            context.put(APPCONTEXT_APPLET2MANAGER_LIST_KEY, managerList);
        }
        return managerList;
    }

    /** Registers this Plugin2Manager in the given AppContext. NOTE
        that this is a helper for the implementation of
        getCurrentManager() which has some semantic problems. */
    protected void registerInAppContext(AppContext context) {
        // NOTE: with the introduction of the class loader cache, we
        // now need to maintain a list of the Plugin2Managers in a
        // given AppContext

        // NOTE: the use of WeakReferences is a workaround for 6616095
        List/*<WeakReference<Plugin2Manager>>*/ managerList = getPlugin2ManagerList(context);
        synchronized (managerList) {
            managerList.add(new WeakReference(this));
        }
    }

    /** Fetches the Plugin2Manager from the current thread's ThreadLocal
     *  All Threads started from applet and LiveConnect Worker thread has
     *  its Plugin2Manager stored in a ThreadLocal.
     *  (see {@link #getFromAppContext getFromAppContext})
     *
     */
    protected static Plugin2Manager getFromThreadLocal() {
        return (Plugin2Manager) currentManagerThreadLocal.get();
    }

    /** Fetches the Plugin2Manager (or a random one) from the current
        AppContext. NOTE that this is a helper for the implementation
        of getCurrentManager() which has some semantic problems. */
    protected static Plugin2Manager getFromAppContext() {
        // NOTE: the use of WeakReferences is a workaround for 6616095
        List/*<WeakReference<Plugin2Manager>>*/ managerList =
            getPlugin2ManagerList(AppContext.getAppContext());
        synchronized (managerList) {
            if (managerList.isEmpty()) {
                return null;
            }
            Plugin2Manager anyManager = null;
            for (Iterator iter = managerList.iterator(); iter.hasNext(); ) {
                WeakReference ref = (WeakReference) iter.next();
                Plugin2Manager manager = (Plugin2Manager) ref.get();
                // Do not return a disconnected manager in the case
                // where we have a live manager in the same AppContext
                // since it is being fetched to make queries of the
                // browser that it can not satisfy
                if (manager != null) {
                    if (manager.isDisconnected()) {
                        anyManager = manager;
                    } else {
                        return manager;
                    }
                }
            }
            if (anyManager != null) {
                // We probably meant to return the disconnected manager
                // This is still generally better than returning the
                // default execution context
                return anyManager;
            }
        }
        return null;
    }

    protected void unregisterFromAppContext(AppContext appContext) {
        if (appContext == null)
            return;

        // NOTE: the use of WeakReferences is a workaround for 6616095
        List/*<WeakReference<Plugin2Manager>>*/ managerList = getPlugin2ManagerList(appContext);
        synchronized (managerList) {
            for (Iterator iter = managerList.iterator(); iter.hasNext(); ) {
                WeakReference ref = (WeakReference) iter.next();
                Plugin2Manager manager = (Plugin2Manager) ref.get();
                if (manager == null || manager == this) {
                    iter.remove();
                    break;
                }
            }
        }
    }

    // This is only present to be in the same part of the file as the above routines
    private boolean isInSameAppContextImpl(Plugin2Manager manager) {
        if (this == manager)
            return true;
        AppContext ctx = appletAppContext;
        if (ctx == null)
            return false;
        List/*<WeakReference<Plugin2Manager>>*/ managerList =
            getPlugin2ManagerList(ctx);
        synchronized(managerList) {
            for (Iterator iter = managerList.iterator(); iter.hasNext(); ) {
                WeakReference ref = (WeakReference) iter.next();
                if (ref.get() == manager)
                    return true;
            }
        }
        return false;
    }

    //----------------------------------------------------------------------
    // Helper routine for clean AppContext shutdown
    //

    private static final String APPCONTEXT_JAPPLET_USED_KEY = "JAppletUsedKey";
    
    private static boolean isSubclassOf(Object o, String className) {
        if (o == null) {
            return false;
        }
        Class c = o.getClass();
        while (c != null) {
            if (c.getName().equals(className)) {
                return true;
            }
            c = c.getSuperclass();
        }
        return false;
    }

    //----------------------------------------------------------------------
    // String processing helper routines
    //

    protected static String trimWhitespace(String str) {
        if (str == null)
            return str;
        
	// This is to be backward compatible with the old plugin
        StringBuffer buffer = new StringBuffer();
        
        for (int i=0; i < str.length(); i++){
	    char c = str.charAt(i);
	    
	    // Skip over whitespaces
	    if (c == '\n' || c == '\f' || c == '\r' || c == '\t')
		continue;
	    else
		buffer.append(c);
	}
        
        // Trim whitespaces on both ends of the strings
        return buffer.toString().trim();
    }

    protected static String[] splitJarList(String jarList, boolean trimOptionString) {
        if (jarList == null)
            return null;

        String[] strs = jarList.split(",");
	ArrayList tmpList = new ArrayList();
        for (int i = 0; i < strs.length; i++) {
            String str = strs[i].trim();
            if (trimOptionString) {
                int idx = str.indexOf(';');
                if (idx >= 0) {
                    str = str.substring(0, idx);
                }
            }
            if (str != null && !str.equals("")) {
		tmpList.add(str);
	    }
        }

        return (String[])tmpList.toArray(new String[0]);
    }

    protected static String[] splitOptionString(String jarName) {
        int idx = jarName.indexOf(';');
        if (idx < 0) {
            return new String[] { jarName, null };
        }
	StringTokenizer strTok = new StringTokenizer(jarName, ";");
	if (strTok.countTokens() >= 3) {
	    // we have a preload and version number specified along with jar name
	    // The preload and version strings can be specified in any order after
	    // jar file name - Return the version string as the last element of array here.
	    String fileName = strTok.nextToken();
	    String str2 = strTok.nextToken();
	    String str3 = strTok.nextToken();
	    if (str2.toLowerCase().indexOf("preload") != -1)
	    	return new String[] {fileName, str2, str3};
	    else 
	    	return new String[] {fileName, str3, str2};
	} else {
            return new String[] {
                jarName.substring(0, idx),
                jarName.substring(idx + 1)
            };
        }
    }

    protected static String buildJarList(String[] jars) {
        if (jars == null)
            return null;

        StringBuffer buf = null;
        boolean needComma = false;
        for (int i = 0; i < jars.length; i++) {
            String str = jars[i];
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


    private static final boolean _INJECT_EXCEPTION_CREATEAPPLET;
    private static final boolean _INJECT_CREATEAPPLET_NULL;
    private static final boolean _INJECT_DELAY_APPLETLOADED;
    private static final boolean _INJECT_NEVER_APPLETLOADED;

    static {
        boolean exceptionCreateApplet=false;
        boolean createAppletNull=false;
        boolean deplayAppletLoaded=false;
        boolean neverAppletLoaded=false;

        String env = SystemUtil.getenv("JPI_PLUGIN2_INJECT_PLUGIN2MANAGER");
        if(null!=env) {
            System.out.println("JPI_PLUGIN2_INJECT_PLUGIN2MANAGER: "+env);
            StringTokenizer tok = new StringTokenizer(env);
            try {
                while (tok.hasMoreTokens()) {
                    String tmp = new String(tok.nextToken());
                    if(null!=tmp) {
                        if(!exceptionCreateApplet)
                            exceptionCreateApplet = "EXCEPTION_CREATEAPPLET".equals(tmp);
                        if(!createAppletNull)
                            createAppletNull = "CREATEAPPLET_NULL".equals(tmp);
                        if(!deplayAppletLoaded)
                            deplayAppletLoaded = "DELAY_APPLETLOADED".equals(tmp);
                        if(!neverAppletLoaded)
                            neverAppletLoaded = "NEVER_APPLETLOADED".equals(tmp);
                    }
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
            System.out.println("\tEXCEPTION_CREATEAPPLET: "+exceptionCreateApplet);
            System.out.println("\tCREATEAPPLET_NULL: "+createAppletNull);
            System.out.println("\tDELAY_APPLETLOADED: "+deplayAppletLoaded);
            System.out.println("\tNEVER_APPLETLOADED: "+neverAppletLoaded);
        }
        _INJECT_EXCEPTION_CREATEAPPLET = exceptionCreateApplet;
        _INJECT_CREATEAPPLET_NULL = createAppletNull;
        _INJECT_DELAY_APPLETLOADED = deplayAppletLoaded;
        _INJECT_NEVER_APPLETLOADED = neverAppletLoaded;
    }

    private static Applet createSerialApplet(final Plugin2ClassLoader loader, final String serName)
                        throws ClassNotFoundException, IOException
    {
        try {
            AccessControlContext loaderACC = loader.getACC();
            if (loaderACC == null) {
                throw new SecurityException();
            }
            return (Applet)AccessController.doPrivileged(
                new PrivilegedExceptionAction() {
                    public Object run() throws IOException, PrivilegedActionException
                    {
                        // Find resource and open stream in ACC of loader.
                        sun.misc.Resource res = loader.getResourceAsResource(serName);
                        final InputStream is = new ByteArrayInputStream( res.getBytes());

                        // Use stream in ACC of itself.
                        AccessControlContext resAcc = loader.getACC(res);
                        if (resAcc == null) {
                            throw new SecurityException();
                        }
                        return AccessController.doPrivileged(
                            new PrivilegedExceptionAction() {
                                public Object run() throws IOException, ClassNotFoundException
                                {
                                    ObjectInputStream ois = new Applet2ObjectInputStream(is, loader);
                                    Object serObject = ois.readObject();
        			    return (Applet)serObject;
                                }
                            },
                            resAcc
                        );
                    }
                },
                loaderACC
            );
        } catch (PrivilegedActionException exc) {
	    Throwable cause = exc.getCause();
            if (cause instanceof PrivilegedActionException) {
                // Unwrap from inner doPrivileged.
                cause = cause.getCause();
            }
            if (cause instanceof IOException) {
                throw (java.io.IOException)cause;
            } else if (cause instanceof ClassNotFoundException) {
                throw (ClassNotFoundException)cause;
            } else {
                throw new Error("Undeclared exception", cause);
            }
        }
    }

}
