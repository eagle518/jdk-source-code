/*
 * Copyright (c) 1998, 2006, Oracle and/or its affiliates. All rights reserved.
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
#include "incls/_preserveException.cpp.incl"

// TODO: These three classes should be refactored

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


// This code is cloned from PreserveExceptionMark, except that:
//   returned pending exceptions do not cause a crash.
//   thread is passed in, not set (not a reference parameter)
//   and bug 6431341 has been addressed.

CautiouslyPreserveExceptionMark::CautiouslyPreserveExceptionMark(Thread* thread) {
  _thread    = thread;
  _preserved_exception_oop = Handle(thread, _thread->pending_exception());
  _preserved_exception_line = _thread->exception_line();
  _preserved_exception_file = _thread->exception_file();
  _thread->clear_pending_exception(); // Pending exceptions are checked in the destructor
}


CautiouslyPreserveExceptionMark::~CautiouslyPreserveExceptionMark() {
  assert(!_thread->has_pending_exception(), "unexpected exception generated");
  if (_thread->has_pending_exception()) {
    _thread->clear_pending_exception();
  }
  if (_preserved_exception_oop() != NULL) {
    _thread->set_pending_exception(_preserved_exception_oop(), _preserved_exception_file, _preserved_exception_line);
  }
}


void WeakPreserveExceptionMark::preserve() {
  _preserved_exception_oop = Handle(_thread, _thread->pending_exception());
  _preserved_exception_line = _thread->exception_line();
  _preserved_exception_file = _thread->exception_file();
  _thread->clear_pending_exception();
}

void WeakPreserveExceptionMark::restore() {
  if (!_thread->has_pending_exception()) {
    _thread->set_pending_exception(_preserved_exception_oop(), _preserved_exception_file, _preserved_exception_line);
  }
}
