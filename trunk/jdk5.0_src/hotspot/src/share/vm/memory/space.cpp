#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)space.cpp	1.185 03/12/23 16:41:29 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_space.cpp.incl"

HeapWord* DirtyCardToOopClosure::get_actual_top(HeapWord* top,
						HeapWord* top_obj) {
  if (top_obj != NULL) {
    if (_sp->block_is_obj(top_obj)) {
      if (_precision == CardTableModRefBS::ObjHeadPreciseArray) {
	if (oop(top_obj)->is_objArray() || oop(top_obj)->is_typeArray()) {
	  // An arrayOop is starting on the dirty card - since we do exact
	  // store checks for objArrays we are done.
	} else {
	  // Otherwise, it is possible that the object starting on the dirty
	  // card spans the entire card, and that the store happened on a
	  // later card.  Figure out where the object ends.
          // Use the block_size() method of the space over which 
          // the iteration is being done.  That space (e.g. CMS) may have 
          // specific requirements on object sizes which will 
          // be reflected in the block_size() method. 
	  top = top_obj + oop(top_obj)->size();
	}
      }
    } else {
      top = top_obj;
    }
  } else {
    assert(top == _sp->end(), "only case where top_obj == NULL");
  }
  return top;
}

void DirtyCardToOopClosure::walk_mem_region(MemRegion mr,
					    HeapWord* bottom,
					    HeapWord* top) {
  // 1. Blocks may or may not be objects.
  // 2. Even when a block_is_obj(), it may not entirely
  //    occupy the block if the block quantum is larger than
  //    the object size.
  // We can and should try to optimize by calling the non-MemRegion
  // version of oop_iterate() for all but the extremal objects
  // (for which we need to call the MemRegion version of
  // oop_iterate()) To be done post-beta XXX
  for (; bottom < top; bottom += _sp->block_size(bottom)) {
    // As in the case of contiguous space above, we'd like to
    // just use the value returned by oop_iterate to increment the
    // current pointer; unfortunately, that won't work in CMS because
    // we'd need an interface change (it seems) to have the space
    // "adjust the object size" (for instance pad it up to its
    // block alignment or minimum block size restrictions. XXX
    if (_sp->block_is_obj(bottom) &&
	!_sp->obj_allocated_since_save_marks(oop(bottom))) {
      oop(bottom)->oop_iterate(_cl, mr);
    }
  }
}

void DirtyCardToOopClosure::do_MemRegion(MemRegion mr) {

  // Some collectors need to do special things whenever their dirty
  // cards are processed. For instance, CMS must remember mutator updates
  // (i.e. dirty cards) so as to re-scan mutated objects; similarly,
  // the train uses dirty card information to update car-specific remembered sets.
  // Such work can be piggy-backed here on dirty card scanning, so as to make
  // it slightly more efficient than doing a complete non-detructive pre-scan
  // of the card table.
  MemRegionClosure* pCl = _sp->preconsumptionDirtyCardClosure();
  if (pCl != NULL) {
    pCl->do_MemRegion(mr);
  }

  HeapWord* bottom = mr.start();
  HeapWord* last = mr.last();
  HeapWord* top = mr.end();
  HeapWord* bottom_obj;
  HeapWord* top_obj;

  assert(_precision == CardTableModRefBS::ObjHeadPreciseArray ||
         _precision == CardTableModRefBS::Precise,
         "Only ones we deal with for now.");
  
  assert(_precision != CardTableModRefBS::ObjHeadPreciseArray ||
	 _last_bottom == NULL ||
	 top <= _last_bottom,
	 "Not decreasing");
  NOT_PRODUCT(_last_bottom = mr.start());
  
  bottom_obj = _sp->block_start(bottom);
  top_obj    = _sp->block_start(last);

  assert(bottom_obj <= bottom, "just checking");
  assert(top_obj    <= top,    "just checking");

  // Given what we think is the top of the memory region and
  // the start of the object at the top, get the actual
  // value of the top.
  top = get_actual_top(top, top_obj);
  
  // If the previous call did some part of this region, don't redo.
  if (_precision == CardTableModRefBS::ObjHeadPreciseArray &&
      _min_done != NULL &&
      _min_done < top) {
    top = _min_done;
  }
  
  // Top may have been reset, and in fact may be below bottom,
  // e.g. the dirty card region is entirely in a now free object
  // -- something that could happen with a concurrent sweeper.
  bottom = MIN2(bottom, top);
  mr     = MemRegion(bottom, top);
  assert(bottom <= top &&
	 (_precision != CardTableModRefBS::ObjHeadPreciseArray ||
	  _min_done == NULL ||
	  top <= _min_done),
	 "overlap!");

  // Walk the region if it is not empty; otherwise there is nothing to do.
  if (!mr.is_empty()) {
    walk_mem_region(mr, bottom_obj, top);
  }

  _min_done = bottom;
}

