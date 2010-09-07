/*
 * Copyright (c) 2003, 2007, Oracle and/or its affiliates. All rights reserved.
 * Copyright 2008 Red Hat, Inc.
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
#include "incls/_nativeInst_zero.cpp.incl"

// This method is called by nmethod::make_not_entrant_or_zombie to
// insert a jump to SharedRuntime::get_handle_wrong_method_stub()
// (dest) at the start of a compiled method (verified_entry) to avoid
// a race where a method is invoked while being made non-entrant.
//
// In Shark, verified_entry is a pointer to a SharkEntry.  We can
// handle this simply by changing it's entry point to point at the
// interpreter.  This only works because the interpreter and Shark
// calling conventions are the same.

void NativeJump::patch_verified_entry(address entry,
                                      address verified_entry,
                                      address dest) {
  assert(dest == SharedRuntime::get_handle_wrong_method_stub(), "should be");

#ifdef CC_INTERP
  ((ZeroEntry*) verified_entry)->set_entry_point(
    (address) CppInterpreter::normal_entry);
#else
  Unimplemented();
#endif // CC_INTERP
}
