/*
 * @(#)PendingException.java	1.1 04/03/31
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.util;

/**
 * An exception that denotes that an operation is pending.
 * Currently used by LoginContext.
 *
 * @version 1.1
 */
public class PendingException extends RuntimeException {

    private static final long serialVersionUID = -5201837247928788640L;

    /**
     * Constructs a PendingException with no detail message. A detail
     * message is a String that describes this particular exception.
     */
    public PendingException() {
        super();
    }

    /**
     * Constructs a PendingException with the specified detail message.
     * A detail message is a String that describes this particular
     * exception.
     *
     * <p>
     *
     * @param msg the detail message.
     */
    public PendingException(String msg) {
        super(msg);
    }
}
