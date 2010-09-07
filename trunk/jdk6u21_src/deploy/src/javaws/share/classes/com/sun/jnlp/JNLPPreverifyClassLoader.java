/*
 * @(#)JNLPPreverifyClassLoader.java	1.11 10/05/21
 * 
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.jnlp;

import java.io.File;
import java.io.FilePermission;
import java.io.IOException;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.jar.JarFile;
import java.util.ArrayList;
import java.security.PrivilegedAction;
import java.security.PrivilegedExceptionAction;
import java.security.AccessController;
import java.security.AccessControlContext;
import java.security.CodeSource;
import java.security.PermissionCollection;
import com.sun.javaws.LaunchDownload;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.javaws.jnl.JARDesc;
import com.sun.javaws.jnl.ResourcesDesc;
import com.sun.javaws.exceptions.JNLPException;
import com.sun.javaws.exceptions.ExitException;
import com.sun.javaws.security.AppPolicy;

import com.sun.deploy.Environment;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.config.Config;
import com.sun.deploy.cache.Cache;
import com.sun.deploy.net.DownloadEngine;
import com.sun.javaws.progress.ProgressListener;

import sun.awt.AppContext;

import com.sun.deploy.cache.CacheEntry;
import java.io.InputStream;
import sun.misc.Resource;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.nio.ByteBuffer;
import java.util.jar.Manifest;
import sun.misc.URLClassPath;

import sun.reflect.Reflection;

import com.sun.deploy.perf.DeployPerfUtil;
import com.sun.deploy.util.NativeLibraryBundle;

/**
 * This classloader is an extension of the URLClassLoader
 * This classloader is the parent of JNLPClassLoader/JNLP2ClassLoader
 * This classloader only loads pre-verified cached JARs from the system cache
 * (i.e. jars for which both code and signature preverification had been done)
 */
public final class JNLPPreverifyClassLoader extends URLClassLoader implements JNLPClassLoaderIf  {


    private static Field ucpField = getUCPField("ucp");
    private static Method defineClassMethod = getDefineClassMethod("defineClass");

    /* There is only going to be one instance */
    private static JNLPPreverifyClassLoader _instance = null;

    private ClassLoader _delegatingClassLoader = null;
    private boolean quiescenceRequested;
    private Thread delegatingThread;
    private int pendingCalls;
    private ClassLoader parent;

    
    /* Reference to LaunchDesc */
    private LaunchDesc _launchDesc = null;
    
    /* Jnlp App policy object */
    private AppPolicy _appPolicy;

    /* The context to be used when loading classes and resources */
    private AccessControlContext _acc = null;

    /* Wheter classloader has been initialized. For 1.4, it gets instantiatd
     * during JVM startup
     */
    private boolean _initialized = false;

    /* jars that are listed in superclass URLClassLoader */
    private ArrayList _jarsInURLClassLoader = new ArrayList();

    /* jars not yet listed in the superclass URLClassLoader */
    private ArrayList _jarsNotInURLClassLoader = new ArrayList();

    // Native libraries knwon and used by this classloader
    // (we need strong reference to prevent early release and
    //  possible deletion of libraries in use)
    // TODO: consider clearing it when classloader is not in use
    //   without finalizers
    private NativeLibraryBundle nativeLibraries = null;

    /**
     * Constructs a new JNLPPreverifyClassLoader
     *
     * <p>If there is a security manager, this method first
     * calls the security manager's <code>checkCreateClassLoader</code> method
     * to ensure creation of a class loader is allowed.
     *
     * @param parent the parent class loader for delegation
     * @exception  SecurityException  if a security manager exists and its
     *             <code>checkCreateClassLoader</code> method doesn't allow
     *             creation of a class loader.
     * @see SecurityManager#checkCreateClassLoader
     */
    public JNLPPreverifyClassLoader(ClassLoader parent) {
        super(new URL[0], parent);
        // this is to make the stack depth consistent with 1.1
        SecurityManager security = System.getSecurityManager();
        if (security != null) {
            security.checkCreateClassLoader();
        }
	this.parent = parent;
    }

    public void setDelegatingClassLoader(ClassLoader cl) {
        _delegatingClassLoader = cl;
    }

    // ----------------------------------------------------
    // Begin JNLP Specific

