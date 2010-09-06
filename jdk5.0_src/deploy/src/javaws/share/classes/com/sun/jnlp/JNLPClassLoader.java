/*
 * @(#)JNLPClassLoader.java	1.28 04/05/01
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.jnlp;

import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.io.File;
import java.io.FilePermission;
import java.io.InputStream;
import java.io.IOException;
import java.net.URL;
import java.net.URLConnection;
import java.net.URLStreamHandlerFactory;
import java.util.Enumeration;
import java.util.NoSuchElementException;
import java.util.StringTokenizer;
import java.util.jar.Manifest;
import java.util.jar.Attributes;
import java.util.jar.Attributes.Name;
import java.security.PrivilegedAction;
import java.security.PrivilegedExceptionAction;
import java.security.AccessController;
import java.security.AccessControlContext;
import java.security.SecureClassLoader;
import java.security.CodeSource;
import java.security.CodeSigner;
import java.security.Permission;
import java.security.PermissionCollection;
import com.sun.javaws.Main;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.javaws.jnl.JARDesc;
import com.sun.javaws.exceptions.JNLPException;
import com.sun.javaws.Globals;
import com.sun.javaws.cache.Cache;
import com.sun.javaws.LaunchDownload;
import com.sun.javaws.security.JNLPClassPath;
import com.sun.javaws.security.AppPolicy;
import com.sun.javaws.security.Resource;
import java.net.SocketPermission;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.config.Config;

/**
 * This classloader is an adaption of the URLClassLoader from 1.3
 * (orginally by David Connelly)
 *
 */
public final class JNLPClassLoader extends SecureClassLoader {
    
    /* There is only going to be one instance */
    private static JNLPClassLoader _instance = null;
    
    /* Reference to LaunchDesc */
    private LaunchDesc _launchDesc = null;
    
    /* Contains the JNLP ClassPath */
    private JNLPClassPath _jcp = null;
    
    /* Jnlp App policy object */
    private AppPolicy _appPolicy;

    /* The context to be used when loading classes and resources */
    private AccessControlContext _acc = null;

    /* Wheter classloader has been initialized. For 1.4, it gets instantiatd
     * during JVM startup
     */
    private boolean _initialized = false;

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
    public JNLPClassLoader(ClassLoader parent) {
        super(parent);
        // this is to make the stack depth consistent with 1.1
        SecurityManager security = System.getSecurityManager();
        if (security != null) {
	    security.checkCreateClassLoader();
        }
    }

    // ----------------------------------------------------
    // Begin JNLP Specific

    private void initialize(LaunchDesc launchDesc, boolean asApplet, AppPolicy appPolicy) {
        _launchDesc = launchDesc;
        _jcp = new JNLPClassPath(launchDesc, asApplet);
        _acc = AccessController.getContext();
        _appPolicy = appPolicy;
	_initialized = true;
    }
    
   /** Factory method to creatre the JNLPClassloader uninitialized */
    public static synchronized JNLPClassLoader createClassLoader() {
        if (_instance == null) {
            ClassLoader cl;    
 
            cl = ClassLoader.getSystemClassLoader();
            if (cl instanceof JNLPClassLoader) {
                _instance = (JNLPClassLoader)cl;
            } else {
                _instance = new JNLPClassLoader(cl);
            }   
        }
        return _instance;
    }    
    

    /** Factory method to create and/or initialize the JNLPClassloader */
    public static synchronized JNLPClassLoader createClassLoader(LaunchDesc ld, AppPolicy appPolicy) {

	JNLPClassLoader jnlpClassLoader = createClassLoader();

        if (!jnlpClassLoader._initialized) { 
            jnlpClassLoader.initialize(ld, ld.isApplet(), appPolicy);
        }
        return jnlpClassLoader;
    }
    
    /** Get instance */
    public static synchronized JNLPClassLoader getInstance() {
        return _instance;
    }
    
    /** Get main JNLP file */
    public LaunchDesc getLaunchDesc() {
        return _launchDesc;
    }
    
    /*
     *  Methods for downloading parts of the application
     */
    public void downloadResource(URL location, String version, LaunchDownload.DownloadProgress dp, boolean isCacheOk)
        throws JNLPException, IOException {
        LaunchDownload.downloadResource(_launchDesc, location, version, dp, isCacheOk);
    }
    
    public void downloadParts(String[] parts, LaunchDownload.DownloadProgress dp, boolean isCacheOk)
        throws JNLPException, IOException {
        LaunchDownload.downloadParts(_launchDesc, parts, dp, isCacheOk);
    }
    
