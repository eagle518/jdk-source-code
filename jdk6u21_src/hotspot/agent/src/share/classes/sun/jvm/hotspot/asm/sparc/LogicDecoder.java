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

class LogicDecoder extends Format3ADecoder {
    LogicDecoder(int op3, String name, int rtlOperation) {
        super(op3, name, rtlOperation);
    }

    Instruction decodeFormat3AInstruction(int instruction,
                                           SPARCRegister rs1,
                                           ImmediateOrRegister operand2,
                                           SPARCRegister rd,
                                           SPARCInstructionFactory factory) {
        Instruction instr = null;
        if (op3 == OR && rs1 == SPARCRegisters.G0 && rd != SPARCRegisters.G0) {
            instr = factory.newMoveInstruction(name, op3, operand2, rd);
        } else {
            instr = factory.newLogicInstruction(name, op3, rtlOperation, rs1, operand2, rd);
        }
        return instr;
    }
}
