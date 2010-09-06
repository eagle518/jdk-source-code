#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_MacroAssembler.hpp	1.16 03/12/23 16:39:15 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class CodeEmitInfo;

class C1_MacroAssembler: public MacroAssembler {
 public:
  // creation
  C1_MacroAssembler(CodeBuffer* code) : MacroAssembler(code) { pd_init(); }

  //----------------------------------------------------
  void explicit_null_check(Register base);

  void inline_cache_check(Register receiver, Register iCache);
  void build_frame(int frame_size_in_bytes);
  void method_exit(bool restore_frame);

  void fast_ObjectHashCode(Register receiver, Register result);
  void exception_handler(bool has_Java_exception_handler, int frame_size_in_bytes);
  void unverified_entry(Register receiver, Register ic_klass);
  void verified_entry();

#include "incls/_c1_MacroAssembler_pd.hpp.incl"
};



// A StubAssembler is a MacroAssembler w/ extra functionality for runtime
// stubs. Currently it 'knows' some stub info. Eventually, the information
// may be set automatically or can be asserted when using specialised
// StubAssembler functions.

class StubAssembler: public C1_MacroAssembler {
 private:
  const char* _name;
  bool  _must_gc_arguments;

 public:
  // creation
  StubAssembler(CodeBuffer* code);
  void set_info(const char* name, bool must_gc_arguments);

  // accessors
  const char* name() const                       { return _name; }
  bool  must_gc_arguments() const                { return _must_gc_arguments; }

  // runtime calls (return offset of call to be used by GC map)
  int call_RT(Register oop_result1, Register oop_result2, address entry, int args_size = 0);
  int call_RT(Register oop_result1, Register oop_result2, address entry, Register arg1);
  int call_RT(Register oop_result1, Register oop_result2, address entry, Register arg1, Register arg2);
  int call_RT(Register oop_result1, Register oop_result2, address entry, Register arg1, Register arg2, Register arg3);
};


