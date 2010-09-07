/*
 * @(#)JNLP2ClassLoader.java	1.23 10/03/24
 * 
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.plugin2.applet;

import java.io.File;
import java.io.FilePermission;
import java.io.IOException;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.jar.JarFile;
import java.util.ArrayList;
import java.util.Properties;
import java.security.PrivilegedAction;
import java.security.AccessController;
import java.security.CodeSource;
import java.security.PermissionCollection;
import com.sun.jnlp.DownloadService2Impl;
import com.sun.jnlp.IntegrationServiceImpl;
import com.sun.jnlp.JNLPClassLoaderIf;
import com.sun.jnlp.JNLPPreverifyClassLoader;
import com.sun.jnlp.ExtendedServiceImpl;
import com.sun.jnlp.FileOpenServiceImpl;
import com.sun.jnlp.PersistenceServiceImpl;
import com.sun.jnlp.ClipboardServiceImpl;
import com.sun.jnlp.ExtensionInstallerServiceImpl;
import com.sun.jnlp.FileSaveServiceImpl;
import com.sun.jnlp.PrintServiceImpl;
import com.sun.jnlp.DownloadServiceImpl;
import com.sun.jnlp.SingleInstanceServiceImpl;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.javaws.jnl.JARDesc;
import com.sun.javaws.jnl.ResourcesDesc;
import com.sun.javaws.exceptions.JNLPException;
import com.sun.javaws.exceptions.ExitException;
import com.sun.javaws.LaunchDownload;
import com.sun.javaws.security.AppPolicy;

import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.config.Config;
import com.sun.deploy.cache.Cache;
import com.sun.deploy.cache.CacheEntry;
import com.sun.deploy.net.DownloadEngine;
import com.sun.deploy.perf.DeployPerfUtil;
import com.sun.javaws.progress.ProgressListener;
import com.sun.deploy.util.NativeLibraryBundle;

import sun.reflect.Reflection;



/**
 * This classloader is an extension of the URLClassLoader
 *
 * FIXME: keep in sync with com.sun.jnlp.JNLPClassLoader
 */
public final class JNLP2ClassLoader extends Plugin2ClassLoader implements JNLPClassLoaderIf {
    
    /* Reference to LaunchDesc */
    private LaunchDesc _launchDesc = null;
    
    /* Jnlp App policy object */
    private AppPolicy _appPolicy;

    /* Wether classloader has been initialized. For 1.4, it gets instantiatd
     * during JVM startup
     */
    private boolean _initialized;

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

    private JNLPPreverifyClassLoader _pcl = null;

    /* Trusted parent JNLP class loader */
    private JNLP2ClassLoader _jclParent;

    /**
     * Constructs a new JNLP2ClassLoader which provides no services until
     * a later call to a private JNLP-specific initialize() method.
     *
     * <p>If there is a security manager, this method first
     * calls the security manager's <code>checkCreateClassLoader</code> method
     * to ensure creation of a class loader is allowed.
     * This is done implicit within URLClassLoader!
     * We also use URLClassLoader's default parent, ClassLoader.getSystemClassLoader()
     *
     * @see URLClassLoader#URLClassLoader
     */
    protected JNLP2ClassLoader(URL base, JNLPPreverifyClassLoader parent) {
        super(new URL[0], base, parent);
        _pcl = parent;
        _initialized=false;
        if ( DEBUG ) {
            Trace.println("JNLP2ClassLoader: cstr ...", TraceLevel.BASIC);
        }
    }

    protected JNLP2ClassLoader(URL base, ClassLoader parent) {
        super(new URL[0], base, parent);
        _initialized=false;
        if (parent instanceof JNLP2ClassLoader) {
            _jclParent = (JNLP2ClassLoader)parent;
        }
        if ( DEBUG ) {
            Trace.println("JNLP2ClassLoader: cstr ...", TraceLevel.BASIC);
        }
    }

    private void setDelegatingClassLoader(JNLP2ClassLoader cl) {
	if (_pcl != null) {
            _pcl.setDelegatingClassLoader(cl);
	    _delegatingClassLoader = cl;
	}
    }
        
