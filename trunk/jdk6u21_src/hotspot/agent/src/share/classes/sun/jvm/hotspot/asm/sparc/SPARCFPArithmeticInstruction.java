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

public class SPARCFPArithmeticInstruction extends SPARCFormat3AInstruction
                    implements ArithmeticInstruction {
    final private SPARCRegister rs2;
    final private int rtlOperation;

    public SPARCFPArithmeticInstruction(String name, int opcode, int rtlOperation,
                                 SPARCRegister rs1, SPARCRegister rs2,
                                 SPARCRegister rd) {
        super(name, opcode, rs1, rs2, rd);
        this.rs2 = rs2;
        this.rtlOperation = rtlOperation;
    }

    protected String getDescription() {
        StringBuffer buf = new StringBuffer();
        buf.append(getName());
        buf.append(spaces);
        buf.append(rs1.toString());
        buf.append(comma);
        buf.append(rs2.toString());
        buf.append(comma);
        buf.append(rd.toString());
        return buf.toString();
    }

    public int getOperation() {
        return rtlOperation;
    }

    public Operand[] getArithmeticSources() {
        return new Operand[] { rs1, rs2 };
    }

    public Operand getArithmeticDestination() {
        return rd;
    }

    public boolean isFloat() {
        return true;
    }
}
