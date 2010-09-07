/*
 * @(#)SetChildWindowHandleMessage.java	1.2 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.*;

/** Notification to the browser side of the child's window handle.
    This is currently only needed on Mac OS X. */

public class SetChildWindowHandleMessage extends AppletMessage {
    public static final int ID = PluginMessages.SET_CHILD_WINDOW_HANDLE;

    private long windowHandle;

    /** For deserialization purposes */
    public SetChildWindowHandleMessage(Conversation c) {
        super(ID, c);
    }

    public SetChildWindowHandleMessage(Conversation c, int appletID, long windowHandle) {
        super(ID, c, appletID);
        this.windowHandle = windowHandle;
    }

    public long getWindowHandle() {
        return windowHandle;
    }

    public void writeFields(Serializer ser) throws IOException {
        super.writeFields(ser);
        ser.writeLong(windowHandle);
    }

    public void readFields(Serializer ser) throws IOException {
        super.readFields(ser);
        windowHandle = ser.readLong();
    }
}
