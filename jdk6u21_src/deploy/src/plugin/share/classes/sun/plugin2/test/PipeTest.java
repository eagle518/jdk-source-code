/*
 * @(#)PipeTest.java	1.2 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.test;

import java.io.*;
import java.nio.*;
import java.util.*;

import sun.plugin2.ipc.*;
import sun.plugin2.message.*;
import sun.plugin2.message.transport.*;
import sun.plugin2.util.*;

public abstract class PipeTest {
    protected static final int NUM_MESSAGES = 30000;
    protected static final int NUM_CONVERSATION_THREADS = 4;
    //    private static final int NUM_CONVERSATION_THREADS = 0;

    protected static class StringMessage extends Message {
        public static final int ID = 1;
        
        private String string;
        
        public StringMessage(Conversation conversation) {
            super(ID, conversation);
        }

        public StringMessage(Conversation conversation,
                             String string) {
            super(ID, conversation);
            this.string = string;
        }

        public boolean equals(Object arg) {
            if (!super.equals(arg)) {
                return false;
            }

            StringMessage other = (StringMessage) arg;
            if (string == null && other.string != null)
                return false;
            return (string == null || string.equals(other.string));
        }

        public String getString() { return string; }

        public void readFields(Serializer ser) throws IOException {
            string = ser.readUTF();
        }

        public void writeFields(Serializer ser) throws IOException {
            ser.writeUTF(string);
        }

        public String toString() {
            return "[StringMessage: string=" + string + "]";
        }
    }

    protected static class RequestMessage extends Message {
        public static final int ID = 2;

        protected boolean z;
        protected byte b;
        protected short s;
        protected char c;
        protected int i;
        protected long l;
        protected float f;
        protected double d;

        public RequestMessage(Conversation conversation) {
            super(ID, conversation);
            Random random = new Random();
            z = random.nextBoolean();
            b = (byte) random.nextInt(256);
            s = (short) random.nextInt(65536);
            c = (char) random.nextInt(65536);
            i = random.nextInt();
            l = random.nextLong();
            f = random.nextFloat();
            d = random.nextDouble();
        }

        protected RequestMessage(int id, Conversation conversation) {
            super(id, conversation);
        }

        public boolean equals(Object arg) {
            if (!super.equals(arg)) {
                return false;
            }

            return fieldsEqual((RequestMessage) arg);
        }

        public boolean fieldsEqual(RequestMessage other) {
            return (z == other.z &&
                    b == other.b &&
                    s == other.s &&
                    c == other.c &&
                    i == other.i &&
                    l == other.l &&
                    f == other.f &&
                    d == other.d);
        }

        public void readFields(Serializer ser) throws IOException {
            z = ser.readBoolean();
            b = ser.readByte();
            s = ser.readShort();
            c = ser.readChar();
            i = ser.readInt();
            l = ser.readLong();
            f = ser.readFloat();
            d = ser.readDouble();
        }

        public void writeFields(Serializer ser) throws IOException {
            ser.writeBoolean(z);
            ser.writeByte(b);
            ser.writeShort(s);
            ser.writeChar(c);
            ser.writeInt(i);
            ser.writeLong(l);
            ser.writeFloat(f);
            ser.writeDouble(d);
        }
    }

    protected static class ReplyMessage extends RequestMessage {
        public static final int ID = 3;

        public ReplyMessage(Conversation conversation) {
            super(ID, conversation);
        }

        public ReplyMessage(RequestMessage msg) {
            super(ID, msg.getConversation());
            z = msg.z;
            b = msg.b;
            s = msg.s;
            c = msg.c;
            i = msg.i;
            l = msg.l;
            f = msg.f;
            d = msg.d;
        }

        public void readFields(Serializer ser) throws IOException {
            super.readFields(ser);
        }

        public void writeFields(Serializer ser) throws IOException {
            super.writeFields(ser);
        }
    }

    protected static class ShutdownMessage extends Message {
        public static final int ID = 4;
        public ShutdownMessage(Conversation conversation) {
            super(ID, conversation);
        }
        public void readFields(Serializer ser) throws IOException {}
        public void writeFields(Serializer ser) throws IOException {}
    }

    private volatile boolean testPassed = true;
    protected boolean testPassed() {
        return testPassed;
    }

    private int numThreadsRunning = 0;
    protected Object lock = new Object();
    protected int incNumThreadsRunning() {
        int i = 0;
        synchronized(lock) {
            i = ++numThreadsRunning;
        }
        return i;
    }

    protected void decNumThreadsRunning() {
        synchronized(lock) {
            --numThreadsRunning;
            lock.notifyAll();
        }
    }

    protected int numThreadsRunning() {
        return numThreadsRunning;
    }

    protected void waitForThreadTermination() {
        synchronized(lock) {
            try {
                lock.wait();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }

    protected class ServerWriterThread extends Thread {
        private Pipe pipe;
        private LinkedList messagesSent;

        public ServerWriterThread(Pipe pipe, LinkedList messagesSent) {
            super("PipeTest ServerWriterThread");
            this.pipe = pipe;
            this.messagesSent = messagesSent;
            incNumThreadsRunning();
        }

        public void run() {
            try {
                for (int i = 0; i < NUM_MESSAGES; i++) {
                    StringMessage msg = new StringMessage(null, randomString());
                    synchronized(messagesSent) {
                        messagesSent.add(msg);
                    }
                    pipe.send(msg);
                }
            } catch (Exception e) {
                testPassed = false;
                throw new RuntimeException(e);
            } finally {
                decNumThreadsRunning();
            }
        }
    }

    protected class ServerReaderThread extends Thread {
        private Pipe pipe;
        private LinkedList messagesSent;

        public ServerReaderThread(Pipe pipe, LinkedList messagesSent) {
            super("PipeTest ServerReaderThread");
            this.pipe = pipe;
            this.messagesSent = messagesSent;
            incNumThreadsRunning();
        }

        public void run() {
            try {
                boolean stop = false;
                int numMessagesProcessed = 0;

                do {
                    Message msg = pipe.receive(0);
                    if (msg instanceof ShutdownMessage) {
                        stop = true;
                    } else {
                        Message written = null;
                        synchronized(messagesSent) {
                            written = (Message) messagesSent.removeFirst();
                        }
                        if (!msg.equals(written)) {
                            testPassed = false;
                            System.out.println("Messages not equal:");
                            System.out.println("  Message sent: " + written);
                            System.out.println("  Message received: " + msg);
                            throw new RuntimeException("Messages were not equal");
                        }
                    }
                    if (++numMessagesProcessed % 1000 == 0) {
                        System.out.println("Server reader thread processed " + numMessagesProcessed + " messages");
                    }
                } while (!stop);
            } catch (Exception e) {
                testPassed = false;
                throw new RuntimeException(e);
            } finally {
                decNumThreadsRunning();
            }
        }
    }

    protected class ServerConversationThread extends Thread {
        private Pipe pipe;
        private int ident;

        public ServerConversationThread(Pipe pipe) {
            super("PipeTest ServerConversationThread-" + incNumThreadsRunning());
            this.pipe = pipe;
        }

        public void run() {
            try {
                int numMessages = NUM_MESSAGES / NUM_CONVERSATION_THREADS / 10;
                int numMessagesProcessed = 0;
                for (int i = 0; i < numMessages; i++) {
                    Conversation c = pipe.beginConversation();
                    RequestMessage msg = new RequestMessage(c);
                    pipe.send(msg);
                    Message replyMsg = pipe.receive(0, c);
                    pipe.endConversation(c);
                    if (!(replyMsg instanceof ReplyMessage)) {
                        testPassed = false;
                        throw new RuntimeException("Expected ReplyMessage, got a " +
                                                   replyMsg.getClass().getName());
                    }
                    ReplyMessage reply = (ReplyMessage) replyMsg;
                    if (!msg.fieldsEqual(reply)) {
                        testPassed = false;
                        throw new RuntimeException("Request / reply not equal");
                    }
                    if (i % 100 == 0) {
                        System.out.println(getName() + ": processed " + i + " request / reply pairs");
                    }
                }
            } catch (Exception e) {
                testPassed = false;
                throw new RuntimeException(e);
            } finally {
                decNumThreadsRunning();
            }
        }
    }

    protected class ClientThread extends Thread {
        private Pipe pipe;
        private boolean exitOnShutdown;

        public ClientThread(Pipe pipe, boolean exitOnShutdown) {
            super("PipeTest ClientThread");
            this.pipe = pipe;
            incNumThreadsRunning();
            this.exitOnShutdown = exitOnShutdown;
        }

        public void run() {
            try {
                boolean stop = false;
                int numMessagesProcessed = 0;
                do {
                    Message m = pipe.receive(5000);
                    if (m == null) {
                        throw new IOException("Server side appears to have died");
                    }
                    // Special message handling
                    switch (m.getID()) {
                        case StringMessage.ID: {
                            // Just send these back
                            pipe.send(m);
                            break;
                        }
                        case RequestMessage.ID: {
                            RequestMessage msg = (RequestMessage) m;
                            // Convert these to ReplyMessages, preserving the Conversation
                            pipe.send(new ReplyMessage(msg));
                            break;
                        }
                        case ShutdownMessage.ID: {
                            stop = true;
                            // Send it back too
                            pipe.send(m);
                            break;
                        }
                        default: {
                            testPassed = false;
                            throw new RuntimeException("Unexpected message ID received from server: " + m.getID());
                        }
                    }
                    // if (++numMessagesProcessed % 1000 == 0) {
                    //     System.out.println("Client thread processed " + numMessagesProcessed + " messages");
                    // }
                } while (!stop);
            } catch (Exception e) {
                testPassed = false;
                throw new RuntimeException(e);
            } finally {
                decNumThreadsRunning();
                if (exitOnShutdown) {
                    System.exit(0);
                }
            }
        }
    }
        
    protected static SerializingTransport registerMessages(SerializingTransport transport) {
        transport.registerMessageID(StringMessage.ID,   StringMessage.class);
        transport.registerMessageID(RequestMessage.ID,  RequestMessage.class);
        transport.registerMessageID(ReplyMessage.ID,    ReplyMessage.class);
        transport.registerMessageID(ShutdownMessage.ID, ShutdownMessage.class);
        return transport;
    }

    private static String randomString() {
        Random random = new Random();
        int len = (int) (100 * random.nextDouble());
        if (len > 0) {
            char[] randomString = new char[len];
            for (int j = 0; j < len; j++) {
                randomString[j] = (char) ('a' + random.nextInt(26));
            }
            return new String(randomString);
        }
        return null;
    }
}
