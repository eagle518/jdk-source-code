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


public class AMD64Helper implements CPUHelper {
   public Disassembler createDisassembler(long startPc, byte[] code) {
      // FIXME: no disassembler yet
      return null;
   }

   public Register getIntegerRegister(int num) {
      return AMD64Registers.getRegister(num);
   }

   public Register getFloatRegister(int num) {
      return AMD64FloatRegisters.getRegister(num);
   }

   public Register getStackPointer() {
      return AMD64Registers.RSP;
   }

   public Register getFramePointer() {
      return AMD64Registers.RBP;
   }
}
