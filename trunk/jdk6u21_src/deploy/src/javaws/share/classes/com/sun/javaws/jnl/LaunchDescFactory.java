/*
 * @(#)LaunchDescFactory.java	1.63 10/05/13
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.jnl;
import java.net.URL;
import java.net.MalformedURLException;
import java.net.UnknownHostException;
import java.net.ConnectException;
import java.net.SocketException;
import java.io.File;
import java.io.InputStream;
import java.io.BufferedInputStream;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.zip.GZIPInputStream;
import com.sun.javaws.exceptions.*;

import com.sun.deploy.net.*;
import com.sun.deploy.cache.Cache;
import com.sun.deploy.cache.LocalApplicationProperties;
import com.sun.deploy.config.Config;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.URLUtil;
import com.sun.deploy.Environment;
import com.sun.deploy.net.offline.DeployOfflineManager;


/**
 * Factory class for parsing a JNL file
 *
 * The factory class can potentially understand
 * several different external JNL file formats, and
 * pass them into a JNLDescriptor object.
 */

public class LaunchDescFactory {
    private static final boolean DEBUG = false;

    private static final int BUFFER_SIZE = 8192;
    
    private static URL derivedCodebase = null;
    private static URL docbase = null;

    public static void setDocBase(URL u) {
        docbase = u;
    }
    
    public static URL getDocBase() {
        return docbase;
    }
    
    public static URL getDerivedCodebase() {
        if (docbase != null && derivedCodebase == null) {
            try {
                derivedCodebase = new URL(docbase.toString().substring(0,
                        docbase.toString().lastIndexOf("/") + 1));
            } catch (MalformedURLException mue) {
                Trace.ignoredException(mue);
            }
        }
        return derivedCodebase;
    }

    /*
     * Constructs a LaunchDesc object form a byte array
     *
     * The factory method can potentially understand several
     * different formats, such as a property file based one,
     * and a XML based one
     *
     * @param codebase The applet parameter codebase, if exist, else null
     * @param documentbase The browsers documentbase. Complete with the document part if exist, else null.
     * @param originalRequest URL used to request this object. It may be differ
     *        from one used inside the file. If this value is not null then
     *        it will be used to overwrite href specified in the file.
     */
    public static LaunchDesc buildDescriptor(byte[] bits, URL codebase, 
            URL documentbase, URL originalRequest)
        throws IOException, BadFieldException, MissingFieldException, JNLParseException {
        return XMLFormat.parse(bits, codebase, documentbase,
                originalRequest, LaunchSelection.createDefaultMatchJRE());
    }

    /* Use this to construct LauncgDesc */
    public static LaunchDesc buildDescriptor(byte[] bits, URL codebase, URL documentbase)
        throws IOException, BadFieldException, MissingFieldException, JNLParseException {
            return buildDescriptor(bits, codebase, documentbase, null);
    }


    /** Constructs a LaunchDesc object from a file (cache). codebase and documentbase may be null */
    public static LaunchDesc buildDescriptor(File f, URL codebase, 
        URL documentbase, URL originalRequest)
        throws IOException, BadFieldException, MissingFieldException, JNLParseException {      
        if(DEBUG) {
            System.out.println("JNLP Build LaunchDesc jnlp file: "+f+", codebase: "+codebase+", documentbase: "+documentbase);
        }
        return buildDescriptor(readBytes(new FileInputStream(f), f.length()), 
                   codebase, documentbase, originalRequest);
    }

