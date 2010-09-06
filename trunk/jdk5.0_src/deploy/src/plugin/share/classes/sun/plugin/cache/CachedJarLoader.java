/*
 * @(#)CachedJarLoader.java	1.33 04/07/18 
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package sun.plugin.cache;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.ByteArrayOutputStream;
import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.RandomAccessFile;
import java.net.URL;
import java.net.URLConnection;
import java.text.DateFormat;
import java.util.List;
import java.util.ArrayList;
import java.util.Date;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Set;
import java.util.StringTokenizer;
import java.util.jar.Attributes;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;
import java.util.jar.Manifest;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;
import java.util.zip.ZipOutputStream;
import java.util.jar.JarOutputStream;
import java.util.Map;
import java.security.CodeSigner;
import java.security.Timestamp;
import java.security.Principal;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.security.cert.CertPath;
import java.security.cert.CertificateFactory;
import java.security.AccessController;
import java.security.PrivilegedActionException;
import java.security.cert.CertificateException;
import sun.misc.JarIndex;
import java.net.HttpURLConnection;
import sun.plugin.net.protocol.http.HttpUtils;
import sun.plugin.util.Trace;
import sun.plugin.resources.ResourceHandler;
import java.util.zip.GZIPInputStream;
import java.util.jar.Pack200;

// Class to handle loading of jar files from the cache
public class CachedJarLoader {
    
    private static final String ACCEPT_ENCODING  	= "accept-encoding";
    private static final String CONTENT_ENCODING 	= "content-encoding";
    private static final String CONTENT_TYPE 	 	= "content-type";
    private static final String PACK200_GZIP_ENCODING   = "pack200-gzip";
    private static final String GZIP_ENCODING           = "gzip";
    private static final String JAR_MIME_TYPE    	= "application/x-java-archive";
    
    private static final int BUFFER_SIZE = 8192;
    
    private URL url;
    private HttpURLConnection uc = null;
    private long lastModified = 0;
    private long expiration = 0;
    
    // To know we are accepting HTTP_COMPRESION
    private boolean httpCompression = true;
    
    // The jar's up-to-date status, file descriptor and size.
    private boolean upToDate = false;
    private boolean upToDateChecked = false;
    private File dataFile = null;
    private File indexFile = null;
    private boolean cached = false;
    private long size = 0;
    private FileVersion version = new FileVersion();

    // Modifications for initialization of the CachedJarLoader.  When
    // determining sizes for the ProgressDialog, call with a getSize of
    // true.  Other other times, call with a getSize of false.
    public CachedJarLoader(URL url, boolean getSize) throws IOException {
        this.url = url;
        
        // Get the file descriptor up front.
        cached = getCacheFile();
        
        // Should only be done during execution of the progress dialog.
        if (getSize) {
            if (cached) {
                upToDate = isUpToDate();
                upToDateChecked = true;
            } else {
                size = Cache.getFileSizeFromServer(url);
            }
        }
    }
            
    // Public query for the jar size.
    public long getJarSize() {
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

    //Set the jarfile version
    public void setVersion(FileVersion ver) {
        version = ver;
    }

    // Load the jar file
    public CachedJarFile load() throws IOException {

        // Try to load the file from the cache
        CachedJarFile jar = loadFromCache();

        // If not successful, download the file
        if (jar == null) {
	    try {
		    Trace.msgPrintln("httpCompression = " + httpCompression);
            jar = download();
	    }catch(IOException e) {
		// have to make sure it is connection failure
		if(uc != null) {
		    int response = uc.getResponseCode();
		    if ((response < 200) || (response >= 300)) {
			throw new DownloadException(e, uc);		
		    }
		}
		// We try w/o HTTP Compression of any sort.
		if (httpCompression == true) {
		    httpCompression = false;
			HttpUtils.cleanupConnection(uc);  	
		    uc = null; // For a good measure.
		    jar = download();
		} else {
		    throw e;
		}
	    }

	    if(jar != null && Cache.cleanupThread != null) {
                try {
                    AccessController.doPrivileged(new Cache.CacheIOAction() {
                        public Object run() throws IOException {
                            Cache.cleanupThread.addedJar(dataFile.length() + indexFile.length());
                            //return value is not used
                            return null;
                        }
                    });
                }// end try
                catch (PrivilegedActionException pae) {
                    throw new IOException(pae.getMessage());
                }
	    }
        }
        
        return jar;
    }

    // Load the jar file from the cache
    private CachedJarFile loadFromCache() {
        CachedJarFile jar = null;
        try {
            // Check if the file is in the cache
            if (cached) {
                
                // Found the file.  Check if it is up to date.
                // If we haven't already processed this jar, we need to
                // determine it's up-to-date status.
                if (!upToDateChecked) {
                    upToDate = isUpToDate();
                }
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
       
                    try {
                        jar = (CachedJarFile)AccessController.doPrivileged(new Cache.CacheIOAction() {
                            public Object run() throws IOException {
                                // Touch the file.  Files are deleted from the
                                // cache in order of use, and this marks the
                                // file as recently used.
                                indexFile.setLastModified(System.currentTimeMillis());
                            
                                // Process the cached authentication
                                return authenticateFromCache();                }
                        });
                    }// end try
                    catch (PrivilegedActionException pae) {
                        throw new IOException(pae.getMessage());
                    }
                } else {
                    // File is out of date
                    try {
                        AccessController.doPrivileged(new Cache.CacheIOAction() {
                            public Object run() throws IOException {
                            // Try to delete this file.  This will not 
                            // work if the file is in use, but that's ok
                            // because we marked it as Cache.UNUSABLE above.  
                            // It will get deleted next time cache cleanup
                            // runs and the file is not in use.
			    Cache.removeFromTable(indexFile, JarCache.filesInCache);
			    if(!dataFile.delete()) {
				dataFile.deleteOnExit();
			    }
			    if(!indexFile.delete()) {
				FileOutputStream fos = new FileOutputStream(indexFile);
				fos.write(Cache.UNUSABLE);
				fos.close();
				indexFile.deleteOnExit();
                            }

                            // Need to null file since it's now a class var.
                            dataFile = null;
			    indexFile = null;
                            
                            return null;         }
                        });
                    }// end try
                    catch (PrivilegedActionException pae) {
                        throw new IOException(pae.getMessage());
                    }
                }
            }
        } catch (IOException e) {
            // Ignore, we'll return null
            e.printStackTrace();
            Cache.msgPrintln("cache.load_warning", new Object[] {url});
        }
        return jar;
    }

    // Find the cached copy of this jar file, if it exists
    private boolean getCacheFile() throws IOException {
	final CachedJarLoader thisLoader = this;
        try {
            return ((Boolean)AccessController.doPrivileged(new Cache.CacheIOAction() {
                public Object run() throws IOException {
                    boolean found = false;
                    found = JarCache.getMatchingFile(thisLoader);
                    return Boolean.valueOf(found);
                }           
            })).booleanValue();
        }// end try
        catch (PrivilegedActionException pae) {
            throw new IOException(pae.getMessage());
        }        

    }

    // Checks if the cached copy of the jarfile's version
    // is latest compared to the version specified in the HTML page
    public boolean isVersionUpToDate(FileVersion inVersion)
    {
	boolean current = false;
	current = version.isUpToDate(inVersion);
	upToDate = current;
	upToDateChecked = true;
	return current;
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
	    
	    if (httpCompression) {
		uc.setRequestProperty(ACCEPT_ENCODING,PACK200_GZIP_ENCODING + "," + GZIP_ENCODING);
		uc.setRequestProperty(CONTENT_TYPE,JAR_MIME_TYPE);
	    }

            uc = HttpUtils.followRedirects(uc);

            int response = uc.getResponseCode();
            if (response == HttpURLConnection.HTTP_NOT_MODIFIED) {

                // The file has not been modified
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
                }
            } else {
                // Unhandled response from server
                Cache.msgPrintln("cache.response_warning", new Object[] {String.valueOf(response), url});
            }
        }
        return upToDate;
    }

    // Download the jar file to the cache
    private CachedJarFile download() throws IOException {
        if (uc == null) {
            // Open the URL if we have not already done so
            uc = (HttpURLConnection)url.openConnection();

			if (httpCompression) {
				uc.setRequestProperty(ACCEPT_ENCODING, PACK200_GZIP_ENCODING + "," + GZIP_ENCODING);
				uc.setRequestProperty(CONTENT_TYPE,JAR_MIME_TYPE);
			}


		    uc.setUseCaches(false);
            uc.setAllowUserInteraction(false);
            uc = HttpUtils.followRedirects(uc);

            int response = uc.getResponseCode();
            if ((response < 200) || (response >= 300)) {
                // Failed to connect
				HttpUtils.cleanupConnection(uc);
                throw new IOException("Could not connect to " + url + " with response code " + response);
            }
            lastModified = uc.getLastModified();
            expiration = uc.getExpiration();
        }

        // Check that this url supports caching
        if ((lastModified == 0) && (expiration == 0)) {
            Trace.msgPrintln("cache.header_fields_missing");
            return null;
        }
        
        // Download the file
        Cache.msgPrintln("cache.downloading", new Object[] {url});
        try {
            return (CachedJarFile)AccessController.doPrivileged(new Cache.CacheIOAction() {
                public Object run() throws IOException {
                    // Find an unused filename for storing the Jar file
                    String filename = JarCache.generateCacheFileName(url);
                    dataFile = new File(JarCache.directory, filename + JarCache.DATA_FILE_EXT);
                    indexFile = new File(JarCache.directory, filename + Cache.INDEX_FILE_EXT);
                
                    //Before downloading the data file, mark the index file
                    //as incomplete so that the data file is not deleted before
                    //it is used.
                    markAsIncomplete();

                    // Decompress the Jar file into the cache
                    decompress();

                    // Authenticate the Jar file
                    CachedJarFile jar = authenticate();

                    //message to indicate new jar file is created in cache
                    Cache.msgPrintln("cache.cached_name", new Object[] {dataFile.getName()});
                
                    return jar;
                }
            });
        }// end try
        catch (PrivilegedActionException pae) {
            throw new IOException(pae.getMessage());
        }        
    }

    // Decompresses the Jar file into the cache
    private void decompress() throws IOException {
		boolean valid = false;
		ZipInputStream jarjarIn = null;
        ZipInputStream in = null;
        JarOutputStream out = null;


        try {
	    // Open the output stream
	    out = new JarOutputStream(
		      new BufferedOutputStream(
		          new FileOutputStream(dataFile)));
	    out.setLevel(JarCache.compression);
	    String encoding = uc.getContentEncoding();
	    Trace.msgPrintln("encoding = " + encoding + " for " + url);
	    // Always test pack-gzip first!.   
	    if (encoding != null && encoding.compareTo(PACK200_GZIP_ENCODING) == 0) {
		InputStream pin = new GZIPInputStream(new BufferedInputStream(uc.getInputStream()));	
		Pack200.Unpacker upkr200 = Pack200.newUnpacker();
		upkr200.unpack(pin, out);
		valid = true;
	    } else if (encoding != null && encoding.indexOf(GZIP_ENCODING) >= 0) {
		in = new ZipInputStream(new GZIPInputStream(new BufferedInputStream(uc.getInputStream()),BUFFER_SIZE));
		decompressWrite(in,out);
		valid = true;
	    } else {
		httpCompression = false; // No http retry!
		in = new ZipInputStream(new BufferedInputStream(uc.getInputStream()));

		ZipEntry entry;
		// Check for .jarjar file
		if (url.toString().toLowerCase().endsWith(JarCache.JARJAR_FILE_EXT)) {
			entry = in.getNextEntry();
		    while (entry != null) {
				if (entry.toString().toLowerCase().startsWith(JarCache.META_FILE_DIR)) {
					//Ignore meta-files inside a .jarjar file
					entry = in.getNextEntry();
				} else if (! entry.toString().toLowerCase().endsWith(JarCache.JAR_FILE_EXT)) {
					//other than .jar files are not allowed in .jarjar 
					throw new IOException(ResourceHandler.getMessage("cache.jarjar.invalid_file"));
				} else {
					//if jar is found break the loop
					break;
				}
		    }
		    
		    jarjarIn = in;						
		    in = new ZipInputStream(in);
		}
		
		decompressWrite(in,out);
		
		//make sure that only one jar file was found in .jarjar file
		if(jarjarIn != null) {
		    entry = jarjarIn.getNextEntry();
		    if(entry != null) {
			String msg = null;
			if (!entry.toString().toLowerCase().endsWith(JarCache.JAR_FILE_EXT)) {
			    msg = ResourceHandler.getMessage("cache.jarjar.invalid_file");
			}
			else {
			    msg = ResourceHandler.getMessage("cache.jarjar.multiple_jar");
			}
			
			throw new IOException(msg);
		    }
		}
		
		valid = true;		
		
	    }


        } finally {
			if(uc != null)
				HttpUtils.cleanupConnection(uc);

            // Close the streams
			if(jarjarIn != null) {
				jarjarIn.close();
		    }
            if (in != null) {
                in.close();
            }
            if (out != null) {
                out.close();
            }
	    if(!valid) {
		dataFile.delete();
		indexFile.delete();			
	    }
        }
    }
    
    // Private helper function to read each zipentry, 
    // decompress and write.
    
    private void decompressWrite(ZipInputStream in, ZipOutputStream out) throws IOException {
	// Decompress each entry
	byte[] buffer = new byte[8192];
	ZipEntry entry = in.getNextEntry();
	while (entry != null) {
	    // It is expensive to create new ZipEntry objects
	    // when compared to cloning the existing entry. 
	    // We need to reset the compression size, since we 
	    // are changing the compression ratio of the entry.  
	    ZipEntry outEntry = (ZipEntry)entry.clone();
	    outEntry.setCompressedSize(-1);
	    out.putNextEntry(outEntry);
	    
	    int n = 0;
	    while((n = in.read(buffer, 0, buffer.length)) != -1) {
		out.write(buffer, 0, n);
	    }
	    
	    out.closeEntry();
	    entry = in.getNextEntry();
	}
	out.flush();
    }
    
    //marks the index file as incomplete
    private void markAsIncomplete() throws IOException {
	RandomAccessFile raf = null;
	try {
	    //mark the file as incomplete
	    raf = new RandomAccessFile(indexFile, "rw");
	    raf.writeByte(Cache.INCOMPLETE);
	} finally {
	    if (raf != null) {
		raf.close();
	    }
	}
    }

    // Authenticates the cached Jar file and saves the results
    private CachedJarFile authenticate() throws IOException {
        // Open the Jar file
	boolean valid = false;
        JarFile jar = new JarFile(dataFile);
        ObjectOutputStream out = null;
        RandomAccessFile raf = null;
        ArrayList signerCerts = new ArrayList(); // Certificates 
        ArrayList signersCS = new ArrayList(); // CodeSigner
        HashMap signerMapCert = new HashMap();
        HashMap signerMapCS = new HashMap();
	Manifest manifest = null;

        try {
            // Write the header
            raf = new RandomAccessFile(indexFile, "rw");
            raf.writeByte(Cache.INCOMPLETE);
            raf.writeUTF(url.toString());
            raf.writeLong(lastModified);
            raf.writeLong(expiration);
            
	    // Add additional headers for Java Plug-in
	    raf.writeInt(FileType.JAR);
	    raf.writeUTF(version.getVersionAsString());

	    // Open the output stream
	    ByteArrayOutputStream bout = new ByteArrayOutputStream();
	    out = new ObjectOutputStream(bout);

            manifest = jar.getManifest();
	    //If no manifest, no need to worry about authentication
	    if(manifest != null) {
		// Jar file authentication
		byte[] buffer = new byte[2048];
		Enumeration entries = jar.entries();

		while (entries.hasMoreElements()) {
		    JarEntry entry = (JarEntry)entries.nextElement();
		    String name = entry.getName();
                
		    // Skip meta-files
		    if (name.toLowerCase().startsWith(JarCache.META_FILE_DIR) &&
			!name.equals(JarIndex.INDEX_NAME)) {
			continue;
		    }
                
		    // Authenticate the entry.  To do so, we must read the
		    // entire entry through the JarVerifier.VeriferStream
		    InputStream in = null;
		    try {
			in = jar.getInputStream(entry);
			while (in.read(buffer, 0, buffer.length) != -1) {
			    // Do nothing
			}
		    } finally {
			if (in != null) {
			    in.close();
			}
		    }
                
		    // Get the signers' certificate obj for this entry
		    Certificate[] certs = entry.getCertificates();

		    if ((certs != null) && (certs.length > 0)) {
			int[] signerIndicesCert = new int[certs.length];
			for (int i = 0; i < certs.length; i++) {

			    // Add the certificate to the list of this Jar file
			    int signerIndexCert = signerCerts.indexOf(certs[i]);
			    if (signerIndexCert == -1){
				signerIndexCert = signerCerts.size();
				signerCerts.add(certs[i]);
			    }

			    // Add the certificate to the list of signers for this entry
			    signerIndicesCert[i] = signerIndexCert;
			}
			signerMapCert.put(name, signerIndicesCert);
		    }
		 
		    // Get the signers' codeSigner object for this entry
		    CodeSigner[] entrySigners = entry.getCodeSigners();

		    if ((entrySigners != null) && (entrySigners.length > 0)) {
			int[] signerIndicesCS = new int[entrySigners.length];
			for (int i = 0; i < entrySigners.length; i++) {

			    // Add the entry signer to the list of signers for this Jar file
			    int signerIndexCS = signersCS.indexOf(entrySigners[i]);
			    if (signerIndexCS == -1){
				signerIndexCS = signersCS.size();
				signersCS.add(entrySigners[i]);
			    }

			    // Add the certificate to the list of signers for this entry
			    signerIndicesCS[i] = signerIndexCS;
			}
			signerMapCS.put(name, signerIndicesCS);
		    }

		    // Now that the signature has been verified for
		    // this entry, remove its attributes from the
		    // manifest.
		    removeSignature(manifest, name);
		}

		// Write the manifest
		ByteArrayOutputStream bos = new ByteArrayOutputStream();
		manifest.write(bos);
		bos.close();
		byte[] manifestBytes = bos.toByteArray();
		out.writeInt(manifestBytes.length);
		out.write(manifestBytes);

		BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(out));

		if(!signerCerts.isEmpty() && !signersCS.isEmpty()) {
		    // Write the list of signers
		    out.writeInt(signerCerts.size());

		    Iterator iterator = signerCerts.iterator();
		    while (iterator.hasNext()) {
			out.writeObject(iterator.next());
		    }
            
		    // Write the map of entry name and signers
		    Iterator keys = signerMapCert.keySet().iterator();
		    String lastPath = null;
		    while (keys.hasNext()) {
			String name = (String)keys.next();
			int[] signerIndicesCert = (int[])signerMapCert.get(name);
			if (name.indexOf("/") != -1) {
			    // Compress path names.  We use a very 
			    // rudimentary but effective scheme.  If the
			    // last entry had the same path, we just store
			    // a "/" at the beginning of the entry name.
			    String path =
				name.substring(0, name.lastIndexOf("/"));
			    if ((lastPath != null) && 
				path.equals(lastPath)) {
				name = name.substring(path.length());
			    }
			    lastPath = path;
			}

			// Write the entry name
			writer.write(name);
			writer.newLine();

			//Write the number of signers, and their names
			String line = String.valueOf(signerIndicesCert.length);
			for (int i = 0; i < signerIndicesCert.length; i++) {
			    line += " " + signerIndicesCert[i];
			}
			writer.write(line, 0, line.length());
			writer.newLine();
		    }
		    // Write one more line to end certificate
		    writer.newLine();

		    // Write codeSigner obj and list of signers
		    Integer csSize = new Integer(signersCS.size());
		    writer.write(csSize.toString());
		    writer.newLine();

		    //flush the data written by writer
		    writer.flush();

		    Iterator iteratorCS = signersCS.iterator();
		    while (iteratorCS.hasNext()) {
			out.writeObject(iteratorCS.next());
		    }
            
		    // Write the map of entry name and signersCS
		    Iterator keysCS = signerMapCS.keySet().iterator();
		    String lastPathCS = null;
		    while (keysCS.hasNext()) {
			String nameCS = (String)keysCS.next();
			int[] signerIndicesCS = (int[])signerMapCS.get(nameCS);
			if (nameCS.indexOf("/") != -1) {
			    String path = nameCS.substring(0, nameCS.lastIndexOf("/"));
			    if ((lastPathCS != null) && 
				path.equals(lastPathCS)) {
				nameCS = nameCS.substring(path.length());
			    }
			    lastPathCS = path;
			}

			writer.write(nameCS);
			writer.newLine();

			// Write the number of signers, and their names
			String lineCS = String.valueOf(signerIndicesCS.length);
			for (int i = 0; i < signerIndicesCS.length; i++) {
			    lineCS += " " + signerIndicesCS[i];
			}
			writer.write(lineCS, 0, lineCS.length());
			writer.newLine();
		    }

		    // flush the data written by writer
		    writer.flush();
		} else {
		    //Write list of signers length as 0
		    out.writeInt(0);
		}
	    } else {
		//Write manifest length as 0
		out.writeInt(0);
	    }

	    //Flush the outer stream to take care of caching
	    //jar files not having manifest
	    out.flush();
	    raf.write(bout.toByteArray());

	    // Mark the file as usable
	    raf.seek(0);
	    raf.writeByte(Cache.VERSION);
	    valid = true;

	} finally {
	    jar.close();
	    if (out != null) {
		out.close();
	    }
	    if (raf != null) {
		raf.close();
	    }
	    if(!valid) {
		dataFile.delete();
		indexFile.delete();			
	    }
	} 

	CodeSigner[] codeSigners = new CodeSigner[signersCS.size()];
	codeSigners = (CodeSigner[])signersCS.toArray(codeSigners);

        return new CachedJarFile(dataFile, codeSigners, signerMapCS, manifest);
    }
    
    // Reads cached authentication data for this Jar file
    private CachedJarFile authenticateFromCache() throws IOException {
        HashMap signerMapCert = new HashMap();
        HashMap signerMap = new HashMap();
	Certificate[] certificates = null;
        CodeSigner[] signers = null;
        Manifest manifest = null;
        RandomAccessFile raf = new RandomAccessFile(indexFile, "r");
	ObjectInputStream in = null;
	ArrayList certNameList = new ArrayList();

        try {
            // Skip header information
            raf.readByte();
            raf.readUTF();
            raf.readLong();
            raf.readLong();
	    raf.readInt();
	    raf.readUTF();
            
            // Create an input stream and reader 
            in = new ObjectInputStream(
            	 new BufferedInputStream(
                 new FileInputStream(raf.getFD())));
	    BufferedReader reader = new BufferedReader(new InputStreamReader(in));
            
            // Read the manifest
            int manifestLength = in.readInt();
	    if (manifestLength > 0) {
		byte[] manifestBytes = new byte[manifestLength];
		in.readFully(manifestBytes, 0, manifestLength);
		ByteArrayInputStream bin = new ByteArrayInputStream(manifestBytes);
		manifest = new Manifest();
		manifest.read(bin);
            
		// Read the certificate array
		int numCerts = in.readInt();

		if (numCerts > 0) {
		   certificates = new Certificate[numCerts];
		   try {
			for (int i = 0; i < numCerts; i++) {
			    certificates[i] = (Certificate)in.readObject();
			}
		   } catch (ClassNotFoundException e) {
			throw new IOException("Error reading signer certificates");
		   }

		   // Read the certificate signer map
		   String certLine = reader.readLine();
		   String lastPackageCert = null;

		   while ((certLine !=null) && (!certLine.equals(""))) {
		    	// Read the entry name
			String certName = certLine;
			if (certName.startsWith("/")) {
			    certName = lastPackageCert + certName;
			} else {
			    int lastSlashCert = certName.lastIndexOf("/");
			    if (lastSlashCert != -1) {
				lastPackageCert = certName.substring(0, lastSlashCert);
			    }
			}
		    	certLine = reader.readLine();

			StringTokenizer tokenizer = new StringTokenizer(certLine, " ", false);
			int numEntryCerts = Integer.parseInt(tokenizer.nextToken());
			int[] signerIndicesCert = new int[numEntryCerts];
			for (int i=0; i< numEntryCerts; i++) {
			    signerIndicesCert[i] = Integer.parseInt(tokenizer.nextToken()); 
			}
			certNameList.add(certName);
			signerMapCert.put(certName, signerIndicesCert);
		        certLine = reader.readLine();
		   }

		   // Read more codeSigner object info if available
		   String numCSStr = reader.readLine();

		   // No codeSigner info, this is from JRE 1.4 or early cache
		   if (numCSStr == null) {
		     Trace.msgPrintln("Reading cached JAR file from JRE 1.4 and early release");

		     try {
		       // Now we have to break certificates chain and create codeSigner object
                       CertificateFactory cf = CertificateFactory.getInstance("X.509");
		       ArrayList cpList = new ArrayList();

		       int chainNum = 0;
		       int start = 0;
		       int end = 0;
		       while (end < certificates.length) {
			 ArrayList certList = new ArrayList();
		     	 int i = start;
			 for (i = start; i < certificates.length; i++) {
			     X509Certificate currentCert = null;
			     X509Certificate issuerCert = null;

			     if (certificates[i] instanceof X509Certificate)
				currentCert = (X509Certificate) certificates[i];

			     if (((i+1)<certificates.length) && certificates[i+1] instanceof X509Certificate)
				issuerCert = (X509Certificate) certificates[i+1];
			     else
				issuerCert = currentCert;

			     certList.add(currentCert);
			     if (!isIssuerOf(currentCert, issuerCert))
				break;
		         }
			 end = (i < certificates.length) ? (i + 1): i;

			 // Create CertPath list
                     	 CertPath cp = cf.generateCertPath(certList);
			 certList.clear();
			 cpList.add(cp);

		     	 start = end;
            	     	 chainNum++;
		       }

		       // Create codeSigner object
		       signers = new CodeSigner[chainNum];
		       int[] csIndices = new int[chainNum];
		       for (int j=0; j<chainNum; j++) {
        	       	   signers[j] = new CodeSigner((CertPath)(cpList.get(j)), (Timestamp)null);
			   csIndices[j] = j; 
		       }

		       for (int k=0; k<certNameList.size(); k++) {
		           signerMap.put(certNameList.get(k), csIndices);
		       }
		     } catch (CertificateException ce) {
			throw new IOException("Error process signer certificates");
		   } 
		 }
		 else { // We do find codeSigner info
		   Trace.msgPrintln("Reading cached JAR file from JRE 1.5 release");

		   int numSigners = Integer.parseInt(numCSStr);
		   if (numSigners > 0) {
		   	signers = new CodeSigner[numSigners];
		     	try {
			    for (int i = 0; i < numSigners; i++) {
		               	signers[i] = (CodeSigner)in.readObject();
			    }
		     	} catch (ClassNotFoundException e) {
			    throw new IOException("Error reading code signer");
			} 
            
			// Read the codeSigner map
			String line = reader.readLine();
			String lastPackage = null;
			while ((line != null) && (!line.equals(""))) {
			   // Read the entry name
			   String name = line;
			   if (name.startsWith("/")) {
			    	name = lastPackage + name;
			   } else {
			 	int lastSlash = name.lastIndexOf("/");
			    	if (lastSlash != -1) {
				   lastPackage = name.substring(0, lastSlash);
			    	}
			   }
			   line = reader.readLine();
			   StringTokenizer tokenizer =
			    	new StringTokenizer(line, " ", false);
			   int numEntrySigners = Integer.parseInt(tokenizer.nextToken());
			   int[] signerIndices = new int[numEntrySigners];
			   for (int i = 0; i < numEntrySigners; i++) {
			    	signerIndices[i] = Integer.parseInt(tokenizer.nextToken());
			   }
			   signerMap.put(name, signerIndices);
			   line = reader.readLine();
		     	}
		   }
		 }

		 Cache.msgPrintln("cache.cert_load", new Object[]{ url });
		} // numCerts>0
		else
		  Trace.msgPrintln("No certificate info, this is unsigned JAR file.");
	    }
	} finally {
	    raf.close();
	    if (in != null) {
		in.close();
	    }
	}

        return new CachedJarFile(dataFile, signers, signerMap, manifest);
    }


    // Removes the signature for the given entry from the manifest
    private void removeSignature(Manifest manifest, String name) {
        Attributes atts = manifest.getAttributes(name);
        if (atts != null) {
	    // Bug - stanleyh 4/29/01
	    // We are modifying the Attributes object
	    // while enumerating it, and it results
	    // in ConcurrentModificationException.
	    // Solution - clone it first before
	    // enumeration.    
	    Set keySet = ((Attributes) atts.clone()).keySet();
            Iterator keys = keySet.iterator();
            while (keys.hasNext()) {
                Attributes.Name keyName =
                    (Attributes.Name)keys.next();
                String key = keyName.toString();
                if (key.endsWith("-Digest") ||
                    (key.indexOf("-Digest-") != -1)) {
                    atts.remove(keyName);
                }
                if (key.equals("Magic:")) {
                    atts.remove(keyName);
                }
            }
            if (atts.isEmpty()) {
                manifest.getEntries().remove(name);
            }
        }
    }

    /*
     * @return true if the issuer of <code>cert1</code> corresponds to the
     * subject (owner) of <code>cert2</code>, false otherwise.
     */
    private static boolean isIssuerOf(X509Certificate cert1,
                                      X509Certificate cert2)
    {
        Principal issuer = cert1.getIssuerDN();
        Principal subject = cert2.getSubjectDN();
        if (issuer.equals(subject))
            return true;
        return false;
    }
}

