/*
 * @(#)StartAppletAckMessage.java	1.6 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.*;
import java.util.*;

/** Notification from the attached JVM instance to the browser that
    the given applet will execute in this JVM instance, or in other
    words, that it will not relaunch in a new JVM. This allows the
    browser side to release certain state required to support
    relaunching of applets. */

public class StartAppletAckMessage extends AppletMessage {
    public static final int ID = PluginMessages.START_APPLET_ACK;

    /** For deserialization purposes */
    public StartAppletAckMessage(Conversation c) {
        super(ID, c);
    }

    /** Creates a new message with the given parameters. A copy of the
        parameters is made internally, so subsequent changes will not
        be visible. */
    public StartAppletAckMessage(Conversation c, int appletID) {
        super(ID, c, appletID);
    }

    public void writeFields(Serializer ser) throws IOException {
        super.writeFields(ser);
    }

    public void readFields(Serializer ser) throws IOException {
        super.readFields(ser);
    }
}