    /** Constructs a LaunchDesc object from a file (cache). codebase and documentbase got from LAP */
    public static LaunchDesc buildDescriptor(File f)
        throws IOException, BadFieldException, MissingFieldException, JNLParseException {

        LocalApplicationProperties lap = Cache.getLocalApplicationProperties(f.getPath());
        if (lap != null) {
            String docbaseStr = lap.getDocumentBase();
            String codebaseStr = lap.getCodebase();

            URL codebase = null;
            URL documentbase = null;

            try {
                codebase = new URL(codebaseStr);
            } catch (MalformedURLException e) {
            }

            try {
                documentbase = new URL(docbaseStr);
            } catch (MalformedURLException e) {
            }
            return buildDescriptor(f, codebase, documentbase, null);
        } else {
            String origFilename = System.getProperty("jnlpx.origFilenameArg");
            if (origFilename != null) {
                File origFile = new File(origFilename);
           
                URL codebase = null;
                try {
                    String filepath = origFile.getAbsoluteFile().getParent();
                    if (filepath.startsWith(File.separator)) {
                        // for unix path
                        filepath = filepath.substring(1, filepath.length());
                    }
                    codebase = new URL("file:/" + filepath +
                            File.separator);
                    
                } catch (MalformedURLException e) {
                    Trace.ignoredException(e);
                }
                if (codebase != null) {
                    LaunchDesc ld = buildDescriptor(f, codebase, null, null);
                    // Set working derivedCodebase for file path case
                    derivedCodebase = codebase;
                    return ld;
                }
            }
            return null;
        }
    }
    
    /** Load launch file from URL. documentbase may be null */
    public static LaunchDesc buildDescriptor(URL url, URL documentbase)
        throws IOException, BadFieldException, MissingFieldException, JNLParseException {

        // Try to get an updated cached launch descriptor.
        //     getCachedFile(URL resourceURL, String versionString, boolean doDownload,
        //                   boolean isPlatformRequest, String knownPlatforms)
        File cachedJnlpFile;
        IOException networkException = null;
        try {
            cachedJnlpFile = DownloadEngine.getCachedFile(url, null, true, false, null);
        } catch (IOException ioe) {
            if (ioe instanceof UnknownHostException ||
                    ioe instanceof FailedDownloadException || 
                    ioe instanceof ConnectException ||
                    ioe instanceof SocketException) {
                // currently we just ignore the above IOException and try
                // to use cached JNLP if available, so that we can allow
                // launching cached applications offline.
                // For more robust offline application support, we should
                // detect network connection issues as soon as possible
                // and switch to a real offline mode that uses cached resource
                // only.  We need to make sure we would not get into a state
                // when we applied partial update to cached resources when
                // there is intermitten network errors during application
                // launch and update.
                Trace.ignoredException(ioe);

                networkException = ioe;

                // cannot make network connection, get from cache
                cachedJnlpFile = DownloadEngine.getCachedFile(url, null, false,
                        false, null);

                if (cachedJnlpFile == null &&
                        DeployOfflineManager.isForcedOffline()) {
                    throw ioe;
                }
            } else {
                throw ioe;
            }
        }
      
        // deduce the absolute codebase from the location
        URL codebase = URLUtil.asPathURL(URLUtil.getBase(url));

        if (cachedJnlpFile != null && cachedJnlpFile.exists()) {
            if(DEBUG) {
                System.out.println("JNLP Build LaunchDesc jnlp URL->Cache: "+url+"->"+cachedJnlpFile+", codebase: "+codebase+", documentbase: "+documentbase);
            }
            LaunchDesc ld = buildDescriptor(cachedJnlpFile, codebase, 
                    documentbase, url);
            if (ld != null && ld.getLaunchType() == LaunchDesc.INTERNAL_TYPE) {
                // remove cached file
                // should not cache the cache viewer jnlp file
                DownloadEngine.removeCachedResource(url, null, null);
            }

            if (networkException != null) {
                if (ld.getInformation().supportsOfflineOperation() == false) {
                    throw networkException;
                }
                // JNLP allow running offline, ignore network exception and
                // continue
            }

            // Set working derivedCodebase for HTTP URL case
            derivedCodebase = codebase;
      
            return ld;
        }

        if(DEBUG) {
            System.out.println("JNLP Build LaunchDesc jnlp URL: "+url+", codebase: "+codebase+", documentbase: "+documentbase);
        }

        HttpRequest httpreq = DownloadEngine.getHttpRequestImpl();
        HttpResponse response = httpreq.doGetRequest(url);
        InputStream is = response.getInputStream();
        int size = response.getContentLength();    
        String encoding = response.getContentEncoding();

        if (encoding != null && 
                encoding.indexOf(HttpRequest.GZIP_ENCODING) >= 0) {
            is = new GZIPInputStream(is, BUFFER_SIZE);
        }

        LaunchDesc ld = buildDescriptor(readBytes(is, size), codebase, documentbase);

        is.close();

        return ld;
    }

