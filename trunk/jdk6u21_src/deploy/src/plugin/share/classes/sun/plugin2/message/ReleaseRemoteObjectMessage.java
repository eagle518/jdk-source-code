/*
 * @(#)ReleaseRemoteObjectMessage.java	1.3 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.*;

/** This class represents a notification from the browser to an
    attached JVM instance that an object that JVM instance has exposed
    back to the browser can be safely released. */

public class ReleaseRemoteObjectMessage extends PluginMessage {
    public static final int ID = PluginMessages.RELEASE_REMOTE_OBJECT;

    private int objectID;

    /** For deserialization purposes */
    public ReleaseRemoteObjectMessage(Conversation c) {
        super(ID, c);
    }

    public ReleaseRemoteObjectMessage(Conversation c,
                                      int objectID) {
        this(c);
        this.objectID = objectID;
    }

    public int getObjectID() {
        return objectID;
    }

    public void writeFields(Serializer ser) throws IOException {
        ser.writeInt(objectID);
    }

    public void readFields(Serializer ser) throws IOException {
        objectID = ser.readInt();
    }
}
