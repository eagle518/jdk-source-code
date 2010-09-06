/*
 * @(#)NativeDispatcher.java	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.nio.ch;

import java.io.*;

/**
 * Allows different platforms to call different native methods
 * for read and write operations.
 */

abstract class NativeDispatcher
{

    abstract int read(FileDescriptor fd, long address, int len)
	throws IOException;

    int pread(FileDescriptor fd, long address, int len,
                             long position, Object lock) throws IOException
    {
        throw new IOException("Operation Unsupported");
    }

    abstract long readv(FileDescriptor fd, long address, int len)
	throws IOException;

    abstract int write(FileDescriptor fd, long address, int len)
	throws IOException;

    int pwrite(FileDescriptor fd, long address, int len,
                             long position, Object lock) throws IOException
    {
        throw new IOException("Operation Unsupported");
    }

    abstract long writev(FileDescriptor fd, long address, int len)
	throws IOException;

    abstract void close(FileDescriptor fd) throws IOException;

    // Prepare the given fd for closing by duping it to a known internal fd
    // that's already closed.  This is necessary on some operating systems
    // (Solaris and Linux) to prevent fd recycling.
    //
    void preClose(FileDescriptor fd) throws IOException {
	// Do nothing by default; this is only needed on Unix
    }

}
