/*
 * @(#)Applet2Manager.java	1.48 10/03/24
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
import java.io.File;
import java.io.FilenameFilter;
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
import java.security.PrivilegedAction;
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

// Should consider breaking the following dependencies
import com.sun.deploy.cache.Cache;
import com.sun.deploy.config.Config;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.URLUtil;
import com.sun.deploy.security.TrustDecider;
import com.sun.deploy.resources.ResourceManager;
import com.sun.javaws.exceptions.JRESelectException;
import com.sun.javaws.exceptions.ExitException;
import com.sun.javaws.jnl.JREDesc;

import sun.plugin2.util.ParameterNames;

// We need a notion of the command-line arguments that were used to
// start this JVM instance to decide whether we need to relaunch the
// applet (or, more precisely, whether to fire an event indicating the
// need to relaunch the applet)
import com.sun.deploy.util.JVMParameters;

// Should break the following dependencies
import com.sun.deploy.ui.UIFactory;
import com.sun.deploy.util.DeployAWTUtil;
import sun.plugin.JavaRunTime;
import sun.plugin.cache.JarCacheUtil;
import sun.plugin.util.GrayBoxPainter;

// Probably can't break the following dependency without completely
// replacing the JSObject implementation
import sun.plugin.javascript.JSContext;

/** The main class which manages the creation and lifecycle of an
    applet. A new Plugin2Manager instance should be created for each
    applet being viewed. */

public class Applet2Manager extends Plugin2Manager {
    // Data members associated with the list of jars, version numbers, etc.
    private final static String VERSION_TAG = "version=";
    private final static String PRELOAD = "preload";
    private boolean initializedJarVersionMap;
    private Map jarVersionMap = new HashMap();
    private Map preloadJarMap = new HashMap();
    private List newStyleJarList = new ArrayList();

    // If this Applet2Manager is using the class loader caching
    // mechanism (on by default, but opt-out on a per-applet basis),
    // this is the class loader cache and entry
    private Applet2ClassLoaderCache classLoaderCache;
    // NOTE: do not access this variable directly, only through getClassLoaderCacheEntry()
    private volatile Applet2ClassLoaderCache.Entry classLoaderCacheEntry;
    private volatile String classLoaderCacheKey;

    // Support for the legacy applet lifecycle
    // Note that this is only used if this Applet2Manager is actually
    // using the legacy lifecycle (but we don't want the outside world
    // to have to know about this and change its behavior)
    private Applet2ManagerCache instanceCache;
    private boolean usingLegacyLifeCycle;
    // For preserving the original hashed parameters before any changes during execution
    private String legacyCacheKey;
    // For transmitting the stop listener between stop() and destroy()
    private Applet2StopListener legacyStopListener;

    //----------------------------------------------------------------------
    // Manager specialisation 
    //

    /** Creates a new Applet2Manager. The "classLoaderCache" argument
        is optional. If it is specified, the class loader cache will
        be consulted to provide the class loader for this manager.
        Otherwise, a new class loader, AppContext and ThreadGroup will
        be used for this applet. The "instanceCache" argument is not
        optional and is used to cache this Applet2Manager instance if
        it is using the legacy lifecycle for applets. */
    public Applet2Manager(Applet2ClassLoaderCache classLoaderCache,
                          Applet2ManagerCache instanceCache, 
                          boolean relaunched) {
        super(relaunched);
        this.classLoaderCache = classLoaderCache;
        this.instanceCache = instanceCache;
    }

