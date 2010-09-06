/*
 * @(#)DiskCache.java	1.68 04/02/02
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.cache;

import java.io.*;
import java.net.URL;
import java.net.MalformedURLException;
import java.util.Date;
import java.util.StringTokenizer;
import java.util.NoSuchElementException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import java.util.Comparator;
import com.sun.javaws.util.VersionString;
import com.sun.javaws.util.VersionID;
import com.sun.javaws.util.URLUtil;
import com.sun.javaws.jnl.*;
import javax.jnlp.FileContents;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;

/**
 * The DiskCache is a low-level implementation of the DiskCache
 * It contains only function - no policy.
 *
 * It allows entries to be looked up based on a (Type, Tag, URL, Version)
 * tuple.
 *
 * This is a colission-free cache. Thus, it will never loose information
 * unless it is explicitly removed.
 *
 * @version 1.34, 03/25/01
 *
 */
public class DiskCache {
    static private final int    BUF_SIZE  = 32*1024;

    /** Main type of entries */
    final static char DIRECTORY_TYPE    = 'D';  // Used internally
    final static char TEMP_TYPE         = 'X';  // Used internally
    final static char VERSION_TYPE      = 'V';  // Used internally
    final static char INDIRECT_TYPE     = 'I';  // Used internally

    // Main JNLP types for downloaded resources
    final static char RESOURCE_TYPE     = 'R';  // JAR/CLASS/IMAGE
    final static char APPLICATION_TYPE  = 'A';  // Application-Desc
    final static char EXTENSION_TYPE    = 'E';  // Extension-Desc
					// (the P is for PersistenceService)
    final static char MUFFIN_TYPE       = 'P';  // Muffins!

    /** Location of DiskCache */
    private File _baseDir;

    /*  Attributes. The following defines attributes that the diskcache knows about
     *  They are automatically read and put in a DiskCacheEntry when we read from
     *  the cache. They are not public visible, since they are accessed through the
     *  DiskCacheEntry element
     */
    final static char MAIN_FILE_TAG         = 'M';  // The main resource (Jar, image or muffin)

    final static char NATIVELIB_FILE_TAG    = 'N';  // A directory where a native lib jar gets expanded into
    final static char TIMESTAMP_FILE_TAG    = 'T';  // The timestamp file
    final static char CERTIFICATE_FILE_TAG  = 'C';  // A certificate stored with the resource
    // Used to keep track of desktop integration
    final static char LAP_FILE_TAG          = 'L'; // LocalApplicationProperties
    final static char MAPPED_IMAGE_FILE_TAG = 'B'; // Translated images (such as bmp)
    final static char MUFFIN_ATTR_FILE_TAG   = 'U'; // running out of letters - U is for mUffin

    final static int MUFFIN_TAG_INDEX = 0;
    final static int MUFFIN_MAXSIZE_INDEX = 1;

    /** File used to keep track of last update */
    private static final String LAST_ACCESS_FILE = "lastAccessed";

    private static final String ORPHAN_LIST_FILE = "orphans";

    /** Creates a new DiskCache based at the given directory */
    public DiskCache(File baseDir) {
	_baseDir = baseDir;
    }

    //
    // Keeps track of latest update
    //

    /** Returns the time the cache was last accessed */
    long getLastUpdate() {
	File f = new File(_baseDir, LAST_ACCESS_FILE);
	return f.lastModified();
    }


    /** Private method to update the last accessed time stamp */
    void recordLastUpdate() {
	File f = new File(_baseDir, LAST_ACCESS_FILE);
	try {
	    OutputStream os = new FileOutputStream(f);
	    os.write((int) '.');
	    os.close();
	} catch(IOException io) { /* ignore */ }
    }

    boolean canWrite() {
        boolean writable = _baseDir.canWrite();

        if (writable == false) {
            Trace.println("Cannot write to cache: " + _baseDir.getAbsolutePath(),
                          TraceLevel.BASIC);
        }

        return (writable);
    }

    /** Returns that directory that contains all files from a specific host */
    String getBaseDirForHost(URL url) {
	try {
	    URL baseUrl = new URL(url.getProtocol(), url.getHost(), url.getPort(), "");
	    String file = keyToFileLocation(RESOURCE_TYPE, MAIN_FILE_TAG, baseUrl, null);
	    int idx = file.lastIndexOf(File.separator);
	    return file.substring(0, idx);
	} catch(MalformedURLException mue) {
	    Trace.ignoredException(mue);
	}
	return null;
    }

    private void removeEmptyDirs(URL url) {
	String base = getBaseDirForHost(url);
	if (base != null) {
	    removeEmptyDirs(new File(base));
	}
    }

    private void removeEmptyDirs(File f) {
	if (f.isDirectory()) {
            File[] children = f.listFiles();
	    boolean hasChildren = false;
            if (children != null) {
                for (int i = 0; i < children.length; i++) {
                    removeEmptyDirs(children[i]);
		    if (children[i].exists()) {
			hasChildren = true;
		    }
                }
            }
	    if (!hasChildren) try {
		f.delete();
	    } catch (Exception e) {
		Trace.ignoredException(e);
	    }
        }
    }

    private File getOrphanFileForHost(URL url) {
	try {
	    return new File(getBaseDirForHost(url), ORPHAN_LIST_FILE);
	} catch (Exception e) {
	    Trace.ignoredException(e);
	}
	return null;
    }

