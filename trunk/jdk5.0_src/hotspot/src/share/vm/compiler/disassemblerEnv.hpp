#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)disassemblerEnv.hpp	1.12 03/12/23 16:40:04 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Call-back interface for external disassembler
class DisassemblerEnv {
 public:
  // printing
  virtual void print_label(intptr_t value)   = 0;
  virtual void print_raw(char* str)     = 0;
  virtual void print(char* format, ...) = 0;
  // helpers
  virtual char* string_for_offset(intptr_t value) = 0;
  virtual char* string_for_constant(unsigned char* pc, intptr_t value, int is_decimal) = 0;
};

