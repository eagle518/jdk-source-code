/*
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
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
#include "incls/_ciCallSite.cpp.incl"

// ciCallSite

// ------------------------------------------------------------------
// ciCallSite::get_target
//
// Return the target MethodHandle of this CallSite.
ciMethodHandle* ciCallSite::get_target() const {
  VM_ENTRY_MARK;
  oop method_handle_oop = java_dyn_CallSite::target(get_oop());
  return CURRENT_ENV->get_object(method_handle_oop)->as_method_handle();
}

// ------------------------------------------------------------------
// ciCallSite::print
//
// Print debugging information about the CallSite.
void ciCallSite::print() {
  Unimplemented();
}