    private void removeOrphans(URL url) {
        File orphans = getOrphanFileForHost(url);
        if (orphans != null && orphans.exists()) {
            BufferedReader br = null;
            PrintStream ps = null;
            boolean didChange = false;
            ArrayList files = new ArrayList();
            try {
                InputStream is = new FileInputStream(orphans);
                br = new BufferedReader(new InputStreamReader(is));
		String filename;
                while ((filename = br.readLine()) != null) {
                    files.add(filename);
                }
                for (int i=(files.size()-1); i>=0; i--) {
                    File f = new File((String) files.get(i));
                    f.delete();
                    if (!f.exists()) {
                        didChange = true;
                        files.remove(i);
                    }
                }
            } catch (IOException ioe) {
                Trace.ignoredException(ioe);
            } finally {
                if (br != null) try {
                    br.close();
                } catch(IOException ioe2) {
                    Trace.ignoredException(ioe2);
                }
            }
            if (didChange) try {
		if (files.isEmpty()) {
		    Trace.println("emptying orphans file", TraceLevel.CACHE);
		    orphans.delete();
                } else {
                    ps = new PrintStream(new FileOutputStream(orphans));
                    for (int i=0; i<files.size(); i++) {

			Trace.println("Remaining orphan: "+files.get(i), TraceLevel.CACHE);
                        ps.println((String) files.get(i));
                    }
               }
            } catch (Exception e) {
                Trace.ignoredException(e);
            } finally {
                if (ps != null) {
                    ps.close();
                }
            }
        }
    }

    private void addOrphan(URL url, File f) {

	Trace.println("addOrphan: "+f, TraceLevel.CACHE);
        File orphans = getOrphanFileForHost(url);
	PrintStream ps = null;
	if (orphans != null) try {
            ps = new PrintStream(new FileOutputStream(orphans.getPath(), true));
	    ps.println(f.getCanonicalPath());
        } catch (Exception e) {
            Trace.ignoredException(e);
	} finally {
	   if (ps != null) { ps.close(); }
	}
    }


    //
    //  Cache upadte methods
    //

    /** Returns a temp. filename into the cache. The caller is responsible
     *  for renaming it into a real name using the addToCache(...) method,
     *  or deleting it.
     */
    File getTempCacheFile(URL url, String versionId) throws IOException {
	String base = keyToFileLocation(TEMP_TYPE, MAIN_FILE_TAG, url, versionId);
	File cacheFile = null;
	int count = 0;
	do {
	    String tempname = base + (new Date().getTime());
	    // Create path
	    cacheFile = new File(tempname);
	    cacheFile.getParentFile().mkdirs();
	    try {
		cacheFile.createNewFile();
	    } catch(IOException ioe) {
		cacheFile = null;
	    }
	    Thread.yield(); // Just to improve scheduling.
	} while(cacheFile == null && ++count < 50); // max 50 attempts
	if (cacheFile == null) {
	    throw new IOException("Unable to create temp. file for: " + url);
	}
	return cacheFile;
    }

    /** Create a directory where native libs can be extracted to */
    File createNativeLibDir(URL url, String versionId) throws IOException {
	File cacheFile = getFileFromCache(RESOURCE_TYPE, NATIVELIB_FILE_TAG,
					  url, versionId, false);
	cacheFile.mkdirs();
	return cacheFile;
    }
    File getNativeLibDir(URL url, String versionId) throws IOException {
	File cacheFile = getFileFromCache(RESOURCE_TYPE, NATIVELIB_FILE_TAG,
					  url, versionId, false);
	return cacheFile;
    }

    void insertMuffinEntry(URL url, File f, int tag, long maxsize) throws IOException {
        File newFile = getFileFromCache(MUFFIN_TYPE, MAIN_FILE_TAG, url, null, false);
        if (newFile.exists()) {
                f.delete();
                throw new IOException(
                        "insert failed in cache: target already exixts");
        }
	// make sure path to newFile exist before rename
	File newFileParent = newFile.getParentFile();
	if (newFileParent != null) {
	    newFileParent.mkdirs();
	}
        if (!f.renameTo(newFile)) {
             throw new IOException("rename failed in cache");
	}
        // Create tag
        putMuffinAttributes(url, tag, maxsize);
    }

    long getMuffinSize(URL url) throws IOException {
        // For now, not counting attribute file size as part of muffin size
        long size = 0;
        File muffinFile = getFileFromCache(MUFFIN_TYPE, MAIN_FILE_TAG, url, null, true);

        if (muffinFile != null && muffinFile.exists()) {
            size += muffinFile.length();
        }
        return size;
    }


    /**
     *  Renames a temp. file into the cache to a real file. This must be
     *  called with the same URL as used when getTempCacheFile was called
     */
    void insertEntry(char type, URL url, String versionId, File f,
			    long timestamp) throws IOException {
	// Create timestamp
	putTimeStamp(type, url, versionId, timestamp);

	// Put the file in the cache
	putFileInCache(type, MAIN_FILE_TAG, url, versionId, f);

	// Update cache update timestamp
	recordLastUpdate();
    }

    /**
     *  Insert a mapped image for a given resource. The mapped image is
     *  stored in a temp. file that we rename.
     */
    File putMappedImage(URL url, String versionId, File tempFile)
			       throws IOException {

        if (tempFile.getPath().endsWith(".ico")) {
	    String path = url.getFile();
	    if (!path.endsWith(".ico")) {
		path = path + ".ico";
		url = new URL(url.getProtocol(), url.getHost(),
			      url.getPort(), path);
	    }
	}
	File newFile = putFileInCache(RESOURCE_TYPE, MAPPED_IMAGE_FILE_TAG,
				      url, versionId, tempFile);

        // Update cache update timestamp
        recordLastUpdate();
        return newFile;
    }

