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

class ShiftDecoder extends InstructionDecoder {
    final int op3;
    final String name;
    final int rtlOperation;

    ShiftDecoder(int op3, String name, int rtlOperation) {
        this.op3 = op3;
        this.name = name;
        this.rtlOperation = rtlOperation;
    }

    private ImmediateOrRegister getShiftLength(int instruction) {
        boolean iBit = isIBitSet(instruction);
        ImmediateOrRegister operand2 = null;
        if (iBit) {
            int value = instruction & SHIFT_COUNT_5_MASK;
            operand2 = new Immediate(new Short((short)value));
        } else {
            operand2 = SPARCRegisters.getRegister(getSourceRegister2(instruction));
        }
        return operand2;
    }

    Instruction decode(int instruction,
                       SPARCInstructionFactory factory) {
        SPARCRegister rs1 = SPARCRegisters.getRegister(getSourceRegister1(instruction));
        SPARCRegister rd = SPARCRegisters.getRegister(getDestinationRegister(instruction));
        ImmediateOrRegister operand2 = getShiftLength(instruction);
        return factory.newShiftInstruction(name, op3, rtlOperation, rs1, operand2, rd);
    }
}
