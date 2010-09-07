/*
 * Copyright (c) 2001, 2009, Oracle and/or its affiliates. All rights reserved.
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

// Inline functions for G1CollectedHeap

inline HeapRegion*
G1CollectedHeap::heap_region_containing(const void* addr) const {
  HeapRegion* hr = _hrs->addr_to_region(addr);
  // hr can be null if addr in perm_gen
  if (hr != NULL && hr->continuesHumongous()) {
    hr = hr->humongous_start_region();
  }
  return hr;
}

inline HeapRegion*
G1CollectedHeap::heap_region_containing_raw(const void* addr) const {
  assert(_g1_reserved.contains(addr), "invariant");
  size_t index = pointer_delta(addr, _g1_reserved.start(), 1)
                                        >> HeapRegion::LogOfHRGrainBytes;

  HeapRegion* res = _hrs->at(index);
  assert(res == _hrs->addr_to_region(addr), "sanity");
  return res;
}

inline bool G1CollectedHeap::obj_in_cs(oop obj) {
  HeapRegion* r = _hrs->addr_to_region(obj);
  return r != NULL && r->in_collection_set();
}

inline HeapWord* G1CollectedHeap::attempt_allocation(size_t word_size,
                                              bool permit_collection_pause) {
  HeapWord* res = NULL;

  assert( SafepointSynchronize::is_at_safepoint() ||
          Heap_lock->owned_by_self(), "pre-condition of the call" );

  if (_cur_alloc_region != NULL) {

    // If this allocation causes a region to become non empty,
    // then we need to update our free_regions count.

    if (_cur_alloc_region->is_empty()) {
      res = _cur_alloc_region->allocate(word_size);
      if (res != NULL)
        _free_regions--;
    } else {
      res = _cur_alloc_region->allocate(word_size);
    }
  }
  if (res != NULL) {
    if (!SafepointSynchronize::is_at_safepoint()) {
      assert( Heap_lock->owned_by_self(), "invariant" );
      Heap_lock->unlock();
    }
    return res;
  }
  // attempt_allocation_slow will also unlock the heap lock when appropriate.
  return attempt_allocation_slow(word_size, permit_collection_pause);
}

inline RefToScanQueue* G1CollectedHeap::task_queue(int i) {
  return _task_queues->queue(i);
}


inline  bool G1CollectedHeap::isMarkedPrev(oop obj) const {
  return _cm->prevMarkBitMap()->isMarked((HeapWord *)obj);
}

inline bool G1CollectedHeap::isMarkedNext(oop obj) const {
  return _cm->nextMarkBitMap()->isMarked((HeapWord *)obj);
}
