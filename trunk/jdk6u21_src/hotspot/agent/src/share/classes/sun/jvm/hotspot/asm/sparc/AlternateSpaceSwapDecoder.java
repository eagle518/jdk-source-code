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

class AlternateSpaceSwapDecoder extends SwapDecoder {
    AlternateSpaceSwapDecoder(int op3, String name, int dataType) {
        super(op3, name, dataType);
    }

    Instruction decodeMemoryInstruction(int instruction,
                                     SPARCRegisterIndirectAddress addr,
                                     SPARCRegister rd,
                                     SPARCInstructionFactory factory) {
        setAddressSpace(instruction, addr);
        return factory.newSwapInstruction(name, addr, rd);
    }
}
