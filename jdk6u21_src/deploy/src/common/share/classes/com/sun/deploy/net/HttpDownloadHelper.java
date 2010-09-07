/*
 * @(#)HttpDownloadHelper.java	1.31 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.net;
import java.io.*;
import java.net.URL;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import java.util.jar.Pack200;
import java.util.jar.JarOutputStream;
import java.util.zip.*;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import com.sun.deploy.config.Config;
import com.sun.deploy.net.DownloadEngine;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.Environment;



/** Implementation class for the HttpRequest and HttpResponse
 *  interfaces.
 */
final class HttpDownloadHelper implements HttpDownload {
    // Default size of download buffer
    private static final int BUF_SIZE = 32 * 1024;
    
    private static final int BUFFER_SIZE = 8192;
    
    // File extensions for cache files
    private static final String JAR_FILE_EXT = ".jar";
    private static final String JARJAR_FILE_EXT = ".jarjar";
    private static final String META_FILE_DIR = "meta-inf/";
       
    private HttpRequest _httpRequest;
    
    public HttpDownloadHelper(HttpRequest httpRequest) {
	_httpRequest = httpRequest;
    }

    public void download(int contentLength, URL url, InputStream in,
            String encoding, File file, final HttpDownloadListener dl, int contentType) throws
            CanceledDownloadException, IOException {
        // Tell delegate about loading
	final int length = contentLength;
	if (dl != null) dl.downloadProgress(0, length);
        if (Trace.isTraceLevelEnabled(TraceLevel.NETWORK)) {
	    Trace.println(ResourceManager.getString(
                "httpDownloadHelper.doingDownload", 
                url == null ? "" : url.toString(), contentLength, encoding),
                TraceLevel.NETWORK);
        }
        int jarCompressionLevel = DownloadEngine.getJarCompressionLevel();
     
	OutputStream out = null;
        JarOutputStream jarout = null;
        ZipInputStream zin = null;
        ZipInputStream jarjarIn = null;
	try {
            // PACK 200 case
	    if (encoding != null && (encoding.compareTo(
                    HttpRequest.PACK200_GZIP_ENCODING) == 0) && 
                    DownloadEngine.isPack200Supported()) {
	
		Pack200.Unpacker upkr200  = Pack200.newUnpacker();
		
		upkr200.addPropertyChangeListener( new PropertyChangeListener() {
		    public void propertyChange(java.beans.PropertyChangeEvent e) {
			if (dl != null && e.getPropertyName().compareTo(
                                Pack200.Unpacker.PROGRESS) == 0) {
			    String value = (String) e.getNewValue();
			    int val = (value != null) ? 
                                Integer.parseInt(value) : 0 ;
			    dl.downloadProgress((val*length)/100, length);
			}	    
	    	    }
		});
                out = new BufferedOutputStream(new FileOutputStream(file));
		JarOutputStream jout = new JarOutputStream(out);
                if (Environment.isJavaPlugin() && jarCompressionLevel != 0) {
                    jout.setLevel(jarCompressionLevel);
                }
		upkr200.unpack(new GZIPInputStream(in), jout);
		jout.close(); // Must be done, to write the signature.
	    } else { // takes care of GZIPInputstream and InputStream
                // special case for jar file to support jar compression
                // and jar-jar format
                if (DownloadEngine.isAlwaysCached(url.toString()) &&
                        DownloadEngine.isZipFile(url.toString()) == false) {
                    jarout =  new JarOutputStream(new BufferedOutputStream(
		          new FileOutputStream(file)));
                    if (jarCompressionLevel != 0) {
                        jarout.setLevel(jarCompressionLevel);
                    }
                    if (encoding != null && 
                            encoding.indexOf(HttpRequest.GZIP_ENCODING) >= 0) {
                        zin = new ZipInputStream(new GZIPInputStream(
                                new BufferedInputStream(in),
                                BUFFER_SIZE));
                        decompressWrite(zin, jarout, length, dl);
                    } else {
                        zin = new ZipInputStream(new BufferedInputStream(
                                in, BUFFER_SIZE));
                        
                        ZipEntry entry;
                        // Check for .jarjar file
                        if (url.toString().toLowerCase().endsWith(
                                JARJAR_FILE_EXT)) {
            
                            entry = zin.getNextEntry();
                            while (entry != null) {
                                if (entry.toString().toLowerCase().startsWith(
                                        META_FILE_DIR)) {
                                    //Ignore meta-files inside a .jarjar file
                                    entry = zin.getNextEntry();
                                } else if (! entry.toString().toLowerCase().
                                        endsWith(JAR_FILE_EXT)) {
                                    // other than .jar files are not allowed in 
                                    // .jarjar
                                    throw new IOException(
                                            "cache.jarjar.invalid_file");
                                } else {
                               
                                    //if jar is found break the loop
                                    break;
                                }
                            }
                            
                            jarjarIn = zin;
                            zin = new ZipInputStream(zin);
                        }
                        
                        decompressWrite(zin, jarout, length, dl);
                        
                        // make sure that only one jar file was found in 
                        // .jarjar file
                        if(jarjarIn != null) {
                            entry = jarjarIn.getNextEntry();
                            if(entry != null) {
                                String msg = null;
                                if (!entry.toString().toLowerCase().endsWith(
                                        JAR_FILE_EXT)) {
                                    msg = "cache.jarjar.invalid_file";
                                } else {
                                    msg = "cache.jarjar.multiple_jar";
                                }
                                
                                throw new IOException(msg);
                            }
                        }
                    }
                } else {
                    // non-jar items
                    // we now support GZIP for non-jar items also
                    InputStream is = new BufferedInputStream(in);
                    URL urlNoQuery = HttpUtils.removeQueryStringFromURL(url);
                    // only decompress gzip format if the original request
                    // does not ends with .gz; otherwise the user code
                    // might expect a GZIP input stream
                    if (urlNoQuery != null && 
                            urlNoQuery.toString().toLowerCase().endsWith(".gz") == 
                            false) {
                        if (encoding != null &&
                                encoding.indexOf(HttpRequest.GZIP_ENCODING) >= 0) {
                            is = new GZIPInputStream(is, BUFFER_SIZE);
                        }
                    }

                    int read = 0;
                    int totalRead = 0;
                    byte[] buf = new byte[BUF_SIZE];
                    out = new BufferedOutputStream(new FileOutputStream(file));

		    int i = 0;
                    while ((read = is.read(buf, 0, buf.length)) != -1) {
			// Check the first 4 byte of magic number is jar file
			if (DownloadEngine.isJarContentType(contentType)) {
			    if (i==0 && !DownloadEngine.isJarHeaderValid(buf)) {
                		throw new IOException("Invalid jar file");
            		    }
			}

                        out.write(buf, 0, read);
                        // Notify delegate
                        totalRead += read;
                        if (totalRead > length && length != 0) {
                            totalRead = length;
                        }
                        if (dl != null) {
                            dl.downloadProgress(totalRead, length);
                        }
			i++;
                    }
                }
	    }
            if (Trace.isTraceLevelEnabled(TraceLevel.NETWORK)) {
                Trace.println(ResourceManager.getString(
                    "httpDownloadHelper.wroteUrlToFile", 
                    url == null ? "" : url.toString(), 
                    file == null ? "" : file.toString()), 
                    TraceLevel.NETWORK);
            }
	    if (in != null) {
                in.close(); 
                in = null;
            }
            if (out != null) {
                out.close(); 
                out = null;
            }
            if (zin != null) {
                zin.close();
                zin = null;
            }
            if (jarout != null) {
                jarout.close();
                jarout = null;
            }
	} catch(IOException ioe) {
	    	  	    
	    // Close before calling delete - otherwise it fails
	    if (in != null)  { in.close(); in = null; }
	    if (out != null) { out.close(); out = null; }
            if (zin != null) { zin.close(); zin = null; }
            if (jarout != null) { jarout.close(); jarout = null; }
	    if (file != null) file.delete();
	    // Rethrow exception
	    throw ioe;
	}
	
	// Inform delegate about loading has completed
	if (dl != null) dl.downloadProgress(length, length);        
    }

