/*
 * Copyright (c) 2002, 2003, Oracle and/or its affiliates. All rights reserved.
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

class V9PrefetchDecoder extends MemoryInstructionDecoder
           implements V9InstructionDecoder {
    V9PrefetchDecoder() {
      // Fake the destination with an integer type so we can get fcn from rd
      super(PREFETCH, "prefetch", RTLDT_SIGNED_WORD);
    }

    Instruction decodeMemoryInstruction(int instruction,
                                   SPARCRegisterIndirectAddress addr,
                                   SPARCRegister rd, SPARCInstructionFactory factory) {
        SPARCV9InstructionFactory v9factory = (SPARCV9InstructionFactory) factory;
        return v9factory.newV9PrefetchInstruction(name, addr, rd.getNumber());
    }
}
