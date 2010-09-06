#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c2compiler.cpp	1.19 03/12/23 16:42:15 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_c2compiler.cpp.incl"


const char* C2Compiler::retry_no_subsuming_loads() {
  return "retry without subsuming loads";
}


void C2Compiler::initialize() {
  mark_initialized();
}

void C2Compiler::compile_method(ciEnv* env,
                                ciMethod* target,
                                int entry_bci) {
  if (target->flags().is_native()) {
    Compile C(env, this, target);
  } else {
    bool subsume_loads = true;
    while (!env->failing()) {
      // Attempt to compile while subsuming loads into machine instructions.
      Compile C(env, this, target, entry_bci, subsume_loads);

      // Check result and retry if appropriate.
      if (C.failure_reason() != NULL) {
        if (0 == strcmp(C.failure_reason(), retry_no_subsuming_loads())) {
          assert(subsume_loads, "must make progress");
          subsume_loads = false;
          continue;  // retry
        }
        // Pass any other failure reason up to the ciEnv.
        // Note that serious, irreversible failures are already logged
        // on the ciEnv via env->record_method_not_compilable().
        env->record_failure(C.failure_reason());
      }

      // No retry; just break the loop.
      break;
    }
  }
}

void C2Compiler::compile_adapter(ciEnv* env,
                                 ciMethod* callee,
                                 int/*AdapterType*/ type) {
  Compile C(env, callee, type);
  if (C.failing()) {
    if (PrintCompilation) {
      ttyLocker ttyl;  // keep the following output all in one block
      tty->print("Adapter for method not compilable: ");
      callee->print_name();
      tty->cr();
    }
    callee->set_not_compilable();
  }
}

void C2Compiler::print_timers() {
  // do nothing
}
