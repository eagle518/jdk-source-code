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

public class X86ShiftInstruction extends X86Instruction implements ShiftInstruction {
   final private int operation;
   final private Operand operand1;
   final private ImmediateOrRegister operand2;

   public X86ShiftInstruction(String name, int operation, Operand operand1, ImmediateOrRegister operand2, int size, int prefixes) {
      super(name, size, prefixes);
      this.operand1 = operand1;
      this.operand2 = operand2;
      this.operation = operation;
   }

   public String asString(long currentPc, SymbolFinder symFinder) {
      StringBuffer buf = new StringBuffer();
      buf.append(getPrefixString());
      buf.append(getName());
      buf.append(spaces);
      buf.append(getOperandAsString(operand1));

      if(operand2 != null) {
         buf.append(comma);

         if ((operand2 instanceof Register)) {
            buf.append(operand2.toString());
         }
         else {
            Number number = ((Immediate)operand2).getNumber();
            buf.append("0x");
            buf.append(Integer.toHexString(number.intValue()));
         }
      }
      return buf.toString();
   }

   public int getOperation() {
      return operation;
   }

   public Operand getShiftDestination() {
      return operand1;
   }

   public Operand getShiftLength() {
      return operand2;
   }

   public Operand getShiftSource() {
      return operand1;
   }

   public boolean isShift() {
      return true;
   }

   protected String getOperand2String() {
      return operand2.toString();
   }
}
