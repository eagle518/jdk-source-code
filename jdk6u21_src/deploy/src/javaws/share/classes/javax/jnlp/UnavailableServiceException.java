/*
 * @(#)UnavailableServiceException.java	1.12 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package javax.jnlp;

/**
 *  The <code>UnavailableServiceException</code> is thrown by the
 *  <code>ServiceManager</code> when a non-existing or unavailable service
 *  is looked up.
 *
 * @since 1.0
 *
 * @see     ServiceManager
 */
public class UnavailableServiceException extends Exception {
    
    /**
     * Constructs an <code>UnavailableServiceException</code> with <code>null</code>
     * as its error detail message.
     */
    public UnavailableServiceException() {
	super();
    }
    
    /**
     * Constructs an <code>UnavailableServiceException</code> with the specified detail
     * message. The error message string <code>s</code> can later be
     * retrieved by the <code>{@link java.lang.Throwable#getMessage}</code>
     * method of class <code>java.lang.Throwable</code>.
     *
     * @param msg the detail message.
     */
    public UnavailableServiceException(String msg) {
	super(msg);
    }
}
