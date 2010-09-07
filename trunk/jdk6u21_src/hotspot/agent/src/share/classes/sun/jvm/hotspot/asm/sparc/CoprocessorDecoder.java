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

class CoprocessorDecoder extends InstructionDecoder {
    private int op3;
    CoprocessorDecoder(int op3) {
        this.op3 = op3;
    }

    Instruction decode(int instruction, SPARCInstructionFactory factory) {
        int rs1Num = getSourceRegister1(instruction);
        int rs2Num = getSourceRegister2(instruction);
        int rdNum = getDestinationRegister(instruction);

        return factory.newCoprocessorInstruction(instruction, op3,
                                     (instruction & OPC_MASK) >> OPF_START_BIT,
                                     rs1Num, rs2Num, rdNum);
    }
}
