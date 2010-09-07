/*
 * Copyright (c) 2004, Oracle and/or its affiliates. All rights reserved.
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

package sun.jvm.hotspot.asm.x86;

import sun.jvm.hotspot.asm.Address;
import sun.jvm.hotspot.asm.BaseIndexScaleDispAddress;

public class X86RegisterDirectAddress extends Address {
    final private X86Register base;

    public X86RegisterDirectAddress(X86Register base) {
        this.base = base;
    }

    public String toString() {
        return base.toString();
    }
}
