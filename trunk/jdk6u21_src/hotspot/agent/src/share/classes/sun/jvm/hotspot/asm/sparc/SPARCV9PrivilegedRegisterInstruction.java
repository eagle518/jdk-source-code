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

public abstract class SPARCV9PrivilegedRegisterInstruction extends SPARCInstruction
                          implements SPARCV9Instruction, /* imports */ SPARCV9PrivilegedRegisters {
    protected static final String regNames[] = {
        "%tpc", "%tnpc", "%tstate", "%tt", "%tick", "%tba", "%pstate", "%tl",
        "%pil", "%cwp",  "%cansave", "%canrestore", "%cleanwin", "%otherwin", "%wstate", "%fq"
    };

    protected static String getPrivilegedRegisterName(int regNum) {
        if ((regNum > 15 && regNum < 31) || regNum > 31)
            return null;
        return (regNum == 31)? "%ver" : regNames[regNum];
    }

    final protected int regNum;

    protected abstract String getDescription();

    protected SPARCV9PrivilegedRegisterInstruction(String name, int regNum) {
        super(name);
        this.regNum = regNum;
    }

    public int getPrivilegedRegisterNumber() {
        return regNum;
    }

    public String asString(long currentPc, SymbolFinder symFinder) {
        return getDescription();
    }
}