    /** Load launch file from URL
     *
     * @param jnlpHRef The applet JNLP HREF, may be absolute, or relative to codebase, or documentbase (if no codebase exist).
     * priority is: absolute jnlp_href, codebase+jnlp_href, documentbase+jnlp_href,
     * where codebase is either absolute or documentbase+codebase !
     *
     * @param codebaseStr The applet parameter codebase, if exist, else null. A URL is being created, 
     * absolute or documentbase+codebase.
     * @param documentbase The browsers documentbase. Complete with the document part if exist, else null.
     */
    public static LaunchDesc buildDescriptor (String jnlpHRef, String codebaseStr, URL documentbase, boolean verbose)
        throws BadFieldException, MissingFieldException, JNLParseException
    {
        URL codebase = null;

        if(DEBUG) { verbose=true; } 

        if (verbose) {
            System.out.println("JNLP Build LaunchDesc jnlp: "+jnlpHRef+", codebaseStr: "+codebaseStr+", documentbase: "+documentbase);
        }

        try {
            codebase = URLUtil.asPathURL(new URL (codebaseStr));
            if (verbose) {
                System.out.println("   JNLP Codebase (absolute): "+codebase);
            }
        } catch (Exception e) { codebase=null; }

        if ( codebase==null ) {
            try {
                codebase = URLUtil.asPathURL(new URL (URLUtil.getBase(documentbase), codebaseStr));
                if (verbose) {
                    System.out.println("   JNLP Codebase (documentbase+codebase): "+codebase);
                }
            } catch (Exception e) { codebase=null; }
        }
        if ( verbose && codebase==null ) {
                    System.out.println("   JNLP Codebase (null)");
        }
        return buildDescriptor (jnlpHRef, codebase, documentbase, verbose);
    }

    private static LaunchDesc buildDescriptorFromCache(URL url, URL documentbase)
            throws BadFieldException, MissingFieldException, JNLParseException {
        try { 
            File f = DownloadEngine.getCachedFile(url, null, false, false, null);
            if (f != null) {

                // deduce the absolute codebase from the location
                URL codebase = URLUtil.asPathURL(URLUtil.getBase(url));

                return buildDescriptor(f, codebase, documentbase, url);
            }
        } catch (IOException e) {}
        return null;
    }

    public static LaunchDesc buildDescriptorFromCache(
            String jnlpHRef, URL codebase, URL documentbase)
        throws BadFieldException, MissingFieldException, JNLParseException {
        URL url = null;
        LaunchDesc ld = null;

        // 1: try absolute jnlp reference first ..
        try {
            url = new URL(jnlpHRef);
        } catch (Exception e) {}
        if (url != null) {
            ld = buildDescriptorFromCache(url, documentbase);
            if (ld != null)
                return ld;
        }

        // 2: try codebase + jnlp_href
        if (codebase != null) {
            try {
               url = new URL(codebase, jnlpHRef);
            } catch (Exception e) { url = null; }
            ld = buildDescriptorFromCache(url, documentbase);
            if (ld != null)
                return ld;
        }

        // 3: try documentbase + jnlp_href
        if (codebase == null && documentbase != null) {
            try {
               url = new URL(URLUtil.getBase(documentbase), jnlpHRef);
            } catch (Exception e) { url = null; }
            ld = buildDescriptorFromCache(url, documentbase);          
            if (ld != null)
                return ld;
        }
        return null;
    }


