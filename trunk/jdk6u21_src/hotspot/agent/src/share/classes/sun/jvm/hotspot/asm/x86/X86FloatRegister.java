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

import sun.jvm.hotspot.asm.Register;
import sun.jvm.hotspot.utilities.Assert;

public class X86FloatRegister extends Register {

   public X86FloatRegister(int number) {
      super(number);
   }

   public int getNumber() {
      return number;
   }

   public int getNumberOfRegisters() {
      return X86FloatRegisters.getNumRegisters();
   }

   public boolean isFloat() {
      return true;
   }

   public boolean isFramePointer() {
      return false;
   }

   public boolean isStackPointer() {
      return false;
   }

   public boolean isValid() {
      return number >= 0 && number < X86FloatRegisters.getNumRegisters();
   }

   public String toString() {
      return X86FloatRegisters.getRegisterName(number);
   }

}
