/*
 * Copyright (c) 2002, 2004, Oracle and/or its affiliates. All rights reserved.
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

import sun.jvm.hotspot.asm.*;

public class CallDecoder extends InstructionDecoder {
    public CallDecoder(String name, int addrMode1, int operandType1) {
        super(name, addrMode1, operandType1);
    }
    protected Instruction decodeInstruction(byte[] bytesArray, boolean operandSize, boolean addrSize, X86InstructionFactory factory) {
        Operand operand = getOperand1(bytesArray, operandSize, addrSize);
        int size = byteIndex - instrStartIndex;
        Address address;
        if (operand instanceof X86Register) {
            address = new X86RegisterDirectAddress((X86Register)operand);
        } else {
            address = (Address) operand;
        }
        return factory.newCallInstruction(name, address, size, prefixes);
    }
}
