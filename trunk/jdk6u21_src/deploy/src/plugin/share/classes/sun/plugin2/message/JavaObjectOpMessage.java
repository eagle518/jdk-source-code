/*
 * @(#)JavaObjectOpMessage.java	1.6 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.*;

import sun.plugin2.liveconnect.*;

/** Single message type sent from the browser to the client to perform
    all Java object operations: calling a method, getting / setting a
    field or array element, etc. We abstract things away sufficiently
    on the other side to allow a single message type to cover all
    types of operations we support. The return value is sent in a
    JavaReplyMessage. See {@link
    sun.plugin2.main.server.LiveConnectSupport#sendRemoteJavaObjectOp
    LiveConnectSupport.sendRemoteJavaObjectOp()} for the theory behind
    the result ID notion. */

public class JavaObjectOpMessage extends AppletMessage {
    public static final int ID = PluginMessages.JAVA_OBJECT_OP;

    private RemoteJavaObject object;
    private String memberName;
    private int    operationKind;
    private Object[] args;
    private int    resultID;

    public static final int CALL_METHOD = 1;
    public static final int GET_FIELD   = 2;
    public static final int SET_FIELD   = 3;
    // The following two queries are concessions to the Mozilla
    // scripting engine and return their results in the form of a
    // Boolean in a JavaReplyMessage.
    public static final int HAS_FIELD   = 4;
    public static final int HAS_METHOD  = 5;
    
    // HAS_FIELD_OR_METHOD query is to better fit Internet Explorer's
    // GetIDsOfNames operation. 
    public static final int HAS_FIELD_OR_METHOD = 6;

    /** For deserialization purposes */
    public JavaObjectOpMessage(Conversation c) {
        super(ID, c);
    }

    /** Constructs a JavaObjectOpMessage with the given parameters.
        The argument array (if non-null) may not contain arbitrary
        objects, but must follow the conventions described in the
        {@link sun.plugin2.liveconnect.ArgumentHelper
        ArgumentHelper}. <P>

        A memberName of <CODE>&lt;init&gt;</CODE> along with an
        operation kind of CALL_METHOD (and a target object of a
        JavaNameSpace) means to invoke a constructor of the named
        class, returning a new object.
    */
    public JavaObjectOpMessage(Conversation c,
                               RemoteJavaObject object,
                               String memberName,
                               int operationKind,
                               Object[] args,
                               int resultID) {
        super(ID, c, object.getAppletID());
        this.object = object;
        this.memberName = memberName;
        this.operationKind = operationKind;
        this.args = args;
        this.resultID = resultID;
    }

    public void writeFields(Serializer ser) throws IOException {
        super.writeFields(ser);
        RemoteJavaObject.write(ser, object);
        ser.writeUTF(memberName);
        ser.writeInt(operationKind);
        ArgumentHelper.writeObjectArray(ser, args);
        ser.writeInt(resultID);
    }
    
    public void readFields(Serializer ser) throws IOException {
        super.readFields(ser);
        object = RemoteJavaObject.read(ser);
        memberName = ser.readUTF();
        operationKind = ser.readInt();
        args = ArgumentHelper.readObjectArray(ser);
        resultID = ser.readInt();
    }

    public RemoteJavaObject getObject() {
        return object;
    }

    public String getMemberName() {
        return memberName;
    }

    public int getOperationKind() {
        return operationKind;
    }

    public Object[] getArguments() {
        return args;
    }

    public int getResultID() {
        return resultID;
    }
}
