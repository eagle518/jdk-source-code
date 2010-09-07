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

package sun.jvm.hotspot.asm.x86;

import sun.jvm.hotspot.asm.*;


public class X86Helper implements CPUHelper {
   public Disassembler createDisassembler(long startPc, byte[] code) {
       return new X86Disassembler(startPc, code);
   }

   public Register getIntegerRegister(int num) {
      return X86Registers.getRegister32(num);
   }

   public Register getFloatRegister(int num) {
      return X86FloatRegisters.getRegister(num);
   }

   public Register getStackPointer() {
      return X86Registers.ESP;
   }

   public Register getFramePointer() {
      return X86Registers.EBP;
   }
}
