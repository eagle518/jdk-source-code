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

class JmplDecoder extends MemoryInstructionDecoder {
    JmplDecoder() {
        super(JMPL, "jmpl", RTLDT_UNSIGNED_WORD);
    }

    Instruction decodeMemoryInstruction(int instruction, SPARCRegisterIndirectAddress addr,
                       SPARCRegister rd,  SPARCInstructionFactory factory) {
        // this may be most probably indirect call or ret or retl
        Instruction instr = null;
        if (rd == SPARCRegisters.O7) {
            instr = factory.newIndirectCallInstruction(addr, rd);
        } else if (rd == SPARCRegisters.G0) {
            int disp = (int) addr.getDisplacement();
            Register base = addr.getBase();
            if (base == SPARCRegisters.I7 && disp == 8) {
                instr = factory.newReturnInstruction(addr, rd, false /* not leaf */);
            } else if (base == SPARCRegisters.O7 && disp == 8) {
                instr = factory.newReturnInstruction(addr, rd, true /* leaf */);
            } else {
                instr = factory.newJmplInstruction(addr, rd);
            }
        } else {
            instr = factory.newJmplInstruction(addr, rd);
        }
        return instr;
    }
}
