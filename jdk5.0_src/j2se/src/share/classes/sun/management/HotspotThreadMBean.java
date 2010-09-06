/*
 * @(#)HotspotThreadMBean.java	1.3 04/04/18
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.management;

import sun.management.counter.Counter;

/**
 * Hotspot internal management interface for the thread system.
 */
public interface HotspotThreadMBean {

    /**
     * Returns the current number of VM internal threads.
     *
     * @return the current number of VM internal threads.
     */
    public int getInternalThreadCount();

    /**
     * Returns a <tt>Map</tt> of the name of all VM internal threads
     * to the thread CPU time in nanoseconds.  The returned value is 
     * of nanoseconds precision but not necessarily nanoseconds accuracy.
     * <p>
     *
     * @return a <tt>Map</tt> object of the name of all VM internal threads
     * to the thread CPU time in nanoseconds.  
     *
     * @throws java.lang.UnsupportedOperationException if the Java virtual
     * machine does not support CPU time measurement.
     * 
     * @see java.lang.management.ThreadMBean#isThreadCpuTimeSupported
     */
    public java.util.Map<String,Long> getInternalThreadCpuTimes();

    /**
     * Returns a list of internal counters maintained in the Java
     * virtual machine for the thread system.
     *
     * @return a list of internal counters maintained in the VM
     * for the thread system.
     */
    public java.util.List<Counter> getInternalThreadingCounters();
}
