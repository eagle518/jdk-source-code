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

abstract class FloatDecoder extends InstructionDecoder {
    final int    opf;
    final String name;
    final int    numSources; // 1 or 2;
    final int    src1Type;   // RTLDT_FL_SINGLE, _DOUBLE, _QUAD
    final int    src2Type;   // RTLDT_FL_SINGLE, _DOUBLE, _QUAD
    final int    resultType; // RTLDT_FL_SINGLE, _DOUBLE, _QUAD

    FloatDecoder(int opf, String name, int src1Type, int src2Type, int resultType) {
        this.opf = opf;
        this.name = name;
        numSources = 2;
        this.src1Type = src1Type;
        this.src2Type = src2Type;
        this.resultType = resultType;
    }

    FloatDecoder(int opf, String name, int src2Type, int resultType) {
        this.opf = opf;
        this.name = name;
        numSources = 1;
        this.src1Type = RTLOP_UNKNOWN;
        this.src2Type = src2Type;
        this.resultType = resultType;
    }

    abstract Instruction decodeFloatInstruction(int instruction,
                               SPARCRegister rs1, SPARCRegister rs2, SPARCRegister rd,
                               SPARCInstructionFactory factory);

    Instruction decode(int instruction, SPARCInstructionFactory factory) {
        int rs1Num = getSourceRegister1(instruction);
        int rs2Num = getSourceRegister2(instruction);
        int rdNum = getDestinationRegister(instruction);

        SPARCRegister rs1 = null;
        if (numSources == 2) {
           rs1 = RegisterDecoder.decode(src1Type, rs1Num);
           if (rs1 == null) {
              return factory.newIllegalInstruction(instruction);
           }
        }

        SPARCRegister rd = RegisterDecoder.decode(resultType, rdNum);
        SPARCRegister rs2 = RegisterDecoder.decode(src2Type, rs2Num);
        if (rd == null || rs2 == null) {
           return factory.newIllegalInstruction(instruction);
        }

        return decodeFloatInstruction(instruction, rs1, rs2, rd, factory);
    }
}
