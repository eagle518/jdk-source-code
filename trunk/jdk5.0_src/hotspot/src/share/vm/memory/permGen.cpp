#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)permGen.cpp	1.37 04/02/05 12:43:55 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_permGen.cpp.incl"

CompactingPermGen::CompactingPermGen(ReservedSpace rs,
                                     ReservedSpace shared_rs,
				     size_t initial_byte_size,
				     GenRemSet* remset,
                                     PermanentGenerationSpec* perm_spec)
{
  CompactingPermGenGen* g =
    new CompactingPermGenGen(rs, shared_rs, initial_byte_size, -1, remset,
                             NULL, perm_spec);
  if (g == NULL)
    vm_exit_during_initialization("Could not allocate a CompactingPermGen");
  _gen = g;

  g->initialize_performance_counters();

  _capacity_expansion_limit = g->capacity() + MaxPermHeapExpansion;
}

HeapWord* CompactingPermGen::mem_allocate(size_t size) {
  MutexLocker ml(Heap_lock);
  HeapWord* obj = _gen->allocate(size, false, false);
  bool tried_collection = false;
  bool tried_expansion = false;
  while (obj == NULL) {
    if (_gen->capacity() >= _capacity_expansion_limit || tried_expansion) {
      // Expansion limit reached, try collection before expanding further
      // For now we force a full collection, this could be changed
      SharedHeap::heap()->collect_locked(GCCause::_permanent_generation_full);
      obj = _gen->allocate(size, false, false);
      tried_collection = true;
      tried_expansion =  false;    // since collection; allows eventual
                                   // expansion to max_capacity
    }
    if (obj == NULL && !tried_expansion) {
      obj = _gen->expand_and_allocate(size, false, false);
      tried_expansion = true;
    }
    if (obj == NULL && tried_collection && tried_expansion) {
      assert(_gen->capacity() <= _gen->max_capacity(), "Invariant");
      if (_gen->capacity() == _gen->max_capacity()) {
        // We have reached our maximum size but not been able to
        // allocate; we now make a last-ditch collection attempt that
        // will try to reclaim as much space as possible; if even
        // that does not succeed in freeing space to accomodate
        // the allocation then we are truly out of space. 
        SharedHeap::heap()->collect_locked(GCCause::_last_ditch_collection);
        obj = _gen->allocate(size, false, false);
        break;
      }
      // Else we have not yet reached the max size, go around the
      // loop once more.
    }
  }
  return obj;
}

void CompactingPermGen::compute_new_size() {
  size_t desired_capacity = align_size_up(_gen->used(), MinPermHeapExpansion);
  if (desired_capacity < PermSize) {
    desired_capacity = PermSize;
  }
  if (_gen->capacity() > desired_capacity) {
    _gen->shrink(_gen->capacity() - desired_capacity);
  }
  _capacity_expansion_limit = _gen->capacity() + MaxPermHeapExpansion;
}

CMSPermGen::CMSPermGen(ReservedSpace rs, size_t initial_byte_size,
             CardTableRS* ct,
             FreeBlockDictionary::DictionaryChoice dictionaryChoice) {
  CMSPermGenGen* g =
    new CMSPermGenGen(rs, initial_byte_size, -1, ct);
  if (g == NULL) {
    vm_exit_during_initialization("Could not allocate a CompactingPermGen");
  }

  g->initialize_performance_counters();

  _gen = g;
  _capacity_expansion_limit = g->capacity() + MaxPermHeapExpansion;
}

// This code is slightly complicated by the need for taking care
// of two cases: we may be calling here sometimes while holding the
// underlying cms space's free list lock and sometimes without.
// The solution is an efficient recursive lock for the  free list,
// but here we use a naive and inefficient solution, that also,
// unfortunately, exposes the implementation details. FIX ME!!!
HeapWord* CMSPermGen::check_lock_and_allocate(bool lock_owned, size_t size) {
  if (lock_owned) {
    return _gen->have_lock_and_allocate(size, false, false);
  }
  return _gen->allocate(size, false, false);
}

void CMSPermGen::check_lock_and_collect(bool lock_owned,
                                        GCCause::Cause cause) {
  if (lock_owned) { 
    SharedHeap::heap()->collect_locked(cause);
  } else {
    SharedHeap::heap()->collect(cause);
  }
}

HeapWord* CMSPermGen::mem_allocate(size_t size) {
  Mutex* lock = _gen->freelistLock();
  HeapWord* obj = NULL;
  bool tried_collection = false;
  bool tried_expansion = false;
  bool lock_owned = lock->owned_by_self();

  obj = check_lock_and_allocate(lock_owned, size);
  while (obj == NULL) {
    if (_gen->capacity() >= _capacity_expansion_limit || tried_expansion) {
      // Expansion limit reached, try collection before expanding further
      // For now we force a full collection, this could be changed
      check_lock_and_collect(lock_owned, GCCause::_permanent_generation_full);
      obj = check_lock_and_allocate(lock_owned, size);
      tried_collection = true;
      tried_expansion  = false;  // ... since collection; allows eventual
                                 // expansion to max_capacity.
    }
    if (obj == NULL && !tried_expansion) {
      obj = _gen->expand_and_allocate(size, false, false);
      tried_expansion = true;
    }
    if (obj == NULL && tried_collection && tried_expansion) {
      if (_gen->capacity() == _gen->max_capacity()) {
        // We have reached our maximum size but not been able to
        // allocate; we now make a last-ditch collection attempt that
        // will try to reclaim as much space as possible; if even
        // that does not succeed in freeing space to accomodate
        // the allocation then we are truly out of space. 
        check_lock_and_collect(lock_owned, GCCause::_last_ditch_collection);
        obj = check_lock_and_allocate(lock_owned, size);
        break;
      }
      // Else we have not yet reached the max size, go around the
      // loop once more.
    }
  }
  return obj;
}

void CMSPermGen::compute_new_size() {
  _gen->compute_new_size();
}

void CMSPermGenGen::initialize_performance_counters() {

  const char* gen_name = "perm";

  // Generation Counters - generation 2, 1 subspace
  _gen_counters = new GenerationCounters(gen_name, 2, 1, &_virtual_space);

  _gc_counters = NULL;

  _space_counters = new GSpaceCounters(gen_name, 0,
                                       _virtual_space.reserved_size(),
                                       this, _gen_counters);
}
