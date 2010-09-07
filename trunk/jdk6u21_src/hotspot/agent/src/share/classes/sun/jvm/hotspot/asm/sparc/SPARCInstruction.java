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

public abstract class SPARCInstruction
                      extends AbstractInstruction
                      implements /* imports */ SPARCOpcodes {
    public SPARCInstruction(String name) {
        super(name);
    }

    public int getSize() {
        return 4;
    }

    protected static String comma = ", ";
    protected static String spaces = "\t";
}
