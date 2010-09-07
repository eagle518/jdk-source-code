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

public class SPARCV9FMOVccInstruction extends SPARCFPMoveInstruction
    implements MoveInstruction {
    final int conditionCode;
    final int conditionFlag;

    public SPARCV9FMOVccInstruction(String name, int opf, int conditionCode,
                              int conditionFlag, SPARCFloatRegister rs,
                              SPARCFloatRegister rd) {
        super(name, opf, rs, rd);
        this.conditionFlag = conditionFlag;
        this.conditionCode = conditionCode;
    }

    public int getConditionCode() {
        return conditionCode;
    }

    public int getConditionFlag() {
        return conditionFlag;
    }

    public boolean isConditional() {
        return false;
    }
}
