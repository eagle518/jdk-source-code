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

abstract class V9RegisterBranchDecoder extends V9BranchDecoder {
    static int getDisp16(int instruction) {
        int offset = (DISP_16_LO_MASK & instruction)  |
           ((DISP_16_HI_MASK & instruction) >>> (DISP_16_HI_START_BIT - DISP_16_LO_NUMBITS));

        // sign extend and word align
        offset = extractSignedIntFromNBits(offset, 16);
        offset <<= 2;

        return offset;
    }

    String getConditionName(int conditionCode, boolean isAnnuled) {
        return null;
    }

    abstract String getRegisterConditionName(int rcond);

    public Instruction decode(int instruction, SPARCInstructionFactory factory) {
        SPARCV9InstructionFactory v9factory = (SPARCV9InstructionFactory) factory;
        int rcond = (BRANCH_RCOND_MASK & instruction) >>> BRANCH_RCOND_START_BIT;
        if (rcond == BRANCH_RCOND_RESERVED1 || rcond == BRANCH_RCOND_RESERVED2)
            return factory.newIllegalInstruction(instruction);

        SPARCRegister rs1 = SPARCRegisters.getRegister(getSourceRegister1(instruction));
        boolean predictTaken = getPredictTaken(instruction);
        boolean annuled = getAnnuledBit(instruction);
        PCRelativeAddress addr = new PCRelativeAddress(getDisp16(instruction));
        String name = getRegisterConditionName(rcond);
        return v9factory.newV9RegisterBranchInstruction(name, addr, annuled, rcond, rs1, predictTaken);
    }
}