DirtyCardToOopClosure* Space::new_dcto_cl(OopClosure* cl,
					  CardTableModRefBS::PrecisionStyle precision,
					  HeapWord* boundary) {
  return new DirtyCardToOopClosure(this, cl, precision, boundary);
}

void FilteringClosure::do_oop(oop* p) {
  do_oop_nv(p);
}

HeapWord* ContiguousSpaceDCTOC::get_actual_top(HeapWord* top,
					       HeapWord* top_obj) {
  if (top_obj != NULL && top_obj < (_sp->toContiguousSpace())->top()) {
    if (_precision == CardTableModRefBS::ObjHeadPreciseArray) {
      if (oop(top_obj)->is_objArray() || oop(top_obj)->is_typeArray()) {
	// An arrayOop is starting on the dirty card - since we do exact
	// store checks for objArrays we are done.
      } else {
	// Otherwise, it is possible that the object starting on the dirty
	// card spans the entire card, and that the store happened on a
	// later card.  Figure out where the object ends.
	assert(_sp->block_size(top_obj) == (size_t) oop(top_obj)->size(), 
	  "Block size and object size mismatch");
	top = top_obj + oop(top_obj)->size();
      }
    }
  } else {
    top = (_sp->toContiguousSpace())->top();
  }
  return top;
}

void Filtering_DCTOC::walk_mem_region(MemRegion mr,
				      HeapWord* bottom,
				      HeapWord* top) {
  // Note that this assumption won't hold if we have a concurrent
  // collector in this space, which may have freed up objects after
  // they were dirtied and before the stop-the-world GC that is
  // examining cards here.
  assert(bottom < top, "ought to be at least one obj on a dirty card.");

  if (_boundary != NULL) {
    // We have a boundary outside of which we don't want to look
    // at objects, so create a filtering closure around the
    // oop closure before walking the region.
    FilteringClosure filter(_boundary, _cl);
    walk_mem_region_with_cl(mr, bottom, top, &filter);
  } else {
    // No boundary, simply walk the heap with the oop closure.
    walk_mem_region_with_cl(mr, bottom, top, _cl);
  }

}

// We must replicate this so that the static type of "FilteringClosure"
// (see above) is apparent at the oop_iterate calls.
#define ContiguousSpaceDCTOC__walk_mem_region_with_cl_DEFN(ClosureType)	\
void ContiguousSpaceDCTOC::walk_mem_region_with_cl(MemRegion mr,	\
						   HeapWord* bottom,	\
						   HeapWord* top,	\
						   ClosureType* cl) {	\
  bottom += oop(bottom)->oop_iterate(cl, mr);				\
  if (bottom < top) {							\
    HeapWord* next_obj = bottom + oop(bottom)->size();			\
    while (next_obj < top) {						\
      /* Bottom lies entirely below top, so we can call the */		\
      /* non-memRegion version of oop_iterate below. */			\
      oop(bottom)->oop_iterate(cl);					\
      bottom = next_obj;						\
      next_obj = bottom + oop(bottom)->size();				\
    }									\
    /* Last object. */							\
    oop(bottom)->oop_iterate(cl, mr);					\
  }									\
}

// (There are only two of these, rather than N, because the split is due
// only to the introduction of the FilteringClosure, a local part of the
// impl of this abstraction.)
ContiguousSpaceDCTOC__walk_mem_region_with_cl_DEFN(OopClosure)
ContiguousSpaceDCTOC__walk_mem_region_with_cl_DEFN(FilteringClosure)