    /**
     *  Get a mapped image for a given resource from cache
     *  since we have special case above, we need to undo here
     */
    File getMappedImage(char type, char tag, URL url,
		String versionId, boolean onlyIf) throws IOException {

	File file = getFileFromCache(type, tag, url, versionId, onlyIf);
	if (file == null || !file.exists()) {
	    String path = url.getFile();
	    if (!path.endsWith(".ico")) {
		path = path + ".ico";
		url = new URL(url.getProtocol(), url.getHost(), url.getPort(), path);
	        file = getFileFromCache(type, tag, url, versionId, onlyIf);
	    }
	}
	return file;
    }

    /** Insert an Application or Extension Descriptor. Type should be either a
     *  APPLICATION_TYPE or EXTENSION_TYPE
     */
    void putLaunchFile(char type, URL url, String versionId, String content) throws IOException {
	byte[] bytes = content.getBytes("UTF8"); // Store content in UTF8
	storeAtomic(type, MAIN_FILE_TAG, url, versionId, bytes);
	putTimeStamp(type, url, versionId, new Date().getTime());
    }

    /** Reads an Application or Extension Descriptor. Type should be either a
     *  APPLICATION_TYPE or EXTENSION_TYPE
     */
    String getLaunchFile(char type, URL url, String versionId, String content) throws IOException {
	byte[] data = getEntryContent(type, MAIN_FILE_TAG, url, versionId);
	if (data == null) return null;
	return new String(data, "UTF8");
    }


    //
    // Medium-level storing methods
    //

    void putMuffinAttributes(URL url, int tag, long maxsize) throws IOException {
        PrintStream ps = new PrintStream(getOutputStream(MUFFIN_TYPE, MUFFIN_ATTR_FILE_TAG, url, null));
        try {
            ps.println(tag);
            ps.println(maxsize);
        } finally {
            if (ps != null) ps.close();
        }
    }


    /** Writes a timestamp for a given URL and version */
    void putTimeStamp(char type, URL url, String versionId, long timestamp) throws IOException {
	if (timestamp == 0) timestamp = new Date().getTime();
	PrintStream ps = new PrintStream(getOutputStream(type, TIMESTAMP_FILE_TAG, url, versionId));
	try {
	    ps.println(timestamp); // Output timestamp
	    ps.println("# " + new Date(timestamp)); // Human-readable version (just to aid debugging)
	} finally {
	    ps.close();
	}
    }

    long [] getMuffinAttributes(URL url) throws IOException {
        BufferedReader br = null;
        long tag = -1;
        long maxsize = -1;
        try {
            InputStream is = getInputStream(MUFFIN_TYPE, MUFFIN_ATTR_FILE_TAG, url, null);
            br = new BufferedReader(new InputStreamReader(is));
            String line = br.readLine();
            try {
                tag = (long) Integer.parseInt(line);
            } catch (NumberFormatException nfe) {
                throw new IOException(nfe.getMessage());
            }
            line = br.readLine();
            try {
                maxsize = Long.parseLong(line);
            } catch (NumberFormatException nfe) {
                throw new IOException(nfe.getMessage());
            }
        } finally {
            if (br != null) br.close();
        }
        return new long [] {tag, maxsize};
    }


    /** Reads a timestamp given a path into the cache. A timestamp defaults to 0 if file not found. */
    long getTimeStamp(char type, URL url, String versionId) {
	BufferedReader br  = null;
	try {
	    InputStream is = getInputStream(type, TIMESTAMP_FILE_TAG,  url, versionId);
	    br = new BufferedReader(new InputStreamReader(is));
	    String line = br.readLine();
	    try {
		return Long.parseLong(line);
	    } catch(NumberFormatException nfe) {
		return 0;
	    }
	} catch(IOException ioe) {
	    return 0;
	} finally {
	    try {
		if (br != null) {
		    br.close();
		}
	    } catch(IOException ioe2) {
		Trace.ignoredException(ioe2);
	    }
	}
    }

    final private static String BEGIN_CERT_MARK = "-----BEGIN CERTIFICATE-----";
    final private static String END_CERT_MARK = "-----END CERTIFICATE-----";

    DiskCacheEntry getMuffinEntry(char type, URL url) throws IOException {
        File file = getFileFromCache(type, MAIN_FILE_TAG, url, null, true);
        if (file == null) {
	    return null;
	}
        File tagfile = getFileFromCache(type, MUFFIN_ATTR_FILE_TAG, url, null, true);
        return new DiskCacheEntry(type, url, null, file, -1, null, null, tagfile);
    }


    /**
     *  Returns a DiskCache entry for an entry in the cache. This might
     *  return null if the entry is not in the cache
     */
    DiskCacheEntry getCacheEntry(char type, URL url,
					String versionId) throws IOException {

	File file = getFileFromCache(type, MAIN_FILE_TAG, url, versionId, true);
	if (file == null) {
	    return null;
	}

	File nativeDirFile = getFileFromCache(type, NATIVELIB_FILE_TAG,
					      url, versionId, true);

	File mappedBitmapFile = getMappedImage(type, MAPPED_IMAGE_FILE_TAG,
					      url, versionId, true);


	long timestamp = getTimeStamp(type, url, versionId);

	DiskCacheEntry dce = new DiskCacheEntry(type,
						url,
						versionId,
						file,
						timestamp,
						nativeDirFile,
						mappedBitmapFile,
						null);
	return dce;
    }

