//
// Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
// ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//

#include "incls/_precompiled.incl"
#include "incls/_abstractCompiler.cpp.incl"

void AbstractCompiler::initialize_runtimes(initializer f, volatile int* state) {
  if (*state != initialized) {

    // We are thread in native here...
    CompilerThread* thread = CompilerThread::current();
    bool do_initialization = false;
    {
      ThreadInVMfromNative tv(thread);
      MutexLocker only_one(CompileThread_lock, thread);
      if ( *state == uninitialized) {
        do_initialization = true;
        *state = initializing;
      } else {
        while (*state == initializing ) {
          CompileThread_lock->wait();
        }
      }
    }
    if (do_initialization) {
      // We can not hold any locks here since JVMTI events may call agents

      // Compiler(s) run as native

      (*f)();

      // To in_vm so we can use the lock

      ThreadInVMfromNative tv(thread);
      MutexLocker only_one(CompileThread_lock, thread);
      assert(*state == initializing, "wrong state");
      *state = initialized;
      CompileThread_lock->notify_all();
    }
  }
}