DirtyCardToOopClosure*
ContiguousSpace::new_dcto_cl(OopClosure* cl,
			     CardTableModRefBS::PrecisionStyle precision,
			     HeapWord* boundary) {
  return new ContiguousSpaceDCTOC(this, cl, precision, boundary);
}

void Space::initialize(MemRegion mr, bool clear_space) {
  HeapWord* bottom = mr.start();
  HeapWord* end    = mr.end();
  assert(Universe::on_page_boundary(bottom) && Universe::on_page_boundary(end),
         "invalid space boundaries");
  set_bottom(bottom);
  set_end(end);
  if (clear_space) clear();
}

void Space::clear() {
  if (ZapUnusedHeapArea) mangle_unused_area();
}

void ContiguousSpace::clear() {
  set_top(bottom());
  set_saved_mark();
  Space::clear();
}

void OffsetTableContigSpace::clear() {
  ContiguousSpace::clear();
  _offsets.initialize_threshold();
}

void OffsetTableContigSpace::set_bottom(HeapWord* new_bottom) {
  Space::set_bottom(new_bottom);
  _offsets.set_bottom(new_bottom);
}

void OffsetTableContigSpace::set_end(HeapWord* new_end) {
  // Space should not advertize an increase in size
  // until after the underlying offest table has been enlarged.
  _offsets.resize(pointer_delta(new_end, bottom()));
  Space::set_end(new_end);
}

void ContiguousSpace::mangle_unused_area() {
  // to-space is used for storing marks during mark-sweep
  mangle_region(MemRegion(top(), end()));
}

void ContiguousSpace::mangle_region(MemRegion mr) {
  debug_only(Copy::fill_to_aligned_words(mr.start(), mr.word_size(), badHeapWord));
}

void CompactibleSpace::initialize(MemRegion mr, bool clear_space) {
  Space::initialize(mr, clear_space);
  _compaction_top = bottom();
  _next_compaction_space = NULL;
}

HeapWord* CompactibleSpace::forward(oop q, size_t size, 
				    CompactPoint* cp, HeapWord* compact_top) {
  // q is alive
  // First check if we should switch compaction space
  assert(this == cp->space, "'this' should be current compaction space.");
  size_t compaction_max_size = pointer_delta(end(), compact_top);
  while (size > compaction_max_size) {
    // switch to next compaction space
    cp->space->set_compaction_top(compact_top);
    cp->space = cp->space->next_compaction_space();
    if (cp->space == NULL) {
      cp->gen = GenCollectedHeap::heap()->prev_gen(cp->gen);
      assert(cp->gen != NULL, "compaction must succeed");
      cp->space = cp->gen->first_compaction_space();
      assert(cp->space != NULL, "generation must have a first compaction space");
    }
    compact_top = cp->space->bottom();
    cp->space->set_compaction_top(compact_top);
    cp->threshold = cp->space->initialize_threshold();
    compaction_max_size = pointer_delta(cp->space->end(), compact_top);
  }

  // store the forwarding pointer into the mark word
  if ((HeapWord*)q != compact_top) {
    q->forward_to(oop(compact_top));
    assert(q->is_gc_marked(), "encoding the pointer should preserve the mark");
  } else {
    // if the object isn't moving we can just set the mark to the default
    // mark and handle it specially later on.  
    q->init_mark();
    assert(q->forwardee() == NULL, "should be forwarded to NULL");
  }

  debug_only(MarkSweep::register_live_oop(q, size));
  compact_top += size;

  // we need to update the offset table so that the beginnings of objects can be
  // found during scavenge.  Note that we are updating the offset table based on
  // where the object will be once the compaction phase finishes.
  if (compact_top > cp->threshold)
    cp->threshold =
      cp->space->cross_threshold(compact_top - size, compact_top);
  return compact_top;
}


