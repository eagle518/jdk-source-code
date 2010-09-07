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

class V9MOVccDecoder extends V9CMoveDecoder {
    private static String getMoveCCName(int conditionCode, int conditionFlag) {
        return "mov" + getConditionName(conditionCode, conditionFlag);
    }

    private static int getMoveConditionFlag(int instruction) {
        boolean cc2Bit = (instruction & CMOVE_CC2_MASK) != 0;
        int conditionFlag = (instruction & CMOVE_CC0_CC1_MASK) >>> CMOVE_CC_START_BIT;
        if (cc2Bit) conditionFlag |= (0x4); // 100;
        return conditionFlag;
    }

    Instruction decode(int instruction, SPARCInstructionFactory factory) {
        SPARCV9InstructionFactory v9factory = (SPARCV9InstructionFactory) factory;
        Instruction instr = null;
        int conditionFlag = getMoveConditionFlag(instruction);
        if (conditionFlag == CFLAG_RESERVED1 || conditionFlag == CFLAG_RESERVED2) {
            instr = v9factory.newIllegalInstruction(instruction);
        } else {
            int rdNum = getDestinationRegister(instruction);
            SPARCRegister rd = SPARCRegisters.getRegister(rdNum);
            int conditionCode = getMoveConditionCode(instruction);
            ImmediateOrRegister source = getCMoveSource(instruction, 11);
            String name = getMoveCCName(conditionCode, conditionFlag);
            instr = v9factory.newV9MOVccInstruction(name, conditionCode, conditionFlag, source, rd);
        }

        return instr;
    }
}
