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

public class SPARCFP2RegisterInstruction extends SPARCInstruction {
    final SPARCFloatRegister rs;
    final SPARCFloatRegister rd;
    final int opf;

    public SPARCFP2RegisterInstruction(String name, int opf, SPARCFloatRegister rs, SPARCFloatRegister rd) {
        super(name);
        this.rs = rs;
        this.rd = rd;
        this.opf = opf;
    }

    public String asString(long currentPc, SymbolFinder symFinder) {
        return getDescription();
    }

    protected String getDescription() {
        StringBuffer buf = new StringBuffer();
        buf.append(getName());
        buf.append(spaces);
        buf.append(rs.toString());
        buf.append(comma);
        buf.append(rd.toString());

        return buf.toString();
    }

    public boolean isFloat() {
        return true;
    }
}
