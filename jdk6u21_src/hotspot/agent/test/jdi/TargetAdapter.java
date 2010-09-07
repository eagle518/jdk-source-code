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
 * Base TargetListener implementation
 */
public class TargetAdapter implements TargetListener {
    boolean shouldRemoveListener = false;

    public void removeThisListener() {
        shouldRemoveListener = true;
    }

    public boolean shouldRemoveListener() {
        return shouldRemoveListener;
    }

    public void eventSetReceived(EventSet set) {}
    public void eventSetComplete(EventSet set) {}
    public void eventReceived(Event event) {}
    public void breakpointReached(BreakpointEvent event) {}
    public void exceptionThrown(ExceptionEvent event) {}
    public void stepCompleted(StepEvent event) {}
    public void classPrepared(ClassPrepareEvent event) {}
    public void classUnloaded(ClassUnloadEvent event) {}
    public void methodEntered(MethodEntryEvent event) {}
    public void methodExited(MethodExitEvent event) {}
    public void fieldAccessed(AccessWatchpointEvent event) {}
    public void fieldModified(ModificationWatchpointEvent event) {}
    public void threadStarted(ThreadStartEvent event) {}
    public void threadDied(ThreadDeathEvent event) {}
    public void vmStarted(VMStartEvent event) {}
    public void vmDied(VMDeathEvent event) {}
    public void vmDisconnected(VMDisconnectEvent event) {}
}
