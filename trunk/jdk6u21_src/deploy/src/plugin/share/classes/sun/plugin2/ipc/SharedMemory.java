/*
 * @(#)SharedMemory.java	1.2 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.ipc;

import java.nio.*;
import java.util.*;

/** Represents a shared memory segment. */

public abstract class SharedMemory {
    protected SharedMemory() {}

    /** Returns the mapped view of this shared memory segment. */
    public abstract ByteBuffer getMemory();

    /** Gets the parameters which need to be passed to the IPCFactory
        to create the child process's view of this shared memory
        segment. */
    public abstract Map getChildProcessParameters();

    /** Unmaps and releases all resources associated with this shared
        memory segment. */
    public abstract void dispose();
}
