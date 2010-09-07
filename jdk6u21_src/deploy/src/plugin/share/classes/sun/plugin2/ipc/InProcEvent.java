/*
 * @(#)InProcEvent.java	1.2 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.ipc;

import java.util.*;

/** An implementation of the Event class which can only be used within
    one process. Mainly for testing purposes. */

public class InProcEvent extends Event {
    private volatile boolean signalled;

    public synchronized void waitForSignal(long millisToWait) {
        // This is only approximate and basically assumes no spurious
        // wakeups
        if (signalled) {
            signalled = false;
            return;
        }

        try {
            wait(millisToWait);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        signalled = false; // Consume the signal
    }

    public synchronized void signal() {
        signalled = true;
        notifyAll();
    }

    public Map getChildProcessParameters() {
        throw new RuntimeException("Should not call this");
    }

    public void dispose() {}
}
