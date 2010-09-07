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

public class SPARCSwapInstruction extends SPARCAtomicLoadStoreInstruction {
    public SPARCSwapInstruction(String name, SPARCRegisterIndirectAddress addr, SPARCRegister rd) {
        super(name, addr, rd);
    }

    public int getDataType() {
        return RTLDT_UNSIGNED_WORD;
    }

    public boolean isConditional() {
        return false;
    }
}
