#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)gcLocker.cpp	1.38 03/12/23 16:41:09 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_gcLocker.cpp.incl"

volatile jint GC_locker::_jni_lock_count = 0;
volatile jint GC_locker::_lock_count     = 0;
volatile bool GC_locker::_needs_gc       = false;

// Implementation of No_GC_Verifier

#ifdef ASSERT

No_GC_Verifier::No_GC_Verifier(bool verifygc) {
  _verifygc = verifygc;
  if (_verifygc) {
    CollectedHeap* h = Universe::heap();
    assert(!h->is_gc_active(), "GC active during No_GC_Verifier");
    _old_invocations = h->total_collections();
  }
}


No_GC_Verifier::~No_GC_Verifier() {
  if (_verifygc) {
    CollectedHeap* h = Universe::heap();
    assert(!h->is_gc_active(), "GC active during No_GC_Verifier");
    if (_old_invocations != h->total_collections()) {
      fatal("collection in a No_GC_Verifier secured function");
    }
  }
}


// JRT_LEAF rules: 
// A JRT_LEAF method may not interfere with safepointing by
//   1) acquiring or blocking on a Mutex or JavaLock - checked
//   2) allocating heap memory - checked
//   3) executing a VM operation - checked
//   4) executing a system call (including malloc) that could block or grab a lock
//   5) invoking GC
//   6) reaching a safepoint
//   7) running too long
// Nor may any method it calls.
JRT_Leaf_Verifier::JRT_Leaf_Verifier()
  : No_Safepoint_Verifier(true, JRT_Leaf_Verifier::should_verify_GC())
{
}

JRT_Leaf_Verifier::~JRT_Leaf_Verifier()
{
}

bool JRT_Leaf_Verifier::should_verify_GC() {
  switch (JavaThread::current()->thread_state()) {
  case _thread_in_Java:
    // is in a leaf routine, there must be no safepoint.
    return true;
  case _thread_in_native:
    // A native thread is not subject to safepoints.
    // Even while it is in a leaf routine, GC is ok
    return false;
  default:
    // Leaf routines cannot be called from other contexts.
    ShouldNotReachHere();
    return false;
  }
}
#endif
