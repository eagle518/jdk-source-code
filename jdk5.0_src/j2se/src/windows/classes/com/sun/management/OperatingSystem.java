/*
 * @(#)OperatingSystem.java	1.3 04/04/18
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.management;

import sun.management.VMManagement;

/**
 * Implementation class for the operating system.
 * Standard and committed hotspot-specific metrics if any.
 *
 * ManagementFactory.getOperatingSystemMXBean() returns an instance
 * of this class.
 */
class OperatingSystem 
    extends    sun.management.OperatingSystemImpl 
    implements OperatingSystemMXBean {

    // psapiLock is a lock to make sure only one thread loading
    // PSAPI DLL.
    private static Object psapiLock = new Object();

    OperatingSystem(VMManagement vm) {
        super(vm, OperatingSystemMXBean.class);
    }

    public long getCommittedVirtualMemorySize() {
        synchronized (psapiLock) {
            return getCommittedVirtualMemorySize0();
        }
    }
    private native long getCommittedVirtualMemorySize0();

    public native long getTotalSwapSpaceSize();
    public native long getFreeSwapSpaceSize();
    public native long getProcessCpuTime();
    public native long getFreePhysicalMemorySize();
    public native long getTotalPhysicalMemorySize();

    static {
        initialize();
    }
    private static native void initialize();
}
