/*
 * @(#)GetAppletMessage.java	1.4 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.*;

/** Represents a request to get a handle to the given applet so that
    method invocations can be performed against it. The result is sent
    in a JavaReply message. */

public class GetAppletMessage extends AppletMessage {
    public static final int ID = PluginMessages.GET_APPLET;

    private int resultID;

    /** For deserialization purposes */
    public GetAppletMessage(Conversation c) {
        super(ID, c);
    }

    public GetAppletMessage(Conversation c,
                            int appletID,
                            int resultID) {
        super(ID, c, appletID);
        this.resultID = resultID;
    }

    public int getResultID() {
        return resultID;
    }

    public void writeFields(Serializer ser) throws IOException {
        super.writeFields(ser);
        ser.writeInt(resultID);
    }

    public void readFields(Serializer ser) throws IOException {
        super.readFields(ser);
        resultID = ser.readInt();
    }
}
