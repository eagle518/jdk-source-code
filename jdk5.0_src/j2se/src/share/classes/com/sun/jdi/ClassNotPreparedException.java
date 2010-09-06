/*
 * @(#)ClassNotPreparedException.java	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jdi;

/**
 * Thrown to indicate that the requested operation cannot be 
 * completed because the specified class has not yet been prepared.
 *
 * @author Gordon Hirsch
 * @since  1.3
 */
public class ClassNotPreparedException extends RuntimeException {
    public ClassNotPreparedException()
    {
	super();
    }

    public ClassNotPreparedException(String s)
    {
	super(s);
    }
}
