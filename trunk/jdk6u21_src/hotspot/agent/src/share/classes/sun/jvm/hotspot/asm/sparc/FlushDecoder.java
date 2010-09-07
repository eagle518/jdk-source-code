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

class FlushDecoder extends MemoryInstructionDecoder {
    FlushDecoder() {
        super(FLUSH, "flush", RTLDT_UNKNOWN);
    }

    Instruction decodeMemoryInstruction(int instruction, SPARCRegisterIndirectAddress addr,
                      SPARCRegister rd, SPARCInstructionFactory factory) {
        return factory.newFlushInstruction(addr);
    }
}
