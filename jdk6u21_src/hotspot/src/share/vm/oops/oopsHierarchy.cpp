/*
 * Copyright (c) 2005, 2006, Oracle and/or its affiliates. All rights reserved.
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

# include "incls/_precompiled.incl"
# include "incls/_oopsHierarchy.cpp.incl"

#ifdef CHECK_UNHANDLED_OOPS

void oop::register_oop() {
  assert (CheckUnhandledOops, "should only call when CheckUnhandledOops");
  if (!Universe::is_fully_initialized()) return;
  // This gets expensive, which is why checking unhandled oops is on a switch.
  Thread* t = ThreadLocalStorage::thread();
  if (t != NULL && t->is_Java_thread()) {
     frame fr = os::current_frame();
     // This points to the oop creator, I guess current frame points to caller
     assert (fr.pc(), "should point to a vm frame");
     t->unhandled_oops()->register_unhandled_oop(this, fr.pc());
  }
}

void oop::unregister_oop() {
  assert (CheckUnhandledOops, "should only call when CheckUnhandledOops");
  if (!Universe::is_fully_initialized()) return;
  // This gets expensive, which is why checking unhandled oops is on a switch.
  Thread* t = ThreadLocalStorage::thread();
  if (t != NULL && t->is_Java_thread()) {
    t->unhandled_oops()->unregister_unhandled_oop(this);
  }
}
#endif // CHECK_UNHANDLED_OOPS
