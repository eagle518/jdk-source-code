#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)psPromotionManager.inline.hpp	1.6 03/12/23 16:40:17 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

inline PSPromotionManager* PSPromotionManager::manager_array(int index) {
  assert(_manager_array != NULL, "access of NULL manager_array");
  assert(index >= 0 && index <= (int)ParallelGCThreads, "out of range manager_array access");
  return _manager_array[index];
}

inline void PSPromotionManager::claim_or_forward_internal(oop* p) {
  if (p != NULL) {
    oop o = *p;
    if (o->is_forwarded()) {
      o = o->forwardee();
    } else {
      o = copy_to_survivor_space(o);
    }

    // Card mark
    if (PSScavenge::is_obj_in_young((HeapWord*) o)) {
      PSScavenge::card_table()->inline_write_ref_field_gc(p, o);
    }
    *p = o;
  }
}

inline void PSPromotionManager::flush_prefetch_queue() {
  for (int i=0; i<_prefetch_queue.length(); i++) {
    claim_or_forward_internal(_prefetch_queue.pop());
  }
}

inline void PSPromotionManager::claim_or_forward(oop* p) {
  assert(PSScavenge::should_scavenge(*p), "Sanity");
  assert(Universe::heap()->kind() == CollectedHeap::ParallelScavengeHeap, "Sanity");
  assert(!((ParallelScavengeHeap*)Universe::heap())->young_gen()->to_space()->contains(*p),
         "Attempt to rescan object");
  assert(Universe::heap()->is_in(p), "Attempt to claim_or_forward pointer outside heap");

  claim_or_forward_internal(_prefetch_queue.push_and_pop(p));
}