    /** Returns set of resources that matches a given VersionString. If the version == null, then
     *  the no-version is returned.
     */
    DiskCacheEntry[] getCacheEntries(char type, URL url, String version,
	    boolean exact) throws IOException {
	// Check for empty version string
	if (version == null) {
	    DiskCacheEntry dce = getCacheEntry(type, url, null);
	    if (dce == null) {
		return new DiskCacheEntry[] { /* empty */ };
	    } else {
		return new DiskCacheEntry[] { dce };
	    }
	}

	// Get a list of all entries in the cache sorted (high versions first)
	ArrayList list = getCacheEntries(type, url);

	VersionString vs = new VersionString(version);
	// Prune list :
	// if exact remove all elements that does not match the given version
	// if not exact we want to keep only the highest thats not higher than
        // any of the version ids in the version string.
        DiskCacheEntry highestFound = null;
        Iterator itr = list.iterator();
        while(itr.hasNext()) {
            DiskCacheEntry dce =  (DiskCacheEntry)itr.next();
            String versionId = dce.getVersionId();
	    // 4474875: non - null dosn't match null ...
	    if (versionId == null) {
                itr.remove();
	    } else if (!vs.contains(versionId)) {
                if (highestFound == null && vs.containsGreaterThan(versionId)) {
                    highestFound = dce;
                }
                itr.remove();
            }
        }
        if (!exact && list.size() == 0 && highestFound != null) {
            list.add(highestFound);
        }

	DiskCacheEntry[] result = new DiskCacheEntry[list.size()];
	return ((DiskCacheEntry[])list.toArray(result));
    }

    void removeMuffinEntry(DiskCacheEntry dce) {
        char type = dce.getType();
        URL url = dce.getLocation();
        String versionId = dce.getVersionId();
        deleteEntry(type, MAIN_FILE_TAG, url, versionId);
        deleteEntry(type, MUFFIN_ATTR_FILE_TAG, url, versionId);
    }

    /** Removes an entry from the cache */
    void removeEntry(DiskCacheEntry dce) {
	char type = dce.getType();
	URL url = dce.getLocation();
	// clean up any orphans
	removeOrphans(url);
	String versionId = dce.getVersionId();
	deleteEntry(type, MAIN_FILE_TAG, url, versionId);
	deleteEntry(type, TIMESTAMP_FILE_TAG, url, versionId);
	deleteEntry(type, CERTIFICATE_FILE_TAG, url, versionId);
	deleteEntry(type, NATIVELIB_FILE_TAG, url, versionId);
	deleteEntry(type, MAPPED_IMAGE_FILE_TAG, url, versionId);
	deleteEntry(type, LAP_FILE_TAG, url, versionId);

	// may also need to delete indirect (whatever it's pointing
	// to will either be gone or orphaned by the above)
	if (type == RESOURCE_TYPE) {
	    deleteEntry(INDIRECT_TYPE, MAIN_FILE_TAG, url, versionId);
	}
	removeEmptyDirs(url);
	recordLastUpdate();
    }

    private void deleteEntry(char type, char tag, URL url, String versionId){
        File file = null;
	try {
	    if (tag == MAPPED_IMAGE_FILE_TAG) {
		file = getMappedImage(type, tag, url, versionId, false);
	    } else {
	        file = getFileFromCache(type, tag, url, versionId, false);
	    }
	    deleteFile(file);
	} catch (IOException ioe) {
	    Trace.ignoredException(ioe);
	}
	if (file != null && file.exists()) {
	    // if fail to delete a main resourse - add it to orphans
	    if (type == RESOURCE_TYPE && tag == MAIN_FILE_TAG) {
		addOrphan(url, file);
	    }
	}
    }

    DiskCacheEntry getCacheEntryFromFile(File f) {
	DiskCacheEntry dce = fileToEntry(f);
	if (dce != null) {
	    try {
 		if (dce.getType() == MUFFIN_TYPE) {
            	    return getMuffinEntry(dce.getType(), dce.getLocation());
        	} else {
            	    return getCacheEntry(dce.getType(), dce.getLocation(),
				 dce.getVersionId());
		}
	    } catch (IOException ioe) {
		Trace.ignoredException(ioe);
	    }
	}
	return dce;
    }

    boolean isMainMuffinFile(File f) throws IOException {
        DiskCacheEntry dce = fileToEntry(f);
	return f.equals(getFileFromCache(MUFFIN_TYPE, MAIN_FILE_TAG,
                dce.getLocation(), null, false));
    }

