/*
 * @(#)StopAppletAckMessage.java	1.2 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.*;

/** Acknowledgment from the client JVM instance that an applet has
    stopped cooperatively. By design, the browser waits only a brief
    period of time (currently 500 ms, or 1/2 second) for this reply
    before deciding that the applet has taken too long to terminate
    and returns control to the end user. */

public class StopAppletAckMessage extends StopAppletMessage {
    public static final int ID = PluginMessages.STOP_APPLET_ACK;

    /** For deserialization purposes */
    public StopAppletAckMessage(Conversation c) {
        super(ID, c);
    }

    public StopAppletAckMessage(Conversation c, int appletID) {
        super(ID, c, appletID);
    }
}
