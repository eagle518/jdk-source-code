/*
 * @(#)HotspotMemoryMBean.java	1.3 04/04/18
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.management;

import sun.management.counter.Counter;
/**
 * Hotspot internal management interface for the compilation system.
 */
public interface HotspotMemoryMBean {

    /**
     * Returns a list of internal counters maintained in the Java
     * virtual machine for the memory system.
     *
     * @return a list of internal counters maintained in the VM
     * for the memory system.
     */
    public java.util.List<Counter> getInternalMemoryCounters();
}
