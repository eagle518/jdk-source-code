/*
 * @(#)OperatingSystemMXBean.java	1.5 04/05/04
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.management;

/**
 * Platform-specific management interface for the operating system 
 * on which the Java virtual machine is running.
 *
 * <p>
 * The <tt>OperatingSystemMXBean</tt> object returned by
 * <a href="../../../api/java/lang/management/ManagementFactory#getOperatingSystemMXBean">
 * java.lang.management.ManagementFactory.getOperatingSystemMXBean</a>
 * is an instance of the implementation class of this interface
 * or {@link UnixOperatingSystemMXBean} interface depending on
 * its underlying operating system.
 * 
 * @author  Mandy Chung
 * @version 1.5, 05/04/04
 * @since   1.5
 */
public interface OperatingSystemMXBean extends
    java.lang.management.OperatingSystemMXBean {

    /**
     * Returns the amount of virtual memory that is guaranteed to
     * be available to the running process in bytes,
     * or <tt>-1</tt> if this operation is not supported.
     *
     * @return the amount of virtual memory that is guaranteed to 
     * be available to the running process in bytes,
     * or <tt>-1</tt> if this operation is not supported.
     */
    public long getCommittedVirtualMemorySize();

    /**
     * Returns the total amount of swap space in bytes.
     *
     * @return the total amount of swap space in bytes.
     */
    public long getTotalSwapSpaceSize();

    /**
     * Returns the amount of free swap space in bytes.
     *
     * @return the amount of free swap space in bytes.
     */
    public long getFreeSwapSpaceSize();

    /**
     * Returns the CPU time used by the process on which the Java
     * virtual machine is running in nanoseconds.  The returned value
     * is of nanoseconds precision but not necessarily nanoseconds
     * accuracy.  This method returns <tt>-1</tt> if the 
     * the platform does not support this operation.
     *
     * @return the CPU time used by the process in nanoseconds,
     * or <tt>-1</tt> if this operation is not supported.
     */
    public long getProcessCpuTime();

    /**
     * Returns the amount of free physical memory in bytes.
     *
     * @return the amount of free physical memory in bytes.
     */
    public long getFreePhysicalMemorySize();
 
    /**
     * Returns the total amount of physical memory in bytes.
     *
     * @return the total amount of physical memory in  bytes.
     */
    public long getTotalPhysicalMemorySize();
}