    public void initialize(LaunchDesc launchDesc, AppPolicy appPolicy) {
        _launchDesc = launchDesc;
        _acc = AccessController.getContext();
        _appPolicy = appPolicy;
        _initialized = false;

        ArrayList urlsToAdd = new ArrayList();
    
        // Add all JARDescs for application to list */
        ResourcesDesc rd = launchDesc.getResources();
        if (rd != null) {
            // Extract list of URLs from Codebase
            sortDescriptors(rd);
            // All JARs in the list must be added; otherwise
            // the JNLPPrevifyClassLoader is not initialized
            for (int i = 0; i < _jarsInURLClassLoader.size(); i++) {
                JARDesc jd = (JARDesc) _jarsInURLClassLoader.get(i);

                if (Cache.isCacheEnabled()) {
              
                    // lookup cache entry and see if cached JAR is pre-verified
                    // Ideally here we should be calling Cache.getCacheEntry, which
                    // would return the latest matching cache entry we have in the
                    // cache.  (If same copy available in both system cache
                    // and user's cache, system cache copy will be returned)
                    // Then we will check if the returned CacheEntry is from
                    // system cache and use it if so.
                    // However, due to the way JavaFX runtime JARs are hosted
                    // currently, we cannot use getCacheEntry here.  this is because
                    // the JavaFX runtime JARs might have different last-modified
                    // time, due to the use of CDN.   For now, the last-modified
                    // time of JavaFX runtime JARs is never used, because
                    // it has a very long expiration and whenever we ship a
                    // new version, JAR filename will change. (version is
                    // part of filename)
                    CacheEntry ce = Cache.getSystemCacheEntry(jd.getLocation(),
                            jd.getVersion());

                    if (ce != null && ce.getClassesVerificationStatus() ==
                            CacheEntry.PREVERIFY_SUCCEEDED &&
                            ce.isKnownToBeSigned()) {
                        // only add JARs to classpath of PreverifyClassLoader if
                        // cached JAR is pre-verified
                        urlsToAdd.add(jd.getLocation());
                        Trace.println("JNLPPreverifyClassLoader.initialize: addURL: " +
                                jd.getLocation(), TraceLevel.CACHE);
                    } else {
                        // not all JARs we expect are preverified
                        if (!Environment.allowAltJavaFxRuntimeURL()) {
                            _initialized = false;
                            Trace.println("JNLPPreverifyClassLoader.initialize: FAILED: " +
                                jd.getLocation(), TraceLevel.CACHE);
                            return;
                        } else {
                            Trace.println("JNLPPreverifyClassLoader.initialize:" +
                                " skip "+jd.getLocation(), TraceLevel.CACHE);
                        }
                    }
                }
            }
        }
        for(int i=0; i<urlsToAdd.size(); i++) {
            addURL((URL) urlsToAdd.get(i));
        }
        _instance = this;
        _initialized = true;
    }

    public boolean contains(URL u) {
        if (!_initialized) return false;

        String s = u.toString();
        URL urls[] = getURLs();
        for(int i=0; i<urls.length; i++) {
            if (s.equals(urls[i].toString())) {
                return true;
            }
        }
        return false;
    }

    /** Returns JARDesc if the JAR file is part of the JNLP codebase */
    public JARDesc getJarDescFromURL(URL url) {
        for (int i = 0; i < _jarsInURLClassLoader.size(); i++) {
            JARDesc jd = (JARDesc) _jarsInURLClassLoader.get(i);
            if (jd.getLocation().toString().equals(url.toString())) {
                return jd;
            }
        }
        return null;
    }

    /*
     *  Methods for downloading parts of the application
     */
    public void downloadResource(URL location, String version,
	ProgressListener dp, boolean isCacheOk)
        throws JNLPException, IOException {
        LaunchDownload.downloadResource(_launchDesc, location, version,
					dp, isCacheOk);
    }

    public void downloadParts(String[] parts,
	ProgressListener dp, boolean isCacheOk)
        throws JNLPException, IOException {
        LaunchDownload.downloadParts(_launchDesc, parts, dp, isCacheOk);
    }

    public void downloadExtensionParts(URL location, String version,
	String[] parts, ProgressListener dp, boolean isCacheOk)
        throws JNLPException, IOException {
        LaunchDownload.downloadExtensionPart(_launchDesc, location, version,
					     parts, dp, isCacheOk);
    }

