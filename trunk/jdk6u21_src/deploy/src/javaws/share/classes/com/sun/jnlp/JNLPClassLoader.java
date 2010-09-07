/*
 * @(#)JNLPClassLoader.java	1.64 10/05/22
 * 
 * Copyright (c) 2008, 2009, Oracle and/or its affiliates. All rights reserved.
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
import java.util.Enumeration;
import java.util.List;
import java.security.PrivilegedAction;
import java.security.PrivilegedExceptionAction;
import java.security.AccessController;
import java.security.AccessControlContext;
import java.security.AllPermission;
import java.security.CodeSource;
import java.security.Permission;
import java.security.PermissionCollection;
import java.security.Policy;
import java.security.cert.Certificate;
import com.sun.javaws.LaunchDownload;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.javaws.jnl.JARDesc;
import com.sun.javaws.jnl.ResourcesDesc;
import com.sun.javaws.exceptions.JNLPException;
import com.sun.javaws.exceptions.ExitException;
import com.sun.javaws.security.AppPolicy;

import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.config.Config;
import com.sun.deploy.cache.Cache;
import com.sun.deploy.net.DownloadEngine;
import com.sun.javaws.progress.ProgressListener;
import com.sun.deploy.security.CPCallbackClassLoaderIf;
import com.sun.deploy.security.CPCallbackHandler;
import com.sun.deploy.security.DeployURLClassPath;
import com.sun.deploy.security.DeployURLClassPathCallback;

import sun.awt.AppContext;

import com.sun.deploy.cache.CacheEntry;
import sun.misc.Resource;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import sun.misc.URLClassPath;



/**
 * This classloader is an extension of the URLClassLoader
 *
 */
public final class JNLPClassLoader extends URLClassLoader implements JNLPClassLoaderIf, CPCallbackClassLoaderIf  {
    
    /* There is only going to be one instance */
    private static JNLPClassLoader _instance = null;
    private static JNLPPreverifyClassLoader _preverifyCL = null;
    
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

    private static Field ucpField = getUCPField("ucp");

    /* dynamic lass path passdown between trusted parent and child loader */
    private List addedURLs = new ArrayList();

    /* Trusted parent JNLP class loader */
    private JNLPClassLoader _jclParent;

