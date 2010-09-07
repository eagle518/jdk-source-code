/*
 * @(#)NativeLibraryBundle.java	1.8 10/03/24
 * 
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.util;

import java.io.File;
import java.io.FileFilter;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.channels.FileChannel;
import java.nio.channels.FileLock;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;

import com.sun.deploy.config.Config;
import com.sun.deploy.util.Trace;
import java.util.jar.Attributes;
import java.util.jar.JarFile;
import java.util.jar.Manifest;

/** This class models a "bundle" of native libraries. The basic
    problem we have is that the shared Java Web Start classes store
    exactly one copy of the native libraries associated with a
    nativelib jar file in the deployment cache. In the context of
    multiple applets that might pull in such native libraries, we need
    to make a separate copy of these native libraries for each
    ClassLoader instance that loads them. This class helps manage such
    groups of native libraries. <P>

    Clonning of libraries hurts performance and we try to avoid this if possible:
    <UL>
      <LI> if this is first request for the libraries from this jar in this
           process then we use original set of libraries directly from cache
      <LI> if jar manifests claims that that jar consists of independent libraries
        then clonning is performed for individual libs (instead of whole sets).
    </UL>

    The directory structure we use is the following:

<PRE>
  java.io.tmpdir
    .java_jnlp_applet_nativelib_cache.<user.name>
        abcdefg.lock
        abcdefg/
           nativelib1.dll
           nativelib2.dll
</PRE>

    A new directory "abcdefg" (random name) is created for each copy
    of a set of native libraries we make out of the deployment
    cache. <P>

    The lock file is used to mediate access to this directory. As long
    as the native libraries are in use, the lock file is locked by the
    current JVM instance. <P>

    We attempt to proactively clean up the temporary copy of the
    native libraries, the directory containing them, and the lock file
    when the ClassLoader instance is garbage collected.  However,
    there is no guarantee that we will be able to do this -- there is
    no way to ensure that the finalizable ClassLoader$NativeLibrary
    object, which takes care of closing the native library, will be
    GCd before the NativeLibraryBundle.  Therefore deletion of the
    native library files might fail. <P>

    Each time a JVM instance starts up and uses this mechanism, we
    scan the nativelib cache directory and try to clean up stale
    temporary directories which were created by earlier, inactive JVM
    instances, to prevent monotonic growth of this directory. <P>
*/

public class NativeLibraryBundle {
    private static File rootDir;

    private static final String LOCK_SUFFIX = ".lck";

    /*
     * This property allows to have "large" jars with several independent native
     * libraries (saving network requests) but do not clone/lock them all together.
     */
    private static Attributes.Name independentLibsAttribute =
            new Attributes.Name("IndependentLibraries");

    // This is the set of temporary directories we have created in
    // this JVM instance. We keep track of this to avoid deleting
    // these directories in the cleaner thread which runs upon
    // startup.
    private static Set/*<String>*/ dirsCreatedByThisJVM =
        Collections.synchronizedSet(new HashSet());

    // The lock file this bundle holds on to
    private File lockFile;
    private FileLockWrapper lock;

    // The directory name within the temporary directory this
    // NativeLibraryBundle owns
    private String dirName;

    static {
        String rootDirName = System.getProperty("java.io.tmpdir") +
            File.separator + ".java_jnlp_applet_nativelib_cache." +
            System.getProperty("user.name");
        rootDir = new File(rootDirName);
        // Create the cache directory if it doesn't exist yet
        if (!rootDir.exists()) {
            rootDir.mkdir();
        }
        // Run the cleaner thread to clean up any previous JVM
        // instances' temporary directories
        Thread t = new Thread() {
                public void run() {
                    deleteOldDirs();
                }
            };
        t.setName("Native Library Cache Cleaner Thread");
        t.setDaemon(true);
        t.start();
    }

    private Map /*<String,String>*/ libNameMap = new HashMap();
    private File destination = null;

    public NativeLibraryBundle() {}

    public synchronized String get(String name) {
        return (String) libNameMap.get(name);
    }

    private synchronized void put(String libname, String path) {
        libNameMap.put(libname, path);
    }

    protected void finalize() {
        releaseLocksOfOriginals(this);

        // Try to clean myself up
        if (deleteRecursively(new File(rootDir, dirName))) {
            lock.release();
            lockFile.delete();
        }
    }

    private synchronized File getDestinationDir() throws IOException {
        if (destination == null) {
            synchronized (dirsCreatedByThisJVM) {
                lockFile = File.createTempFile("tmp", LOCK_SUFFIX, rootDir);
                String lockFileName = lockFile.getName();
                dirName = lockFileName.substring(0, lockFileName.lastIndexOf(LOCK_SUFFIX));
                dirsCreatedByThisJVM.add(dirName);
            }
            // Lock the lock file
            lock = FileLockWrapper.lockFile(lockFile);
            // OK, now we're okay to create the directory
            destination = new File(rootDir, dirName);
            destination.mkdir();
        }
        return destination;
    }

    static Map /* <String, NativeLibraryBundle> */ originFileLocks = new HashMap();

    //if noone is using libary yet (in this process) then we can
    //use it. Lock it first, so next attempt to use it in place will fail.
    //
    //We may need to lock whole directory if libs in the directory are dependent
    private synchronized static boolean tryLockOriginalCopy(
            File src, boolean dirmode, NativeLibraryBundle bundle) {
        String path;

        if (dirmode) {
            path = src.getParent();
        } else {
            path = src.getAbsolutePath();
        }

        Object o = originFileLocks.get(path);
        if (o == null) {
            originFileLocks.put(src.getParent(), bundle);
            return true;
        } else if (o == bundle) {
            return true;
        }
        return false;
    }

