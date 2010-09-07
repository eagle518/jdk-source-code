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

package sun.jvm.hotspot.asm.amd64;

import sun.jvm.hotspot.asm.Register;
import sun.jvm.hotspot.utilities.Assert;

public class AMD64FloatRegister extends Register {

   public AMD64FloatRegister(int number) {
      super(number);
   }

   public int getNumber() {
      return number;
   }

   public int getNumberOfRegisters() {
      return AMD64FloatRegisters.getNumRegisters();
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
      return number >= 0 && number < AMD64FloatRegisters.getNumRegisters();
   }

   public String toString() {
      return AMD64FloatRegisters.getRegisterName(number);
   }

}
