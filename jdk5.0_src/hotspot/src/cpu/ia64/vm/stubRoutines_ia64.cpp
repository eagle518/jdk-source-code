#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)stubRoutines_ia64.cpp	1.12 04/01/08 01:01:19 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_stubRoutines_ia64.cpp.incl"

// Implementation of the platform-specific part of StubRoutines - for
// a description of how to extend it, see the stubRoutines.hpp file.

extern "C" {
  // At bootstrap time we can do native transitions but don't really need the
  // registers flushed to the stack
  void bootstrap_flush_windows(void) { 
    JavaThread* jt = JavaThread::current();
    guarantee(jt != NULL, "NO THREAD!");
    guarantee(!jt->has_last_Java_frame(), "Must be able to flush registers!");
  };
};

address StubRoutines::ia64::_flush_register_stack_entry       = CAST_FROM_FN_PTR(address, bootstrap_flush_windows);

address StubRoutines::ia64::_handler_for_divide_by_zero_entry = NULL;
address StubRoutines::ia64::_handler_for_null_exception_entry = NULL;
address StubRoutines::ia64::_handler_for_stack_overflow_entry = NULL;

address StubRoutines::ia64::_get_previous_fp_entry            = NULL;
address StubRoutines::ia64::_acquire_entry                    = NULL;
address StubRoutines::ia64::_partial_subtype_check            = NULL;
address StubRoutines::ia64::_get_backing_store_pointer        = NULL;

address StubRoutines::ia64::_ldffill_entry		      = NULL;
