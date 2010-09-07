/*
 * @(#)CachedJarURLConnection.java	1.48 10/03/24
 *
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.net.protocol.jar;

import java.net.MalformedURLException;
import java.net.URL;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.InputStream;
import java.io.BufferedInputStream;
import java.io.IOException;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import com.sun.deploy.util.URLUtil;
import com.sun.deploy.net.DownloadEngine;
import com.sun.deploy.cache.Cache;
import com.sun.deploy.cache.DeployCacheHandler;
import com.sun.deploy.config.Config;
import sun.awt.AppContext;
import com.sun.deploy.net.URLEncoder;
import com.sun.deploy.cache.CachedJarFile;

// This class handles URL connections to JAR files and caches the JAR
// files to the local disk.
public class CachedJarURLConnection extends
    sun.net.www.protocol.jar.JarURLConnection {

    // The URL of the jar file
    private URL jarFileURL = null;
    private URL jarFileURLOverride = null;

    // The name of the jar entry this url points to
    private String entryName;

    // The jar entry this url points to
    private JarEntry jarEntry;

    // The jar file corresponding to this connection
    private JarFile jarFile;

    private String contentType;

    private boolean useJarCache = false;
    
    private Map headerFields = new HashMap();

    // Constructor
    public CachedJarURLConnection(URL url, Handler handler)
    throws MalformedURLException, IOException {
        super(url, handler);

	// Obtain jar file URL
	getJarFileURL();
	entryName = getEntryName();
    }
    
    // Get the JAR file URL for this connection
    public synchronized URL getJarFileURL() 
    {       
        if (jarFileURLOverride != null) {
            return jarFileURLOverride;
        }
	if (jarFileURL == null)
	{
	    // Obtain jar file URL from parent class
	    jarFileURL = super.getJarFileURL();

	    // To make jar file works nicely with UNC, we need to 
	    // canonicalize the URL
	    try
	    {
		jarFileURL = new URL(URLUtil.canonicalize(jarFileURL.toString()));
	    }
	    catch (MalformedURLException e)
	    {
	    }
	}
	return jarFileURL;
    }

    // Get the JAR file for this connection
    public synchronized JarFile getJarFile() throws IOException {
	jarFile = getJarFileInternal();
        try {
            return (JarFile)java.security.AccessController.doPrivileged(
                    new java.security.PrivilegedExceptionAction() {
                public Object run() throws Exception {
                    if (jarFile instanceof CachedJarFile) {                        
                        try {
                            return ((CachedJarFile)jarFile).clone();
                        } catch (CloneNotSupportedException cnse) {
                            throw new IOException(cnse.getMessage());
                        }                 
                    } else {
                        String path = jarFile.getName();
                        if (new File(path).exists()) {
                            // no cache case
                            return new JarFile(jarFile.getName());
                        }
                        // jar downloaded but not by download engine
                        return jarFile;
                    }
                }
            });          
        } catch (java.security.PrivilegedActionException e) {
            throw new IOException(e.getCause().getMessage());
        }
    }

    public synchronized JarFile getJarFileInternal() throws IOException {
        if (jarFile != null) {
            return jarFile;
        }
        if (Cache.isCacheEnabled() && Cache.isSupportedProtocol(jarFileURL)) {
            String jarVersion = (String)AppContext.getAppContext().get(
                    Config.getAppContextKeyPrefix() + jarFileURL.toString());
            if (jarVersion != null) {
                
                jarFileURLOverride = new URL(URLUtil.canonicalize(
                        jarFileURL.toString() + "?" + 
                        URLEncoder.encode("version-id", "UTF-8") + "=" +
                        URLEncoder.encode(jarVersion, "UTF-8")));
                
                // check current version if exists
                String currentVersion = Cache.getCacheEntryVersion(jarFileURL, 
                        null);
                
                if (currentVersion != null &&
                        currentVersion.equals(jarVersion) == false) {
                    jarFileURLOverride = new URL(jarFileURLOverride.toString() +
                            "&" +
                            URLEncoder.encode("current-version-id", "UTF-8") +
                            "=" + URLEncoder.encode(currentVersion, "UTF-8"));
                }               
            }
        }
        connect();
        jarFileURLOverride = null;
        return jarFile;
    }

    // Get the JAR file entry for this connection
    public JarEntry getJarEntry() throws IOException {
        connect();
        return jarEntry;
    }
    
    public String getHeaderField(String name) {
        String headerString = null;
        try {
            connect();
        } catch (IOException ioe) {
            ioe.printStackTrace();
        }
        List headers = (List)headerFields.get(name);
        if (headers != null) {
            headerString = (String)headers.get(0);
        }
        return headerString;
    }

    // Connect to the server
    public void connect() throws IOException {
        if (!connected) {
	    URL urlNoQuery = new URL(jarFileURL.getProtocol(),
                        jarFileURL.getHost(), jarFileURL.getPort(),
                        jarFileURL.getPath());

	    boolean resourceNotCacheable = false;

	    try {
		resourceNotCacheable = 
		    DeployCacheHandler.resourceNotCached(urlNoQuery.toString());
	    } catch (Throwable t) {
		// DeployCacheHandler depends on ResponseCache which was
		// introduced in JDK 5.0, this failure is expected on 1.4.2
	    }

            // Try to load the JAR file through the cache
            // Always try to use cached jar if cache is enabled
	    // and the resource was cacheable
            if(Cache.isSupportedProtocol(jarFileURL) && Cache.isCacheEnabled()
						    && !resourceNotCacheable) {
                
                String jarVersion = (String)AppContext.getAppContext().get(
                        Config.getAppContextKeyPrefix() +
                        urlNoQuery.toString());
                
                // set use cache to false to override the hashmap in
                // sun.net.www.protocol.jar.JarFileFactory,
                setUseCaches(false);
                
                super.connect();               
                
                jarFile = DownloadEngine.getCachedJarFile(jarVersion == null ? jarFileURL : urlNoQuery, 
		    jarVersion);
                
                headerFields = DownloadEngine.getCachedHeaders(jarVersion == null ? jarFileURL : urlNoQuery,
                        null, jarVersion, null, false);
                
                Cache.addLoadedResource(jarVersion == null ? jarFileURL : urlNoQuery, null, jarVersion);

                if (jarFile != null) {
                    useJarCache = true;
                } else {
                    // if the jar file cannot be cached, get it
                    // from the superclass
                    // See DeployCacheHandler.put for cases where resource
                    // cannot be cached
                    jarFile = super.getJarFile();
                }
            } else {
                // If the JAR file could not be loaded using the cache,
                // try connecting to it directly through the superclass.
                super.connect();
                jarFile = super.getJarFile();
            }
            
            // Get the JAR file entry, if one is requested
            if (entryName != null) {
                jarEntry = jarFile.getJarEntry(entryName);
                if (jarEntry == null) {
                    throw new FileNotFoundException("JAR entry " +
                            entryName +
                            " not found in " +
                            jarFile.getName());
                }
            }
            connected = true;
        }
    }

    // Get an input stream for the requested JAR entry
    public InputStream getInputStream() throws IOException {           
        connect();
        
	if (useJarCache == false)
	    return super.getInputStream();

        InputStream result = null;
	    
	if (entryName == null) {
	    throw new IOException("no entry name specified");
	} else {
	    if (jarEntry == null) {
	        throw new FileNotFoundException("JAR entry " + entryName +
						" not found in " +
					        jarFile.getName());
	    }
	    result = jarFile.getInputStream(jarEntry);
	}
        return result;
    }

    public Object getContent() throws IOException {

	Object result = null;

	connect();

	if (useJarCache == false)
	    return super.getContent();

	if (entryName == null) { 
	    result = getJarFile();
	} else {
	    result = super.getContent();
	}
	return result;
    }

    public String getContentType() 
    {
	// Zhengyu for fixing bug 4470007
	if(!connected)
	{
	   try
	   {
		connect();
	   }
	   catch(IOException e)
	   {
		//As super class, do nothing
	   }
	}
		
	if (useJarCache == false)
	    return super.getContentType();

	if (contentType == null) {
	    if (entryName == null) {
		contentType = "x-java/jar";
	    } else {
		try {
		    connect();
		    InputStream in = getJarFileInternal().getInputStream(jarEntry);
		    contentType = guessContentTypeFromStream(
					new BufferedInputStream(in));
		    in.close();
		} catch (IOException e) {
		    // don't do anything
		}
	    }
	    if (contentType == null) {
		contentType = guessContentTypeFromName(entryName);
	    }
	    if (contentType == null) {
		contentType = "content/unknown";
	    }
	}
	return contentType;
    }


    // Get the length of the file.  We don't currently implement this.
    public int getContentLength() {
	// Fixed bug 5009114
	if (!connected) {
	    try {
		connect();
	    }
	    catch(IOException e) {}
	}
	
	if (useJarCache == false)
	    return super.getContentLength();
	
	// If jarEntry is NULL, FileNotFoundException
	// will be thrown during connect() call.
	if (jarEntry != null)
	    return (int)jarEntry.getSize();

        return -1;
    }
}



