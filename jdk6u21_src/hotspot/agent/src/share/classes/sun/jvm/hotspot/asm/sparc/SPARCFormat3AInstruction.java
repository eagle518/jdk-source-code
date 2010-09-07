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

public abstract class SPARCFormat3AInstruction extends SPARCInstruction {
    final protected int opcode;
    final protected SPARCRegister rs1;
    final protected ImmediateOrRegister operand2;
    final protected SPARCRegister rd;

    public SPARCFormat3AInstruction(String name, int opcode, SPARCRegister rs1,
                                    ImmediateOrRegister operand2, SPARCRegister rd) {
        super(name);
        this.opcode = opcode;
        this.rs1 = rs1;
        this.operand2 = operand2;
        this.rd = rd;
    }

    protected String getOperand2String() {
        StringBuffer buf = new StringBuffer();
        if (operand2.isRegister()) {
            buf.append(operand2.toString());
        } else {
            Number number = ((Immediate)operand2).getNumber();
            buf.append("0x");
            buf.append(Integer.toHexString(number.intValue()));
        }
        return buf.toString();
    }

    protected String getDescription() {
        StringBuffer buf = new StringBuffer();
        buf.append(getName());
        buf.append(spaces);
        buf.append(rs1.toString());
        buf.append(comma);
        buf.append(getOperand2String());
        buf.append(comma);
        buf.append(rd.toString());
        return buf.toString();
    }

    public String asString(long currentPc, SymbolFinder symFinder) {
        return getDescription();
    }

    public int getOpcode() {
        return opcode;
    }

    public SPARCRegister getDestinationRegister() {
        return rd;
    }

    public ImmediateOrRegister getOperand2() {
        return operand2;
    }

    public SPARCRegister getSourceRegister1() {
        return rs1;
    }
}
