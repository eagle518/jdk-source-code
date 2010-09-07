/*
 * Copyright (c) 1997, 2008, Oracle and/or its affiliates. All rights reserved.
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

  // machine-dependent parts of class relocInfo
 private:
  enum {
    // Intel instructions are byte-aligned.
    offset_unit        =  1,

    // Encodes Assembler::disp32_operand vs. Assembler::imm32_operand.
#ifndef AMD64
    format_width       =  1
#else
    // vs Assembler::narrow_oop_operand.
    format_width       =  2
#endif
  };
