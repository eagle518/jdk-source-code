/*
 * Copyright (c) 2002, 2003, Oracle and/or its affiliates. All rights reserved.
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

class FPStoreDecoder extends FPInstructionDecoder {
   FPStoreDecoder(String name, int addrMode1, int operandType1) {
      super(name, addrMode1, operandType1);
   }

   protected Instruction decodeInstruction(byte[] bytesArray, boolean operandSize, boolean addrSize, X86InstructionFactory factory) {
      Operand op = getOperand1(bytesArray, operandSize, addrSize);
      int size = byteIndex - instrStartIndex;
      return factory.newFPStoreInstruction(name, op, size, prefixes);
   }
}
