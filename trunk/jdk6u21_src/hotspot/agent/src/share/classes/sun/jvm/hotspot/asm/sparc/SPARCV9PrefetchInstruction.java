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

public class SPARCV9PrefetchInstruction extends SPARCInstruction
                    implements SPARCV9Instruction {
    final private SPARCRegisterIndirectAddress addr;
    final private int prefetchFcn;
    final private String description;

    public static final int PREFETCH_MANY_READS  = 0;
    public static final int PREFETCH_ONE_READ    = 1;
    public static final int PREFETCH_MANY_WRITES = 2;
    public static final int PREFETCH_ONE_WRITE   = 3;
    public static final int PREFETCH_PAGE        = 4;

    public SPARCV9PrefetchInstruction(String name, SPARCRegisterIndirectAddress addr, int prefetchFcn) {
        super(name);
        this.addr = addr;
        this.prefetchFcn = prefetchFcn;
        description = initDescription();
    }

    private String initDescription() {
        StringBuffer buf = new StringBuffer();
        buf.append(getName());
        buf.append(spaces);
        buf.append(addr.toString());
        buf.append(comma);
        buf.append(prefetchFcn);
        return buf.toString();
    }

    public int getPrefetchFunction() {
        return prefetchFcn;
    }

    public SPARCRegisterIndirectAddress getPrefetchAddress() {
        return addr;
    }

    public String asString(long currentPc, SymbolFinder symFinder) {
        return description;
    }
}