    public void downloadEager(ProgressListener dp, boolean
	isCacheOk) throws JNLPException, IOException {
        LaunchDownload.downloadEagerorAll(_launchDesc, false, dp, isCacheOk);

    }
    
    /** Get instance */
    public static JNLPClassLoaderIf getInstance() {
        return _instance;
    }
    
    /** Get main JNLP file */
    public LaunchDesc getLaunchDesc() {
        return _launchDesc;
    }

    /** Returns the default security model for this application */
    public int getDefaultSecurityModel() {
        return _launchDesc.getSecurityModel();
    }
   
    /** Overwrite of standard classLoader method */
    public URL getResource(final String name)  {
        // The below code is a workaround a bug in the SocketPermision.implied
        // code. If we are running behind a firewall, and the domain from 
	// where the jar file is obtained from is not directly reachable 
	// (i.e. only reachable through a proxyserver), the first timearound 
	// the SocketPermission will return false. The second time around, 
	// the permission check will have degenerated into a textual
        // domain-name check (since we are running with -DtrustProxy=true), 
        // and therefore will succeed.

        // The do priviliged block below is needed for file protocol.
        // w/o it if a resource in a differant jar is accessed, the URLClassLoader
        // can throw a security exception because it dosn't have read permission
        // on that other jar.
        URL url = (URL) AccessController.doPrivileged(
                    new PrivilegedAction() {
            public Object run() throws SecurityException {
                    URL resource = null;
                    for (int i = 0; resource == null && i < 3; i++) {
                        resource = JNLPPreverifyClassLoader.super.getResource(name);
                    }
                    return resource;
            }
        });

        return url;
    }
    
    /** Called when mapping a native library
    The logic is the following:
    - check if we had loaded this library before
    - find jar with library
    - request private copy (may be same jar if we are first
    in this process who needs it or it is marked as "safe")
    - make sure we release our copy when classloader is gc-ed
     */
    protected String findLibrary(String name) {
        DeployPerfUtil.put("JNLPPreverifyClassLoader.findLibrary - start()");
        // Pass through during startup
        if (!_initialized) {
            Trace.println("JNLPPreverifyClassLoader.findLibrary: " + name + ": not initialized -> super()", TraceLevel.BASIC);
            return super.findLibrary(name);
        }

        name = Config.getInstance().getLibraryPrefix() + name +
                Config.getInstance().getLibrarySufix();

        Trace.println("JNLPPreverifyClassLoader.findLibrary: Looking up native library: " + name, TraceLevel.BASIC);

        synchronized (this) {
            // See whether we already know about it
            if (nativeLibraries != null) {
                String fullPath = (String) nativeLibraries.get(name);
                if (fullPath != null) {
                    Trace.println("JNLPPreverifyClassLoader.findLibrary: native library found: " + fullPath, TraceLevel.BASIC);
                    DeployPerfUtil.put("JNLPPreverifyClassLoader.findLibrary - reusing library");
                    // Got it
                    return fullPath;
                }
            } else {
                nativeLibraries = new NativeLibraryBundle();
            }
        }

        // We don't know about this library yet; need toobtain private copy
        // since multiple class loaders in the same JVM might load the library

        /* This might return different results, depending on downloads */
        ResourcesDesc rd = _launchDesc.getResources();
        JARDesc[] jars = rd.getEagerOrAllJarDescs(true);

        for (int i = 0; i < jars.length; i++) {
            if (jars[i].isNativeLib()) {
                //TODO: consult jar index we may be able to skip this jar
                try {
                    String dir = DownloadEngine.getLibraryDirForJar(
                            name, jars[i].getLocation(),
                            jars[i].getVersion());
                    if (dir != null) {
                        CacheEntry ce = Cache.getSystemCacheEntry(jars[i].getLocation(),
                            jars[i].getVersion());
                        if (ce != null) {
                            JarFile jf = ce.getJarFile();

                            nativeLibraries.prepareLibrary(name, jf, dir);
                            String fullPath = nativeLibraries.get(name);
                            Trace.println("JNLPPreverifyClassLoader.findLibrary: native library found: " + fullPath, TraceLevel.BASIC);
                            DeployPerfUtil.put("JNLPPreverifyClassLoader.findLibrary - found library");
                            return fullPath;
                        }
                    }
                } catch (IOException ioe) {
                    Trace.ignoredException(ioe);
                }
            }
        }

        Trace.println("Native library " + name + " not found", TraceLevel.BASIC);
        DeployPerfUtil.put("JNLPPreverifyClassLoader.findLibrary - return super.findLibrary");
        return super.findLibrary(name);
    }

