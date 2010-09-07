/*
 * @(#)PrintAppletReplyMessage.java	1.1 07/11/15
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.*;

/** A reply from the client JVM instance that print process
    is received and in process. */

public class PrintAppletReplyMessage extends AppletMessage {
    public static final int ID = PluginMessages.PRINT_APPLET_REPLY;

    // Print result
    private boolean res = false;

    /** For deserialization purposes */
    public PrintAppletReplyMessage(Conversation c) {
        super(ID, c);
    }

    public PrintAppletReplyMessage(Conversation c, int appletID, boolean res) {
        super(ID, c, appletID);
        this.res = res;
    }

    public boolean getRes() {
        return res;
    }

    public void writeFields(Serializer ser) throws IOException {
        super.writeFields(ser);
        ser.writeBoolean(res);
    }

    public void readFields(Serializer ser) throws IOException {
        super.readFields(ser);
        res = ser.readBoolean();
    }

}
