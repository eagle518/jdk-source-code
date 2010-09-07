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

class V9AlternateSpacePrefetchDecoder extends V9AlternateSpaceDecoder {
    V9AlternateSpacePrefetchDecoder() {
      // Fake the destination with an integer type so we can get fcn from rd
      super(PREFETCHA, "prefetcha", RTLDT_SIGNED_WORD);
    }

    Instruction decodeV9AsiLoadStore(int instruction,
                                     SPARCV9RegisterIndirectAddress addr,
                                     SPARCRegister rd,
                                     SPARCV9InstructionFactory factory) {
        SPARCV9InstructionFactory v9factory = (SPARCV9InstructionFactory) factory;
        return v9factory.newV9PrefetchInstruction(name, addr, rd.getNumber());
    }
}