    /*
     * Helper method to get the private defineClass method in superclass
     */
    private static Method getDefineClassMethod(final String name) {
        return (Method) AccessController.doPrivileged(new PrivilegedAction() {

            public Object run() {
                try {
                    Method m = URLClassLoader.class.getDeclaredMethod(name,
                            new Class[]{String.class, Resource.class, Boolean.TYPE});
                    m.setAccessible(true);
                    return m;
                } catch (Exception e) {
                    return null;
                }
            }
        });
    }

    /*
     * Helper method to get the private ucp field in superclass
     */
    private static Field getUCPField(final String name) {
        return (Field) AccessController.doPrivileged(new PrivilegedAction() {

            public Object run() {
                try {
                    Field field = URLClassLoader.class.getDeclaredField(name);
                    field.setAccessible(true);
                    return field;
                } catch (Exception e) {
                    return null;
                }
            }
        });
    }


    /*
     * Call superclass' private method defineClass() using reflection
     */
    private Class defineClassHelper(String name, Resource res) throws IOException {
     
        try {
            String url = res.getURL().toString();
            // extract orignal URL from JAR url
            String orig_url = url.substring(4, url.indexOf('!'));

            URL u = new URL(orig_url);

            // obtain JAR version information from JARDesc
            JARDesc jd = getJarDescFromURL(u);

            // always do class verification by default
            Boolean doVerification = Boolean.TRUE;

            if (Cache.isCacheEnabled()) {
                // lookup system cache entry and see if cached JAR is pre-verified
                CacheEntry ce = Cache.getSystemCacheEntry(jd.getLocation(),
                            jd.getVersion());

                if (ce != null && ce.getClassesVerificationStatus() ==
                        CacheEntry.PREVERIFY_SUCCEEDED) {
                    // if cached JAR has class pre-verification succeed, skip
                    // class verification
                    doVerification = Boolean.FALSE;
                }
            }

            Class c = (Class) defineClassMethod.invoke(this,
                    new Object[]{name, res, doVerification});

            return c;
        } catch (Exception e) {
            // look up for errors in defineClass() call
            Throwable t = e.getCause();
            while (t != null) {
                // All linkage errors, or just NoClassDefFoundError?
                if (t instanceof LinkageError) {
                    throw (LinkageError) t;
                }
                if (t instanceof IOException) {
                    throw (IOException) t;
                }
                if (t instanceof SecurityException) {
                    throw (SecurityException) t;
                }

                t = t.getCause();
            }

            throw new RuntimeException(e);
        }
    }

    /**
     * This is a private wrapper class for sun.misc.Resource to:
     *    1) allow multiple calls to Resource.getBytes()
     *       without loading data from real resource every time
     *    2) avoid loading of signers info
     *       (performance optimization that can not be applied in generic case)
     */
    private class PreverifiedResource extends Resource {
        private Resource res = null;
        private byte[] cbytes;

        public PreverifiedResource(Resource res) {
            this.res = res;
        }

        public String getName() {
            return res.getName();
        }

        public URL getURL() {
            return res.getURL();
        }

        public URL getCodeSourceURL() {
            return res.getCodeSourceURL();
        }

        public InputStream getInputStream() throws IOException {
            return res.getInputStream();
        }

        public int getContentLength() throws IOException {
            return res.getContentLength();
        }

        public byte[] getBytes() throws IOException {
            if (cbytes != null) {
                return cbytes;
            }
            return cbytes = super.getBytes();
        }

        public ByteBuffer getByteBuffer() throws IOException {
            return res.getByteBuffer();
        }

        public Manifest getManifest() throws IOException {
            return res.getManifest();
        }

        public java.security.cert.Certificate[] getCertificates() {
            return null;
        }

        public java.security.CodeSigner[] getCodeSigners() {
            return null;
        }
    }
    
