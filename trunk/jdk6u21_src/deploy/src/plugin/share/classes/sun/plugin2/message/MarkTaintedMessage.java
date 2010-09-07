/*
 * @(#)MarkTaintedMessage.java	1.2 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.*;

public class MarkTaintedMessage extends PluginMessage {
    public static final int ID = PluginMessages.MARK_JVM_TAINTED;

    /** For deserialization purposes */
    public MarkTaintedMessage(Conversation c) {
        super(ID, c);
    }

    public void writeFields(Serializer ser) throws IOException {
    }

    public void readFields(Serializer ser) throws IOException {
    }
}
