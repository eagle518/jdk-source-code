/*
 * @(#)Transport.java	1.4 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message.transport;

import java.io.*;

import sun.plugin2.message.*;

/** Defines the interface to the transport mechanism used by the Pipe
    class. */

public interface Transport {

    /** Writes a message to the transport. It is undefined what
        happens if the message size exceeds the internal buffer size
        of the transport. */
    public void write(Message msg) throws IOException;

    /** Reads a message from the transport. Returns null immediately
        if no data is available. */
    public Message read() throws IOException;

    /** Waits for data to become available to read. */
    public void waitForData(long millis) throws IOException;

    public String toString();
}
