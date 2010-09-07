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
#include "incls/_ciMethodHandle.cpp.incl"

// ciMethodHandle

// ------------------------------------------------------------------
// ciMethodHandle::get_adapter
//
// Return an adapter for this MethodHandle.
ciMethod* ciMethodHandle::get_adapter(bool is_invokedynamic) const {
  VM_ENTRY_MARK;

  Handle h(get_oop());
  methodHandle callee(_callee->get_methodOop());
  MethodHandleCompiler mhc(h, callee, is_invokedynamic, THREAD);
  methodHandle m = mhc.compile(CHECK_NULL);
  return CURRENT_ENV->get_object(m())->as_method();
}


// ------------------------------------------------------------------
// ciMethodHandle::print_impl
//
// Implementation of the print method.
void ciMethodHandle::print_impl(outputStream* st) {
  st->print(" type=");
  get_oop()->print();
}
