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

class V9MOVrDecoder extends V9CMoveDecoder {
    private static final String regConditionNames[] = {
        null, "movrz", "movrlez", "movrlz", null, "movrnz", "movrgz", "movrgez"
    };

    private static String getMOVrName(int conditionCode) {
        return regConditionNames[conditionCode];
    }

    Instruction decode(int instruction, SPARCInstructionFactory factory) {
        SPARCV9InstructionFactory v9factory = (SPARCV9InstructionFactory) factory;
        Instruction instr = null;
        int regConditionCode = getRegisterConditionCode(instruction);
        String name = getMOVrName(regConditionCode);
        if (name == null) {
            instr = v9factory.newIllegalInstruction(instruction);
        } else {
            int rdNum = getDestinationRegister(instruction);
            SPARCRegister rd = SPARCRegisters.getRegister(rdNum);
            SPARCRegister rs1 = SPARCRegisters.getRegister(getSourceRegister1(instruction));
            ImmediateOrRegister operand2 = getCMoveSource(instruction, 10);
            instr = v9factory.newV9MOVrInstruction(name, rs1, operand2, rd, regConditionCode);
        }

        return instr;
    }
}
