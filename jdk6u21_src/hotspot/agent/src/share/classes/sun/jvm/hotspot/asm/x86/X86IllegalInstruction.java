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

import sun.jvm.hotspot.asm.SymbolFinder;

public final class X86IllegalInstruction extends X86Instruction {
   final private String description;

   public X86IllegalInstruction() {
      super("illegal", 1, 0);
      description = "bad opcode";
   }

   public String asString(long currentPc, SymbolFinder symFinder) {
      return description;
   }

   public boolean isIllegal() {
      return true;
   }
}
