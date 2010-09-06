/*
 * @(#)InvocationException.java	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jdi;

/**
 * Thrown to indicate an exception occurred in an invoked method within
 * the target VM.
 *
 * @author Gordon Hirsch
 * @since  1.3
 */
public class InvocationException extends Exception
{
    ObjectReference exception;

    public InvocationException(ObjectReference exception)
    {
	super("Exception occurred in target VM");
        this.exception = exception;
    }

    public ObjectReference exception()
    {
        return exception;
    }
}
