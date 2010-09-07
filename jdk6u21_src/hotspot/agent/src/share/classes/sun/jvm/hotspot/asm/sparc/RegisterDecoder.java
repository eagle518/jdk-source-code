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

import sun.jvm.hotspot.asm.RTLDataTypes;

class RegisterDecoder implements /* imports */ RTLDataTypes {
    // refer to page 40 - section 5.1.4.1 - Floating-point Register Number Encoding
    private static SPARCFloatRegister decodeDouble(int num) {
        // 6 bit double precision registers are encoded in 5 bits as
        // b<4> b<3> b<2> b<1> b<5>.

        boolean lsb = (0x1 & num) != 0;
        if (lsb)
            num |= 0x20;  // 10000b

        if ((num % 2) != 0)
            return null;

        return SPARCFloatRegisters.getRegister(num);
    }

    private static SPARCFloatRegister decodeQuad(int num) {
        // 6 bit quad precision registers are encoded in 5 bits as
        // b<4> b<3> b<2> 0 b<5>

        boolean lsb = (0x1 & num) != 0;
        if (lsb)
            num |= 0x20; // 10000b

        if ((num % 4) != 0)
            return null;

        return SPARCFloatRegisters.getRegister(num);
    }

    static SPARCRegister decode(int dataType, int regNum) {
        regNum &= 0x1F; // mask out all but lsb 5 bits
        SPARCRegister result = null;
        switch (dataType) {
            case RTLDT_FL_SINGLE:
                result = SPARCFloatRegisters.getRegister(regNum);
                break;

            case RTLDT_FL_DOUBLE:
                result = decodeDouble(regNum);
                break;

            case RTLDT_FL_QUAD:
                result = decodeQuad(regNum);
                break;

            case RTLDT_UNKNOWN:
                result = null;
                break;

            default: // some integer register
                result = SPARCRegisters.getRegister(regNum);
                break;
        }
        return result;
    }
}
