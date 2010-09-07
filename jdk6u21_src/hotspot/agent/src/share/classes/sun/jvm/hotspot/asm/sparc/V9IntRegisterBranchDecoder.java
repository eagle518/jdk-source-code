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

class V9IntRegisterBranchDecoder extends V9RegisterBranchDecoder {
    static final String integerRegisterConditionNames[] = {
        null, "brz", "brlez", "brlz", null, "brnz", "brgz", "brgez"
    };

    String getRegisterConditionName(int index) {
        return integerRegisterConditionNames[index];
    }
}
