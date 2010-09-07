/*
 * @(#)PollSelectorImpl.java	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.nio.ch;

import java.io.IOException;
import java.nio.channels.*;
import java.nio.channels.spi.*;
import java.util.*;
import sun.misc.*;


/**
 * An implementation of Selector for Solaris.
 */

class PollSelectorImpl
    extends AbstractPollSelectorImpl
{

    // File descriptors used for interrupt
    private int fd0;
    private int fd1;
    
    // Lock for interrupt triggering and clearing
    private Object interruptLock = new Object();
    private boolean interruptTriggered = false;

    /**
     * Package private constructor called by factory method in
     * the abstract superclass Selector.
     */
    PollSelectorImpl(SelectorProvider sp) {
        super(sp, 1, 1);
        int[] fdes = new int[2];
        IOUtil.initPipe(fdes, false);
        fd0 = fdes[0];
        fd1 = fdes[1];
        pollWrapper = new PollArrayWrapper(INIT_CAP);
        pollWrapper.initInterrupt(fd0, fd1);
        channelArray = new SelectionKeyImpl[INIT_CAP];
    }

    protected int doSelect(long timeout)
        throws IOException
    {
        if (channelArray == null)
            throw new ClosedSelectorException();
        processDeregisterQueue();
        try {
            begin();
            pollWrapper.poll(totalChannels, 0, timeout);
        } finally {
            end();
        }
        processDeregisterQueue();
        int numKeysUpdated = updateSelectedKeys();
        if (pollWrapper.getReventOps(0) != 0) {
            // Clear the wakeup pipe
            pollWrapper.putReventOps(0, 0);
            synchronized (interruptLock) {
                IOUtil.drain(fd0);
                interruptTriggered = false;
            }
        }
        return numKeysUpdated;
    }

    protected void implCloseInterrupt() throws IOException {
        FileDispatcher.closeIntFD(fd0);
        FileDispatcher.closeIntFD(fd1);
        fd0 = -1;
        fd1 = -1;
        pollWrapper.release(0);
    }

    public Selector wakeup() {
        synchronized (interruptLock) {
            if (!interruptTriggered) {
                pollWrapper.interrupt();
                interruptTriggered = true;
            }
        }
	return this;
    }

}