    protected void initialize(LaunchDesc launchDesc, AppPolicy appPolicy) 
    {
        if ( DEBUG ) {
            Trace.println("JNLP2ClassLoader: initialize ...", TraceLevel.BASIC);
        }

        _launchDesc = launchDesc;
        _appPolicy = appPolicy;
        _initialized = true;

	if (_jclParent != null) {
            _jclParent.initialize(launchDesc, appPolicy);
	    _jclParent.setDelegatingClassLoader(this);
	    drainPendingURLs();
	    return;
	}

	if (_pcl != null) {
            _pcl.initialize(launchDesc, appPolicy);
	    _pcl.setDelegatingClassLoader(this);
	}
        
        // Add all JARDescs for application to list */
        ResourcesDesc rd = launchDesc.getResources();
        if (rd != null) {
            // Extract list of URLs from Codebase
            sortJarDescriptors(rd);
            for (int i = 0; i<_jarsInURLClassLoader.size(); i++) {
                JARDesc jd = (JARDesc) _jarsInURLClassLoader.get(i);
                if ( DEBUG ) {
                        Trace.println("\t addURL: "+jd.getLocation(), TraceLevel.BASIC);
                }

                //add url unless it is part of preverify classloader
                if (_pcl == null || !_pcl.contains(jd.getLocation())) {
                    addURL2(jd.getLocation());
                }               
            }
        }
        if ( DEBUG ) {
            Trace.println("JNLP2ClassLoader: initialize done", TraceLevel.BASIC);
        }
    }

    //----------------------------------------------------------------------
    // ClassLoader implementations
    //

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
                        resource = JNLP2ClassLoader.super.getResource(name);
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
	if (_jclParent != null) {
	    return _jclParent.findLibrary(name);
	}

	DeployPerfUtil.put("JNLP2ClassLoader.findLibrary - start()");
        // Pass through during startup
        if (!_initialized) {
            Trace.println("JNLP2ClassLoader.findLibrary: "+name+": not initialized -> super()", TraceLevel.BASIC);
            return super.findLibrary(name);
        }

        name = Config.getInstance().getLibraryPrefix() + name + 
               Config.getInstance().getLibrarySufix();

        Trace.println("JNLP2ClassLoader.findLibrary: Looking up native library: " + name, TraceLevel.BASIC);

