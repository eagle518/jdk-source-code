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

import sun.jvm.hotspot.asm.*;

public class AMD64Register extends Register {
   protected String name;
   public AMD64Register(int num, String name) {
     super(num);
     this.name = name;
   }
   public int getNumberOfRegisters() {
     return AMD64Registers.getNumberOfRegisters();
   }
   public String toString() {
     return name;
   }
   public boolean isFramePointer() {
     return number == 5; //rbp
   }
   public boolean isStackPointer() {
     return number == 4; //rsp
   }
   public boolean isFloat() {
     return false;
   }
   public boolean isSegmentPointer() {
     return false;
   }
}
