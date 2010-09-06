#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)collectedHeap.cpp	1.13 04/03/28 16:22:25 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_collectedHeap.cpp.incl"

// Memory state functions.

CollectedHeap::CollectedHeap() :
  _reserved(), _barrier_set(NULL), _is_gc_active(false),
  _total_collections(0), _max_heap_capacity(0),
  _gc_cause(GCCause::_no_gc), _gc_lastcause(GCCause::_no_gc) {

  if (UsePerfData) {
    EXCEPTION_MARK;

    // create the gc cause jvmstat counters
    _perf_gc_cause = PerfDataManager::create_string_variable(SUN_GC, "cause",
                             80, GCCause::to_string(_gc_cause), CHECK);

    _perf_gc_lastcause =
                PerfDataManager::create_string_variable(SUN_GC, "lastCause",
                             80, GCCause::to_string(_gc_lastcause), CHECK);
  }
}


#ifndef PRODUCT
void CollectedHeap::check_for_bad_heap_word_value(HeapWord* addr, size_t size) {
  if (CheckMemoryInitialization && ZapUnusedHeapArea) {
    for (size_t slot = 0; slot < size; slot += 1) {
      assert((*(intptr_t*) (addr + slot)) != ((intptr_t) badHeapWordVal),
             "Found badHeapWordValue in post-allocation check");
    }
  }
}

void CollectedHeap::check_for_non_bad_heap_word_value(HeapWord* addr, size_t size)
 {
  if (CheckMemoryInitialization && ZapUnusedHeapArea) {
    for (size_t slot = 0; slot < size; slot += 1) {
      assert((*(intptr_t*) (addr + slot)) == ((intptr_t) badHeapWordVal),
             "Found non badHeapWordValue in pre-allocation check");
    }
  }
}
#endif // PRODUCT

#ifdef ASSERT
void CollectedHeap::check_for_valid_allocation_state() {
  Thread *thread = Thread::current();
  // How to choose between a pending exception and a potential
  // OutOfMemoryError?  Don't allow pending exceptions.
  // This is a VM policy failure, so how do we exhaustively test it?
  assert(!thread->has_pending_exception(),
         "shouldn't be allocating with pending exception");
  if (StrictSafepointChecks) {
    assert(thread->allow_allocation(),
           "Allocation done by thread for which allocation is blocked "
           "by No_Allocation_Verifier!");
    // Allocation of an oop can always invoke a safepoint,
    // hence, the true argument
    thread->check_for_valid_safepoint_state(true);
  }
}
#endif

HeapWord* CollectedHeap::allocate_from_tlab_slow(Thread* thread, size_t size) {

  // Retain tlab and allocate object in shared space if
  // the amount free in the tlab is too large to discard.
  if (thread->tlab().free() > thread->tlab().refill_waste_limit()) {
    thread->tlab().record_slow_allocation(size);
    return NULL;
  }

  // Discard tlab and allocate a new one.
  // To minimize fragmentation, the last TLAB may be smaller than the rest.
  size_t new_tlab_size = thread->tlab().compute_size(size);

  thread->tlab().clear_before_allocation();

  if (new_tlab_size == 0) {
    return NULL;
  }

  // Allocate a new TLAB...
  HeapWord* obj = Universe::heap()->allocate_new_tlab(new_tlab_size);
  if (obj == NULL) {
    return NULL;
  }
  if (ZeroTLAB) {		
    // ..and clear it.
    Copy::zero_to_words(obj, new_tlab_size);
  } else {			
    // ...and clear just the allocated object.
    Copy::zero_to_words(obj, size);
  }
  thread->tlab().fill(obj, obj + size, new_tlab_size);
  return obj;
}

HeapWord* CollectedHeap::allocate_new_tlab(size_t size) {
  guarantee(false, "thread-local allocation buffers not supported");
  return NULL;
}

void CollectedHeap::fill_all_tlabs() {
  assert(UseTLAB, "should not reach here");
  // See note in ensure_parseability() below.
  assert(SafepointSynchronize::is_at_safepoint() ||
         !is_init_completed(),
	 "should only fill tlabs at safepoint");
  // The main thread starts allocating via a TLAB even before it
  // has added itself to the threads list at vm boot-up.
  assert(Threads::first() != NULL,
         "Attempt to fill tlabs before main thread has been added"
         " to threads list is doomed to failure!");
  for(JavaThread *thread = Threads::first(); thread; thread = thread->next()) {
     thread->tlab().clear();
  }
}

void CollectedHeap::ensure_parseability() {
  // The second disjunct in the assertion below makes a concession
  // for the start-up verification done while the VM is being
  // created. Callers be careful that you know that mutators
  // aren't going to interfere -- for instance, this is permissible
  // if we are still single-threaded and have either not yet
  // started allocating (nothing much to verify) or we have
  // started allocating but are now a full-fledged JavaThread
  // (and have thus made our TLAB's) available for filling.
  assert(SafepointSynchronize::is_at_safepoint() ||
         !is_init_completed(), 
         "Should only be called at a safepoint or at start-up"
         " otherwise concurrent mutator activity may make heap "
         " unparseable again");
  if (UseTLAB) {
    fill_all_tlabs();
  }
}

void CollectedHeap::accumulate_statistics_all_tlabs() {
  if (UseTLAB) {
    assert(SafepointSynchronize::is_at_safepoint() ||
         !is_init_completed(),
	 "should only accumulate statistics on tlabs at safepoint");

    ThreadLocalAllocBuffer::accumulate_statistics_before_gc();
  }
}

void CollectedHeap::resize_all_tlabs() {
  if (UseTLAB) {
    assert(SafepointSynchronize::is_at_safepoint() ||
         !is_init_completed(),
	 "should only resize tlabs at safepoint");

    ThreadLocalAllocBuffer::resize_all_tlabs();
  }
}
