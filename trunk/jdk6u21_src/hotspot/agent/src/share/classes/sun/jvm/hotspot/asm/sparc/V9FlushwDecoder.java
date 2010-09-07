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

class V9FlushwDecoder extends InstructionDecoder
               implements V9InstructionDecoder {
    Instruction decode(int instruction, SPARCInstructionFactory factory) {
        SPARCV9InstructionFactory v9factory = (SPARCV9InstructionFactory) factory;
        Instruction instr = null;
        // "i" bit has to be zero. see page 169 - A.21 Flush Register Windows.
        if (isIBitSet(instruction)) {
            instr = v9factory.newIllegalInstruction(instruction);
        } else {
            instr = v9factory.newV9FlushwInstruction();
        }
        return instr;
    }
}
