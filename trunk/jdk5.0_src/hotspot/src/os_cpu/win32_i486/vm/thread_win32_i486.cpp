#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)thread_win32_i486.cpp	1.4 04/03/03 17:21:00 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_thread_win32_i486.cpp.incl"

// For Forte Analyzer AsyncGetCallTrace profiling support - thread is
// currently interrupted by SIGPROF
bool JavaThread::pd_get_top_frame_for_signal_handler(frame* fr_addr,
  void* ucontext, bool isInJava) {

  assert(Thread::current() == this, "caller must be current thread");
  assert(this->is_Java_thread(), "must be JavaThread");

  JavaThread* jt = (JavaThread *)this;

  // If we have a last_Java_frame, then we should use it even if
  // isInJava == true.  It should be more reliable than CONTEXT info.
  if (jt->has_last_Java_frame()) {
    *fr_addr = jt->pd_last_frame();
    return true;
  }

  // At this point, we don't have a last_Java_frame, so
  // we try to glean some information out of the CONTEXT
  // if we were running Java code when SIGPROF came in.
  if (isInJava) {
    CONTEXT* uc = (CONTEXT*)ucontext;

    // copied this idiom from os::fetch_top_frame()
    intptr_t* ret_fp = (intptr_t*) uc->Ebp;
    intptr_t* ret_sp = (intptr_t*) uc->Esp;
    ExtendedPC addr = ExtendedPC((address)uc->Eip);
    if (addr.contained_pc() == NULL || ret_sp == NULL || ret_fp == NULL) {
      // CONTEXT wasn't useful
      return false;
    }

    frame ret_frame(ret_sp, ret_fp, addr.pc());
    if (!ret_frame.safe_for_sender(jt)) {
      // nothing else to try if the frame isn't good
      return false;
    }
    *fr_addr = ret_frame;
    return true;
  }

  // nothing else to try
  return false;
}
