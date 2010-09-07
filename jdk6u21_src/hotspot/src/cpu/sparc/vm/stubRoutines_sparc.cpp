/*
 * Copyright (c) 1997, 2007, Oracle and/or its affiliates. All rights reserved.
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
#include "incls/_stubRoutines_sparc.cpp.incl"

// Implementation of the platform-specific part of StubRoutines - for
// a description of how to extend it, see the stubRoutines.hpp file.


extern "C" {
  address _flush_reg_windows();   // in .s file.
  // Flush registers to stack. In case of error we will need to stack walk.
  address bootstrap_flush_windows(void) {
    Thread* thread = ThreadLocalStorage::get_thread_slow();
    // Very early in process there is no thread.
    if (thread != NULL) {
      guarantee(thread->is_Java_thread(), "Not a Java thread.");
      JavaThread* jt = (JavaThread*)thread;
      guarantee(!jt->has_last_Java_frame(), "Must be able to flush registers!");
    }
    return (address)_flush_reg_windows();
  };
};

address StubRoutines::Sparc::_test_stop_entry = NULL;
address StubRoutines::Sparc::_stop_subroutine_entry = NULL;
address StubRoutines::Sparc::_flush_callers_register_windows_entry = CAST_FROM_FN_PTR(address, bootstrap_flush_windows);

address StubRoutines::Sparc::_partial_subtype_check = NULL;

int StubRoutines::Sparc::_atomic_memory_operation_lock = StubRoutines::Sparc::unlocked;

int StubRoutines::Sparc::_v8_oop_lock_cache[StubRoutines::Sparc::nof_v8_oop_lock_cache_entries];
