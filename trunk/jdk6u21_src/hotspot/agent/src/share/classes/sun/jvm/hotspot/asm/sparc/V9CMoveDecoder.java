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

abstract class V9CMoveDecoder extends InstructionDecoder
                   implements V9InstructionDecoder {
    static private final String iccConditionNames[] = {
        "n", "e", "le", "l", "leu", "cs", "neg", "vs",
        "a", "ne", "g", "ge", "gu", "cc", "pos", "vc"
    };

    static private final String fccConditionNames[] = {
        "fn", "fne", "flg", "ful", "fl", "fug", "fg", "fu",
        "fa", "fe",  "fue", "fge", "fuge", "fle", "fule", "fo"
    };

    static String getConditionName(int conditionCode, int conditionFlag) {
        return (conditionFlag == icc || conditionFlag == xcc) ?
                       iccConditionNames[conditionCode]
                     : fccConditionNames[conditionCode];
    }

    static int getMoveConditionCode(int instruction) {
        return (instruction & CMOVE_COND_MASK) >>> CMOVE_COND_START_BIT;
    }

    static int getRegisterConditionCode(int instruction) {
        return (instruction & CMOVE_RCOND_MASK) >>> CMOVE_RCOND_START_BIT;
    }

    static ImmediateOrRegister getCMoveSource(int instruction, int numBits) {
        ImmediateOrRegister source = null;
        if (isIBitSet(instruction)) {
            source = new Immediate(new Short((short) extractSignedIntFromNBits(instruction, numBits)));
        } else {
            source = SPARCRegisters.getRegister(getSourceRegister2(instruction));
        }
        return source;
    }

    static String getFloatTypeCode(int dataType) {
        String result = null;
        switch(dataType) {
            case RTLDT_FL_SINGLE:
                result = "s";
                break;

            case RTLDT_FL_DOUBLE:
                result = "d";
                break;

            case RTLDT_FL_QUAD:
                result = "q";
                break;

            default:
                if (Assert.ASSERTS_ENABLED)
                    Assert.that(false, "should not reach here");
        }
        return result;
    }
}
