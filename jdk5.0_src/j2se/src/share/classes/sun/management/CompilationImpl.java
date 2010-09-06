/*
 * @(#)CompilationImpl.java	1.7 04/04/18
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.management;

import java.lang.management.CompilationMXBean;

/**
 * Implementation class for the compilation subsystem.
 * Standard and committed hotspot-specific metrics if any.
 *
 * ManagementFactory.getCompilationMXBean() returns an instance
 * of this class.
 */
class CompilationImpl extends MXBeanSupport implements CompilationMXBean {

    private final VMManagement jvm;
    private final String name;

    /**
     * Constructor of CompilationImpl class.
     */
    CompilationImpl(VMManagement vm) {
        super(CompilationMXBean.class);
        this.jvm = vm;
        this.name = jvm.getCompilerName();
        if (name == null) {
            throw new InternalError("Null compiler name");
        }
    }

    public java.lang.String getName() {
        return name;
    }

    public boolean isCompilationTimeMonitoringSupported() {
        return jvm.isCompilationTimeMonitoringSupported();
    }

    public long getTotalCompilationTime() {
        if (!isCompilationTimeMonitoringSupported()) {
            throw new UnsupportedOperationException(
                "Compilation time monitoring is not supported.");
        }

        return jvm.getTotalCompileTime();
    }

}
