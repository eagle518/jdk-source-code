/*
 * @(#)Applet2ManagerCache.java	1.4 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.applet;

import java.net.*;
import java.util.*;
import com.sun.deploy.util.Trace;

/** This class implements a cache of Applet2Manager instances and the
    applets they host in support of the "legacy applet lifecycle",
    where actual applet instances persist from page view to page
    view. */

public class Applet2ManagerCache {
    /** Retrieves an Applet2Manager instance, and its associated
        applet, which was previously inserted in the cache. */
    public Applet2Manager get(String documentBase,
                              Map/*<String, String>*/ appletParameters) {
        // Skip unnecessary work
        if (appletParameters.get("legacy_lifecycle") == null)
            return null;
        String key = getCacheKey(documentBase, appletParameters);
        synchronized(this) {
            for (Iterator iter = entries.iterator(); iter.hasNext(); ) {
                Entry entry = (Entry) iter.next();
                if (entry.getKey().equals(key)) {
                    iter.remove();
                    Trace.msgPrintln("lifecycle.applet.found");
                    return entry.getManager();
                }
            }
        }
        return null;
    }

    /** Places an Applet2Manager instance in the cache. Note that
        there is an implicit assumption that this method must be
        called from the system AppContext, to ensure that the thread
        calling destroy() isn't in the applet's AppContext. Note also
        that in general the Applet2Manager is responsible for putting
        itself in this cache, so users should not need to call this
        method. */
    public void put(Applet2Manager manager) {
        String key = manager.getLegacyLifeCycleCacheKey();
        List removedEntries = new ArrayList();
        Entry entry = new Entry(key, manager);
        synchronized(this) {
            Trace.msgPrintln("lifecycle.applet.support");
            entries.add(0, entry);
            while (entries.size() > cacheSize) {
                Entry tmp = (Entry) entries.remove(entries.size() - 1);
                removedEntries.add(tmp);
            }
        }
        if (removedEntries.size() > 0) {
            Trace.msgPrintln("lifecycle.applet.cachefull");
            destroy(removedEntries);
        }
        startWatchdog();
    }

    /** Indicates whether the cache is empty. */
    public boolean isEmpty() {
        synchronized(this) {
            return entries.isEmpty();
        }
    }

    /** Clears the cache, destroying any applets contained within. */
    public void clear() {
        List removedEntries = new ArrayList();
        synchronized(this) {
            removedEntries.addAll(entries);
            entries.clear();
        }
        destroy(removedEntries);
        stopWatchdog();
    }

    /** Converts a document base and parameter map into a key for this
        cache. */
    public String getCacheKey(String documentBase,
                              Map/*<String, String>*/ appletParameters) {
        StringBuffer key = new StringBuffer();
        if (documentBase != null) {
            key.append("<NAME=_documentBase VALUE=");
            key.append(documentBase);
            key.append(">");
        }
        for (Iterator iter = appletParameters.keySet().iterator(); iter.hasNext(); ) {
            String k = (String) iter.next();
            String v = (String) appletParameters.get(k);
            if (k != null && v != null) {
                key.append("<NAME=");
                key.append(k);
                key.append(" VALUE=");
                key.append(v);
                key.append(">");
            }
        }
        return key.toString();
    }

    //----------------------------------------------------------------------
    // Internals only below this point
    //

    class Entry {
        private String key;
        private Applet2Manager manager;

        Entry(String key, Applet2Manager manager) {
            this.key = key;
            this.manager = manager;
        }

        public String getKey() {
            return key;
        }

        public Applet2Manager getManager() {
            return manager;
        }
    }

    // Provide control over the cache size via this system property for backward compatibility
    // Note increased cache size over previous implementation
    private int cacheSize = Integer.getInteger("javaplugin.lifecycle.cachesize", 4).intValue();

    // We ideally would like a multi-map with least-recently-used
    // iteration order, and where each entry is treated
    // individually. A LinkedHashMap mapping keys to lists of values
    // can approximate this, but we would like to do better, so here
    // we keep individual entries in a linked list. The entries at the
    // front are the newest, and those at the back are the oldest.
    private List/*<Entry>*/ entries = new LinkedList();

    // Destroys this list of entries
    private void destroy(List/*<Entry>*/ removedEntries) {
        for (Iterator iter = removedEntries.iterator(); iter.hasNext(); ) {
            ((Entry) iter.next()).getManager().destroy();
        }
    }

    // Because this cache must call the destroy() method on applets it
    // flushes, it can not simply use weak references like the
    // Applet2ClassLoaderCache. This opens up the possibility that an
    // applet pointing to a large object graph might hold on to a lot
    // of storage, leading to OutOfMemoryErrors when attempting to
    // load subsequent applets.
    //
    // To mitigate this problem, we start a timer which watches the
    // memory consumption of this JVM, and clears the cache when
    // memory pressure gets high. This is only an approximation and is
    // not guaranteed to work, or even to detect problems in time. We
    // ordinarily schedule the timer to go off only rarely and
    // increase the frequency as memory pressure gets higher.

    private Timer memoryPressureTimer = new Timer();
    private TimerTask currentTask = null;

    // Starts the watchdog timer
    private synchronized void startWatchdog() {
        if (currentTask == null) {
            currentTask = new WatchdogTask();
            long period = getTimerPeriod(getMemoryPressure());
            memoryPressureTimer.schedule(currentTask, period, period);
        }
    }

    // Stops the watchdog timer; it doesn't need to run unless there's
    // at least one entry in the cache
    private synchronized void stopWatchdog() {
        if (currentTask != null) {
            currentTask.cancel();
            currentTask = null;
        }
    }

    // Returns a fraction (0..1) where 0 indicates no memory pressure
    // and 1 indicates the heap is completely full
    private static float getMemoryPressure() {
        Runtime runtime = Runtime.getRuntime();
        long maxMemory = runtime.maxMemory();
        if (maxMemory == Long.MAX_VALUE) {
            // Assume this JVM isn't constrained by a fixed heap size
            return 0;
        }
        long totalMemory = runtime.totalMemory();
        float pressure = (float) ((double) totalMemory / (double) maxMemory);
        if (pressure < 0)
            pressure = 0;
        if (pressure > 1)
            pressure = 1;
        return pressure;
    }

    // Returns
    private static long getTimerPeriod(float pressure) {
        if (pressure < 0.5f) {
            return 30 * 1000; // 30 seconds if pressure is low
        }

        if (pressure < 0.75f) {
            return 15 * 1000; // 15 seconds if pressure is medium
        }

        return 5 * 1000; // Every 5 seconds if memory pressure is high
    }

    class WatchdogTask extends TimerTask {
        private long period = getTimerPeriod(getMemoryPressure());

        public void run() {
            float pressure = getMemoryPressure();

            if (pressure > 0.9f) {
                // Clear the cache
                clear();
            }

            // Figure out whether we need to reschedule a new timer
            // with a different period
            if (getTimerPeriod(getMemoryPressure()) != period) {
                stopWatchdog();
                startWatchdog();
            }
        }
    }
}
