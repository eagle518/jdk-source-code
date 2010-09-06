/*
 * @(#)Cache.java	1.100 04/06/11
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.cache;

import java.io.*;
import java.net.*;
import java.text.*;
import java.util.*;
import java.util.jar.*;
import java.util.zip.*;
import sun.net.www.protocol.jar.Handler;
import java.security.AccessControlException;
import java.security.*;
import com.sun.javaws.*;
import com.sun.javaws.jnl.*;
import com.sun.javaws.LocalApplicationProperties;
import com.sun.javaws.exceptions.*;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.config.Config;
import com.sun.deploy.config.JREInfo;

/**
 * Cache
 *
 * A Cache for resources loaded and cached on the
 * local computer. The is mostly a policy object.
 * The nity-gritty of managing the cache is delegated
 * to the various DiskCache objects
 */

/**
 * This was all imported from InstallCache.java, making everything static,
 * and hiding the underlying DiskCache objects so we can implement both
 * a user cache, and a system cache for Java 1.5
 */


public class Cache {
    public static final char RESOURCE_TYPE    = DiskCache.RESOURCE_TYPE;
    public static final char APPLICATION_TYPE = DiskCache.APPLICATION_TYPE;
    public static final char EXTENSION_TYPE   = DiskCache.EXTENSION_TYPE;
    public static final char MUFFIN_TYPE      = DiskCache.MUFFIN_TYPE;
    public static final char MUFFIN_TAG_INDEX = DiskCache.MUFFIN_TAG_INDEX;
    public static final char MUFFIN_MAXSIZE_INDEX =
					DiskCache.MUFFIN_MAXSIZE_INDEX;

    private static DiskCache _activeCache = null;
    private static DiskCache _readOnlyCache = null;
    private static DiskCache _muffincache = null;

    /** File name of the file used to indicated the last time the
     * cache was accessed (that is a file was added/removed).
     */
    private static final String LAST_ACCESSED_FILE_NAME = "lastAccessed";

    /**
     * Extension of indirect files in the cache.
     */
    private static final String INDIRECT_EXTENSION = ".ind";

    /**
     * Maps from path (either URL, or path on file system) to
     * LocalApplicationProperties.
     */
    private static HashMap _loadedProperties;

    /**
     *  Statically initialize everything
     */
    static {
	initialize();
    }

    /**
     *  The various disk cache objects implement the real caches
     */
    private static void initialize() {
        // Setup base directory. Do this during initialization,
	// so this does not change during midrun of an application.
        DiskCache usercache = new DiskCache(getUserBaseDir());
	DiskCache systemcache = null;
	File sysBaseDir = getSysBaseDir();
	if (sysBaseDir != null) {
	    systemcache = new DiskCache(sysBaseDir);
	}
        File muffinbasedir = getMuffinCacheBaseDir();
        _muffincache = new DiskCache(muffinbasedir);
        _loadedProperties = new HashMap();

	if ((systemcache != null) && Globals.isSystemCache()) {
	    _readOnlyCache = null;
	    _activeCache = systemcache;
	} else {
	    _readOnlyCache = systemcache;
	    _activeCache = usercache;
            if (Globals.isSystemCache()) {
		Globals.setSystemCache(false);
		Trace.println("There is no system cache configured, " +
				"\"-system\" option ignored");
	    }
	}
    }

    private final static char[] cacheTypes = {
        DiskCache.DIRECTORY_TYPE,
        DiskCache.TEMP_TYPE,
        DiskCache.VERSION_TYPE,
        DiskCache.INDIRECT_TYPE,
        DiskCache.RESOURCE_TYPE,
        DiskCache.APPLICATION_TYPE,
        DiskCache.EXTENSION_TYPE,
        DiskCache.MUFFIN_TYPE
    };

    public static boolean canWrite() {
        return (_activeCache.canWrite());
    }

