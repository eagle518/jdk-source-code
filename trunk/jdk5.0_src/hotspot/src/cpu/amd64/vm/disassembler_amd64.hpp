#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)disassembler_amd64.hpp	1.2 03/12/23 16:35:45 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// The disassembler prints out amd64 code annotated
// with Java specific information.

class Disassembler 
{
#ifndef PRODUCT
 private:
  typedef address (*decode_func)(address start, DisassemblerEnv* env);
  // points the library.
  static void* _library;
  // points to the decode function.
  static decode_func _decode_instruction;
  // tries to load library and return whether it succedded.
  static bool load_library();
  // decodes one instruction and return the start of the next instruction.
  static address decode_instruction(address start, DisassemblerEnv* env);
#endif
 public:
  static void decode(CodeBlob *cb, outputStream* st = NULL) PRODUCT_RETURN;
  static void decode(nmethod* nm, outputStream* st = NULL) PRODUCT_RETURN;
  static void decode(u_char* begin, 
                     u_char* end, 
                     outputStream* st = NULL) PRODUCT_RETURN;
};
