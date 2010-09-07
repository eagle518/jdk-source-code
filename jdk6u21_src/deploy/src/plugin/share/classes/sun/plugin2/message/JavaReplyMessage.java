/*
 * @(#)JavaReplyMessage.java	1.3 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.*;

import sun.plugin2.liveconnect.*;

/** Represents the return value and exception, if any, that occurred
    as a result of an operation on a Java object. Such exceptions need
    to be propagated back to the JavaScript engine in the browser for
    the best user experience and feedback. */

public class JavaReplyMessage extends PluginMessage {
    public static final int ID = PluginMessages.JAVA_REPLY;

    private int    resultID;
    private Object result;
    private boolean resultIsVoid;
    private String exceptionMessage;

    /** For deserialization purposes */
    public JavaReplyMessage(Conversation c) {
        super(ID, c);
    }

    /** Constructs a JavaReplyMessage with the given parameters. The
        result Object may not be an arbitrary Object, but must follow
        the conventions described in the {@link
        sun.plugin2.liveconnect.ArgumentHelper ArgumentHelper}. The
        exception message, if non-null, should cause an exception to
        be raised in the JavaScript engine in the browser. It must be
        non-null to cause an exception to be raised. If the exception
        message is non-null, the result must be null. */
    public JavaReplyMessage(Conversation c,
                            int resultID,
                            Object result,
                            boolean resultIsVoid,
                            String exceptionMessage) throws IllegalArgumentException {
        this(c);
        if (exceptionMessage != null && result != null) {
            throw new IllegalArgumentException("If the exception message is non-null, the result should be null");
        }
        this.resultID = resultID;
        this.result = result;
        this.resultIsVoid = resultIsVoid;
        this.exceptionMessage = exceptionMessage;
    }

    public int getResultID() {
        return resultID;
    }

    public Object getResult() {
        return result;
    }

    // Indicates whether the result is supposed to be void rather than null
    public boolean isResultVoid() {
        return resultIsVoid;
    }

    public String getExceptionMessage() {
        return exceptionMessage;
    }

    public void writeFields(Serializer ser) throws IOException {
        ser.writeInt(resultID);
        ArgumentHelper.writeObject(ser, result);
        ser.writeBoolean(resultIsVoid);
        ser.writeUTF(exceptionMessage);
    }
    
    public void readFields(Serializer ser) throws IOException {
        resultID = ser.readInt();
        result = ArgumentHelper.readObject(ser);
        resultIsVoid = ser.readBoolean();
        exceptionMessage = ser.readUTF();
    }
}
