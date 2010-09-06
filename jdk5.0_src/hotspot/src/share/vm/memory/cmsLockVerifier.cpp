#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)cmsLockVerifier.cpp	1.3 03/12/23 16:40:57 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_cmsLockVerifier.cpp.incl"

///////////// Locking verification specific to CMS //////////////
// Much like "assert_lock_strong()", except that it relaxes the
// assertion somewhat for the parallel GC case, where VM thread
// or the CMS thread might hold the lock on behalf of the parallel
// threads. The second argument is in support of an extra locking
// check for CFL spaces' free list locks.
#ifndef PRODUCT
void CMSLockVerifier::assert_locked(const Mutex* lock, const Mutex* p_lock) {
  if (ParallelGCThreads == 0) {
    assert_lock_strong(lock);
  } else {
    Thread* myThread = Thread::current();
    if (myThread->is_VM_thread()
        || myThread->is_ConcurrentMarkSweep_thread()
        || myThread->is_Java_thread()) {
      // Make sure that we are holding the free list lock.
      assert_lock_strong(lock);
      // The checking of p_lock is a spl case for CFLS' free list
      // locks: we make sure that none of the parallel GC work gang
      // threads are holding "sub-locks" of freeListLock(). We check only
      // the parDictionaryAllocLock because the others are too numerous.
      // This spl case code is somewhat ugly and any improvements
      // are welcome XXX FIX ME!!
      if (p_lock != NULL) {
        assert(!p_lock->is_locked() || p_lock->owned_by_self(),
               "Possible race between this and parallel GC threads");
      }
    } else if (myThread->is_GC_task_thread()) {
      // Make sure that the VM or CMS thread holds lock on our behalf
      // XXX If there were a concept of a gang_master for a (set of)
      // gang_workers, we could have used the identity of that thread
      // for checking ownership here; for now we just disjunct.
      assert(lock->owner() == VMThread::vm_thread() ||
             lock->owner() == ConcurrentMarkSweepThread::first_thread(),
             "Should be locked by VM thread or CMS thread on my behalf");
    } else {
      // Make sure we didn't miss some obscure corner case
      ShouldNotReachHere();
    }
  }
}
#endif

