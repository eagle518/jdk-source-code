/*
 * @(#)JVMStartedMessage.java	1.3 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.*;

import sun.plugin2.liveconnect.*;

/** Empty message indicating the target JVM started successfully. */
public class JVMStartedMessage extends PluginMessage {
    public static final int ID = PluginMessages.JVM_STARTED_ID;

    /** For deserialization purposes */
    public JVMStartedMessage(Conversation c) {
        super(ID, c);
    }

    public void writeFields(Serializer ser) throws IOException {
    }
    
    public void readFields(Serializer ser) throws IOException {
    }
}
