/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_thread_linux_ia64.cpp.incl"

#if 0
// Forte Analyzer AsyncGetCallTrace profiling support is not implemented
// on Linux IA64 (yet).
bool JavaThread::pd_get_top_frame_for_signal_handler(frame* fr_addr,
  void* ucontext, bool isInJava) {
  ShouldNotReachHere();
  return false;
}
#endif
