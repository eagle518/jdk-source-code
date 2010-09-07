/*
 * Copyright (c) 2003, Oracle and/or its affiliates. All rights reserved.
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

package sun.jvm.hotspot.asm.ia64;

import sun.jvm.hotspot.asm.Register;
import sun.jvm.hotspot.utilities.Assert;

public class IA64FloatRegister extends IA64Register {

    public IA64FloatRegister(int number) {
        super(number);
    }

    public int getNumber() {
        return number;
    }

    public static final int SINGLE_PRECISION = 1;
    public static final int DOUBLE_PRECISION = 2;
    public static final int QUAD_PRECISION = 3;

    public int getNumber(int width) {
        return number;
    }

    private static final int nofRegisters = 128;
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

    public boolean isValid() {
        return number >= 0 && number < nofRegisters;
    }

    public String toString() {
        return IA64FloatRegisters.getRegisterName(number);
    }

}
