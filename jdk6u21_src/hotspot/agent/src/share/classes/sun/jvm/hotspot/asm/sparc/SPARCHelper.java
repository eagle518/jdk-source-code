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

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

public class SPARCHelper implements CPUHelper {
   public Disassembler createDisassembler(long startPc, byte[] code) {
      return new SPARCV9Disassembler(startPc, code);
   }

   public Register getIntegerRegister(int num) {
      return SPARCRegisters.getRegister(num);
   }

   public Register getFloatRegister(int num) {
      return SPARCFloatRegisters.getRegister(num);
   }

   public Register getStackPointer() {
      return SPARCRegisters.O7;
   }

   public Register getFramePointer() {
      return SPARCRegisters.I7;
   }
}
