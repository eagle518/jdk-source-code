/*
 * @(#)SetAppletSizeMessage.java	1.3 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.*;

/** A request to change an applet's size. */

public class SetAppletSizeMessage extends AppletMessage {
    public static final int ID = PluginMessages.SET_APPLET_SIZE;

    // The width and height specified by the browser; note that since
    // it's in its parent EmbeddedFrame the x and y coordinates are
    // (0, 0)
    private int width;
    private int height;

    /** For deserialization purposes */
    public SetAppletSizeMessage(Conversation c) {
        super(ID, c);
    }

    public SetAppletSizeMessage(Conversation c, int appletID, int width, int height) {
        super(ID, c, appletID);
        this.width = width;
        this.height = height;
    }

    public int getWidth() {
        return width;
    }

    public int getHeight() {
        return height;
    }

    public void writeFields(Serializer ser) throws IOException {
        super.writeFields(ser);
        ser.writeInt(width);
        ser.writeInt(height);
    }

    public void readFields(Serializer ser) throws IOException {
        super.readFields(ser);
        width = ser.readInt();
        height = ser.readInt();
    }
}