    private Class findClassHelper(final String name)
            throws ClassNotFoundException {
        if (ucpField == null || defineClassMethod == null) {
            return super.findClass(name);
        }

        try {
            return (Class) AccessController.doPrivileged(new PrivilegedExceptionAction() {

                public Object run() throws ClassNotFoundException {
                    String path = name.replace('.', '/').concat(".class");
                    URLClassPath ucp;
                    try {
                        // Use reflection to get the package private field "ucp" of URLClassLoader
                        ucp = (URLClassPath) ucpField.get(JNLPPreverifyClassLoader.this);
                    } catch (Exception ex) {
                        throw new ClassNotFoundException(name, ex);
                    }
                    Resource res = ucp.getResource(path, false);
                    if (res != null) {
                        try {
                            return defineClassHelper(name,
                                    new PreverifiedResource(res));
                        } catch (IOException e) {
                            throw new ClassNotFoundException(name, e);
                        }
                    } else {
                        throw new ClassNotFoundException(name);
                    }
                }
            }, _acc);
        } catch (java.security.PrivilegedActionException pae) {
            throw (ClassNotFoundException) pae.getException();
        }
    }

    /*
     * Normally, findClass(String, boolean) is called from our own
     * overridden loadClass() method. Calls originating elsewhere are unexpected
     * but technically possible.
     */
    protected Class findClass(String name) throws ClassNotFoundException {
	return findClass(name, false);
    }

    private boolean processingException = false;

    protected Class findClass(String name, boolean delegated) throws ClassNotFoundException {
        if (!_initialized) {
            return super.findClass(name);
        }

        try {
            return findClassHelper(name);
        } catch (ClassNotFoundException e) {
            synchronized (this) {
		if (!delegated && !processingException && _delegatingClassLoader != null && needToApplyWorkaround()) {
                    // workaround to allow runtime JARs and applet JARs to be
                    // loaded by different class loader
                    processingException = true;
		    /*
		     * In order to avoid unsafely violating the child-to-parent class loader
		     * locking hierarchy we carefully turn off all incoming loads between
		     * _delegatingClassLoader and all loaders leading to and including us.
		     * Once we have the _delegatingClassLoader to ourselves we then do the
		     * load on the original thread and finally turn any quiesced loaders back on.
		     */
                    try {
                        boolean interrupted = false;
			DelegatingThread dt = new DelegatingThread(_delegatingClassLoader, this);
			dt.start();
			while (!dt.done()) {
			    try {
			        this.wait();
			    } catch (InterruptedException ie) {
                                interrupted = true;
			    }
			}
                        if (!interrupted) {
		            return _delegatingClassLoader.loadClass(name);
			}
                    } finally {
                        boolean interrupted = false;
			UndelegatingThread udt = new UndelegatingThread(_delegatingClassLoader, this);
			udt.start();
			while (!udt.done()) {
			    try {
			        this.wait();
                            } catch (InterruptedException ie) {
                                interrupted = true;
                            }
			}
                        if (!interrupted) {
                            processingException = false;
			}
                    }
                }
            }
            throw e;
        }
    }


    /*
     * Quiesce all loaders between the target loader and the waiting loader
     * in child to parent class loader locking order. This class is extended
     * below by UndelegatingThread and in sun.plugin2.Applet2ClassLoader.
     */
    public static class DelegatingThread extends Thread {
	protected ClassLoader cl;
	protected ClassLoader waiter;
	protected Thread thread;

	public DelegatingThread(ClassLoader cl, ClassLoader waiter) {
	    super(waiter.getClass().getName());
	    setDaemon(true);
	    this.cl = cl;
	    this.waiter = waiter;
	    this.thread = Thread.currentThread();
	}

        public void run() {
	    /*
	     * Validate that cl is the waiting loader or a child of it.
	     */
	    if (isAncestor(cl, waiter) || cl.equals(waiter)) {
	        quiesce(cl, waiter);
	    }
	    synchronized (waiter) {
	        thread = null;
	        waiter.notifyAll();
	    }
	}

	public boolean done() {
	    synchronized (waiter) {
	        return thread == null;
	    }
	}

        protected boolean isAncestor(ClassLoader child, ClassLoader parent) {
            do {
                child = child.getParent();
                if (child == parent) {
                    return true;
                }
            } while (child != null);
            return false;
        }