bool CompactibleSpace::insert_deadspace(size_t& allowed_deadspace_words,
					HeapWord* q, size_t deadlength) {
  if (allowed_deadspace_words >= deadlength) {
    allowed_deadspace_words -= deadlength;
    oop(q)->set_mark(markOopDesc::prototype()->set_marked());
    const size_t min_int_array_size = typeArrayOopDesc::header_size(T_INT);
    if (deadlength >= min_int_array_size) {
      oop(q)->set_klass(Universe::intArrayKlassObj());
      typeArrayOop(q)->set_length((int)((deadlength - min_int_array_size)
                                            * (HeapWordSize/sizeof(jint))));
    } else {
      assert((int) deadlength == instanceOopDesc::header_size(),
	     "size for smallest fake dead object doesn't match");
      oop(q)->set_klass(SystemDictionary::object_klass());
    }
    assert((int) deadlength == oop(q)->size(),
	   "make sure size for fake dead object match");
    // Recall that we required "q == compaction_top".
    return true;
  } else {
    allowed_deadspace_words = 0;
    return false;
  }
}

#define block_is_always_obj(q) true
#define obj_size(q) oop(q)->size()
#define adjust_obj_size(s) s

void CompactibleSpace::prepare_for_compaction(CompactPoint* cp) {
  SCAN_AND_FORWARD(cp, end, block_is_obj, block_size);
}

// Faster object search.
void ContiguousSpace::prepare_for_compaction(CompactPoint* cp) {
  SCAN_AND_FORWARD(cp, top, block_is_always_obj, obj_size);
}

void Space::adjust_pointers() {
  // adjust all the interior pointers to point at the new locations of objects
  // Used by MarkSweep::mark_sweep_phase3()

  // First check to see if there is any work to be done.
  if (used() == 0) {
    return;  // Nothing to do.
  }

  // Otherwise...
  HeapWord* q = bottom();
  HeapWord* t = end();

  debug_only(HeapWord* prev_q = NULL);
  while (q < t) {
    if (oop(q)->is_gc_marked()) {
      // q is alive
	
      debug_only(MarkSweep::track_interior_pointers(oop(q)));
      // point all the oops to the new location
      size_t size = oop(q)->adjust_pointers();
      debug_only(MarkSweep::check_interior_pointers());
      
      debug_only(prev_q = q);
      debug_only(MarkSweep::validate_live_oop(oop(q), size));
	
      q += size;
    } else {
      // q is not a live object.  But we're not in a compactible space,
      // So we don't have live ranges.
      debug_only(prev_q = q);
      q += block_size(q);
      assert(q > prev_q, "we should be moving forward through memory");
    }
  }
  assert(q == t, "just checking");
}

void CompactibleSpace::adjust_pointers() {
  // Check first is there is any work to do.
  if (used() == 0) {
    return;   // Nothing to do.
  }

  SCAN_AND_ADJUST_POINTERS(adjust_obj_size);
}

void CompactibleSpace::compact() {
  SCAN_AND_COMPACT(obj_size);
}

void Space::print_short() const { print_short_on(tty); }

void Space::print_short_on(outputStream* st) const {
  st->print(" space " SIZE_FORMAT "K, %3d%% used", capacity() / K, 
              (int) ((double) used() * 100 / capacity()));
}

void Space::print() const { print_on(tty); }

void Space::print_on(outputStream* st) const {
  print_short_on(st);
  st->print_cr(" [" INTPTR_FORMAT ", " INTPTR_FORMAT ")",
                bottom(), end());
}
  
void ContiguousSpace::print_on(outputStream* st) const {
  print_short_on(st);
  st->print_cr(" [" INTPTR_FORMAT ", " INTPTR_FORMAT ", " INTPTR_FORMAT ")",
                bottom(), top(), end());
}
  
void OffsetTableContigSpace::print_on(outputStream* st) const {
  print_short_on(st);
  st->print_cr(" [" INTPTR_FORMAT ", " INTPTR_FORMAT ", "
                INTPTR_FORMAT ", " INTPTR_FORMAT ")",
              bottom(), top(), _offsets.threshold(), end());
}

#ifndef PRODUCT

