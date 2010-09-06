/*
 * @(#)JNLPClassPath.java	1.22 04/02/03
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.security;

import java.util.Enumeration;
import java.util.NoSuchElementException;
import java.util.Stack;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.jar.JarFile;
import java.util.jar.JarEntry;
import java.util.jar.Manifest;
import java.util.jar.Attributes;
import java.util.jar.Attributes.Name;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLConnection;
import java.net.HttpURLConnection;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.InputStream;
import java.io.IOException;
import java.security.AccessControlException;
import java.security.Permission;
import java.security.cert.Certificate;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.javaws.jnl.ResourcesDesc;
import com.sun.javaws.jnl.JARDesc;
import com.sun.javaws.util.URLUtil;
import com.sun.javaws.cache.DownloadProtocol;
import com.sun.javaws.cache.DiskCacheEntry;
import com.sun.javaws.exceptions.JNLPException;
import com.sun.javaws.Globals;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;

/**
 * This class is used to maintain a search path of URLs for loading classes
 * and resources from both a JNLP file. (This code is adapted from original
 * URLClasspath class written by David Connelly)
 */
public class JNLPClassPath {
    
    /* List of all JAR descs that does not have a loader */
    private Stack _pendingJarDescs = new Stack();
    
    /* The current classpath. This path gets expanded as requests comes in */
    private ArrayList _loaders = new ArrayList();
    
    /* Applet Loader if needed. This is kept separate, so we always search this one last */
    private Loader _appletLoader = null;
    
    /* JNLP file */
    private LaunchDesc _launchDesc = null;
    
    /* File URLs to JARDesc mapping. This is currently needed for the AppPolicy to map
     * back to JARDescs
     */
    private HashMap _fileToUrls = new HashMap();
    
    /**
     * Creates a new URLClassPath for the given URLs. The URLs will be
     * searched in the order specified for classes and resources. A URL
     * ending with a '/' is assumed to refer to a directory. Otherwise,
     * the URL is assumed to refer to a JAR file.
     *
     * @param urls the directory and JAR file URLs to search for classes
     *        and resources
     * @param factory the URLStreamHandlerFactory to use when creating new URLs
     */
    public JNLPClassPath(LaunchDesc launchDesc, boolean asApplet) {
        _launchDesc = launchDesc;
        
        /** In order to be compatible with Applets, loading from the codebase
	 *  must be supported. This is rather unfortunatle, since if prevents
	 *  Applets from working offline
	 */
        if (asApplet) {
	    URL appletCodebase = URLUtil.getBase(launchDesc.getCanonicalHome());
	    Trace.println("Classpath: " + appletCodebase, TraceLevel.BASIC);
	    if ("file".equals(appletCodebase.getProtocol())) {
		_appletLoader = new FileDirectoryLoader(appletCodebase);
	    } else {
		_appletLoader = new URLDirectoryLoader(appletCodebase);
	    }
	    
        }
        
        // Add all JARDescs for application to list */
        ResourcesDesc cd = launchDesc.getResources();
        if (cd != null) {
	    // Extract list of URLs from Codebase
	    JARDesc[] jars = cd.getEagerOrAllJarDescs(true);
	    // Add jars in reverse order, since we pop them off
	    for(int i = jars.length -1 ; i >= 0; i--) {
		if (jars[i].isJavaFile()) {
		    Trace.println("Classpath: " + jars[i].getLocation() + ":" + jars[i].getVersion(), TraceLevel.BASIC);
		    _pendingJarDescs.add(jars[i]);
		}
	    }
        }
    }
    
    /** Find the JARDesc element that is behind a particular File URL */
    public synchronized JARDesc getJarDescFromFileURL(URL fileUrl) {
        return (JARDesc)_fileToUrls.get(fileUrl.toString());
    }
    
    /** Load all resources for application */
    private void loadAllResources() {
        try {
	    JARDesc jd = getNextPendingJarDesc();
	    while(jd != null) {	    
		createLoader(jd);
		jd = getNextPendingJarDesc();
	    }
        } catch(IOException ioe) {
	    Trace.ignoredException(ioe);
        }
        
        // Add Applet loader last, so getResources will also return it	
	if (_appletLoader != null) {
	    synchronized(_loaders) { _loaders.add(_appletLoader); } 	
	}
    }
    
