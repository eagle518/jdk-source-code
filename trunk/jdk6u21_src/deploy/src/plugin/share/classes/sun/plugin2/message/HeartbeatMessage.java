/*
 * @(#)HeartbeatMessage.java	1.4 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.*;

/** Used by the client JVM instances to see whether the web browser
    has exited. */

public class HeartbeatMessage extends PluginMessage {
    public static final int ID = PluginMessages.HEARTBEAT;

    public HeartbeatMessage(Conversation c) {
        super(ID, c);
    }

    public void writeFields(Serializer ser) throws IOException {
    }
    
    public void readFields(Serializer ser) throws IOException {
    }
}
