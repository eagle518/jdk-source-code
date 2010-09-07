/*
 * @(#)JavaScriptReplyMessage.java	1.2 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.*;

import sun.plugin2.liveconnect.*;

/** Message sent from the browser back to the client in reply to a
    call or other action against a JavaScript object. Encapsulates
    both a return value, if any, as well as any exception information
    that might need to be propagated back to the client. */

public class JavaScriptReplyMessage extends PluginMessage {
    public static final int ID = PluginMessages.JAVASCRIPT_REPLY;

    private Object result;
    private String exceptionMessage;

    /** For deserialization purposes */
    public JavaScriptReplyMessage(Conversation c) {
        super(ID, c);
    }

    /** Constructs a JavaScriptReplyMessage with the given
        parameters. The result Object may not be an arbitrary Object,
        but must follow the conventions described in the {@link
        sun.plugin2.liveconnect.ArgumentHelper ArgumentHelper}. The
        exception message, if non-null, causes a JSException to be
        raised on the other side. It must be non-null to cause an
        exception to be raised. If the exception message is non-null,
        the result must be null. */
    public JavaScriptReplyMessage(Conversation c,
                                  Object result,
                                  String exceptionMessage) throws IllegalArgumentException {
        this(c);
        if (exceptionMessage != null && result != null) {
            throw new IllegalArgumentException("If the exception message is non-null, the result should be null");
        }
        this.result = result;
        this.exceptionMessage = exceptionMessage;
    }

    public Object getResult() {
        return result;
    }

    public String getExceptionMessage() {
        return exceptionMessage;
    }

    public void writeFields(Serializer ser) throws IOException {
        ArgumentHelper.writeObject(ser, result);
        ser.writeUTF(exceptionMessage);
    }
    
    public void readFields(Serializer ser) throws IOException {
        result = ArgumentHelper.readObject(ser);
        exceptionMessage = ser.readUTF();
    }
}