    /** Returns next pending JAR desc. Returns null if none left */
    private synchronized JARDesc getNextPendingJarDesc() {
	return _pendingJarDescs.isEmpty() ? null :(JARDesc)_pendingJarDescs.pop();	
    }        
    
    /** Returns JAR desc if it is pending, otherwise null */
    private synchronized JARDesc getIfPendingJarDesc(JARDesc jd) {
	if (_pendingJarDescs.contains(jd)) {
	    _pendingJarDescs.remove(jd);
	    return jd;
	}
	return null;
    }        
    
    /** Create a loader for a particular resource */
    private Loader createLoader(final JARDesc jd) throws IOException {
        try {
	    return (Loader)java.security.AccessController.doPrivileged(new java.security.PrivilegedExceptionAction() {
			public Object run() throws IOException {
			    return createLoaderHelper(jd);
			}
		    });
        } catch (java.security.PrivilegedActionException pae) {
	    Trace.println("Failed to create loader for: " + jd + " (" + pae.getException() + ")", TraceLevel.BASIC);
	    throw (IOException)pae.getException();
        }
        
    }
    
    /** Executed in a doPrivileged */
    private Loader createLoaderHelper(JARDesc jd) throws IOException {
        URL location = jd.getLocation();
        String version = jd.getVersion();
        
        try {
	    DiskCacheEntry dce = DownloadProtocol.getResource(location, version, DownloadProtocol.JAR_DOWNLOAD, true, null);
	    
	    if (dce == null || !dce.getFile().exists()) {
		throw new IOException("Resource not found: " + jd.getLocation() + ":" + jd.getVersion());
	    }
	    
	    String path = URLUtil.getEncodedPath(dce.getFile());
	    final URL fileUrl = new URL("file", "", path);


	    Trace.println("Creating loader for: " + fileUrl, TraceLevel.BASIC);
	    Loader loader = new JarLoader(fileUrl);
	    // Update lists
	    synchronized(this) {
		_loaders.add(loader);	    	    
		// Keep mapping from fileURL to JarDesc element
		_fileToUrls.put(fileUrl.toString(), jd);
	    }
	    
	    return loader;
	    
        } catch(JNLPException je) {
	    Trace.println("Failed to download: " + je + " (" + je + ")", TraceLevel.BASIC);
	    Trace.ignoredException(je);
	    throw new IOException(je.getMessage());
        }
    }
    
    /** Find a resource with a particular name */
    private Resource findNamedResource(String name, boolean check) throws IOException {
        // Check if element can be found in an already existing loader
        Resource res = findNamedResourceInLoaders(name, check);
        if (res != null) return res;
        
        // Is all loaders created?
	synchronized(this) {
	    if (_pendingJarDescs.isEmpty()) return null;
	}
        
        // Check if there is a package element for this to guide downloading
        ResourcesDesc.PackageInformation pi = _launchDesc.getResources().getPackageInformation(name);
        if (pi != null) {
	    // Get all resources for that particular part
	    JARDesc[] jds = pi.getLaunchDesc().getResources().getPart(pi.getPart());
	    // Create a loader for each resource
	    for(int i = 0; i < jds.length; i++) {
		// Synchonize so each pending JARDesc is only added ones
		JARDesc jd = getIfPendingJarDesc(jds[i]);
		if (jd != null) createLoader(jd);
	    }
	    // Do another lookup
	    res = findNamedResourceInLoaders(name, check);
	    if (res != null) return res;
        }
        
        // Otherwise, load resources until we find the particular one
	JARDesc jd = getNextPendingJarDesc();
        while(jd != null) {	    
	    Loader loader = createLoader(jd);
	    res = loader.getResource(name, check);
	    if (res != null) {
		return res;
	    }
	    // Get next one
	    jd = getNextPendingJarDesc();
        }
        
        // Finally try AppletLoader if needed
        if (_appletLoader != null) {
	    res = _appletLoader.getResource(name, check);
        }
        
        return res;
    }
    
    private Resource findNamedResourceInLoaders(String name, boolean check) throws IOException {
	int size = 0;
	// Make sure to read size when array is not being updated
	synchronized(this) { size = _loaders.size(); }	
        for(int i = 0; i < size; i++) {	    
	    Loader loader = null;
	    synchronized(this) { loader = (Loader)_loaders.get(i); }
	    Resource res = loader.getResource(name, check);
	    if (res != null) return res;
        }
        return null;
    }
    
