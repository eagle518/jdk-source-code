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

import sun.jvm.hotspot.asm.SymbolFinder;
import sun.jvm.hotspot.utilities.Assert;

public class SPARCV9ReadInstruction extends SPARCV9SpecialRegisterInstruction {
    final private int specialReg;
    final private int asrRegNum;
    final private SPARCRegister rd;

    public SPARCV9ReadInstruction(int specialReg, int asrRegNum, SPARCRegister rd) {
        super("rd");
        this.specialReg = specialReg;
        this.asrRegNum = asrRegNum;
        this.rd = rd;
    }

    public int getSpecialRegister() {
        return specialReg;
    }

    public int getAncillaryRegister() {
        if (Assert.ASSERTS_ENABLED)
            Assert.that(specialReg == ASR, "not an ancillary register");
        return asrRegNum;
    }

    protected String getDescription() {
        StringBuffer buf = new StringBuffer();
        buf.append(getName());
        buf.append(spaces);
        if(specialReg == ASR)
            buf.append("%asr" + asrRegNum);
        else
            buf.append(getSpecialRegisterName(specialReg));
        buf.append(comma);
        buf.append(rd.toString());
        return buf.toString();
    }
}