    /** Returns a set of DiskCache entries for a given URL. The returned array will
     *  contain all version's of the given URL resource
     */
    private ArrayList getCacheEntries(char type, URL url) throws IOException{
	ArrayList list = new ArrayList();

	// Iterate through all versions of this URL. The layout of the
	// cache is as follows:
	//
	//   <protocol>/<host>/<port>/<version>/<path>
	//
	// The <protocol>/<host>/<port> and the <path> components are fixed.
	// We need to loop over the <version> part.
	String generalPath = keyToFileLocation(type, MAIN_FILE_TAG, url, "MATCH");
	// Find location of Version part.
	// Note, given the way the directory is created, the below search can only
	// match on the version part.
	int idx = generalPath.indexOf(File.separator + VERSION_TYPE + "MATCH" + File.separator);
	if (idx == -1) {
	    throw new IllegalStateException("the javaws cache is corrupted");
	}
	String hostStr = generalPath.substring(0, idx);
	File urlBaseDir = new File(hostStr);
	File[] dirList = urlBaseDir.listFiles();
	if (dirList == null) return list;
	for(int i = 0; i < dirList.length; i++) {
	    String filename = dirList[i].getName();
	    if (filename.length() > 0 && filename.charAt(0) == VERSION_TYPE) {
		String versionId = filename.substring(1);
		File file = getFileFromCache(type, MAIN_FILE_TAG,
					     url, versionId, true);
		if (file != null) {
		    DiskCacheEntry dce = getCacheEntry(type, url, versionId);
		    list.add(dce);
		}
	    }
	}
	// if we have more than 1 version based entries then sort them ...
	int length = list.size();
	if (length > 1) {
            DiskCacheEntry[] dces = new DiskCacheEntry[length];
            dces = (DiskCacheEntry[])list.toArray(dces);

	    Arrays.sort(dces, new Comparator() {
			public int compare(Object o1, Object o2) {
			    DiskCacheEntry dce1 = (DiskCacheEntry)o1;
			    DiskCacheEntry dce2 = (DiskCacheEntry)o2;
                            VersionID v1 = new VersionID(dce1.getVersionId());
                            VersionID v2 = new VersionID(dce2.getVersionId());
                            return (v1.isGreaterThan(v2)) ? -1 : 1;
			}
	    });
	    // put array back sorted into ArrayList
	    for (int i=0; i<length; i++) {
		list.set(i, dces[i]);
	    }
	}
	// Add the no version type if it exists
	File file = getFileFromCache(type, MAIN_FILE_TAG, url, null, true);
	if (file != null) {
	    DiskCacheEntry dce = getCacheEntry(type, url, null);
	    list.add(dce);
	}

	return list;
    }

    /** Visitor class used by the visitDiskCache */
    public static interface DiskCacheVisitor {
	public void visitEntry(DiskCacheEntry dce);
    }

    /** Iterates over the cache and returns all the cache entries of a given type */
    void visitDiskCache(char type, DiskCacheVisitor dcv) throws IOException {
	visitDiskCacheHelper(_baseDir, 0, type, dcv);
    }

    private void visitDiskCacheHelper(File f, int level, char type, DiskCacheVisitor dvc) throws IOException  {
	String name = f.getName();
	// Itereate through all directories, except the ones for native libs
	if (f.isDirectory() && (name.length() <= 2 ||
				f.getName().charAt(1) != NATIVELIB_FILE_TAG)) {
	    File[] children = f.listFiles();
	    for (int i = 0; i < children.length; i++) {
		visitDiskCacheHelper(children[i], level + 1, type, dvc);
	    }
	} else {
	    // Level greater than 3, starts the real entries.
	    if (name.length() > 2 && level > 3) {
		char thisType = name.charAt(0);
		char tag  = name.charAt(1);
		if (thisType == type && tag == MAIN_FILE_TAG) {
		    DiskCacheEntry dce = getCacheEntryFromFile(f);
		    if (dce == null) {
			throw new IllegalStateException(
			    "the javaws cache is corrupted");
		    }
		    dvc.visitEntry(dce);
		}
	    }
	}
    }

    private static class MuffinAccessVisitor implements DiskCacheVisitor {
        private DiskCache _diskCache;
        private URL _theURL;
        private URL [] _urls = new URL[255];
        private int _counter = 0;

        MuffinAccessVisitor(DiskCache diskCache, URL url) {
            _diskCache = diskCache;
            _theURL = url;
        }

        public void visitEntry(DiskCacheEntry dce) {
            URL loc = dce.getLocation();
            if (loc == null) return;
            if (loc.getHost().equals(_theURL.getHost())) {
                _urls[_counter++] = loc;
            }
        }

        public URL [] getAccessibleMuffins() {
            return _urls;
        }
    }

    File getMuffinFileForURL(URL url) {
      try {
        return getFileFromCache(MUFFIN_TYPE, MAIN_FILE_TAG, url, null, false);
      } catch (IOException e) {
	return null;
      }
    }

    URL [] getAccessibleMuffins(URL url) throws IOException {
        MuffinAccessVisitor mav = new MuffinAccessVisitor(this, url);
        visitDiskCache(MUFFIN_TYPE, mav);
        return mav.getAccessibleMuffins();
    }

    /** Helper class for the delete visitor */
    private static class DeleteVisitor implements DiskCacheVisitor {
	private DiskCache _diskCache;

	DeleteVisitor(DiskCache diskCache) { _diskCache = diskCache; }

	public void visitEntry(DiskCacheEntry dce) {
	    _diskCache.removeEntry(dce);
	}
    }

    /** Helper class for the delete visitor */
    private static class SizeVisitor implements DiskCacheVisitor {
	private DiskCache _diskCache;
	long _size = 0;

	SizeVisitor(DiskCache diskCache) {
	    _diskCache = diskCache;
	    _size = 0;
	}

	public void visitEntry(DiskCacheEntry dce) {
	    if (dce.getDirectory() != null && dce.getDirectory().exists()) {
		// Count size of directory
		File[] files = dce.getDirectory().listFiles();
		for(int i = 0; i < files.length; i++) {
		    _size += files[i].length();
		}
	    } else {
		// Otherwise, size of main file
		_size += dce.getFile().length();
	    }
	}

	public long getSize() { return _size; };
    }

    /** Clear cache */
    long getCacheSize() throws IOException {

	Trace.println("Computing diskcache size: " + _baseDir.getAbsoluteFile(), TraceLevel.CACHE);

	SizeVisitor sv = new SizeVisitor(this);
	visitDiskCache(RESOURCE_TYPE, sv);
	return sv.getSize();
    }