    // Private helper function to read each zipentry,
    // decompress and write.    
    private void decompressWrite(ZipInputStream in, ZipOutputStream out, 
            int length, final HttpDownloadListener dl) throws IOException {
        // Decompress each entry
        byte[] buffer = new byte[BUFFER_SIZE];
        ZipEntry entry = in.getNextEntry();
        int totalRead = 0;
        while (entry != null) {
            // It is expensive to create new ZipEntry objects
            // when compared to cloning the existing entry.
            // We need to reset the compression size, since we
            // are changing the compression ratio of the entry.
            ZipEntry outEntry = (ZipEntry)entry.clone();
            outEntry.setCompressedSize(-1);
            out.putNextEntry(outEntry);
            
            int read = 0;
            
            while((read = in.read(buffer, 0, buffer.length)) != -1) {
                out.write(buffer, 0, read);
                // Notify delegate
                totalRead += read;
                if (totalRead > length && length != 0) {
                    totalRead = length;
                }
                if (dl != null) {
                    dl.downloadProgress(totalRead, length);
                }
            }
            
            out.closeEntry();
            entry = in.getNextEntry();
        }
        out.flush();
    }
    
    class PropertyChangeListenerTask implements PropertyChangeListener {
	HttpDownloadListener _dl = null;
	PropertyChangeListenerTask(HttpDownloadListener dl) {
	    _dl = dl;
	}

	public void propertyChange(PropertyChangeEvent e) {

	    if (e.getPropertyName().compareTo(Pack200.Unpacker.PROGRESS) == 0) {
		String value = (String) e.getNewValue();
		if (_dl != null && value != null) {
		    _dl.downloadProgress(Integer.parseInt(value), 100);
		}
	    }
	}
    }
}





