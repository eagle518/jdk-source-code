/*
 * @(#)OSMBeanFactory.java	1.6 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.management;

import java.lang.management.OperatingSystemMXBean;
import sun.management.VMManagement;

/**
 * Operating system dependent MBean factory.
 * <p>
 * <b>WARNING:</b> While this class is public, it should not be treated as
 * public API and its API may change in incompatable ways between dot dot
 * releases and even patch releases. You should not rely on this class.
 */
public class OSMBeanFactory {
    /* static factory class */
    private OSMBeanFactory() {};

    private static UnixOperatingSystem osMBean = null;

    public static synchronized OperatingSystemMXBean 
        getOperatingSystemMXBean(VMManagement jvm) {

        if (osMBean == null) {
            osMBean = new UnixOperatingSystem(jvm);
        }
        return (OperatingSystemMXBean) osMBean;
    }
}
