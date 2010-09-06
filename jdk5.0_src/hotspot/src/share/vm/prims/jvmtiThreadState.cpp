#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)jvmtiThreadState.cpp	1.23 03/12/23 16:43:25 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_jvmtiThreadState.cpp.incl"

// marker for when the stack depth has been reset and is now unknown.
// any negative number would work but small ones might obscure an
// underrun error.
static const int UNKNOWN_STACK_DEPTH = -99;

///////////////////////////////////////////////////////////////
//
// class JvmtiThreadState
//
// Instances of JvmtiThreadState hang off of each thread.
// Thread local storage for JVMTI.
//

JvmtiThreadState *JvmtiThreadState::_head = NULL;


JvmtiThreadState::JvmtiThreadState(JavaThread* thread)
  : _thread_event_enable()
  , _jvmdi_cached_frames(thread) {
  assert(JvmtiThreadState_lock->is_locked(), "sanity check");
  _thread               = thread;
  _exception_detected   = false;
  _exception_caught     = false;
  _debuggable           = true;
  _hide_single_stepping = false;
  _hide_level           = 0;
  _pending_step_for_popframe = false;
  _class_being_redefined = NULL;
  _dynamic_code_event_collector = NULL;
  _vm_object_alloc_event_collector = NULL;
  
  // add all the JvmtiEnvThreadState to the new JvmtiThreadState
  int ec = JvmtiEnvBase::env_count();
  _env_thread_states    = new (ResourceObj::C_HEAP) GrowableArray<JvmtiEnvThreadState *>(ec+1,true);
  for (int i = 0; i < ec ; ++i) {
    JvmtiEnvBase *env = JvmtiEnvBase::env_at(i);
    add_env(env);
  }
  assert(env_count() == ec, "Should now be one JvmtiEnvThreadState per environment");

  // link us into the list
  _prev = NULL;
  _next = _head;
  if (_head != NULL) {
    _head->_prev = this;
  }
  _head = this;

  // set this as the state for the thread
  thread->set_jvmti_thread_state(this);
}


JvmtiThreadState::~JvmtiThreadState()   { 
  assert(JvmtiThreadState_lock->is_locked(), "sanity check");

  // clear this as the state for the thread
  get_thread()->set_jvmti_thread_state(NULL);

  if (JvmtiExport::must_purge_jvmdi_frames_on_native_exit()) { 
    _jvmdi_cached_frames.destroy();
  }

  int ec = env_count();
  assert(ec == JvmtiEnvBase::env_count(), "Should be one JvmtiEnvThreadState per environment");
  for (int i = 0; i < ec ; ++i) {
    JvmtiEnvThreadState *ets = _env_thread_states->at(i);
    _env_thread_states->at_put(i, NULL);
    delete ets;
  }
  FreeHeap(_env_thread_states);
  _env_thread_states = NULL;

  // remove us from the list
  if (_prev == NULL) {
    assert(_head == this, "sanity check");
    _head = _next;
  } else {
    assert(_head != this, "sanity check");
    _prev->_next = _next;
  }
  if (_next != NULL) {
    _next->_prev = _prev;
  }
  _next = NULL;
  _prev = NULL;
}


void JvmtiThreadState::add_env(JvmtiEnvBase *env) {
  JvmtiEnvThreadState *ets = new JvmtiEnvThreadState(_thread, env);
  _env_thread_states->at_put_grow(env->index(), ets);
  assert(!_env_thread_states->contains(NULL), "_env_thread_states grow left gap");
}


void JvmtiThreadState::remove_env(JvmtiEnvBase *env) {
  int index = env->index();
  JvmtiEnvThreadState *ets = env_thread_state(index);
  _env_thread_states->remove_at(index);
  delete ets;
}


// to do: make this an inline - probably requires creating jvmtiThreadState_inline.hpp
JvmtiEnvThreadState* JvmtiThreadState::env_thread_state(JvmtiEnvBase *env) {
  return env_thread_state(env->index());
}


void JvmtiThreadState::enter_interp_only_mode() { 
  assert(_thread->get_interp_only_mode() == 0, "entering interp only when mode not zero");
  _thread->increment_interp_only_mode();
}


void JvmtiThreadState::leave_interp_only_mode() { 
  assert(_thread->get_interp_only_mode() == 1, "leaving interp only when mode not one");
  _thread->decrement_interp_only_mode();
}


// Helper routine used in several places
int JvmtiThreadState::count_frames() {
#ifdef ASSERT
  uint32_t debug_bits = 0;
#endif
  assert(SafepointSynchronize::is_at_safepoint() ||
         JvmtiEnv::is_thread_fully_suspended(get_thread(), false, &debug_bits),
         "at safepoint or must be suspended");

  if (!get_thread()->has_last_Java_frame()) return 0;  // no Java frames

  ResourceMark rm;
  RegisterMap reg_map(get_thread());
  javaVFrame *jvf = get_thread()->last_java_vframe(&reg_map);
  int n = 0;
  // tty->print_cr("CSD: counting frames on %s ...", 
  //               JvmtiTrace::safe_get_thread_name(get_thread()));
  while (jvf != NULL) {
    methodOop method = jvf->method();
    // tty->print_cr("CSD: frame - method %s.%s - loc %d",
    //               method->klass_name()->as_C_string(),
    //               method->name()->as_C_string(),
    //               jvf->bci() );
    jvf = jvf->java_sender();
    n++;
  }
  // tty->print_cr("CSD: frame count: %d", n);
  return n;
}


