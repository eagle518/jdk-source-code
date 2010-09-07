/*
 * @(#)Applet2ClassLoaderCache.java	1.3 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.applet;

import java.io.*;
import java.lang.ref.*;
import java.util.*;
import sun.awt.AppContext;
import sun.plugin2.util.SystemUtil;

/** This class implements the class loader caching mechanism present
    in the old plug-in. It appears that for compatibility reasons it
    is a requirement to implement a similar class loader cache to the
    old plug-in's. The rule is that if two applets have the same
    codebase, and the same archive tag, then they will be loaded by
    the same class loader. This allows the two applets to see each
    others' static variables. It appears that web sites such as Pogo
    using multiple applets on the same web page are relying on this
    behavior. <P>

    There are non-trivial problems associated with the class loader
    caching mechanism. For one thing, we can no longer expect to be
    able even to cooperatively shut down an applet. Doing so involves
    disposing the AppContext, and therefore all top-level windows
    created by the applet, as well as terminating all threads in the
    applet's ThreadGroup and sub-ThreadGroups. Since now more than one
    applet might be using the same ClassLoader, we can't dispose the
    AppContext if another applet is still running. For this reason we
    want to move away from this semantic as soon as possible, for
    example when launching applets from JNLP files. <P>

    In this new implementation, an individual applet may opt out of
    the class loader caching mechanism by specifying the applet
    parameter <CODE>classloader_cache=false</CODE> (more precisely,
    <CODE>&lt;PARAM name="classloader_cache"
    value="false"&gt;&lt;/PARAM&gt;</CODE>). The support for this is
    at a higher level than this class; if the class loader cache is
    passed in to an Applet2Manager instance, then it will be
    consulted. <P>

    NOTE: the implicit lock ranking implies that the Applet2Manager
    which calls into this class must always be locked upon entry.
*/

public class Applet2ClassLoaderCache {
    private static final boolean DEBUG = (SystemUtil.getenv("JPI_PLUGIN2_DEBUG") != null);

    /** An implementation of this interface must be provided, which
        decouples the creation and destruction of entries in the cache
        (and therefore creation of the AppContext, ThreadGroup, etc.)
        from the management of the cache itself. */
    public static interface EntryCreator {
        /** Creates a new Applet2ClassLoader for the given Entry. This
            has the side-effect of creating a new AppContext and
            ThreadGroup as well. */
        public void createAll(Applet2Manager manager, Entry entry);

        /** Creates a new ThreadGroup and AppContext for the given
            Entry, which already has a usable Applet2ClassLoader. */
        public void createThreadGroupAndAppContext(Applet2Manager manager, Entry entry);

        /** Destroys the ThreadGroup and AppContext associated with a
            given Entry. This is called when the number of applets
            associated with a given Entry goes to zero. */
        public void destroyThreadGroupAndAppContext(Applet2Manager manager,
                                                    Applet2StopListener stopListener,
                                                    long timeToWait,
                                                    Entry entry);
    }

    private EntryCreator entryCreator;

    /** Represents an entry in the class loader cache. */
    public class Entry {
        // The key under which this Entry is mapped
        private String             classLoaderCacheKey;

        // The number of running applets currently referencing this
        // class loader cache entry
        private int                refCount;

        private Applet2ClassLoader classLoader;
        private AppContext         appContext;
        private ThreadGroup        threadGroup;

        // Called only by the ClassLoaderCache
        private Entry(String classLoaderCacheKey) {
            this.classLoaderCacheKey = classLoaderCacheKey;
        }

        /** Retrieves the key in the class loader cache associated
            with this Entry. */
        public String getClassLoaderCacheKey() {
            return classLoaderCacheKey;
        }

        /** Sets the ClassLoader associated with this entry. */
        public void setClassLoader(Applet2ClassLoader classLoader) {
            this.classLoader = classLoader;
        }

        /** Gets the ClassLoader associated with this entry. */
        public Applet2ClassLoader getClassLoader() {
            return classLoader;
        }
        
        /** Sets the AppContext associated with this entry. */
        public void setAppContext(AppContext appContext) {
            this.appContext = appContext;
        }

        /** Gets the AppContext associated with this entry. */
        public AppContext getAppContext() {
            return appContext;
        }

        /** Sets the ThreadGroup associated with this entry. */
        public void setThreadGroup(ThreadGroup threadGroup) {
            this.threadGroup = threadGroup;
        }

