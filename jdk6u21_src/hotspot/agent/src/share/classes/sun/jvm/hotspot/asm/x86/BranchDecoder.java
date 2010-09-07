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
import sun.jvm.hotspot.utilities.*;

public class BranchDecoder extends InstructionDecoder {

   public BranchDecoder(String name) {
      super(name);
   }
   public BranchDecoder(String name, int addrMode1, int operandType1) {
      super(name, addrMode1, operandType1);
   }
   protected Instruction decodeInstruction(byte[] bytesArray, boolean operandSize, boolean addrSize, X86InstructionFactory factory) {
      Operand addr = getOperand1(bytesArray, operandSize, addrSize);
      if (Assert.ASSERTS_ENABLED) {
         Assert.that(addr == null || addr instanceof X86PCRelativeAddress, "Address should be PC Relative!");
      }
      int size = byteIndex - instrStartIndex;
      return factory.newBranchInstruction(name, (X86PCRelativeAddress)addr, size, prefixes);
   }
}
