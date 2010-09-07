/*
 * @(#)DeployURLClassPath.java	1.5 10/05/20
 *
 * Copyright (c) 2006, 2010, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;

import java.util.Enumeration;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.Hashtable;
import java.util.NoSuchElementException;
import java.util.Stack;
import java.util.Set;
import java.util.HashSet;
import java.util.StringTokenizer;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.jar.JarFile;
import sun.misc.JarIndex;
import sun.misc.InvalidJarIndexException;
import sun.net.www.ParseUtil;
import java.util.zip.ZipEntry;
import java.util.jar.JarEntry;
import java.util.jar.Manifest;
import java.util.jar.Attributes;
import java.util.jar.Attributes.Name;
import java.net.JarURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLConnection;
import java.net.HttpURLConnection;
import java.net.URLStreamHandler;
import java.net.URLStreamHandlerFactory;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.InputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.security.AccessController;
import java.security.AccessControlException;
import java.security.CodeSigner;
import java.security.Permission;
import java.security.PrivilegedAction;
import java.security.PrivilegedExceptionAction;
import java.security.cert.Certificate;
import sun.misc.FileURLMapper;
/* import sun.net.util.URLUtil; */
import sun.misc.URLClassPath;
import sun.misc.Resource;
/* import sun.misc.MetaIndex; */
import sun.misc.SharedSecrets;
import sun.misc.ExtensionDependency;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;


import com.sun.deploy.util.URLUtil;
import com.sun.deploy.config.Config;

/**
 * This derived version of 6u19 sun.misc.URLClassPath implements a
 * callback extension which allows CPCallbackHandler to manage
 * a dual loader strategy.
 */
public class DeployURLClassPath extends URLClassPath {
    final static String USER_AGENT_JAVA_VERSION = "UA-Java-Version";
    final static String JAVA_VERSION;
    private static final boolean DEBUG;

    static {
 	JAVA_VERSION = (String) java.security.AccessController.doPrivileged(
            new sun.security.action.GetPropertyAction("java.version"));
 	DEBUG        = (java.security.AccessController.doPrivileged(
            new sun.security.action.GetPropertyAction("sun.misc.URLClassPath.debug")) != null);
	try {
	    if (Class.forName("sun.misc.MetaIndex") != null) {
		hasRealMetaIndex = true;
	    }
	} catch (ClassNotFoundException cnfe) {
	}
    }

    /* Workaround for JarIndex.getJarIndex() for 1.4.2 and 1.5 */
    static private boolean hasRealMetaIndex;

    /* The original search path of URLs. */
    private ArrayList path = new ArrayList();

    /* The stack of unopened URLs */
    Stack urls = new Stack();

    /* The resulting search path of Loaders */
    ArrayList loaders = new ArrayList();

    /* Map of each URL opened to its corresponding Loader */
    HashMap lmap = new HashMap();

    /* The jar protocol handler to use when creating new URLs */
    private URLStreamHandler jarHandler;

    /* The class path manager hook, if set */
    private DeployURLClassPathCallback cb;

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
    private DeployURLClassPath(URL[] urls, URLStreamHandlerFactory factory) {
	super(urls);
	for (int i = 0; i < urls.length; i++) {
	    path.add(urls[i]);
	}
	push(urls);
	if (factory != null) {
	    jarHandler = factory.createURLStreamHandler("jar");
	}
    }

    public DeployURLClassPath(URL[] urls) {
	this(urls, null);
    }

    public DeployURLClassPath(URLClassPath ucp) {
	this(ucp.getURLs());
    }

    /**
     * Appends the specified URL to the search path of directory and JAR
     * file URLs from which to load classes and resources.
     */
    public void addURL(URL url) {
	synchronized (urls) {
            if (path.contains(url))
                return;

	    urls.add(0, url);
	    path.add(url);
	}
    }

    /**
     * Returns the original search path of URLs.
     */
    public URL[] getURLs() {
	synchronized (urls) {
	    return (URL[])path.toArray(new URL[path.size()]);
	}
    }

    public void setDeployURLClassPathCallback(DeployURLClassPathCallback cb) {
        this.cb = cb;
    }


    /*
     * Maintains the iteration state while walking through the class path.
     */
    private static class PathIterator {
        int index;
        boolean found;

        int index() {
            return index;
        }

        void next() {
            index++;
        }

        void nextResource() {
            index++;
            found = false;
        }

