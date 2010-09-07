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

public class X86FPInstruction extends X86Instruction {

   final private Operand operand1;
   final private String description;

   public X86FPInstruction(String name, int size, int prefixes) {
      super(name, size, prefixes);
      this.operand1 = null;
      description = initDescription();
   }

   public X86FPInstruction(String name, Operand op1, int size, int prefixes) {
      super(name, size, prefixes);
      this.operand1 = op1;
      description = initDescription();
   }

   protected String initDescription() {
      StringBuffer buf = new StringBuffer();
      buf.append(getPrefixString());
      buf.append(getName());
      buf.append(spaces);
      if (operand1 != null) {
         buf.append(getOperandAsString(operand1));
      }
      return buf.toString();
   }

   public String asString(long currentPc, SymbolFinder symFinder) {
      return description;
   }

   public Operand getOperand1() {
      return operand1;
   }

   public boolean isFloat() {
      return true;
   }

}
