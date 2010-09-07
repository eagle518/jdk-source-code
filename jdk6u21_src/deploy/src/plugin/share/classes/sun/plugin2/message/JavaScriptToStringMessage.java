/*
 * @(#)JavaScriptToStringMessage.java	1.2 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.*;

import sun.plugin2.liveconnect.*;

/** Message sent from the client to the browser to convert a JSObject
    into a string representation. The return value is sent in a
    JavaScriptReplyMessage. */

public class JavaScriptToStringMessage extends JavaScriptBaseMessage {
    public static final int ID = PluginMessages.JAVASCRIPT_TO_STRING;

    /** For deserialization purposes */
    public JavaScriptToStringMessage(Conversation c) {
        super(ID, c);
    }

    /** Constructs a JavaScriptToStringMessage with the given
        parameters. */
    public JavaScriptToStringMessage(Conversation c,
                                     BrowserSideObject object,
                                     int appletID) {
        super(ID, c, object, appletID);
    }
}
