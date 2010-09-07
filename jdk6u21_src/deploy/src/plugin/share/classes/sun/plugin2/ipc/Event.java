/*
 * @(#)Event.java	1.2 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.ipc;

import java.util.*;

/** An Event object represents an IPC construct that can be waited on
    for signalling by another entity. If a consumer is waiting on it
    when it is signalled, the signalled state is cleared and the
    consumer is released from its wait. If a consumer is not waiting
    on it when it is signalled, the signalled state persists, and any
    subsequent wait returns immediately. */

public abstract class Event {
    /** Waits up to the specified number of milliseconds for a signal
        on this Event object. A zero number of milliseconds indicates
        that the caller should wait indefinitely. */
    public abstract void waitForSignal(long millisToWait);

    /** Signals this event object. */
    public abstract void signal();

    /** Gets the parameters which need to be passed to the IPCFactory
        to create the child process's view of this Event object. */
    public abstract Map getChildProcessParameters();

    /** Disposes any operating system-level resources associated with
        this event object. */
    public abstract void dispose();
}
