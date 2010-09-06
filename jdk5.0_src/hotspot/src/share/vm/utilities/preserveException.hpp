#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)preserveException.hpp	1.7 03/12/23 16:44:50 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Thid file provides more support for exception handling; see alos exceptions.hpp
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



// use global exception mark when allowing pending exception to be set and
// saving and restoring them 
#define PRESERVE_EXCEPTION_MARK                    Thread* THREAD; PreserveExceptionMark __em(THREAD);