    public static void updateCache() {
	String olddir = Config.getProperty(Config.JAVAWS_CACHE_KEY);
	String newdir = Config.getProperty(Config.CACHEDIR_KEY) +
			File.separator + "javaws";
	final File oldFile = new File(olddir);
	final File newFile = new File(newdir);

	// should we keep track of old orphans,
	// or we just leave them as orphans
	Iterator it = _activeCache.getOrphans();
	while (it.hasNext()) {
	    DiskCacheEntry dce = (DiskCacheEntry) it.next();
	}

        it =  _activeCache.getJnlpCacheEntries();
	ArrayList reinstall = new ArrayList();
	LocalInstallHandler lih = LocalInstallHandler.getInstance();
        while (it.hasNext()) {
            DiskCacheEntry dce = (DiskCacheEntry) it.next();
            LaunchDesc ld = null;
            LocalApplicationProperties lap = null;
            try {
                ld = LaunchDescFactory.buildDescriptor(dce.getFile());
            	lap = getLocalApplicationProperties(dce, ld);
		if (lap != null && lap.isLocallyInstalled()) {
		    lih.uninstall(ld, lap, true);
		    reinstall.add(dce);
		}
            } catch (Exception e) {
                Trace.ignoredException(e);
            }
	}

	boolean unsafe = olddir.startsWith(newdir) || newdir.startsWith(olddir);

	if (!unsafe && oldFile.exists() && oldFile.isDirectory()) {
	    copy(oldFile, newFile, new FilenameFilter() {
		public boolean accept(File dir, String name) {
		    if (dir.equals(oldFile) ||
			dir.getParentFile().equals(oldFile)) {
			// everything at top but splash cache
			return !(name.equals("splashes"));
		    }
		    if (name.length() == 0) {
			return false;
		    }
		    char type = name.charAt(0);
		    for (int i=0; i<cacheTypes.length; i++) {
			if (type == cacheTypes[i]) {
			    return true;
			}
		    }
		    return false;
		}
	    });
	}
	Config.setProperty(Config.JAVAWS_CACHE_KEY, null);
	Config.storeIfDirty();
	synchronized(Cache.class) {
	    initialize();
        }

	// restore old shortcuts
	it = reinstall.iterator();
	while (it.hasNext()) {
	    try {
                LaunchDesc ld = null;
                LocalApplicationProperties lap = null;
		DiskCacheEntry dce = (DiskCacheEntry) it.next();
		ld = LaunchDescFactory.buildDescriptor(dce.getFile());
		lap = getLocalApplicationProperties(dce, ld);
		lih.doInstall(ld, lap);
	    } catch (Exception e) {
		Trace.ignoredException(e);
	    }
	}
    }


    private static void copy(File source, File dest, FilenameFilter filter) {
	if (source.isDirectory()) {
	    dest.mkdirs();
	    File[] children = source.listFiles(filter);
	    for (int i=0; i<children.length; i++) {
		copy(children[i], new File(dest.getPath() +
			File.separator + children[i].getName()), filter);
	    }
	} else {
	    // dest.createNewFile();
	    byte b[] = new byte[1024];
            FileOutputStream fos = null;
            FileInputStream fis = null;
	    try {
	        fos = new FileOutputStream(dest);
	        fis = new FileInputStream(source);
	        while (true) {
		    int b_read = fis.read(b);
		    if (b_read == -1) { break; }
		    fos.write(b, 0, b_read);
	        }
	    } catch (Exception e) {
		Trace.ignoredException(e);
	    } finally {
		try {
		    if (fos != null) { fos.close(); }
		} catch (Exception e) {}
		try {
		    if (fis != null) { fis.close(); }
		} catch (Exception e) {}
	    }
	}
    }

    public static void saveRemovedApp(URL href, String title) {
	Properties p = getRemovedApps();
	p.setProperty(href.toString(), title);
	setRemovedApps(p);
    }

    public static void setRemovedApps(Properties props) {
	try {
	    FileOutputStream fos = new FileOutputStream(getRemovePath());
	    props.store(fos, "Removed JNLP Applications");
	} catch (IOException ioe) { }
    }

    public static Properties getRemovedApps() {
	Properties props = new Properties();
	try {
	    InputStream is = new FileInputStream(getRemovePath());
	    props.load(is);
	} catch (IOException ioe) { }
	return props;
    }

    public static String getRemovePath() {
	return Config.getJavawsCacheDir() + File.separator + "removed.apps";
    }

    private static File getMuffinCacheBaseDir() {
        String base = Config.getJavawsCacheDir() + File.separator + "muffins";
        File muffinDir = new File(base);
        if (!muffinDir.exists()) {
            muffinDir.mkdirs();
        }
	Trace.println("Muffin Cache = " + muffinDir, TraceLevel.CACHE);
        return muffinDir ;
    }

    private static File getUserBaseDir() {
        String base = Config.getJavawsCacheDir();
        File userDir = new File(base);
        if (!userDir.exists()) {
            userDir.mkdirs();
        }
	Trace.println("User cache dir = " + userDir, TraceLevel.CACHE);
        return userDir ;
    }

