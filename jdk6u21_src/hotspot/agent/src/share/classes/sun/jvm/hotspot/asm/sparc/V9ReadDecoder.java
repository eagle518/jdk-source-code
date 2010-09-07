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

class V9ReadDecoder extends InstructionDecoder
              implements V9InstructionDecoder {
    Instruction decode(int instruction, SPARCInstructionFactory factory) {
        SPARCV9InstructionFactory v9factory = (SPARCV9InstructionFactory) factory;
        Instruction instr = null;
        int specialRegNum = getSourceRegister1(instruction);

        // rs1 values 1, 7-14 are reserved - see page 214, A.44 Read State Register.
        if (specialRegNum == 1 || (specialRegNum > 6 && specialRegNum < 15)) {
            instr = v9factory.newIllegalInstruction(instruction);
        } else {
            int rdNum = getDestinationRegister(instruction);
            if (specialRegNum == 15) {
                // may be stbar, member or illegal
                if (rdNum == 0) {
                    boolean iBit = isIBitSet(instruction);
                    if (iBit) {
                        instr = v9factory.newV9MembarInstruction((instruction & MMASK_MASK) >>> MMASK_START_BIT,
                                                        (instruction & CMASK_MASK) >>> CMASK_START_BIT);
                    } else {
                        instr = v9factory.newStbarInstruction();
                    }
                } else { // rd != 0 && rs1 == 15
                    instr = v9factory.newIllegalInstruction(instruction);
                }
             } else {
                int asrRegNum = -1;
                if (specialRegNum > 15){
                    asrRegNum = specialRegNum;
                    specialRegNum = SPARCV9SpecialRegisters.ASR;
                }
                SPARCRegister rd = SPARCRegisters.getRegister(rdNum);
                instr = v9factory.newV9ReadInstruction(specialRegNum, asrRegNum, rd);
             }
        }
        return instr;
    }
}