void ContiguousSpace::verify(bool allow_dirty) const {
  HeapWord* p = bottom();
  HeapWord* t = top();
  HeapWord* prev_p = NULL;
  while (p < t) {
    oop(p)->verify();
    prev_p = p;
    p += oop(p)->size();
  }
  guarantee(p == top(), "end of last object must match end of space");
  if (top() != end()) {
    guarantee(top() == block_start(end()-1) &&
              top() == block_start(top()), 
	      "top should be start of unallocated block, if it exists");
  }
}
#endif

void Space::oop_iterate(OopClosure* blk) {
  ObjectToOopClosure blk2(blk);
  object_iterate(&blk2);
}

void Space::object_iterate_mem(MemRegion mr, UpwardsObjectClosure* cl) {
  HeapWord* prev = cl->previous();   // max address from last time
  MemRegion cur(MAX2(prev, bottom()), end()); 
  mr = mr.intersection(cur);
  if (mr.is_empty()) {
    return;
  }
  if (mr.equals(MemRegion(bottom(),end()))) {
    object_iterate(cl);
    return;
  }
  HeapWord *addr, *end;
  size_t size;
  for (addr = (prev == mr.start() ? prev : block_start(mr.start())),
       end  = mr.end();
       addr < end; addr += size) {
    size = block_size(addr);
    if (block_is_obj(addr)) {
      cl->do_object(oop(addr));
    }
  }
  cl->set_previous(MAX2(addr, prev)); // min address for next time
}

#define EdenSpace_PAR_OOP_ITERATE_DEFN(OopClosureType, nv_suffix)           \
                                                                            \
  void EdenSpace::par_oop_iterate(MemRegion mr, OopClosureType* blk) {      \
    if (mr.is_empty()) return;                                              \
    HeapWord* obj_addr = mr.start();                                        \
    HeapWord* t = mr.end();                                                 \
    while (obj_addr < t) {                                                  \
      assert(oop(obj_addr)->is_oop(), "Should be an oop");                  \
      obj_addr += oop(obj_addr)->oop_iterate(blk);                          \
    }                                                                       \
  }
  
  ALL_PAR_OOP_ITERATE_CLOSURES(EdenSpace_PAR_OOP_ITERATE_DEFN)

#undef EdenSpace_PAR_OOP_ITERATE_DEFN

void ContiguousSpace::object_iterate_mem(MemRegion mr, UpwardsObjectClosure* cl) {
  HeapWord* prev = cl->previous();   // max address from last time
  MemRegion cur(MAX2(prev, bottom()), top()); 
  mr = mr.intersection(cur);
  if (mr.is_empty()) {
    return;
  }
  if (mr.equals(MemRegion(bottom(),end()))) {
    object_iterate(cl);
    return;
  }
  HeapWord *addr, *end;
  size_t size;
  for (addr = (prev == mr.start() ? prev : block_start(mr.start())),
       end  = mr.end();
       addr < end; addr += size) {
    oop obj = oop(addr);
    size    = obj->size();
    cl->do_object(obj);
  }
  cl->set_previous(MAX2(addr, prev)); // min address for next time
}

void ContiguousSpace::oop_iterate(OopClosure* blk) {
  if (is_empty()) return;
  HeapWord* obj_addr = bottom();
  HeapWord* t = top();
  // Could call objects iterate, but this is easier.
  while (obj_addr < t) {
    obj_addr += oop(obj_addr)->oop_iterate(blk);
  }
}

void ContiguousSpace::oop_iterate(MemRegion mr, OopClosure* blk) {
  if (is_empty()) {
    return;
  }
  MemRegion cur = MemRegion(bottom(), top());
  mr = mr.intersection(cur);
  if (mr.is_empty()) {
    return;
  }
  if (mr.equals(cur)) {
    oop_iterate(blk);
    return;
  }
  assert(mr.end() <= top(), "just took an intersection above");
  HeapWord* obj_addr = block_start(mr.start());
  HeapWord* t = mr.end();

  // Handle first object specially.
  oop obj = oop(obj_addr);
  SpaceMemRegionOopsIterClosure smr_blk(blk, mr);
  obj_addr += obj->oop_iterate(&smr_blk);
  while (obj_addr < t) {
    oop obj = oop(obj_addr);
    assert(obj->is_oop(), "expected an oop");
    obj_addr += obj->size();
    // If "obj_addr" is not greater than top, then the
    // entire object "obj" is within the region.
    if (obj_addr <= t) {
      obj->oop_iterate(blk);
    } else {
      // "obj" extends beyond end of region
      obj->oop_iterate(&smr_blk);
      break;
    }
  };
}

