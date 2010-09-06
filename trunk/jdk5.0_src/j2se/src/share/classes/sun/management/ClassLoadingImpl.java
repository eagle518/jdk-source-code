/*
 * @(#)ClassLoadingImpl.java	1.7 04/04/18
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.management;

import java.lang.management.ClassLoadingMXBean;

/**
 * Implementation class for the class loading subsystem. 
 * Standard and committed hotspot-specific metrics if any.
 *
 * ManagementFactory.getClassLoadingMXBean() returns an instance
 * of this class.
 */
class ClassLoadingImpl extends MXBeanSupport implements ClassLoadingMXBean {

    private final VMManagement jvm;

    /**
     * Constructor of ClassLoadingImpl class.
     */ 
    ClassLoadingImpl(VMManagement vm) {
        super(ClassLoadingMXBean.class);
        this.jvm = vm;
    }

    public long getTotalLoadedClassCount() {
        return jvm.getTotalClassCount();
    }

    public int getLoadedClassCount() {
        return jvm.getLoadedClassCount();
    }

    public long getUnloadedClassCount() {
        return jvm.getUnloadedClassCount();
    }

    public boolean isVerbose() {
        return jvm.getVerboseClass();
    }

    public void setVerbose(boolean value) {
        ManagementFactory.checkControlAccess();

        setVerboseClass(value);
    }
    native static void setVerboseClass(boolean value);
}
