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

package sun.jvm.hotspot.asm.ia64;

import sun.jvm.hotspot.asm.*;

public class IA64Helper implements CPUHelper {
   public Disassembler createDisassembler(long startPc, byte[] code) {
      // FIXME: IA64 disassembler not implemented
      return null;
   }

   public Register getIntegerRegister(int num) {
      // FIXME: IA64 disassembler not implemented
      return null;
   }

   public Register getFloatRegister(int num) {
      // FIXME: IA64 disassembler not implemented
      return null;
   }

   public Register getStackPointer() {
      // FIXME: IA64 disassembler not implemented
      return null;
   }

   public Register getFramePointer() {
      // FIXME: IA64 disassembler not implemented
      return null;
   }
}
