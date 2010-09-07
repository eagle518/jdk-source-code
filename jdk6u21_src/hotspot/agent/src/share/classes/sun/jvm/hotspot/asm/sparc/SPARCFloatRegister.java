/*
 * Copyright (c) 2002, Oracle and/or its affiliates. All rights reserved.
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

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.Register;
import sun.jvm.hotspot.utilities.Assert;

public class SPARCFloatRegister extends SPARCRegister {

    public SPARCFloatRegister(int number) {
        super(number);
    }

    public int getNumber() {
        return number;
    }

    public static final int SINGLE_PRECISION = 1;
    public static final int DOUBLE_PRECISION = 2;
    public static final int QUAD_PRECISION = 3;

    public int getNumber(int width) {
        switch (width) {
        case SINGLE_PRECISION:
            Assert.that(number < 32, "bad single-prec fp register");
            return number;

        case DOUBLE_PRECISION:
            Assert.that(number < 64 && (number & 1) == 0, "bad double-prec fp register");
            return number & 0x1e | (number & 0x20) >> 5;

        case QUAD_PRECISION:
            Assert.that(number < 64 && (number & 3) == 0, "bad quad-prec fp register");
            return number & 0x1c | (number & 0x20) >> 5;
        }
        throw new RuntimeException("Invalid floating point width supplied");
    }

    private static final int nofRegisters = 63;
    public int getNumberOfRegisters() {
        return nofRegisters;
    }

    public boolean isFloat() {
        return true;
    }

    public boolean isFramePointer() {
        return false;
    }

    public boolean isStackPointer() {
        return false;
    }

    public boolean isV9Only() {
        return number > 31;
    }

    public boolean isValid() {
        return number >= 0 && number < nofRegisters;
    }

    public String toString() {
        return SPARCFloatRegisters.getRegisterName(number);
    }

}