    /** Load launch file from URL
     *
     * @param jnlpHRef The applet JNLP HREF, may be absolute, or relative to codebase, or documentbase (if no codebase exist).
     * priority is: absolute jnlp_href, codebase+jnlp_href, documentbase+jnlp_href,
     * where codebase is either absolute or documentbase+codebase !
     *
     * @param codebase The absolute codebase URL
     * @param documentbase The browsers documentbase. Complete with the document part if exist, else null.
     */
    public static LaunchDesc buildDescriptor (String jnlpHRef, URL codebase, URL documentbase, 
                                              boolean verbose)
        throws BadFieldException, MissingFieldException, JNLParseException
    {
        URL url = null;

        // 1: try absolute jnlp reference first ..
        try {
            url = new URL(jnlpHRef);
        } catch (Exception e) { if(DEBUG) { System.out.println(e); e.printStackTrace(); } url=null; }

        if ( url!=null) {
           try {
            LaunchDesc ld = buildDescriptor(url, documentbase);
            if (verbose) {
                    System.out.println("   JNLP Ref (absolute): "+url.toString());
            }
            return ld;
           } catch (BadFieldException e) {
               throw (e);
           } catch (MissingFieldException e) {
               throw (e);
           } catch (JNLParseException e) {
               throw (e);
           } catch (Exception e) { if(verbose) { System.out.println(e); e.printStackTrace(); } url=null; }
        }

        // 2: try codebase + jnlp_href
        if(codebase!=null) {
            try {
               url = new URL(codebase, jnlpHRef);
            } catch (Exception e) { if(DEBUG) { System.out.println(e); e.printStackTrace(); } url=null; }

            if ( url!=null) {
               try {
                  LaunchDesc ld = buildDescriptor(url, documentbase);
                  if (verbose) {
                    System.out.println("   JNLP Ref (codebase + ref): "+url.toString());
                  }
                  return ld;
               } catch (BadFieldException e) {
                   throw (e);
               } catch (MissingFieldException e) {
                   throw (e);
               } catch (JNLParseException e) {
                   throw (e);
               } catch (Exception e) { if(verbose) { System.out.println(e); e.printStackTrace(); } url=null; }
            }
        }

        // 3: try documentbase + jnlp_href
        if(codebase==null && documentbase!=null) {
            try {
               url = new URL(URLUtil.getBase(documentbase), jnlpHRef);
            } catch (Exception e) { if(DEBUG) { System.out.println(e); e.printStackTrace(); } url=null; }

            if ( url!=null) {
               try {
                  LaunchDesc ld = buildDescriptor(url, documentbase);
                  if (verbose) {
                    System.out.println("   JNLP Ref (documentbase + ref): "+url.toString());
                  }
                  return ld;
               } catch (BadFieldException e) {
                   throw (e);
               } catch (MissingFieldException e) {
                   throw (e);
               } catch (JNLParseException e) {
                   throw (e);
               } catch (Exception e) { if(verbose) { System.out.println(e); e.printStackTrace(); } url=null; }
            }
        }

        if (verbose) {
            System.out.println("   JNLP Ref (...): NULL !");
        }
        return null;
    }


    /** Load launch file from file/url. We read the entire launch file into a string for
     *  both efficentcy, but also so we only have to deal with IOExceptions one place
     */
    public static LaunchDesc buildDescriptor(String urlfile)
        throws IOException, BadFieldException, MissingFieldException, JNLParseException {
        InputStream is = null;
        String alt_codebase = null;
        int size = -1;
        try {     
            URL url = new URL(urlfile);

            return buildDescriptor(url, null);

        } catch(MalformedURLException e) {
            File f = new File(urlfile);
            // check for https support
            if ((!f.exists()) && (!Config.isJavaVersionAtLeast14()) &&
                    e.getMessage().indexOf("https") != -1) {
            throw new BadFieldException(
                        ResourceManager.getString("launch.error.badfield.download.https"), 
                            "<jnlp>", "https");

            }
            // Try to open as file
            is = new FileInputStream(urlfile);
            long lsize = f.length();
            if (lsize > 1024 * 1024) throw new IOException("File too large");
            size = (int)lsize;
            if (Environment.isImportMode()) {
                String dir = f.getParent();
                if (Environment.getImportModeCodebaseOverride() == null && 
                    dir != null) try {
                    URL altCodebase = new URL("file", null, 
                        URLUtil.encodePath(dir));
                    Environment.setImportModeCodebaseOverride(
                        altCodebase.toString());
                } catch (MalformedURLException mue) {
                    Trace.ignoredException(mue);
                }
            }
	    }
        return buildDescriptor(readBytes(is, size), null, null);
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
            null,
            LaunchDesc.INTERNAL_TYPE,
            null,
            null,
            null,
            null,
            (tab == null) ? cmd : tab,
            source,
	        null,
            LaunchSelection.createDefaultMatchJRE());
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

