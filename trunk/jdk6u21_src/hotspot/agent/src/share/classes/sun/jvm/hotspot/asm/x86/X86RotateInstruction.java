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

public class X86RotateInstruction extends X86Instruction {
   final private Operand operand1;
   final private ImmediateOrRegister operand2;

   public X86RotateInstruction(String name, Operand operand1, ImmediateOrRegister operand2, int size, int prefixes) {
      super(name, size, prefixes);
      this.operand1 = operand1;
      this.operand2 = operand2;
   }

   public String asString(long currentPc, SymbolFinder symFinder) {
      StringBuffer buf = new StringBuffer();
      buf.append(getPrefixString());
      buf.append(getName());
      buf.append(spaces);
      if(operand2 != null) {
         if ((operand2 instanceof Register)) {
            buf.append(operand2.toString());
         }
         else {
            Number number = ((Immediate)operand2).getNumber();
            buf.append("0x");
            buf.append(Integer.toHexString(number.intValue()));
         }
         buf.append(comma);
      }
      buf.append(getOperandAsString(operand1));
      return buf.toString();
   }

   public Operand getRotateDestination() {
      return operand1;
   }

   public Operand getRotateSource() {
      return operand1;
   }
}
