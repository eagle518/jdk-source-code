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

import sun.jvm.hotspot.utilities.Assert;

public class X86FloatRegisters {

   public static int getNumRegisters() {
      return NUM_REGISTERS;
   }

   public static X86FloatRegister getRegister(int regNum) {
      if (Assert.ASSERTS_ENABLED) {
         Assert.that(regNum > -1 && regNum < NUM_REGISTERS, "invalid float register number!");
      }
      return registers[regNum];
   }

   public static String getRegisterName(int i) {
      return "ST(" + i + ")";
   }

   public static final X86FloatRegister ST0;
   public static final X86FloatRegister ST1;
   public static final X86FloatRegister ST2;
   public static final X86FloatRegister ST3;
   public static final X86FloatRegister ST4;
   public static final X86FloatRegister ST5;
   public static final X86FloatRegister ST6;
   public static final X86FloatRegister ST7;

   public static final int NUM_REGISTERS = 8;

   private static final X86FloatRegister registers[];

   static {
      ST0 = new X86FloatRegister(0);
      ST1 = new X86FloatRegister(1);
      ST2 = new X86FloatRegister(2);
      ST3 = new X86FloatRegister(3);
      ST4 = new X86FloatRegister(4);
      ST5 = new X86FloatRegister(5);
      ST6 = new X86FloatRegister(6);
      ST7 = new X86FloatRegister(7);
      registers = (new X86FloatRegister[] {
         ST0, ST1, ST2, ST3, ST4, ST5, ST6, ST7
      });
   }
}
