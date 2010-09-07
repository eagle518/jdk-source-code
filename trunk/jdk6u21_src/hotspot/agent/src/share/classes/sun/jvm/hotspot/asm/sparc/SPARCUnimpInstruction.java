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

public class SPARCUnimpInstruction extends SPARCInstruction {
    final private String description;
    final private int const22;

    protected SPARCUnimpInstruction(String name, int const22) {
        super(name);
        this.const22 = const22;
        description = initDescription();
    }

    public SPARCUnimpInstruction(int const22) {
        this("unimp", const22);
    }

    public int getConst22() {
        return const22;
    }

    public boolean isIllegal() {
        return true;
    }

    private String initDescription() {
        StringBuffer buf = new StringBuffer();
        buf.append(getName());
        buf.append(spaces);
        buf.append("0x");
        buf.append(Integer.toHexString(const22));
        return buf.toString();
    }

    public String asString(long currentPc, SymbolFinder symFinder) {
        return description;
    }
}
