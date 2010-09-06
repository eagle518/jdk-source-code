#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)freeBlockDictionary.cpp	1.4 03/12/23 16:41:07 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_freeBlockDictionary.cpp.incl"

#ifndef PRODUCT
Mutex* FreeBlockDictionary::par_lock() const {
  return _lock;
}

void FreeBlockDictionary::set_par_lock(Mutex* lock) {
  _lock = lock;
}

void FreeBlockDictionary::verify_par_locked() const {
#ifdef ASSERT
  if (ParallelGCThreads > 0) {
    Thread* myThread = Thread::current();
    if (myThread->is_GC_task_thread()) {
      assert(par_lock() != NULL, "Should be using locking?");
      assert_lock_strong(par_lock());
    }
  }
#endif ASSERT
}
#endif
