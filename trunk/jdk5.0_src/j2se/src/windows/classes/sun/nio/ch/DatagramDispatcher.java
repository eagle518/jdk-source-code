/*
 * @(#)DatagramDispatcher.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.nio.ch;

import java.io.*;
import java.net.*;

/**
 * Allows different platforms to call different native methods
 * for read and write operations.
 */

class DatagramDispatcher extends NativeDispatcher
{
    static {
        Util.load();
    }

    int read(FileDescriptor fd, long address, int len) throws IOException {
        return read0(fd, address, len);
    }

    long readv(FileDescriptor fd, long address, int len) throws IOException {
        return readv0(fd, address, len);
    }

    int write(FileDescriptor fd, long address, int len) throws IOException {
        return write0(fd, address, len);
    }

    long writev(FileDescriptor fd, long address, int len) throws IOException {
        return writev0(fd, address, len);
    }

    void close(FileDescriptor fd) throws IOException {
        SocketDispatcher.close0(fd);
    }

    static native int read0(FileDescriptor fd, long address, int len)
	throws IOException;

    static native long readv0(FileDescriptor fd, long address, int len)
	throws IOException;

    static native int write0(FileDescriptor fd, long address, int len)
	throws IOException;

    static native long writev0(FileDescriptor fd, long address, int len)
	throws IOException;
}
