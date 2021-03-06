/*
 * @(#)ThreadImpl.java	1.18 04/04/18
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.management;

import java.lang.management.ThreadMXBean;

import java.lang.management.ThreadInfo;

/**
 * Implementation class for the thread subsystem.
 * Standard and committed hotspot-specific metrics if any.
 *
 * ManagementFactory.getThreadMXBean() returns an instance
 * of this class.
 */
class ThreadImpl extends MXBeanSupport implements ThreadMXBean {

    private final VMManagement jvm;
 
    // default for thread contention monitoring is disabled.
    private boolean contentionMonitoringEnabled = false;
    private boolean cpuTimeEnabled;

    /**
     * Constructor of ThreadImpl class.
     */
    ThreadImpl(VMManagement vm) {
        super(ThreadMXBean.class);
        this.jvm = vm;
        this.cpuTimeEnabled = jvm.isThreadCpuTimeEnabled();
    }

    public int getThreadCount() {
        return jvm.getLiveThreadCount();
    }

    public int getPeakThreadCount() {
        return jvm.getPeakThreadCount();
    }

    public long getTotalStartedThreadCount() {
        return jvm.getTotalThreadCount();
    }

    public int getDaemonThreadCount() {
        return jvm.getDaemonThreadCount();
    }

    public boolean isThreadContentionMonitoringSupported() {
        return jvm.isThreadContentionMonitoringSupported();
    }

    public synchronized boolean isThreadContentionMonitoringEnabled() {
       if (!isThreadContentionMonitoringSupported()) {
            throw new UnsupportedOperationException(
                "Thread contention monitoring is not supported.");
        }
        return contentionMonitoringEnabled;
    }

    public boolean isThreadCpuTimeSupported() {
        return jvm.isOtherThreadCpuTimeSupported();
    }

    public boolean isCurrentThreadCpuTimeSupported() {
        return jvm.isCurrentThreadCpuTimeSupported();
    }

    public boolean isThreadCpuTimeEnabled() {
        if (!isThreadCpuTimeSupported() &&
            !isCurrentThreadCpuTimeSupported()) {
            throw new UnsupportedOperationException(
                "Thread CPU time measurement is not supported");
        }
        return cpuTimeEnabled;
    }

    public long[] getAllThreadIds() {
        ManagementFactory.checkMonitorAccess();

        Thread[] threads = getThreads();
        int length = threads.length;
        long[] ids = new long[length];
        for (int i = 0; i < length; i++) {
            Thread t = threads[i];
            ids[i] = t.getId();
        }
        return ids;
    }
    
    public ThreadInfo getThreadInfo(long id) {
        if (id <= 0) {
            throw new IllegalArgumentException(
                "Invalid thread ID parameter: " + id);
        }

        long[] ids = new long[1];
        ids[0] = id;
        final ThreadInfo[] infos = getThreadInfo(ids, 0);
        return infos[0];
    }
     
    public ThreadInfo getThreadInfo(long id, int maxDepth) {
        if (id <= 0) {
            throw new IllegalArgumentException(
                "Invalid thread ID parameter: " + id);
        }
        if (maxDepth < 0) {
            throw new IllegalArgumentException(
                "Invalid maxDepth parameter: " + maxDepth);
        }

        long[] ids = new long[1];
        ids[0] = id;
        final ThreadInfo[] infos = getThreadInfo(ids, maxDepth);
        return infos[0];
    }
  
    public ThreadInfo[] getThreadInfo(long[] ids) {
        return getThreadInfo(ids, 0);
    }

    public ThreadInfo[] getThreadInfo(long[] ids, int maxDepth) {
        if (ids == null) {
            throw new NullPointerException("Null ids parameter.");
        }

        if (maxDepth < 0) {
            throw new IllegalArgumentException(
                "Invalid maxDepth parameter: " + maxDepth);
        }

        ManagementFactory.checkMonitorAccess();

        ThreadInfo[] infos = new ThreadInfo[ids.length];
        if (maxDepth == Integer.MAX_VALUE) {
            getThreadInfo0(ids, -1, infos);
        } else {
            getThreadInfo0(ids, maxDepth, infos);
        }
        return infos;
    }


    public void setThreadContentionMonitoringEnabled(boolean enable) {
        if (!isThreadContentionMonitoringSupported()) {
            throw new UnsupportedOperationException(
                "Thread contention monitoring is not supported");
        }

        ManagementFactory.checkControlAccess();

        synchronized (this) {
            if (contentionMonitoringEnabled != enable) {
                if (enable) {
                    // if reeabled, reset contention time statistics 
                    // for all threads
                    resetContentionTimes0(0);
                }

                // update the VM of the state change
                setThreadContentionMonitoringEnabled0(enable);

                contentionMonitoringEnabled = enable;
            }
        }
    }

