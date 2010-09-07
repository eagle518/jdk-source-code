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

abstract class FPopDecoder extends InstructionDecoder {
    abstract InstructionDecoder getOpfDecoder(int opf);

    Instruction decode(int instruction, SPARCInstructionFactory factory) {
        int opf = getOpf(instruction);
        InstructionDecoder decoder = getOpfDecoder(opf);
        return (decoder == null) ? factory.newIllegalInstruction(instruction)
                                 : decoder.decode(instruction, factory);
    }
}
