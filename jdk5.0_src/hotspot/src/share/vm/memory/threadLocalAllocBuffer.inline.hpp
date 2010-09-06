#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)threadLocalAllocBuffer.inline.hpp	1.20 03/12/23 16:41:35 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

inline HeapWord* ThreadLocalAllocBuffer::allocate(size_t size) {
  invariants();
  HeapWord* obj = top();
  if (pointer_delta(end(), obj) >= size) {
    // successful thread-local allocation
    if (!ZeroTLAB) {
      // need to clear individual objects
      Copy::fill_to_aligned_words(obj, size);
    }
    // This addition is safe because we know that top is
    // at least size below end, so the add can't wrap.
    set_top(obj + size);
    
    invariants();
    return obj;
  }
  return NULL;
}

inline size_t ThreadLocalAllocBuffer::compute_size(size_t obj_size) {
  const size_t aligned_obj_size = align_object_size(obj_size);

  // Compute the size for the new TLAB.
  // The "last" tlab may be smaller to reduce fragmentation.
  // unsafe_max_tlab_alloc is just a hint.
  const size_t available_size = Universe::heap()->unsafe_max_tlab_alloc() /
                                                  HeapWordSize;
  size_t new_tlab_size = MIN2(available_size, size() + aligned_obj_size);

  // Make sure there's enough room for object and filler int[].
  const size_t obj_plus_filler_size = aligned_obj_size + alignment_reserve();
  if (new_tlab_size < obj_plus_filler_size) {
    // If there isn't enough room for the allocation, return failure.
    if (PrintTLAB && Verbose) {
      gclog_or_tty->print_cr("ThreadLocalAllocBuffer::compute_size(" SIZE_FORMAT ")"
                    " returns failure",
                    obj_size);
    }
    return 0;
  }
  if (PrintTLAB && Verbose) {
    gclog_or_tty->print_cr("ThreadLocalAllocBuffer::compute_size(" SIZE_FORMAT ")"
                  " returns " SIZE_FORMAT,
                  obj_size, new_tlab_size);
  }
  return new_tlab_size;
}


void ThreadLocalAllocBuffer::record_slow_allocation(size_t obj_size) {
  // Raise size required to bypass TLAB next time. Why? Else there's
  // a risk that a thread that repeatedly allocates objects of one
  // size will get stuck on this slow path.

  set_refill_waste_limit(refill_waste_limit() + refill_waste_limit_increment());

  _slow_allocations++;

  if (PrintTLAB && Verbose) {
    Thread* thrd = myThread();
    gclog_or_tty->print("TLAB: %s thread: "INTPTR_FORMAT" [id: %2d]"
                        " obj: "SIZE_FORMAT
                        " free: "SIZE_FORMAT
                        " waste: "SIZE_FORMAT"\n",
                        "slow", thrd, thrd->osthread()->thread_id(),
                        obj_size, free(), refill_waste_limit());
  }
}
