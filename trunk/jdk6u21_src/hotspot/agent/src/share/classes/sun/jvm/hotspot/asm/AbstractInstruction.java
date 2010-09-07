/*
 * Copyright (c) 2002, Oracle and/or its affiliates. All rights reserved.
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

package sun.jvm.hotspot.asm;

public abstract class AbstractInstruction implements Instruction {
   protected final String name;

   public AbstractInstruction(String name) {
      this.name = name;
   }

   public String getName() {
      return name;
   }

   // some type testers
   public boolean isIllegal() {
      return false;
   }

   public boolean isArithmetic() {
      return false;
   }

   public boolean isLogical() {
      return false;
   }

   public boolean isShift() {
      return false;
   }

   public boolean isMove() {
      return false;
   }

   public boolean isBranch() {
      return false;
   }

   public boolean isCall() {
      return false;
   }

   public boolean isReturn() {
      return false;
   }

   public boolean isLoad() {
      return false;
   }

   public boolean isStore() {
      return false;
   }

   public boolean isFloat() {
      return false;
   }

   public boolean isTrap() {
      return false;
   }

   public boolean isNoop() {
      return false;
   }

   // convert the instruction as String given currentPc
   // and SymbolFinder

   public String asString(long currentPc, SymbolFinder symFinder) {
      return name;
   }
}
