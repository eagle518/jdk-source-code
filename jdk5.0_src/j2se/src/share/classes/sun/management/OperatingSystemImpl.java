/*
 * @(#)OperatingSystemImpl.java	1.6 04/04/18
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.management;

import java.lang.management.OperatingSystemMXBean;

/**
 * Implementation class for the operating system.
 * Standard and committed hotspot-specific metrics if any.
 *
 * ManagementFactory.getOperatingSystemMXBean() returns an instance
 * of this class.
 */
public class OperatingSystemImpl extends MXBeanSupport
                                 implements OperatingSystemMXBean {

    private final VMManagement jvm;

    /**
     * Constructor of OperatingSystemImpl class.
     */
    protected OperatingSystemImpl(VMManagement vm, Class mxbeanInterface) {
        super(mxbeanInterface);
        this.jvm = vm;
    }

    public String getName() {
        return jvm.getOsName();
    }

    public String getArch() {
        return jvm.getOsArch();
    }

    public String getVersion() {
        return jvm.getOsVersion();
    }

    public int getAvailableProcessors() {
        return jvm.getAvailableProcessors();
    }
}