        /** Gets the ThreadGroup associated with this entry. */
        public ThreadGroup getThreadGroup() {
            return threadGroup;
        }

        // NOTE: the implicit lock ranking in this class implies that
        // the Applet2ClassLoaderCache must actually NOT be locked
        // when these methods are called. We want to avoid having
        // applets using different class loader cache entries block
        // each other during termination. unref() requires locking the
        // Applet2ClassLoaderCache, so we must require that it not be
        // locked before calling these methods.

        private synchronized void ref(Applet2Manager caller) {
            assert !Thread.holdsLock(Applet2ClassLoaderCache.this);

            if (++refCount == 1) {
                // Either this entry was just created, or just
                // resurrected from the zombie list
                if (classLoader == null) {
                    entryCreator.createAll(caller, this);
                } else {
                    entryCreator.createThreadGroupAndAppContext(caller, this);
                }
            }
        }

        private synchronized void unref(Applet2Manager caller,
                                        Applet2StopListener stopListener,
                                        long timeToWait) {
            assert !Thread.holdsLock(Applet2ClassLoaderCache.this);

            if (--refCount == 0) {
                entryCreator.destroyThreadGroupAndAppContext(caller, stopListener, timeToWait, this);
                moveToZombieList(this);
            }
        }

        private synchronized void dump(PrintStream ps) {
            ps.println("key=" + classLoaderCacheKey +
                       ", refCount=" + refCount +
                       ", threadGroup=" + threadGroup);
        }
    }

    /** Creates an Applet2ClassLoaderCache with the given EntryCreator
        for creating new Entry objects. */
    public Applet2ClassLoaderCache(EntryCreator entryCreator) {
        this.entryCreator = entryCreator;
    }

    /** Indicates whether this class loader cache is even supposed to
        be consulted at all. */
    public boolean isInUse() {
        return IS_IN_USE;
    }

    /** Fetches a class loader cache entry for the given
        Applet2Manager. It will be populated with a ClassLoader,
        ThreadGroup and AppContext upon return. */
    public Entry get(String classLoaderCacheKey, Applet2Manager caller) {
        assert Thread.holdsLock(caller);

        Entry entry = null;

        synchronized (this) {
            boolean mustAdd = false;
            entry = (Entry) cache.get(classLoaderCacheKey);
            if (entry == null) {
                mustAdd = true;
                // Look for an entry in the zombie list instead
                entry = lookupInZombieList(classLoaderCacheKey);
                if (entry == null) {
                    // Create a new entry instead
                    entry = new Entry(classLoaderCacheKey);
                    if (DEBUG) {
                        System.out.println("Applet2ClassLoaderCache created new entry for " + classLoaderCacheKey);
                    }
                } else {
                    if (DEBUG) {
                        System.out.println("Applet2ClassLoaderCache using zombie list entry for " + classLoaderCacheKey);
                    }
                }
            } else {
                if (DEBUG) {
                    System.out.println("Applet2ClassLoaderCache reusing entry for " + classLoaderCacheKey);
                }
            }

            if (mustAdd) {
                cache.put(classLoaderCacheKey, entry);
            }
        }
        
        entry.ref(caller);
        return entry;
    }

    /** Releases a class loader cache entry previously provided to the
        given Applet2Manager. The afterStopRunnable and stopListener
        have the same semantics as in {@link
        sun.plugin2.applet.Applet2Manager#stop(Runnable,
        Applet2StopListener) Applet2Manager.stop}. */
    public void release(Entry entry,
                        Applet2Manager caller,
                        Applet2StopListener stopListener,
                        long timeToWait) {
        assert Thread.holdsLock(caller);

        entry.unref(caller, stopListener, timeToWait);
    }

    /** Marks any class loader cache entry corresponding to the given
        cache key as not cacheable any more. */
    public synchronized void markNotCacheable(String classLoaderCacheKey) {
        cache.remove(classLoaderCacheKey);
        for (Iterator iter = zombieList.iterator(); iter.hasNext(); ) {
            SoftReference ref = (SoftReference) iter.next();
            Entry entry = (Entry) ref.get();
            if (entry != null) {
                if (entry.getClassLoaderCacheKey().equals(classLoaderCacheKey)) {
                    iter.remove();
                }
            }
        }
    }

