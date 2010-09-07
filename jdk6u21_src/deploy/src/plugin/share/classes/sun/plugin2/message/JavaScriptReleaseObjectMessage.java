/*
 * @(#)JavaScriptReleaseObjectMessage.java	1.2 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.*;

import sun.plugin2.liveconnect.*;

/** Message sent from the client to the browser indicating that the
    given BrowserSideObject is no longer referenced by the given applet. */

public class JavaScriptReleaseObjectMessage extends JavaScriptBaseMessage {
    public static final int ID = PluginMessages.JAVASCRIPT_RELEASE_OBJECT;

    private BrowserSideObject obj;
    private int appletID;

    /** For deserialization purposes */
    public JavaScriptReleaseObjectMessage(Conversation c) {
        super(ID, c);
    }

    public JavaScriptReleaseObjectMessage(Conversation c,
                                          BrowserSideObject obj,
                                          int appletID) {
        super(ID, c, obj, appletID);
    }
}
