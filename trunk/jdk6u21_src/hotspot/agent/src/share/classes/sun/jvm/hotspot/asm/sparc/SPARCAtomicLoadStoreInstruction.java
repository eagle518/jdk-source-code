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

public abstract class SPARCAtomicLoadStoreInstruction extends SPARCInstruction
                        implements LoadInstruction, StoreInstruction {
    final protected SPARCRegisterIndirectAddress addr;
    final protected SPARCRegister rd;
    final protected Register[] regs = new Register[1];
    final protected String description;

    public SPARCAtomicLoadStoreInstruction(String name, SPARCRegisterIndirectAddress addr, SPARCRegister rd) {
        super(name);
        this.addr = addr;
        this.rd = rd;
        regs[0] = rd;
        description = initDescription();
    }

    private String initDescription() {
        StringBuffer buf = new StringBuffer();
        buf.append(getName());
        buf.append(spaces);
        buf.append(addr.toString());
        buf.append(comma);
        buf.append(rd.toString());
        return buf.toString();
    }

    public Address getLoadSource() {
        return addr;
    }

    public Address getStoreDestination() {
        return addr;
    }

    public Register[] getLoadDestinations() {
        return regs;
    }

    public Register[] getStoreSources() {
        return regs;
    }

    public boolean isLoad() {
        return true;
    }

    public boolean isStore() {
        return true;
    }

    public String asString(long currentPc, SymbolFinder symFinder) {
        return description;
    }
}