    //release locks owned by given bundle
    private synchronized static void releaseLocksOfOriginals(NativeLibraryBundle b) {
        Iterator it = originFileLocks.entrySet().iterator();
        while(it.hasNext()) {
            Map.Entry m = (Map.Entry) it.next();
            if (m.getValue() != b) {
                it.remove();
            }
        }
    }

    //process one library file
    //  try to grab lock of original main copy first. Register it on success.
    //  Otherwise clone the library and register clonned copy
    //
    // NB: In theory some libraries can be shared between multiple applets
    //   common platform staff we need that does not depend on JNI
    //   (e.g. MS runtime). However, at the moment VM is not allowing to
    //   request loading of the same lib from different classloaders.
    //   Therefore we need to create multiple copies.
    private void processLib(Manifest man, File src, boolean dirmode) throws IOException {
        String path = null;
        if (!tryLockOriginalCopy(src, dirmode, this)) {
            Trace.println("Failed to grab lock for " + src, TraceLevel.CACHE);
            path = copyLib(src);
        } else {
            path = src.getAbsolutePath();
        }
        put(src.getName(), path);
    }


    // Prepares private copy of required subset of libraries from given jar
    // It might be originals if we manage to get the lock or lib is marked as 
    // "safe to share between several applets in the same VM".
    //
    // For jars with multiple libs we will lock all libraries in the same jar 
    // as they could be dependant and we need to make sure we end up with 
    // all files in the same directory when OS will try to load one of them.
    // This is default as it resembles existed behavior.
    // If jar is specially marked as safe to process libs independendly then
    // we can obtain private copy of one lib at time.
    //
    // Note: assumes that given jar file contains the library we looking for
    //  and this jar is already unpacked to given dir.
    public void prepareLibrary(String libname, JarFile jf, String dir) throws IOException {
        Manifest man = jf.getManifest();
        Attributes mainAttr = (man != null) ? man.getMainAttributes() : null;
        //see if we can copy single library only
        if (mainAttr != null 
                && "true".equals(mainAttr.get(independentLibsAttribute))) {
            Trace.println("Lock individual library "+libname, TraceLevel.CACHE);
            File f = new File(dir, libname);
            processLib(man, f, false);
        } else { //dependent libraries - copy everything
            //note that we only accept top-level elements for native jars
            //and therefore we could expect flat directory structure here
            // (see CacheEntry implementation)
            File src = new File(dir);
            File files[] = src.listFiles();
            for (int i=0; i<files.length; i++) {
                processLib(man, files[i], true);
            }
        }
    }

    static class FileLockWrapper {
        private FileOutputStream str;
        private FileChannel chan;
        private FileLock lock;

        private FileLockWrapper(FileOutputStream str,
                                FileChannel chan,
                                FileLock lock) {
            this.str = str;
            this.chan = chan;
            this.lock = lock;
        }

        public void release() {
            try {
                lock.release();
            } catch (IOException e) {
                Trace.ignoredException(e);
            }
            try {
                chan.close();
            } catch (IOException e) {
                Trace.ignoredException(e);
            }
        }
        
        public static FileLockWrapper lockFile(File file) throws IOException {
            FileOutputStream str = new FileOutputStream(file);
            FileChannel chan = str.getChannel();
            FileLock lock = chan.lock();
            if (lock == null) {
                chan.close();
                return null;
            }
            return new FileLockWrapper(str, chan, lock);
        }

        public static FileLockWrapper tryLockFile(File file) throws IOException {
            FileOutputStream str = new FileOutputStream(file);
            FileChannel chan = str.getChannel();
            FileLock lock = chan.tryLock();
            if (lock == null) {
                chan.close();
                return null;
            }
            return new FileLockWrapper(str, chan, lock);
        }
    }

    private String copyLib(File src) throws IOException {
        if (src.isFile() &&
                src.getName().endsWith(Config.getInstance().getLibrarySufix())) {
            FileChannel in = new FileInputStream(src).getChannel();
            File dest = new File(getDestinationDir(), src.getName());
            FileChannel out = new FileOutputStream(dest).getChannel();
            in.transferTo(0, in.size(), out);
            out.force(true);
            in.close();
            out.close();

            return dest.getAbsolutePath();
        }
        return null;
    }

    private static void deleteOldDirs() {
        File[] dirs = rootDir.listFiles(new FileFilter() {
                public boolean accept(File file) {
                    return file.isDirectory();
                }
            });
        for (int i = 0; i < dirs.length; i++) {
            File dir = dirs[i];
            if (!dirsCreatedByThisJVM.contains(dir.getName())) {
                // Try to acquire the lock file associated with this directory
                File lockFile = new File(rootDir, dir.getName() + LOCK_SUFFIX);
                try {
                    FileLockWrapper lock = FileLockWrapper.tryLockFile(lockFile);
                    if (lock != null) {
                        // Delete this directory recursively
                        boolean success = deleteRecursively(dir);
                        if (success) {
                            // Release the lock
                            lock.release();
                            // Delete the lock file
                            lockFile.delete();
                        }
                    }
                } catch (IOException e) {
                    Trace.ignoredException(e);
                }
            }
        }
    }

    private static boolean deleteRecursively(File dir) {
        File[] files = dir.listFiles();
        for (int i = 0; i < files.length; i++) {
            File file = files[i];
            if (file.isDirectory()) {
                deleteRecursively(file);
            } else {
                file.delete();
            }
        }
        return dir.delete();
    }
}
