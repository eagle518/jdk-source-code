#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)bytecodeStream.cpp	1.37 03/12/23 16:40:33 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_bytecodeStream.cpp.incl"


RawBytecodeStream::RawBytecodeStream(methodHandle method) {
  _method = method;
  set_interval(0, _method->code_size());
}


void RawBytecodeStream::set_interval(int beg_bci, int end_bci) {
  assert(0 <= beg_bci && beg_bci <= method()->code_size(), "illegal beg_bci");
  assert(0 <= end_bci && end_bci <= method()->code_size(), "illegal end_bci");
  // setup of iteration pointers
  _bci      = beg_bci;
  _next_bci = beg_bci;
  _end_bci  = end_bci;
}

