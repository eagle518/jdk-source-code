/*
 * @(#)VoidValueImpl.java	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdi;

import com.sun.jdi.*;

public class VoidValueImpl extends ValueImpl implements VoidValue {

    VoidValueImpl(VirtualMachine aVm) {
        super(aVm);
    }

    public boolean equals(Object obj) {
        return (obj != null) && (obj instanceof VoidValue) && super.equals(obj);
    }

    public int hashCode() {
        /*
         * TO DO: Better hash code
         */
        return 47245;
    }

    public Type type() {
        return vm.theVoidType();
    }

    ValueImpl prepareForAssignmentTo(ValueContainer destination)
                    throws InvalidTypeException {

        throw new InvalidTypeException();
    }

    public String toString() {
        return "<void value>";
    }

    byte typeValueKey() {
        return JDWP.Tag.VOID;
    }
}
