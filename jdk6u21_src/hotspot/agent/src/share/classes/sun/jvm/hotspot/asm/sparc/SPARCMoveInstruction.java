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

public class SPARCMoveInstruction extends SPARCFormat3AInstruction
    implements MoveInstruction, RTLOperations {

    public SPARCMoveInstruction(String name, int opcode, ImmediateOrRegister operand2, SPARCRegister rd) {
        super(name, opcode, null, operand2, rd);
    }

    protected String getDescription() {
        StringBuffer buf = new StringBuffer();
        if (operand2 == SPARCRegisters.G0) {
            buf.append("clr");
            buf.append(spaces);
            buf.append(rd.toString());
        } else {
            buf.append("mov");
            buf.append(spaces);
            buf.append(getOperand2String());
            buf.append(comma);
            buf.append(rd.toString());
        }

        return buf.toString();
    }

    public Register getMoveDestination() {
        return getDestinationRegister();
    }

    public ImmediateOrRegister getMoveSource() {
        return operand2;
    }

    public boolean isConditional() {
        return false;
    }

    public boolean isMove() {
        return true;
    }
}
