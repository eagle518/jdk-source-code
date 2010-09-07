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

public class SPARCFPMoveInstruction extends SPARCFP2RegisterInstruction
    implements MoveInstruction {

    public SPARCFPMoveInstruction(String name, int opf, SPARCFloatRegister rs, SPARCFloatRegister rd) {
        super(name, opf, rs, rd);
    }

    public Register getMoveDestination() {
        return rd;
    }

    public ImmediateOrRegister getMoveSource() {
        return rs;
    }

    public int getMoveOpcode() {
        return opf;
    }

    public boolean isConditional() {
        return false;
    }

    public boolean isMove() {
        return true;
    }
}