	/*
	 * Request quiesence on each class loader from child to parent
	 * including the waiting parent class.
	 *
	 * This method is overridden in Applet2ClassLoader.
	 */
	protected void quiesce(ClassLoader child, ClassLoader parent) {
	    boolean self = waiter.equals(child);
	    if (child instanceof JNLPClassLoaderIf) {
	        ((JNLPClassLoaderIf)child).quiescenceRequested(thread, self);
	    }
	    if (self) {
		return;
	    }
	    quiesce(child.getParent(), parent);
	}
    }

    /*
     * Undo what we did earlier starting with the waiting loader
     * and working ourselves back down to the child loader.
     */
    public static class UndelegatingThread extends DelegatingThread {

	public UndelegatingThread(ClassLoader cl, ClassLoader waiter) {
	    super(cl, waiter);
	}

        public void run() {
	    if (isAncestor(cl, waiter) || cl.equals(waiter)) {
	        unquiesce(cl, waiter);
	    }
	    synchronized (waiter) {
	        thread = null;
	        waiter.notifyAll();
	    }
	}

	public boolean done() {
	    synchronized (waiter) {
	        return thread == null;
	    }
	}

	/*
	 * Cancel quiesence on each class loader from parent to child
	 * beginning with the waiting parent class.
	 *
	 * This method is overridden in Applet2ClassLoader.
	 */
	protected void unquiesce(ClassLoader child, ClassLoader parent) {
	    boolean self = parent.equals(child);
	    if (!self) {
	        unquiesce(child.getParent(), parent);
	    }
	    if (child instanceof JNLPClassLoaderIf) {
	        ((JNLPClassLoaderIf)child).quiescenceCancelled(self);
	    }
	}
    }

    protected synchronized Class loadClass(String name, boolean resolve)
        throws ClassNotFoundException
    {
	return loadClass(name, resolve, false);
    }

    /*
     * An incoming request potentially delegated under our control.
     * 
     */
    public synchronized Class loadClass(String name, boolean resolve, boolean delegated)
        throws ClassNotFoundException
    {
	/*
	 * Other threads wait here if quiescence is pending.
	 */
	while (quiescenceRequested && pendingCalls == 0 && !Thread.currentThread().equals(delegatingThread)) {
	    try {
	        this.wait();
	    } catch (InterruptedException ie) {
		throw new ClassNotFoundException("Quiescence interrupted");
	    }
	}

	try {
	    pendingCalls++;
	    return loadClass0(name, resolve, delegated);
	} finally {
	    pendingCalls--;
	}
    }

    /*
     * Called by DelegatingThread for each loader.
     */
    public synchronized void quiescenceRequested(Thread thread, boolean initiator) {
	/*
	 * If we are the loader initiating quiescence on behalf of an incoming
	 * Class.forName() call then temporarily remove our pending call from the
         * loadClass counter in preparation for delegating to the child loader.
	 */
	if (initiator) {
	    pendingCalls--;
	}
	this.delegatingThread = thread;
	this.quiescenceRequested = true;
    }

    /*
     * Called by UndelegatingThread for each loader.
     */
    public synchronized void quiescenceCancelled(boolean initiator) {
        if (quiescenceRequested) {
	    /*
	     * Restore the original incoming Class.forName() call into the loadClass
	     * counter in preparation for returning after delegating to the child loader.
	     */
            if (initiator) {
	        pendingCalls++;
	    }
	    this.delegatingThread = null;
	    this.quiescenceRequested = false;

            /*
             * Wake up any threads waiting in loadClass() for this loader.
	     * The initiator gets notified by the caller.
             */
	    if (!initiator) {
	        this.notifyAll();
	    }
	}
    }

    /*
     * Full override of ClassLoader.loadClass() except we always have a parent
     * and when it is one of our loaders we pass along a delegation flag which
     * distinguishes direct calls to loadClass() which may come from Class.forName()
     * vs. delegated calls from other class loaders. This allows optimization
     * of the temporary workaround which allows JOGL JNLPAppletLoader and FX
     * AppletStartupRoutine to load from a parent of the normal JNLP applet loader.
     */
    private Class loadClass0(String name, boolean resolve, boolean delegated)
	throws ClassNotFoundException
    {
	// First, check if the class has already been loaded
	Class c = findLoadedClass(name);
	if (c == null) {
	    try {
		c = parent.loadClass(name);
	    } catch (ClassNotFoundException e) {
	        // If still not found, then invoke findClass in order
	        // to find the class.
	        c = findClass(name, delegated);
	    }
	}
	if (resolve) {
	    resolveClass(c);
	}
	return c;
    }