    private static File getSysBaseDir() {
        String base = Config.getSystemCacheDirectory();
	if ((base == null) || (base.length() == 0)) {
	    return null;
	}
        File sysDir = new File(base + File.separator +"javaws");
        if (!sysDir.exists()) {
            sysDir.mkdirs();
        }
	Trace.println("System cache dir = " + sysDir, TraceLevel.CACHE);
        return sysDir ;
    }

    /**
     * Removes all the related entry for all applications installed in the cache
     */
    public static void remove() {
	Iterator it =  _activeCache.getJnlpCacheEntries();
	while (it.hasNext()) {
	    DiskCacheEntry dce = (DiskCacheEntry) it.next();
	    LaunchDesc ld = null;
 	    try {
                ld = LaunchDescFactory.buildDescriptor(dce.getFile());
            } catch (Exception e) {
                Trace.ignoredException(e);
            }
            if (ld != null) {
		LocalApplicationProperties lap =
			getLocalApplicationProperties(dce, ld);
	        remove(dce, lap, ld);
	    }
	}
        uninstallActiveCache();
        uninstallMuffinCache();
    }

    public static void remove(String path, LocalApplicationProperties lap,
				LaunchDesc ld) {
	try {
	    DiskCacheEntry dce = getCacheEntryFromFile(new File(path));
	    remove (dce, lap, ld);
	} catch (Exception e) {
	    Trace.ignoredException(e);
	}
    }

    /**
     * Removes all the related entry for <code>ld</code> from the cache.
     */
    public static void remove(DiskCacheEntry dce,
	LocalApplicationProperties lap, LaunchDesc ld) {
        InformationDesc id = ld.getInformation();
	LocalInstallHandler lih = LocalInstallHandler.getInstance();

	// save removed app href (for undelete)
	if (ld.isApplicationDescriptor() && (ld.getLocation() != null)) {
	    saveRemovedApp(ld.getLocation(), id.getTitle());
	}


	// first uninstall shortcuts or installers
	lap.refresh();
	if (lap.isLocallyInstalled()) {
	    if (ld.isApplicationDescriptor()) {
		if (lih != null) {
	            lih.uninstall(ld, lap, true);
		}
	    } else if (ld.isInstaller()) {
	        ArrayList list = new ArrayList();
	        list.add(dce.getFile());
	        try {
		    String path = lap.getInstallDirectory();
		    Launcher.executeUninstallers(list);
		    JREInfo.removeJREsIn(path);
		    deleteFile(new File(path));
	        } catch (ExitException ex ) {
		    Trace.ignoredException(ex);
	        }
	    }
	}

	// next remove Add/Remove entries in registry:
        String title = ld.getInformation().getTitle();
        Config.getInstance().addRemoveProgramsRemove(title);

	// remove association
	lih.removeAssociations(ld, lap);

	// remove custom splash if present
	SplashScreen.removeCustomSplash(ld);

        // next Remove images and mapped images.
        if (id != null) {
            IconDesc[] icons = id.getIcons();
            if (icons != null) {
                for (int i = 0; i < icons.length; i++) {
                    URL url = icons[i].getLocation();
                    String version = icons[i].getVersion();
                    removeEntries(RESOURCE_TYPE, url, version);
                }
            }
	    RContentDesc[] rc = id.getRelatedContent();
	    if (rc != null) for (int i=0; i<rc.length; i++) {
		URL url = rc[i].getIcon();
		if (url != null) {
		    removeEntries(RESOURCE_TYPE, url, null);
		}
	    }

        }
/**  Resources
 * 	(1.5) - leave resources in cache as orphans ...
 *      ResourcesDesc cbd = ld.getResources();
 *      if (cbd != null) {
 *          JARDesc[] cps = cbd.getEagerOrAllJarDescs(true);
 *          if (cps != null) {
 *              for (int counter = cps.length - 1; counter >= 0; counter--) {
 *                  URL location = cps[counter].getLocation();
 *                  String version = cps[counter].getVersion();
 *                  removeEntries(RESOURCE_TYPE, location, version);
 *              }
 *          }
 *      }
*/
	URL canonical = ld.getCanonicalHome();

        if (canonical != null) {
            removeEntries(APPLICATION_TYPE, canonical, null);
            removeEntries(EXTENSION_TYPE, canonical, null);
        }
	if (dce != null) {
	    removeEntry(dce);
	}
    }

