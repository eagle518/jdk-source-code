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

public class SPARCReturnInstruction extends SPARCJmplInstruction
    implements ReturnInstruction {

    private final boolean leaf;

    public SPARCReturnInstruction(SPARCRegisterIndirectAddress addr, SPARCRegister rd, boolean leaf) {
        super(leaf? "retl" : "ret", addr, rd);
        this.leaf = leaf;
    }

    public boolean isLeaf() {
        return leaf;
    }

    protected String getDescription() {
        return getName();
    }
}
