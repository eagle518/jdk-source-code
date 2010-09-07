/*
 * @(#)CacheUpdateHelper.java	1.11 10/03/24
 *
 * Copyright (c) 2005, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws;

import java.net.URL;
import java.io.*;
import java.util.*;
import javax.swing.SwingUtilities;
import java.text.DateFormat;
import com.sun.deploy.Environment;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.URLUtil;
import com.sun.deploy.config.Config;
import com.sun.deploy.cache.Cache;
import com.sun.deploy.cache.LocalApplicationProperties;
import com.sun.deploy.net.DownloadEngine;
import com.sun.deploy.ui.ProgressDialog;
import com.sun.deploy.ui.CacheUpdateProgressDialog;
import com.sun.javaws.jnl.*;

public class CacheUpdateHelper {

    /*
     * these are from the old cache structure:
     */
    private final static char DIRECTORY_TYPE	= 'D';
    private final static char TEMP_TYPE		= 'X';
    private final static char VERSION_TYPE	= 'V';
    private final static char INDIRECT_TYPE	= 'I';
    private final static char RESOURCE_TYPE	= 'R';
    private final static char APPLICATION_TYPE	= 'A';
    private final static char EXTENSION_TYPE	= 'E';
    private final static char MUFFIN_TYPE	= 'P';

    private final static char MAIN_FILE_TAG	    = 'M';  
    private final static char NATIVELIB_FILE_TAG    = 'N';  
    private final static char TIMESTAMP_FILE_TAG    = 'T';  
    private final static char CERTIFICATE_FILE_TAG  = 'C';  
    private final static char LAP_FILE_TAG	    = 'L'; 
    private final static char MAPPED_IMAGE_FILE_TAG = 'B'; 
    private final static char MUFFIN_ATTR_FILE_TAG  = 'U'; 

    private final static String MUFFIN_PREFIX = "PM";
    private final static String MUFFIN_ATTRIBUTE_PREFIX = "PU";
    private final static String APP_PREFIX = "AM";
    private final static String EXT_PREFIX = "XM";
    private final static String LAP_PREFIX = "AL";
    private final static String JAR_PREFIX = "RM";
    private final static String NATIVE_PREFIX = "RN";
    private final static String DIR_PREFIX = "DM";

    /**
     * UpdateCache()
     *
     * This is called once when version is upgraded to 6.0
     * it will search for an older javaws cache (first look for 1.5.0 cache, 
     * then look for a 1.4.2 cache, then finally a 1.2 or 1.0.1 cache.
     * need to move to new cache structure:
     * 1) the generated splash screens
     * 2) the cached muffins
     * 3) the list of removed apps
     * 4) the cached resources
     *  
     */
    public static boolean updateCache() {
        String oldPath = Config.getOldJavawsCacheDir();
        File oldCache = new File(oldPath);
        File newCache = Cache.getCacheDir();

        try {
            if (oldCache.exists() && oldCache.isDirectory() && 
                !oldCache.equals(newCache)) {

                CacheUpdateProgressDialog.showProgress(0, 100);
                
                Cache.setCleanupEnabled(false);
                
                // 1) splash screen format hasn't changed
                File splash = new File(oldCache, "splash");
                File newSplash = new File(newCache, "splash");
                copyDir(splash, newSplash);

                // 2) muffins - old muffins need to be parsed just like 
                //    the old cache format
                File muffinDir = new File(oldCache, "muffins");
                if (muffinDir.exists() && muffinDir.isDirectory()) {
                    File[] muffins = findFiles(muffinDir, MUFFIN_PREFIX);

                    for (int i=0; i<muffins.length; i++) try {
                        String name = muffins[i].getName().substring(2);
                        File attrFile = new File(muffins[i].getParentFile(), 
                                        MUFFIN_ATTRIBUTE_PREFIX + name);
                        if (attrFile.exists()) {

                            long[] attributes = getMuffinAttributes(attrFile);
                            URL href = deriveURL(muffinDir, muffins[i], null);
                            if (href != null) {
                                Cache.insertMuffin(href, muffins[i], 
                                        (int) attributes[0], attributes[1]);
                            }
                        } 
                    } catch (Exception e) {
                        Trace.ignored(e);
                    }
                }

                // 3) removed apps
                File removed = new File(oldCache, "removed.apps");

                if (removed.exists()) {
                    File newRemoved = new File(newCache, "removed.apps");
                    try {
                        Cache.copyFile(removed, newRemoved);
                    } catch (IOException e) {
                        Trace.ignored(e);
                    }
                }

                // 4) the cached resources
                File[] apps = findFiles(oldCache, APP_PREFIX);
                File[] exts = findFiles(oldCache, EXT_PREFIX);
                File[] jars = findFiles(oldCache, JAR_PREFIX);
                File[] nativejars = findFiles(oldCache, NATIVE_PREFIX);
                int total = apps.length + exts.length + jars.length +
                            nativejars.length;
                int done = 0;

                // 4.1) applications
                for (int i=0; i<apps.length; i++) {
                    updateJnlpFile(apps[i], oldCache, newCache, false);
                    CacheUpdateProgressDialog.showProgress(++done, total);
                }

                // 4.2) extensions
                for (int i=0; i<exts.length; i++) {
                    updateJnlpFile(exts[i], oldCache, newCache, true);
                    CacheUpdateProgressDialog.showProgress(++done, total);
                }

                // 4.3) jar files
                for (int i=0; i<jars.length; i++) {
                    String dir = jars[i].getParent();
                    String name = jars[i].getName();
                    String nativeDir = name.replaceFirst(JAR_PREFIX, 
                        NATIVE_PREFIX);
                    boolean isNative = (new File(dir, nativeDir)).exists();
                    updateJarFile(jars[i], oldCache, newCache, isNative);
                    CacheUpdateProgressDialog.showProgress(++done, total);
                }
                
            }
        } catch (CacheUpdateProgressDialog.CanceledException ce) {
            // user choose to cancel cache upgrade, we should not do it
            // again anymore
        } catch (Throwable t) {
            Trace.ignored(t);
        } finally {
            CacheUpdateProgressDialog.dismiss();
            Cache.setCleanupEnabled(true);
        }

        // reset "deployment.javaws.cachedir"
        Config.setProperty(Config.JAVAWS_CACHE_KEY, null);

        return true;                // All done
    }

    private static void updateJnlpFile(File file, File oldCache, File newc, 
                                        boolean isExtension) {
        String [] ver = new String[1];
        URL href = deriveURL(oldCache, file, ver);
        if (href != null) try {
            long ts = getTimeStamp(file);
            int type = DownloadEngine.NORMAL_CONTENT_BIT;
            Cache.insertFile(file, type, href, ver[0], ts, 0);
            String name = file.getName().substring(2);
            File lapFile = new File(file.getParentFile(), LAP_PREFIX + name);
            if (lapFile.exists()) {
                Properties props = new Properties();
                BufferedInputStream bis = new BufferedInputStream(
                    new FileInputStream(lapFile));
                props.load(bis);
                bis.close();
                updateLapFile(file, props, href, ver[0], isExtension);
            }
        } catch (Exception e) {
            Trace.ignored(e);
        }
    }

    private static void updateJarFile(File file, File oldCache, File newc,
        boolean nativelib) {

        String [] ver = new String[1];
        URL href = deriveURL(oldCache, file, ver);
        if (href != null) try {
            long ts = getTimeStamp(file);
            int type = DownloadEngine.JAR_CONTENT_BIT;
            if (nativelib) {
                type = type | DownloadEngine.NATIVE_CONTENT_BIT;
            }
            Cache.insertFile(file, type, href, ver[0], ts, 0);
        } catch (Exception e) {
            Trace.ignored(e);
        }
    }

    private static final DateFormat _df = DateFormat.getDateTimeInstance();
    private static final LocalInstallHandler _lih = 
        LocalInstallHandler.getInstance();

    private static void updateLapFile(File jnlpFile, Properties props, 
                URL href, String version, boolean isExtension) {

        LocalApplicationProperties lap = Cache.getLocalApplicationProperties(
            href, version, !isExtension);

        String lastAccessed = props.getProperty("_default.lastAccessed");
        Date d = new Date();
        if (lastAccessed != null) try {
            d = _df.parse(lastAccessed);
        } catch (Exception e) { 
        }
        lap.setLastAccessed(d);

        String launchCount = props.getProperty("_default.launchCount");
        if (launchCount != null) {
           if (launchCount != "0") {
                // this isn't used for anything but knowing if a launch is the 
                // fist launch or not, so just increment it if non-zero
                lap.incrementLaunchCount();
            }
        }

        // policys change between versions - so just say all apps already
        // installed have already asked for desktop integration 
        lap.setAskedForInstall(true);

        // meaning depends on if this is an installer
        String shortcut = props.getProperty("_default.locallyInstalled");

        String title = props.getProperty("_default.title");
        if (title != null) {
            Config.getInstance().addRemoveProgramsRemove(title, false);
        }

        String assMimeType = props.getProperty("_default.mime.types.");
        String assExtensions = props.getProperty("_default.extensions.");

        // extensions are easier.  It is best to say of an extension 
        // installer that it is not installed, and allow it to re-run
        try {
            if (isExtension) {
                if (shortcut != null) {
                    // this really means the the extension is installed, 
                    // but we will not say anything in the new lap file.
                    // this way, the extension installer will be re-run.
                }
            } else {
                // We need a LaunchDesc to install shortcuts, 
                // add/remove, and assoc
                LaunchDesc ld = LaunchDescFactory.buildDescriptor(
                    jnlpFile, URLUtil.getBase(href), null, href);

                if (shortcut != null && shortcut.equalsIgnoreCase("true")) {
                    String path;
                    boolean restoreMenu = false;
                    boolean restoreDesktop = false;
                    path = props.getProperty(
                                         "windows.installedDesktopShortcut");
                    if (path != null) {
                        restoreDesktop = _lih.removeShortcuts(path);
                    }
                    path = props.getProperty(
                                        "windows.installedStartMenuShortcut");
                    if (path != null) {
                        restoreMenu = _lih.removeShortcuts(path);
                    }
                    path = props.getProperty(
                                        "windows.uninstalledStartMenuShortcut");
                    if (path != null) {
                        _lih.removeShortcuts(path);
                    }
                    path = props.getProperty("windows.RContent.shortcuts");
                        if (path != null) {
                        _lih.removeShortcuts(path);
                    }
                    path = props.getProperty("unix.installedDesktopShortcut");
                    if (path != null) {
                        restoreDesktop = _lih.removeShortcuts(path);
                    }
                    path = props.getProperty("unix.installedDirectoryFile");
                    if (path != null) {
                        _lih.removeShortcuts(path);
                    }
                    path = props.getProperty(
                                "unix.gnome.installedStartMenuShortcut");
                    if (path != null) {
                        restoreMenu = _lih.removeShortcuts(path);
                    }
                    path = props.getProperty(
                                "unix.gnome.installedUninstallShortcut");
                    if (path != null) {
                        _lih.removeShortcuts(path);
                    }
                    path = props.getProperty("unix.gnome.installedRCShortcut");
                    if (path != null) {
                        _lih.removeShortcuts(path);
                    }
                    if (restoreDesktop || restoreMenu) {
                        _lih.reinstallShortcuts(ld, lap, 
                            restoreDesktop, restoreMenu);
                    }
                }
                if (assMimeType != null || assExtensions != null) {
                    _lih.removeAssociations(
                                            assMimeType, assExtensions);
                    _lih.reinstallAssociations(ld, lap);
                }
                _lih.removeFromInstallPanel(ld, lap);
                _lih.registerWithInstallPanel(ld, lap);
            }  
        } catch (Exception e) {
            Trace.ignored(e);
        } finally {
            try {
                lap.store();
            } catch (IOException ioe) {
                Trace.ignoredException(ioe);
            }
        }
    }

    private static long getTimeStamp(File file) {
        try {
            String name = file.getName();
            if (name.charAt(1) == MAIN_FILE_TAG) {
                // replace MAIN_FILE_TAG with TIMESTAMP_TAG
                name = name.replaceFirst("M","T");
                File tsFile = new File(file.getParentFile(), name);
                BufferedReader br  = null;
                try {
                    InputStream is = new FileInputStream(tsFile);
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
            } else {
            }
        } catch (Exception e) {
        }
        return 0;
    }

    private static URL deriveURL(File dir, File file, String [] version) {


        String rest = file.toString().substring(dir.toString().length()+ 1);

        StringTokenizer st = new StringTokenizer(rest, File.separator);

        try {
            String protocol = st.nextToken();
            if (protocol.equals("http") || protocol.equals("https")) {
                String path = "/";
                String host = st.nextToken().substring(1);
                int port = 
                    (new Integer(st.nextToken().substring(1)).intValue());
                String next = st.nextToken();
                if (next.startsWith("V")) {
                    version[0] = next.substring(1);
                    next = st.nextToken();
                }
                while (next.startsWith("DM")) {
                    path += next.substring(2) + "/";
                    next = st.nextToken();
                }
                path += next.substring(2);
                if (port == 80) {
                    port = -1;
                }
                return new URL(protocol, host, port, path);
            }
        } catch (Exception e) {
            // whatever goes wrong, we get no URL
            Trace.ignored(e);
        }
        return null;
    }

    // this is a very forgiving dirCopy.  create and copy if you can
    //
    private static void copyDir(File src, File dst) {
        if (src.exists() && src.isDirectory()) {
            dst.mkdirs();
            File [] files = src.listFiles();
            for (int i=0; i<files.length; i++) {
                File target = new File(dst, files[i].getName());
                if (files[i].isDirectory()) {
                    copyDir(files[i], target);
                } else if (!target.exists()) {
                    try {
                        Cache.copyFile(files[i], target);
                    } catch (IOException e) {
                        Trace.ignored(e);
                    }
                }
            }
        }
    }

    private static File[] findFiles(final File dir, final String prefix) {
        ArrayList list = new ArrayList();
        String[] children = dir.list(new FilenameFilter() {
            public boolean accept(File dir, String name) {
                try {
                    if (new File(dir, name).isDirectory()) {
                        return (!name.startsWith(NATIVE_PREFIX));
                    }
                } catch (Exception e) {
                    return false;
                }
                return (name.startsWith(prefix));
            }
        });
        for (int i=0;i < children.length; i++) {
            if (children[i].startsWith(prefix)) {
                list.add(new File(dir, children[i]));
            } else {
                File subdir = new File(dir, children[i]);
                File[] files = findFiles(subdir, prefix);
                for (int j=0; j<files.length; j++) {
                    list.add(files[j]);
                }
            }
        }
        return (File []) list.toArray(new File[0]);
    }


   private static long[] getMuffinAttributes(File attributeFile) 
                                                throws IOException {
        BufferedReader br = null;
        long tag = -1;
        long maxsize = -1;
        try {
            InputStream is = new FileInputStream(attributeFile);
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
            if (br != null) {
                br.close();
            }
        }
        return new long [] {tag, maxsize};
    }


    public static boolean systemUpdateCheck() {
        if (!Environment.isSystemCacheMode()) {
            return false;
	}

	long lastAccessed = Cache.getLastAccessed(true);
	if (lastAccessed > 0) {
	    // system cach allready updated
	    return false;
	}

	String sysdir = Config.getSystemCacheDirectory();
	File oldCache = new File(sysdir, "javaws");
	File newCache = new File(sysdir, Cache.getCacheVersionString());

	if (!oldCache.exists() || !oldCache.isDirectory()) {
	    // no old cache to update from
	    return false;
	}

        try {
	    CacheUpdateProgressDialog.setSystemCache(true);
            CacheUpdateProgressDialog.showProgress(0, 100);

            Cache.setCleanupEnabled(false);
            
            // 1) splash screen - not kept in system cache

            // 2) muffins - not kept in system cache

            // 3) removed apps - not kept in system directory

            // 4) the cached resources
            File[] apps = findFiles(oldCache, APP_PREFIX);
            File[] exts = findFiles(oldCache, EXT_PREFIX);
            File[] jars = findFiles(oldCache, JAR_PREFIX);
            File[] nativejars = findFiles(oldCache, NATIVE_PREFIX);
            int total = apps.length + exts.length + jars.length +
                        nativejars.length;
            int done = 0;

            // 4.1) applications
            for (int i=0; i<apps.length; i++) {
                updateJnlpFile(apps[i], oldCache, newCache, false);
                CacheUpdateProgressDialog.showProgress(++done, total);
            }

            // 4.2) extensions
            for (int i=0; i<exts.length; i++) {
                updateJnlpFile(exts[i], oldCache, newCache, true);
                CacheUpdateProgressDialog.showProgress(++done, total);
            }

            // 4.3) jar files
            for (int i=0; i<jars.length; i++) {
                String dir = jars[i].getParent();
                String name = jars[i].getName();
                String nativeDir = name.replaceFirst(JAR_PREFIX, 
                    NATIVE_PREFIX);
                boolean isNative = (new File(dir, nativeDir)).exists();
                updateJarFile(jars[i], oldCache, newCache, isNative);
                CacheUpdateProgressDialog.showProgress(++done, total);
            }
        } catch (CacheUpdateProgressDialog.CanceledException ce) {
            // user choose to cancel cache upgrade, we should not do it
            // again anymore
        } catch (Throwable t) {
            Trace.ignored(t);
        } finally {
            CacheUpdateProgressDialog.dismiss();
            Cache.setCleanupEnabled(true);
        }

        return true;                // All done
    }

}
