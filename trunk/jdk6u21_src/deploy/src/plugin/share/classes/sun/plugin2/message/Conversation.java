/*
 * @(#)Conversation.java	1.2 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.*;

/** Identifies a conversation between two entities. */

public class Conversation {
    private boolean initiatingSide;
    private int     id;

    /** Do not call this directly; initiate a new Conversation with
        {@link Pipe#beginConversation Pipe.beginConversation()}. */
    public Conversation() {}

    /** Do not call this directly; initiate a new Conversation with
        {@link Pipe#beginConversation Pipe.beginConversation()}. */
    public Conversation(boolean initiatingSide, int id) {
        this.initiatingSide = initiatingSide;
        this.id = id;
    }

    public int hashCode() {
        return id;
    }

    public boolean equals(Object arg) {
        if (arg == null || getClass() != arg.getClass()) {
            return false;
        }

        Conversation other = (Conversation) arg;
        return (other.initiatingSide == initiatingSide &&
                other.id == id);
    }

    /** Indicates whether this Conversation object was created by the
        initiating side of the pipe. */
    public boolean isInitiatingSide() { return initiatingSide; }

    /** Returns the conversation ID. */
    public int getID() {
        return id;
    }

    // Serialization routines
    public void readFields(Serializer ser) throws IOException {
        initiatingSide = ser.readBoolean();
        id = ser.readInt();
    }
    public void writeFields(Serializer ser) throws IOException {
        ser.writeBoolean(initiatingSide);
        ser.writeInt(id);
    }

    public String toString() {
        return "[Conversation: id=" + id + ", initiatingSide=" + initiatingSide + "]";
    }
}