    /** Removes all directories used by the cache. This should only be calle when Java Web
     *  Start gets uninstalled. All shortcuts etc. should have probably been removed before
     *  this is called too
     */
    void uninstallCache() {
	deleteFile(_baseDir);
        // if it still exists, it must have a "LastAccessed" file. or it will
        // be an invalid cache - restore it and set time to now. (bug #4775777);
        if (_baseDir.exists()) {
            recordLastUpdate();
        }
    }

    /* Deal with error conditions (permissions in shared cache?)
     * Won't work if another javaws is using a jar file from the cache on Windows
     */
    private void deleteFile(File f) {
	// Descend and delete:
	if (f.isDirectory()) {
	    File[] children = f.listFiles();
	    if (children != null) {
		for (int i = 0; i < children.length; i++) {
		    deleteFile(children[i]);
		}
	    }
	}
	f.delete();
    }

    /*
     * Low-level methods for going from (type, tag, url, version-id) -> Filename
     * Storing information and reading information
     */

    /** Returns an output stream for the given (type, url, version) entry.
     */
    private OutputStream getOutputStream(char type, char tag, URL url,
					 String versionId) throws IOException {
	// create path
	File cacheFile = getFileFromCache(type, tag, url, versionId, false);

	cacheFile.getParentFile().mkdirs();
	cacheFile.createNewFile();
	// Create timestamp
	recordLastUpdate();
	// Return output stream
	return new FileOutputStream(cacheFile);
    }

    /**
     *  Returns an inputStream for a given entry
     */
    private InputStream getInputStream(char type, char tag, URL url,
				      String versionId) throws IOException  {
	// Return input stream
	return new FileInputStream(getFileFromCache(type, tag, url,
						    versionId, false));
    }

    byte[] getLapData(char type, URL url, String versionId)
	throws IOException  {
	return getEntryContent(type, LAP_FILE_TAG, url, versionId);
    }

    void putLapData(char type, URL url, String versionId, byte[] data)
	throws IOException  {
	storeAtomic(type, LAP_FILE_TAG, url, versionId, data);
    }

    /** Returns an inputStream for a given entry
     */
    private byte[] getEntryContent(char type, char tag, URL url,
				  String versionId) throws IOException  {
	File f = getFileFromCache(type, tag, url, versionId, true);
	if (f == null) { return null; };
	// Return data
	long size = f.length();
	if (size > 1024 * 1024 * 1024) return null; // This should not really happen
	BufferedInputStream is = new BufferedInputStream(new FileInputStream(f));
	ByteArrayOutputStream baos = new ByteArrayOutputStream((int)size);

	byte[] buffer = new byte[BUF_SIZE];
	try {
	    int n = is.read(buffer);
	    while(n >= 0) {
		baos.write(buffer, 0, n);
		n = is.read(buffer);
	    }
	} finally {
	    baos.close();
	    is.close();
	}

	return baos.toByteArray();
    }

    /** Stores an entry into the cache atomicly.
     */
    private void storeAtomic(char type, char tag, URL url, String versionId,
	byte[] data) throws IOException  {
	// First create a temp. file to store the contents into
	File tempName = getTempCacheFile(url, versionId);
	InputStream is = new ByteArrayInputStream(data);
	BufferedOutputStream bof = new BufferedOutputStream(new FileOutputStream(tempName));
	byte[] buffer = new byte[BUF_SIZE];
	try {
	    int n = is.read(buffer);
	    while(n >= 0) {
		bof.write(buffer, 0, n);
		n = is.read(buffer);
	    }
	} finally {
	    bof.close();
	    is.close();
	}

	// Put the file in the cache
	putFileInCache(type, tag, url, versionId, tempName);
    }

    private File putFileInCache(char type, char tag, URL url, String versionId,
		 File tempFile) throws IOException {

	// Create real cache name
	File realFile = new File(keyToFileLocation(type, tag, url, versionId));

	// need to remove orphans first so we don't replace orphan with real file
	removeOrphans(url);

	// delete any old file
	realFile.delete();

	// Move atomically into cache
	if (!tempFile.renameTo(realFile)) {
	    // cannot rename - deleteEntry will add to orphan list if necessary
	    deleteEntry(type, tag, url, versionId);
	    // and if main resourse - use indirect file
	    if (type == RESOURCE_TYPE && tag == MAIN_FILE_TAG) {
		PrintStream ps = new PrintStream(getOutputStream(
				INDIRECT_TYPE, MAIN_FILE_TAG, url, versionId));
		try {
		    ps.println(tempFile.getCanonicalPath());
		} finally {
		    ps.close();
		}
		return tempFile;
	    }
	    throw new IOException("rename failed in cache to: "+realFile);
	}
	if (type == RESOURCE_TYPE && tag == MAIN_FILE_TAG) {
	    // if there is an indirect file, we must delete it ,
	    // and the temp file it points to (or orphan it)
            File f = getFileFromCache(INDIRECT_TYPE, tag, url,
                                      versionId, false);
	    if (f.exists()) {
		// note - this will delete (or orphan) the file pointed
		//        to by the indirect file.
		deleteEntry(type, tag, url, versionId);
		//      - and this will delete the indirect file itself
	        deleteEntry(INDIRECT_TYPE, tag, url, versionId);
	    }
	}
	return realFile;
    }

