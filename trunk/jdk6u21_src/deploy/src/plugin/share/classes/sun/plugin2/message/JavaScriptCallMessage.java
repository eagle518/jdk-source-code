/*
 * @(#)JavaScriptCallMessage.java	1.2 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.*;

import sun.plugin2.liveconnect.*;

/** Message sent from the client to the browser to call a JavaScript
    method. The return value is sent in a JavaScriptReplyMessage. */

public class JavaScriptCallMessage extends JavaScriptBaseMessage {
    public static final int ID = PluginMessages.JAVASCRIPT_CALL;

    private String methodName;
    private Object[] args;

    /** For deserialization purposes */
    public JavaScriptCallMessage(Conversation c) {
        super(ID, c);
    }

    /** Constructs a JavaScriptCallMessage with the given
        parameters. The argument array (if non-null) may not contain
        arbitrary objects, but must follow the conventions described
        in the {@link sun.plugin2.liveconnect.ArgumentHelper
        ArgumentHelper}. */
    public JavaScriptCallMessage(Conversation c,
                                 BrowserSideObject object,
                                 int appletID,
                                 String methodName,
                                 Object[] args) {
        super(ID, c, object, appletID);
        this.methodName = methodName;
        this.args = args;
    }

    public void writeFields(Serializer ser) throws IOException {
        super.writeFields(ser);
        ser.writeUTF(methodName);
        ArgumentHelper.writeObjectArray(ser, args);
    }
    
    public void readFields(Serializer ser) throws IOException {
        super.readFields(ser);
        methodName = ser.readUTF();
        args = ArgumentHelper.readObjectArray(ser);
    }

    public String getMethodName() {
        return methodName;
    }

    public Object[] getArguments() {
        return args;
    }
}
