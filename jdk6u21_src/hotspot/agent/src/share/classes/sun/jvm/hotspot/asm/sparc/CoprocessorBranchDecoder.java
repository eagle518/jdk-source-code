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

// format 2 - condition code names.
// Appendix F - Opcodes and Condition Codes - Page 231 - Table F-7.

class CoprocessorBranchDecoder extends BranchDecoder {
    private static final String coprocessorConditionNames[] = {
        "cbn", "cb123", "cb12", "cb13", "cb1", "cb23", "cb2", "cb3",
        "cba", "cb0",  "cb03", "cb02", "cb023", "cb01", "cb013", "cb012"
    };

    private static final String coprocessorAnnuledConditionNames[] = {
        "cbn,a", "cb123,a", "cb12,a", "cb13,a", "cb1,a", "cb23,a", "cb2,a", "cb3,a",
        "cba,a", "cb0,a",  "cb03,a", "cb02,a", "cb023,a", "cb01,a", "cb013,a", "cb012,a"
    };

    String getConditionName(int conditionCode, boolean isAnnuled) {
        return isAnnuled ? coprocessorAnnuledConditionNames[conditionCode]
                         : coprocessorConditionNames[conditionCode];
    }
}