    File getFileFromCache(char type, char tag, URL url,
		 String versionId, boolean onlyIfExists) throws IOException{
	File cacheFile;
        BufferedReader br = null;

	// first see if there is an indirect file for a MAIN RESOURCE
	if (type == RESOURCE_TYPE && tag == MAIN_FILE_TAG) {
	    File f = getFileFromCache(INDIRECT_TYPE, tag, url,
				      versionId, false);
	    if (f.exists()) try {
	        InputStream is = getInputStream(
				INDIRECT_TYPE, MAIN_FILE_TAG, url, versionId);
		br = new BufferedReader(new InputStreamReader(is));
	        String filename = br.readLine();
		cacheFile = new File(filename);
		return cacheFile;
	    } catch (IOException ioe) {
		// Indirect file may be empty, as in when a file is attempted
		// to be deleted, but cannot because it is open (Windows).
		// In such a case, if wanted only existing file, return null
		// otherwise fall thru to return the original resource.
		if (onlyIfExists) {
		    return null;
		}
	    } finally {
		if (br != null) {
		    br.close();
		}
	    }
	}
	cacheFile = new File(keyToFileLocation(type, tag, url, versionId));
	if (onlyIfExists) {
	    if (!cacheFile.exists()) {
		return null;
	    }
	}
	return cacheFile;
    }

    /** Computes the URL, version-id, and type based on the File
     */
    private DiskCacheEntry fileToEntry(File file) {

	char type = 0;
	URL url = null;
	String versionId = null;
	long timestamp = 0;

	String path = file.getAbsolutePath();
	// Make sure it points into cache
	String cacheDir =  _baseDir.getAbsolutePath();
	if (!path.startsWith(cacheDir)) return null;

	// Strip off cache directory
	path = path.substring(cacheDir.length());

	StringTokenizer st = new StringTokenizer(path, File.separator, false);

	// Get version
	try {
	    // Get protocol
	    String protocol = st.nextToken();

	    // Get Host
	    String host = st.nextToken();
	    if (host.length() < 1) return null;
	    host = host.substring(1);

	    // Get Port
	    String portStr = st.nextToken();
	    if (portStr.length() < 1) return null;
	    int port = 0;
	    try {
		port = Integer.parseInt(portStr.substring(1));
		if (port == 80) { port = -1; }
	    } catch(NumberFormatException nfe) {
		return null;
	    }

	    // Get path
	    StringBuffer urlPath = new StringBuffer();
	    while(st.hasMoreElements()) {
		String dir = st.nextToken();
		dir = removeEscapes(dir);
		if (dir.length() < 1) return null;
		type = dir.charAt(0);
		if (type == VERSION_TYPE) {
		    versionId = dir.substring(1); // Remove 'V' suffix
		} else {
		    urlPath.append('/');
		    urlPath.append(dir.substring(2));
		}
	    }
	    url = new URL(protocol, host, port, urlPath.toString());
	} catch(MalformedURLException mue) {
	    return null;
	} catch(NoSuchElementException nsee) {
	    // Required part was missing
	    return null;
	}

	DiskCacheEntry dce = new DiskCacheEntry(type, url, versionId, file, 0);

	return dce;
    }

    // Converts all &c -> : and && -> &
    private static String removeEscapes(String str) {
	if (str == null || str.indexOf('&') == -1) return str;

	StringBuffer sb = new StringBuffer(str.length());
	int i = 0;
	while(i < str.length() - 1) {
	    char ch = str.charAt(i);
            char ch2 = str.charAt(i + 1);
	    if (ch == '&' && ch2 == 'p') {
		// skip both these and put in '%'
	        i++;
	        sb.append('%');
	    } else if (ch == '&' && ch2 == 'c') {
		// skip both these and put in ':'
	        i++;
	        sb.append(':');
            } else if (ch == '&' && ch2 == '&') {
		// skip one of these
	    } else {
		sb.append(ch);
	    }
	    i++;
	}
	if ( i < str.length() ) {
	    sb.append(str.charAt(i)); // Append last char (unless ends with &c)
	}
	return sb.toString();
    }

    /** Converts a (type, url, versionId) identifier to a unique file URL */
    private String keyToFileLocation(char type, char tag, URL url, String versionId) {
	StringBuffer path = new StringBuffer(url.toString().length() + ((versionId == null) ? 0 : versionId.length()) * 2);

	// Add protocol
	path.append(url.getProtocol());
	path.append(File.separatorChar);

	// Add Host
	path.append(DIRECTORY_TYPE);
	path.append(url.getHost());
	path.append(File.separatorChar);

	// Add port
	String port = null;
	if (url.getPort() == -1 && url.getProtocol().equals("http")) {
	    // Default port for http
	    port = "P80";
	} else {
	    port = "P" + (new Integer(url.getPort()).toString());
	}
	path.append(port);
	path.append(File.separatorChar);

	// First entry into the cache is the version identifier.
	// If version, is null, it is the goes into a special 'no version'
	// category. A "V" is prepended to the version identifier, in order
	// to avid colision with the NOVERS key.
	if (versionId != null) {
	    path.append(VERSION_TYPE);
	    path.append(versionId);
	    path.append(File.separatorChar);
	}

	// Add file
	path.append(convertURLfile(type, tag, url.getFile()));

	return  _baseDir.getAbsolutePath() + File.separator + path.toString();
    }


