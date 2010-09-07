/*
 * @(#)StopAppletMessage.java	1.3 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.*;

/** A request to stop an applet. */

public class StopAppletMessage extends AppletMessage {
    public static final int ID = PluginMessages.STOP_APPLET;

    /** For deserialization purposes */
    public StopAppletMessage(Conversation c) {
        super(ID, c);
    }

    /** For StopAppletAck purposes */
    public StopAppletMessage(int id, Conversation c) {
        super(id, c);
    }

    public StopAppletMessage(Conversation c, int appletID) {
        super(ID, c, appletID);
    }

    /** For StopAppletAck purposes */
    public StopAppletMessage(int id, Conversation c, int appletID) {
        super(id, c, appletID);
    }

    public void writeFields(Serializer ser) throws IOException {
        super.writeFields(ser);
    }

    public void readFields(Serializer ser) throws IOException {
        super.readFields(ser);
    }
}
