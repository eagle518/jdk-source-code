#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_Compiler.cpp	1.91 03/12/23 16:39:02 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

#include "incls/_precompiled.incl"
#include "incls/_c1_Compiler.cpp.incl"


Compiler::Compiler() {
}


Compiler::~Compiler() {
  Unimplemented();
}


void Compiler::initialize() {
  mark_initialized();
}


void Compiler::compile_method(ciEnv* env, ciMethod* method, int entry_bci) {
  CodeBuffer* code = Runtime1::new_code_buffer();
  C1_MacroAssembler* masm = new C1_MacroAssembler(code);
  // invoke compilation
  Compilation c(this, env, method, entry_bci, masm);
}


void Compiler::print_timers() {
  Compilation::print_timers();
}


void compiler1_init() {
  // This is needed as runtime1 stubs are generated always
  SharedInfo::set_stack0(FrameMap::stack0);
  // No initialization needed if interpreter only
  if (Arguments::mode() != Arguments::_int) {
    RegAlloc::init_register_allocation();
  }
  Runtime1::initialize();
}