    /*
     * An incoming loadClass() call might be from Class.forName()
     * from one of two classes noted below. We actually only care about
     * forName(String) and not forName(String, boolean, ClassLoader)
     * but can't tell the difference. We skip any frames which we expect
     * between java.lang.Class and us on the call stack.
     */
    protected boolean needToApplyWorkaround() {
        StackTraceElement[] trace = null;
	int i;

        try {
            trace = new Throwable().getStackTrace();
        } catch (NoSuchMethodError nsme) {
        } catch (NoClassDefFoundError ncdfe) {
	}

        /*
         * Any JRE prior to 1.4 won't support the workaround.
         */
	if (trace == null) {
            return false;
	}

	for (i=0; i < trace.length; i++) {
	     String caller = trace[i].getClassName();
             if (caller.equals(JNLPPreverifyClassLoader.class.getName()) ||
                caller.equals(java.lang.Class.class.getName()) ||
                caller.equals(java.lang.ClassLoader.class.getName())) {
		continue;
	    }
	    break;
	}
	if (i > 0 && i < trace.length) {
	    if (trace[i-1].getClassName().equals(java.lang.Class.class.getName()) &&
		trace[i-1].getMethodName().equals("forName")) {
	    	String caller = trace[i].getClassName();
	    	if (caller.equals("com.sun.javafx.runtime.adapter.AppletStartupRoutine")) {
                    return true;
		}
	    }
	}
	return false;
    }



    public URL findResource(String name) {
	return super.findResource(name);
    }

    /**
     * Returns the permissions for the given codesource object.
     * The implementation of this method first calls super.getPermissions,
     * to get the permissions
     * granted by the policy, and then adds additional permissions
     * based on the URL of the codesource.
     * <p>
     * If the protocol is "file"
     * and the path specifies a file, then permission to read that
     * file is granted. If protocol is "file" and the path is
     * a directory, permission is granted to read all files
     * and (recursively) all files and subdirectories contained in
     * that directory.
     * <p>
     * If the protocol is not "file", then
     * to connect to and accept connections from the URL's host is granted.
     * @param codesource the codesource
     * @return the permissions granted to the codesource
     */
    protected PermissionCollection getPermissions(CodeSource codesource) {

        /** 
	 *  This reads the permissions from the default policy file, such
         *  as the .java.policy file. It it kind of questionable if this
         *  is the right thing to do. This would actually allow the policies
         *  used by Java Web Start to be overwritten using the .java.policy
         *  file. Which some people want to do. 
         */
        PermissionCollection perms = super.getPermissions(codesource);
        
        /* Get permissions based on JNLP file that defines code resource */
        try {
            _appPolicy.addPermissions(JNLPPreverifyClassLoader.getInstance(), perms, codesource, true);
        } catch (ExitException ee) { 
            Trace.println("_appPolicy.addPermissions: "+ee, TraceLevel.BASIC);
            Trace.ignoredException(ee);
        }
        
        URL url = codesource.getLocation();
        
	// Get the URL this JAR file and add permissions to read all JARs 
	// that are retreived from the same host. This is to provide the 
	// same semantics as the SocketPermission gives for HTTP URLs
	JARDesc jd = getJarDescFromURL(url);
	if (jd != null) {

            CacheEntry ce = Cache.getSystemCacheEntry(jd.getLocation(),
                            jd.getVersion());
            if (ce != null) {
                File f = ce.getDataFile();

                if (f != null) {
                    String jarPath = f.getPath();
                    perms.add(new FilePermission(jarPath, "read"));
                }
            }

        }
	
	// #5012841 [AppContext security permissions for untrusted 
	// clipboard access]
	//
	// If permissions do not imply clipboard permission, 
	// app is untrusted for clipboard access
	//
	if (!perms.implies(new java.awt.AWTPermission("accessClipboard"))) {
	    AppContext.getAppContext().put(
		"UNTRUSTED_URLClassLoader", Boolean.TRUE);
	}
	
	return perms;
    }