    public void downloadExtensionParts(URL location, String version, String[] parts, LaunchDownload.DownloadProgress dp, boolean isCacheOk)
        throws JNLPException, IOException {
        LaunchDownload.downloadExtensionPart(_launchDesc, location, version, parts, dp, isCacheOk);
    }
    
    public void downloadEager(LaunchDownload.DownloadProgress dp, boolean isCacheOk)
        throws JNLPException, IOException {
        LaunchDownload.downloadEagerorAll(_launchDesc, false, dp, isCacheOk);
        
    }
    
    /** Returns true, if the JAR file is part of the JNLP codebase */
    public JARDesc getJarDescFromFileURL(URL fileUrl) {
        return _jcp.getJarDescFromFileURL(fileUrl);
    }
    
    /** Returns the default security model for this application */
    public int getDefaultSecurityModel() {
        return _launchDesc.getSecurityModel();
    }
    
    /** Overwrite of standard classLoader method */
    public URL getResource(String name)  {
        // The below code is a workaround a bug in the SocketPermision.implied
        // code. If we are running behind a firewall, and the domain from where the
        // jar file is obtained from is not directly reachable (i.e. only reachable through
        // a proxyserver), the first timearound the SocketPermission will return false. The
        // second time around, the permission check will have degenerated into a textual
        // domain-name check (since we are running with -DtrustProxy=true), and therefor
        // will succeed.
        URL url = null;
        for(int i = 0; url == null && i < 3; i++) {
	    
	    Trace.println("Looking up resource: " + name + " (attempt: " + i + ")", TraceLevel.BASIC);
	    
	    url = super.getResource(name);
        }
        return url;
    }
    
    /** Called when mapping a native library */
    public String findLibrary(String name) {
        // Pass through during startup
	if (!_initialized) return super.findLibrary(name);
	

	name = Config.getInstance().getLibraryPrefix() + name + 
	       Config.getInstance().getLibrarySufix();
        
	Trace.println("Looking up native library: " + name, TraceLevel.BASIC);
        
        /* This might return different results, depending on what downloads has happended */
        File[] dirs = LaunchDownload.getNativeDirectories(_launchDesc);
        for(int i = 0; i < dirs.length; i++) {
	    File f = new File(dirs[i], name);
	    if (f.exists()) {
		
		Trace.println("Native library found: " + f.getAbsolutePath(), TraceLevel.BASIC);
		
		return f.getAbsolutePath();
	    }
        }
        
	Trace.println("Native library not found", TraceLevel.BASIC);
        
        return super.findLibrary(name);
    }
    
    // End JNLP Specific
    // ----------------------------------------------------
    
