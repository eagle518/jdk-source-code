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

public class SPARCRettInstruction extends SPARCInstruction
    implements BranchInstruction {
    final protected SPARCRegisterIndirectAddress addr;
    final protected String description;

    protected SPARCRettInstruction(String name, SPARCRegisterIndirectAddress addr) {
        super(name);
        this.addr = addr;
        description = initDescription();
    }

    public SPARCRettInstruction(SPARCRegisterIndirectAddress addr) {
        this("rett", addr);
    }

    private String initDescription() {
        StringBuffer buf = new StringBuffer();
        buf.append(getName());
        buf.append(spaces);
        buf.append(addr.toString());
        return buf.toString();
    }

    public String asString(long currentPc, SymbolFinder symFinder) {
        return description;
    }

    public Address getBranchDestination() {
        return addr;
    }

    public boolean isAnnuledBranch() {
        return false;
    }

    public boolean isBranch() {
        return true;
    }

    public boolean isConditional() {
        return false;
    }
}