        synchronized (this) {
            // See whether we already know about it
            if (nativeLibraries != null) {
                String fullPath = (String) nativeLibraries.get(name);
                if (fullPath != null) {
                    Trace.println("JNLP2ClassLoader.findLibrary: native library found: " + fullPath, TraceLevel.BASIC);
                    DeployPerfUtil.put("JNLP2ClassLoader.findLibrary - reusing library");
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
                        JarFile jf = DownloadEngine.getCachedJarFile(
                                jars[i].getLocation(), jars[i].getVersion());
                        nativeLibraries.prepareLibrary(name, jf, dir);
                        String fullPath =  nativeLibraries.get(name);
                        Trace.println("JNLP2ClassLoader.findLibrary: native library found: " + fullPath, TraceLevel.BASIC);
                        DeployPerfUtil.put("JNLP2ClassLoader.findLibrary - found library");
                        return fullPath;
                    }
                } catch (IOException ioe) {
                            Trace.ignoredException(ioe);
                }
            }
        }
        
        Trace.println("Native library "+name+" not found", TraceLevel.BASIC);
	DeployPerfUtil.put("JNLP2ClassLoader.findLibrary - return super.findLibrary");
        return super.findLibrary(name);
    }

    //----------------------------------------------------------------------
    // URLClassLoader implementations
    //

    private boolean processingException = false;


    // Can't happen since we override loadClass() in Plugin2ClassLoader
    protected Class findClass(String name) throws ClassNotFoundException {
	throw new ClassNotFoundException("can't happen");
	// return findClass(name, false);
    }
   
    /*
     * Try to load a class from our search path. As a last resort, see if we
     * are being called via Class.forName() as a workaround for legacy JNLP
     * applet loading mechanisms in FX and JOGL. We only need to check for
     * this when called directly (not via class loader delegation).
     */
    protected Class findClass(String name, boolean delegated)  throws ClassNotFoundException {
        if (!_initialized) {
            Trace.println("JNLP2ClassLoader.findClass: "+name+": not initialized -> super()", TraceLevel.BASIC);
            return super.findClass(name);
        }

        try {
            return findClassHelper(name);
        } catch (ClassNotFoundException cnfe) {
            // try again if checkPackageParts adds anymore urls
            Trace.println("JNLP2ClassLoader.findClass: "+name+": try again ..", TraceLevel.BASIC);
            if (checkPackageParts(name)) {
                return findClassHelper(name);
            }
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
			JNLPPreverifyClassLoader.DelegatingThread dt;
			dt = new JNLPPreverifyClassLoader.DelegatingThread(_delegatingClassLoader, this);
			dt.start();
			while (!dt.done()) {
			    try {
			        this.wait();
			    } catch (InterruptedException ie) {
				throw cnfe;
			    }
			}
		        return _delegatingClassLoader.loadClass(name);
                    } finally {
			boolean interrupted = false;
			JNLPPreverifyClassLoader.UndelegatingThread udt;
			udt = new JNLPPreverifyClassLoader.UndelegatingThread(_delegatingClassLoader, this);
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
            throw cnfe;
        }
    }

    public URL findResource(String name) {
        URL url = super.findResource(name);
        if (!_initialized) {
            Trace.println("JNLP2ClassLoader.findResource: "+name+": not initialized -> super()", TraceLevel.BASIC);
            return url;
        }
        if (url == null) {
            // try again if checkPackageParts adds anymore urls
            if (checkPackageParts(name)) {
                url = super.findResource(name);
            }
        }
        return url;
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
    protected PermissionCollection getPermissions(CodeSource codesource) 
    {
        Trace.println("JNLP2ClassLoader.getPermissions() ..", TraceLevel.BASIC);

        /** 
        *  This reads the permissions from the default policy file, such
        *  as the .java.policy file. It it kind of questionable if this
        *  is the right thing to do. This would actually allow the policies
        *  used by Java Web Start to be overwritten using the .java.policy
        *  file. Which some people want to do. 
        */
        PermissionCollection perms = super.getPermissions(codesource);

        // NOTE that we do NOT grant permissions based on the contents
        // of the JNLP file. We instead grant permissions based ONLY
        // on trusting the signing of the code, just like normal
        // applets. This allows us to skip the checking of the JNLP
        // security and allows us to create, for example, unsigned
        // applets which pull in signed extensions from other hosts.
        try {
            _appPolicy.addPermissions(this, perms, codesource, false);
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
                    JarFile jf = DownloadEngine.getCachedJarFile(
                            jd.getLocation(), jd.getVersion());
                    if (jf != null) {
                        String jarPath = jf.getName();
                        perms.add(new FilePermission(jarPath, "read"));
                    }
                } catch (IOException ioe) {
                    Trace.ignoredException(ioe);
                }
        }
	
        Trace.println("JNLP2ClassLoader.getPermissions() X", TraceLevel.BASIC);
        return perms;
    }

    //----------------------------------------------------------------------
    // JNLP helpers
    //

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

    /** Returns the default security model for this application */
    public int getDefaultSecurityModel() {
        return _launchDesc.getSecurityModel();
    }
    
    /**
     * Try to find a package part by name and addURL it.
     *
     * If there is no such entity, go through the pending _jarsNotInURLClassLoader list,
     * and addURL 'them, if not yet done.
     */
    private boolean checkPackageParts(String name) 
    {
        // Check if there is a package element for this to guide downloading
        boolean ret = false;

	if (_jclParent != null) {
	    return drainPendingURLs();
	}

        try {
            JARDesc[] jds = null;

            ResourcesDesc resources = _launchDesc.getResources();
            ResourcesDesc.PackageInformation pi = resources.getPackageInformation(name);

            if (pi != null) {
                jds = pi.getLaunchDesc().getResources().getPart(pi.getPart());
            }

            if (jds!=null)
            {
                for (int i = 0; i < jds.length; i++) {
                    if (_jarsNotInURLClassLoader.contains(jds[i])) {
                        _jarsNotInURLClassLoader.remove(jds[i]);
                        if (!_jarsInURLClassLoader.contains(jds[i])) {
                            _jarsInURLClassLoader.add(jds[i]);
                            addURL2(jds[i].getLocation());
                            ret = true;
                        }
                    }
                }
            }
        } catch (Exception e) {
            Trace.ignoredException(e);
        }
        return ret;
    }


    private void sortJarDescriptors(ResourcesDesc rd) {

        ArrayList lazyJarDescArrayList = new ArrayList();

        JARDesc [] allJarDescs = rd.getEagerOrAllJarDescs(true);
        JARDesc main = rd.getMainJar(true);
        JARDesc progress = rd.getProgressJar();

        /* if there is a progress jar, it is first */
        if (progress != null) {
            _jarsInURLClassLoader.add(progress);
        }

        /* then add main jar */
        if (main != null) {
            _jarsInURLClassLoader.add(main);
        }

        for (int i=0; i<allJarDescs.length; i++) {
            if (allJarDescs[i] != main && allJarDescs[i] != progress) {
                if ((!allJarDescs[i].isLazyDownload())) {
                    /* second add other eager jars */
                    _jarsInURLClassLoader.add(allJarDescs[i]);
                } else if (!rd.isPackagePart(allJarDescs[i].getPartName())) {
                    lazyJarDescArrayList.add(allJarDescs[i]);
                } else {
                    /* add those with package parts to seperate list */
                    _jarsNotInURLClassLoader.add(allJarDescs[i]);
                }
            }
        }

        /* third add the lazy jars w/o package parts */
        for (int i=0; i<lazyJarDescArrayList.size(); i++) {
            _jarsInURLClassLoader.add(lazyJarDescArrayList.get(i));
        }
    }

    /**
     * update the Jar descriptors, incl. addURL
     *
     * necessary, if the JARDesc list has altered, e.g. an extension is loaded
     */
    protected void updateJarDescriptors(ResourcesDesc rd)
    {
	if (_jclParent != null) {
	    _jclParent.updateJarDescriptors(rd);
	    drainPendingURLs();
	    return;
	}

        JARDesc [] jds = rd.getEagerOrAllJarDescs(true);

        if (jds!=null)
        {
            for (int i = 0; i < jds.length; i++) {
                updateJarDescriptor(rd, jds[i]);
            }
        }
    }

    protected void updateJarDescriptor(ResourcesDesc rd, JARDesc jd)
    {
        if (jd!=null)
        {
            if ( !_jarsInURLClassLoader.contains(jd)    &&
                 !_jarsNotInURLClassLoader.contains(jd)    )
            {
                if ( !jd.isLazyDownload() ||
                     !rd.isPackagePart(jd.getPartName()) ) 
                {
                    // add: eager jars OR those without package parts
                    _jarsInURLClassLoader.add(jd);
                    addURL2(jd.getLocation());
                } else {
                    // add: non eager jars AND those with package parts
                    _jarsNotInURLClassLoader.add(jd);
                }
            }
        }
    }

    /* addResource(URL resource, String version, String id)
     *
     *     When the DownloadService.loadResource(URL, String) is called, 
     * the given item no longer needs to be listed in the jnlp file, 
     * providing it is at least from the same codebase.
     *     In such a case, this method is used to add the resource to the 
     * JNLP2ClassLoader, so it can later be loaded.
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
            _jarsInURLClassLoader.add(jd);
            addURL2(resource);
        }
    }

    //----------------------------------------------------------------------
    // JNLP Service interface
    //
    javax.jnlp.BasicService _basicService = null;

    // provides an instance of BasicService within this Classloader
    public javax.jnlp.BasicService getBasicService() {
        if ( _basicService == null ) {
            // If there is a Codebase in the jnlp file, the BasicService.getCodebase() needs
            // to use it, otherwise applications cannot generate URLs to use in the 
            // DownloadService API, or patterns to use in the DownloadService2 APIs.
            // both these APIs use the codebase from the jnlp file to determine if applet
            // has access to specific URLs.
            // If there is no codebase in the jnlp file, use the DoccumentBase as before.
            URL codebase = _launchDesc.getCodebase();
            if (codebase == null) {
                codebase = base;
            }
            _basicService = new Plugin2BasicService(codebase);
        }
        return _basicService;
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