        boolean found() {
            return found;
        }
        void found(boolean found) {
            this.found = found;
        }
   }


    /**
     * Finds the resource with the specified name on the URL search path
     * or null if not found or security check fails.
     *
     * @param name 	the name of the resource
     * @param check     whether to perform a security check
     * @return a <code>URL</code> for the resource, or <code>null</code>
     * if the resource could not be found.
     */
    public URL findResource(String name, boolean check) {
	Loader loader;
        PathIterator pi = new PathIterator();
        for (; (loader = getLoader(pi)) != null; pi.next()) {
            URL url = loader.findResource(name, check, pi);
            if (url != null) {
                return url;
            }	
        }
        return null;
    }

    /**
     * Finds the first Resource on the URL search path which has the specified
     * name. Returns null if no Resource could be found.
     *
     * @param name the name of the Resource
     * @param check 	whether to perform a security check
     * @return the Resource, or null if not found
     */
    public Resource getResource(String name, boolean check) {
        if (DEBUG) {
            System.err.println("URLClassPath.getResource(\"" + name + "\")");
        }

	Loader loader;
        PathIterator pi = new PathIterator();
	for (; (loader = getLoader(pi)) != null; pi.next()) {
	    Resource res = loader.getResource(name, check, pi);
	    if (res != null) {
		return res;
	    }
	}
	return null;
    }

