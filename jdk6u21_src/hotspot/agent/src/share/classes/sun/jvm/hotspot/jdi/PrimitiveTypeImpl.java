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

abstract class PrimitiveTypeImpl extends TypeImpl implements PrimitiveType {

    PrimitiveTypeImpl(VirtualMachine vm) {
        super(vm);
    }

    /*
     * Converts the given primitive value to a value of this type.
     */
    abstract PrimitiveValue convert(PrimitiveValue value) throws InvalidTypeException;

    public String toString() {
        return name();
    }
}
