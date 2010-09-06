/*
 * @(#)InvalidTypeException.java	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jdi;

/**
 * Thrown to indicate a type mismatch in setting the value of a field
 * or variable.
 *
 * @author James McIlree
 * @since  1.3
 */
public class InvalidTypeException extends Exception
{
    public InvalidTypeException()
    {
	super();
    }

    public InvalidTypeException(String s)
    {
	super(s);
    }
}