void ContiguousSpace::object_iterate(ObjectClosure* blk) {
  if (is_empty()) return;
  WaterMark bm = bottom_mark();
  object_iterate_from(bm, blk);
}

void ContiguousSpace::object_iterate_from(WaterMark mark, ObjectClosure* blk) {
  assert(mark.space() == this, "Mark does not match space");
  HeapWord* p = mark.point();
  while (p < top()) {
    blk->do_object(oop(p));
    p += oop(p)->size();
  }
}

#define ContigSpace_OOP_SINCE_SAVE_MARKS_DEFN(OopClosureType, nv_suffix)  \
                                                                          \
void ContiguousSpace::                                                    \
oop_since_save_marks_iterate##nv_suffix(OopClosureType* blk) {            \
  HeapWord* t;                                                            \
  HeapWord* p = saved_mark_word();                                        \
  assert(p != NULL, "expected saved mark");                               \
                                                                          \
  const intx interval = PrefetchScanIntervalInBytes;                      \
  do {                                                                    \
    t = top();                                                            \
    while (p < t) {                                                       \
      Prefetch::write(p, interval);                                       \
      debug_only(HeapWord* prev = p);                                     \
      oop m = oop(p);                                                     \
      p += m->oop_iterate(blk);                                           \
    }                                                                     \
  } while (t < top());                                                    \
                                                                          \
  set_saved_mark_word(p);           	                                  \
}

ALL_SINCE_SAVE_MARKS_CLOSURES(ContigSpace_OOP_SINCE_SAVE_MARKS_DEFN)

#undef ContigSpace_OOP_SINCE_SAVE_MARKS_DEFN

// Very general, slow implementation.
HeapWord* ContiguousSpace::block_start(const void* p) const {
  assert(MemRegion(bottom(), end()).contains(p), "p not in space");
  if (p >= top()) {
    return top();
  } else {
    assert(oop(p)->is_oop(), "p is not an object start");
    HeapWord* last = bottom();
    HeapWord* cur = last;
    while (cur <= p) {
      last = cur;
      cur += oop(cur)->size();
    }
    return last;
  }
}

size_t ContiguousSpace::block_size(const HeapWord* p) const {
  assert(MemRegion(bottom(), end()).contains(p), "p not in space");
  HeapWord* current_top = top();
  assert(p <= current_top, "p is not a block start"); 
  assert(p == current_top || oop(p)->is_oop(), "p is not a block start"); 
  if (p < current_top) 
    return oop(p)->size();
  else {
    assert(p == current_top, "just checking"); 
    return pointer_delta(end(), (HeapWord*) p);
  }
}

// This version requires locking.
inline HeapWord* ContiguousSpace::allocate_impl(size_t size,
						HeapWord* const end_value) {
  assert(Heap_lock->owned_by_self() ||
         (SafepointSynchronize::is_at_safepoint() &&
          Thread::current()->is_VM_thread()),
         "not locked");
  HeapWord* obj = top();
  HeapWord* new_top = obj + size;
  // The 'new_top>obj' check is needed to detect overflow of obj+size.
  if (new_top > obj && new_top <= end_value) {
    set_top(new_top);
    assert(is_aligned(obj) && is_aligned(new_top), "checking alignment");
    return obj;
  } else {
    return NULL;
  }
}

// This version is lock-free.
inline HeapWord* ContiguousSpace::par_allocate_impl(size_t size,
						    HeapWord* const end_value) {
  do {
    HeapWord* obj = top();
    HeapWord* new_top = obj + size;
    // The 'new_top>obj' check is needed to detect overflow of obj+size.
    if (new_top > obj && new_top <= end_value) {
      HeapWord* result = (HeapWord*)Atomic::cmpxchg_ptr(new_top, top_addr(), obj);
      // result can be one of two:
      //  the old top value: the exchange succeeded
      //  otherwise: the new value of the top is returned.
      if (result == obj) {
	assert(is_aligned(obj) && is_aligned(new_top), "checking alignment");
	return obj;
      }
    } else {
      return NULL;
    }
  } while (true);
}

