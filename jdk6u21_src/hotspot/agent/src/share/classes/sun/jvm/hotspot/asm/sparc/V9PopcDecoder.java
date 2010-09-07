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

class V9PopcDecoder extends InstructionDecoder
              implements V9InstructionDecoder {
    Instruction decode(int instruction, SPARCInstructionFactory factory) {
        SPARCV9InstructionFactory v9factory = (SPARCV9InstructionFactory) factory;
        int rs1Num = getSourceRegister1(instruction);
        Instruction instr = null;
        // in POPC, rs1 should be zero. see page 205 - A.41 Population Count
        if (rs1Num != 0) {
            instr = v9factory.newIllegalInstruction(instruction);
        } else {
            SPARCRegister rd = SPARCRegisters.getRegister(getDestinationRegister(instruction));
            instr = v9factory.newV9PopcInstruction(getOperand2(instruction), rd);
        }
        return instr;
    }
}
