/*
 * @(#)InconsistentDebugInfoException.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jdi;

/**
 * Thrown to indicate that there is an inconistency in the debug
 * information provided by the target VM. For example, this exception
 * is thrown if there is a type mismatch between a retrieved value's
 * runtime type and its declared type as reported by the target VM.
 *
 * @author Gordon Hirsch
 * @since  1.3
 */
public class InconsistentDebugInfoException extends RuntimeException {
    public InconsistentDebugInfoException() {
	super();
    }

    public InconsistentDebugInfoException(String s) {
	super(s);
    }
}