    /** Marks the given class loader cache entry as not cacheable any
        more. This is necessary when exceptions during applet startup
        or termination might pollute the class loader's state. */
    public synchronized void markNotCacheable(Entry entry) {
        // Don't remove an Entry from the cache unless it's this one
        // (to prevent repeated notifications from blowing away new
        // cache entries)
        conditionallyRemoveFromCache(entry);
    }

    /** Clears out the class loader cache. */
    public synchronized void clear() {
        cache.clear();
        zombieList.clear();
    }

    /** Dumps the contents of the class loader cache to the given
        PrintStream. */
    public synchronized void dump(PrintStream ps) {
        ps.println("Dumping class loader cache...");
        for (Iterator iter = cache.values().iterator(); iter.hasNext(); ) {
            Entry entry = (Entry) iter.next();
            ps.print(" Live entry: ");
            entry.dump(ps);
        }
        for (Iterator iter = zombieList.iterator(); iter.hasNext(); ) {
            SoftReference ref = (SoftReference) iter.next();
            Entry entry = (Entry) ref.get();
            if (entry != null) {
                ps.print(" Zombie entry: ");
                entry.dump(ps);
            }
        }
        ps.println("Done.");
    }

    //----------------------------------------------------------------------
    // Internals only below this point
    //

    // We keep here a global indication (not just per-applet) of
    // whether the cache is in use to keep all of the system property
    // querying code in one place
    private static final boolean IS_IN_USE;

    // This is the map of class loaders of currently running applets
    private Map/*<String, Entry>*/ cache = new HashMap/*<String, Entry>*/();

    // This is a list of cached class loaders which don't currently
    // have any running applets
    private List/*<SoftReference<Entry>>*/ zombieList = new ArrayList/*<SoftReference<Entry>>*/();

    // This is the maximum size of the zombie list; this used to be a
    // tunable parameter in the old plug-in but nobody was using that
    // functionality
    private static final int MAX_ZOMBIES;

    static {
        String value = (String) java.security.AccessController.doPrivileged(
                           new sun.security.action.GetPropertyAction("javaplugin.classloader.cache.enabled"));
        IS_IN_USE = (value == null || value.equals("true"));

        int zombies = ((Integer) java.security.AccessController.doPrivileged(
		    new sun.security.action.GetIntegerAction("javaplugin.classloader.cache.sizes", 4))).intValue();

	// For XP/IE, we limit the size to 4 max. to
	// reduce the chance of running out of memory.
	if (zombies > 4)
	    zombies = 4;

        MAX_ZOMBIES = zombies;
    }

    // Removes the given Entry from the cache if the entry is
    // currently in the cache. Returns the entry if it was actually
    // removed, or null if it was not in the cache.
    // NOTE: it is assumed this is called under the cover of a higher-level lock
    private Entry conditionallyRemoveFromCache(Entry entry) {
        Entry res = (Entry) cache.remove(entry.getClassLoaderCacheKey());
        if (res == entry) {
            return res;
        }
        return null;
    }

    // NOTE: it is assumed this is called under the cover of a higher-level lock
    private void cleanupZombieList() {
        for (Iterator iter = zombieList.iterator(); iter.hasNext(); ) {
            SoftReference ref = (SoftReference) iter.next();
            if (ref.get() == null) {
                iter.remove();
            }
        }
    }

    // Moves the given Entry to the zombie list assuming it is still
    // valid to use this Entry.
    private synchronized void moveToZombieList(Entry entry) {
        entry = conditionallyRemoveFromCache(entry);
        if (entry != null) {
            cleanupZombieList();
            zombieList.add(0, new SoftReference(entry));
            if (zombieList.size() > MAX_ZOMBIES) {
                // Remove least recently used entry
                zombieList.remove(zombieList.size() - 1);
            }
        }
    }

    // NOTE: it is assumed this is called under the cover of a higher-level lock
    private Entry lookupInZombieList(String classLoaderCacheKey) {
        for (Iterator iter = zombieList.iterator(); iter.hasNext(); ) {
            SoftReference ref = (SoftReference) iter.next();
            Entry entry = (Entry) ref.get();
            if (entry != null && entry.getClassLoaderCacheKey().equals(classLoaderCacheKey)) {
                iter.remove();
                return entry;
            }
        }
        return null;
    }
}
