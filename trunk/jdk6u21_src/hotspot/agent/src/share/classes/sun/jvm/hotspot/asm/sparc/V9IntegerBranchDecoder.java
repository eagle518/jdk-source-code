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

class V9IntegerBranchDecoder extends V9CCBranchDecoder {
    String getConditionName(int conditionCode, boolean isAnnuled) {
        return isAnnuled ? integerAnnuledConditionNames[conditionCode]
                         : integerConditionNames[conditionCode];
    }

    int getConditionFlag(int instruction) {
        return ((BPcc_CC_MASK & instruction) >>> BPcc_CC_START_BIT) + icc;
    }
}
