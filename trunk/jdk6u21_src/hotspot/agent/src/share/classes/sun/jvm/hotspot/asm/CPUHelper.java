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

package sun.jvm.hotspot.asm;

public interface CPUHelper {
   public Disassembler createDisassembler(long startPc, byte[] code);
   public Register getIntegerRegister(int num);
   public Register getFloatRegister(int num);
   public Register getStackPointer();
   public Register getFramePointer();
}
