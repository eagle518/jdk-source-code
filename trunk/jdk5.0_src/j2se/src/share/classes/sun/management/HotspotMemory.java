/*
 * @(#)HotspotMemory.java	1.4 04/04/18
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.management;

import java.util.List;
import sun.management.counter.*;

/**
 * Implementation class of HotspotMemoryMBean interface.
 *
 * Internal, uncommitted management interface for Hotspot memory 
 * system.
 *
 */
class HotspotMemory
    implements HotspotMemoryMBean {

    private VMManagement jvm;

    /**
     * Constructor of HotspotRuntime class.
     */ 
    HotspotMemory(VMManagement vm) {
        jvm = vm;
    }

    // Performance counter support
    private static final String JAVA_GC    = "java.gc.";
    private static final String COM_SUN_GC = "com.sun.gc.";
    private static final String SUN_GC     = "sun.gc.";
    private static final String GC_COUNTER_NAME_PATTERN =
        JAVA_GC + "|" + COM_SUN_GC + "|" + SUN_GC;

    public java.util.List getInternalMemoryCounters() {
        return jvm.getInternalCounters(GC_COUNTER_NAME_PATTERN); 
    } 
}
