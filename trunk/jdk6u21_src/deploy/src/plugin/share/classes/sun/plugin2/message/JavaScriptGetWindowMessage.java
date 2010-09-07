/*
 * @(#)JavaScriptGetWindowMessage.java	1.3 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.*;

import sun.plugin2.liveconnect.*;

/** Message sent from the client to the browser to fetch the JSObject
    corresponding to the window of the given applet. The return value
    is sent in a JavaScriptReplyMessage. */

public class JavaScriptGetWindowMessage extends AppletMessage {
    public static final int ID = PluginMessages.JAVASCRIPT_GET_WINDOW;

    /** For deserialization purposes */
    public JavaScriptGetWindowMessage(Conversation c) {
        super(ID, c);
    }

    /** Constructs a JavaScriptGetWindowMessage with the given
        parameters. */
    public JavaScriptGetWindowMessage(Conversation c,
                                      int appletID) {
        super(ID, c, appletID);
    }

    public void writeFields(Serializer ser) throws IOException {
        super.writeFields(ser);
    }
    
    public void readFields(Serializer ser) throws IOException {
        super.readFields(ser);
    }
}
