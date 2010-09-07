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

import sun.jvm.hotspot.utilities.*;

public class AMD64Registers {
   public static final int NUM_REGISTERS = 16;

   public static final AMD64Register RAX;
   public static final AMD64Register RCX;
   public static final AMD64Register RDX;
   public static final AMD64Register RBX;
   public static final AMD64Register RSP;
   public static final AMD64Register RBP;
   public static final AMD64Register RSI;
   public static final AMD64Register RDI;
   public static final AMD64Register R8;
   public static final AMD64Register R9;
   public static final AMD64Register R10;
   public static final AMD64Register R11;
   public static final AMD64Register R12;
   public static final AMD64Register R13;
   public static final AMD64Register R14;
   public static final AMD64Register R15;

   private static final AMD64Register[] registers;

   static {
      RAX = new AMD64Register(0, "rax");
      RCX = new AMD64Register(1, "rcx");
      RDX = new AMD64Register(2, "rdx");
      RBX = new AMD64Register(3, "rbx");
      RSP = new AMD64Register(4, "rsp");
      RBP = new AMD64Register(5, "rbp");
      RSI = new AMD64Register(6, "rsi");
      RDI = new AMD64Register(7, "rdi");
      R8  = new AMD64Register(8, "r8" );
      R9  = new AMD64Register(9, "r9" );
      R10 = new AMD64Register(10,"r10");
      R11 = new AMD64Register(11,"r11");
      R12 = new AMD64Register(12,"r12");
      R13 = new AMD64Register(13,"r13");
      R14 = new AMD64Register(14,"r14");
      R15 = new AMD64Register(15,"r15");
      registers = new AMD64Register[] {
                     RAX, RCX, RDX, RBX, RSP, RBP, RSI, RDI,
                     R8, R9, R10, R11, R12, R13, R14, R15
                  };
   }

   public static int getNumberOfRegisters() {
      return NUM_REGISTERS;
   }

   public static AMD64Register getRegister(int regNum) {
      if (Assert.ASSERTS_ENABLED) {
         Assert.that(regNum > -1 && regNum < NUM_REGISTERS, "invalid integer register number!");
      }
      return registers[regNum];
   }

   //Return the register name
   public static String getRegisterName(int regNum) {
      if (Assert.ASSERTS_ENABLED) {
         Assert.that(regNum > -1 && regNum < NUM_REGISTERS, "invalid integer register number!");
      }
      return registers[regNum].toString();
   }
}
