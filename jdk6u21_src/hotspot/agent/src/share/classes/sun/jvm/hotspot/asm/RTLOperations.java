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

public interface RTLOperations {

   // arithmetic operations
   public static final int RTLOP_ADD          = 0;
   // with carry
   public static final int RTLOP_ADDC         = 1;
   public static final int RTLOP_SUB          = 2;
   // with carry
   public static final int RTLOP_SUBC         = 3;
   public static final int RTLOP_SMUL         = 4;
   public static final int RTLOP_UMUL         = 5;
   public static final int RTLOP_SDIV         = 6;
   public static final int RTLOP_UDIV         = 7;

   public static final int RTLOP_MAX_ARITHMETIC = RTLOP_UDIV;

   // logical operations
   public static final int RTLOP_AND          = 8;
   public static final int RTLOP_OR           = 9;
   public static final int RTLOP_NOT          = 10;
   public static final int RTLOP_NAND         = 11;
   public static final int RTLOP_NOR          = 12;
   public static final int RTLOP_XOR          = 13;
   public static final int RTLOP_XNOR         = 14;

   public static final int RTLOP_MAX_LOGICAL  = RTLOP_XNOR;

   // shift operations
   public static final int RTLOP_SRL          = 15;
   public static final int RTLOP_SRA          = 16;
   public static final int RTLOP_SLL          = 17;

   public static final int RTLOP_MAX_SHIFT    = RTLOP_SLL;

   public static final int RTLOP_UNKNOWN      = Integer.MAX_VALUE;
}
