/*
 * @(#)UnixOperatingSystemMXBean.java	1.4 04/04/18
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.management;

/**
 * Platform-specific management interface for the Unix 
 * operating system on which the Java virtual machine is running.
 * 
 * @author  Mandy Chung
 * @version 1.4, 04/18/04
 * @since   1.5
 */
public interface UnixOperatingSystemMXBean extends
    com.sun.management.OperatingSystemMXBean {

    /**
     * Returns the number of open file descriptors.
     *
     * @return the number of open file descriptors. 
     */
    public long getOpenFileDescriptorCount();

    /**
     * Returns the maximum number of file descriptors.
     *
     * @return the maximum number of file descriptors. 
     */
    public long getMaxFileDescriptorCount();
}
