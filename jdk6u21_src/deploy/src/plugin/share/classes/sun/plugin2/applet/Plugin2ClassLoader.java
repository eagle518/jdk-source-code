/*
 * @(#)Plugin2ClassLoader.java	1.34 10/05/21
 *
 * Copyright (c) 2007, 2010, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.applet;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.net.URL;
import java.net.URLClassLoader;
import java.net.MalformedURLException;
import java.io.File;
import java.io.FilePermission;
import java.io.IOException;
import java.io.InputStream;
import java.io.FileNotFoundException;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.List;
import java.util.HashMap;
import java.util.Date;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.text.MessageFormat;
import java.security.AccessController;
import java.security.AccessControlContext;
import java.security.PrivilegedAction;
import java.security.PrivilegedExceptionAction;
import java.security.CodeSigner;
import java.security.CodeSource;
import java.security.Permission;
import java.security.PermissionCollection;
import java.security.Policy;
import java.security.ProtectionDomain;
import java.security.cert.Certificate;
import java.security.cert.CertificateExpiredException;
import java.security.cert.CertificateNotYetValidException;
import sun.awt.AppContext;
import sun.net.www.ParseUtil;
import sun.misc.Resource;
import sun.misc.URLClassPath;
import sun.security.util.SecurityConstants;
import sun.security.validator.ValidatorException;
import com.sun.deploy.net.DownloadEngine;
import com.sun.deploy.config.Config;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.security.CeilingPolicy;
import com.sun.deploy.security.CPCallbackClassLoaderIf;
import com.sun.deploy.security.CPCallbackHandler;
import com.sun.deploy.security.DeployURLClassPath;
import com.sun.deploy.security.DeployURLClassPathCallback;
import com.sun.deploy.security.TrustDecider;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.perf.DeployPerfUtil;
import com.sun.deploy.ui.AppInfo;
import com.sun.deploy.ui.UIFactory;
import sun.plugin2.util.SystemUtil;
import java.util.jar.Manifest;
import java.nio.ByteBuffer;
import com.sun.deploy.cache.Cache;
import com.sun.deploy.cache.CacheEntry;
import com.sun.jnlp.JNLPPreverifyClassLoader;

public abstract class Plugin2ClassLoader extends URLClassLoader implements CPCallbackClassLoaderIf {
    protected static final boolean DEBUG   = (SystemUtil.getenv("JPI_PLUGIN2_DEBUG") != null);

    protected URL base;	/* applet code base URL */
    protected CodeSource codesource; /* codesource for the base URL */

    private static RuntimePermission usePolicyPermission;

    /* The context to be used when loading classes and resources */
    protected AccessControlContext _acc;

    // The Applet2SecurityManager currently needs to be able to query
    // the Plugin2ClassLoader for its associated AppContext and
    // ThreadGroup. Due to bootstrapping issues we don't have these
    // available at the time we instantiate the ClassLoader.
    private AppContext  appContext;
    private ThreadGroup threadGroup;

    private boolean codebaseLookup = true;
    private boolean codebaseLookupInitialized = false;
    private boolean securityCheck = false;
    private volatile boolean allowRecursiveDirectoryRead = true;

    private static Field ucpField = getUCPField("ucp");

    private static Method defineClassMethod = getDefineClassMethod("defineClass");

    // Hash map to store applet compatibility info
    private HashMap jdk11AppletInfo = new HashMap();
    private HashMap jdk12AppletInfo = new HashMap();

    /* Trusted parent plugin class loader */
    private Plugin2ClassLoader pclParent;

    /* dynamic lass path passdown between trusted parent and child loader */
    private List addedURLs = new ArrayList();

    // CPCallbackHandler needed for codebase lookup in applet loader
    private CPCallbackHandler cpHandler;

    private DeployURLClassPathCallback.Element codebaseElement;

    /* Applet startup workaround */
    protected ClassLoader _delegatingClassLoader = null;
    protected boolean quiescenceRequested;
    protected Thread delegatingThread;
    protected int pendingCalls;
    protected ClassLoader parent;

    private static final String UNSIGNED_MESSAGE = " because the class is not signed.";

    //----------------------------------------------------------------------
    // Plugin2ClassLoader construction
    //
    /*
     * Creates a new Plugin2ClassLoader, either Applet2ClassLoader or JNLPClassLoader
     *
     * @see Applet2ClassLoader.newInstance
     * @see JNLP2ClassLoader.newInstance
     */
    protected Plugin2ClassLoader(URL[] urls, URL base) {
        super(urls);
	setUCP(this, new DeployURLClassPath(urls));
        this.base = base;
        this.codesource =
            new CodeSource(base, (java.security.cert.Certificate[]) null);
        this._acc = AccessController.getContext();
	this.parent = getParent();
    }

    protected Plugin2ClassLoader(URL[] urls, URL base, ClassLoader parent) {
	super(urls, parent);
	setUCP(this, new DeployURLClassPath(urls));
	if (parent instanceof Plugin2ClassLoader) {
	    pclParent = (Plugin2ClassLoader)parent;
	}
	this.parent = parent;
        this.base = base;
        this.codesource =
                new CodeSource(base, (java.security.cert.Certificate[]) null);
        this._acc = AccessController.getContext();
    }

    //----------------------------------------------------------------------
    // ClassLoader implementations
    //
    abstract protected Class findClass(String name, boolean delegated)
	throws ClassNotFoundException;

    protected synchronized Class loadClass(String name, boolean resolve)
        throws ClassNotFoundException
    {
	return loadClass(name, resolve, false);
    }

    protected synchronized Class loadClass(String name, boolean resolve, boolean delegated)
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
             */
	    this.notifyAll();
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
		if (pclParent != null) {
		    c = pclParent.loadClass(name, false, true);
		} else if (parent instanceof JNLPPreverifyClassLoader) {
		    JNLPPreverifyClassLoader jpcl = (JNLPPreverifyClassLoader)parent;
		    c = jpcl.loadClass(name, false, true);
		} else {
		    c = parent.loadClass(name);
		}
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
     * between java.lang.Class and us on the call stack. We first try
     * a test which is cheaper for long call stacks but that does not
     * account for the method name.
     */
    protected boolean needToApplyWorkaround() {
	int i;
	StackTraceElement[] trace = new Throwable().getStackTrace();

	for (i=0; i < trace.length; i++) {
	     String caller = trace[i].getClassName();
             if (caller.equals(JNLP2ClassLoader.class.getName()) ||
                caller.equals(Plugin2ClassLoader.class.getName()) ||
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
	    	} else if (caller.equals("org.jdesktop.applet.util.JNLPAppletLauncher")) {
                    return true;
		}
	    }
	}
	return false;
    }


    //----------------------------------------------------------------------
    // URLClassLoader implementations
    //

    /**
     * Returns the permissions for the given codesource object.
     * The implementation of this method first calls super.getPermissions,
     * to get the permissions
     * granted by the super class, and then adds additional permissions
     * based on the URL of the codesource.
     * <p>
     * If the protocol is "file"
     * and the path specifies a file, permission is granted to read all files 
     * and (recursively) all files and subdirectories contained in 
     * that directory. This is so applets with a codebase of
     * file:/blah/some.jar can read in file:/blah/, which is needed to
     * be backward compatible. We also add permission to connect back to
     * the "localhost".
     *
     * @param codesource the codesource
     * @return the permissions granted to the codesource
     */
    protected PermissionCollection getPermissions(CodeSource codesource)
    {
	long t0 = DeployPerfUtil.put(0, "Plugin2ClassLoader - getPermissions() - BEGIN");

        // Code previously from AppletClassLoader
        final PermissionCollection perms = super.getPermissions(codesource);

        URL url = codesource.getLocation();

        String path = null;
        Permission p;

        try {
            p = url.openConnection().getPermission();
        } catch (java.io.IOException ioe) {
            p = null;
        }
        
        if (p instanceof FilePermission) {
            path = p.getName();
        } else if ((p == null) && (url.getProtocol().equals("file"))) {
            path = url.getFile().replace('/', File.separatorChar);
            path = ParseUtil.decode(path);
        }

        if (path != null) {
            final String rawPath = path;
            if (!path.endsWith(File.separator)) {
                int endIndex = path.lastIndexOf(File.separatorChar);
                if (endIndex != -1) {
                    path = path.substring(0, endIndex+1) + "-";
                    perms.add(new FilePermission(path,
                                                 SecurityConstants.FILE_READ_ACTION));
                }
            }
      
            final File f = new File(rawPath);
            
            final boolean isDirectory = f.isDirectory();

            // grant codebase recursive read permission
            // this should only be granted to non-UNC file URL codebase and
            // the codesource path must either be a directory, or a file
            // that ends with .jar or .zip
            if (allowRecursiveDirectoryRead && (isDirectory ||
                    rawPath.toLowerCase().endsWith(".jar") ||
                    rawPath.toLowerCase().endsWith(".zip"))) {
                Permission bperm;
                try {
                    bperm = base.openConnection().getPermission();
                } catch (java.io.IOException ioe) {
                    bperm = null;
                }
                if (bperm instanceof FilePermission) {
                    String bpath = bperm.getName();
                    if (bpath.endsWith(File.separator)) {
                        bpath += "-";
                    }
                    perms.add(new FilePermission(bpath,
                            SecurityConstants.FILE_READ_ACTION));
                } else if ((bperm == null) && (base.getProtocol().equals("file"))) {
                    String bpath = base.getFile().replace('/', File.separatorChar);
                    bpath = ParseUtil.decode(bpath);
                    if (bpath.endsWith(File.separator)) {
                        bpath += "-";
                    }
                    perms.add(new FilePermission(bpath, SecurityConstants.FILE_READ_ACTION));
                }
            }
        }
        // Start of original plugin code
    
        // Give all permissions for java beans embedded thru activex bridge
        if(url != null && url.getProtocol().equals("file")) {
            path = ParseUtil.decode(url.getFile());
            if(path != null) {
                path = path.replace('/', File.separatorChar);
                String axBridgePath = File.separator + Config.getJREHome() + 
                    File.separator + "axbridge" + File.separator + "lib";
                try {
                    path = new File(path).getCanonicalPath();
                    axBridgePath = new File(axBridgePath).getCanonicalPath();
                    if( path != null && axBridgePath != null && 
                        path.startsWith(axBridgePath) ){
                        perms.add(new java.security.AllPermission());
                        Trace.println("Plugin2ClassLoader.getPermissions() X0", TraceLevel.BASIC);
                        return perms;
                    }
                }catch(IOException exc) {
                    //Exception when getCanonicalPath() is called is ignored 
                }
            }
        }

        // Added to check usePolicyPermission only
        PermissionCollection perms2 = null;
        
        long startTime = 0;
        if (DEBUG) {
            startTime = System.currentTimeMillis();
        }
        
        // Get Policy object
        Policy newPolicy = (Policy) AccessController.doPrivileged(new PrivilegedAction() {
                public Object run() {
                    return Policy.getPolicy();
                }
            } );

        if (DEBUG) {
            long curTime = System.currentTimeMillis();
            System.out.println("Applet2ClassLoader: Policy.getPolicy() took " +
                               (curTime - startTime) + " ms");
        }

        // Get Policy permisions
        perms2 = newPolicy.getPermissions(codesource);
	DeployPerfUtil.put(t0, "Plugin2ClassLoader - after newPolicy.getPermissions");

        addDefaultPermissions(perms);

        if (usePolicyPermission == null)
            usePolicyPermission = new RuntimePermission("usePolicy");

        if (!perms2.implies(usePolicyPermission)) {
            boolean allPerms = isTrustedByTrustDecider(codesource);
            DeployPerfUtil.put(t0, "Plugin2ClassLoader - after isAllPermissionGranted()");
            if (allPerms) {
                CeilingPolicy.addTrustedPermissions(perms);
                Trace.println("Plugin2ClassLoader.getPermissions CeilingPolicy allPerms", TraceLevel.BASIC);
            }
        }

        // #5012841 [AppContext security permissions for untrusted clipboard access]
        //
        // If permissions do not imply clipboard permission, app is untrusted for clipboard access
        //
        // Added to check AWTPermission from user policy file too
        if ((perms.implies(new java.awt.AWTPermission("accessClipboard")) == false) &&
            (perms2.implies(new java.awt.AWTPermission("accessClipboard")) == false)) {
            sun.awt.AppContext.getAppContext().put("UNTRUSTED_CLIPBOARD_ACCESS_KEY", Boolean.TRUE);
        }

	DeployPerfUtil.put(t0, "Plugin2ClassLoader - getPermissions() - END");
        return perms;
    }

    private boolean usePolicyPermissions(PermissionCollection perms) {
        if (usePolicyPermission == null) {
            usePolicyPermission = new RuntimePermission("usePolicy");
	}

        return perms.implies(usePolicyPermission);
    }

    private boolean isTrustedByTrustDecider(CodeSource codesource) {
        boolean allPerms = false;
        if (isTrustedCodesource(codesource.getLocation())) {
            allPerms = true;
        } else if (codesource.getCertificates() != null) {
            // code is signed, and user wants to be prompted for AllPermission  
            try {
                long pm = TrustDecider.isAllPermissionGranted(codesource, new AppInfo(), false);
                allPerms = (pm != TrustDecider.PERMISSION_DENIED);
                if (allPerms && pm != TrustDecider.PERMISSION_GRANTED_FOR_SESSION) {
                    //permissions were permanently granted for this code source
                    //=> update cache 
                    CacheEntry ce = getCacheEntry(codesource.getLocation(), (String)null);
                    if (ce != null) {
                        ce.updateValidationResultsForApplet(true, null, System.currentTimeMillis(), pm);
                    }
                }
            } catch (CertificateExpiredException e1) {
                Trace.securityPrintException(e1, ResourceManager.getMessage("rsa.cert_expired"),
                                         ResourceManager.getMessage("security.dialog.caption"));    
            } catch (CertificateNotYetValidException e2) {
                Trace.securityPrintException(e2, ResourceManager.getMessage("rsa.cert_notyieldvalid"),
                                         ResourceManager.getMessage("security.dialog.caption"));    
            } catch (Exception e3) {
                Trace.securityPrintException(e3, ResourceManager.getMessage("rsa.general_error"),
                                         ResourceManager.getMessage("security.dialog.caption"));    
            }
        }

	return allPerms;
    }

    /** Helper method to add a default set of permissions that both
        untrusted Java code as well as JavaScript should be able to
        access. */
    public static void addDefaultPermissions(PermissionCollection perms) {
        // Inject permission to access sun.audio package
        //
        perms.add(new java.lang.RuntimePermission("accessClassInPackage.sun.audio"));

        // Inject permission to read certain generic browser properties
        //
        perms.add(new java.util.PropertyPermission("browser", "read"));
        perms.add(new java.util.PropertyPermission("browser.version", "read"));
        perms.add(new java.util.PropertyPermission("browser.vendor", "read"));
        perms.add(new java.util.PropertyPermission("http.agent", "read"));
    
        // allow read/write to jnlp.*, javaws.*, and javapi.*
        perms.add(new java.util.PropertyPermission("javapi.*", "read,write"));
        perms.add(new java.util.PropertyPermission("javaws.*", "read,write"));
        perms.add(new java.util.PropertyPermission("jnlp.*", "read,write"));

        // Inject permission to read certain Java Plug-in specific properties
        //
        perms.add(new java.util.PropertyPermission("javaplugin.version", "read"));
        perms.add(new java.util.PropertyPermission("javaplugin.vm.options", "read"));
    }

    //----------------------------------------------------------------------
    // Applet2SecurityManager service
    //
    // FIXME: we should rethink the Applet2SecurityManager
    // implementation. It may be possible to implement some of its
    // algorithms in a simpler, more robust, and/or higher performance
    // fashion.

    public AppContext getAppContext() {
        return appContext;
    }

    public void setAppContext(AppContext appContext) {
        // Note that this admits the possibility of setting the
        // AppContext back to null, which happens when class loader
        // caching is enabled
        if (appContext != null && this.appContext != null) {
            throw new IllegalStateException("May not set the AppContext twice");
        }
	if (pclParent != null) {
	    pclParent.setAppContext(appContext);
	}
        this.appContext = appContext;
    }

    public ThreadGroup getThreadGroup() {
        return threadGroup;
    }

    public void setThreadGroup(ThreadGroup threadGroup) {
        // Note that this admits the possibility of setting the
        // ThreadGroup back to null, which happens when class loader
        // caching is enabled
        if (threadGroup != null && this.threadGroup != null) {
            throw new IllegalStateException("May not set the ThreadGroup twice");
        }
	if (pclParent != null) {
	    pclParent.setThreadGroup(threadGroup);
	}
        this.threadGroup = threadGroup;
    }

    //----------------------------------------------------------------------
    // Plugin2Manager service
    //

    /**
     * Get the codebase lookup flag.
     */
    public synchronized boolean getCodebaseLookup()  {
	if (codebaseLookup && !codebaseLookupInitialized) {

	    /*
	     * Check if codebase lookup should be disabled for a Trusted-Only
	     * child class loader.
	     */
	    Object obj = getCallbackHandler();
	    if (obj != null) {
	        CPCallbackHandler cb = (CPCallbackHandler)obj;
		DeployURLClassPathCallback child = cb.getChildCallback();
		DeployURLClassPathCallback parent = cb.getParentCallback();
		DeployURLClassPathCallback.Element codebaseElement;

		/*
		 * Codebase lookup always happens in child loader. Parent callback
		 * sets up state. Child callback tells us whether we are enabled.
		 */
		try {
		    codebaseLookup = false;
		    parent.openClassPathElement(getBaseURL());
		    codebaseElement = child.openClassPathElement(getBaseURL());
		    setCodebaseElement(codebaseElement);
		    codebaseLookup = !codebaseElement.skip();
		} catch (SecurityException se) {
		    /* ignore */
		} catch (IOException ioe) {
		    /* ignore */
		}
	    }
	    codebaseLookupInitialized = true;
	}
        return codebaseLookup;
    }

    /**
     * Set the codebase lookup flag.
     */
    public void setCodebaseLookup(boolean codebaseLookup)  {
	if (pclParent != null) {
	    /* Parent loads only trusted libraries from JARs */
	    pclParent.setCodebaseLookup(false);
	}
        this.codebaseLookup = codebaseLookup;
    }

    void setSecurityCheck(boolean securityCheck) {
	if (pclParent != null) {
	    pclParent.setSecurityCheck(securityCheck);
	}
	this.securityCheck = securityCheck;
    }

    boolean getSecurityCheck() {
        return securityCheck;
    }

    void disableRecursiveDirectoryRead() {
	if (pclParent != null) {
	    pclParent.disableRecursiveDirectoryRead();
	}
        allowRecursiveDirectoryRead = false;
    }

    /*
     * Load and resolve the file specified by the applet tag CODE
     * attribute. The argument can either be the relative path
     * of the class file itself or just the name of the class.
     */
    public Class loadCode(String name) throws ClassNotFoundException {
	long t0 = DeployPerfUtil.put(0, "Plugin2ClassLoader - loadCode() - BEGIN");
        // first convert any '/' or native file separator to .
        name = name.replace('/', '.');
        name = name.replace(File.separatorChar, '.');

        // deal with URL rewriting
        String cookie = null;
        int index = name.indexOf(";");
        if(index != -1) {
            cookie = name.substring(index, name.length());
            name = name.substring(0, index);
        }

        // save that name for later
        String fullName = name;
        // then strip off any suffixes
        if (name.endsWith(".class") || name.endsWith(".java")) {
            name = name.substring(0, name.lastIndexOf('.'));
        }

        ClassNotFoundException exc = null;
        try {
            if(cookie != null)
                name = (new StringBuffer(name)).append(cookie).toString();
	    return loadClass(name);
        } catch (ClassNotFoundException e) {
            if (e.getMessage().indexOf(UNSIGNED_MESSAGE) >= 0) {
                exc = e;
            }
        }
        // then if it didn't end with .java or .class, or in the 
        // really pathological case of a class named class or java
        if(cookie != null)
            fullName = (new StringBuffer(fullName)).append(cookie).toString();

        try {
	    return loadClass(fullName);
        } catch (ClassNotFoundException e) {
            if (exc != null) {
                throw exc;
            }
            throw e;
        }
    }

    /**
     * Set applet target level as JDK 1.1.
     *
     * @param clazz Applet class.
     * @param bool true if JDK is targeted for JDK 1.1;
     *             false otherwise.
     */
    void setJDK11Target(Class clazz, boolean bool)  
    {
	if (pclParent != null) {
	    pclParent.setJDK11Target(clazz, bool);
	}
        jdk11AppletInfo.put(clazz.toString(), Boolean.valueOf(bool));
    }

    /**
     * Set applet target level as JDK 1.2.
     *
     * @param clazz Applet class.
     * @param bool true if JDK is targeted for JDK 1.2;
     *             false otherwise.
     */
    void setJDK12Target(Class clazz, boolean bool)  
    {
	if (pclParent != null) {
	    pclParent.setJDK12Target(clazz, bool);
	}
        jdk12AppletInfo.put(clazz.toString(), Boolean.valueOf(bool));
    }

    /**
     * Determine if applet is targeted for JDK 1.1.
     *
     * @param applet Applet class.
     * @return TRUE if applet is targeted for JDK 1.1;
     *         FALSE if applet is not;
     *         null if applet is unknown.
     */
    Boolean isJDK11Target(Class clazz) 
    {
        return (Boolean) jdk11AppletInfo.get(clazz.toString());
    }

    /**
     * Determine if applet is targeted for JDK 1.2.
     *
     * @param applet Applet class.
     * @return TRUE if applet is targeted for JDK 1.2;
     *         FALSE if applet is not;
     *         null if applet is unknown.
     */
    Boolean isJDK12Target(Class clazz) 
    {
        return (Boolean) jdk12AppletInfo.get(clazz.toString());
    }
  
    //---------------------------------------------------------------------------
    // Security-related helper methods
    //

    private CacheEntry getCacheEntry(URL u, String jarVersion) {
        if (Cache.isCacheEnabled()) {
           return Cache.getCacheEntry(u, null, jarVersion);
        }
        return null;
    }

    private HashMap knownSources = new HashMap();

    private boolean isTrustedCodesource(URL u) {
        Object o = knownSources.get(u);
        if (o != null) {
          if (Boolean.TRUE.equals(o)) {
            return true;
          } else {
            return false;
          }
        }

        //if cached && cached validation state is ok 
        CacheEntry ce = getCacheEntry(u, (String)null);
        if (ce != null && ce.getValidationTimestampt() != 0 && ce.isKnownToBeSigned()) {
            knownSources.put(u, Boolean.TRUE);
            return true;
	}
        knownSources.put(u, Boolean.FALSE);
        return false;
    }


    // For propagating ClassNotFoundExceptions due to insecure JVMs
    protected static final ThreadLocal cnfeThreadLocal = new ThreadLocal();

    private void setCallbackHandler(CPCallbackHandler cb) {
	cpHandler = cb;
    }

    private Object getCallbackHandler() {
	return cpHandler;
    }

    private void setCodebaseElement(DeployURLClassPathCallback.Element e) {
	codebaseElement = e;
    }

    private Object getCodebaseElement() {
	return codebaseElement;
    }

    static boolean setDeployURLClassPathCallbacks(Plugin2ClassLoader parent, Plugin2ClassLoader child) {
        try {

	    /*
	     * We are dependent on new JavaUtilJarAccess methods.
	     */
	    if (!CacheEntry.hasEnhancedJarAccess()) {
        	Trace.println("setDeployURLClassPathCallbacks: no enhanced access" , TraceLevel.BASIC);
		return false;
	    }


            CPCallbackHandler handler;
            handler = new CPCallbackHandler(parent, child);
            getDUCP(parent).setDeployURLClassPathCallback(handler.getParentCallback());
            getDUCP(child).setDeployURLClassPathCallback(handler.getChildCallback());

	    /* Callbacks for child codebase lookup */
	    child.setCallbackHandler(handler);
	    parent.setCodebaseLookup(false);
        } catch (ThreadDeath td) {
	    throw td;
        } catch (Exception e) {
            return false;
        } catch (Error err) {
            return false;
        }
        return true;
    }

    /*
     * Install the child callback in the temporary URLClassPath used
     * to load serialized applets from the codebase in the
     * getResourceAsResource() method.
     */
    void setDeployURLClassPathCallback(DeployURLClassPath ucp) {
	Object obj = getCallbackHandler();
	if (obj != null) {
	    CPCallbackHandler cb = (CPCallbackHandler)obj;
            ucp.setDeployURLClassPathCallback(cb.getChildCallback());
	}
    }

    /*
     * Check if CPCallbackHandler allows codebase lookups to succeed.
     */
    void checkResource(String name) throws SecurityException {
	Object obj = getCodebaseElement();
	if (obj != null) {
	    DeployURLClassPathCallback.Element element = (DeployURLClassPathCallback.Element)obj;
	    try {
	        element.checkResource(name);
 	    } catch (SecurityException se) {
		Trace.println("resource name \"" + name + "\" in " + base + " : " + se, TraceLevel.SECURITY);
		throw se;
	    }
	}
    }

    private static DeployURLClassPath getDUCP(Plugin2ClassLoader cl) {
	return (DeployURLClassPath)getUCP(cl);
    }

    private static URLClassPath getUCP(Plugin2ClassLoader cl) {
        URLClassPath ucp = null;
        try {
            // Use reflection to get the package private field "ucp" of URLClassLoader
            ucp = (URLClassPath) ucpField.get(cl);
        } catch (Exception ex) {
        }
        return ucp;
    }

    private static void setUCP(Plugin2ClassLoader cl, URLClassPath ucp) {
        try {
            // Use reflection to set the package private field "ucp" of URLClassLoader
            ucpField.set(cl, ucp);
        } catch (Exception ex) {
        }
    }

    protected void addURL(URL url)
    {
        if (pclParent != null) {
	    /* Also in parent... */
        Trace.println("Plugin2ClassLoader.addURL parent called for " + url, TraceLevel.BASIC);
            pclParent.addURL(url);
        }
        super.addURL(url);
    }

    /* Add a URL to the local class path without forwarding to the parent loader */
    void addURL2(URL url)
    {
	if (pclParent != null) {
	    /* from parent */
	    drainPendingURLs();
	} else {
	    putAddedURL(url);
	}
        Trace.println("Plugin2ClassLoader.addURL2 called for " + url, TraceLevel.BASIC);
	super.addURL(url);
    }

    /* Update child loader with pending previous parent addURL2s */
    boolean drainPendingURLs() {
        List addURLs = pclParent.grabAddedURLs();
	int i;
        for (i=0; i < addURLs.size(); i++) {
            Trace.println("Plugin2ClassLoader.drainPendingURLs addURL called for " + addURLs.get(i), TraceLevel.BASIC);
            super.addURL((URL)addURLs.get(i));
        }
	return i != 0;
    }

    /* Get the list of URLs added via calls to addURL2() on the trusted plugin parent loader */
    synchronized List grabAddedURLs()
    {
	/* Optimize to return singleton immutable empty list when empty? */
	List urls = addedURLs;
	addedURLs = new ArrayList();
	return urls;
    }

    synchronized void putAddedURL(URL url)
    {
	addedURLs.add(url);
    }
    
    protected Class findClassHelper(String name) throws ClassNotFoundException {
	if (ucpField == null || defineClassMethod == null) {
            return super.findClass(name);
	} else {
	    final String className = name;
            try {
                return (Class)
                    AccessController.doPrivileged(new PrivilegedExceptionAction() {
                            public Object run() throws ClassNotFoundException {
                                String path = className.replace('.', '/').concat(".class");
                                URLClassPath ucp;
                                try {
                                    // Use reflection to get the package private field "ucp" of URLClassLoader
                                    ucp = (URLClassPath) ucpField.get(Plugin2ClassLoader.this);
                                } catch (Exception ex) {
                                    throw new ClassNotFoundException(className, ex);
                                }
                                Resource res = ucp.getResource(path, false);
                                if (res != null) {
                                    // Wrap this Resource (JarEntry) with our local implementation which
                                    // can handle getBytes() to be called more than once.
                                    WrapResource wrapRes = new WrapResource(res);
                                    if (getSecurityCheck()) {                                    
                                        // completely read the InputStream for the JarEntry here once so that 
                                        // JarEntry.getCertificates() or JarEntry.getCodeSigners() can be done
                                        // later on in any isAllPermissionGranted() check.
                                        try {
                                            wrapRes.getBytes();
                                        } catch (IOException e) {
                                            throw new ClassNotFoundException(className, e);
                                        }
                                    }

                                    // If the ClassLoader is required to check all classes being signed
                                    // before they can be loaded, we extract Certificates/CodeSigners 
                                    // from the Resource and test whether they are valid and AllPermission
                                    // granted.
                                    if (getSecurityCheck() && !isAllPermissionGranted(wrapRes)) {
                                        cnfeThreadLocal.set(newClassNotFoundException(className));
                                        throw (ClassNotFoundException) cnfeThreadLocal.get();
                                    } else {
                                        try {
                                            return defineClassHelper(className, wrapRes);
                                        } catch (IOException e) {
					    // we don't catch LinkageError here, 
					    // propagate them to callers
                                            throw new ClassNotFoundException(className, e);
                                        }
                                    }
                                } else {
                                    throw new ClassNotFoundException(className);
                                }
                            }
                        }, _acc);
            } catch (java.security.PrivilegedActionException pae) {
                throw (ClassNotFoundException) pae.getException();
            }
	}
    }

    /*
     * Call superclass' private method defineClass() using reflection
     */
    private Class defineClassHelper(String name, Resource res) throws IOException {
        try {
            return (Class) defineClassMethod.invoke(this, new Object[] {name, res});
                  
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

    /*
     * Helper method to get the private defineClass method in superclass
     */
    private static Method getDefineClassMethod(final String name) {
        return (Method) AccessController.doPrivileged(new PrivilegedAction() {

            public Object run() {
                
                try {
                    Method m = URLClassLoader.class.getDeclaredMethod(name,
                            new Class[]{String.class, Resource.class});
                    m.setAccessible(true);
                    return m;
                } catch (NoSuchMethodException nsme) {                
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
     * Helper method to throw a new ClassNotFoundException to explain
     * that the class is not found because it is not signed when the
     * Java VM is launched with insecure properties
     */

    protected static ClassNotFoundException newClassNotFoundException(String name) {
	return new ClassNotFoundException(name + UNSIGNED_MESSAGE);
    }

    /*
     * Helper method to check if AllPermission for a Resource
     */
    protected static boolean isAllPermissionGranted(Resource res) {
	CodeSource cs;
	URL url = res.getCodeSourceURL();
	
	if (Config.isJavaVersionAtLeast15()){
	    CodeSigner [] signers = res.getCodeSigners();
	    if (signers != null) {
		cs = new CodeSource(url, signers);
	    } else {
		// CodeSigner[] is null, try to get Certificate[]
		cs = new CodeSource(url, res.getCertificates());
	    }
	} else {
	    Certificate [] certs = res.getCertificates();
	    cs = new CodeSource(url, certs);
	}
	
	try {
	    return TrustDecider.isAllPermissionGranted(cs) != TrustDecider.PERMISSION_DENIED;
	} catch (Exception ex) {
	    return false;
	}
    }

    //
    // CPCallbackClassLoader methods
    //
    public CodeSource[] getTrustedCodeSources(CodeSource[] sources) {
	List list = new ArrayList();

        Policy policy = (Policy) AccessController.doPrivileged(new PrivilegedAction() {
            public Object run() {
                return Policy.getPolicy();
            }
        } );

	for (int i=0; i < sources.length; i++) {
	    CodeSource cs = sources[i];
	    boolean trusted = false;
	    PermissionCollection perms;

	    perms = policy.getPermissions(cs);
	    if (usePolicyPermissions(perms)) {
	        trusted = isTrustedByPolicy(policy, perms);
	    } else {
	        trusted = isTrustedByTrustDecider(cs);
	    }
	    if (trusted) {
	        list.add(cs);
	    }
	}
	return (CodeSource[]) list.toArray(new CodeSource[list.size()]);
    }

    

    /*
     * Is every permission implied by the default permissions?
     * In other words, does coming from this codesource get us
     * any special policy grants?
     */
    private boolean isTrustedByPolicy(Policy policy, PermissionCollection perms) {
	PermissionCollection sandbox;

        Trace.println("Plugin2ClassLoader.isTrustedByPolicy called ", TraceLevel.BASIC);
	sandbox = policy.getPermissions(new CodeSource(null, (Certificate[]) null));
	Enumeration e = perms.elements();
	while (e.hasMoreElements()) {
	    Permission perm = (Permission)e.nextElement();
	    if (!sandbox.implies(perm)) {
                Trace.println("Plugin2ClassLoader.isTrustedByPolicy extended policy perm " + perm, TraceLevel.BASIC);
		return true;
	    }
	}
        Trace.println("Plugin2ClassLoader.isTrustedByPolicy returns false ", TraceLevel.BASIC);
	return false;
    }

    //----------------------------------------------------------------------
    // Methods for SSV support
    //

    private boolean ssvDialogShown;

    /** Indicates whether the SSV dialog has been shown yet for this ClassLoader. */
    public boolean getSSVDialogShown() {
        return ssvDialogShown;
    }

    /** Sets whether the SSV dialog has been shown yet for this ClassLoader. */
    public void setSSVDialogShown(boolean shown) {
        ssvDialogShown = shown;
    }

    /**
     * This is a private wrapper class for sun.misc.Resource to override Resource.getBytes() so that 
     * this function can be called more than once.
     */
    private class WrapResource extends Resource {
        private Resource res = null;
        private byte[] cbytes;

        public WrapResource(Resource res) {
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
            //make sure bytes were read at least once, otherwise certificates
            //can be null
            try {
                getBytes();
            } catch (IOException e) {}
            return res.getCertificates();
        }
 
        public java.security.CodeSigner[] getCodeSigners() {
            //make sure bytes were read at least once, otherwise signers
            //can be null
            try {
                getBytes();
            } catch (IOException e) {}
            return res.getCodeSigners();
        }
    }

    /**
     * Returns the context to be used when loading classes and resources.
     */
    AccessControlContext getACC() {
        return _acc;
    }

    /*
     * Returns the applet code base URL.
     */
    URL getBaseURL() {
        return base;
    }

    /**
     * Looks up resource in URLCLassLoader path,
     *   and then optionally in codebase.
     * Returns resource as Resource
     *   so as to include actual source URL and signers.
     */
    Resource getResourceAsResource (String name) throws MalformedURLException, FileNotFoundException
    {
	if (pclParent != null) {
	    try {
		return pclParent.getResourceAsResource(name);
	    } catch (FileNotFoundException e) {
		// Ignore
	    }
	}
        URLClassPath ucp1 = sun.misc.SharedSecrets.getJavaNetAccess().getURLClassPath(this);
        Resource res1 = ucp1.getResource(name, false);
        if (res1 != null) {
            return res1;
        }
        if (getCodebaseLookup()) {
            DeployURLClassPath ucp = new DeployURLClassPath( new java.net.URL[] { getBaseURL() });

            setDeployURLClassPathCallback(ucp);
            Resource res = ucp.getResource(name, false);
            if (res != null) {
                return res;
            }
        }

        throw new FileNotFoundException("Resource "+name+" not found");
    }

    /**
     * Returns an acc appropriate for a given resource.
     * If the resource was code, this is the acc it would put on the stack.
     */
    AccessControlContext getACC(sun.misc.Resource res) {
        CodeSource codeSource = new CodeSource(res.getCodeSourceURL(), res.getCodeSigners());
        ProtectionDomain protectionDomain = new ProtectionDomain(codeSource,
                                        getPermissions(codeSource), this, null);
          return new AccessControlContext(new ProtectionDomain[] { protectionDomain });
    }
}
