#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)thread_solaris_sparc.cpp	1.8 04/03/03 17:20:58 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_thread_solaris_sparc.cpp.incl"

// For Forte Analyzer AsyncGetCallTrace profiling support - thread is
// currently interrupted by SIGPROF
//
// NOTE: On Solaris, register windows are flushed in the signal handler
// except for possibly the top frame.
//
bool JavaThread::pd_get_top_frame_for_signal_handler(frame* fr_addr,
  void* ucontext, bool isInJava) {

  assert(Thread::current() == this, "caller must be current thread");
  assert(this->is_Java_thread(), "must be JavaThread");

  JavaThread* jt = (JavaThread *)this;

  if (!isInJava) {
    // make_walkable flushes register windows and grabs last_Java_pc
    // which can not be done if the ucontext sp matches last_Java_sp
    // stack walking utilities assume last_Java_pc set if marked flushed
    jt->frame_anchor()->make_walkable(jt);
  }

  // If we have a walkable last_Java_frame, then we should use it
  // even if isInJava == true. It should be more reliable than
  // ucontext info.
  if (jt->has_last_Java_frame() && jt->frame_anchor()->walkable()) {
#if 0
    // This sanity check may not be needed with the new frame
    // walking code. Remove it for now.
    if (!jt->frame_anchor()->post_Java_state_is_pc()
    && frame::next_younger_sp_or_null(last_Java_sp(),
    jt->frame_anchor()->post_Java_sp()) == NULL) {
      // the anchor contains an SP, but the frame is not walkable
      // because post_Java_sp isn't valid relative to last_Java_sp
      return false;
    }
#endif
    *fr_addr = jt->pd_last_frame();
    return true;
  }

  ucontext_t* uc = (ucontext_t*) ucontext;

  // At this point, we don't have a walkable last_Java_frame, so
  // we try to glean some information out of the ucontext.
  intptr_t* ret_sp;
  ExtendedPC addr = os::Solaris::fetch_frame_from_ucontext(this, uc,
    &ret_sp, NULL /* ret_fp only used on Solaris X86 */);
  if (addr.contained_pc() == NULL || ret_sp == NULL) {
    // ucontext wasn't useful
    return false;
  }

  // we were running Java code when SIGPROF came in
  if (isInJava) {
    // If we have a last_Java_sp, then the SIGPROF signal caught us
    // right when we were transitioning from _thread_in_Java to a new
    // JavaThreadState. We use last_Java_sp instead of the sp from
    // the ucontext since it should be more reliable.
    if (jt->has_last_Java_frame()) {
      ret_sp = jt->last_Java_sp();
    }
    // Implied else: we don't have a last_Java_sp so we use what we
    // got from the ucontext.

    frame ret_frame(ret_sp, frame::unpatchable, addr.pc());
    if (!ret_frame.safe_for_sender(jt)) {
      // nothing else to try if the frame isn't good
      return false;
    }
    *fr_addr = ret_frame;
    return true;
  }

  // At this point, we know we weren't running Java code. We might
  // have a last_Java_sp, but we don't have a walkable frame.
  // However, we might still be able to construct something useful
  // if the thread was running native code.
  if (jt->has_last_Java_frame()) {
    assert(!jt->frame_anchor()->walkable(), "case covered above");

    if (jt->thread_state() == _thread_in_native) {
      frame ret_frame(jt->last_Java_sp(), frame::unpatchable, addr.pc());
      if (!ret_frame.safe_for_sender(jt)) {
        // nothing else to try if the frame isn't good
        return false;
      }
      *fr_addr = ret_frame;
      return true;
    }
  }

  // nothing else to try
  return false;
}
