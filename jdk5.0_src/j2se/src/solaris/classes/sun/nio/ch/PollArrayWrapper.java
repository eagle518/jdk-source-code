/*
 * @(#)PollArrayWrapper.java	1.12 04/01/12
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.nio.ch;

import sun.misc.*;


/**
 * Manipulates a native array of pollfd structs on Solaris:
 *
 * typedef struct pollfd {
 *    int fd;
 *    short events;
 *    short revents;
 * } pollfd_t;
 *
 * @author Mike McCloskey
 * @version 1.12, 04/01/12
 * @since 1.4
 */

class PollArrayWrapper extends AbstractPollArrayWrapper {

    static final short POLLCONN = POLLOUT;

    // File descriptor to write for interrupt
    int interruptFD;

    PollArrayWrapper(int newSize) {
        newSize = (newSize + 1) * SIZE_POLLFD;
        pollArray = new AllocatedNativeObject(newSize, false);
        pollArrayAddress = pollArray.address();
        totalChannels = 1;
    }

    void initInterrupt(int fd0, int fd1) {
        interruptFD = fd1;
        putDescriptor(0, fd0);
        putEventOps(0, POLLIN);
        putReventOps(0, 0);
    }

    void release(int i) {
        return;
    }

    void free() {
        pollArray.free();
    }

    /**
     * Prepare another pollfd struct for use.
     */
    void addEntry(SelChImpl sc) {
        putDescriptor(totalChannels, IOUtil.fdVal(sc.getFD()));
        putEventOps(totalChannels, 0);
        putReventOps(totalChannels, 0);
        totalChannels++;
    }

    /**
     * Writes the pollfd entry from the source wrapper at the source index
     * over the entry in the target wrapper at the target index. The source
     * array remains unchanged unless the source array and the target are
     * the same array.
     */
    static void replaceEntry(PollArrayWrapper source, int sindex,
                      PollArrayWrapper target, int tindex) {
        target.putDescriptor(tindex, source.getDescriptor(sindex));
        target.putEventOps(tindex, source.getEventOps(sindex));
        target.putReventOps(tindex, source.getReventOps(sindex));
    }

    /**
     * Grows the pollfd array to a size that will accommodate newSize
     * pollfd entries. This method does no checking of the newSize
     * to determine if it is in fact bigger than the old size: it
     * always reallocates an array of the new size.
     */
    void grow(int newSize) {
        // create new array
        PollArrayWrapper temp = new PollArrayWrapper(newSize);

        // Copy over existing entries
        for (int i=0; i<totalChannels; i++)
            replaceEntry(this, i, temp, i);

        // Swap new array into pollArray field
        pollArray.free();
        pollArray = temp.pollArray;
        pollArrayAddress = pollArray.address();
    }

    int poll(int numfds, int offset, long timeout) {
        return poll0(pollArrayAddress + (offset * SIZE_POLLFD),
                     numfds, timeout);
    }

    public void interrupt() {
        interrupt(interruptFD);
    }

    private native int poll0(long pollAddress, int numfds, long timeout);

    private static native void interrupt(int fd);

}
