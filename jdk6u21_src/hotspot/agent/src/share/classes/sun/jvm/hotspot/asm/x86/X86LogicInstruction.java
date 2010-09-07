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

public class X86LogicInstruction extends X86Instruction
                                      implements LogicInstruction {
   final private Operand operand1;
   final private Operand operand2;
   final private int operation;

   public X86LogicInstruction(String name, int operation, Operand op1, Operand op2, int size, int prefixes) {
      super(name, size, prefixes);
      this.operation = operation;
      this.operand1 = op1;
      this.operand2 = op2;
   }

   public String asString(long currentPc, SymbolFinder symFinder) {
      StringBuffer buf = new StringBuffer();
      buf.append(getPrefixString());
      buf.append(getName());
      buf.append(spaces);
      buf.append(getOperandAsString(operand1));
      if(operand2 != null) {
         buf.append(comma);
         buf.append(getOperandAsString(operand2));
      }
      return buf.toString();
   }

   public Operand getLogicDestination() {
      return operand1;
   }

   public Operand[] getLogicSources() {
      return (new Operand[] { operand2 });
   }

   public int getOperation() {
      return operation;
   }

   public boolean isLogic() {
      return true;
   }
}
