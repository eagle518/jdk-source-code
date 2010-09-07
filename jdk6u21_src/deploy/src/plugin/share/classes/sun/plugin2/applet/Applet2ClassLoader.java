/*
 * @(#)Applet2ClassLoader.java	1.21 10/05/21
 *
 * Copyright (c) 2007, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.applet;

import java.lang.NullPointerException;
import java.net.URL;
import java.net.URLClassLoader;
import java.net.SocketPermission;
import java.net.URLConnection;
import java.net.MalformedURLException;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.io.File;
import java.io.FilePermission;
import java.io.IOException;
import java.io.BufferedInputStream;
import java.io.InputStream;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.NoSuchElementException;
import java.security.AccessController;
import java.security.AccessControlContext;
import java.security.PrivilegedAction;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;
import java.security.CodeSource;
import java.security.Permission;
import java.security.PermissionCollection;
import java.security.Policy;
import java.security.cert.CertificateExpiredException;
import java.security.cert.CertificateNotYetValidException;
import sun.awt.AppContext;
import sun.awt.SunToolkit;
import sun.net.www.ParseUtil;
import sun.security.util.SecurityConstants;
import com.sun.deploy.config.Config;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.security.CeilingPolicy;
import com.sun.deploy.security.TrustDecider;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.jnlp.JNLPPreverifyClassLoader;

import sun.plugin2.util.SystemUtil;
import sun.reflect.Reflection;

public class Applet2ClassLoader extends Plugin2ClassLoader {
    private static final boolean DEBUG   = (SystemUtil.getenv("JPI_PLUGIN2_DEBUG") != null);

    // Object for synchronization around getResourceAsStream()
    private Object syncResourceAsStream = new Object();
    private Object syncResourceAsStreamFromJar = new Object();

    // Flag to indicate getResourceAsStream() is in call
    private boolean resourceAsStreamInCall = false;
    private boolean resourceAsStreamFromJarInCall = false;

    /*
     * Creates a new Applet2ClassLoader for the specified base URL.
     */
    private Applet2ClassLoader(URL base) {
        super(new URL[0], base);
    }

    private Applet2ClassLoader(URL base, ClassLoader parent) {
        super(new URL[0], base, parent);
    }

    /*
     * Constructs a new Applet2ClassLoader
     */
    public static Applet2ClassLoader newInstance(URL base) {
        Applet2ClassLoader parent = new Applet2ClassLoader(base);

        // Only use callback if mixed code security enhancement is enabled
        if (Config.getMixcodeValue() != Config.MIXCODE_DISABLE) {
            Applet2ClassLoader child = new Applet2ClassLoader(base, parent);
            if (!Plugin2ClassLoader.setDeployURLClassPathCallbacks(parent, child)) {
                return parent;
            }
	    parent._delegatingClassLoader = child;
            return child;
        } else {
            return parent;
        }
    }
        
    //----------------------------------------------------------------------
    // URLClassLoader implementations
    //

    /*
     * Returns the URLs used for loading classes and resources.
     */
    public URL[] getURLs() {
        URL[] jars = super.getURLs();
        URL[] urls = new URL[jars.length + 1];
        System.arraycopy(jars, 0, urls, 0, jars.length);
        urls[urls.length - 1] = base;
        return urls;
    }

    /*
     * Normally, findClass(String, boolean) is called from Plugin2ClassLoader's
     * overridden loadClass() method. Calls originating elsewhere are unexpected
     * but technically possible.
     */
    protected Class findClass(String name) throws ClassNotFoundException {
	return findClass(name, false);
    }
   
    private boolean processingException = false;

    /*
     * Finds the applet class with the specified name. First searches
     * loaded JAR files then the applet code base for the class.
     */
    protected Class findClass(String name, boolean delegated) throws ClassNotFoundException {
        int index = name.indexOf(";");
        String cookie = "";
        if(index != -1) {
            cookie = name.substring(index, name.length());
            name = name.substring(0, index);
        }

	// check loaded JAR files
        try {
            return findClassHelper(name);
        } catch (ClassNotFoundException e) {
            synchronized (this) { // redundant?
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
        }

        ClassNotFoundException exc = (ClassNotFoundException) cnfeThreadLocal.get();
        if (exc != null) {
            cnfeThreadLocal.set(null);
        }

        // Otherwise, try loading the class from the code base URL
	
        // 4668479: Option to turn off codebase lookup in AppletClassLoader 
        // during resource requests. [stanley.ho]
        if (getCodebaseLookup() == false) {
            if (exc != null) {
                throw exc;
            }
            throw new ClassNotFoundException(name);
        }
	
        String encodedName = parseUtilEncodePath(name.replace('.', '/'), false);
        final String path = (new StringBuffer(encodedName)).append(".class").append(cookie).toString();
        try {
            byte[] b = (byte[]) AccessController.doPrivileged(
                new PrivilegedExceptionAction() {
                    public Object run() throws IOException {
                        return getBytes(new URL(base, path));
                    }
                }, _acc);

            if (b != null) {
		// check if AllPermission granted for the CodeSource
		try {
		    if (getSecurityCheck() && 
                            TrustDecider.isAllPermissionGranted(codesource) == TrustDecider.PERMISSION_DENIED) {
			throw new ClassNotFoundException(name);
		    }
		} catch (Exception ex) {
		    throw newClassNotFoundException(name);
		}
		checkResource(name.replace('.', '/') + ".class");

		return defineClass(name, b, 0, b.length, codesource);
            } else {
                if (exc != null) {
                    throw exc;
                }
                throw new ClassNotFoundException(name);
            }
        } catch (PrivilegedActionException e) {
            if (exc != null) {
                throw exc;
            }
            throw new ClassNotFoundException(name, e.getException());
        }
    }


    /*
     * See JNLPPreverifyClassLoader for comments.
     */
    static class DelegatingThread extends JNLPPreverifyClassLoader.DelegatingThread {

	DelegatingThread(ClassLoader cl, ClassLoader waiter) {
	    super(cl, waiter);
	}

	/*
	 * Override for applets
	 */
	protected void quiesce(ClassLoader child, ClassLoader parent) {
	    boolean self = waiter.equals(child);
	    if (child instanceof Applet2ClassLoader) {
	        ((Applet2ClassLoader)child).quiescenceRequested(thread, self);
	    }
	    if (self) {
		return;
	    }
	    quiesce(child.getParent(), parent);
	}
    }

    /*
     * See JNLPPreverifyClassLoader for comments.
     */
    public static class UndelegatingThread extends JNLPPreverifyClassLoader.UndelegatingThread {
	public UndelegatingThread(ClassLoader cl, ClassLoader waiter) {
	    super(cl, waiter);
	}

	/*
	 * Override for applets
	 */
	protected void unquiesce(ClassLoader child, ClassLoader parent) {
	    boolean self = parent.equals(child);
	    if (!self) {
	        unquiesce(child.getParent(), parent);
	    }
	    if (child instanceof Applet2ClassLoader) {
	        ((Applet2ClassLoader)child).quiescenceCancelled(self);
	    }
	}
    }

    /**
     * Returns an input stream for reading the specified resource.
     *
     * The search order is described in the documentation for {@link
     * #getResource(String)}.<p>
     * 
     * @param  name the resource name
     * @return an input stream for reading the resource, or <code>null</code>
     *         if the resource could not be found
     * @since  JDK1.1
     */
    public InputStream getResourceAsStream(String name) 
    {    
    
        if (name == null) {    	
            throw new NullPointerException("name");
        }       	
	
        try 
            {		   	    	
                InputStream is = null;
	    	    	    
                // Fixed #4507227: Slow performance to load
                // class and resources. [stanleyh]
                //
                // The following is used to avoid calling
                // AppletClassLoader.findResource() in
                // super.getResourceAsStream(). Otherwise,
                // unnecessary connection will be made.
                //	
                synchronized(syncResourceAsStream)
                    {
                        resourceAsStreamInCall = true;		
	
                        // Call super class
                        is = super.getResourceAsStream(name);
	
                        resourceAsStreamInCall = false;		
                    }

                // 4668479: Option to turn off codebase lookup in AppletClassLoader 
                // during resource requests. [stanley.ho]
                if (is == null && getCodebaseLookup() == true)
                    {	    
                        // If resource cannot be obtained,
                        // try to download it from codebase
                        URL url = new URL(base, parseUtilEncodePath(name, false));
                        is = url.openStream();
			checkResource(name);
                    }

                return is;
            } 
        catch (Exception e) 
            {		
                return null;	    
            }
    }

    /*
     * Finds the applet resource with the specified name. First checks
     * loaded JAR files then the applet code base for the resource.
     */
    public URL findResource(String name) {
        // check loaded JAR files
        URL url = super.findResource(name);

        // 6215746:  Disable META-INF/* lookup from codebase in 
        // applet/plugin classloader. [stanley.ho] 
        if (name.startsWith("META-INF/"))
            return url;

        if (url == null) 
            {
               // 4668479: Option to turn off codebase lookup in AppletClassLoader 
               // during resource requests. [stanley.ho]
               if (getCodebaseLookup() == false)
                   return url;
	    
                //#4805170, if it is a call from Applet.getImage()
                //we should check for the image only in the archives
                boolean insideGetResourceAsStreamFromJar = false;
                synchronized(syncResourceAsStreamFromJar) {
                    insideGetResourceAsStreamFromJar = resourceAsStreamFromJarInCall;
                }		

                if (insideGetResourceAsStreamFromJar) {
                    return null;
                }

                // Fixed #4507227: Slow performance to load
                // class and resources. [stanleyh]
                //
                // Check if getResourceAsStream is called.
                //	    
                boolean insideGetResourceAsStream = false;

                synchronized(syncResourceAsStream)
                    {
                        insideGetResourceAsStream = resourceAsStreamInCall;
                    }		

                // If getResourceAsStream is called, don't 
                // trigger the following code. Otherwise,
                // unnecessary connection will be made.
                //
                if (insideGetResourceAsStream == false)
                    {
                        // otherwise, try the code base
                        try {
                            url = new URL(base, parseUtilEncodePath(name, false));
                            // check if resource exists
                            if(!resourceExists(url)) {
                                url = null;
			    } else {
				checkResource(name);
			    }
                        } catch (Exception e) {
                            // all exceptions, including security exceptions, are caught
                            url = null;
                        }
                    }
            }
        return url;
    }


    /*
     * Returns an enumeration of all the applet resources with the specified
     * name. First checks loaded JAR files then the applet code base for all
     * available resources.
     */
    public Enumeration findResources(String name) throws IOException {
    
        final Enumeration e = super.findResources(name);

        // 6215746:  Disable META-INF/* lookup from codebase in 
        // applet/plugin classloader. [stanley.ho] 
        if (name.startsWith("META-INF/"))  
                                               return e;
	    
        // 4668479: Option to turn off codebase lookup in AppletClassLoader 
        // during resource requests. [stanley.ho]
        if (getCodebaseLookup() == false)
            return e;

        URL u = new URL(base, parseUtilEncodePath(name, false));
        if (!resourceExists(u)) {
            u = null;
        } else {
	    try {
		checkResource(name);
	    } catch (Exception ex) {
		u = null;
	    }
	}
	
        final URL url = u;
        return new Enumeration() {
                private boolean done;
                public Object nextElement() {
                    if (!done) {
                        if (e.hasMoreElements()) {
                            return e.nextElement();
                        }
                        done = true;
                        if (url != null) {
                            return url;
                        }
                    }
                    throw new NoSuchElementException();
                }
                public boolean hasMoreElements() {
                    return !done && (e.hasMoreElements() || url != null);
                }
            };
    }


    //----------------------------------------------------------------------
    // Applet2Manager service
    //

    /*
     * Adds the specified JAR file to the search path of loaded JAR files.
     * Changed modifier to protected in order to be able to overwrite addJar()
     * in PluginClassLoader.java
     */
    protected void addJar(String name) throws IOException {
	if (name == null || name.equals("")) return;

        URL url;
	if (DEBUG) {
	    System.out.println("Applet2ClassLoader: addJar:  base "+base+ " jar: "+name);
	}

        try {
            url = new URL(base, name);
        } catch (MalformedURLException e) {
            throw new IllegalArgumentException("name");
        }
        addURL(url);
    }

    /*
     * Adds the specified JAR file to the search path of loaded JAR files.
     */
    public void addLocalJar(URL url) {
        addURL(url);
    }


    //----------------------------------------------------------------------
    // Applet2*Factory service
    //

    /*
     * Returns the applet code base URL.
     */
    public URL getBaseURL() {
        return base;
    }

    /**
     * Returns an input stream for reading the specified resource from the 
     * the loaded jar files.
     *
     * The search order is described in the documentation for {@link
     * #getResource(String)}.<p>
     * 
     * @param  name the resource name
     * @return an input stream for reading the resource, or <code>null</code>
     *         if the resource could not be found
     * @since  JDK1.1
     */
    public InputStream getResourceAsStreamFromJar(String name) {    
    
        if (name == null) {    	
            throw new NullPointerException("name");
        }       	
	
        try {		   	    	
            InputStream is = null;
            synchronized(syncResourceAsStreamFromJar) {
                resourceAsStreamFromJarInCall = true;		
                // Call super class
                is = super.getResourceAsStream(name);
                resourceAsStreamFromJarInCall = false;		
            }

            return is;
        } catch (Exception e) {		
            return null;	    
        }
    }

    //----------------------------------------------------------------------
    // Private utility functions
    //

    private boolean resourceExists(URL url) {
        // Check if the resource exists.
        // It almost works to just try to do an openConnection() but
        // HttpURLConnection will return true on HTTP_BAD_REQUEST
        // when the requested name ends in ".html", ".htm", and ".txt"
        // and we want to be able to handle these
        //
        // Also, cannot just open a connection for things like FileURLConnection,
        // because they succeed when connecting to a nonexistent file.
        // So, in those cases we open and close an input stream.
        boolean ok = true;
        try {
            URLConnection conn = url.openConnection();
            if (conn instanceof java.net.HttpURLConnection) {
                java.net.HttpURLConnection hconn = 
                    (java.net.HttpURLConnection) conn;

                // To reduce overhead, using http HEAD method instead of GET method
                hconn.setRequestMethod("HEAD");

                int code = hconn.getResponseCode();
                if (code == java.net.HttpURLConnection.HTTP_OK) {
                    return true;
                }
                if (code >= java.net.HttpURLConnection.HTTP_BAD_REQUEST) {
                    return false;
                }
            } else {
                /**
                 * Fix for #4182052 - stanleyh
                 *
                 * The same connection should be reused to avoid multiple
                 * HTTP connections
                 */

                // our best guess for the other cases
                InputStream is = conn.getInputStream();
                is.close();
            }
        } catch (Exception ex) {
            ok = false;
        }
        return ok;
    }

    /*
     * Returns the contents of the specified URL as an array of bytes.
     */
    private static byte[] getBytes(URL url) throws IOException {
        URLConnection uc = url.openConnection();
        if (uc instanceof java.net.HttpURLConnection) {
            java.net.HttpURLConnection huc = (java.net.HttpURLConnection) uc;
            int code = huc.getResponseCode();
            if (code >= java.net.HttpURLConnection.HTTP_BAD_REQUEST) {
                throw new IOException("open HTTP connection failed:" + url);
            }
        }
        int len = uc.getContentLength();

        // Fixed #4507227: Slow performance to load
        // class and resources. [stanleyh]
        //
        // Use buffered input stream [stanleyh]
        InputStream in = new BufferedInputStream(uc.getInputStream());
	
        byte[] b;
        try {
            if (len != -1) {
                // Read exactly len bytes from the input stream
                b = new byte[len];
                while (len > 0) {
                    int n = in.read(b, b.length - len, len);
                    if (n == -1) {
                        throw new IOException("unexpected EOF");
                    }
                    len -= n;
                }
            } else {
                // Read until end of stream is reached - use 8K buffer
                // to speed up performance [stanleyh]
                b = new byte[8192];
                int total = 0;
                while ((len = in.read(b, total, b.length - total)) != -1) {
                    total += len;
                    if (total >= b.length) {
                        byte[] tmp = new byte[total * 2];
                        System.arraycopy(b, 0, tmp, 0, total);
                        b = tmp;
                    }
                }
                // Trim array to correct size, if necessary
                if (total != b.length) {
                    byte[] tmp = new byte[total];
                    System.arraycopy(b, 0, tmp, 0, total);
                    b = tmp;
                }
            }
        } finally {
            in.close();
        }
        return b;
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
