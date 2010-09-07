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

public class SPARCV9RdprInstruction extends SPARCV9PrivilegedRegisterInstruction {
    final private SPARCRegister rd;

    public SPARCV9RdprInstruction(int regNum, SPARCRegister rd) {
        super("rdpr", regNum);
        this.rd = rd;
    }

    protected String getDescription() {
        StringBuffer buf = new StringBuffer();
        buf.append(getName());
        buf.append(spaces);
        buf.append(getPrivilegedRegisterName(regNum));
        buf.append(comma);
        buf.append(rd.toString());
        return buf.toString();
    }

    public SPARCRegister getDestination() {
        return rd;
    }
}
