/*
 * @(#)ValueImpl.java	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdi;

import com.sun.jdi.*;

abstract class ValueImpl extends MirrorImpl implements Value {

    ValueImpl(VirtualMachine aVm) {
        super(aVm);
    }

    static ValueImpl prepareForAssignment(Value value,
                                          ValueContainer destination)
                  throws InvalidTypeException, ClassNotLoadedException {
        if (value == null) {
            /*
             * TO DO: Centralize JNI signature knowledge
             */
            if (destination.signature().length() == 1) {
                throw new InvalidTypeException("Can't set a primitive type to null");
            }
            return null;    // no further checking or conversion necessary
        } else {
            return ((ValueImpl)value).prepareForAssignmentTo(destination);
        }
    }

    static byte typeValueKey(Value val) {
        if (val == null) {
            return JDWP.Tag.OBJECT;
        } else {
            return ((ValueImpl)val).typeValueKey();
        }
    }

    abstract ValueImpl prepareForAssignmentTo(ValueContainer destination)
                 throws InvalidTypeException, ClassNotLoadedException;

    abstract byte typeValueKey();
}