    /**
     * Finds all resources on the URL search path with the given name.
     * Returns an enumeration of the URL objects.
     *
     * @param name the resource name
     * @return an Enumeration of all the urls having the specified name
     */
    public Enumeration findResources(final String name,
                                     final boolean check) {
        return new Enumeration() {
            private PathIterator pi = new PathIterator();
            private URL url = null;

            private boolean next() {
                if (url != null) {
                    return true;
                } else {
                    Loader loader;
                    while ((loader = getLoader(pi)) != null) {
                        url = loader.findResource(name, check, pi);
			pi.nextResource();
                        if (url != null) {
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
                URL u = url;
                url = null;
                return u;
            }
        };
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
	return new Enumeration() {
            PathIterator pi = new PathIterator();
	    private Resource res = null;

	    private boolean next() {
		if (res != null) {
		    return true;
		} else {
		    Loader loader;
		    while ((loader = getLoader(pi)) != null) {
			res = loader.getResource(name, check, pi);
			pi.nextResource();
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
     * Returns the Loader at the specified position in the URL search
     * path. The URLs are opened and expanded as needed. Returns null
     * if the specified index is out of range.
     */
     private synchronized Loader getLoader(PathIterator pi) {
	 // Expand URL search path until the request can be satisfied
	 // or the URL stack is empty.
        if (pi.found()) {
            return null;
        }
	while (loaders.size() < pi.index() + 1) {
	    // Pop the next URL from the URL stack
	    URL url;
	    synchronized (urls) {
	        if (urls.empty()) {
		    return null;
 		} else {
		    url = (URL)urls.pop();
		}
	    }
            // Skip this URL if it already has a Loader. (Loader
            // may be null in the case where URL has not been opened
            // but is referenced by a JAR index.)
            String urlNoFragString = URLUtil.urlNoFragString(url);
	    if (lmap.containsKey(urlNoFragString)) {
		continue;
	    }
	    // Otherwise, create a new Loader for the URL.
	    Loader loader;
	    try {
		loader = getLoader(url);
		// If the loader defines a local class path then add the
		// URLs to the list of URLs to be opened.
		URL[] urls = loader.getClassPath();
		if (urls != null) {
		    push(urls);
		}
	    } catch (IOException e) {
		// Silently ignore for now...
		continue;
	    }
	    // Finally, add the Loader to the search path.
	    loaders.add(loader);
	    lmap.put(urlNoFragString, loader);
	}
	return (Loader)loaders.get(pi.index());
    }

    /*
     * Returns the Loader for the specified base URL.
     */
    private Loader getLoader(final URL url) throws IOException {
	try {
	    return (Loader)java.security.AccessController.doPrivileged
		(new java.security.PrivilegedExceptionAction() {
		public Object run() throws IOException {
		    String file = url.getFile();
		    if (file != null && file.endsWith("/")) {
			if ("file".equals(url.getProtocol())) {
			    return new FileLoader(url);
			} else {
			    return new UrlLoader(url);
			}
		    } else {
			return new JarLoader(url, jarHandler, lmap);
		    }
		}
	    });
	} catch (java.security.PrivilegedActionException pae) {
	    throw (IOException)pae.getException();
	}
    }

    /*
     * Pushes the specified URLs onto the list of unopened URLs.
     */
    private void push(URL[] us) {
	synchronized (urls) {
	    for (int i = us.length - 1; i >= 0; --i) {
		urls.push(us[i]);
	    }
	}
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
    static void check(URL url) throws IOException {
	SecurityManager security = System.getSecurityManager();
	if (security != null) {
	    URLConnection urlConnection = url.openConnection();
	    Permission perm = urlConnection.getPermission();
	    if (perm != null) {
		try {
		    security.checkPermission(perm);
		} catch (SecurityException se) {
		    // fallback to checkRead/checkConnect for pre 1.2
		    // security managers
		    if ((perm instanceof java.io.FilePermission) &&
		        perm.getActions().indexOf("read") != -1) {
			security.checkRead(perm.getName());
		    } else if ((perm instanceof 
			java.net.SocketPermission) &&
			perm.getActions().indexOf("connect") != -1) {
			URL locUrl = url;
			if (urlConnection instanceof JarURLConnection) {
			    locUrl = ((JarURLConnection)urlConnection).getJarFileURL();
			}
			security.checkConnect(locUrl.getHost(), 
					      locUrl.getPort());
		    } else {
			throw se;
		    }
		}
	    }
	}
    }

    /**
     * Inner class used to represent a loader of resources and classes
     * from a base URL.
     */
    private abstract class Loader {
	private final URL base;
	protected DeployURLClassPathCallback.Element cpe;
        protected boolean skip;
        protected boolean defer;

	/*
	 * Creates a new Loader for the specified URL.
	 */
	Loader(URL url) {
	    base = url;
	}

	/*
	 * Returns the base URL for this Loader.
	 */
	URL getBaseURL() {
	    return base;
	}

        /*
         * Returns the local class path for this loader, or null if none.
         */
        URL[] getClassPath() throws IOException {
            return null;
        }

	/*
	 * Try to find the resource but defer the final resolution and any
	 * consequences. Return null on failure.
	 */
        abstract URL findResource(final String name, boolean check, PathIterator pi);

	/*
	 * Try to find the resource. Return null on failure but allow SecurityExceptions
	 * to pass through.
	 */
        abstract Resource getResource(final String name, boolean check, PathIterator pi);
    }

    private class UrlLoader extends Loader {

        UrlLoader(URL url) throws IOException {
            super(url);
            if (cb != null) {
                cpe = cb.openClassPathElement(url);
		skip = cpe.skip(); defer = cpe.defer();
            }
        }


	URL findResource(final String name, boolean check, PathIterator pi) {
	    URL url;

            if (skip) {
                return null;
            }
	    try {
		url = new URL(getBaseURL(), parseUtilEncodePath(name, false));
            } catch (MalformedURLException e) {
                throw new IllegalArgumentException("name");
            }

            try {
                if (check) {
                    DeployURLClassPath.check(url);
                }

		/*
		 * For a HTTP connection we use the HEAD method to
	 	 * check if the resource exists.
		 */
		URLConnection uc = url.openConnection();
                if (uc instanceof HttpURLConnection) {
		    HttpURLConnection hconn = (HttpURLConnection)uc;
		    hconn.setRequestMethod("HEAD");
                    if (hconn.getResponseCode() >= HttpURLConnection.HTTP_BAD_REQUEST) {
                        return null;
                    }
		} else {
		    // our best guess for the other cases 
		    InputStream is = url.openStream();
                    is.close();
		}
                if (cpe != null) {
		    if (defer) {
                        pi.found(true);
                        return null;
		    }
                    cpe.checkResource(name);
                }
		return url;
	    } catch (SecurityException se) {
		Trace.println("resource name \"" + name + "\" in " + url + " : " + se, TraceLevel.SECURITY);
		return null;
	    } catch (Exception e) {
		return null;
	    }
	}

	Resource getResource(final String name, boolean check, PathIterator pi) {
	    final URL url;

	    if (skip) {
		return null;
	    }
	    try {
		url = new URL(getBaseURL(), parseUtilEncodePath(name, false));
	    } catch (MalformedURLException e) {
		throw new IllegalArgumentException("name");
	    }
	    final URLConnection uc;
	    try {
		if (check) {
		    DeployURLClassPath.check(url);
		}
		uc = url.openConnection();
		InputStream in = uc.getInputStream();
	    } catch (Exception e) {
		return null;
	    }
            if (cb != null) {
                if (defer) {
                    pi.found(true);
                    return null;
                }
		try {
                    cpe.checkResource(name);
	        } catch (SecurityException se) {
		    Trace.println("resource name \"" + name + "\" in " + url + " : " + se, TraceLevel.SECURITY);
		    throw se;
                }
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
     * Inner class used to represent a Loader of resources from a JAR URL.
     */
    class JarLoader extends Loader {
	private JarFile jar;
	private URL csu;
        private JarIndex index;
        private MetaIndex metaIndex;
        private URLStreamHandler handler;
        private HashMap lmap;
        
	/*
	 * Creates a new JarLoader for the specified URL referring to
	 * a JAR file.
	 */
	JarLoader(URL url, URLStreamHandler jarHandler, HashMap loaderMap) 
            throws IOException 
        {
	    super(new URL("jar", "", -1, url + "!/", jarHandler));
	    csu = url;
            handler = jarHandler;
            lmap = loaderMap;

            if (!isOptimizable(url)) {
                ensureOpen();
            } else {
		 String fileName = url.getFile();
                if (fileName != null) {
		    fileName = ParseUtil.decode(fileName);
		    File f = new File(fileName);
                    metaIndex = MetaIndex.forJar(f);
		    // If the meta index is found but the file is not
		    // installed, set metaIndex to null. A typical
		    // senario is charsets.jar which won't be installed
		    // when the user is running in certain locale environment.
		    // The side effect of null metaIndex will cause 
		    // ensureOpen get called so that IOException is thrown.
		    if (metaIndex != null && !f.exists()) {
			metaIndex = null;
		    }
                }

		// metaIndex is null when either there is no such jar file
		// entry recorded in meta-index file or such jar file is
		// missing in JRE. See bug 6340399.
		if (metaIndex == null) {
		    ensureOpen();
		}
            }
        }

	JarFile getJarFile () {
	    return jar;
	}

        private boolean isOptimizable(URL url) {
            return "file".equals(url.getProtocol());
        }

        private void ensureOpen() throws IOException {
            if (jar == null) {
                try {
                    java.security.AccessController.doPrivileged(
                        new java.security.PrivilegedExceptionAction() {
                            public Object run() throws IOException {
                                if (DEBUG) {
                                    System.err.println("Opening " + csu);
                                    Thread.dumpStack();
                                }

                                jar = getJarFile(csu);
				if (hasRealMetaIndex) {
                                    index = JarIndex.getJarIndex(jar, null);
				} else {
                                    index = JarIndex.getJarIndex(jar);
				}
                                if (index != null) {
                                    String[] jarfiles = index.getJarFiles();
                                // Add all the dependent URLs to the lmap so that loaders
                                // will not be created for them by URLClassPath.getLoader(PathIterator)
                                // if the same URL occurs later on the main class path.  We set 
                                // Loader to null here to avoid creating a Loader for each 
                                // URL until we actually need to try to load something from them.
                                    for(int i = 0; i < jarfiles.length; i++) {
                                        try {
                                            URL jarURL = new URL(csu, jarfiles[i]);
                                            // If a non-null loader already exists, leave it alone.
                                            String urlNoFragString = URLUtil.urlNoFragString(jarURL);
                                            if (!lmap.containsKey(urlNoFragString)) {
                                                lmap.put(urlNoFragString, null);
                                            }
                                        } catch (MalformedURLException e) {
                                            continue;
                                        }
                                    }
                                }
                                return null;
                            }
                        }
                    );
                } catch (java.security.PrivilegedActionException pae) {
                    throw (IOException)pae.getException();
                }
            }
	}

	private JarFile getJarFile(URL url) throws IOException {
            JarFile jf;

	    // Optimize case where url refers to a local jar file
	    if (isOptimizable(url)) {
		FileURLMapper p = new FileURLMapper (url);
		if (!p.exists()) {
		    throw new FileNotFoundException(p.getPath());
		}
                jf = new JarFile (p.getPath());
	    } else {
	        URLConnection uc = getBaseURL().openConnection();
	        uc.setRequestProperty(USER_AGENT_JAVA_VERSION, JAVA_VERSION);
	        jf = ((JarURLConnection)uc).getJarFile();
	    }
            if (cb != null) {
                cpe = cb.openClassPathElement(jf, csu);
		skip = cpe.skip(); defer = cpe.defer();
            }

            return jf;
	}

        /*
         * Returns the index of this JarLoader if it exists.
         */
        JarIndex getIndex() {
            try {
                ensureOpen();
            } catch (IOException e) {
                throw (InternalError) new InternalError().initCause(e);
            }
            return index;
        }
        
	/* 
	 * Creates the resource and if the check flag is set to true, checks if
	 * is its okay to return the resource.
	 */
	Resource checkResource(final String name, boolean check, 
	    final JarEntry entry, final JarFile jar, PathIterator pi) {

	    final URL url;
	    try {
		url = new URL(getBaseURL(), parseUtilEncodePath(name, false));
		if (check) {
		    DeployURLClassPath.check(url);
		}
	    } catch (MalformedURLException e) {
		return null;
		// throw new IllegalArgumentException("name");
	    } catch (IOException e) {
		return null;
	    } catch (AccessControlException e) {
		return null;
	    }
            if (cb != null) {
                if (defer) {
                    pi.found(true);
                    return null;
                }
		try {
                    cpe.checkResource(name);
	        } catch (SecurityException se) {
		    Trace.println("resource name \"" + name + "\" in " + csu + " : " + se, TraceLevel.SECURITY);
		    throw se;
                }
	    }
	    if (Config.isJavaVersionAtLeast15()) {
	        return new Resource() {
		    public String getName() { return name; }
		    public URL getURL() { return url; }
		    public URL getCodeSourceURL() { return csu; }
		    public InputStream getInputStream() throws IOException
		        { return jar.getInputStream(entry); }
		    public int getContentLength()
		        { return (int)entry.getSize(); }
		    public Manifest getManifest() throws IOException
		        { return jar.getManifest(); }
		    public Certificate[] getCertificates()
		        { return entry.getCertificates(); }
		    public CodeSigner[] getCodeSigners()
		        { return entry.getCodeSigners(); }
	        };
	    } else {
	        return new Resource() {
		    public String getName() { return name; }
		    public URL getURL() { return url; }
		    public URL getCodeSourceURL() { return csu; }
		    public InputStream getInputStream() throws IOException
		        { return jar.getInputStream(entry); }
		    public int getContentLength()
		        { return (int)entry.getSize(); }
		    public Manifest getManifest() throws IOException
		        { return jar.getManifest(); }
		    public Certificate[] getCertificates()
		        { return entry.getCertificates(); }
	        };
	    }
	}


	/*
	 * Returns true iff atleast one resource in the jar file has the same
	 * package name as that of the specified resource name.
	 */
	boolean validIndex(final String name) {
	    String packageName = name;
	    int pos;
	    if((pos = name.lastIndexOf("/")) != -1) {
		packageName = name.substring(0, pos);
	    }

	    String entryName;
	    ZipEntry entry;
	    Enumeration enum_ = jar.entries();
	    while (enum_.hasMoreElements()) {
		entry = (ZipEntry)enum_.nextElement();
	        entryName = entry.getName();
		if((pos = entryName.lastIndexOf("/")) != -1) 
		    entryName = entryName.substring(0, pos);
		if (entryName.equals(packageName)) {
		    return true;
		}
	    }
	    return false;
	}

	/*
	 * Returns the URL for a resource with the specified name
	 */
	URL findResource(final String name, boolean check, PathIterator pi) {
	    try {
                Resource rsc = getResource(name, check, pi);
                if (rsc != null) {
                    return rsc.getURL();
                }   
	    } catch (Exception e) {
	    }
            return null;
        }

	/*
	 * Returns the JAR Resource for the specified name.
	 */
	Resource getResource(final String name, boolean check, PathIterator pi) {
            if (skip) {
                return null;
            }
            if (metaIndex != null) {
                if (!metaIndex.mayContain(name)) {
                    return null;
                }
            }

            try {
                ensureOpen();
            } catch (IOException e) {
                throw (InternalError) new InternalError().initCause(e);
            }
            final JarEntry entry = jar.getJarEntry(name);
            if (entry != null) 
		return checkResource(name, check, entry, jar, pi);

	    if (index == null) 
		return null;

	    HashSet visited = new HashSet();
	    return getResource(name, check, visited, pi);
	}

	/*
	 * Version of getResource() that tracks the jar files that have been
	 * visited by linking through the index files. This helper method uses
	 * a HashSet to store the URLs of jar files that have been searched and
	 * uses it to avoid going into an infinite loop, looking for a
	 * non-existent resource
	 */
	Resource getResource(final String name, boolean check, 
		Set visited, PathIterator pi) {

            Resource res;
	    Object[] jarFiles;
	    boolean done = false;
	    int count = 0;
	    LinkedList jarFilesList = null;
	    
	    /* If there no jar files in the index that can potential contain
	     * this resource then return immediately.
	     */
	    if((jarFilesList = index.get(name)) == null)
		return null;

	    do {
		jarFiles = jarFilesList.toArray();
		int size = jarFilesList.size();
		/* loop through the mapped jar file list */
		while(count < size) {
		    String jarName = (String)jarFiles[count++];
		    JarLoader newLoader;
		    final URL url;
		    
		    try{
			url = new URL(csu, jarName);
                        String urlNoFragString = URLUtil.urlNoFragString(url);
			if ((newLoader = (JarLoader)lmap.get(urlNoFragString)) == null) {
			    /* no loader has been set up for this jar file
			     * before 
			     */
			    newLoader = (JarLoader)
				AccessController.doPrivileged(
				    new PrivilegedExceptionAction() {
				    public Object run() throws IOException {
					return new JarLoader(url, handler, 
					    lmap);
				    }
				});

			    /* this newly opened jar file has its own index,
			     * merge it into the parent's index, taking into
			     * account the relative path.
			     */
			    JarIndex newIndex = 
				((JarLoader)newLoader).getIndex();
			    if(newIndex != null) {
				int pos = jarName.lastIndexOf("/");
				newIndex.merge(this.index, (pos == -1 ? 
				    null : jarName.substring(0, pos + 1)));
			    }
			     
			    /* put it in the global hashtable */
			    lmap.put(urlNoFragString, newLoader);
			} 
		    } catch (java.security.PrivilegedActionException pae) {
			continue;
		    } catch (MalformedURLException e) {
			continue;
		    }


		    /* Note that the addition of the url to the list of visited
		     * jars incorporates a check for presence in the hashmap
		     */
		    boolean visitedURL = !visited.add(URLUtil.urlNoFragString(url));
		    if (!visitedURL) {
                        try {
                            newLoader.ensureOpen();
                        } catch (IOException e) {
                            throw (InternalError) new InternalError().initCause(e);
                        }
			final JarEntry entry = newLoader.jar.getJarEntry(name);
			if (entry != null) {
			    return newLoader.checkResource(name, check, entry, newLoader.jar, pi);
			}

			/* Verify that at least one other resource with the
			 * same package name as the lookedup resource is
			 * present in the new jar
		 	 */
			if (!newLoader.validIndex(name)) {
			    /* the mapping is wrong */
			    throw new InvalidJarIndexException("Invalid index");
			}
		    }

		    /* If newLoader is the current loader or if it is a
		     * loader that has already been searched or if the new
		     * loader does not have an index then skip it
		     * and move on to the next loader. 
		     */
		    if (visitedURL || newLoader == this || 
			    newLoader.getIndex() == null) {
			continue;
		    }

		    /* Process the index of the new loader
		     */
		    if((res = newLoader.getResource(name, check, visited, pi)) 
			    != null) {
			return res;
		    }
		}
		// Get the list of jar files again as the list could have grown
		// due to merging of index files.
		jarFilesList = index.get(name);

	    // If the count is unchanged, we are done.
	    } while(count < jarFilesList.size());
            return null;
        }
        

	/*
	 * Returns the JAR file local class path, or null if none.
	 */
	URL[] getClassPath() throws IOException {
            if (index != null) {
                return null;
            }

            if (metaIndex != null) {
                return null;
            }

            ensureOpen();
	    parseExtensionsDependencies();
	    // SharedSecrets class only available in JRE 1.4.2 and later, if we are using old JRE
	    // we will simple skip the performance optimization check.
            if (Config.checkClassName("sun.misc.SharedSecrets")) { 
		if (SharedSecrets.javaUtilJarAccess().jarFileHasClassPathAttribute(jar)) { 
		    // Only get manifest when necessary
                    Manifest man = jar.getManifest();
                    if (man != null) {
                        Attributes attr = man.getMainAttributes();
                        if (attr != null) {
                            String value = attr.getValue(Name.CLASS_PATH);
                            if (value != null) {
                                return parseClassPath(csu, value);
                            }
                        }
                    }
	        }
	    }
	    else {
                Manifest man = jar.getManifest();
                if (man != null) {
                    Attributes attr = man.getMainAttributes();
                    if (attr != null) {
                        String value = attr.getValue(Name.CLASS_PATH);
                        if (value != null) {
                            return parseClassPath(csu, value);
                        }
                    }
                }
	    }
	    return null;
	}

	/*
	 * parse the standard extension dependencies
	 */
	private void  parseExtensionsDependencies() throws IOException {
	    ExtensionDependency.checkExtensionsDependencies(jar);
	}

	/*
	 * Parses value of the Class-Path manifest attribute and returns
	 * an array of URLs relative to the specified base URL.
	 */
	private URL[] parseClassPath(URL base, String value)
	    throws MalformedURLException
	{
	    StringTokenizer st = new StringTokenizer(value);
	    URL[] urls = new URL[st.countTokens()];
	    int i = 0;
	    while (st.hasMoreTokens()) {
		String path = st.nextToken();
		urls[i] = new URL(base, path);
		i++;
	    }
	    return urls;
	}
    }

    /*
     * Inner class used to represent a loader of classes and resources
     * from a file URL that refers to a directory.
     */
    private class FileLoader extends Loader {
        /* Canonicalized File */
	private File dir;

	FileLoader(URL url) throws IOException {
	    super(url);
	    if (!"file".equals(url.getProtocol())) {
		throw new IllegalArgumentException("url");
	    }
	    String path = url.getFile().replace('/', File.separatorChar);
	    path = ParseUtil.decode(path);
	    dir = (new File(path)).getCanonicalFile();
            if (cb != null) {
                cpe = cb.openClassPathElement(url);
		skip = cpe.skip(); defer = cpe.defer();
            }
	}

	/*
         * Returns the URL for a resource with the specified name
         */
	URL findResource(final String name, boolean check, PathIterator pi) {
	    try {
                Resource rsc = getResource(name, check, pi);
	        if (rsc != null) {
		    return rsc.getURL();
 	        }
	    } catch (Exception e) {
	    }
	    return null;
	}

	Resource getResource(final String name, boolean check, PathIterator pi) {
	    final URL url;

            if (skip) {
                return null;
            }
	    File f = null;
	    try {
		URL normalizedBase = new URL(getBaseURL(), ".");
		url = new URL(getBaseURL(), parseUtilEncodePath(name, false));

                if (url.getFile().startsWith(normalizedBase.getFile()) == false) {
                    // requested resource had ../..'s in path
                    return null;
		}

		if (check)
		    DeployURLClassPath.check(url);
                if (name.indexOf("..") != -1) {
                    f = (new File(dir, name.replace('/', File.separatorChar)))
                          .getCanonicalFile();
                    if ( !((f.getPath()).startsWith(dir.getPath())) ) {
                        /* outside of base dir */
                        return null;
                    }
                } else {
                    f = new File(dir, name.replace('/', File.separatorChar)); 
                } 

		if (!f.exists()) {
		    return null;
		}
	    } catch (Exception e) {
		return null;
	    }
            if (cb != null) {
                if (defer) {
                    pi.found(true);
                    return null;
                }
		try {
                    cpe.checkResource(name);
		} catch (SecurityException se) {
		    Trace.println("resource name \"" + name + "\" in " + dir + " : " + se, TraceLevel.SECURITY);
		    throw se;
		}
            }

	    final File file = f;
	    return new Resource() {
		public String getName() { return name; }
		public URL getURL() { return url; }
		public URL getCodeSourceURL() { return getBaseURL(); }
		public InputStream getInputStream() throws IOException
		    { return new FileInputStream(file); }
		public int getContentLength() throws IOException
		    { return (int)file.length(); }
	    };
	}
    }

    /*
     * Fake stub for sun.misc.MetaIndex to enhance source-level
     * maintainability with the j2se sun.misc.URLClassPath code.
     */
    static private class MetaIndex {
	static MetaIndex forJar(File jar) {
	    return null;
	}

        boolean mayContain(String entry) {
	    return false;
	}
    }

    // ParseUtil.encodePath(String, boolean) wasn't introduced until later 1.4.2 updates
    private String parseUtilEncodePath(String name, boolean flag) {
        try {
            return ParseUtil.encodePath(name, flag);
        } catch (NoSuchMethodError e) {
            return ParseUtil.encodePath(name);
        }
    }
}
