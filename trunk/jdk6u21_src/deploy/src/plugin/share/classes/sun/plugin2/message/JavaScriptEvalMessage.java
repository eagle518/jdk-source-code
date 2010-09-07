/*
 * @(#)JavaScriptEvalMessage.java	1.2 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.*;

import sun.plugin2.liveconnect.*;

/** Message sent from the client to the browser to evaluate a piece of
    JavaScript code. The return value is sent in a JavaScriptReplyMessage. */

public class JavaScriptEvalMessage extends JavaScriptBaseMessage {
    public static final int ID = PluginMessages.JAVASCRIPT_EVAL;

    private String code;

    /** For deserialization purposes */
    public JavaScriptEvalMessage(Conversation c) {
        super(ID, c);
    }

    /** Constructs a JavaScriptCallMessage with the given
        parameters. The argument array (if non-null) may not contain
        arbitrary objects, but must follow the conventions described
        in the {@link sun.plugin2.liveconnect.ArgumentHelper
        ArgumentHelper}. */
    public JavaScriptEvalMessage(Conversation c,
                                 BrowserSideObject object,
                                 int appletID,
                                 String code) {
        super(ID, c, object, appletID);
        this.code = code;
    }

    public void writeFields(Serializer ser) throws IOException {
        super.writeFields(ser);
        ser.writeUTF(code);
    }
    
    public void readFields(Serializer ser) throws IOException {
        super.readFields(ser);
        code = ser.readUTF();
    }

    public String getCode() {
        return code;
    }
}