    public JarFile getJarFile(URL url) throws IOException {
	final JARDesc jd = getJarDescFromURL(url);
        JarFile jf = null;
        if (jd != null) {
     
            jf = (JarFile) AccessController.doPrivileged(
                    new PrivilegedAction() {
                public Object run() throws SecurityException {
                    try {
                        CacheEntry ce = Cache.getSystemCacheEntry(jd.getLocation(),
                            jd.getVersion());
                        if (ce == null) {
                            return null;
                        }
                        Object jf = ce.getJarFile();
                    
                        if (jf != null) {
                            return jf;
                        }
                        // fetch resource from net if not in cache
                        return (Object) DownloadEngine.getUpdatedJarFile(
                                jd.getLocation(), jd.getVersion());
                    } catch (IOException ioe) {
                        Trace.ignoredException(ioe);
                    }
                    return null;
                }
            }
            );
            
       
            if (jf == null) {
                throw new IOException("Resource not found: " +
                        jd.getLocation() + ":" + jd.getVersion());
            }
	    return jf;
        }
        return null;
    }

    private void addLoadedJarsEntry(JARDesc jd) {
        if (_jarsInURLClassLoader.contains(jd) == false) {
            // only add non-native lib system cache JARs from dl.javafx.com
            if (jd.isNativeLib() == false &&
                    (Environment.allowAltJavaFxRuntimeURL() ||
                    jd.getLocation().getHost().equals("dl.javafx.com"))) {
                _jarsInURLClassLoader.add(jd);
            }
        }
    }

    private void sortDescriptors(ResourcesDesc rd) {

        ArrayList lazyJarDescArrayList = new ArrayList();

        JARDesc[] allJarDescs = rd.getEagerOrAllJarDescs(true);
        JARDesc main = rd.getMainJar(true);
        JARDesc progress = rd.getProgressJar();

        /* if there is a progress jar, it is first */
        if (progress != null) {
            addLoadedJarsEntry(progress);
        }

        /* then add main jar */
        if (main != null) {
            addLoadedJarsEntry(main);
        }

        for (int i = 0; i < allJarDescs.length; i++) {
            if (allJarDescs[i] != main && allJarDescs[i] != progress) {
                if ((!allJarDescs[i].isLazyDownload())) {
                    /* second add other eager jars */
                    addLoadedJarsEntry(allJarDescs[i]);
                } else if (!rd.isPackagePart(allJarDescs[i].getPartName())) {
                    lazyJarDescArrayList.add(allJarDescs[i]);
                } else {
                    /* add those with package parts to seperate list */
                    _jarsNotInURLClassLoader.add(allJarDescs[i]);
                }
            }
        }

        /* third add the lazy jars w/o package parts */
        for (int i = 0; i < lazyJarDescArrayList.size(); i++) {
            addLoadedJarsEntry((JARDesc)lazyJarDescArrayList.get(i));
        }
    }

    public void addResource(URL resource, String version, String id) {
        // do nothing in this implementation
    }

    //----------------------------------------------------------------------
    // JNLP Service interface
    //

    public javax.jnlp.BasicService getBasicService() {
        return BasicServiceImpl.getInstance();
    }
    public javax.jnlp.FileOpenService getFileOpenService() {
        return FileOpenServiceImpl.getInstance();
    }
    public javax.jnlp.FileSaveService getFileSaveService() {
        return FileSaveServiceImpl.getInstance();
    }
    public javax.jnlp.ExtensionInstallerService getExtensionInstallerService() {
        return ExtensionInstallerServiceImpl.getInstance();
    }
    public javax.jnlp.DownloadService getDownloadService() {
        return DownloadServiceImpl.getInstance();
    }
    public javax.jnlp.ClipboardService getClipboardService() {
        return ClipboardServiceImpl.getInstance();
    }
    public javax.jnlp.PrintService getPrintService() {
        return PrintServiceImpl.getInstance();
    }
    public javax.jnlp.PersistenceService getPersistenceService() {
        return PersistenceServiceImpl.getInstance();
    }
    public javax.jnlp.ExtendedService getExtendedService() {
		return ExtendedServiceImpl.getInstance();
    }
    public javax.jnlp.SingleInstanceService getSingleInstanceService() {
		return SingleInstanceServiceImpl.getInstance();
    }

    public javax.jnlp.IntegrationService getIntegrationService() {
        return new IntegrationServiceImpl(this);
    }

    public javax.jnlp.DownloadService2 getDownloadService2() {
        return DownloadService2Impl.getInstance();
    }
}


