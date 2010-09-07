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

public class RotateDecoder extends InstructionDecoder {

   public RotateDecoder(String name, int addrMode1, int operandType1) {
      super(name, addrMode1, operandType1);
   }
   public RotateDecoder(String name, int addrMode1, int operandType1, int addrMode2, int operandType2) {
      super(name, addrMode1, operandType1, addrMode2, operandType2);
   }
   protected Instruction decodeInstruction(byte[] bytesArray, boolean operandSize, boolean addrSize, X86InstructionFactory factory) {
      Operand op1 = getOperand1(bytesArray, operandSize, addrSize);
      Operand op2 = getOperand2(bytesArray, operandSize, addrSize);
      int size = byteIndex - instrStartIndex;
      return factory.newRotateInstruction(name, op1, (ImmediateOrRegister)op2, size, prefixes);
   }
}
