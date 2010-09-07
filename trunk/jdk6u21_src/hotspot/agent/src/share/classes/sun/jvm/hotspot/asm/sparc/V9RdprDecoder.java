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

class V9RdprDecoder extends V9PrivilegedReadWriteDecoder {
    Instruction decode(int instruction, SPARCInstructionFactory factory) {
        SPARCV9InstructionFactory v9factory = (SPARCV9InstructionFactory) factory;
        Instruction instr = null;
        int prNum = getSourceRegister1(instruction);
        if (isLegalPrivilegedRegister(prNum)) {
            SPARCRegister rd = SPARCRegisters.getRegister(getDestinationRegister(instruction));
            instr = v9factory.newV9RdprInstruction(prNum, rd);
        } else {
            instr = v9factory.newIllegalInstruction(instruction);
        }

        return instr;
    }
}
