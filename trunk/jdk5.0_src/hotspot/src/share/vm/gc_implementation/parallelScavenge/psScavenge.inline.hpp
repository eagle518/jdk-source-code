#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)psScavenge.inline.hpp	1.10 03/12/23 16:40:18 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */


inline bool PSScavenge::should_scavenge(oop p) {
  return p == NULL ? false : PSScavenge::is_obj_in_young((HeapWord*) p);
}

// Attempt to "claim" oop at p via CAS, push the new obj if successful
// This version tests the oop* to make sure it is within the heap before
// attempting marking.
inline void PSScavenge::copy_and_push_safe_barrier(PSPromotionManager* pm,
                                                   oop*                p) {
  assert(should_scavenge(*p), "Sanity");
  assert(Universe::heap()->kind() == CollectedHeap::ParallelScavengeHeap,
         "Sanity");
  assert(!((ParallelScavengeHeap*)Universe::heap())->
           young_gen()->to_space()->contains(*p),
         "Attempt to rescan object");

  oop o = *p;
  if (o->is_forwarded()) {
    *p = o->forwardee();
  } else {
    *p = pm->copy_to_survivor_space(o);
  }
  
  // We cannot mark without test, as some code passes us pointers 
  // that are outside the heap.
  if ((!PSScavenge::is_obj_in_young((HeapWord*) p)) &&
      Universe::heap()->is_in_reserved(p)) {
    o = *p;
    if (PSScavenge::is_obj_in_young((HeapWord*) o)) {
      card_table()->inline_write_ref_field_gc(p, o);
    }
  }
}
