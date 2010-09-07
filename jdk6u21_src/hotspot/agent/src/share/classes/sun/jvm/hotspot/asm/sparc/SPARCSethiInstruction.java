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

public class SPARCSethiInstruction extends SPARCInstruction
    implements MoveInstruction {
    final private SPARCRegister register;
    final private ImmediateOrRegister value;
    final private String description;

    public SPARCSethiInstruction(int value, SPARCRegister register) {
        super("sethi");
        this.register = register;
        value <<= 10;
        this.value = new Immediate(new Integer(value));
        description = initDescription(value);
    }

    private String initDescription(int val) {
        if (val == 0 && register == SPARCRegisters.G0) {
            return "nop";
        } else {
            StringBuffer buf = new StringBuffer();
            buf.append(getName());
            buf.append(spaces);
            buf.append("%hi(0x");
            buf.append(Integer.toHexString(val));
            buf.append(')');
            buf.append(comma);
            buf.append(register.toString());
            return buf.toString();
        }
    }

    public String asString(long currentPc, SymbolFinder symFinder) {
        return description;
    }

    public Register getMoveDestination() {
        return register;
    }

    public ImmediateOrRegister getMoveSource() {
        return value;
    }

    public boolean isConditional() {
        return false;
    }
}
