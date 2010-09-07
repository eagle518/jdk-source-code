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

import sun.jvm.hotspot.asm.MemoryInstruction;
import sun.jvm.hotspot.asm.SymbolFinder;

public abstract class SPARCMemoryInstruction extends SPARCInstruction
    implements MemoryInstruction {
    final protected SPARCRegisterIndirectAddress address;
    final protected SPARCRegister register;
    final protected int dataType;
    final protected int opcode;

    public SPARCMemoryInstruction(String name, int opcode, SPARCRegisterIndirectAddress address, SPARCRegister register, int dataType) {
        super(name);
        this.address = address;
        this.register = register;
        this.dataType = dataType;
        this.opcode = opcode;
    }

    protected String getDescription() {
        StringBuffer buf = new StringBuffer();
        buf.append(getName());
        buf.append(spaces);
        buf.append(address.toString());
        buf.append(comma);
        buf.append(register.toString());
        return buf.toString();
    }

    public String asString(long currentPc, SymbolFinder symFinder) {
        return getDescription();
    }

    public int getDataType() {
        return dataType;
    }

    public boolean isConditional() {
        return false;
    }

    public int getOpcode() {
        return opcode;
    }
}
