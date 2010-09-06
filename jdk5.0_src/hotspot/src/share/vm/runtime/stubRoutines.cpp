#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)stubRoutines.cpp	1.98 04/03/23 07:42:33 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_stubRoutines.cpp.incl"


// Implementation of StubRoutines - for a description
// of how to extend it, see the header file.

// Class Variables

BufferBlob* StubRoutines::_code1                                = NULL;
BufferBlob* StubRoutines::_code2                                = NULL;

address StubRoutines::_call_stub_return_address                 = NULL;
address StubRoutines::_call_stub_entry                          = NULL;

address StubRoutines::_catch_exception_entry                    = NULL;
address StubRoutines::_forward_exception_entry                  = NULL;
address StubRoutines::_throw_AbstractMethodError_entry          = NULL;
address StubRoutines::_throw_ArithmeticException_entry          = NULL;
address StubRoutines::_throw_NullPointerException_entry         = NULL;
address StubRoutines::_throw_NullPointerException_at_call_entry = NULL;
address StubRoutines::_throw_StackOverflowError_entry           = NULL;
jint    StubRoutines::_verify_oop_count                         = 0;
address StubRoutines::_verify_oop_subroutine_entry              = NULL;
address StubRoutines::_atomic_xchg_entry                        = NULL;
address StubRoutines::_atomic_xchg_ptr_entry                    = NULL;
address StubRoutines::_atomic_store_entry                       = NULL;
address StubRoutines::_atomic_store_ptr_entry                   = NULL;
address StubRoutines::_atomic_cmpxchg_entry                     = NULL;
address StubRoutines::_atomic_cmpxchg_ptr_entry                 = NULL;
address StubRoutines::_atomic_cmpxchg_long_entry                = NULL;
address StubRoutines::_atomic_add_entry                         = NULL;
address StubRoutines::_atomic_add_ptr_entry                     = NULL;
address StubRoutines::_fence_entry                              = NULL;
address StubRoutines::_d2i_wrapper                              = NULL;
address StubRoutines::_d2l_wrapper                              = NULL;

jint    StubRoutines::_fpu_cntrl_wrd_std                        = NULL;
jint    StubRoutines::_fpu_cntrl_wrd_24                         = NULL;
jint    StubRoutines::_fpu_cntrl_wrd_64                         = NULL;
jint    StubRoutines::_fpu_cntrl_wrd_trunc                      = NULL;
jint    StubRoutines::_mxcsr_std                                = NULL;
jint    StubRoutines::_fpu_subnormal_bias1[3]                   = { 0, 0, 0 };
jint    StubRoutines::_fpu_subnormal_bias2[3]                   = { 0, 0, 0 };


// Initialization
//
// Note: to break cycle with universe initialization, stubs are generated in two phases.
// The first one generates stubs needed during universe init (e.g., _handle_must_compile_first_entry).
// The second phase includes all other stubs (which may depend on universe being initialized.)

extern void StubGenerator_generate(CodeBuffer* code, bool all); // only interface to generators

void StubRoutines::initialize1() {
  if (_code1 == NULL) {
    ResourceMark rm;
    TraceTime timer("StubRoutines generation 1", TraceStartupTime);
    _code1 = BufferBlob::create("StubRoutines (1)", code_size1);
    if( _code1 == NULL) fatal1( "CodeCache: no room for %s", "StubRoutines (1)");
    CodeBuffer* buffer = new CodeBuffer(_code1->instructions_begin(), _code1->instructions_size());
    StubGenerator_generate(buffer, false);
  }
}


void StubRoutines::initialize2() {
  if (_code2 == NULL) {
    ResourceMark rm;
    TraceTime timer("StubRoutines generation 2", TraceStartupTime);
    _code2 = BufferBlob::create("StubRoutines (2)", code_size2);
    if( _code2 == NULL) fatal1( "CodeCache: no room for %s", "StubRoutines (2)");
    CodeBuffer* buffer = new CodeBuffer(_code2->instructions_begin(), _code2->instructions_size());
    StubGenerator_generate(buffer, true);
  }
}


void stubRoutines_init1() { StubRoutines::initialize1(); }
void stubRoutines_init2() { StubRoutines::initialize2(); }
