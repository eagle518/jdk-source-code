/*
 * Copyright (c) 2002, 2003, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

package sun.jvm.hotspot.jdi;

import com.sun.jdi.*;

public class VoidValueImpl extends ValueImpl implements VoidValue {

    VoidValueImpl(VirtualMachine aVm) {
        super(aVm);
    }

    public boolean equals(Object obj) {
        return (obj != null) && (obj instanceof VoidValue) && super.equals(obj);
    }

    public int hashCode() {
        return type().hashCode();
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
}
