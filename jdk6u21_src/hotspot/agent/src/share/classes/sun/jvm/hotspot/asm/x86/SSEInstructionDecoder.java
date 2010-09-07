/*
 * Copyright (c) 2003, Oracle and/or its affiliates. All rights reserved.
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

// SSE instructions decoder class
public class SSEInstructionDecoder extends InstructionDecoder {

   public SSEInstructionDecoder(String name) {
      super(name);
   }
   public SSEInstructionDecoder(String name, int addrMode1, int operandType1) {
      super(name, addrMode1, operandType1);
   }
   public SSEInstructionDecoder(String name, int addrMode1, int operandType1, int addrMode2, int operandType2) {
      super(name, addrMode1, operandType1, addrMode2, operandType2);
   }

   public SSEInstructionDecoder(String name, int addrMode1, int operandType1, int addrMode2, int operandType2, int addrMode3, int operandType3) {
      super(name, addrMode1, operandType1, addrMode2, operandType2, addrMode3, operandType3);
   }

   protected Instruction decodeInstruction(byte[] bytesArray, boolean operandSize, boolean addrSize, X86InstructionFactory factory) {
      Operand op1 = getOperand1(bytesArray, operandSize, addrSize);
      Operand op2 = getOperand2(bytesArray, operandSize, addrSize);
      Operand op3 = getOperand3(bytesArray, operandSize, addrSize);
      int size = byteIndex - instrStartIndex;
      return factory.newGeneralInstruction(name, op1, op2, op3, size, 0);
   }
}
