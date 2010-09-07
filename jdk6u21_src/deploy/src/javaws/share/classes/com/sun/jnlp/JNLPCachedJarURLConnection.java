/*
 * @(#)JNLPCachedJarURLConnection.java	1.18 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.jnlp;

import java.net.MalformedURLException;
import java.net.URL;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.InputStream;
import java.io.BufferedInputStream;
import java.io.IOException;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;
import java.util.jar.Manifest;
import java.util.jar.Attributes.Name;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.URLUtil;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.cache.CachedJarFile;
import com.sun.deploy.cache.CachedJarFile14;
import com.sun.deploy.net.DownloadEngine;
import com.sun.deploy.net.BasicHttpRequest;
import com.sun.javaws.jnl.JARDesc;

// note - dependant on sun.net.www.protocol.jar.JarURLConnection !!!
import sun.net.www.protocol.jar.JarURLConnection;

import com.sun.javaws.net.protocol.jar.Handler;

/*
 *
 * This class handles URL connections to JAR files and caches the JAR  files
 *
 */
public class JNLPCachedJarURLConnection 
	     extends sun.net.www.protocol.jar.JarURLConnection {

    // The URL of the jar file
    private URL _jarFileURL = null;

    // The name of the jar entry this url points to
    private String _entryName;

    // The jar entry this url points to
    private JarEntry _jarEntry;

    // The jar file corresponding to this connection
    private JarFile _jarFile;

    private String _contentType;

    private boolean _useCachedJar = false;
    
    private Map headerFields = null;

    // Constructor
    public JNLPCachedJarURLConnection(URL url, Handler handler)
            throws MalformedURLException, IOException {
        super(url, handler);

	// Obtain jar file URL
	getJarFileURL();
	_entryName = getEntryName();
    }
    
    public String getHeaderField(String name) {
        if (name == null) {
            return null;
        }
        try {
            connect();
        } catch (IOException ioe) {
            Trace.ignoredException(ioe);
        }
        if (headerFields != null && 
                BasicHttpRequest.isHeaderFieldCached(name)) {
            // use cached headers if available
            // only certain header field name is cached
            // convert header field name to lower case first, because all cached
            // header field names are stored in lower case, see
            // com.sun.deploy.net.BasicHttpRequest, fieldName array
            List headers = (List)headerFields.get(name.toLowerCase());
            if (headers != null) {
                return (String)headers.get(0);
            }
            return null;
        }
        // fallback to superclass
        return  super.getHeaderField(name);	
    }

    // Get the JAR file URL for this connection
    public URL getJarFileURL() 
    {       
	if (_jarFileURL == null)
	{
	    // Obtain jar file URL from parent class
	    _jarFileURL = super.getJarFileURL();

	}
	return _jarFileURL;
    }

    // Get the JAR file for this connection
    public JarFile getJarFile() throws IOException {
        connect();
        try {
            return (JarFile) java.security.AccessController.doPrivileged(
                    new java.security.PrivilegedExceptionAction() {

                        public Object run() throws Exception {
                            if (_jarFile instanceof CachedJarFile) {
                                try {
                                    /* If we got here this means that jar was saved into cache
                                     * and index file generated.
                                     * Any update of jar will require generation
                                     * of new idx file.
                                     *
                                     * On other hand, generation of index file checks the
                                     * signatures of the all entries, i.e. we
                                     * do not need to doublecheck them here.
                                     *
                                     * We create copy of CachedJarFile object
                                     * that will return null if signers are requested.
                                     * This will avoid redundant check of loaded classes
                                     * and even more importnat that this helps to avoid reading
                                     * of set of signers at all if cache was not updated since last run.
                                     *
                                     * Note that this is only used by JNLP class loader
                                     * that does not need signers for other purposes.
                                     */
                                    CachedJarFile nf = (CachedJarFile) ((CachedJarFile) _jarFile).clone();
                                    // temporary disable ignoreSigners performance enhancement
                                    //nf.setIgnoreSigners();
                                    return (JarFile) nf;
                                } catch (CloneNotSupportedException cnse) {
                                    throw new IOException(cnse.getMessage());
                                }
                            } else if (_jarFile instanceof CachedJarFile14) {
                                try {
                                    return (JarFile) (((CachedJarFile14) _jarFile).clone());
                                } catch (CloneNotSupportedException cnse) {
                                    throw new IOException(cnse.getMessage());
                                }
                            } else {
                                String path = _jarFile.getName();
                                if ((new File(path)).exists()) {
                                    // no cache case
                                    JarFile jf = new JarFile(path);
                                    Manifest mf = jf.getManifest();
                                    // ignore Class-Path attribute when running in Java Web Start
                                    if (mf != null) {
                                        mf.getMainAttributes().remove(Name.CLASS_PATH);
                                    }
                                    return jf;
                                }
                                // jar downloaded but not by download engine
                                return _jarFile;
                            }
                        }
                    });
        } catch (java.security.PrivilegedActionException e) {
            throw new IOException(e.getCause().getMessage());
        }

    }
    
    // Get the JAR file for this connection
    private JarFile getJarFileInternal() throws IOException {
        connect();
        return _jarFile;
    }    

    // Get the JAR file entry for this connection
    public JarEntry getJarEntry() throws IOException {
        connect();
        return _jarEntry;
    }

    // Connect to the server
    public void connect() throws IOException {
        if (!connected) {

            // Try to load the JAR file through the cache
	    _jarFile = JNLPClassLoaderUtil.getInstance().getJarFile(_jarFileURL);

	    // in our case we only can get the jar file from the cache
            // so we don't ever call super.getJarFile();

            if (_jarFile != null) {
		_useCachedJar = true;
	    } else {
		// If the JAR file could not be loaded using the cache,
		// try connecting to it directly through the superclass.
		super.connect();
		_jarFile = super.getJarFile();
            }
            
            JARDesc jd = 
                 JNLPClassLoaderUtil.getInstance().getJarDescFromURL(_jarFileURL);
            
            if (jd != null) {		
                Map ch = DownloadEngine.getCachedHeaders(jd.getLocation(),
                        null, jd.getVersion(), null, false);
		if (ch != null) {
		    headerFields = new HashMap();
		    headerFields = ch;
		}
            }

            // Get the JAR file entry, if one is requested
            if (_entryName != null) {
                _jarEntry = _jarFile.getJarEntry(_entryName);
                if (_jarEntry == null) {
                    throw new FileNotFoundException("JAR entry " +
                                                     _entryName +
                                                     " not found in " +
                                                     _jarFile.getName());
                }
	    }
            connected = true;
        }
    }

    // Get an input stream for the requested JAR entry
    public InputStream getInputStream() throws IOException {

        connect();

	if (_useCachedJar) {
            InputStream result = null;
	    
	    if (_entryName == null) {
	        throw new IOException("no entry name specified");
	    } 
	    if (_jarEntry == null) {
	        throw new FileNotFoundException("JAR entry " + _entryName +
			" not found in " + _jarFile.getName());
	    }
	    return _jarFile.getInputStream(_jarEntry);
        }
	return super.getInputStream();
    }

    public Object getContent() throws IOException {

	connect();

	if (_useCachedJar) {
	    if (_entryName == null) { 
	        return getJarFile();
	    }
	}
	return super.getContent();
    }

    public String getContentType() 
    {
	try {
	    connect();
	} catch(IOException e) {
	    // As in super class, do nothing
	}
		
	if (_useCachedJar) {
	    if (_contentType == null) {
	        if (_entryName == null) {
		    _contentType = "x-java/jar";
	        } else {
		    try {
		        connect();
		        InputStream in = getJarFileInternal().getInputStream(_jarEntry);
		        _contentType = guessContentTypeFromStream(
					new BufferedInputStream(in));
		        in.close();
		    } catch (IOException e) {
		        // don't do anything
		    }
	        }
	        if (_contentType == null) {
		    _contentType = guessContentTypeFromName(_entryName);
	        }
	        if (_contentType == null) {
		    _contentType = "content/unknown";
	        }
	    }
	} else {
	    _contentType = super.getContentType();
	}
	return _contentType;
    }


    // Get the length of the file.  We don't currently implement this.
    public int getContentLength() {
        //make sure we will use cached jar if available
        try {
            connect();
        } catch(IOException e) {
            return super.getContentLength();
        }

        if (_useCachedJar) {
            // If jarEntry is NULL, FileNotFoundException
            // will be thrown during connect() call
            // and we will get into catch block above
            if (_jarEntry != null)
                return (int) _jarEntry.getSize();
            return -1;
        }

        return super.getContentLength();
    }
}
