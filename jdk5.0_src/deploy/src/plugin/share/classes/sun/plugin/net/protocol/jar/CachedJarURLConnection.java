/*
 * @(#)CachedJarURLConnection.java	1.21 04/03/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.net.protocol.jar;

import java.net.MalformedURLException;
import java.net.URL;
import java.io.FileNotFoundException;
import java.io.InputStream;
import java.io.BufferedInputStream;
import java.io.IOException;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;
import sun.plugin.cache.JarCache;
import sun.plugin.util.Trace;
import sun.plugin.util.URLUtil;


// This class handles URL connections to JAR files and caches the JAR
// files to the local disk.
public class CachedJarURLConnection extends
    sun.net.www.protocol.jar.JarURLConnection {

    // The URL of the jar file
    private URL jarFileURL = null;

    // The name of the jar entry this url points to
    private String entryName;

    // The jar entry this url points to
    private JarEntry jarEntry;

    // The jar file corresponding to this connection
    private JarFile jarFile;

    private String contentType;

    private boolean useJarCache = false;

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
    public JarFile getJarFile() throws IOException {
        connect();
        return jarFile;
    }

    // Get the JAR file entry for this connection
    public JarEntry getJarEntry() throws IOException {
        connect();
        return jarEntry;
    }

    // Connect to the server
    public void connect() throws IOException {
        if (!connected) {

            // Try to load the JAR file through the cache
	    if(getUseCaches())
		jarFile = JarCache.get(jarFileURL);

            if (jarFile != null) 
	    {
		useJarCache = true;
	    }
	    else
	    {
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
		    InputStream in = getJarFile().getInputStream(jarEntry);
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



