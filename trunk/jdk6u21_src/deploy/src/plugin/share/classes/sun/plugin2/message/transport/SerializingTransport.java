/*
 * @(#)SerializingTransport.java	1.4 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message.transport;

import java.io.*;
import java.lang.reflect.*;
import java.util.*;

import sun.plugin2.message.*;

/** An implementation of the Transport interface using the {@link
    sun.plugin2.message.Serializer Serializer} mechanism. */

public abstract class SerializingTransport implements Transport {

    // A map of message IDs to Message classes. This Transport
    // requires that each Message subclass provides a public
    // constructor taking a Conversation for deserialization purposes,
    // although some Transport implementations (like in-process) might
    // not actually touch the fields of the Message objects
    private Map/*<Integer, Constructor<Message>>*/ messageIDMap =
        new HashMap/*<Integer, Constructor<Message>>*/();

    protected SerializingTransport() {}

    /** Registers a given Message ID with its associated class. It is
        illegal to register the same ID twice. A Class supplied here
        must have a public constructor taking only a Conversation
        object as argument. */
    public void registerMessageID(int id, Class/*<Message>*/ messageClass)
        throws IllegalArgumentException
    {
        Integer i = new Integer(id);
        if (messageIDMap.get(i) != null) {
            throw new IllegalArgumentException("Message ID " + id + " already registered");
        }

        try {
            Constructor/*<Message>*/ c = messageClass.getConstructor(new Class[] {
                Conversation.class
            });
            messageIDMap.put(i, c);
        } catch (Exception e) {
            throw (IllegalArgumentException)
                new IllegalArgumentException().initCause(e);
        }
    }


    // We use a private object for synchronization to avoid the
    // possibility that subclasses' use of wait / notify may break
    // invariants
    private final Object writeLock = new Object();

    /** Writes a message to the transport. */
    public void write(Message msg) throws IOException {
        synchronized (writeLock) {
            Serializer ser = getSerializer();
            // Note: we do this work here instead of Message.writeFields()
            // basically to keep things symmetric (i.e., to keep both
            // Message.readFields() and writeFields() abstract)
            ser.writeInt(msg.getID());
            ser.writeConversation(msg.getConversation());
            msg.writeFields(ser);
            // Signal the other side that data is ready in case it is waiting
            signalDataWritten();
        }
    }

    /** Reads a message from the transport. Returns null immediately
        if no data is available. */
    public Message read() throws IOException {
        // Note that we do not make this method synchronous. It is
        // expected that only one thread is going to pull messages off
        // the incoming transport and put them in the appropriate
        // message queues. The write side, however, needs to be
        // synchronized so that we don't collide from multiple threads
        // trying to simultaneously send messages.

        if (!isDataAvailable()) {
            return null;
        }

        // From this point on we assume we are either going to get a
        // well-formed message or we're going to throw an IOException
        // indicating that we essentially have to tear down the
        // transport because its contents are corrupted. Note that the
        // lower-level calls to the Serializer may throw exceptions if
        // for example the other side of the transport died and our
        // heartbeat detection noticed this.
        Serializer ser = getSerializer();
        int id = ser.readInt();
        Constructor ctor = (Constructor) messageIDMap.get(new Integer(id));
        if (ctor == null) {
            throw new IOException("Unregistered message ID " + id);
        }
        Conversation conv = ser.readConversation();
        Message msg = null;
        try {
            msg = (Message) ctor.newInstance(new Object[] { conv });
        } catch (Exception e) {
            throw (IOException)
                new IOException().initCause(e);
        }
        msg.readFields(ser);
        signalDataRead();
        return msg;
    }

    /** Waits for data to become available. */
    public abstract void waitForData(long millis) throws IOException;

    /** Signals the other side of the transport that data has been
        written. This is used to wake up the reader if it is asleep
        waiting for data. */
    protected abstract void signalDataWritten() throws IOException;

    /** Signals the other side of the transport that data has been
        read. This is used to wake up the writer if the transport
        became full in the middle of a write of a large message and
        the writer is waiting for the transport to drain. */
    protected abstract void signalDataRead();
    
    /** Indicates whether data is available to be read. */
    protected abstract boolean isDataAvailable() throws IOException;

    /** Fetches the Serializer associated with this transport. */
    protected abstract Serializer getSerializer();
}