    /**
     * Finds the first Resource on the URL search path which has the specified
     * name. Returns null if no Resource could be found.
     *
     * @param name the name of the Resource
     * @return the Resource, or null if not found
     */
    public Resource getResource(String name, boolean check) {
      
	Trace.println("getResource: " + name + " (check: " + check + ")", TraceLevel.BASIC);
        
        try {
	    return findNamedResource(name, check);
        } catch(IOException ioe) {
	    Trace.ignoredException(ioe);
	    return null;
        }
    }
    
    public Resource getResource(String name) {
        return getResource(name, true);
    }
    
    /**
     * Finds all resources on the URL search path with the given name.
     * Returns an enumeration of the Resource objects.
     *
     * @param name the resource name
     * @return an Enumeration of all the resources having the specified name
     */
    public Enumeration getResources(final String name,
				    final boolean check) {
        // Load all resources
        loadAllResources();
	// Read max size of loaders
	int s;
	synchronized(this) { s = _loaders.size(); }
	final int size = s;	
	
	// Scan all loaders
	return new Enumeration() {
	    private int index = 0;
	    private Resource res = null;
	    
	    private boolean next() {
		if (res != null) {
		    return true;
		} else {
		    Loader loader;		    
		    while (index < size) {
			loader = (Loader)_loaders.get(index++);
			res = loader.getResource(name, check);
			if (res != null) {
			    return true;
			}
		    }
		    return false;
		}
	    }
	    
	    public boolean hasMoreElements() {
		return next();
	    }
	    
	    public Object nextElement() {
		if (!next()) {
		    throw new NoSuchElementException();
		}
		Resource r = res;
		res = null;
		return r;
	    }
	};
    }
    
    public Enumeration getResources(final String name) {
        return getResources(name, true);
    }
    
    /*
     * Check whether the resource URL should be returned.
     * Return null on security check failure.
     * Called by java.net.URLClassLoader.
     */
    public URL checkURL(URL url) {
        try {
	    check(url);
        } catch (Exception e) {
	    return null;
        }
        
        return url;
    }
    
    /*
     * Check whether the resource URL should be returned.
     * Throw exception on failure.
     * Called internally within this file.
     */
    static private void check(URL url) throws IOException {
        SecurityManager security = System.getSecurityManager();
        if (security != null) {
	    Permission perm = url.openConnection().getPermission();
	    if (perm != null) {
		security.checkPermission(perm);
	    }
        }
    }
    
    
    private static abstract class Loader {
        private final URL base;
        
        /*
	 * Creates a new Loader for the specified URL.
	 */
        Loader(URL url) {
	    base = url;
        }
        
        /*
	 * Returns the Resource for the specified name, or null if not
	 * found or the caller does not have the permission to get the
	 * resource.
	 */
        Resource getResource(final String name) {
	    return getResource(name, true);
        }
        
        abstract Resource getResource(final String name, boolean check);
        
        /* Returns the base URL for this Loader */
        URL getBaseURL() { return base; }
    }
    
    /**
     * Inner class used to represent a loader of resources and classes
     * from a base URL.
     */
    private static class URLDirectoryLoader extends Loader {
        
        URLDirectoryLoader(URL url) {
	    super(url);
        }
        
        /* Returns resource */
        Resource getResource(final String name, boolean check) {
	    final URL url;
	    try {
		url = new URL(getBaseURL(), name);
	    } catch (MalformedURLException e) {
		throw new IllegalArgumentException("name");
	    }
	    final URLConnection uc;
	    try {
		if (check) {
		    JNLPClassPath.check(url);
		}
		
		// check to see if it exists
		//
		// It almost works to just try to do an openConnection() but
		// HttpURLConnection will return true on HTTP_BAD_REQUEST
		// when the requested name ends in ".html", ".htm", and ".txt"
		// and we want to be able to handle these
		//
		// Also, cannot just open a connection for things like
		// FileURLConnection, because they suceed when connecting to a
		// non-existant file.  So, in those cases we open and close an
		// input stream.
		uc = url.openConnection();
		if (uc instanceof HttpURLConnection) {
		    HttpURLConnection hconn = (HttpURLConnection)uc;
		    int code = hconn.getResponseCode();
		    hconn.disconnect();
		    if (code >= HttpURLConnection.HTTP_BAD_REQUEST) {
			return null;
		    }
		} else {
		    // our best guess for the other cases
		    InputStream is = url.openStream();
		    is.close();
		}
	    } catch (Exception e) {
		return null;
	    }
	    return new Resource() {
		public String getName() { return name; }
		public URL getURL() { return url; }
		public URL getCodeSourceURL() { return getBaseURL(); }
		public InputStream getInputStream() throws IOException {
		    return uc.getInputStream();
		}
		public int getContentLength() throws IOException {
		    return uc.getContentLength();
		}
	    };
        }
    }
    
