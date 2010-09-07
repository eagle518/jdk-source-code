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

class LdstubDecoder extends MemoryInstructionDecoder {
    LdstubDecoder(int op3, String name, int dataType) {
        super(op3, name, dataType);
    }

    Instruction decodeMemoryInstruction(int instruction,
                                     SPARCRegisterIndirectAddress addr,
                                     SPARCRegister rd,
                                     SPARCInstructionFactory factory) {
        return factory.newLdstubInstruction(name, addr, rd);
    }
}
