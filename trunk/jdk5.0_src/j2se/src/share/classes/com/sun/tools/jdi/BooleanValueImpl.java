/*
 * @(#)BooleanValueImpl.java	1.15 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdi;

import com.sun.jdi.*;

public class BooleanValueImpl extends PrimitiveValueImpl
                              implements BooleanValue {
    private boolean value;

    BooleanValueImpl(VirtualMachine aVm,boolean aValue) {
        super(aVm);

        value = aValue;
    }

    public boolean equals(Object obj) {
        if ((obj != null) && (obj instanceof BooleanValue)) {
            return (value == ((BooleanValue)obj).value())
                   && super.equals(obj);
        } else {
            return false;
        }
    }

    public int hashCode() {
        /*
         * TO DO: Better hash code
         */
        return intValue();
    }

    public Type type() {
        return vm.theBooleanType();
    }

    public boolean value() {
        return value;
    }

    public boolean booleanValue() {
        return value;
    }

    public byte byteValue() {
        return(byte)((value)?1:0);
    }

    public char charValue() {
        return(char)((value)?1:0);
    }

    public short shortValue() {
        return(short)((value)?1:0);
    }

    public int intValue() {
        return(int)((value)?1:0);
    }

    public long longValue() {
        return(long)((value)?1:0);
    }

    public float floatValue() {
        return(float)((value)?1.0:0.0);
    }

    public double doubleValue() {
        return(double)((value)?1.0:0.0);
    }

    public String toString() {
        return "" + value;
    }

    byte typeValueKey() {
        return JDWP.Tag.BOOLEAN;
    }
}
