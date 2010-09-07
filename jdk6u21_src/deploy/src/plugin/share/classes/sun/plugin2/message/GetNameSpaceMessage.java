/*
 * @(#)GetNameSpaceMessage.java	1.3 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.*;

/** Represents a request to get a handle to a portion of the Java
    namespace in the form of a sun.plugin2.liveconnect.JavaNameSpace
    object. Field operations and invocations against these objects
    support getting and setting of static fields, invocation of static
    methods, and allocation of new objects from JavaScript. The result
    of this message is sent in a JavaReply message. */

public class GetNameSpaceMessage extends AppletMessage {
    public static final int ID = PluginMessages.GET_NAMESPACE;

    private String nameSpace;
    private int resultID;

    /** For deserialization purposes */
    public GetNameSpaceMessage(Conversation c) {
        super(ID, c);
    }

    public GetNameSpaceMessage(Conversation c,
                               int appletID,
                               String nameSpace,
                               int resultID) {
        super(ID, c, appletID);
        this.nameSpace = nameSpace;
        this.resultID = resultID;
    }

    public String getNameSpace() {
        return nameSpace;
    }

    public int getResultID() {
        return resultID;
    }

    public void writeFields(Serializer ser) throws IOException {
        super.writeFields(ser);
        ser.writeUTF(nameSpace);
        ser.writeInt(resultID);
    }

    public void readFields(Serializer ser) throws IOException {
        super.readFields(ser);
        nameSpace = ser.readUTF();
        resultID = ser.readInt();
    }
}