// Requires locking.
HeapWord* ContiguousSpace::allocate(size_t size) {
  return allocate_impl(size, end());
}

// Lock-free.
HeapWord* ContiguousSpace::par_allocate(size_t size) {
  return par_allocate_impl(size, end());
}

void ContiguousSpace::allocate_temporary_filler(int factor) {
  // allocate temporary type array decreasing free size with factor 'factor' 
  assert(factor >= 0, "just checking");
  size_t size = pointer_delta(end(), top());

  // if space is full, return
  if (size == 0) return;

  if (factor > 0) {
    size -= size/factor;
  }
  size = align_object_size(size);

  const size_t min_int_array_size = typeArrayOopDesc::header_size(T_INT);
  if (size >= min_int_array_size) {
    size_t length = (size - min_int_array_size) * (HeapWordSize / sizeof(jint));
    // allocate uninitialized int array
    typeArrayOop t = (typeArrayOop) allocate(size);
    assert(t != NULL, "allocation should succeed");
    t->set_mark();
    t->set_klass(Universe::intArrayKlassObj());
    t->set_length((int)length);
  } else {
    assert((int) size == instanceOopDesc::header_size(),
           "size for smallest fake object doesn't match");
    instanceOop obj = (instanceOop) allocate(size);
    obj->set_mark();
    obj->set_klass(SystemDictionary::object_klass());
  }
}

void EdenSpace::clear() {
  ContiguousSpace::clear();
  set_soft_end(end());
}

// Requires locking.
HeapWord* EdenSpace::allocate(size_t size) {
  return allocate_impl(size, soft_end());
}

// Lock-free.
HeapWord* EdenSpace::par_allocate(size_t size) {
  return par_allocate_impl(size, soft_end());
}

HeapWord* OffsetTableContigSpace::initialize_threshold() {
  return _offsets.initialize_threshold();
}

HeapWord* OffsetTableContigSpace::cross_threshold(HeapWord* start, HeapWord* end) {
  _offsets.alloc_block(start, end);
  return _offsets.threshold();
}

OffsetTableContigSpace::OffsetTableContigSpace(BlockOffsetSharedArray* sharedOffsetArray,
					       MemRegion mr) :
  _offsets(sharedOffsetArray, mr),
  _par_alloc_lock(Mutex::leaf, "OffsetTableContigSpace par alloc lock", true)
{
  _offsets.set_contig_space(this);
  initialize(mr, true);
}


#ifndef PRODUCT

class VerifyOldOopClosure : public OopClosure {
 public:
  oop the_obj;
  bool allow_dirty;
  void do_oop(oop* p) {
    the_obj->verify_old_oop(p, allow_dirty);
  }
};

#define OBJ_SAMPLE_INTERVAL 0
#define BLOCK_SAMPLE_INTERVAL 100

void OffsetTableContigSpace::verify(bool allow_dirty) const {
  HeapWord* p = bottom();
  HeapWord* prev_p = NULL;
  VerifyOldOopClosure blk;
  blk.allow_dirty = allow_dirty;
  int objs = 0;
  int blocks = 0;
  while (p < top()) {
    size_t size = oop(p)->size();
    if (blocks == BLOCK_SAMPLE_INTERVAL) {
      guarantee(p == block_start(p + (size/2)), "check offset computation");
      blocks = 0;
    } else {
      blocks++;
    }
    if (objs == OBJ_SAMPLE_INTERVAL) {
      oop(p)->verify();
      blk.the_obj = oop(p);
      oop(p)->oop_iterate(&blk);
      objs = 0;
    } else {
      objs++;
    }
    prev_p = p;
    p += size;
  }
  guarantee(p == top(), "end of last object must match end of space");
}
#endif

void OffsetTableContigSpace::serialize_block_offset_array_offsets(
                                                      SerializeOopClosure* soc) {
  _offsets.serialize(soc);
}


int TenuredSpace::allowed_dead_ratio() const {
  return MarkSweepDeadRatio;
}


