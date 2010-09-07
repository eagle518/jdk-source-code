/*
 * Copyright (c) 2003, 2008, Oracle and/or its affiliates. All rights reserved.
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
#include "incls/_thread_solaris_x86.cpp.incl"

// For Forte Analyzer AsyncGetCallTrace profiling support - thread is
// currently interrupted by SIGPROF
bool JavaThread::pd_get_top_frame_for_signal_handler(frame* fr_addr,
  void* ucontext, bool isInJava) {

  assert(Thread::current() == this, "caller must be current thread");
  assert(this->is_Java_thread(), "must be JavaThread");
  JavaThread* jt = (JavaThread *)this;

  // last_Java_frame is always walkable and safe use it if we have it

  if (jt->has_last_Java_frame()) {
    *fr_addr = jt->pd_last_frame();
    return true;
  }

  ucontext_t* uc = (ucontext_t*) ucontext;

  // We always want to use the initial frame we create from the ucontext as
  // it certainly signals where we currently are. However that frame may not
  // be safe for calling sender. In that case if we have a last_Java_frame
  // then the forte walker will switch to that frame as the virtual sender
  // for the frame we create here which is not sender safe.

  intptr_t* ret_fp;
  intptr_t* ret_sp;
  ExtendedPC addr = os::Solaris::fetch_frame_from_ucontext(this, uc, &ret_sp, &ret_fp);

  // Something would really have to be screwed up to get a NULL pc

  if (addr.pc() == NULL ) {
    assert(false, "NULL pc from signal handler!");
    return false;

  }

  // If sp and fp are nonsense just leave them out

  if ((address)ret_sp >= jt->stack_base() ||
      (address)ret_sp < jt->stack_base() - jt->stack_size() ) {

      ret_sp = NULL;
      ret_fp = NULL;
  } else {

    // sp is reasonable is fp reasonable?
    if ( (address)ret_fp >= jt->stack_base() || ret_fp < ret_sp) {
      ret_fp = NULL;
    }
  }

  frame ret_frame(ret_sp, ret_fp, addr.pc());

  *fr_addr = ret_frame;
  return true;

}
