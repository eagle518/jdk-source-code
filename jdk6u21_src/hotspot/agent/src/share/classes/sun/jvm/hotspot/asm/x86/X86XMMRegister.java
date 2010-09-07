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

public class X86XMMRegister extends X86Register {

   public X86XMMRegister(int num, String name) {
     super(num, name);
   }
   public int getNumberOfRegisters() {
     return X86XMMRegisters.getNumberOfRegisters();
   }
   public String toString() {
     return name;
   }
}
