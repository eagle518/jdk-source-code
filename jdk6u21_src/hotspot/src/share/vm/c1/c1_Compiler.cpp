/*
 * Copyright (c) 1999, 2007, Oracle and/or its affiliates. All rights reserved.
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

#include "incls/_precompiled.incl"
#include "incls/_c1_Compiler.cpp.incl"

volatile int Compiler::_runtimes = uninitialized;

volatile bool Compiler::_compiling = false;


Compiler::Compiler() {
}


Compiler::~Compiler() {
  Unimplemented();
}


void Compiler::initialize() {
  if (_runtimes != initialized) {
    initialize_runtimes( Runtime1::initialize, &_runtimes);
  }
  mark_initialized();
}


void Compiler::compile_method(ciEnv* env, ciMethod* method, int entry_bci) {

  if (!is_initialized()) {
    initialize();
  }
  // invoke compilation
#ifdef TIERED
  // We are thread in native here...
  CompilerThread* thread = CompilerThread::current();
  {
    ThreadInVMfromNative tv(thread);
    MutexLocker only_one (C1_lock, thread);
    while ( _compiling) {
      C1_lock->wait();
    }
    _compiling = true;
  }
#endif // TIERED
  {
    // We are nested here because we need for the destructor
    // of Compilation to occur before we release the any
    // competing compiler thread
    ResourceMark rm;
    Compilation c(this, env, method, entry_bci);
  }
#ifdef TIERED
  {
    ThreadInVMfromNative tv(thread);
    MutexLocker only_one (C1_lock, thread);
    _compiling = false;
    C1_lock->notify();
  }
#endif // TIERED
}


void Compiler::print_timers() {
  Compilation::print_timers();
}
