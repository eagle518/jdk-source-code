/*
 * Copyright (c) 2001, 2002, Oracle and/or its affiliates. All rights reserved.
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

public class X86Register extends Register {
   protected String name;
   public X86Register(int num, String name) {
     super(num);
     this.name = name;
   }
   public int getNumberOfRegisters() {
     return X86Registers.getNumberOfRegisters();
   }
   public String toString() {
     return name;
   }
   public boolean isFramePointer() {
     return number == 5; //ebp
   }
   public boolean isStackPointer() {
     return number == 4; //esp
   }
   public boolean isFloat() {
     return false;
   }
   public boolean isSegmentPointer() {
     return false;
   }
}
