#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)defNewGeneration.inline.hpp	1.10 03/12/23 16:41:05 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

CompactibleSpace* DefNewGeneration::first_compaction_space() const {
  return eden();
}

HeapWord* DefNewGeneration::allocate(size_t word_size,
                                     bool is_large_noref,
                                     bool is_tlab) {
  // This is the slow-path allocation for the DefNewGeneration.
  // Most allocations are fast-path in compiled code.
  // We try to allocate from the eden.  If that works, we are happy.
  // Note that since DefNewGeneration supports lock-free allocation, we
  // have to use it here, as well.
  HeapWord* result = eden()->par_allocate(word_size);
  if (result == NULL) {
    // Tell the next generation we reached a limit.
    HeapWord* new_limit = 
      next_gen()->allocation_limit_reached(eden(), eden()->top(), word_size);
    if (new_limit != NULL) {
      eden()->set_soft_end(new_limit);
      result = eden()->par_allocate(word_size);
    } else {
      assert(eden()->soft_end() == eden()->end(),
	     "invalid state after allocation_limit_reached returned null")
    }

    // If the eden is full and the last collection bailed out, we are running
    // out of heap space, and we try to allocate the from-space, too.
    // allocate_from_space can't be inlined because that would introduce a
    // circular dependency at compile time.
    if (result == NULL) {
      result = allocate_from_space(word_size);
    }
  }
  return result;
}

HeapWord* DefNewGeneration::par_allocate(size_t word_size,
					 bool is_large_noref,
					 bool is_tlab) {
  return eden()->par_allocate(word_size);
}

void DefNewGeneration::gc_prologue(bool full) {
  // Ensure that _end and _soft_end are the same in eden space.
  eden()->set_soft_end(eden()->end());
}

size_t DefNewGeneration::tlab_capacity() const {
  return eden()->capacity();
}

size_t DefNewGeneration::unsafe_max_tlab_alloc() const {
  return unsafe_max_alloc_nogc();
}
