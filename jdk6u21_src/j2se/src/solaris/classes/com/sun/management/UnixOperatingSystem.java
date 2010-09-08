/*
 * @(#)UnixOperatingSystem.java	1.6 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
class UnixOperatingSystem 
    extends    sun.management.OperatingSystemImpl 
    implements UnixOperatingSystemMXBean {

    UnixOperatingSystem(VMManagement vm) {
        super(vm);
    }

    public native long getCommittedVirtualMemorySize();
    public native long getTotalSwapSpaceSize();
    public native long getFreeSwapSpaceSize();
    public native long getProcessCpuTime();
    public native long getFreePhysicalMemorySize();
    public native long getTotalPhysicalMemorySize();
    public native long getOpenFileDescriptorCount();
    public native long getMaxFileDescriptorCount();

    static {
        initialize();
    }
    private static native void initialize();
}
