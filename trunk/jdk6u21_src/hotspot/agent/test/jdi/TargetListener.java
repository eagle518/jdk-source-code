/*
 * Copyright (c) 2002, Oracle and/or its affiliates. All rights reserved.
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

import com.sun.jdi.event.*;

/**
 * Event listener framework
 */
public interface TargetListener {
    boolean shouldRemoveListener();

    void eventSetReceived(EventSet set);
    void eventSetComplete(EventSet set);
    void eventReceived(Event event);
    void breakpointReached(BreakpointEvent event);
    void exceptionThrown(ExceptionEvent event);
    void stepCompleted(StepEvent event);
    void classPrepared(ClassPrepareEvent event);
    void classUnloaded(ClassUnloadEvent event);
    void methodEntered(MethodEntryEvent event);
    void methodExited(MethodExitEvent event);
    void fieldAccessed(AccessWatchpointEvent event);
    void fieldModified(ModificationWatchpointEvent event);
    void threadStarted(ThreadStartEvent event);
    void threadDied(ThreadDeathEvent event);
    void vmStarted(VMStartEvent event);
    void vmDied(VMDeathEvent event);
    void vmDisconnected(VMDisconnectEvent event);
}
