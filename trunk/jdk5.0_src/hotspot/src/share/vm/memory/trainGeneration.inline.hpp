#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)trainGeneration.inline.hpp	1.7 03/12/23 16:41:37 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

void TrainGeneration::weak_ref_barrier_check(oop* p) {
  if (is_in_reserved(p) && is_in_reserved(*p)) {
    // Update train remembered set if reference is in this generation.
    // This check is required whether the object moved or not; for
    // example, we might just be scanning followers.
    oop_update_remembered_set(p);
  } else {
    // During weak reference discovery, sometimes p is not in the
    // heap: it might be one of the lists.
    if (Universe::heap()->is_in_reserved(p)) {
      // A later generation might want to examine this reference.
      _ct->ct_bs()->inline_write_ref_field(p, *p);
    }
  }
}

// this method assumes that the size is measured in heap words
void TrainGeneration::inc_used_counter(size_t size) {

  // detect missed counter updates, concurrent updates, and
  // unaccounted fragmentation differences in debug mode.
  assert(!UsePerfData ||
         (_space_counters->used() == (used() - (size * HeapWordSize))),
         "counter value and reality should be consistent");

  if (UsePerfData) _space_counters->inc_used(size * HeapWordSize);

  // detect missed counter updates, concurrent updates, and
  // unaccounted fragmentation differences in debug mode.
  assert(!UsePerfData || (_space_counters->used() == used()),
         "counter value and reality should be consistent");
}
