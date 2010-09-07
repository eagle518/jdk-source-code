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

class V9ShiftDecoder extends InstructionDecoder
                     implements V9InstructionDecoder {
    final int op3;
    final String name;
    final int rtlOperation;

    V9ShiftDecoder(int op3, String name, int rtlOperation) {
        this.op3 = op3;
        this.name = name;
        this.rtlOperation = rtlOperation;
    }

    static boolean isXBitSet(int instruction) {
        return (instruction & X_MASK) != 0;
    }

    Instruction decode(int instruction,
                       SPARCInstructionFactory factory) {
        SPARCRegister rs1 = SPARCRegisters.getRegister(getSourceRegister1(instruction));
        SPARCRegister rd = SPARCRegisters.getRegister(getDestinationRegister(instruction));
        boolean xBit = isXBitSet(instruction);
        ImmediateOrRegister operand2 = null;

        if (isIBitSet(instruction)) {
            // look for 64 bits shift operations.
            int value = instruction & ( xBit ? SHIFT_COUNT_6_MASK : SHIFT_COUNT_5_MASK);
            operand2 = new Immediate(new Short((short) value));
        } else {
            operand2 = SPARCRegisters.getRegister(getSourceRegister2(instruction));
        }

        return factory.newShiftInstruction(xBit? name + "x" : name, op3, rtlOperation, rs1, operand2, rd);
    }
}
