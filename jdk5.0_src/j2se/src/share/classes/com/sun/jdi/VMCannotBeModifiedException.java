/*
 * @(#)VMCannotBeModifiedException.java	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jdi;

/**
 * Thrown to indicate that the operation is invalid because it would
 * modify the VM and the VM is read-only.  See {@link VirtualMachine#canBeModified()}.
 *
 * @author Jim Holmlund
 * @since  1.5
 */
public class VMCannotBeModifiedException extends UnsupportedOperationException {
    public VMCannotBeModifiedException() {
	super();
    }

    public VMCannotBeModifiedException(String s) {
	super(s);
    }
}
