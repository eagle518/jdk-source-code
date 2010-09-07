/*
 * @(#)ShutdownJVMMessage.java	1.2 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.*;

/** A request to shut down a given JVM instance. FIXME: not currently
    used -- need to implement timeout and shutdown of idle JVMs. */

public class ShutdownJVMMessage extends PluginMessage {
    public static final int ID = PluginMessages.SHUTDOWN_JVM;

    public ShutdownJVMMessage(Conversation c) {
        super(ID, c);
    }

    public void writeFields(Serializer ser) throws IOException {
    }

    public void readFields(Serializer ser) throws IOException {
    }
}
