#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)mutableSpace.cpp	1.11 03/12/23 16:40:29 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_mutableSpace.cpp.incl"

void MutableSpace::initialize(MemRegion mr, bool clear_space) {
  HeapWord* bottom = mr.start();
  HeapWord* end    = mr.end();

  assert(Universe::on_page_boundary(bottom) && Universe::on_page_boundary(end),
         "invalid space boundaries");
  set_bottom(bottom);
  set_end(end);

  if (clear_space) clear();
}

void MutableSpace::clear() {
  set_top(bottom());
  if (ZapUnusedHeapArea) mangle_unused_area();
}

// This version requires locking. */
HeapWord* MutableSpace::allocate(size_t size) {
  assert(Heap_lock->owned_by_self() ||
         (SafepointSynchronize::is_at_safepoint() &&
          Thread::current()->is_VM_thread()),
         "not locked");
  HeapWord* obj = top();
  HeapWord* new_top = obj + size;
  // The 'new_top>obj' check is needed to detect overflow of obj+size.
  if (new_top > obj && new_top <= end()) {
    set_top(new_top);
    assert(is_object_aligned((intptr_t)obj) && is_object_aligned((intptr_t)new_top),
	   "checking alignment");
    return obj;
  } else {
    return NULL;
  }
}

// This version is lock-free.
HeapWord* MutableSpace::cas_allocate(size_t size) {
  do {
    HeapWord* obj = top();
    HeapWord* new_top = obj + size;
    // The 'new_top>obj' check is needed to detect overflow of obj+size.
    if ((new_top > obj) && (new_top <= end())) {
      HeapWord* result = (HeapWord*)Atomic::cmpxchg_ptr(new_top, top_addr(), obj);
      // result can be one of two:
      //  the old top value: the exchange succeeded
      //  otherwise: the new value of the top is returned.
      if (result != obj) {          
	continue; // another thread beat us to the allocation, try again
      }
      assert(is_object_aligned((intptr_t)obj) && is_object_aligned((intptr_t)new_top),
             "checking alignment");
      return obj;
    } else {
      return NULL;
    }
  } while (true);
}

void MutableSpace::oop_iterate(OopClosure* cl) {
  HeapWord* obj_addr = bottom();
  HeapWord* t = top();
  // Could call objects iterate, but this is easier.
  while (obj_addr < t) {
    obj_addr += oop(obj_addr)->oop_iterate(cl);
  }
}

void MutableSpace::object_iterate(ObjectClosure* cl) {
  HeapWord* p = bottom();
  while (p < top()) {
    cl->do_object(oop(p));
    p += oop(p)->size();
  }
}

void MutableSpace::print_short() const { print_short_on(tty); }
void MutableSpace::print_short_on( outputStream* st) const {
  st->print(" space " SIZE_FORMAT "K, %d%% used", capacity_in_bytes() / K, 
            (int) ((double) used_in_bytes() * 100 / capacity_in_bytes()));
}

void MutableSpace::print() const { print_on(tty); }
void MutableSpace::print_on(outputStream* st) const {
  print_short_on(st);
  st->print_cr(" [" INTPTR_FORMAT "," INTPTR_FORMAT "," INTPTR_FORMAT ")", 
                 bottom(), top(), end());
}

#ifndef PRODUCT

void MutableSpace::verify(bool allow_dirty) const {
  HeapWord* p = bottom();
  HeapWord* t = top();
  HeapWord* prev_p = NULL;
  while (p < t) {
    oop(p)->verify();
    prev_p = p;
    p += oop(p)->size();
  }
  guarantee(p == top(), "end of last object must match end of space");
}

#endif
