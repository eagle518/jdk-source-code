#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_CodePatterns.hpp	1.15 03/12/23 16:39:00 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class CodeEmitInfo;

// Note: we may want to merge C1_Macroassembler and C1_CodePatterns
class C1_CodePatterns: public C1_MacroAssembler {
 public:
  C1_CodePatterns(CodeBuffer* code)
    : C1_MacroAssembler(code) {}

  virtual C1_CodePatterns* as_CodePatterns()        { return this; }

  //----------------------------------------------------
  void explicit_null_check(Register base);

  void inline_cache_check(Register receiver, Register iCache);
  void build_frame(int frame_size_in_bytes);
  void method_exit(bool restore_frame);

  void fast_ObjectHashCode(Register receiver, Register result);
  void exception_handler(bool has_Java_exception_handler, int frame_size_in_bytes);
  void unverified_entry(Register receiver, Register ic_klass);
  void verified_entry();
};

