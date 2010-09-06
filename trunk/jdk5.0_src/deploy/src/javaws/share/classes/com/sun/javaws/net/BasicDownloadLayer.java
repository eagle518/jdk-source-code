/*
 * @(#)BasicDownloadLayer.java	1.10 04/01/20
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.net;
import java.io.*;
import java.net.URL;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import java.util.jar.Pack200;
import java.util.jar.JarOutputStream;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import com.sun.javaws.Globals;



/** Implementation class for the HttpRequest and HttpResponse
 *  interfaces.
 */
public class BasicDownloadLayer implements HttpDownload {
    // Default size of download buffer
    private static final int BUF_SIZE = 32 * 1024;
       
    private HttpRequest _httpRequest;
    
    public BasicDownloadLayer(HttpRequest httpRequest) {
	_httpRequest = httpRequest;
    }
    
    /** Download resource to the given file */
    public void download(HttpResponse hr, File file, final HttpDownloadListener dl)
	throws CanceledDownloadException, IOException {
	// Tell delegate about loading
	final int length = hr.getContentLength();
	if (dl != null) dl.downloadProgress(0, length);
	
	Trace.println("Doing download", TraceLevel.NETWORK);

	
	InputStream in = hr.getInputStream();;
	BufferedOutputStream out = new BufferedOutputStream(new FileOutputStream(file));
	String encoding = hr.getContentEncoding();
	try {
	    if (encoding != null && (encoding.compareTo(HttpRequest.PACK200_GZIP_ENCODING) == 0) && Globals.havePack200()) {
		Trace.println("download:encoding Pack200: = " + encoding, TraceLevel.NETWORK);
		Pack200.Unpacker upkr200  = Pack200.newUnpacker();
		
		upkr200.addPropertyChangeListener(	new PropertyChangeListener() {
		    public void propertyChange(java.beans.PropertyChangeEvent e) {
			if (dl != null && e.getPropertyName().compareTo(Pack200.Unpacker.PROGRESS) == 0) {
			    String value = (String) e.getNewValue();
			    int val = (value != null) ? Integer.parseInt(value) : 0 ;
			    dl.downloadProgress((val*length)/100, length);
			}	    
	    	    }
		});
		JarOutputStream jout = new JarOutputStream(out);
		upkr200.unpack(in, jout);
		jout.close(); // Must be done, to write the signature.
	    } else { // takes care of GZIPInputstream and InputStream
		Trace.println("download:encoding GZIP/Plain = " + encoding, TraceLevel.NETWORK);
		int read = 0;
		int totalRead = 0;
		byte[] buf = new byte[BUF_SIZE];
		while ((read = in.read(buf)) != -1) {
		    out.write(buf, 0, read);
		    // Notify delegate
		    totalRead += read;
		    if (totalRead > length && length != 0) totalRead = length;
		    if (dl != null) dl.downloadProgress(totalRead, length);
		}
	    }
	    Trace.println("Wrote URL " + hr.getRequest() + " to file " + file, TraceLevel.NETWORK);
	    
	    in.close(); in = null;
	    out.close(); out = null;
	} catch(IOException ioe) {
	    
	    Trace.println("Got exception while downloading resource: " + ioe, TraceLevel.NETWORK);
	    
	    // Close before calling delete - otherwise it fails
	    if (in != null)  { in.close(); in = null; }
	    if (out != null) { out.close(); out = null; }
	    if (file != null) file.delete();
	    // Rethrow exception
	    throw ioe;
	}
	
	// Inform delegate about loading has completed
	if (dl != null) dl.downloadProgress(length, length);
    }
    
    /** Download resource to the given file */
    public void download(URL url, File location, HttpDownloadListener dl)
	throws CanceledDownloadException, IOException {
	HttpResponse hr = _httpRequest.doGetRequest(url);
	download(hr, location, dl);
	hr.disconnect();
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





