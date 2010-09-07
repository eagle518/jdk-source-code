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

class AlternateSpaceStoreDecoder extends StoreDecoder {
    AlternateSpaceStoreDecoder(int op3, String name, int dataType) {
        super(op3, name, dataType);
    }

    protected Instruction decodeMemoryInstruction(int instruction,
                                               SPARCRegisterIndirectAddress addr,
                                               SPARCRegister rd,
                                               SPARCInstructionFactory factory) {
        setAddressSpace(instruction, addr);
        return factory.newStoreInstruction(name, op3, addr, rd, dataType);
    }
}
