/*
 * @(#)JavaScriptMemberOpMessage.java	1.2 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.*;

import sun.plugin2.liveconnect.*;

/** Message sent from the client to the browser to perform an
    operation on a member of a JavaScript object (get, set, or
    remove). The return value is sent in a JavaScriptReplyMessage.
    Note that even for operations not having a return value (set and
    remove) we still need to wait for a reply in case we need to raise
    an exception. */

public class JavaScriptMemberOpMessage extends JavaScriptBaseMessage {
    public static final int ID = PluginMessages.JAVASCRIPT_MEMBER_OP;

    // Kinds of member operations
    public static final int GET    = 1;
    public static final int SET    = 2;
    public static final int REMOVE = 3;

    private String memberName;
    private int operationKind;
    // Optional argument
    private Object arg;

    /** For deserialization purposes */
    public JavaScriptMemberOpMessage(Conversation c) {
        super(ID, c);
    }

    /** Constructs a JavaScriptMemberOpMessage with the given
        parameters. The argument (if non-null) may not be an arbitrary
        object, but must follow the conventions described in the
        {@link sun.plugin2.liveconnect.ArgumentHelper
        ArgumentHelper}. */
    public JavaScriptMemberOpMessage(Conversation c,
                                     BrowserSideObject object,
                                     int appletID,
                                     String memberName,
                                     int operationKind,
                                     Object arg) {
        super(ID, c, object, appletID);
        this.memberName = memberName;
        this.operationKind = operationKind;
        this.arg = arg;
    }

    public void writeFields(Serializer ser) throws IOException {
        super.writeFields(ser);
        ser.writeUTF(memberName);
        ser.writeByte((byte) operationKind);
        ArgumentHelper.writeObject(ser, arg);
    }
    
    public void readFields(Serializer ser) throws IOException {
        super.readFields(ser);
        memberName = ser.readUTF();
        operationKind = ser.readByte() & 0xFF;
        arg = ArgumentHelper.readObject(ser);
    }

    public String getMemberName() {
        return memberName;
    }

    /** Returns the kind of operation this message implies -- GET,
        SET, or REMOVE. */
    public int getOperationKind() {
        return operationKind;
    }

    public Object getArgument() {
        return arg;
    }
}
