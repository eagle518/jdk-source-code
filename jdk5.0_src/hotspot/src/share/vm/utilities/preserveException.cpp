#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)preserveException.cpp	1.8 03/12/23 16:44:50 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_preserveException.cpp.incl"

PreserveExceptionMark::PreserveExceptionMark(Thread*& thread) {
  thread     = Thread::current();
  _thread    = thread;
  _preserved_exception_oop = Handle(thread, _thread->pending_exception());
  _thread->clear_pending_exception(); // Needed to avoid infinite recursion  
  _preserved_exception_line = _thread->exception_line();
  _preserved_exception_file = _thread->exception_file();
}


PreserveExceptionMark::~PreserveExceptionMark() {
  if (_thread->has_pending_exception()) {
    oop exception = _thread->pending_exception();
    _thread->clear_pending_exception(); // Needed to avoid infinite recursion
    exception->print();
    fatal("PreserveExceptionMark destructor expects no pending exceptions");
  }
  if (_preserved_exception_oop() != NULL) {
    _thread->set_pending_exception(_preserved_exception_oop(), _preserved_exception_file, _preserved_exception_line);
  }
}

