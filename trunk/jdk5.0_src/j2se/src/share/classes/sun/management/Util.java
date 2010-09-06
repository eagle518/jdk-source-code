/*
 * @(#)Util.java	1.10 04/04/18
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.management;

import java.lang.management.*;
import static java.lang.management.ManagementFactory.*;
import java.util.List;

class Util {
    static String getMBeanObjectName(MemoryPoolMXBean pool) {
        return MEMORY_POOL_MXBEAN_DOMAIN_TYPE +
            ",name=" + pool.getName();
    }

    static String getMBeanObjectName(MemoryManagerMXBean mgr) {
        if (mgr instanceof GarbageCollectorMXBean) {
            return getMBeanObjectName((GarbageCollectorMXBean) mgr);
        } else {
            return MEMORY_MANAGER_MXBEAN_DOMAIN_TYPE +
                ",name=" + mgr.getName();
        }
    }

    static String getMBeanObjectName(GarbageCollectorMXBean gc) {
        return GARBAGE_COLLECTOR_MXBEAN_DOMAIN_TYPE +
            ",name=" + gc.getName();
    }

    static RuntimeException newException(Exception e) {
        RuntimeException e1 = new RuntimeException(e.getMessage());
        e1.initCause(e);
        return e1;
    }

    static InternalError newInternalError(Exception e) {
        InternalError e1 = new InternalError(e.getMessage());
        e1.initCause(e);
        return e1;
    }
    static AssertionError newAssertionError(Exception e) {
        AssertionError e1 = new AssertionError(e.getMessage());
        e1.initCause(e);
        return e1;
    }

    private static String[] EMPTY_STRING_ARRAY = new String[0];
    static String[] toStringArray(List<String> list) {
        return (String[]) list.toArray(EMPTY_STRING_ARRAY);
    }
}
