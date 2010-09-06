#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)thread_win32_ia64.cpp	1.4 04/03/03 17:21:01 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_thread_win32_ia64.cpp.incl"

#if 0
// Forte Analyzer AsyncGetCallTrace profiling support is not implemented
// on Windows IA64 (yet).
bool JavaThread::pd_get_top_frame_for_signal_handler(frame* fr_addr,
  void* ucontext, bool isInJava) {
  ShouldNotReachHere();
  return false;
}
#endif