    private static void deleteFile(File f) {
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


    /** Removes all entries matching a given url and Version String */
    private static void removeEntries(char type, URL location, String version) {
        if (location == null) return;
        try {
            DiskCacheEntry[] dces = getCacheEntries(
                type,
                location,
                version,
                true);
            for(int i = 0; i < dces.length; i++) {
                removeEntry(dces[i]);
            }

        } catch (IOException ioe) {
            Trace.ignoredException(ioe);
        }
    }


    /**
     * Returns the location of a JNLP file given a jnlpUrl
     */
    public static File getCachedLaunchedFile(URL jnlpUrl) throws IOException {
        DiskCacheEntry dce = getCacheEntry( APPLICATION_TYPE, jnlpUrl, null);
        return (dce == null) ? null : dce.getFile();
    }

    public static File getCachedFile(URL url) {
	File f = null;
	if (url.getProtocol().equals("jar")) {
	    String path = url.getPath();
	    int bang = path.indexOf("!/");
	    if (bang > 0) try {
		String entry = path.substring(bang+2);
		URL lib = new URL(path.substring(0,bang));
		// NOTE: if version based resource won't work ...
		File dir = createNativeLibDir(lib, null);
		return new File(dir, entry);
	    } catch (MalformedURLException mue) {
		Trace.ignoredException(mue);
	    } catch (IOException ioe) {
		Trace.ignoredException(ioe);
	    }
	    return null;
	} else if (url.toString().endsWith(".jnlp")) {
	    try {
		f = getCachedLaunchedFile(url);
	    } catch (IOException ioe) {
		Trace.ignoredException(ioe);
	    }
	}
	return f;
    }

    /**
     * Returns the LocalApplciationProperties for the launch descriptor
     * loaded from the diskCacheEntry
     */
    public static LocalApplicationProperties getLocalApplicationProperties(
				     DiskCacheEntry dce, LaunchDesc ld) {
        return getLocalApplicationProperties(dce.getLocation(),
		dce.getVersionId(), ld, dce.getType() == APPLICATION_TYPE);
    }

    /**
     * Returns the LocalApplciationProperties for the launch descriptor
     * loaded from the file pointed into the cache
     */
    public static LocalApplicationProperties getLocalApplicationProperties(
					     String path, LaunchDesc ld) {
        DiskCacheEntry dce = getCacheEntryFromFile(new File(path));
	if (dce == null) {
	    return null;
	}
        return getLocalApplicationProperties(dce.getLocation(),
		dce.getVersionId(), ld, dce.getType() == APPLICATION_TYPE);
    }

    /**
     * Returns the LocalApplicationProperties for the launch descriptor loaded
     * from <code>jnlpUrl</code>. Unless home is null, this will always return
     * a non-null value.
     */
    public static LocalApplicationProperties getLocalApplicationProperties(
					     URL jnlpUrl, LaunchDesc ld) {
        return getLocalApplicationProperties(jnlpUrl, null, ld, true);
    }

    /**
     * Returns the LocalApplicationProperties for the launch descriptor loaded
     * from <code>jnlpUrl</code>. Unless home is null, this will always return
     * a non-null value.
     */
    public static LocalApplicationProperties getLocalApplicationProperties(
				URL jnlpUrl, String versionId,
				LaunchDesc ld, boolean isApplicationDesc) {
        if (jnlpUrl == null) {
            return null;
        }

        // Generating the hashcode for a URL requires a nameserver lookup,
        // i.e., we need to be online. This should fix the problem.
        String key = jnlpUrl.toString().intern() + "?" + versionId;

        LocalApplicationProperties props;
        synchronized(_loadedProperties) {
            props = (LocalApplicationProperties)_loadedProperties.get(key);
            if (props == null) {
		props = new DefaultLocalApplicationProperties(
                                jnlpUrl, versionId, ld, isApplicationDesc);
                _loadedProperties.put(key, props);
            } else {
		props.refreshIfNecessary();
	    }
        }
        return props;
    }

    public static LaunchDesc getLaunchDesc(URL jnlpUrl, String versionId) {
        try {
            DiskCacheEntry dce = getCacheEntry(
		APPLICATION_TYPE, jnlpUrl, versionId);
            if (dce != null) {
                try {
                    return LaunchDescFactory.buildDescriptor(dce.getFile());
                } catch(Exception e) {
                    return null;
                }
            }
        } catch(IOException ioe) {
            // Not critical
            Trace.ignoredException(ioe);
        }
        return null;
    }

    /** Returns a new directory where to install an extension */
    public static String getNewExtensionInstallDirectory() throws IOException {
        String dir = getUserBaseDir().getAbsolutePath() +
	    				File.separator + "ext";
        String tempname = null;
        int count = 0;
        do {
            tempname =  dir + File.separator + "E" +
		(new Date().getTime()) + File.separator;
            // Create path
            File cacheDir = new File(tempname);
            if (!cacheDir.mkdirs()) {
                tempname = null;
            }
            Thread.yield(); // Just to improve scheduling.
        } while(tempname == null && ++count < 50); // max 50 attempts
        if (tempname == null) {
            throw new IOException("Unable to create temp. dir for extension");
        }
        return tempname;
    }

    /**
     * Returns a file in the cache that is unique, or null if there is
     * an error in creating one.
     */
    private static String createUniqueIndirectFile() throws IOException {
        String dir = getUserBaseDir().getAbsolutePath() + File.separator +
            "indirect";
        File parent = new File(dir);

        parent.mkdirs();
        File file = File.createTempFile("indirect", INDIRECT_EXTENSION,
                                        parent);
        return file.getAbsolutePath();
    }

    /**
     * Direct indirection into the appropriate cache:
     *
     * ALL access to the cache must go through one of the methods below
     *
     */

    public static void removeEntry(DiskCacheEntry dce) {
	_activeCache.removeEntry(dce);
    }

    public static long getLastAccessed(boolean bSystem) {
	if (bSystem) {
	    if (_readOnlyCache == null) { return 0; }
	    return _readOnlyCache.getLastUpdate();
	} else {
	    return _activeCache.getLastUpdate();
	}
    }
    public static long getLastAccessed() {
        return _activeCache.getLastUpdate();
    }

    public static void setLastAccessed() {
	_activeCache.recordLastUpdate();
    }

    public static String[] getBaseDirsForHost(URL url) {
	String [] dirs;
	if (_readOnlyCache == null) {
	    dirs = new String[1];
	    dirs[0] = _activeCache.getBaseDirForHost(url);
	} else {
	    dirs = new String[2];
	    dirs[0] = _readOnlyCache.getBaseDirForHost(url);
	    dirs[0] = _activeCache.getBaseDirForHost(url);
	}
	return dirs;
    }

    public static long getCacheSize() throws IOException {
	return _activeCache.getCacheSize();
    }

    public static void clean() {
	_activeCache.cleanResources();
    }

    public static long getOrphanSize(boolean bSystem) {
	if (bSystem) {
	    if (_readOnlyCache == null) { return 0; }
	    return _readOnlyCache.getOrphanSize();
	} else {
	    return _activeCache.getOrphanSize();
	}
    }

    public static void cleanResources() {
	_activeCache.cleanResources();
    }

    public static long getCacheSize(boolean bSystem) {
	try {
	    if (bSystem) {
	        if (_readOnlyCache == null) { return -1; }
	        return _readOnlyCache.getCacheSize();
	    } else {
	        return _activeCache.getCacheSize();
	    }
	} catch (Exception e) {
	    Trace.ignoredException(e);
	}
	return 0;
    }

    public static DiskCacheEntry[] getCacheEntries(char type, URL url,
		String version, boolean exact) throws IOException {
	DiskCacheEntry[] active;
	active = _activeCache.getCacheEntries(type, url, version, exact);
	DiskCacheEntry[] readOnly = new DiskCacheEntry[0];
	if (_readOnlyCache != null) {
	    readOnly =
		_readOnlyCache.getCacheEntries(type, url, version, exact);
	}
	if (readOnly.length == 0) {
	    return active;
	}
	int len = readOnly.length + active.length;

	DiskCacheEntry[] ret = new DiskCacheEntry[len];
	int i=0;
	for (i=0; i<readOnly.length; i++) {
	    ret[i] = readOnly[i];
	}
	for (int j=0; j<active.length; j++) {
	   ret[i++] = active[j];
	}
	return ret;
    }

    public static DiskCacheEntry getMuffinCacheEntryFromFile(File f) {
	return  _muffincache.getCacheEntryFromFile(f);
    }

    public static DiskCacheEntry getCacheEntryFromFile(File f) {
	DiskCacheEntry active = _activeCache.getCacheEntryFromFile(f);
	if (_readOnlyCache != null) {
	    DiskCacheEntry readOnly = _readOnlyCache.getCacheEntryFromFile(f);
	    if ((readOnly != null) && readOnly.newerThan(active)) {
		return readOnly;
	    }
	}
	return active;
    }

    public static File getTempCacheFile(URL url, String versionId)
	throws IOException {
	return _activeCache.getTempCacheFile(url, versionId);
    }

    public static DiskCacheEntry getCacheEntry(char type, URL url,
	String versionId) throws IOException {
        DiskCacheEntry active =
	    _activeCache.getCacheEntry(type, url, versionId);
        if (_readOnlyCache != null) {
            DiskCacheEntry readOnly =
		_readOnlyCache.getCacheEntry(type, url, versionId);
            if ((readOnly != null) && readOnly.newerThan(active)) {
                return readOnly;
            }
        }
        return active;
    }

    public static File createNativeLibDir(URL url, String versionId)
	throws IOException {
	return _activeCache.createNativeLibDir(url, versionId);
    }

    public static Iterator getJnlpCacheEntries(boolean readOnly) {
	if (readOnly) {
	    if (_readOnlyCache == null) {
		return (new ArrayList().iterator());
	    }
	    return _readOnlyCache.getJnlpCacheEntries();
	} else {
            return _activeCache.getJnlpCacheEntries();
	}
    }

    public static File putMappedImage(URL url, String versionId, File tempFile)
        throws IOException {
	return _activeCache.putMappedImage(url, versionId, tempFile);
    }

    public static byte[] getLapData(char type, URL url, String versionId,
				    boolean readOnly) throws IOException  {
	if (readOnly) {
	    return (_readOnlyCache == null) ? null :
	    	    _readOnlyCache.getLapData(type, url, versionId);
	}
	return  _activeCache.getLapData(type, url, versionId);
    }

    public static void putLapData(char type, URL url, String versionId,
	byte[] data) throws IOException  {
	_activeCache.putLapData(type, url, versionId, data);
    }

    public static void insertEntry(char type, URL url, String versionId, File f,
                            long timestamp) throws IOException {
	_activeCache.insertEntry(type, url, versionId, f, timestamp);
    }

    public static void putCanonicalLaunchDesc(URL ref, LaunchDesc ld)
	throws IOException {
	// only Applications and Applets ...
	// extensions can have versions, and exist due to being referenced
	if (ld.isApplicationDescriptor()) {
	    File f = getTempCacheFile(ref, null);
	    FileOutputStream fos = new FileOutputStream(f);
	    try {
	        fos.write(ld.getSource().getBytes());
	    } finally {
	        fos.close();
	    }
	    insertEntry(APPLICATION_TYPE, ref, null, f, new Date().getTime());
	}
    }

    public static void uninstallActiveCache() {
	_activeCache.uninstallCache();
    }

    /**
     * indirection to MuffinCache
     *
     * ALL access to the muffin cache must go through one of the methods below
     *
     */

    public static long getMuffinSize(URL url) throws IOException {
	return _muffincache.getMuffinSize(url);
    }

    public static long [] getMuffinAttributes(URL url) throws IOException {
	return _muffincache.getMuffinAttributes(url);
    }

    public static void putMuffinAttributes(URL url, int tag, long maxsize)
	throws IOException {
	_muffincache.putMuffinAttributes(url, tag, maxsize);
    }

    public static URL [] getAccessibleMuffins(URL url) throws IOException {
	return _muffincache.getAccessibleMuffins(url);
    }

    public static void insertMuffinEntry(URL url, File f, int tag,
	long maxsize) throws IOException {
	_muffincache.insertMuffinEntry(url, f, tag, maxsize);
    }

    public static File getMuffinFileForURL(URL url) {
	return _muffincache.getMuffinFileForURL(url);
    }

    public static DiskCacheEntry getMuffinEntry(char type, URL url)
	throws IOException {
	return _muffincache.getMuffinEntry(type, url);
    }

    public static boolean isMainMuffinFile(File f) throws IOException {
	return _muffincache.isMainMuffinFile(f);
    }

    public static void removeMuffinEntry(DiskCacheEntry dce) {
	_muffincache.removeMuffinEntry(dce);
    }

    public static void uninstallMuffinCache() {
	_muffincache.uninstallCache();
    }

}


