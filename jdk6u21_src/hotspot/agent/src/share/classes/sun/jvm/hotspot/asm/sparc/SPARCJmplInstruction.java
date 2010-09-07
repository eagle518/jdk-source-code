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

public class SPARCJmplInstruction extends SPARCInstruction
    implements BranchInstruction {
    final protected SPARCRegisterIndirectAddress addr;
    final protected SPARCRegister rd;

    protected SPARCJmplInstruction(String name, SPARCRegisterIndirectAddress addr, SPARCRegister rd) {
        super(name);
        this.addr = addr;
        this.rd = rd;
    }

    public SPARCJmplInstruction(SPARCRegisterIndirectAddress addr, SPARCRegister rd) {
        this("jmpl", addr, rd);
    }

    protected String getDescription() {
        StringBuffer buf = new StringBuffer();
        String addrStr = addr.toString();
        // remove '[' & ']' from address
        addrStr = addrStr.substring(1, addrStr.length() - 1);
        if (rd == SPARCRegisters.G0) {
            buf.append("jmp");
            buf.append(spaces);
            buf.append(addrStr);
        } else {
            buf.append(getName());
            buf.append(spaces);
            buf.append(addrStr);
            buf.append(comma);
            buf.append(rd.toString());
        }

        return buf.toString();
    }

    public String asString(long currentPc, SymbolFinder symFinder) {
        return getDescription();
    }

    public Address getBranchDestination() {
        return addr;
    }

    public SPARCRegister getReturnAddressRegister() {
        return rd;
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
