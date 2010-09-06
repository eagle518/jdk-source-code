#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)ciExceptionHandler.cpp	1.6 03/12/23 16:39:28 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_ciExceptionHandler.cpp.incl"

// ciExceptionHandler
//
// This class represents an exception handler for a method.

// ------------------------------------------------------------------
// ciExceptionHandler::catch_klass
// 
// Get the exception klass that this handler catches.
ciInstanceKlass* ciExceptionHandler::catch_klass() {
  assert(!is_catch_all(), "bad index");
  if (_catch_klass == NULL) {
    bool will_link;
    ciKlass* k = CURRENT_ENV->get_klass_by_index(_loading_klass,
                                                 _catch_klass_index,
                                                 will_link);
    if (!will_link && k->is_loaded()) {
      GUARDED_VM_ENTRY(
        k = CURRENT_ENV->get_unloaded_klass(_loading_klass, k->name());
      )
    }
    _catch_klass = k->as_instance_klass();
  }
  return _catch_klass;
}

// ------------------------------------------------------------------
// ciExceptionHandler::print()
void ciExceptionHandler::print() {
  tty->print("<ciExceptionHandler start=%d limit=%d"
             " handler_bci=%d ex_klass_index=%d",
             start(), limit(), handler_bci(), catch_klass_index());
  if (_catch_klass != NULL) {
    tty->print(" ex_klass=");
    _catch_klass->print();
  }
  tty->print(">");
}
