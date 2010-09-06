/*
 * @(#)CachedFileLoader.java	1.23 04/06/18
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package sun.plugin.cache;

import java.io.File;
import java.io.RandomAccessFile;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.FileOutputStream;
import java.io.BufferedInputStream;
import java.io.ByteArrayOutputStream;
import java.io.BufferedOutputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.URL;
import java.net.URLConnection;
import java.text.DateFormat;
import java.util.Date;
import java.util.Enumeration;
import java.util.Iterator;
import java.net.HttpURLConnection;
import sun.plugin.net.protocol.http.HttpUtils;
import sun.plugin.util.Trace;
import sun.net.www.MessageHeader;
import java.util.Map;
import java.security.AccessController;
import java.security.PrivilegedActionException;

// Class to handle loading of jar files from the cache
public class CachedFileLoader {
    
    private URL url;
    private HttpURLConnection uc = null;
    private long lastModified = 0;
    private long expiration = 0;
    
    // The jar's up-to-date status, file descriptor and size.
    private boolean upToDate = false;
    private File dataFile = null;
    private File indexFile = null;
    private boolean cached = false;
    private long size = 0;
    private MessageHeader headerFields = null;

    private static String[] fieldName = {
	"content-length",
	"last-modified",
	"expires",
	"content-type",
	"content-encoding",
	"date",
	"server",
    };

    public CachedFileLoader(URL url) throws IOException {
        this.url = url;
    }
            
    // Public query for the jar size.
    public long getFileSize() {
        return size;
    }

    public URL getURL() {
        return url;
    }

    // Public query for the jar size.
    public void setDataFile(File file) {
        dataFile = file;
    }

    public void setIndexFile(File file) {
        indexFile = file;
    }

    public void setLastModify(long time) {
        lastModified = time;
    }

    public void setExpiration(long time) {
        expiration = time;
    }

    public void setHeaders(MessageHeader headers) {
        headerFields = headers;
    }

    // Load the jar file
    public CachedFile load() throws IOException {
	boolean done = true;
        // Try to load the file from the cache
        // Get the file descriptor up front.
        cached = getCacheFile();
	if(cached) {
	    done = loadFromCache();
	} else {
	    try {
		done = download();
	    }catch(IOException e) {
		// have to make sure it is connection failure
		if(uc != null) {
		    int response = uc.getResponseCode();
		    if ((response < 200) || (response >= 300)) {
			    HttpUtils.cleanupConnection(uc);
			    throw new DownloadException(e, uc);		
		    }
		}
		
		throw e;
	    }

    	    if(done && dataFile != null && Cache.cleanupThread != null) {
                try {
                    AccessController.doPrivileged(new Cache.CacheIOAction() {
                        public Object run() throws IOException {
                            Cache.cleanupThread.addedFile(dataFile.length() + indexFile.length());
                            return null;
                        }
           
                    });
                }// end try
                catch (PrivilegedActionException pae) {
                    throw new IOException(pae.getMessage());
                }                
	    }
	}
		    
	if(done == true) {	    
	    URL targetURL = (uc != null)?uc.getURL():null;
	    return new CachedFile(dataFile, headerFields, targetURL);
	}
	else
	    return null;
    }

    // Find the cached copy of this jar file, if it exists
    private boolean getCacheFile() throws IOException {
	final CachedFileLoader thisLoader = this;
        try {
            return ((Boolean)AccessController.doPrivileged(new Cache.CacheIOAction() {
                public Object run() throws IOException {
                    boolean found = false;
                    found = FileCache.getMatchingFile(thisLoader);
                    return Boolean.valueOf(found);
                }           
            })).booleanValue();
        }// end try
        catch (PrivilegedActionException pae) {
            throw new IOException(pae.getMessage());
        }        
    }


    // Load the jar file from the cache
    private boolean loadFromCache() {
	boolean retVal = false;
        try {
	    // Check if the cached file is upto date.
	    upToDate = isUpToDate();

            if (upToDate) {
                
                // File is up to date.
                Cache.msgPrintln("cache.loading", new Object[]{ url });
                
		// Check if URL should be metered
		boolean meteredInput = sun.net.ProgressMonitor.getDefault().shouldMeterInput(url, "GET");		    
		if (meteredInput)	
		{
		    // Update fake progress for cached file
		    sun.net.ProgressSource ps = new sun.net.ProgressSource(url, "GET", 10000);
		    ps.beginTracking();
		    ps.updateProgress(10000, 10000);
		    ps.finishTracking();
		    ps.close();  
		}	    
                
                // Touch the file.  Files are deleted from the
                // cache in order of use, and this marks the
                // file as recently used.
		Cache.touch(indexFile);

		retVal = true;
            } else {
		retVal = updateCache();
            }
        } catch (IOException e) {
            // Ignore, we'll return false
            //e.printStackTrace();
            Cache.msgPrintln("cache.load_warning", new Object[] {url});
        }

	return retVal;
    }

    // Checks if the cached copy of the jar file is up to date
    private boolean isUpToDate() throws IOException {
        boolean upToDate = false;

        // Check the expiration date
        if (expiration != 0) {
            if ((new Date()).before(new Date(expiration))) {
                upToDate = true;
            }
        }


        // Check the last modified date
        if (!upToDate && (lastModified != 0)) {
            uc = (HttpURLConnection)url.openConnection();
            uc.setUseCaches(false);
            uc.setAllowUserInteraction(false);
            uc.setIfModifiedSince(lastModified);
	        uc = HttpUtils.followRedirects(uc);
            int response = uc.getResponseCode();
            if (response == HttpURLConnection.HTTP_NOT_MODIFIED) {
                // The file has not been modified
			    HttpUtils.cleanupConnection(uc);  	
                upToDate = true;
            } else if (response >= 200 && response <= 299) {

                long oldModified = lastModified;
                long oldExpiration = expiration;
                lastModified = uc.getLastModified();
                expiration = uc.getExpiration();
                if (lastModified == oldModified) {

                    // There is a common web server Y2K bug where
                    // the server will say a file is modified,
                    // then return the exact same last modified
                    // date.  In this case, treat the file as
                    // unmodified.
					HttpUtils.cleanupConnection(uc);  	
                    upToDate = true;
                } else {
                    // Output the dates in the user's locale
                    // and time zone.
                    DateFormat formatter = 
                        DateFormat.getDateTimeInstance();
                    String cacheDate = 
                        formatter.format(new Date(oldModified));
                    String serverDate =
                        formatter.format(new Date(lastModified));
                    Cache.msgPrintln("cache.out_of_date", new Object[]{ url, cacheDate, serverDate});
                            
                    // Cached jar's out of date, so get the download size.
                    size = uc.getContentLength();
	    	    initializeHeaderFields(uc);
                }
            } else {
                // Unhandled response from server
                Cache.msgPrintln("cache.response_warning", new Object[] {String.valueOf(response), url});
            }
        }
        return upToDate;
    }


    private boolean download() throws IOException {
	boolean retVal = false;
	//open URLConnection with useCache flag set to false
	if(uc == null) {
	    uc = (HttpURLConnection)url.openConnection();
	    uc.setUseCaches(false);
	    uc.setAllowUserInteraction(false);
	    uc = HttpUtils.followRedirects(uc);
	    int response = uc.getResponseCode();
	    if(response == HttpURLConnection.HTTP_OK) {
		size = uc.getContentLength();
		lastModified = uc.getLastModified();
		expiration = uc.getExpiration();
		// Check that this url supports caching
		if ((lastModified == 0) && (expiration == 0)) {
                    Trace.msgPrintln("cache.header_fields_missing");
                    return false;
		}

	        initializeHeaderFields(uc);
	    }
	}


	//create the cache and index file        
        try {
            retVal = ((Boolean)AccessController.doPrivileged(new Cache.CacheIOAction() {
                public Object run() throws IOException {
                    return Boolean.valueOf(createCacheFiles());
                }
            })).booleanValue();
        }// end try
        catch (PrivilegedActionException pae) {
            throw new IOException(pae.getMessage());
        }        

	return retVal;
    }

    private void initializeHeaderFields(URLConnection uc)throws IOException{
	headerFields = new MessageHeader();
	String key = uc.getHeaderFieldKey(0);
	String status = uc.getHeaderField(0);
	if(null == key && null != status)
		headerFields.add(null, status);

	for(int i=0;i<fieldName.length;i++) {
			String value = uc.getHeaderField(fieldName[i]);
			if(value != null) {
				headerFields.add(fieldName[i], value);
	    }
	}
    }

    private boolean updateCache() throws IOException {
        try {
            AccessController.doPrivileged(new Cache.CacheIOAction() {
                public Object run() throws IOException {
                    // Try to delete this file.  This will not 
                    // work if the file is in use, but that's ok
                    // because we marked it as Cache.UNUSABLE above.  
                    // It will get deleted next time cache cleanup
                    // runs and the file is not in use.
                    Cache.removeFromTable(indexFile, FileCache.filesInCache);
                    if(!dataFile.delete()) {
                        dataFile.deleteOnExit();
                    }
                    if(!indexFile.delete()) {
                        //mark the file to be deleted
                        FileOutputStream fos = new FileOutputStream(indexFile);
                        fos.write(Cache.UNUSABLE);
                        fos.close();
                        indexFile.deleteOnExit();
                    }
                    
                    //return value is not used
                    return null;
                }
            });
        }// end try
        catch (PrivilegedActionException pae) {
            throw new IOException(pae.getMessage());
        }        
        
        //download
	try {
	    return  download();
	}catch(IOException e) {
	    // have to make sure it is connection failure
	    if(uc != null) {
		int response = uc.getResponseCode();
		if ((response < 200) || (response >= 300)) {
			HttpUtils.cleanupConnection(uc);
		    throw new DownloadException(e, uc);		
		}
	    }
	    
	    throw e;
	}
    }


    private boolean createCacheFiles() throws IOException {
	String filename = FileCache.generateCacheFileName(url);
	dataFile = new File(FileCache.directory, filename + Cache.getFileExtension(url.toString()));
	indexFile = new File(FileCache.directory, filename + Cache.INDEX_FILE_EXT);
	RandomAccessFile raf = null;

	boolean valid = false;
	InputStream bis = null;
	OutputStream bos = null;
	try {

	    //open the index file
            raf = new RandomAccessFile(indexFile, "rw");
	    //mark the index file as incomplete            
	    raf.writeByte(Cache.INCOMPLETE);

	    // Write the header
            raf.writeUTF(url.toString());
            raf.writeLong(lastModified);
            raf.writeLong(expiration);
	    raf.writeInt(FileType.getType(Cache.getFileExtension(url.toString())));
	    writeHeaders(raf);

	    //Open the necessary streams
	    bis = new BufferedInputStream(uc.getInputStream());
	    bos = new BufferedOutputStream(new FileOutputStream(dataFile));

	    int len = 0, dataLen = 0;
	    // Read/Write until EOF
	    byte[] buf = new byte[2048];
	    while((len = bis.read(buf, 0, buf.length)) != -1) {
		dataLen = dataLen + len;
    		bos.write(buf, 0, len);
	    }

	    //mark the index file as usable
	    raf.seek(0);
	    raf.writeByte(Cache.VERSION);

	    valid = true;
	} finally {
	    // Close the streams
	    if (bis != null) {
		bis.close();
	    }
	    if (bos != null) {
		bos.close();
	    }
	    if (raf != null) {
		raf.close();
	    }
	    if(!valid) {
		dataFile.delete();
		indexFile.delete();
		dataFile = null;
		indexFile = null;			
	    }
	}
	//message to indicate new jar file is created in cache
	Cache.msgPrintln("cache.cached_name", new Object[] {dataFile.getName()});

	return true;
    }

    //Writes the header into the cache index file
    public void writeHeaders(RandomAccessFile raf) throws IOException{
	ByteArrayOutputStream out = new ByteArrayOutputStream();
	DataOutputStream dos = new DataOutputStream(out);

	try {
		// To get size
		Map headers = headerFields.getHeaders();
	    if(!headers.isEmpty()) {
		// Write the list of signers
		dos.writeInt(headers.size());
		// Write all headers, still keep in some kind of order		
		for(int index = 0; index < headers.size(); index ++) {
			String name = headerFields.getKey(index);
			if(null == name)
				name = "<null>";
			dos.writeUTF(name);
		    dos.writeUTF(headerFields.getValue(index));
		}

		dos.flush();
	    }else {
		//Write list of header fields as 0
		dos.writeInt(0);
	    }

        raf.write(out.toByteArray());
	} finally {
	    //do not close the stream since raf could be used
	    //by the calling method
        }
    }

}





