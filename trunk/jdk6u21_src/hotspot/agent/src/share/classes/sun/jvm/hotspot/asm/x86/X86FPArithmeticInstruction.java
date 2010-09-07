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

public class X86FPArithmeticInstruction extends X86FPInstruction
                                        implements ArithmeticInstruction {
   final private int operation; //RTL operation
   final private Operand operand1;
   final private Operand operand2;
   final private String description;

   public X86FPArithmeticInstruction(String name, int operation, Operand op1, Operand op2, int size, int prefixes) {
      super(name, size, prefixes);
      this.operation = operation;
      this.operand1 = op1;
      this.operand2 = op2;
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
      if (operand2 != null) {
         buf.append(comma);
         buf.append(getOperandAsString(operand2));
      }
      return buf.toString();
   }

   public String asString(long currentPc, SymbolFinder symFinder) {
      return description;
   }

   public Operand getArithmeticDestination() {
      return operand1;
   }
   public Operand getOperand1() {
      return operand1;
   }

   public Operand getOperand2() {
      return operand2;
   }

   public Operand[] getArithmeticSources() {
      return (new Operand[] { operand1, operand2});
   }

   public int getOperation() {
      return operation;
   }

   public boolean isArithmetic() {
      return true;
   }
}
