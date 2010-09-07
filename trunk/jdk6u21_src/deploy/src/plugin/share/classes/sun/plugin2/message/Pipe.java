/*
 * @(#)Pipe.java	1.9 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.io.*;
import java.util.*;

import sun.plugin2.message.transport.Transport;
import sun.plugin2.util.SystemUtil;

/** Represents a bi-directional pipe between two entities across which
    messages can be sent and received. */

public class Pipe {
    private static final boolean DEBUG = (SystemUtil.getenv("JPI_PLUGIN2_DEBUG") != null);

    private Transport transport;

    // Indicates whether this half of the pipe is the "initiating"
    // side. This is used principally to detect whether we are the
    // initiator of a particular Conversation.
    private boolean initiatingSide;

    // We maintain a lazily created message queue per thread. Incoming
    // messages which correspond to a Conversation initiated by a
    // given thread are routed back to that thread, which is likely
    // expecting a reply.
    private static final ThreadLocal/*<Queue>*/ perThreadMsgQueue =
        new ThreadLocal/*<Queue>*/();

    // We also maintain a main message queue. This is where all
    // messages not corresponding to a given ongoing Conversation are
    // routed.
    private Queue mainMsgQueue = new Queue();

    // A map of the currently active conversations to the thread
    // queues where their messages should be posted.
    private Map/*<Conversation, Queue>*/ activeConversations =
        new HashMap/*<Conversation, Queue>*/();
    private int curConversationID;

    private volatile boolean shouldShutdown;
    private volatile boolean shutdownComplete;

    /** Constructs a pipe with the given transport mechanism and
        indicator of whether we are the side of the pipe initiating
        the connection to the other. */
    public Pipe(Transport transport, boolean initiatingSide) {
        if(DEBUG) {
            System.out.println("Pipe.cstr: "+transport.toString());
        }
        this.transport = transport;
        this.initiatingSide = initiatingSide;
        WorkerThread worker = new WorkerThread();
        worker.setDaemon(true);
        worker.start();
    }

    /** Asynchronously sends the given message to the other side of
        the pipe. */
    public void send(Message message) throws IOException {
        transport.write(message);
    }

    /** Polls for a message on the pipe. Returns null immediately if
        there is no message available. */
    public Message poll() throws IOException {
        checkForShutdown();
        return mainMsgQueue.get();
    }

    /** Polls for a message on the pipe corresponding to a given
        Conversation. Returns null immediately if there is no message
        available. */
    public Message poll(Conversation conversation) throws IOException {
        checkForShutdown();
        Queue threadLocalQueue = (Queue) perThreadMsgQueue.get();
        if (threadLocalQueue == null) {
            return null;
        }
        return threadLocalQueue.get(-1, conversation);
    }

    /** Waits for a message on the pipe for the given duration in
        milliseconds. A zero duration causes it to wait indefinitely
        (strongly not recommended for robustness). */
    public Message receive(long millisToWait) throws InterruptedException, IOException {
        checkForShutdown();
        return mainMsgQueue.waitForMessage(millisToWait);
    }

    /** Waits for a message on the pipe corresponding to the given
        Conversation, waiting up to the specified duration in
        milliseconds. A zero duration causes it to wait indefinitely
        (strongly not recommended for robustness). Returns null if no
        message was available in this time duration. */
    public Message receive(long millisToWait, Conversation conversation) throws InterruptedException, IOException {
        checkForShutdown();
        Queue threadLocalQueue = (Queue) perThreadMsgQueue.get();
        if (threadLocalQueue == null) {
            return null;
        }
        return threadLocalQueue.waitForMessage(millisToWait, -1, conversation);
    }

    /** Creates a new conversation with the other side of the pipe.
        This is used to identify back-and-forth communication between
        the two entities (i.e., request-reply semantics). Messages
        sent in reply with the same Conversation ID will be routed
        back to this thread, which can use {@link #poll(Conversation)
        poll} or {@link #receive(long, Conversation) receive} to see
        these reply messages. There may be limitations to the number
        of concurrent conversations; the current implementation
        supports 2^32 concurrent conversations. */
    public synchronized Conversation beginConversation() {
        int id = curConversationID++;
        Conversation conversation = new Conversation(initiatingSide, id);
        Queue threadLocalQueue = (Queue) perThreadMsgQueue.get();
        if (threadLocalQueue == null) {
            threadLocalQueue = new Queue();
            perThreadMsgQueue.set(threadLocalQueue);
        }
        activeConversations.put(conversation, threadLocalQueue);
        return conversation;
    }

    /** Indicates that the given Conversation has ended, and allows
        certain resources to be reclaimed. */
    public void endConversation(Conversation conversation) {
        activeConversations.remove(conversation);
    }

    /** Shuts down this Pipe, reclaiming internal resources. It is not
        legal to send or attempt to receive further messages on this
        Pipe after shutting it down. */
    public void shutdown() {
        shouldShutdown = true;
        // Note that we don't wait for completion of this because
        // doing so can block the browser's main thread if for example
        // the transport failed to connect properly and is deadlocked
    }

    private synchronized Queue getQueue(Conversation conversation) {
        return (Queue) activeConversations.get(conversation);
    }
    
    private synchronized void interruptActiveQueues() {
        mainMsgQueue.interrupt();
        for (Iterator iter = activeConversations.values().iterator(); iter.hasNext(); ) {
            ((Queue) iter.next()).interrupt();
        }
    }

    private void checkForShutdown() throws IOException {
        if (shutdownComplete) {
            throw new IOException("Pipe is already shut down");
        }
    }

    // An internal worker thread responsible for reading data from the
    // transport and posting the messages read to the appropriate
    // message queues.
    class WorkerThread extends Thread {
        public WorkerThread() {
            super("Java Plug-In Pipe Worker Thread (" +
                  (initiatingSide ? "Server-Side" : "Client-Side") + ")");
        }

        public void run() {
            try {
                // Wake up periodically checking to see whether we should shut down
                try {
                    while (!shouldShutdown) {
                        Message msg = null;
                        transport.waitForData(500);
                        while ((msg = transport.read()) != null) {
                            Conversation c = msg.getConversation();
                            // Figure out which message queue to dispatch it to
                            boolean dispatched = false;
                            if (c != null) {
                                Queue queue = getQueue(c);
                                if (queue != null) {
                                    queue.put(msg);
                                    dispatched = true;
                                }
                            }
                            if (!dispatched) {
                                // Put it on the main queue
                                mainMsgQueue.put(msg);
                            }
                        }
                    }
                } catch (IOException e) {
                    // Throw InterruptedException to any threads waiting for messages
                    interruptActiveQueues();
                    if (DEBUG) {
                        // FIXME: any better way to propagate this out?
                        System.out.println("Terminating " + Thread.currentThread().getName() + " due to exception:");
                        e.printStackTrace();
                    }
                }
            } finally {
                shutdownComplete = true;
                synchronized(Pipe.this) {
                    Pipe.this.notifyAll();
                }
            }
        }
    }
}