    public long getCurrentThreadCpuTime() {
        // check if Thread CPU time measurement is supported.
        if (!isCurrentThreadCpuTimeSupported()) {
            throw new UnsupportedOperationException(
                "Current thread CPU time measurement is not supported.");
        }

        if (!isThreadCpuTimeEnabled()) {
            return -1;
        }

        return getThreadTotalCpuTime0(0);
    }

    public long getThreadCpuTime(long id) {
        // check if Thread CPU time measurement is supported.
        if (!isThreadCpuTimeSupported() &&
            !isCurrentThreadCpuTimeSupported()) {
            throw new UnsupportedOperationException(
                "Thread CPU Time Measurement is not supported.");
        }

        if (!isThreadCpuTimeSupported()) {
            // support current thread only
            if (id != Thread.currentThread().getId()) {
                throw new UnsupportedOperationException(
                    "Thread CPU Time Measurement is only supported" +
                    " for the current thread.");
            }
        }

        if (id <= 0) {
            throw new IllegalArgumentException(
                "Invalid thread ID parameter: " + id);
        }

        if (!isThreadCpuTimeEnabled()) {
            return -1;
        }

        if (id == Thread.currentThread().getId()) {
            // current thread
            return getThreadTotalCpuTime0(0);
        } else {
            return getThreadTotalCpuTime0(id);
        }
    }

    public long getCurrentThreadUserTime() {
        // check if Thread CPU time measurement is supported.
        if (!isCurrentThreadCpuTimeSupported()) {
            throw new UnsupportedOperationException(
                "Current thread CPU time measurement is not supported.");
        }

        if (!isThreadCpuTimeEnabled()) {
            return -1;
        }

        return getThreadUserCpuTime0(0);
    }

    public long getThreadUserTime(long id) {
        // check if Thread CPU time measurement is supported.
        if (!isThreadCpuTimeSupported() &&
            !isCurrentThreadCpuTimeSupported()) {
            throw new UnsupportedOperationException(
                "Thread CPU time measurement is not supported.");
        }

        if (!isThreadCpuTimeSupported()) {
            // support current thread only
            if (id != Thread.currentThread().getId()) {
                throw new UnsupportedOperationException(
                    "Thread CPU time measurement is only supported" +
                    " for the current thread.");
            }
        }

        if (id <= 0) {
            throw new IllegalArgumentException(
                "Invalid thread ID parameter: " + id);
        }

        if (!isThreadCpuTimeEnabled()) {
            return -1;
        }

        if (id == Thread.currentThread().getId()) {
            // current thread
           return getThreadUserCpuTime0(0);
        } else {
           return getThreadUserCpuTime0(id);
        }
    }


    public void setThreadCpuTimeEnabled(boolean enable) {
        if (!isThreadCpuTimeSupported() &&
            !isCurrentThreadCpuTimeSupported()) {
            throw new UnsupportedOperationException(
                "Thread CPU time measurement is not supported");
        }

        ManagementFactory.checkControlAccess();
        synchronized (this) {
            if (cpuTimeEnabled != enable) {
                // update VM of the state change
                setThreadCpuTimeEnabled0(enable);
                cpuTimeEnabled = enable;
            }
        }
    }

    public long[] findMonitorDeadlockedThreads() {
        ManagementFactory.checkMonitorAccess();

        Thread[] threads = findMonitorDeadlockedThreads0();
        if (threads == null) {
            return null;
        }

        long[] ids = new long[threads.length];
        for (int i = 0; i < threads.length; i++) {
            Thread t = threads[i];
            ids[i] = t.getId();
        }
        return ids;
    }

    public void resetPeakThreadCount() {
        ManagementFactory.checkControlAccess();
        resetPeakThreadCount0();
    }

    // VM support where maxDepth == -1 to request entire stack dump
    private static native Thread[] getThreads();
    private static native void getThreadInfo0(long[] ids, 
                                              int maxDepth,
                                              ThreadInfo[] result);
    private static native long getThreadTotalCpuTime0(long id);
    private static native long getThreadUserCpuTime0(long id);
    private static native void setThreadCpuTimeEnabled0(boolean enable);
    private static native void setThreadContentionMonitoringEnabled0(boolean enable);
    private static native Thread[] findMonitorDeadlockedThreads0();
    private static native void resetPeakThreadCount0();

    // tid == 0 to reset contention times for all threads
    private static native void resetContentionTimes0(long tid);
}
