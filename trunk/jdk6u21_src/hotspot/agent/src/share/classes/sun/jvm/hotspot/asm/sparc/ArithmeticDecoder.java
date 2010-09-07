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

import sun.jvm.hotspot.asm.*;

class ArithmeticDecoder extends Format3ADecoder {
    ArithmeticDecoder(int op3, String name, int rtlOperation) {
        super(op3, name, rtlOperation);
    }

    Instruction decodeFormat3AInstruction(int instruction,
                                       SPARCRegister rs1,
                                       ImmediateOrRegister operand2,
                                       SPARCRegister rd,
                                       SPARCInstructionFactory factory) {
        return factory.newArithmeticInstruction(name, op3, rtlOperation, rs1, operand2, rd);
    }
}
