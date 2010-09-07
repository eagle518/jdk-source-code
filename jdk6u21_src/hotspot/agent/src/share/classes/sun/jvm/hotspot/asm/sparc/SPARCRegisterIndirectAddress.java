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

import sun.jvm.hotspot.asm.Address;
import sun.jvm.hotspot.asm.BaseIndexScaleDispAddress;

public class SPARCRegisterIndirectAddress extends BaseIndexScaleDispAddress {
    protected int addressSpace = -1;

    public SPARCRegisterIndirectAddress(SPARCRegister register, int offset) {
        super(register, offset);
    }

    public SPARCRegisterIndirectAddress(SPARCRegister base, SPARCRegister index) {
        super(base, index);
    }

    public int getAddressSpace() {
        return addressSpace;
    }

    public void setAddressSpace(int addressSpace) {
        this.addressSpace = addressSpace;
    }

    public String getAddressWithoutAsi() {
        StringBuffer buf = new StringBuffer();
        buf.append('[');
        buf.append(getBase().toString());
        sun.jvm.hotspot.asm.Register register = getIndex();
        if (register != null) {
            buf.append(" + ");
            buf.append(register.toString());
        } else {
            long disp = getDisplacement();
            if (disp < 0) {
                buf.append(" - 0x");
                disp = -disp;
            } else {
                buf.append(" + 0x");
            }
            buf.append(Long.toHexString(disp));
        }
        buf.append(']');
        return buf.toString();
    }

    public String toString() {
        StringBuffer buf = new StringBuffer();
        buf.append(getAddressWithoutAsi());
        if(addressSpace != -1)
            buf.append((new Integer(addressSpace)).toString());
        return buf.toString();
    }
}
