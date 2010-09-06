#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)disassembler_sparc.hpp	1.19 03/12/23 16:37:09 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// The disassembler prints out sparc code annotated
// with Java specific information.

class Disassembler {
#ifndef PRODUCT
 private:
  // points to the library.
  static void*    _library;
  // points to the print_insn_sparc function.
  static dll_func _print_insn_sparc;
  // tries to load library and return whether it succedded.
  static bool load_library();
  // decodes one instruction and return the start of the next instruction.
  static address decode_instruction(address start, DisassemblerEnv* env);
#endif
 public:
  static void decode(CodeBlob *cb,               outputStream* st = NULL) PRODUCT_RETURN;
  static void decode(nmethod* nm,                outputStream* st = NULL) PRODUCT_RETURN;
  static void decode(u_char* begin, u_char* end, outputStream* st = NULL) PRODUCT_RETURN;
};

//Reconciliation History
// 1.9 98/04/29 10:45:51 disassembler_i486.hpp
// 1.10 98/05/11 16:47:20 disassembler_i486.hpp
// 1.12 99/06/22 16:37:37 disassembler_i486.hpp
// 1.13 99/08/06 10:09:04 disassembler_i486.hpp
//End
