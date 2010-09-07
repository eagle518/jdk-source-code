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

class V9FMOVrDecoder extends V9CMoveDecoder {
    private final int opf;
    private final String name;
    private final int dataType;

    V9FMOVrDecoder(int opf, String name, int dataType) {
        this.opf = opf;
        this.name = name;
        this.dataType = dataType;
    }

    Instruction decode(int instruction, SPARCInstructionFactory factory) {
        SPARCV9InstructionFactory v9factory = (SPARCV9InstructionFactory) factory;
        int regConditionCode = getRegisterConditionCode(instruction);
        int rdNum = getDestinationRegister(instruction);
        int rs1Num = getSourceRegister1(instruction);
        int rs2Num = getSourceRegister2(instruction);
        SPARCRegister rd = RegisterDecoder.decode(dataType, rdNum);
        SPARCRegister rs2 = RegisterDecoder.decode(dataType, rs2Num);
        SPARCRegister rs1 = SPARCRegisters.getRegister(rs1Num);
        return v9factory.newV9FMOVrInstruction(name, opf, rs1, (SPARCFloatRegister)rs2,
                              (SPARCFloatRegister)rd, regConditionCode);
    }
}
