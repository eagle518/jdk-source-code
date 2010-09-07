/*
 * @(#)SocketDispatcher.java	1.9 03/12/19
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

class SocketDispatcher extends NativeDispatcher
{

    int read(FileDescriptor fd, long address, int len) throws IOException {
        return FileDispatcher.read0(fd, address, len);
    }

    long readv(FileDescriptor fd, long address, int len) throws IOException {
        return FileDispatcher.readv0(fd, address, len);
    }

    int write(FileDescriptor fd, long address, int len) throws IOException {
        return FileDispatcher.write0(fd, address, len);
    }

    long writev(FileDescriptor fd, long address, int len) throws IOException {
        return FileDispatcher.writev0(fd, address, len);
    }

    void close(FileDescriptor fd) throws IOException {
        FileDispatcher.close0(fd);
    }
}
