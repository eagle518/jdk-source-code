/*
 * @(#)MemoryCache.java	1.8 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.cache;

import com.sun.deploy.net.DownloadEngine;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import java.lang.ref.WeakReference;
import java.lang.ref.Reference;
import java.lang.ref.ReferenceQueue;
import java.util.HashMap;
import java.util.Set;
import java.util.HashSet;
import java.util.Map;

/* 
     This class tracks used resources for 2 reasons:
       a) faster loading if resources are needed again
       b) prevents resources from being removed from the disk cache 
          (by CleanupThread)

     Implementation is based on the following:
       a) every get() requests gets new copy of the object
          (CachedJarFile, File or CacheEntry)
       b) phantom references to returned values are saved 
          (in the LoadedResourceReference) 
       c) every time reference is processed by cleanup thread
          reference counter in the corresponding LoadedResourceReference 
          is decreased
       d) LoadedResourceReference is removed as counter gets to 0
          (this happen when clones of object we have returned will be gc-ed).

      Note that CachedJarFile.clone() will increment usage counter 
      but if someone will clone File object usage counter will not increase
      automatically. It is not possible to protect from this fully 
      because it is always possible to get full pathname and create 
      File object from scratch.
 */
public class MemoryCache {
    // table of LoadedResourceReferences to loaded resources. 
    private static final Map loadedResource;
    // Reference queue to handle weak references in hashtable
    private static ReferenceQueue refQueue = new ReferenceQueue();
    private static Thread cleanupThread;

    static {
        loadedResource = new HashMap();
        cleanupThread = new LoadedResourceCleanupThread("CacheMemoryCleanUpThread");
        cleanupThread.setDaemon(true);
        cleanupThread.start();
    }

    synchronized static void reset() {
        loadedResource.clear();
    }

    /* returns reference to previous resource with same url (if any) */
    synchronized public static Object addLoadedResource(String url, Object o) {
        LoadedResourceReference r = 
                (LoadedResourceReference) loadedResource.get(url);
        if (r != null && r.get() != null) {
            Trace.println("Replacing MemoryCache entry (cnt="+r.refcnt+") for " + url
                    + "was="+r.get().getClass().getName()+" ("+r.get().hashCode()+")"
                    + " now="+o.getClass().getName()+ " ("+o.hashCode()+")",
                    TraceLevel.CACHE);
        }
        LoadedResourceReference ref = new LoadedResourceReference(o);
        if (!(o instanceof CacheEntry)) {
            ref.registerReference(new CachedResourceReference(o, refQueue, url));
        }
        LoadedResourceReference ref2 = 
                (LoadedResourceReference) loadedResource.put(url, ref);
        if (ref2 != null) {
           /* do not need to register returned reference 
              (unlike we do in getLoadedResource ()) 
              because old version of loaded resource is not tracked anymore */
           return ref2.get();
        }
        return null;
    }

    synchronized public static Object getLoadedResource(String url) {
        LoadedResourceReference ref =
                (LoadedResourceReference) loadedResource.get(url);
        if (ref == null) {
            return null;
        }
        Object result = ref.get();
        if (result != null) {
          addResourceReference(result, url);
        }
        return result;
    }

    synchronized public static Object removeLoadedResource(String url) {
        LoadedResourceReference ref =
                (LoadedResourceReference) loadedResource.remove(url);
        return (ref == null ? null : ref.get());
    }

    synchronized public static void clearLoadedResources() {
        loadedResource.clear();
        DownloadEngine.clearUpdateCheckDoneList();
        DownloadEngine.clearNoCacheJarFileList();
        return;
    }

    synchronized static void addResourceReference(Object file, String url) {
        LoadedResourceReference loadedRef =
                (LoadedResourceReference) loadedResource.get(url);

        if (loadedRef != null) {
            loadedRef.registerReference(
                    new CachedResourceReference(file, refQueue, url));
        }
    }

    synchronized static boolean contains(String url) {
        return loadedResource.containsKey(url);
    }

    private static class LoadedResourceReference {
        /* This class encapsulates reference to resource, 
         * usage counter and set of phantom references to 
         * clones of resource that are being used.
         * 
         * There is just one LoadedResourceReference per resource 
         * referenced via loadedResourceMap (CachedJarFile, File or CacheEntry).
         * 
         * On EVERY request returning JarFile or File to external code resource 
         * new Reference is created and registered with LoadedResourceReference.
         * This includes 3 cases:
         *   1) JarFile/File objects directly stored in the LoadedResourceReference
         *   2) JarFile/File objects returned by CacheEntry that 
         *      is referenced from loadedResources
         *   3) Attempt to clone CachedJarFile instance
         */

        private Set resourceRefs = new HashSet();
        private int refcnt = 0;
        Object o;

        LoadedResourceReference(Object ref) {
            o = ref;
        }

        Object get() {
            return o;
        }

        synchronized void registerReference(Reference r) {
            if (resourceRefs.add(r)) {
                refcnt++;
            }
        }

        /* returns true when last reference was released */
        synchronized boolean deregisterReference(Reference r) {
            refcnt--;
            resourceRefs.remove(r);
            return (refcnt == 0);
        }
    }

    // This is reference used to implement usage counter for resource.
    // We need to keep URL to be able to find out clone of which 
    // resource was released.
    // Weak references are used because we only need to track when 
    // referenced object got garbage collected. 
    // (they seems to have even fewer restriction on referents
    //  compared to phantom references)
    private static class CachedResourceReference extends WeakReference {

        String url;

        public CachedResourceReference(Object obj, ReferenceQueue q, String url) {
            super(obj, q);
            this.url = url;
        }

        public String getURL() {
            return url;
        }

        public int hashCode() {
            //can not use object for hashcode calculation as
            //object reference can be cleared and hashcode has to stay the same (!)
            return url.hashCode();
        }

        //@Override
        public boolean equals(Object o) {
            CachedResourceReference r = (CachedResourceReference) o;
            if (r != null && (get() == r.get()) && url.equals(r.getURL())) {
                return true;
            }
            return false;
        }
    }

    static class LoadedResourceCleanupThread extends Thread {

        LoadedResourceCleanupThread(String name) {
            super(name);
        }

        public void run() {
            CachedResourceReference ref;
            LoadedResourceReference loadedRef;
            String u;

            // Clean up the reference queue
            while (true) {
                try {
                    ref = (CachedResourceReference) refQueue.remove();
                    synchronized (loadedResource) {
                        u = ref.getURL();
                        loadedRef = (LoadedResourceReference) loadedResource.get(u);
                        if (loadedRef != null && loadedRef.deregisterReference(ref)) {
                            removeLoadedResource(u);
                        }
                    }
                } catch (InterruptedException e) {}
            }
        }
    }
}
