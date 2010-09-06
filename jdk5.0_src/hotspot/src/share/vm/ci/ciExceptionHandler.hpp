#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)ciExceptionHandler.hpp	1.5 03/12/23 16:39:28 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// ciExceptionHandler
//
// This class represents an exception handler for a method.
class ciExceptionHandler : public ResourceObj {
private:
  friend class ciMethod;

  // The loader to be used for resolving the exception
  // klass.
  ciInstanceKlass* _loading_klass;

  // Handler data.
  int _start;
  int _limit;
  int _handler_bci;
  int _catch_klass_index;

  // The exception klass that this handler catches.
  ciInstanceKlass* _catch_klass;

  ciExceptionHandler(ciInstanceKlass* loading_klass,
                     int start, int limit,
                     int handler_bci, int klass_index) {
    _loading_klass = loading_klass;
    _start  = start;
    _limit  = limit;
    _handler_bci = handler_bci;
    _catch_klass_index = klass_index;
    _catch_klass = NULL;
  }

public:
  int       start()             { return _start; }
  int       limit()             { return _limit; }
  int       handler_bci()       { return _handler_bci; }
  int       catch_klass_index() { return _catch_klass_index; }

  // Get the exception klass that this handler catches.
  ciInstanceKlass* catch_klass();

  bool      is_catch_all() { return catch_klass_index() == 0; }
  bool      is_in_range(int bci) {
    return start() <= bci && bci < limit();
  }
  bool      catches(ciInstanceKlass *exc) {
    return is_catch_all() || exc->is_subtype_of(catch_klass());
  }
  bool      is_rethrow() { return handler_bci() == -1; }

  void      print();
};