    /**
     * Finds and loads the class with the specified name from the URL search
     * path. Any URLs referring to JAR files are loaded and opened as needed
     * until the class is found.
     *
     * @param name the name of the class
     * @return the resulting class
     * @exception ClassNotFoundException if the class could not be found
     */
    protected Class findClass(final String name) throws ClassNotFoundException {
        // Pass through during startup
	if (!_initialized) return super.findClass(name);

       
	Trace.println("Loading class " + name, TraceLevel.BASIC);
        
        try {
	    return (Class)
		AccessController.doPrivileged(new PrivilegedExceptionAction() {
			public Object run() throws ClassNotFoundException {
			    String path = name.replace('.', '/').concat(".class");
			    Resource res = _jcp.getResource(path, false);
			    if (res != null) {
				try {
				    return defineClass(name, res);
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
     * Defines a Class using the class bytes obtained from the specified
     * Resource. The resulting Class must be resolved before it can be
     * used.
     */
    private Class defineClass(String name, Resource res) throws IOException {
        int i = name.lastIndexOf('.');
        URL url = res.getCodeSourceURL();
        if (i != -1) {
	    String pkgname = name.substring(0, i);
	    // Check if package already loaded.
	    Package pkg = getPackage(pkgname);
	    Manifest man = res.getManifest();
	    if (pkg != null) {
		// Package found, so check package sealing.
		boolean ok;
		if (pkg.isSealed()) {
		    // Verify that code source URL is the same.
		    ok = pkg.isSealed(url);
		} else {
		    // Make sure we are not attempting to seal the package
		    // at this code source URL.
		    ok = (man == null) || !isSealed(pkgname, man);
		}
		if (!ok) {
		    throw new SecurityException("sealing violation");
		}
	    } else {
		if (man != null) {
		    definePackage(pkgname, man, url);
		} else {
		    definePackage(pkgname, null, null, null, null, null, null, null);
		}
	    }
        }
        // Now read the class bytes and define the class
        byte[] b = res.getBytes();
        CodeSource cs;
        // must read certificates AFTER reading bytes.
        if (Globals.isJavaVersionAtLeast15()) {
            // 1.5 and later - use CodeSigner[] constructor
            cs = new CodeSource(url, res.getCodeSigners());
        } else {
            // pre 1.5 - use Certificate[] constructor
            cs = new CodeSource(url, res.getCertificates());
        }
        return defineClass(name, b, 0, b.length, cs);
    }
    
    /**
     * Defines a new package by name in this ClassLoader. The attributes
     * contained in the specified Manifest will be used to obtain package
     * version and sealing information. For sealed packages, the additional
     * URL specifies the code source URL from which the package was loaded.
     *
     * @param name  the package name
     * @param man   the Manifest containing package version and sealing
     *              information
     * @param url   the code source url for the package, or null if none
     * @exception   IllegalArgumentException if the package name duplicates
     *              an existing package either in this class loader or one
     *              of its ancestors
     * @return the newly defined Package object
     */
    protected Package definePackage(String name, Manifest man, URL url)
        throws IllegalArgumentException
    {
        String path = name.replace('.', '/').concat("/");
        String specTitle = null, specVersion = null, specVendor = null;
        String implTitle = null, implVersion = null, implVendor = null;
        String sealed = null;
        URL sealBase = null;
        
        Attributes attr = man.getAttributes(path);
        if (attr != null) {
	    specTitle   = attr.getValue(Name.SPECIFICATION_TITLE);
	    specVersion = attr.getValue(Name.SPECIFICATION_VERSION);
	    specVendor  = attr.getValue(Name.SPECIFICATION_VENDOR);
	    implTitle   = attr.getValue(Name.IMPLEMENTATION_TITLE);
	    implVersion = attr.getValue(Name.IMPLEMENTATION_VERSION);
	    implVendor  = attr.getValue(Name.IMPLEMENTATION_VENDOR);
	    sealed      = attr.getValue(Name.SEALED);
        }
        attr = man.getMainAttributes();
        if (attr != null) {
	    if (specTitle == null) {
		specTitle = attr.getValue(Name.SPECIFICATION_TITLE);
	    }
	    if (specVersion == null) {
		specVersion = attr.getValue(Name.SPECIFICATION_VERSION);
	    }
	    if (specVendor == null) {
		specVendor = attr.getValue(Name.SPECIFICATION_VENDOR);
	    }
	    if (implTitle == null) {
		implTitle = attr.getValue(Name.IMPLEMENTATION_TITLE);
	    }
	    if (implVersion == null) {
		implVersion = attr.getValue(Name.IMPLEMENTATION_VERSION);
	    }
	    if (implVendor == null) {
		implVendor = attr.getValue(Name.IMPLEMENTATION_VENDOR);
	    }
	    if (sealed == null) {
		sealed = attr.getValue(Name.SEALED);
	    }
        }
        if ("true".equalsIgnoreCase(sealed)) {
	    sealBase = url;
        }
        return definePackage(name, specTitle, specVersion, specVendor,
			     implTitle, implVersion, implVendor, sealBase);
    }
    
    /*
     * Returns true if the specified package name is sealed according to the
     * given manifest.
     */
    private boolean isSealed(String name, Manifest man) {
        String path = name.replace('.', '/').concat("/");
        Attributes attr = man.getAttributes(path);
        String sealed = null;
        if (attr != null) {
	    sealed = attr.getValue(Name.SEALED);
        }
        if (sealed == null) {
	    if ((attr = man.getMainAttributes()) != null) {
		sealed = attr.getValue(Name.SEALED);
	    }
        }
        return "true".equalsIgnoreCase(sealed);
    }
    
    /**
     * Finds the resource with the specified name on the URL search path.
     *
     * @param name the name of the resource
     * @return a <code>URL</code> for the resource, or <code>null</code>
     * if the resource could not be found.
     */
    public URL findResource(final String name) {
        // Pass through during startup
	if (!_initialized) return super.findResource(name);

        /*
	 * The same restriction to finding classes applies to resources
	 */
        Resource res =
	    (Resource) AccessController.doPrivileged(new PrivilegedAction() {
		    public Object run() {
			return _jcp.getResource(name, true);
		    }
		}, _acc);
        return res != null ? _jcp.checkURL(res.getURL()) : null;
    }
    
    /**
     * Returns an Enumeration of URLs representing all of the resources
     * on the URL search path having the specified name.
     *
     * @param name the resource name
     * @exception if an I/O exception occurs
     * @return an <code>Enumeration</code> of <code>URL</code>s
     */
    public Enumeration findResources(final String name) throws IOException {
        // Pass through during startup
	if (!_initialized) return super.findResources(name);

        final Enumeration e = (Enumeration) AccessController.doPrivileged(new PrivilegedAction() {
		    public Object run() {
			return _jcp.getResources(name, true);
		    }
		}, _acc);
                
        return new Enumeration() {
	    private URL res;
	    
	    public Object nextElement() {
		if (res == null)
		    throw new NoSuchElementException();
		URL url = res;
		res = null;
		return url;
	    }
	    
	    public boolean hasMoreElements() {
		if (Thread.currentThread().getThreadGroup() == 
			Main.getSecurityThreadGroup()) {
		    return false;
		}
		if (res != null)
		    return true;
		do {
		    Resource r = (Resource)
			AccessController.doPrivileged(new PrivilegedAction() {
				public Object run() {
				    if (!e.hasMoreElements())
					return null;
				    return e.nextElement();
				}
			    }, _acc);
		    if (r == null)
			break;
		    res = _jcp.checkURL(r.getURL());
		} while (res == null);
		return res != null;
	    }
        };
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

        /** This reads the permissions from the default policy file, such
         *  as the .java.policy file. It it kind of questionable if this
         *  is the right thing to do. This would actually allow the policies
         *  used by Java Web Start to be overwritten using the .java.policy
         *  file. Which some people want to do. However, this is a change in direction
         *  from the 1.0 implementation - which was very strict.
         */
        PermissionCollection perms = super.getPermissions(codesource);
        
        /* Get permissions based on JNLP file that defines code resource */
        _appPolicy.addPermissions(perms, codesource);
        
        URL url = codesource.getLocation();
        
        Permission p;
        
        try {
	    p = url.openConnection().getPermission();
        } catch (java.io.IOException ioe) {
	    p = null;
        }
        
	// Get the URL this JAR file and add permissions to read all JARs that are
	// retreived from the same host. This is to provide the same semantics as
	// the SocketPermission gives for HTTP URLs
	JARDesc jd = _jcp.getJarDescFromFileURL(url);
	if (jd != null) {
	    String paths[] = Cache.getBaseDirsForHost(jd.getLocation());
	    for (int i=0; i<paths.length; i++) {
		String path = paths[i];
		if (path != null) {
	            if (path.endsWith(File.separator)) {
		        path += '-';
	            } else {
		        path += File.separator + '-';
	            }
	            perms.add(new FilePermission(path, "read"));
		}
	    }
	}
	
	if (p instanceof FilePermission) {
	    // if the permission has a separator char on the end,
	    // it means the codebase is a directory, and we need
	    // to add an additional permission to read recursively
	    String path = p.getName();
	    if (path.endsWith(File.separator)) {
		path += "-";
		p = new FilePermission(path, "read");
	    }
	} else if ((p == null) && (url.getProtocol().equals("file"))) {
	    String path = url.getFile().replace('/', File.separatorChar);
	    if (path.endsWith(File.separator))
		path += "-";
	    p =  new FilePermission(path, "read");
	} else {
	    String host = url.getHost();
	    if (host == null)
		host = "localhost";
	    p = new SocketPermission(host,"connect, accept");
	}
	
	// make sure the person that created this class loader
	// would have this permission
	if (p != null) {
	    final SecurityManager sm = System.getSecurityManager();
	    if (sm != null) {
		final Permission fp = p;
		AccessController.doPrivileged(new PrivilegedAction() {
                    public Object run() throws SecurityException {
				sm.checkPermission(fp);
				return null;
			    }
                }, _acc);
	    }
	    perms.add(p);
	}
	
	// #5012841 [AppContext security permissions for untrusted clipboard access]
	//
	// If permissions do not imply clipboard permission, app is untrusted for clipboard access
	//
	if (perms.implies(new java.awt.AWTPermission("accessClipboard")) == false)	
	    sun.awt.AppContext.getAppContext().put("UNTRUSTED_CLIPBOARD_ACCESS_KEY", Boolean.TRUE);
	
	return perms;
    }

    // from java.net.URLClassLoader
    public final synchronized Class loadClass(String name, boolean resolve)
	throws ClassNotFoundException
    {
	// First check if we have permission to access the package. This
	// should go away once we've added support for exported packages.
	SecurityManager sm = System.getSecurityManager();
	if (sm != null) {
	    int i = name.lastIndexOf('.');
	    if (i != -1) {
		sm.checkPackageAccess(name.substring(0, i));
	    }
	}
	return super.loadClass(name, resolve);
    }
}


