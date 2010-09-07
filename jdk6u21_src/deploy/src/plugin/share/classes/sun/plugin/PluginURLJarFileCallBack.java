/*
 * @(#)PluginURLJarFileCallBack.java	1.40 10/03/24
 *
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.plugin;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.FileNotFoundException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.URL;
import java.net.URLConnection;
import java.util.Enumeration;
import java.util.jar.JarFile;
import java.util.zip.ZipFile;
import java.util.zip.ZipEntry;
import java.security.cert.Certificate;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;
import sun.net.www.protocol.jar.URLJarFile;
import sun.net.www.protocol.jar.URLJarFileCallBack;
import com.sun.deploy.net.DownloadEngine;
import com.sun.deploy.net.HttpRequest;
import com.sun.deploy.net.HttpUtils;
import com.sun.deploy.config.Config;
import com.sun.deploy.cache.Cache;
import com.sun.deploy.cache.DeployCacheHandler;
import com.sun.deploy.net.DownloadEngine;
import com.sun.deploy.util.Trace;
import sun.awt.AppContext;

/*
 * This class implements function retrieve, in order to add functionality
 * to URLJarFile and handle cache failure when downloading JARJAR file.
 * 
 */

public class PluginURLJarFileCallBack implements URLJarFileCallBack
{

    private static int BUF_SIZE = 8192;
                
    public PluginURLJarFileCallBack()
    {}
    
    private void downloadJAR(URLConnection conn, 
            boolean useCompression) throws IOException {
        
        if (useCompression) {
            conn.setRequestProperty("accept-encoding",
                    "pack200-gzip, gzip");
        }
        
        // set the JAR content type
        conn.setRequestProperty(HttpRequest.CONTENT_TYPE,
                HttpRequest.JAR_MIME_TYPE);      
        
        InputStream is = null;
   
        try {    
            
            // this calls into DeployCacheHandler.get
            byte[] buf = new byte[BUF_SIZE];
            is = new BufferedInputStream(conn.getInputStream());
            while (is.read(buf) != -1) {
            }      
            
        } finally {
            
            if (is != null) {
                // this calls into DeployCacheHandler.put
                // DeployCacheHandler.put will check if the
                // resourceshould be cached             
                is.close();             
            }
        }
    }

