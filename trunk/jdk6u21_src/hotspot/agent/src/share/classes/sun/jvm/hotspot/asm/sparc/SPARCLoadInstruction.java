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

public class SPARCLoadInstruction extends SPARCMemoryInstruction
    implements LoadInstruction {
    final protected SPARCRegister register2; // used for double word load instructions
    final protected Register[] loadDestinations;

    public SPARCLoadInstruction(String name, int opcode, SPARCRegisterIndirectAddress address, SPARCRegister register, int dataType) {
        super(name, opcode,address, register, dataType);
        if (opcode == LDD || opcode == LDDA) {
            int nextRegNum = (register.getNumber() + 1) % SPARCRegisters.NUM_REGISTERS;
            register2 = SPARCRegisters.getRegister(nextRegNum);
            loadDestinations = new Register[2];
            loadDestinations[0] = register;
            loadDestinations[1] = register2;
        } else {
            register2 = null;
            loadDestinations = new Register[1];
            loadDestinations[0] = register;
        }
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

    public Register[] getLoadDestinations() {
        return loadDestinations;
    }

    public Address getLoadSource() {
        return address;
    }

    public boolean isLoad() {
        return true;
    }
}
