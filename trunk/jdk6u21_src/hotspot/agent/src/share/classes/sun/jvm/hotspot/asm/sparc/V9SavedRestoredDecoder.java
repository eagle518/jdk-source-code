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

class V9SavedRestoredDecoder extends InstructionDecoder
              implements V9InstructionDecoder {
    Instruction decode(int instruction, SPARCInstructionFactory factory) {
        SPARCV9InstructionFactory v9factory = (SPARCV9InstructionFactory) factory;
        Instruction instr = null;
        int rdNum = getDestinationRegister(instruction);
        // "rd" field is "fcn". Only values 0 and 1 are defined.
        // see page 219 - A.47 Saved and Restored
        switch (rdNum) {
            case 0:
                instr = v9factory.newV9SavedInstruction();
                break;
            case 1:
                instr = v9factory.newV9RestoredInstruction();
                break;
            default:
                instr = v9factory.newIllegalInstruction(instruction);
                break;
        }

        return instr;
    }
}
