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

// address is specified as an offset from current PC

public class X86PCRelativeAddress extends PCRelativeAddress {
   private int instrSize;

   public X86PCRelativeAddress(long disp) {
       super(disp);
   }


   public void setInstructionSize(int size) {
      instrSize = size;
   }

   public String toString() {
      long displacement = this.getDisplacement();
      return new Long(displacement).toString();
   }

   //In intel assembly the displacement is from start of next instruction.
   //So we add the size of current instruction to get the correct disp.
   public long getDisplacement() {
      long displacement = super.getDisplacement() + (long)instrSize;
      return displacement;
   }
}
