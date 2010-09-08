/*
 * @(#)GeneralSecurityException.java	1.16 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package java.security;

/**
 * The <code>GeneralSecurityException</code> class is a generic
 * security exception class that provides type safety for all the
 * security-related exception classes that extend from it.
 *
 * @version 1.16, 03/12/19
 * @author Jan Luehe
 */

public class GeneralSecurityException extends Exception {

    private static final long serialVersionUID = 894798122053539237L;

    /** 
     * Constructs a GeneralSecurityException with no detail message.  
     */
    public GeneralSecurityException() {
        super();
    }

    /**
     * Constructs a GeneralSecurityException with the specified detail
     * message.
     * A detail message is a String that describes this particular
     * exception.
     *
     * @param msg the detail message.  
     */
    public GeneralSecurityException(String msg) {
        super(msg);
    }

    /**
     * Creates a <code>GeneralSecurityException</code> with the specified
     * detail message and cause.
     *
     * @param message the detail message (which is saved for later retrieval
     *        by the {@link #getMessage()} method).
     * @param cause the cause (which is saved for later retrieval by the
     *        {@link #getCause()} method).  (A <tt>null</tt> value is permitted,
     *        and indicates that the cause is nonexistent or unknown.)
     * @since 1.5
     */
    public GeneralSecurityException(String message, Throwable cause) {
        super(message, cause);
    }

    /**
     * Creates a <code>GeneralSecurityException</code> with the specified cause
     * and a detail message of <tt>(cause==null ? null : cause.toString())</tt>
     * (which typically contains the class and detail message of
     * <tt>cause</tt>).
     *
     * @param cause the cause (which is saved for later retrieval by the
     *        {@link #getCause()} method).  (A <tt>null</tt> value is permitted,
     *        and indicates that the cause is nonexistent or unknown.)
     * @since 1.5
     */
    public GeneralSecurityException(Throwable cause) {
        super(cause);
    }
}