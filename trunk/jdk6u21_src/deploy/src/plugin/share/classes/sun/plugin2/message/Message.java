/*
 * @(#)Message.java	1.2 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.*;

/** The base Message class which the Pipe is responsible for sending
    from side to side. */

public abstract class Message {
    private int id;
    private Conversation conversation;

    /** Constructs a new Message with the given ID and Conversation. */

    public Message(int id, Conversation conversation) {
        this.id = id;
        this.conversation = conversation;
    }

    public int hashCode() {
        int hash = 0;
        if (conversation != null) {
            hash = 17 * conversation.hashCode();
        }
        hash += id;
        return hash;
    }

    public boolean equals(Object arg) {
        if (arg == null || getClass() != arg.getClass())
            return false;
        Message other = (Message) arg;
        if (conversation == null && other.conversation != null)
            return false;
        return (id == other.id &&
                (conversation == null || conversation.equals(other.conversation)));
    }

    /** Returns the ID associated with this message. */
    public int getID() { return id; }

    /** Returns the Conversation associated with this message. */
    public Conversation getConversation() { return conversation; }

    // Serialization routines
    // Note that we have to pull the ID and Conversation off the
    // stream manually, so there are no fields to read at this level
    // in the class hierarchy

    /** Writes the fields of this Message using the given Serializer. */
    public abstract void writeFields(Serializer ser) throws IOException;
    /** Reads the fields of this Message from the given Serializer. */
    public abstract void readFields(Serializer ser) throws IOException;
}
