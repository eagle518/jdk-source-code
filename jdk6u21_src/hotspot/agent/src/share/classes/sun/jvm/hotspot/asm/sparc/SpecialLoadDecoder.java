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

class SpecialLoadDecoder extends SpecialLoadStoreDecoder {
    SpecialLoadDecoder(int op3, String name, int specialRegNum) {
        super(op3, name, specialRegNum);
    }

    Instruction decodeSpecialLoadStoreInstruction(int cregNum,
                                    SPARCRegisterIndirectAddress addr,
                                    SPARCInstructionFactory factory) {
        return factory.newSpecialLoadInstruction(name, specialRegNum, cregNum, addr);
    }
}
