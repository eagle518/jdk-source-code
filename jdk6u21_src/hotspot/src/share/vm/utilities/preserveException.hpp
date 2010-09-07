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

// This file provides more support for exception handling; see also exceptions.hpp
class PreserveExceptionMark {
 private:
  Thread*     _thread;
  Handle      _preserved_exception_oop;
  int         _preserved_exception_line;
  const char* _preserved_exception_file;

 public:
  PreserveExceptionMark(Thread*& thread);
  ~PreserveExceptionMark();
};


// This is a clone of PreserveExceptionMark which asserts instead
// of failing when what it wraps generates a pending exception.
// It also addresses bug 6431341.
class CautiouslyPreserveExceptionMark {
 private:
  Thread*     _thread;
  Handle      _preserved_exception_oop;
  int         _preserved_exception_line;
  const char* _preserved_exception_file;

 public:
  CautiouslyPreserveExceptionMark(Thread* thread);
  ~CautiouslyPreserveExceptionMark();
};


// Like PreserveExceptionMark but allows new exceptions to be generated in
// the body of the mark. If a new exception is generated then the original one
// is discarded.
class WeakPreserveExceptionMark {
private:
  Thread*     _thread;
  Handle      _preserved_exception_oop;
  int         _preserved_exception_line;
  const char* _preserved_exception_file;

  void        preserve();
  void        restore();

  public:
    WeakPreserveExceptionMark(Thread* pThread) :  _thread(pThread), _preserved_exception_oop()  {
      if (pThread->has_pending_exception()) {
        preserve();
      }
    }
    ~WeakPreserveExceptionMark() {
      if (_preserved_exception_oop.not_null()) {
        restore();
      }
    }
};



// use global exception mark when allowing pending exception to be set and
// saving and restoring them
#define PRESERVE_EXCEPTION_MARK                    Thread* THREAD; PreserveExceptionMark __em(THREAD);
