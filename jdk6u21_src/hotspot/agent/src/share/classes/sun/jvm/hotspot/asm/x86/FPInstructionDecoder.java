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

// basic float instruction decoder class
public class FPInstructionDecoder extends InstructionDecoder {

   public FPInstructionDecoder(String name) {
      super(name);
   }
   public FPInstructionDecoder(String name, int addrMode1, int operandType1) {
      super(name, addrMode1, operandType1);
   }
   public FPInstructionDecoder(String name, int addrMode1, int operandType1, int addrMode2, int operandType2) {
      super(name, addrMode1, operandType1, addrMode2, operandType2);
   }

   protected Instruction decodeInstruction(byte[] bytesArray, boolean operandSize, boolean addrSize, X86InstructionFactory factory) {
      Operand op1 = getOperand1(bytesArray, operandSize, addrSize);
      int size = byteIndex - instrStartIndex;
      return new X86FPInstruction(name, op1, size, prefixes);
   }
}
