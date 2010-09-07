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

public class SPARCBranchInstruction extends SPARCInstruction
    implements BranchInstruction {
    final protected PCRelativeAddress addr;
    final protected int conditionCode;
    final protected boolean isAnnuled;

    public SPARCBranchInstruction(String name, PCRelativeAddress addr, boolean isAnnuled, int conditionCode) {
        super(name);
        this.addr = addr;
        this.conditionCode = conditionCode;
        this.isAnnuled = isAnnuled;
    }

    public String asString(long currentPc, SymbolFinder symFinder) {
        long address = addr.getDisplacement() + currentPc;
        StringBuffer buf = new StringBuffer();
        buf.append(getName());
        buf.append(spaces);
        buf.append(symFinder.getSymbolFor(address));
        return buf.toString();
    }

    public Address getBranchDestination() {
        return addr;
    }

    public int getConditionCode() {
        return conditionCode;
    }

    public boolean isAnnuledBranch() {
        return isAnnuled;
    }

    public boolean isBranch() {
        return true;
    }

    public boolean isConditional() {
        return conditionCode != CONDITION_BN && conditionCode != CONDITION_BA;
    }
}
