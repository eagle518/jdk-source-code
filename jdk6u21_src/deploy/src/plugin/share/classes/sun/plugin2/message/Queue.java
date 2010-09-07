/*
 * @(#)Queue.java	1.3 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message;

import java.util.*;

/** A mostly first-in, first-out queue of messages. Supports
    read-ahead to pull off messages matching certain criteria. */

class Queue {
    private LinkedList/*<Message>*/ messages = new LinkedList/*<Message>*/();
    private Thread waiter;
    
    /** Puts a message on the end of the queue. */
    public synchronized void put(Message message) {
        messages.add(message);
        notifyAll();
    }

    /** Gets the message at the head of the queue, or null if no
        messages are available. */
    public synchronized Message get() {
        if (messages.size() == 0)
            return null;

        return (Message) messages.removeFirst();
    }

    /** Retrieves and removes the first message on the queue
        satisfying the given search criteria. The id may be less than
        0, in which case it is ignored. The Conversation may be null,
        in which case any message with the given ID will be
        returned. */
    public synchronized Message get(int id, Conversation conversation) {
        for (int i = 0; i < messages.size(); i++) {
            Message msg = (Message) messages.get(i);
            if ((id < 0 || msg.getID() == id)
                &&
                (conversation == null || conversation.equals(msg.getConversation()))) {
                messages.remove(i);
                return msg;
            }
        }
        return null;
    }

    /** Waits for a message to be posted to the queue for up to the
        specified duration in milliseconds. If milliseconds is zero,
        waits indefinitely. */
    public synchronized Message waitForMessage(long milliseconds)
        throws InterruptedException
    {
        if (messages.size() == 0) {
            waiter = Thread.currentThread();
            try {
                wait(milliseconds);
            } finally {
                waiter = null;
            }
        }
        return get();
    }

    /** Waits for a message matching the given search criteria to be
        posted to the queue for up to the specified duration in
        milliseconds. If milliseconds is zero, waits indefinitely. */
    public synchronized Message waitForMessage(long milliseconds,
                                               int id,
                                               Conversation conversation)
        throws InterruptedException
    {
        Message msg = get(id, conversation);
        if (msg != null)
            return msg;

        boolean infinite = (milliseconds == 0);
        do {
            long startTime = System.currentTimeMillis();
            waiter = Thread.currentThread();
            try {
                wait(milliseconds);
            } finally {
                waiter = null;
            }
            long endTime = System.currentTimeMillis();
            if (!infinite) {
                milliseconds -= Math.max(0, endTime - startTime);
            }
            msg = get(id, conversation);
        } while (msg == null &&
                 (infinite ||
                  (milliseconds > 0)));
        return msg;
    }

    /** Interrupts the thread, if any, which is waiting for an
        incoming message on the Queue. */
    public synchronized void interrupt() {
        if (waiter != null) {
            waiter.interrupt();
        }
    }
}
