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
import sun.jvm.hotspot.utilities.Assert;

class FPMoveDecoder extends FloatDecoder {

    FPMoveDecoder(int opf, String name, int srcType, int resultType) {
        super(opf, name, srcType, resultType);
    }

    Instruction decodeFloatInstruction(int instruction,
                                       SPARCRegister rs1, SPARCRegister rs2,
                                       SPARCRegister rd,
                                       SPARCInstructionFactory factory) {
        if (Assert.ASSERTS_ENABLED)
            Assert.that(rs2.isFloat() && rd.isFloat(), "rs2, rd have to be float registers");

        return factory.newFPMoveInstruction(name, opf, (SPARCFloatRegister)rs2, (SPARCFloatRegister)rd);
    }
}
