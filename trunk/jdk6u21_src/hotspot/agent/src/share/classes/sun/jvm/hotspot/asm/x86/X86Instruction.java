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

public abstract class X86Instruction implements Instruction, X86Opcodes {
   final private String name;
   final private int size;
   final private int prefixes;

   public X86Instruction(String name, int size, int prefixes) {
      this.name = name;
      this.size = size;
      this.prefixes = prefixes;
   }

   public abstract String asString(long currentPc, SymbolFinder symFinder);

   public String getName() {
      return name;
   }

   public String getPrefixString() {
      StringBuffer buf = new StringBuffer();
      if ((prefixes & PREFIX_REPZ) != 0)
         buf.append("repz ");
      if ((prefixes & PREFIX_REPNZ) != 0)
         buf.append("repnz ");
      if ((prefixes & PREFIX_LOCK) != 0)
         buf.append("lock ");

      return buf.toString();
   }

   protected String getOperandAsString(Operand op) {
      StringBuffer buf = new StringBuffer();
      if ((op instanceof Register) || (op instanceof Address)) {
         buf.append(op.toString());
      } else {
         Number number = ((Immediate)op).getNumber();
         buf.append("0x");
         buf.append(Integer.toHexString(number.intValue()));
      }
      return buf.toString();
   }

   public int getSize() {
      return size;
   }

   public boolean isArithmetic() {
      return false;
   }

   public boolean isBranch() {
      return false;
   }

   public boolean isCall() {
      return false;
   }

   public boolean isFloat() {
      return false;
   }

   public boolean isIllegal() {
      return false;
   }

   public boolean isLoad() {
      return false;
   }

   public boolean isLogical() {
      return false;
   }

   public boolean isMove() {
      return false;
   }

   public boolean isReturn() {
      return false;
   }

   public boolean isShift() {
      return false;
   }

   public boolean isStore() {
      return false;
   }

   public boolean isTrap() {
      return false;
   }

   public boolean isNoop() {
      return false;
   }

   protected static String comma = ", ";
   protected static String spaces = "\t";
}
