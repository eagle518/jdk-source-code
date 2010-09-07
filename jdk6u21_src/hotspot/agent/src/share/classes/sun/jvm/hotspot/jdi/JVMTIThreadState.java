/*
 * Copyright (c) 2004, Oracle and/or its affiliates. All rights reserved.
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

// from JVMTI specification - refer to jvmti.xml
public interface JVMTIThreadState {
    public static final int JVMTI_THREAD_STATE_ALIVE = 0x0001;
    public static final int JVMTI_THREAD_STATE_TERMINATED = 0x0002;
    public static final int JVMTI_THREAD_STATE_RUNNABLE = 0x0004;
    public static final int JVMTI_THREAD_STATE_WAITING = 0x0008;
    public static final int JVMTI_THREAD_STATE_WAITING_INDEFINITELY = 0x0010;
    public static final int JVMTI_THREAD_STATE_WAITING_WITH_TIMEOUT = 0x0020;
    public static final int JVMTI_THREAD_STATE_SLEEPING = 0x0040;
    public static final int JVMTI_THREAD_STATE_WAITING_FOR_NOTIFICATION = 0x0080;
    public static final int JVMTI_THREAD_STATE_IN_OBJECT_WAIT = 0x0100;
    public static final int JVMTI_THREAD_STATE_PARKED = 0x0200;
    public static final int JVMTI_THREAD_STATE_BLOCKED_ON_MONITOR_ENTER = 0x0400;
    public static final int JVMTI_THREAD_STATE_SUSPENDED = 0x100000;
    public static final int JVMTI_THREAD_STATE_INTERRUPTED = 0x200000;
    public static final int JVMTI_THREAD_STATE_IN_NATIVE = 0x400000;
}
