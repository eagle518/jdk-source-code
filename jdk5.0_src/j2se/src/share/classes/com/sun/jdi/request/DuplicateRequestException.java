/*
 * @(#)DuplicateRequestException.java	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jdi.request;

/**
 * Thrown to indicate a duplicate event request.
 *
 * @author Robert Field
 * @since  1.3
 */
public class DuplicateRequestException extends RuntimeException
{
    public DuplicateRequestException()
    {
	super();
    }

    public DuplicateRequestException(String s)
    {
	super(s);
    }
}
