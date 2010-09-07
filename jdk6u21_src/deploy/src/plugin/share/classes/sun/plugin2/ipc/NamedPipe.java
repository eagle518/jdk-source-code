/*
 * @(#)NamedPipe.java	1.3 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.ipc;

import java.io.*;
import java.nio.*;
import java.nio.channels.*;
import java.util.*;

/** Represents a named pipe. */

public abstract class NamedPipe implements ReadableByteChannel, WritableByteChannel {
    public abstract int read(ByteBuffer dest) throws IOException;
    public abstract int write(ByteBuffer src) throws IOException;
    public abstract void close() throws IOException;
    public abstract boolean isOpen();
    public abstract String toString();

    /** Gets the parameters which need to be passed to the IPCFactory
        to create the child process's view of this Event object. */
    public abstract Map getChildProcessParameters();
}
