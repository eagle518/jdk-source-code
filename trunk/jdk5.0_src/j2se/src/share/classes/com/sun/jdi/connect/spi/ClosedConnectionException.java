/*
 * @(#)ClosedConnectionException.java	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jdi.connect.spi;

/**
 * This exception may be thrown as a result of an asynchronous
 * close of a {@link Connection} while an I/O operation is
 * in progress.
 *
 * <p> When a thread is blocked in {@link Connection#readPacket
 * readPacket} waiting for packet from a target VM the
 * {@link Connection} may be closed asynchronous by another
 * thread invokving the {@link Connection#close close} method.
 * When this arises the thread in readPacket will throw this
 * exception. Similiarly when a thread is blocked in 
 * {@link Connection#writePacket} the Connection may be closed.
 * When this occurs the thread in writePacket will throw
 * this exception.
 *
 * @see Connection#readPacket
 * @see Connection#writePacket
 * 
 * @since 1.5
 */
public class ClosedConnectionException extends java.io.IOException {

    /**
     * Constructs a <tt>ClosedConnectionException</tt> with no detail
     * message.
     */
    public ClosedConnectionException() {
    }
 
    /**
     * Constructs a <tt>ClosedConnectionException</tt> with the 
     * specified detail message.
     *
     * @param message the detail message pertaining to this exception.
     */
    public ClosedConnectionException(String message) {
        super(message);
    }

}