    public void setAppletExecutionContext(Applet2ExecutionContext context) {
        super.setAppletExecutionContext(context);

        // Compute whether we're using the legacy applet lifecycle
        String lifecycle = getParameter("legacy_lifecycle");
        if (lifecycle != null && lifecycle.equalsIgnoreCase("true")) {
            usingLegacyLifeCycle = true;
            legacyCacheKey = instanceCache.getCacheKey(getDocumentBase().toString(),
                                                       context.getAppletParameters());
        } else {
            usingLegacyLifeCycle = false;
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
                Applet2ClassLoaderCache.Entry entry = getClassLoaderCacheEntry();
                if (entry != null) {
                    // Using class loader caching -- pick up the cached loader
                    loader = entry.getClassLoader();
                    if (loader == null) {
                        throw new InternalError("Error during bootstrapping of ClassLoader");
                    }
                    // when we re-use cache loader from classloader cache, we need to
                    // check whether we allow codebase recursive read again
                    setupClassLoaderCodebaseRecursiveRead(loader);
                } else {
                    // Not using class loader caching -- create a new loader
                    loader = getOrCreatePlugin2ClassLoader();
                }
            }
      
            return loader;
        }
    }

    /** Fetch and/or create the ThreadGroup associated with this
        applet's execution. */
    public ThreadGroup getAppletThreadGroup() {
        synchronized(this) {
            if (appletThreadGroup == null) {
                Applet2ClassLoaderCache.Entry entry = getClassLoaderCacheEntry();
                if (entry != null) {
                    // Using class loader caching -- pick up the shared ThreadGroup
                    appletThreadGroup = entry.getThreadGroup();
                    if (appletThreadGroup == null) {
                        throw new InternalError("Error during bootstrapping of ThreadGroup");
                    }
                } else {
                    appletThreadGroup = getOrCreateAppletThreadGroup();
                }
            }

            return appletThreadGroup;
        }
    }

    /** Fetch and/or create the AppContext associated with this applet's
        execution. */
    public AppContext getAppletAppContext() {
        synchronized(this) {
            if (appletAppContext == null) {
                Applet2ClassLoaderCache.Entry entry = getClassLoaderCacheEntry();
                if (entry != null) {
                    // Using class loader caching -- pick up the shared AppContext
                    appletAppContext = entry.getAppContext();
                    if (appletAppContext == null) {
                        throw new InternalError("Error during bootstrapping of AppContext");
                    }
                } else {
                    appletAppContext = getOrCreateAppletAppContext();
                }

                // Immediately put ourselves into the AppContext
                registerInAppContext(appletAppContext);
            }

            return appletAppContext;
        }
    }

    /** Obtains a key in the form of a String which uniquely identifies an applet.
     *  The key contains document base, codebase and the list of jars in the archive tag.
     */
    public String getAppletUniqueKey() {
	String keyString = "|";

	URL docBase = getDocumentBase();
	if (docBase != null) {
	    keyString += docBase.toString();
	}
	keyString += "|";

	URL codeBase = getCodeBase();
	if (codeBase != null) {
	    keyString += codeBase.toString();
	}
	keyString += "|";

	String jarFilesList = getJarFiles();
	if (jarFilesList != null) {
	    keyString += jarFilesList;
	}
	keyString += "|";

	return keyString;
    }

    /** This is a hack to work around the problem that we change the
        width and height during the applet's execution and need to
        preserve the original values that come over the wire */
    public String getLegacyLifeCycleCacheKey() {
        if (!usingLegacyLifeCycle) {
            throw new IllegalStateException("Only legal for applets using the legacy lifecycle");
        }
        return legacyCacheKey;
    }

    /** Destroys this Applet2Manager, calling destroy() on its
        contained applet and performing all associated cleanups. This
        should only be called if the legacy applet lifecycle is in use
        for this Applet2Manager. In general this is called
        automatically by the Applet2ManagerCache and should not be
        called by end users. */
    public void destroy() {
        if (!usingLegacyLifeCycle) {
            throw new IllegalStateException("May only call destroy() for applets using the legacy lifecycle");
        }

        // We replicate a small amount of code from the ordinary
        // AppletExecutionRunnable and stop() to offer the applet a
        // similar environment in which to call destroy() (namely, on
        // a worker thread in the applet's AppContext)

        final long stopTimeout = getAppletStopTimeout();
	final Plugin2ClassLoader loader = getAppletClassLoader();
        ThreadGroup group = getAppletThreadGroup();
        String name = "thread destroy applet-" + getCode();
        final Thread destroyThread =
            new Thread(group,
                       new Runnable() {
                           public void run() {
                               // Destroy the applet
                               try {
                                   getApplet().destroy();
                               } catch (Throwable t) {
                                   invalidateClassLoaderCacheEntry();
                                   t.printStackTrace();
                               }

                               synchronized(stopLock) {
                                   stopLock.notifyAll();
                               }
                           }
                       },
                       name);
        // set the context class loader for this thread
        AccessController.doPrivileged(new PrivilegedAction() {
                public Object run() {
                    destroyThread.setContextClassLoader(loader);
                    return null;
                }
            });
        long startTime = System.currentTimeMillis();
        synchronized(stopLock) {
            destroyThread.start();
            try {
                stopLock.wait(stopTimeout);
            } catch (InterruptedException e) {
            }
        }

        // Take ourselves out of the AppContext, regardless of whether
        // we actually dispose it at this point
        unregisterFromAppContext(appletAppContext);

        cleanupAppContext(startTime, stopTimeout, legacyStopListener);
    }

    protected void shutdownAppContext(AppContext context,
                                      long startTime,
		                      long timeout,
                                      Applet2StopListener stopListener,
                                      boolean stopSuccessfulSoFar) {
        if (!usingLegacyLifeCycle) {
            super.shutdownAppContext(context, startTime, timeout, stopListener, stopSuccessfulSoFar);
        } else {
            legacyStopListener = stopListener;
            if (stopSuccessfulSoFar) {
                // stop was successful -- put myself in cache
                instanceCache.put(this);
                // Reset stop state
                synchronized (stopLock) {
                    shouldStop = false;
                    stopSuccessful = false;
                }
            } else {
                // stop was unsuccessful -- continue destroy process
                // and discard this applet instance
                destroy();
            }
        }
        
    }

    protected void cleanupAppContext(long startTime, long timeout, Applet2StopListener stopListener) {
        Applet2ClassLoaderCache cache = null;
        Applet2ClassLoaderCache.Entry entry = null;
        AppContext ac = null;
        synchronized (this) {
            ac = appletAppContext;
            appletAppContext = null;
            cache = classLoaderCache;
            classLoaderCache = null;
            entry = classLoaderCacheEntry;
            classLoaderCacheEntry = null;
        }

        long timeRemaining = timeout - (System.currentTimeMillis() - startTime);

        if (entry != null) {
            assert cache != null;
            // Class loader caching is in use. We don't know whether
            // we will actually be disposing the ThreadGroup and
            // AppContext at this point -- only the
            // Applet2ClassLoaderCache knows this.
            // Must hold the lock on this object before calling in
            synchronized(this) {
                cache.release(entry, this, stopListener, timeRemaining);
            }
        } else {
            destroyAppContext(ac, stopListener, timeRemaining);
        }

    }

    protected Plugin2ClassLoader newClassLoader()
    {
        final URL codebase = getCodeBase();
        Applet2ClassLoader res = Applet2ClassLoader.newInstance(codebase);
        if (isForDummyApplet()) {
            // Do not look up partial package names on the server
            res.setCodebaseLookup(false);
        }
        return res;
    }

    //----------------------------------------------------------------------
    // old-style jar versioning management
    //

    /*
     * initialize the jar version/preload map
     */
    protected synchronized void initJarVersionMap() {
        if (initializedJarVersionMap)
            return;

        initializedJarVersionMap = true;
        int i = 1;
        String archive_tag_value = getParameter("archive_" + i);
        if (archive_tag_value != null) {
            // process new style tags
            while (archive_tag_value != null) {               
                // process the archive tag string                               
                // parse the string and extract tokens separated by comma
                String[] jars = splitJarList(archive_tag_value, false);
                String jarName = null;
                String version = null;
                boolean preloadJar = false;
                for (int j = 0; j < jars.length; j++) {
                    String str = jars[j];
                    // first argument must be jar name
                    if (jarName == null) {
                        jarName = str;
                    } else {
                        str = str.toLowerCase();
                        if (str.startsWith(VERSION_TAG)) {
                            version = str.substring(VERSION_TAG.length());
                        } else if (str.equals(PRELOAD)) {
                            preloadJar = true;
                        }
                    }
                }
                
                if (jarName != null) {
                    if (preloadJar) {
                        preloadJarMap.put(jarName, version);
                    }
                    jarVersionMap.put(jarName, version);
                    newStyleJarList.add(jarName);
                }
                i++;
                archive_tag_value = getParameter("archive_" + i);
            }
        } else {
            // process old style tags
            String jpi_archive = getParameter("cache_archive");
            String jpi_version = getParameter("cache_version");
            String jpi_archive_ex = getParameter("cache_archive_ex");
            try {
                // use old style tags, cache_archive, cache_version and cache_archive_ex
                jarVersionMap = JarCacheUtil.getJarsWithVersion(jpi_archive, jpi_version,
                                                                jpi_archive_ex);
            } catch (Exception ex) {
                Trace.printException(ex, ResourceManager.getMessage("cache.error.text"),
                                     ResourceManager.getMessage("cache.error.caption"));
            }
            
            // Figure out which JAR files still need to be loaded.
            if (jpi_archive_ex != null) {
                String[] jarNames = splitJarList(jpi_archive_ex, false);
                for (int j = 0; j < jarNames.length; j++) {
                    String[] nameAndOption = splitOptionString(jarNames[j]);
                    if (nameAndOption.length > 1 && nameAndOption[1] != null &&
                        nameAndOption[1].toLowerCase().indexOf(PRELOAD) != -1) {
			String ver = null;
			if(nameAndOption.length > 2)
			    ver = nameAndOption[2];
                        preloadJarMap.put(nameAndOption[0], ver);
                    }
                }
            }
        }
    }

    private void storeJarVersionMapInAppContext() {
        // iterate the jarVersionMap and store into Applet AppContext      
        Iterator iter = jarVersionMap.keySet().iterator();
        while(iter.hasNext()) {
            String jarName = (String)iter.next();
            String jarVersion = (String)jarVersionMap.get(jarName);
            URL url = null;
            try {
                // this will resolve a/abc/../ to a/ in the url
                url = new URL(getCodeBase(), jarName);
            } catch (MalformedURLException mue) {
                // should not happen
                Trace.ignoredException(mue);
            }
            if (url != null) {
                appletAppContext.put(Config.getAppContextKeyPrefix() +
                                     url.toString(), jarVersion);
            }
        }
    }

    protected void setupAppletAppContext() {
        storeJarVersionMapInAppContext();
        super.setupAppletAppContext();
    }

    //----------------------------------------------------------------------
    // Loading of jar files
    //

    protected void loadJarFiles() throws ExitException {
        try{
            // pre-load jar files are loaded first                
            // FIXME: JarCacheUtil should not reference the HashMap class
            JarCacheUtil.preload(getCodeBase(), (HashMap) preloadJarMap);
                
        } catch(Exception ex) {
            Trace.printException(ex, ResourceManager.getMessage("cache.error.text"),
                                 ResourceManager.getMessage("cache.error.caption"));
        }

        try  {
            // Get a list of jar files
            String archive = getJarFiles();
            Applet2ClassLoader loader = (Applet2ClassLoader) getAppletClassLoader();

            // FIXME: locking code present in original elided in this version
            // due to elimination of class loader sharing
            {
                String fSep = File.separator;
                String appletDir = System.getProperty("java.home") + fSep + 
                    "lib" + fSep + "applet";
                loadLocalJarFiles(loader, appletDir);
                // Add the jar files from system wide untrusted directory
                if (Config.getOSName().equalsIgnoreCase("Windows")) {
                    String untrustDir = Config.getSystemHome() + fSep + "Lib" + 
                        fSep + "Untrusted";
                    loadLocalJarFiles(loader, untrustDir);
                }
            }    

            // If there are no JARs, this is easy.
            if (archive == null) {
                return;
            }
        
            // Figure out which JAR files still need to be loaded.

            // FIXME: need to revisit this -- the old logic which tries to
            // guard against re-loading of jar files (presumably related
            // to ClassLoader caching) is convoluted and this doesn't
            // match it

            String[] jars = splitJarList(archive, false);
            for (int i = 0; i < jars.length; i++) {
                loader.addJar(jars[i]);
            }
        } catch (Throwable t) {
            ExitException ee = (t instanceof ExitException) ? (ExitException) 
                t : new ExitException(t, ExitException.LAUNCH_ERROR);
            int exitValue =  (ee.getReason() == ExitException.OK) ? 0 : -1;
            if (exitValue!=0) {
                throw ee; // propagate the exception, if it's not an OK one
            }
        }
    }

    /*
     *
     */
    
    protected void appletSSVRelaunch() throws JRESelectException {
	// user choose to relaunch in earlier version of jvm	
	// get java arguments 
	String vmArgs = System.getProperty("javaplugin.vm.options");
	
	// get requested java_version 
	JREDesc newJre = new JREDesc(getParameter(ParameterNames.SSV_VERSION), 0, 0, null, null, null);
	
	throw new JRESelectException(newJre, vmArgs);
    }

    protected void checkRunningJVMArgsSatisfying() throws JRESelectException {
	// no op for Non-JNLP applets
    }

    /*
     *  Allow pre-loading of local .jar files in plug-in lib/app directory
     *  These .jar files are loaded with the Applet2ClassLoader so they
     *  run in the applet's sandbox thereby saving developers the trouble
     *  of writing trusted support classes.
     */
    private void loadLocalJarFiles(Applet2ClassLoader loader, String basePath) {
        File dir = new File(basePath);

        if (dir.exists()) {
            String[] jarList = dir.list(new FilenameFilter() {
                    public boolean accept(File f, String s)  {
                        return(s.endsWith(".jar"));
                    }
                });
            
            for (int i = 0; i < jarList.length; i++) {
                try {
                    URL localJarUrl = (new File(basePath + File.separator + jarList[i])).toURI().toURL(); 

                    loader.addLocalJar(localJarUrl);
                } catch (MalformedURLException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    /**
     * Return the list of jar files if specified.
     * Otherwise return null.
     */
    protected String getJarFiles() {
        // jar listed under new archive_* tag
        if (!newStyleJarList.isEmpty()) {
            // use new style tags archive_*
            return buildJarList((String[]) newStyleJarList.toArray(new String[0]));
        }
        
        // old style tags
        // Figure out the list of all required JARs.
        String archive_ex = getParameter("cache_archive_ex");
        if (archive_ex != null) {
            int idx = archive_ex.indexOf(";");
            if (idx >= 0) {
                archive_ex = buildJarList(splitJarList(archive_ex, true));
            }
        }

	String list = buildJarList(new String[] {
		archive_ex,
		getParameter("cache_archive"),
		getParameter("java_archive"),
		getParameter("archive")
	    }); 
	
	if (DEBUG) {
	    System.out.println("Applet2Manager.getJarFiles() for applet ID " + appletID +
			       " Jar Files:" +list);
	}
	
	return list;
    }

    
    /**
     * Return the list of jar files if specified.
     * Otherwise return null.
     * This method works the same as getJarFiles()
     */
    protected String getCodeSourceLocations() {
	return getJarFiles();
    }
    
    //----------------------------------------------------------------------
    // Support for class loader caching, required for backward compatibility
    //

    // Returns a class loader cache entry, or null if class loader
    // caching is not in use for this manager
    private synchronized Applet2ClassLoaderCache.Entry getClassLoaderCacheEntry() {
        if (classLoaderCache == null)
            return null;

        // Using class loader caching -- see whether we can use a cached loader

        // We need the jar version map to be initialized at this point
        // if it isn't already
        if (classLoaderCacheEntry == null) {
            initJarVersionMap();
            verifyJarVersions();
            classLoaderCacheEntry = classLoaderCache.get(getClassLoaderCacheKey(), this);
            
            if (DEBUG) {
                System.out.println("Applet2Manager.getClassLoaderCacheEntry() for applet ID " + appletID +
                                   ": ClassLoader=" + objToString(classLoaderCacheEntry.getClassLoader()) +
                                   ", ThreadGroup=" + objToString(classLoaderCacheEntry.getThreadGroup()) +
                                   ", AppContext=" + objToString(classLoaderCacheEntry.getAppContext()));
            }
        }
        return classLoaderCacheEntry;
    }

    // Invalidates the class loader cache entry if one is in use
    protected synchronized void invalidateClassLoaderCacheEntry() {
        if (classLoaderCache != null && classLoaderCacheEntry != null) {
            classLoaderCache.markNotCacheable(classLoaderCacheEntry);
        }
    }

    protected boolean usingLegacyLifeCycle() {
        return usingLegacyLifeCycle;
    }

    protected void clearUsingLegacyLifeCycle() {
        usingLegacyLifeCycle = false;
    }

    private static String objToString(Object obj) {
        StringBuffer buf = new StringBuffer();
        buf.append(obj.getClass().getName());
        buf.append("@~0x");
        buf.append(Integer.toHexString(System.identityHashCode(obj)));
        return buf.toString();
    }

    static class CacheEntryCreator implements Applet2ClassLoaderCache.EntryCreator {
        public void createAll(Applet2Manager manager, Applet2ClassLoaderCache.Entry entry) {
            if (DEBUG) {
                System.out.println("Applet2Manager executing createAll() for entry " +
                                   entry.getClassLoaderCacheKey());
            }
            entry.setClassLoader((Applet2ClassLoader)manager.getOrCreatePlugin2ClassLoader());
            entry.setThreadGroup(manager.getOrCreateAppletThreadGroup());
            entry.setAppContext(manager.getOrCreateAppletAppContext());
        }

        public void createThreadGroupAndAppContext(Applet2Manager manager, Applet2ClassLoaderCache.Entry entry) {
            if (DEBUG) {
                System.out.println("Applet2Manager executing createTGAndAC() for entry " +
                                   entry.getClassLoaderCacheKey());
            }

            // Pick up the loader from the entry before doing this work
            manager.loader = entry.getClassLoader();
            if (manager.loader == null) {
                throw new InternalError("Error during bootstrapping of new ThreadGroup and AppContext");
            }
            entry.setThreadGroup(manager.getOrCreateAppletThreadGroup());
            entry.setAppContext(manager.getOrCreateAppletAppContext());
            // Must fix up the class loader to point to the new ThreadGroup and AppContext
            Applet2ClassLoader loader = entry.getClassLoader();
            loader.setThreadGroup(entry.getThreadGroup());
            loader.setAppContext(entry.getAppContext());
            // when we re-use cache loader from classloader cache, we need to
            // check whether we allow codebase recursive read again
            manager.setupClassLoaderCodebaseRecursiveRead(loader);
        }

        public void destroyThreadGroupAndAppContext(Applet2Manager manager,
                                                    Applet2StopListener stopListener,
                                                    long timeToWait,
                                                    Applet2ClassLoaderCache.Entry entry) {
            if (DEBUG) {
                System.out.println("Applet2Manager executing destroyTGAndAC() for entry " +
                                   entry.getClassLoaderCacheKey());
            }

            manager.destroyAppContext(entry.getAppContext(), stopListener, timeToWait);
            // At the point that this method returns, it must be
            // acceptable to zero out these fields in the Entry
            entry.setThreadGroup(null);
            entry.setAppContext(null);
            // We also need to clear out the same fields in the
            // ClassLoader so they get set up properly next time
            Applet2ClassLoader loader = entry.getClassLoader();
            loader.setThreadGroup(null);
            loader.setAppContext(null);
        }
    }

    /** Returns an EntryCreator for use with an Applet2ClassLoaderCache instance. */
    public static Applet2ClassLoaderCache.EntryCreator getCacheEntryCreator() {
        return new CacheEntryCreator();
    }

    private String getClassLoaderCacheKey() {
        if (classLoaderCacheKey == null) {
            // Fixed #4516442 - Switch for classloader policy.
            //
            // If classic classloader policy is set to true
            //
            String param = getParameter("classloader-policy");
        
            if (param != null && param.equals("classic")) {
                /**
                 * Fixed #4501142: Classlaoder sharing policy doesn't 
                 * take "archive" into account. [stanleyh]
                 */
                classLoaderCacheKey = getCodeBase().toString();
            } else {
                // This method is responsible for determining
                // the classloader sharing policy.
                //
                // Classloader is shared between applets if
                // and only if
                //
                // 1. codebase's values are the same
                // 2. list of jar files are the same
                //
                StringBuffer buffer = new StringBuffer();
            
                buffer.append(getCodeBase());
            
                String jarFilesList = getJarFiles();
            
                if (jarFilesList != null) {
                    buffer.append(",");
                    buffer.append(jarFilesList);
                }
            
                classLoaderCacheKey = buffer.toString();
            }
        }
        return classLoaderCacheKey;
    }

    private void verifyJarVersions() {
        // If we aren't using class loader caching, we shouldn't call this
        assert classLoaderCache != null;

        // Copied from JarCacheUtil and repurposed to work with the
        // new class loader cache implementation
    	boolean markClassLoader = false;
	// If versions are specified for all the jar files, try to mark
	// the cached files 
        URL codeBase = getCodeBase();
	Iterator iter = jarVersionMap.keySet().iterator();
	while(iter.hasNext()) {
	    String jarFileName = (String) iter.next();
	    String jarFileVersion = (String) jarVersionMap.get(jarFileName);
            try {
                URL url = new URL(codeBase, jarFileName);

                Trace.msgNetPrintln("cache.version_checking", new Object[] {jarFileName, jarFileVersion});
                // only check versioned resource
                if (jarFileVersion != null) {
                    String cacheVersion = Cache.getCacheEntryVersion(url, null);

                    // if there's a cached jar file, check if the version of the cached jar satisfies the required version
                    if (cacheVersion != null && cacheVersion.compareTo(jarFileVersion) < 0) {
                        // there is no cached jar that matches the required version
                        markClassLoader = true;
                        break;
                    }
                }
            } catch (MalformedURLException e) {
                if (DEBUG) {
                    e.printStackTrace();
                }
            }
	}

	// If one of the jar files has been modified, mark the class
	// loader as not cacheable since it is no longer correct to
	// use it.
	if (markClassLoader) {
            classLoaderCache.markNotCacheable(getClassLoaderCacheKey());
	}
    }

}
