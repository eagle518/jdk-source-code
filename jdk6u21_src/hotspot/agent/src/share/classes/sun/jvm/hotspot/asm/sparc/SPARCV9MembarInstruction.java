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
import java.util.Vector;

public class SPARCV9MembarInstruction extends SPARCInstruction
                  implements SPARCV9Instruction {
    final private int mmask;
    final private int cmask;
    final private String description;

    public SPARCV9MembarInstruction(int mmask, int cmask) {
        super("membar");
        this.mmask = mmask & 0xF;
        this.cmask = cmask & 0x7;
        description = initDescription();
    }

    private String initDescription() {
        StringBuffer buf = new StringBuffer();
        buf.append(getName());
        buf.append(spaces);

        Vector masks = new Vector();
        if ((mmask & 0x1) != 0)
            masks.add("#LoadLoad");
        if ((mmask & 0x2) != 0)
            masks.add("#StoreLoad");
        if ((mmask & 0x4) != 0)
            masks.add("#LoadStore");
        if ((mmask & 0x8) != 0)
            masks.add("#StoreStore");

        if ((cmask & 0x1) != 0)
            masks.add("#Lookaside");
        if ((cmask & 0x2) != 0)
            masks.add("#MemIssue");
        if ((cmask & 0x4) != 0)
            masks.add("#Sync");

        // add all masks
        Object[] tempMasks = masks.toArray();
        for (int i=0; i < tempMasks.length - 1; i++) {
            buf.append((String)tempMasks[i]);
            buf.append("| ");
        }
        buf.append((String)tempMasks[tempMasks.length - 1]);

        return buf.toString();
    }

    public int getMMask() {
        return mmask;
    }

    public int getCMask() {
        return cmask;
    }

    public String asString(long currentPc, SymbolFinder symFinder) {
        return description;
    }
}
