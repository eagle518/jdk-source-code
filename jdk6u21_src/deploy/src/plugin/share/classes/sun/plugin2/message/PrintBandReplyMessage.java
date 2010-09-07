/*
 * @(#)PrintBandReplyMessage.java	1.1 07/11/15
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.*;

/**  */

public class PrintBandReplyMessage extends AppletMessage {
    public static final int ID = PluginMessages.PRINT_BAND_REPLY;

    private int destY;

    // Print result
    private boolean res = false;

    /** For deserialization purposes */
    public PrintBandReplyMessage(Conversation c) {
        super(ID, c);
    }

    public PrintBandReplyMessage(Conversation c, int appletID, int destY, boolean res) {
        super(ID, c, appletID);
        this.destY = destY;
        this.res = res;
    }

    public int getDestY() {
        return destY;
    }

    public boolean getRes() {
        return res;
    }

    public void writeFields(Serializer ser) throws IOException {
        super.writeFields(ser);
        ser.writeInt(destY);
        ser.writeBoolean(res);
    }

    public void readFields(Serializer ser) throws IOException {
        super.readFields(ser);
        destY = ser.readInt();
        res = ser.readBoolean();
    }

}
