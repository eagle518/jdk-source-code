/*
 * Copyright (c) 2005, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

package sun.jvm.hotspot.jdi;

import com.sun.jdi.*;

// FIXME: This class should implement com.sun.jdi.MonitorInfo.
//        So fix this when hotspot is started to build with
//        jdk1.6.
public class MonitorInfoImpl extends MirrorImpl {

    /* Once false, monitorInfo should not be used.
     * access synchronized on (vm.state())
     */
    private boolean isValid = true;

    ObjectReference monitor;
    ThreadReference thread;
    int  stack_depth;

    MonitorInfoImpl(VirtualMachine vm, ObjectReference mon,
                    ThreadReference thread, int dpth) {
        super(vm);
        this.monitor = mon;
        this.thread = thread;
        this.stack_depth = dpth;
    }

    private void validateMonitorInfo() {
        if (!isValid) {
            throw new InvalidStackFrameException("Thread has been resumed");
        }
    }

    public ObjectReference monitor() {
        validateMonitorInfo();
        return monitor;
    }

    public int stackDepth() {
        validateMonitorInfo();
        return stack_depth;
    }

    public ThreadReference thread() {
        validateMonitorInfo();
        return thread;
    }
}