    /**
     * Returns the URL path converted into a path appropriate for the local file system.
     * Specifically this maps / (the directory separator char for URIs) to
     * the local file system separator character plus the DIRECTORY_TYPE character.
     *
     * This should not be passed a URL pointing to a directory
     */
    private String convertURLfile(char type, char tag, String urlpath) {
	int coln, ques;
	String params = null;
	if ((coln = urlpath.indexOf(";")) != -1) {
	    params = urlpath.substring(coln);
	    urlpath = urlpath.substring(0, coln);
	}
	if ((ques = urlpath.indexOf("?")) != -1) {
	    params = urlpath.substring(ques) + params;
	    urlpath = urlpath.substring(0, ques);
	}

	if (params != null) {
	    Trace.println("     URL: "+urlpath+"\n  PARAMS: " +
					params, TraceLevel.CACHE);
	}

	// Not all file systems use / as the directory separator, which is
	// what URIs use.
	StringBuffer filepath = new StringBuffer(urlpath.length() * 2);

	// 1. Convert all '/' to '/D' or '\D' depending on platform
	// 2. Convert all & -> &&
	// 3. Convert all : -> &c
	// 4. Convert all % -> &p
	int lastEntry = -1;
	for(int i = 0; i < urlpath.length(); i++) {
	    if (urlpath.charAt(i) == '/') {
		filepath.append(File.separatorChar);
		filepath.append((char)DIRECTORY_TYPE);
		filepath.append((char)MAIN_FILE_TAG);
		lastEntry = filepath.length();
	    } else if (urlpath.charAt(i) == ':') {
		filepath.append("&c");
	    } else if (urlpath.charAt(i) == '&') {
		filepath.append("&&");
	    } else if (urlpath.charAt(i) == '%') {
		filepath.append("&p");
	    } else {
		filepath.append(urlpath.charAt(i));
	    }
	}

	// Convert the last entry to the specific type (since it is not
	// a directory)
	if (lastEntry == -1) {
	    // No directories
	    filepath.insert(0, type);
	    filepath.insert(1, tag);
	} else {
	    filepath.setCharAt(lastEntry - 2, type);
	    filepath.setCharAt(lastEntry - 1, tag);
	}
	return filepath.toString();
    }

    long getOrphanSize() {
	long size = 0;
	try {
            Iterator orphans = getOrphans();
	    while (orphans.hasNext()) {
	        size += ((DiskCacheEntry) orphans.next()).getSize();
	    }
	} catch (Exception e) {
	    Trace.ignoredException(e);
	}
	return size;
    }

    void cleanResources() {
	try {
            Iterator orphans = getOrphans();
	    while (orphans.hasNext()) {
	        removeEntry((DiskCacheEntry) orphans.next());
	    }
	} catch (Exception e) {
	    Trace.ignoredException(e);
	}
    }

    /**
     * Returns an iterator of the Orphaned DiskCacheEntries in the cache.
     */
    Iterator getOrphans() {
        final ArrayList appResources = new ArrayList();
        final ArrayList orphanResources = new ArrayList();
        DiskCache.DiskCacheVisitor dcv = new DiskCache.DiskCacheVisitor() {
            public void visitEntry(DiskCacheEntry dce) {
		LaunchDesc ld = null;
                try {
                    ld = LaunchDescFactory.buildDescriptor(dce.getFile());
                } catch (Exception e) {
                    Trace.ignoredException(e);
                }
		if (ld != null) {
		    ResourcesDesc rds = ld.getResources();
		    JARDesc[] jds = rds.getEagerOrAllJarDescs(true);
            	    if (jds != null) {
                        for (int i=0; i<jds.length; i++ ) {
			    try {
			        File f = getFileFromCache(RESOURCE_TYPE,
				    MAIN_FILE_TAG, jds[i].getLocation(),
				    jds[i].getVersion(), false);
			        if (f != null) {
				    appResources.add(f);
			        }
			    } catch (IOException ioe) {
			    }
                        }
                    }
		    InformationDesc id = ld.getInformation();
       		    if (id != null) {
                        IconDesc[] icons = id.getIcons();
                        if (icons != null)  {
			    for (int i = 0; i < icons.length; i++) {
			        try {
			            File f = getFileFromCache(RESOURCE_TYPE,
				        MAIN_FILE_TAG, icons[i].getLocation(),
				        icons[i].getVersion(), false);
			            if (f != null) {
				        appResources.add(f);
			            }
			        } catch (IOException ioe) {
                                }
			    }
                        }
		    }
                }

            }
        };
	try {
            visitDiskCache(APPLICATION_TYPE, dcv);
            visitDiskCache(EXTENSION_TYPE, dcv);

            dcv = new DiskCache.DiskCacheVisitor() {
                public void visitEntry(DiskCacheEntry dce) {
		    if (!appResources.contains(dce.getFile())) {
                        orphanResources.add(dce);
		    }
                }
            };
            visitDiskCache(RESOURCE_TYPE, dcv);
            visitDiskCache(INDIRECT_TYPE, dcv);
	} catch (IOException ioe) {
	    Trace.ignoredException(ioe);
	}
        return orphanResources.iterator();
    }


    /**
     * Returns an array of the Applications, Applets, Librarys, and Installers
     * in the cache. The elements of the array are of type DiskCacheEntry.
     */
    Iterator getJnlpCacheEntries() {
        final ArrayList al = new ArrayList();
        try {
            DiskCache.DiskCacheVisitor dcv = new DiskCache.DiskCacheVisitor() {
                public void visitEntry(DiskCacheEntry dce) {
                    al.add(dce);
                }
            };
            visitDiskCache(APPLICATION_TYPE, dcv);
            visitDiskCache(EXTENSION_TYPE, dcv);
        } catch (IOException ioe) {
            // Just ignore this - not critical
            Trace.ignoredException(ioe);
        }
        return al.iterator();
    }
}

