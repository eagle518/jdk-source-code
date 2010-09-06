/*
 * @(#)AbstractPollArrayWrapper.java	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.nio.ch;

import sun.misc.*;


/**
 * Manipulates a native array of pollfd structs.
 *
 * @author Mike McCloskey
 * @version 1.5, 03/12/19
 * @since 1.4
 */

abstract class AbstractPollArrayWrapper {
 
    // Event masks
    static final short POLLIN       = 0x0001;
    static final short POLLOUT	    = 0x0004;
    static final short POLLERR	    = 0x0008;
    static final short POLLHUP	    = 0x0010;
    static final short POLLNVAL	    = 0x0020;
    static final short POLLREMOVE   = 0x0800;

    // Miscellaneous constants
    static final short SIZE_POLLFD   = 8;
    static final short FD_OFFSET     = 0;
    static final short EVENT_OFFSET  = 4;
    static final short REVENT_OFFSET = 6;

    // The poll fd array
    protected AllocatedNativeObject pollArray;

    // Number of valid entries in the pollArray
    protected int totalChannels = 0;

    // Base address of the native pollArray
    protected long pollArrayAddress;

    // Access methods for fd structures
    int getEventOps(int i) {
        int offset = SIZE_POLLFD * i + EVENT_OFFSET;
        return pollArray.getShort(offset);
    }

    int getReventOps(int i) {
        int offset = SIZE_POLLFD * i + REVENT_OFFSET;
        return pollArray.getShort(offset);
    }

    int getDescriptor(int i) {
        int offset = SIZE_POLLFD * i + FD_OFFSET;
        return pollArray.getInt(offset);
    }

    void putEventOps(int i, int event) {
        int offset = SIZE_POLLFD * i + EVENT_OFFSET;
        pollArray.putShort(offset, (short)event);
    }

    void putReventOps(int i, int revent) {
        int offset = SIZE_POLLFD * i + REVENT_OFFSET;
        pollArray.putShort(offset, (short)revent);
    }

    void putDescriptor(int i, int fd) {
        int offset = SIZE_POLLFD * i + FD_OFFSET;
        pollArray.putInt(offset, fd);
    }

}
