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

public class SPARCStoreInstruction extends SPARCMemoryInstruction
    implements StoreInstruction {
    final protected SPARCRegister register2; // used by double word store instructions
    final protected Register[] storeSources;

    public SPARCStoreInstruction(String name, int opcode, SPARCRegisterIndirectAddress address, SPARCRegister register, int dataType) {
        super(name, opcode, address, register, dataType);
        if (opcode == STD || opcode == STDA) {
            storeSources = new Register[2];
            storeSources[0] = register;
            int nextRegNum = (register.getNumber() + 1) % SPARCRegisters.NUM_REGISTERS;
            register2 = SPARCRegisters.getRegister(nextRegNum);
            storeSources[1] = register2;
        } else {
            storeSources = new Register[1];
            storeSources[0] = register;
            register2 = null;
        }
    }

    private String defaultInitDescription(StringBuffer buf) {
        buf.append(getName());
        buf.append(spaces);
        buf.append(register.toString());
        buf.append(comma);
        buf.append(address.toString());
        return buf.toString();
    }

    protected String getDescription() {
        StringBuffer buf = new StringBuffer();
        if (register == SPARCRegisters.G0) {
            switch (opcode) {
                case ST:
                    buf.append("clr");
                    break;
                case STH:
                    buf.append("clrh");
                    break;
                case STB:
                    buf.append("clrb");
                    break;
                default:
                    return defaultInitDescription(buf);
            }
            buf.append(spaces);
            buf.append(address.toString());
            return buf.toString();
        } else {
            return defaultInitDescription(buf);
        }
    }

    public int getDataType() {
        return dataType;
    }

    public Address getStoreDestination() {
        return address;
    }

    public Register[] getStoreSources() {
        return storeSources;
    }

    public boolean isStore() {
        return true;
    }
}