    /**
     * Constructs a new JNLPClassLoader which provides no services until
     * a later call to a private JNLP-specific initialize() method.
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
    private JNLPClassLoader(ClassLoader parent) {
        super(new URL[0], parent);
        // this is to make the stack depth consistent with 1.1
        SecurityManager security = System.getSecurityManager();
        if (security != null) {
            security.checkCreateClassLoader();
        }
	if (parent instanceof JNLPClassLoader) {
	    _jclParent = (JNLPClassLoader) parent;
	}
	setUCP(this, new DeployURLClassPath(new URL[0]));
    }

    // ----------------------------------------------------
    // Begin JNLP Specific

    private void initialize(LaunchDesc launchDesc, AppPolicy appPolicy) {
        _launchDesc = launchDesc;
        _acc = AccessController.getContext();
        _appPolicy = appPolicy;
        _initialized = true;

        if (_jclParent != null) {
            _jclParent.initialize(launchDesc, appPolicy);
            drainPendingURLs();
            return;
        }

	if (_preverifyCL != null) {
            _preverifyCL.initialize(launchDesc, appPolicy);
	}

        // Add all JARDescs for application to list */
        ResourcesDesc rd = launchDesc.getResources();
        if (rd != null) {
            // Extract list of URLs from Codebase
            sortDescriptors(rd);
            for (int i = 0; i < _jarsInURLClassLoader.size(); i++) {
                JARDesc jd = (JARDesc) _jarsInURLClassLoader.get(i);

                //add url unless it is part of preverify classloader
                if (_preverifyCL == null ||
                        !_preverifyCL.contains(jd.getLocation())) {
                    addURL2(jd.getLocation());
                }
            }
        }
    }
    
    public JNLPPreverifyClassLoader getJNLPPreverifyClassLoader() {
        return _preverifyCL;
    }

   /** Factory method to creatre the JNLPClassloader uninitialized */
    public static JNLPClassLoader createClassLoader() {
        if (_instance == null) {
            _preverifyCL = new JNLPPreverifyClassLoader(ClassLoader.getSystemClassLoader());
	    JNLPClassLoader parent = new JNLPClassLoader(_preverifyCL);

            // Only use callback if mixed code security enhancement is enabled
            if (Config.getMixcodeValue() != Config.MIXCODE_DISABLE) {    
                JNLPClassLoader child = new JNLPClassLoader(parent);
                if (!JNLPClassLoader.setDeployURLClassPathCallbacks(parent, child)) {
                    _instance = parent;
                } else {
                    _instance = child;
		}
            }
            else {
                _instance = parent;
            }
        }
        return _instance;
    }    
    

    /** Factory method to create and/or initialize the JNLPClassloader */
    public static JNLPClassLoader createClassLoader(
	LaunchDesc ld, AppPolicy appPolicy) {

	JNLPClassLoader jnlpClassLoader = createClassLoader();

        if (!jnlpClassLoader._initialized) {
            jnlpClassLoader.initialize(ld, appPolicy);
        }
        return jnlpClassLoader;
    }
    
    /** Get instance */
    public static JNLPClassLoaderIf getInstance() {
        return _instance;
    }
    
    /** Get main JNLP file */
    public LaunchDesc getLaunchDesc() {
        return _launchDesc;
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
    
    /** Returns JARDesc if the JAR file is part of the JNLP codebase */
    public JARDesc getJarDescFromURL(URL url) {
        if (_jclParent != null) {
            return _jclParent.getJarDescFromURL(url);
        }
	for (int i=0; i<_jarsInURLClassLoader.size(); i++) {
	    JARDesc jd = (JARDesc) _jarsInURLClassLoader.get(i);
            if (jd.getLocation().toString().equals(url.toString())) {
		return jd;
            }
	}
        return null;
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
                        resource = JNLPClassLoader.super.getResource(name);
                    }
                    return resource;
            }
        });

        return url;
    }
    
    /** Called when mapping a native library */
    protected String findLibrary(String name) {
        if (_jclParent != null) {
            return _jclParent.findLibrary(name);
        }

        // Pass through during startup
        if (!_initialized) {
            return super.findLibrary(name);
        }

        name = Config.getInstance().getLibraryPrefix() + name +
                Config.getInstance().getLibrarySufix();

        Trace.println("Looking up native library: " + name, TraceLevel.NETWORK);

        ResourcesDesc rd = _launchDesc.getResources();
        // Get list of all resources
        JARDesc[] jars = rd.getEagerOrAllJarDescs(true);

        //walk through list of native resources checking those that may contain
        //library we need. Check may require loading native jar from network
        for (int i = 0; i < jars.length; i++) {
            if (jars[i].isNativeLib()) {
                //TODO: consult jar index we may be able to skip this jar
                try {
                    String r = DownloadEngine.getLibraryDirForJar(
                            name, jars[i].getLocation(),
                            jars[i].getVersion());
                    if (r != null) {
                        return (new File(r, name)).getAbsolutePath();
                    }
                } catch (IOException ioe) {
                            Trace.ignoredException(ioe);
                }
            }
        }

        Trace.println("Native library " + name + " not found", TraceLevel.NETWORK);
        return super.findLibrary(name);
    }

    protected Class findClass(String name) throws ClassNotFoundException {
        if (!_initialized) {
            return super.findClass(name);
        }

        try {
            return super.findClass(name);
        } catch (ClassNotFoundException cnfe) {
            // try again if checkPackageParts adds anymore urls
            if (checkPackageParts(name)) {
                return super.findClass(name);
            } else {
                throw cnfe;
	    }
        }
    }

    /*
     * Default no-op implementations for Web Start.
     */
    public void quiescenceRequested(Thread thread, boolean initiator) {}
    public void quiescenceCancelled(boolean initiator) {}


    public URL findResource(String name) {
	URL url = super.findResource(name);
	if (!_initialized) return url;
	if (url == null) {
	    // try again if checkPackageParts adds anymore urls
	    if (checkPackageParts(name)) {
	        url = super.findResource(name);
	    }
	}
	return url;
    }

    private boolean checkPackageParts(String name) {
        if (_jclParent != null) {
            return drainPendingURLs();
        }

	// Check if there is a package element for this to guide downloading
	boolean ret = false;
	ResourcesDesc.PackageInformation pi = 
	    _launchDesc.getResources().getPackageInformation(name);
	if (pi != null) {
            JARDesc[] jds = 
		pi.getLaunchDesc().getResources().getPart(pi.getPart());
    	    
            for (int i = 0; i < jds.length; i++) {
		if (_jarsNotInURLClassLoader.contains(jds[i])) {
		    _jarsNotInURLClassLoader.remove(jds[i]);
		    addLoadedJarsEntry(jds[i]);
		    addURL2(jds[i].getLocation());
		    ret = true;
		}
	    }
	}
	return ret;
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
            _appPolicy.addPermissions(JNLPClassLoader.getInstance(), perms, codesource, true);
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
	
            try {
       
                File f = DownloadEngine.getCachedFile(jd.getLocation(), 
                        jd.getVersion());
          
                if (f != null) {
                    String jarPath = f.getPath();
                    perms.add(new FilePermission(jarPath, "read"));
                }
            } catch (IOException ioe) {
                Trace.ignoredException(ioe);
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
                        Object jf = DownloadEngine.getCachedJarFile(
                                jd.getLocation(), jd.getVersion());
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
            _jarsInURLClassLoader.add(jd);
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

    /* addResource(URL resource, String version, String id)
     *
     *     When the DownloadService.loadResource(URL, String) is called, 
     * the given item no longer needs to be listed in the jnlp file, 
     * providing it is at least from the same codebase.
     *     In such a case, this method is used to add the resource to the 
     * JNLPClassLoader, so it can later be loaded.
     */
    public void addResource(URL resource, String version, String id) {
        if (_jclParent != null) {
            _jclParent.addResource(resource, version, id);
            drainPendingURLs();
            return;
        }
        JARDesc jd = new JARDesc(resource, version, true, false, 
				     false, null, 0, null);
	if (!_jarsInURLClassLoader.contains(jd)) {
	    _launchDesc.getResources().addResource(jd);
	    addLoadedJarsEntry(jd);
	    addURL2(resource);
	}
    }

    //---------------------------------------------------------------------------
    // Security-related helper methods
    //

    static boolean setDeployURLClassPathCallbacks(JNLPClassLoader parent, JNLPClassLoader child) {

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
        } catch (ThreadDeath td) {
            throw td;
        } catch (Exception e) {
            return false;
        } catch (Error err) {
            return false;
        }
        return true;
    }

    private static DeployURLClassPath getDUCP(JNLPClassLoader cl) {
	return (DeployURLClassPath)getUCP(cl);
    }
    private static URLClassPath getUCP(JNLPClassLoader cl) {
        URLClassPath ucp = null;
        try {
            // Use reflection to get the package private field "ucp" of URLClassLoader
            ucp = (URLClassPath) ucpField.get(cl);
        } catch (Exception ex) {
        }
        return ucp;
    }

    private static void setUCP(JNLPClassLoader cl, URLClassPath ucp) {
        try {
            // Use reflection to set the package private field "ucp" of URLClassLoader
            ucpField.set(cl, ucp);
        } catch (Exception ex) {
        }
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

    protected void addURL(URL url)
    {
        if (_jclParent != null) {
            /* Also in parent... */
            _jclParent.addURL(url);
        }
        super.addURL(url);
    }

    /* Add a URL to the local class path without forwarding to the parent loader */
    void addURL2(URL url)
    {
        if (_jclParent != null) {
            /* from parent */
            drainPendingURLs();
        } else {
            putAddedURL(url);
        }
        super.addURL(url);
    }

    /* Update child loader with pending previous parent addURLs */
    boolean drainPendingURLs() {
        List addURLs = _jclParent.grabAddedURLs();
        int i;
        for (i=0; i < addURLs.size(); i++) {
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


    //----------------------------------------------------------------------
    // CPCallbackClassLoaderIf methods
    //
    public CodeSource[] getTrustedCodeSources(CodeSource[] sources) {
        List list = new ArrayList();

        Policy policy = (Policy) AccessController.doPrivileged(new PrivilegedAction() {
            public Object run() {
                return Policy.getPolicy();
            }
        });

        for (int i=0; i < sources.length; i++) {
            CodeSource cs = sources[i];
            boolean trusted = false;
            PermissionCollection perms;

            perms = policy.getPermissions(cs);
            /* Get permissions based on JNLP file that defines code resource */
            try {
                trusted = _appPolicy.addPermissions(JNLPClassLoader.getInstance(), perms, cs, true);
            } catch (ExitException ee) {
                Trace.println("_appPolicy.addPermissions: "+ee, TraceLevel.BASIC);
                Trace.ignoredException(ee);
            }

	    if (!trusted) {
		trusted = perms.implies(new AllPermission());
	    }

            if (!trusted) {
                trusted = isTrustedByPolicy(policy, cs);
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
    private boolean isTrustedByPolicy(Policy policy, CodeSource cs) {
        PermissionCollection perms;
        PermissionCollection sandbox;

        perms = policy.getPermissions(cs);
        sandbox = policy.getPermissions(new CodeSource(null, (Certificate[]) null));
        Enumeration e = perms.elements();
        while (e.hasMoreElements()) {
            Permission perm = (Permission)e.nextElement();
            if (!sandbox.implies(perm)) {
                //Trace.println("Plugin2ClassLoader.isTrustedByPolicy extended policy perm " + perm, TraceLevel.BASIC);
                return true;
            }
        }
        return false;
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


