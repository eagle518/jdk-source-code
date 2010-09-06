#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)gcLocker.inline.hpp	1.13 03/12/23 16:41:09 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

inline void GC_locker::lock() {
  // cast away volatile
  Atomic::inc(&_lock_count);
  assert(Universe::heap() == NULL ||
	 !Universe::heap()->is_gc_active(), "locking failed");
}

inline void GC_locker::unlock_critical(JavaThread* thread) {
  MutexLocker mu(JNICritical_lock);
  thread->exit_critical();
  jni_unlock();
  if (needs_gc() && !is_jni_active()) {
    // We're the last thread out. Cause a GC to occur.
    assert(!thread->in_critical(),
           "thread in critical yet GC_locker not active?");
    // GC will also check is_active, so this check is not
    // strictly needed. It's added here to make it clear that
    // the GC will NOT be performed if JVMPI (or any other caller
    // of GC_locker::lock()) still needs GC locked.
    if (!is_active()) {
      // Must give up the lock while at a safepoint
      MutexUnlocker munlock(JNICritical_lock);
      if (Universe::heap()->kind() == CollectedHeap::GenCollectedHeap) {
        GenCollectedHeap::heap()->collect(GCCause::_tenured_generation_full,
	  0);
      } else {
        Universe::heap()->collect(GCCause::_tenured_generation_full);
      }
    }
    clear_needs_gc();
    JNICritical_lock->notify_all();
  }
}

