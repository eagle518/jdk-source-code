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

class V9FloatBranchDecoder extends V9CCBranchDecoder {
    String getConditionName(int conditionCode, boolean isAnnuled) {
        return isAnnuled ? floatAnnuledConditionNames[conditionCode]
                         : floatConditionNames[conditionCode];
    }

    int getConditionFlag(int instruction) {
        return (FBPfcc_CC_MASK & instruction) >>> FBPfcc_CC_START_BIT;
    }
}
