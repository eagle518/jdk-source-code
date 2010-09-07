/*
 * Copyright (c) 2002, 2003, Oracle and/or its affiliates. All rights reserved.
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

public class SPARCV9RegisterIndirectAddress extends SPARCRegisterIndirectAddress {
    protected boolean indirectAsi;

    public SPARCV9RegisterIndirectAddress(SPARCRegister register, int offset) {
        super(register, offset);
    }

    public SPARCV9RegisterIndirectAddress(SPARCRegister base, SPARCRegister index) {
        super(base, index);
    }

    public boolean getIndirectAsi() {
        return indirectAsi;
    }

    public void setIndirectAsi(boolean indirectAsi) {
        this.indirectAsi = indirectAsi;
    }

    public String toString() {
        StringBuffer buf = new StringBuffer();
        buf.append(getAddressWithoutAsi());
        if (indirectAsi) {
            buf.append("%asi");
        } else if (addressSpace != -1) {
            buf.append(Integer.toString(addressSpace));
        }
        return buf.toString();
    }
}