    public JarFile retrieve(final URL url) throws IOException {
        JarFile result = null;

        final boolean isPackEnabled;
        
        if (Config.isJavaVersionAtLeast15()) {
            isPackEnabled =
                ((Boolean) AccessController.doPrivileged(new PrivilegedAction() {
                        public Object run() {
                            return Boolean.valueOf(System.getProperty("jnlp.packEnabled"));
                        }
                    })).booleanValue();
        } else {
            isPackEnabled = false;
        }
        
        URL u = null;

        if (isPackEnabled) {
            // first use pack200 .pack.gz url for making networking connection
            u = DownloadEngine.getPack200Url(url);
            // store the original url for cache lookup
            DeployCacheHandler.setDeployPackURL(url);
        } else {
            u = url;
        }
      
        /* get the stream before asserting privileges */
        final URLConnection conn =  u.openConnection();
        
        try {
            result = (JarFile)
            AccessController.doPrivileged(new PrivilegedExceptionAction() {
                public Object run() throws IOException {
                    URL urlNoQuery = new URL(url.getProtocol(), url.getHost(), url.getPort(),
                            url.getPath());
                    
                    String jarVersion = (String)AppContext.getAppContext().get(
                            Config.getAppContextKeyPrefix() +
                            urlNoQuery.toString());

		    boolean resourceNotCacheable = false;

		    try {
			resourceNotCacheable = 
			    DeployCacheHandler.resourceNotCached(urlNoQuery.toString());
		    } catch (Throwable t) {
			// DeployCacheHandler depends on ResponseCache which was
			// introduced in JDK 5.0, this failure is expected on 1.4.2
		    }
            
		    if (Cache.isCacheEnabled() && !resourceNotCacheable) {
                        try {
                            try {
                                downloadJAR(conn, true);
                            } catch (FileNotFoundException fnfe) {
                                if (isPackEnabled) {
                                    // retry without .pack.gz
                                    HttpUtils.cleanupConnection(conn);
                                    DeployCacheHandler.clearDeployPackURL();
                                    URLConnection conn2 = url.openConnection();
                                    downloadJAR(conn2, true);
                                } else {
                                    throw fnfe;
                                }
                            }
                        } catch (IOException ioe) {
                            Trace.ignoredException(ioe);
                            HttpUtils.cleanupConnection(conn);
                            URLConnection conn2 =  url.openConnection();
                            // retry with no HTTP compression
                            downloadJAR(conn2, false);
                        }
		
			// return cached jar file
			JarFile jf = DownloadEngine.getCachedJarFile(
					jarVersion == null ? url: urlNoQuery,
                                        jarVersion);
                 
			if (jf != null) {
			    
			    return jf;
			}          
		    }   
                    
                    int contentType = DownloadEngine.JAR_CONTENT_BIT;
                    if (isPackEnabled) {
                        contentType = contentType | 
                                DownloadEngine.PACK200_CONTENT_BIT;
                    }
                    
                    // Cache disabled
		    // use original url for cache disabled case for non
		    // versioned resource
                    File tmpFile = DownloadEngine.downloadJarWithoutCache(
                            jarVersion == null ? url : urlNoQuery, null,
                            jarVersion, null, contentType);
                    
                    //Check if this is the JARJAR file...
                    if (url.toString().toUpperCase().endsWith(".JARJAR")) {
                                /*
                                 * It is.  See what is inside the JARJAR file
                                 * There should be only manifest (optional) and
                                 * only one JAR file.
                                 */
                        JarFile tmpJar = new JarFile(tmpFile, false);
                        Enumeration e = tmpJar.entries();
                        ZipEntry ze = null;
                        
                        // Track number of JAR files inside the JARJAR.  If there is more
                        // then one, nothing should load, and exception should be thrown.
                        int jarCounter = 0;
                        
                        while(e.hasMoreElements()) {
                            ze = (ZipEntry) e.nextElement();
                            if (ze.getName().toUpperCase().startsWith("META-INF") )
                                //This is a manifest file, ignore it.
                                ;
                            else if (! ze.toString().toUpperCase().endsWith(".JAR"))
                                // There must not be anything but a JAR inside a JARJAR file.
                                throw new IOException("Invalid entry in jarjar file");
                            
                            else {
                                //Increment JAR Counter.
                                jarCounter++;
                                if (jarCounter>1)
                                    break;
                            }
                        } // end while
                        
                        if ( jarCounter > 1) {
                            //There is more then one JAR file inside the JARJAR file,
                            //throw an exception.
                            throw new IOException("Multiple JAR files inside JARJAR file");
                        }
                        
                        else {
                            //Extract the nested JAR file to the temp directory
                            InputStream nestedIn = null;
                            OutputStream nestedOut = null;
                            
                            try {
                                byte[] buf = new byte[BUF_SIZE];
                                int read = 0;
                                File nestedJar = File.createTempFile("jar_cache", null);
                                nestedJar.deleteOnExit();
                                nestedIn = new BufferedInputStream(tmpJar.getInputStream(ze));
                                nestedOut = new BufferedOutputStream(new FileOutputStream(nestedJar));
                                read = 0;
                                
                                while ((read = nestedIn.read(buf)) != -1)
                                    nestedOut.write(buf, 0, read);
                                
                                nestedOut.close();
                                nestedOut = null;
                                
                                tmpJar = new JarFile(nestedJar, false);
                            } finally {
                                if (nestedIn != null)
                                    nestedIn.close();
                                
                                if (nestedOut != null)
                                    nestedOut.close();
                            }
                        }
                        return tmpJar;
                        
                        
                    }
                    // This is not a JARJAR file, just a JAR, return it.
                    return new JarFile(tmpFile, false);
                    
                }
            });
        } catch (PrivilegedActionException pae) {
            throw (IOException) pae.getException();
        } finally {
            if (isPackEnabled) {
                // clear orignal url
                DeployCacheHandler.clearDeployPackURL();
            }
        }
        
        return result;
    }
}
