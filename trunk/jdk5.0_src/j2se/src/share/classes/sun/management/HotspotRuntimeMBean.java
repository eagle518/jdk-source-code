/*
 * @(#)HotspotRuntimeMBean.java	1.4 04/04/18
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.management;

import sun.management.counter.Counter;

/**
 * Hotspot internal management interface for the runtime system.
 *
 * This management interface is internal and uncommitted
 * and subject to change without notice.
 */
public interface HotspotRuntimeMBean {

    /**
     * Returns the number of safepoints taken place since the Java
     * virtual machine started.
     *
     * @return the number of safepoints taken place since the Java
     * virtual machine started.
     */
    public long getSafepointCount();

    /**
     * Returns the accumulated time spent at safepoints in milliseconds.
     * This is the accumulated elapsed time that the application has 
     * been stopped for safepoint operations.
     *
     * @return the accumulated time spent at safepoints in milliseconds.
     */
    public long getTotalSafepointTime();

    /**
     * Returns the accumulated time spent getting to safepoints in milliseconds.
     *
     * @return the accumulated time spent getting to safepoints in milliseconds.
     */
    public long getSafepointSyncTime();

    /**
     * Returns a list of all VM internal flags.
     *
     * @return a list of {@link Flag} objects, each for an VM internal flag.
     *
     * @throws  java.security.SecurityException
     *     if a security manager exists and the caller does not have
     *     ManagementPermission("monitor").
     */
    public java.util.List<Flag> getInternalFlags();

    /**
     * Returns a list of the names of all VM internal flags.
     *
     * @return a list of {@link String} objects, each is 
     * an VM internal flag name.
     *
     * @throws  java.security.SecurityException
     *     if a security manager exists and the caller does not have
     *     ManagementPermission("monitor").
     */
    public java.util.List<String> getInternalFlagNames();

    /**
     * Returns a {@link Flag} object for the given name.  This method
     * returns <tt>null</tt> if no flag exists for the given name.
     *
     * @return a {@link Flag} object if the VM flag of the given name exists;  
     * otherwise <tt>null</tt>.
     *
     * @throws  java.security.SecurityException
     *     if a security manager exists and the caller does not have
     *     ManagementPermission("monitor").
     */
    public Flag getFlag(String name);

    /**
     * Returns a list of internal counters maintained in the Java
     * virtual machine for the runtime system.
     *
     * @return a <tt>List</tt> of internal counters maintained in the VM
     * for the runtime system.
     */
    public java.util.List<Counter> getInternalRuntimeCounters();
}