void JvmtiThreadState::invalidate_cur_stack_depth() {
  Thread *cur = Thread::current();
  uint32_t debug_bits = 0;

  // The caller can be the VMThread at a safepoint, the current thread
  // or the target thread must be suspended.
  guarantee((cur->is_VM_thread() && SafepointSynchronize::is_at_safepoint()) ||
    (JavaThread *)cur == get_thread() ||
    JvmtiEnv::is_thread_fully_suspended(get_thread(), false, &debug_bits),
    "sanity check");

  _cur_stack_depth = UNKNOWN_STACK_DEPTH;
}

void JvmtiThreadState::incr_cur_stack_depth() {
  guarantee(JavaThread::current() == get_thread(), "must be current thread");

  if (!is_interp_only_mode()) {
    _cur_stack_depth = UNKNOWN_STACK_DEPTH;
  }
  if (_cur_stack_depth != UNKNOWN_STACK_DEPTH) {
    ++_cur_stack_depth;
  }
}

void JvmtiThreadState::decr_cur_stack_depth() {
  guarantee(JavaThread::current() == get_thread(), "must be current thread");

  if (!is_interp_only_mode()) {
    _cur_stack_depth = UNKNOWN_STACK_DEPTH;
  }
  if (_cur_stack_depth != UNKNOWN_STACK_DEPTH) {
    --_cur_stack_depth;
    assert(_cur_stack_depth >= 0, "incr/decr_cur_stack_depth mismatch");
  }
}

int JvmtiThreadState::cur_stack_depth() { 
  uint32_t debug_bits = 0;
  guarantee(JavaThread::current() == get_thread() ||
    JvmtiEnv::is_thread_fully_suspended(get_thread(), false, &debug_bits),
    "must be current thread or suspended");

  if (!is_interp_only_mode() || _cur_stack_depth == UNKNOWN_STACK_DEPTH) {
    _cur_stack_depth = count_frames();
  } else {
    // heavy weight assert
    assert(_cur_stack_depth == count_frames(),
           "cur_stack_depth out of sync");
  }
  return _cur_stack_depth;
}

bool JvmtiThreadState::may_be_walked() {
  return (get_thread()->is_being_ext_suspended() || (JavaThread::current() == get_thread()));
}


void JvmtiThreadState::process_pending_step_for_popframe() {
  // We are single stepping as the last part of the PopFrame() dance
  // so we have some house keeping to do.

  JavaThread *thr = get_thread();
  if (thr->popframe_condition() != JavaThread::popframe_inactive) {
    // If the popframe_condition field is not popframe_inactive, then
    // we missed all of the popframe_field cleanup points:
    //
    // - unpack_frames() was not called (nothing to deopt)
    // - remove_activation_preserving_args_entry() was not called
    //   (did not get suspended in a call_vm() family call and did
    //   not complete a call_vm() family call on the way here)
    //
    // One legitimate way for us to miss all the cleanup points is
    // if we got here right after handling a compiled return. If that
    // is the case, then we consider our return from compiled code
    // to complete the PopFrame() request and we clear the condition.
    if (!thr->was_popframe_compiled_return()) {
      guarantee(false, "unexpected popframe_condition value");
    }
    thr->clear_popframe_condition();
  }

  // clearing the flag indicates we are done with the PopFrame() dance
  clear_pending_step_for_popframe();

  // If step is pending for popframe then it may not be
  // a repeat step. The new_bci and method_id is same as current_bci
  // and current method_id after pop and step for recursive calls.
  // Force the step by clearing the last location.
  int ec = env_count();
  for (int i = 0; i < ec ; ++i) {
    JvmtiEnvThreadState *ets = env_thread_state(i);
    ets->clear_current_location();
  }
}


// Class:     JvmtiThreadState
// Function:  update_for_pop_top_frame
// Description:
//   This function removes any frame pop notification request for
//   the top frame and invalidates both the current stack depth and
//   all cached frameIDs.
//
// Called by: PopFrame
//
void JvmtiThreadState::update_for_pop_top_frame() {
  if (is_interp_only_mode()) {
    // remove any frame pop notification request for the top frame
    // in any environment
    int popframe_number = cur_stack_depth();
    int ec = env_count();
    for (int i = 0; i < ec ; ++i) {
      JvmtiEnvThreadState *ets = env_thread_state(i);
      if (ets->is_frame_pop(popframe_number)) {
        ets->clear_frame_pop(popframe_number);
      }
    }
    // force stack depth to be recalculated
    invalidate_cur_stack_depth();
  } else {
    assert(!is_enabled(JVMTI_EVENT_FRAME_POP), "Must have no framepops set");
  }

  // Invalidate all frame IDs
  clear_cached_frames();
}