    /*
     * Inner class used to represent a Loader of resources from a File JAR .
     */
    private static class JarLoader extends Loader {
        private JarFile jar;
        private URL csu;
        
        /*
	 * Creates a new JarLoader for the specified URL referring to
	 * a JAR file.
	 */
        JarLoader(URL url) throws IOException {
	    super(new URL("jar", "", -1, url + "!/"));
	    jar = getJarFile(url);
	    csu = url;
        }
        
        private JarFile getJarFile(URL url) throws IOException {
	    // Optimize case where url refers to a local jar file
	    if ("file".equals(url.getProtocol())) {
		String path = URLUtil.getPathFromURL(url);
		File file = new File(path);
		if (!file.exists()) {
		    throw new FileNotFoundException(path);
		}
		return new JarFile(path);
	    }
	    throw new IOException("Must be file URL");
        }
        
        /*
	 * Returns the JAR Resource for the specified name.
	 */
        Resource getResource(final String name, boolean check) {
	    final JarEntry entry = jar.getJarEntry(name);
	    Resource res;
	    
	    if (entry != null) {
		final URL url;
		try {
		    url = new URL(getBaseURL(), name);
		    if (check) {
			JNLPClassPath.check(url);
		    }
		} catch (MalformedURLException e) {
		    Trace.ignoredException(e);
		    return null;
		} catch (IOException e) {
		    Trace.ignoredException(e);
		    return null;
		} catch (AccessControlException e) {
		    Trace.ignoredException(e);
		    return null;
		}
		
		return new Resource() {
		    public String getName() { return name; }
		    public URL getURL() { return url; }
		    public URL getCodeSourceURL() { return csu; }
		    public InputStream getInputStream() throws IOException
		    { return jar.getInputStream(entry); }
		    public int getContentLength()
		    { return (int)entry.getSize(); }
		    public Manifest getManifest() throws IOException
		    { return jar.getManifest(); };
		    public Certificate[] getCertificates()
		    { return entry.getCertificates(); };
		    public java.security.CodeSigner[] getCodeSigners() { 
			if (Globals.isJavaVersionAtLeast15()) {
			    return entry.getCodeSigners(); 
			} else {
			    // no CodeSigners in pre-1.5 java
			    return null;
			}
		    };

		};
	    }
	    return null;
        }
    }
    
    /*
     * Inner class used to represent a loader of classes and resources
     * from a file URL that refers to a directory.
     */
    private static class FileDirectoryLoader extends Loader {
        private File dir;
        
        FileDirectoryLoader(URL url) {
	    super(url);
	    if (!("file".equals(url.getProtocol()))) {
		throw new IllegalArgumentException("must be FILE URL");
	    }
	    dir = new File(URLUtil.getPathFromURL(url));
        }
        
        Resource getResource(final String name, boolean check) {
	    final URL url;
	    try {
		url = new URL(getBaseURL(), name);
		if (url.getFile().startsWith(getBaseURL().getFile()) == false) {
		    // requested resource had ../..'s in path
		    return null;
		}
		
		if (check)
		    JNLPClassPath.check(url);
		final File file = new File(dir, name.replace('/', File.separatorChar));
		if (file.exists()) {
		    return new Resource() {
			public String getName() { return name; };
			public URL getURL() { return url; };
			public URL getCodeSourceURL() { return getBaseURL(); };
			public InputStream getInputStream() throws IOException
			{ return new FileInputStream(file); };
			public int getContentLength() throws IOException
			{ return (int)file.length(); };
		    };
		}
	    } catch (Exception e) {
		return null;
	    }
	    return null;
        }
    }
}


