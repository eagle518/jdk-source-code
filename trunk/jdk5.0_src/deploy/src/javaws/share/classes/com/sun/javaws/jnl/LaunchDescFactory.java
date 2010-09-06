/*
 * @(#)LaunchDescFactory.java	1.31 04/01/21
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.jnl;
import java.net.URL;
import java.net.URLConnection;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.io.File;
import java.io.InputStream;
import java.io.BufferedInputStream;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.Reader;
import java.io.BufferedReader;
import java.io.ByteArrayInputStream;
import java.io.InputStreamReader;
import java.io.UnsupportedEncodingException;
import com.sun.javaws.exceptions.*;
import com.sun.javaws.util.URLUtil;
import com.sun.javaws.JavawsFactory;
import com.sun.javaws.net.*;
import com.sun.javaws.cache.Cache;
import com.sun.deploy.resources.ResourceManager;
/**
 * Factory class for parsing a JNL file
 *
 * The factory class can potentially understand
 * several different external JNL file formats, and
 * pass them into a JNLDescriptor object.
 */

public class LaunchDescFactory {
    /*
     * Constructs a LaunchDesc object form a stream
     * The InputStream object must support the reset() method
     *
     * The factory method can potentially understand several
     * different formats, such as a property file based one,
     * and a XML based one
     */
    public static LaunchDesc buildDescriptor(byte[] bits)
        throws IOException, BadFieldException, MissingFieldException, JNLParseException {
        return XMLFormat.parse(bits);
    }
    
    /** Constructs a LaunchDesc object from an input stream */
    public static LaunchDesc buildDescriptor(InputStream is)
        throws IOException, BadFieldException, MissingFieldException, JNLParseException  {
        return buildDescriptor(readBytes(is, -1));
    }
    
    /** Constructs a LaunchDesc object from an input stream */
    public static LaunchDesc buildDescriptor(InputStream is, long length)
        throws IOException, BadFieldException, MissingFieldException, JNLParseException {
        return buildDescriptor(readBytes(is, length));
    }
    
    /**
     * Constructs a LaunchDesc object from a file
     */
    public static LaunchDesc buildDescriptor(File f)
        throws IOException, BadFieldException, MissingFieldException, JNLParseException {
        return buildDescriptor(new FileInputStream(f), f.length());
    }
    
    /** Load launch file from URL */
    public static LaunchDesc buildDescriptor(URL url) throws
        IOException, BadFieldException, MissingFieldException, JNLParseException {       
	File cachedJnlpFile = Cache.getCachedLaunchedFile(url);	
	if (cachedJnlpFile != null) {	    
	    return buildDescriptor(cachedJnlpFile);
	}	
	HttpRequest httpreq = JavawsFactory.getHttpRequestImpl();
	HttpResponse response = httpreq.doGetRequest(url);
	InputStream is = response.getInputStream();
	int size = response.getContentLength();

        LaunchDesc ld =  buildDescriptor(is, size);

	is.close();

	return ld;
    }
    
    /** Load launch file from file/url. We read the entire launch file into a string for
     *  both efficentcy, but also so we only have to deal with IOExceptions one place
     */
    public static LaunchDesc buildDescriptor(String urlfile)
        throws IOException, BadFieldException, MissingFieldException, JNLParseException {
        InputStream is = null;
        int size = -1;
        try {
            URL url = new URL(urlfile);

	    return buildDescriptor(url);

        } catch(MalformedURLException e) {
	    // check for https support
	    if (e.getMessage().indexOf("https") != -1) {
		throw new BadFieldException(ResourceManager.getString("launch.error.badfield.download.https"), "<jnlp>", "https");

	    }
            // Try to open as file
            is = new FileInputStream(urlfile);
            long lsize = new File(urlfile).length();
            if (lsize > 1024 * 1024) throw new IOException("File too large");
            size = (int)lsize;
	}
        return buildDescriptor(is, size);
    }
    
    /** Create a launchDesc that instructs the client to launch the player */
    static public LaunchDesc buildInternalLaunchDesc(
		String cmd, String source, String tab) {
        return new LaunchDesc(
            "0.1",
            null,
            null,
            null,
            null,
            LaunchDesc.ALLPERMISSIONS_SECURITY,
            null,
            LaunchDesc.INTERNAL_TYPE,
            null,
            null,
            null,
            null,
            (tab == null) ? cmd : tab,
            source,
	    null);
    }
    
    static public byte[] readBytes(InputStream is, long size) throws IOException {
        // Sanity on file size (should not be a practical limitation, since
        // launch files must be small)
        if (size > 1024 * 1024) throw new IOException("File too large");
        
        BufferedInputStream bis = null;
        if (is instanceof BufferedInputStream) {
            bis = (BufferedInputStream)is;
        } else {
            bis = new BufferedInputStream(is);
        }
        
        if (size <= 0) size = 10*1024; // Default to 10K
        byte[] b = new byte[(int)size];
        int pos, n;
        int bytesRead = 0;
        n = bis.read(b, bytesRead, b.length - bytesRead);
        while(n != -1) {
            bytesRead += n;
            // Still room in array
            if (b.length == bytesRead) {
                byte[] bb = new byte[b.length * 2];
                System.arraycopy(b, 0, bb, 0, b.length);
                b = bb;
            }
            // Read next line
            n = bis.read(b, bytesRead, b.length - bytesRead);
        }
        bis.close();
        is.close();
        
        if (bytesRead != b.length) {
            byte[] bb = new byte[bytesRead];
            System.arraycopy(b, 0, bb, 0, bytesRead);
            b = bb;
        }
        return b;
    }
}

