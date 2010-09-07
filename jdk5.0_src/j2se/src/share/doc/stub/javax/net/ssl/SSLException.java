/*
 * @(#)SSLException.java	1.8 04/02/16
 *
 * Copyright (c) 2004 Sun Microsystems, Inc. All Rights Reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */
  
/*
 * NOTE:
 * Because of various external restrictions (i.e. US export
 * regulations, etc.), the actual source code can not be provided
 * at this time. This file represents the skeleton of the source
 * file, so that javadocs of the API can be created.
 */

package javax.net.ssl;

import java.io.IOException;

/** 
 * Indicates some kind of error detected by an SSL subsystem.
 * This class is the general class of exceptions produced
 * by failed SSL-related operations.
 *
 * @since 1.4
 * @version 1.13
 * @author David Brownell
 */
public class SSLException extends IOException
{

    /** 
     * Constructs an exception reporting an error found by
     * an SSL subsystem.
     *
     * @param reason describes the problem.
     */
    public SSLException(String reason) { }

    /** 
     * Creates a <code>SSLException</code> with the specified
     * detail message and cause.
     *
     * @param message the detail message (which is saved for later retrieval
     *		by the {@link #getMessage()} method).
     * @param cause the cause (which is saved for later retrieval by the
     *		{@link #getCause()} method).  (A <tt>null</tt> value is
     *		permitted, and indicates that the cause is nonexistent or
     *		unknown.)
     * @since 1.5
     */
    public SSLException(String message, Throwable cause) { }

    /** 
     * Creates a <code>SSLException</code> with the specified cause
     * and a detail message of <tt>(cause==null ? null : cause.toString())</tt>
     * (which typically contains the class and detail message of
     * <tt>cause</tt>).
     *
     * @param cause the cause (which is saved for later retrieval by the
     *		{@link #getCause()} method).  (A <tt>null</tt> value is
     *		permitted, and indicates that the cause is nonexistent or
     *		unknown.)
     * @since 1.5
     */
    public SSLException(Throwable cause) { }
}