int ContigPermSpace::allowed_dead_ratio() const {
  return PermMarkSweepDeadRatio;
}


size_t CarSpace::_car_size = 0;
size_t CarSpace::_car_size_in_words = 0;
size_t CarSpace::_oversized_car_size_in_words = 0;


CarSpace::CarSpace(TrainGeneration* tg,
		   CardTableRS* ct,
		   BlockOffsetSharedArray* sharedOffsetArray,
		   MemRegion mr, size_t blocks,
		   CarTableDesc* _desc_) :
    OffsetTableContigSpace(sharedOffsetArray, mr)
{
  assert(mr.byte_size() % car_size() == 0, "Illegal car limits");
  assert(mr.byte_size() / car_size() == (size_t) blocks, "Illegal car limits");
  set_next_car(NULL);
  set_blocks(blocks);
  set_remembered_set(new CarRememberedSet(tg, ct));
  set_desc(_desc_);
  set_marked(false);
  set_oversized(false);
}


CarSpace::~CarSpace() {
  delete _remembered_set;
}


CompactibleSpace* CarSpace::next_compaction_space() const {
  assert(next_car() != NULL, "shouldn't compact past end of train");
  return next_car();
}


void CarSpace::init() {
  _car_size          = 1 << LogOfCarSpaceSize;
  _car_size_in_words = _car_size / HeapWordSize;
  _oversized_car_size_in_words = _car_size_in_words * OversizedCarThreshold;

  size_t page_size = (size_t)os::vm_page_size();
  assert(page_size > 0, "page size should be set");
  if (_car_size < page_size || _car_size % os::vm_page_size() != 0) {
    vm_exit_during_initialization("Invalid car size");
  }
  if (OversizedCarThreshold == 0) {
    debug_only(
      gclog_or_tty->print_cr("Invalid OversizedCarThreshold option %u, disabling",
                             OversizedCarThreshold);
    )
    OversizedCarThreshold = 100;   // A high number, should be unused...
    UseOversizedCarHandling = false;
  }
}

void CarSpace::clear() {
  OffsetTableContigSpace::clear();
  _offsets.zero_bottom_entry();
}


HeapWord* CarSpace::allocate(size_t size) {
  HeapWord* obj = NULL;
  // If this is an oversized car, we only want one large object
  // in it. Don't allow allocation here. 
  if (!oversized()) {
    obj = OffsetTableContigSpace::allocate(size);
    if (obj != NULL) {
      set_oversized(CarSpace::is_oversized_size(size));
    }
  }
  return obj;
}

void CarSpace::print_short() const { print_short_on(gclog_or_tty); }
void CarSpace::print_short_on(outputStream* st) const {
  if (oversized()) {
    st->print(" oversized");
  }
  st->print(" car (");
  st->print_julong(train_number());
  st->print(",%u)", car_number());
  Space::print_short_on(st);
}

#ifndef PRODUCT

void CarSpace::verify(bool allow_dirty) const {
  OffsetTableContigSpace::verify(allow_dirty);
  remembered_set()->verify(this);
  if (oversized()) {
    // There should only be one object in this car
    guarantee(bottom() + ((oop)bottom())->size() == top(), 
              "more than one object in oversized car");
  }
  guarantee(desc()->space() == this, "invalid desc in CarSpace");
  guarantee(!marked(), "CarSpace should not be marked outside scavenge");
}

#endif // PRODUCT

void CarSpace::set_target(bool value) {
  size_t b = blocks();
  CarTableDesc* d = desc();
  while (b-- > 0) {
    d++->set_target(value);
  }
}

bool CarSpace::commit() {
  return os::commit_memory((char*)bottom(), sizeof(*bottom()) * 
    pointer_delta(end(), bottom()));
}


void CarSpace::uncommit() {
  os::uncommit_memory((char*)bottom(), sizeof(*bottom()) * 
    pointer_delta(end(), bottom()));
  debug_only(os::guard_memory((char*)bottom(), sizeof(*bottom()) * 
    pointer_delta(end(), bottom())));
}


void carSpace_init() {
  CarSpace::init();
}

