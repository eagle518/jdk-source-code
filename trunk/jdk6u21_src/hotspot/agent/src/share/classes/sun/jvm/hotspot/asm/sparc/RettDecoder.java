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

class RettDecoder extends MemoryInstructionDecoder {
    RettDecoder() {
        super(RETT, "rett", RTLDT_UNKNOWN);
    }

    Instruction decodeMemoryInstruction(int instruction, SPARCRegisterIndirectAddress addr,
                      SPARCRegister rd, SPARCInstructionFactory factory) {
        return factory.newRettInstruction(addr);
    }
}
