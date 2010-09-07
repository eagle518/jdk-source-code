/*
 * Copyright (c) 2000, 2005, Oracle and/or its affiliates. All rights reserved.
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

inline bool GC_locker::is_active() {
  return _lock_count > 0 || _jni_lock_count > 0;
}

inline bool GC_locker::check_active_before_gc() {
  if (is_active()) {
    set_needs_gc();
  }
  return is_active();
}

inline void GC_locker::lock() {
  // cast away volatile
  Atomic::inc(&_lock_count);
  CHECK_UNHANDLED_OOPS_ONLY(
    if (CheckUnhandledOops) { Thread::current()->_gc_locked_out_count++; })
  assert(Universe::heap() == NULL ||
         !Universe::heap()->is_gc_active(), "locking failed");
}

inline void GC_locker::unlock() {
  // cast away volatile
  Atomic::dec(&_lock_count);
  CHECK_UNHANDLED_OOPS_ONLY(
    if (CheckUnhandledOops) { Thread::current()->_gc_locked_out_count--; })
}

inline void GC_locker::lock_critical(JavaThread* thread) {
  if (!thread->in_critical()) {
    if (!needs_gc()) {
      jni_lock();
    } else {
      jni_lock_slow();
    }
  }
  thread->enter_critical();
}

inline void GC_locker::unlock_critical(JavaThread* thread) {
  thread->exit_critical();
  if (!thread->in_critical()) {
    if (!needs_gc()) {
      jni_unlock();
    } else {
      jni_unlock_slow();
    }
  }
}
