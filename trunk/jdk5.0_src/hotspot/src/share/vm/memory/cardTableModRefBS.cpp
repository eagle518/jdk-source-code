#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)cardTableModRefBS.cpp	1.46 03/12/23 16:40:55 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

// This kind of "BarrierSet" allows a "CollectedHeap" to detect and
// enumerate ref fields that have been modified (since the last
// enumeration.)

# include "incls/_precompiled.incl"
# include "incls/_cardTableModRefBS.cpp.incl"

CardTableModRefBS::CardTableModRefBS(MemRegion whole_heap,
				     int max_covered_regions) :
  ModRefBarrierSet(max_covered_regions), _whole_heap(whole_heap)
{
  _kind = BarrierSet::CardTableModRef;

  HeapWord* low_bound  = _whole_heap.start();
  HeapWord* high_bound = _whole_heap.end();
  assert((uintptr_t(low_bound)  & (card_size - 1))  == 0, "heap must start at card boundary");
  assert((uintptr_t(high_bound) & (card_size - 1))  == 0, "heap must end at card boundary");

  assert(card_size <= 512, "card_size must be less than 512");
  size_t heap_size_in_words = _whole_heap.word_size();
  // Add one for the last_card, treated as a guard card
  _byte_map_size = ReservedSpace::allocation_align_size_up((heap_size_in_words / 
                                                      card_size_in_words) + 1);
  // A couple of useful indicies
  _guard_index      = _byte_map_size - 1;
  _last_valid_index = _byte_map_size - 2;

  _covered   = new MemRegion[max_covered_regions];
  _committed = new MemRegion[max_covered_regions];
  if (_covered == NULL || _committed == NULL)
    vm_exit_during_initialization("couldn't alloc card table covered region set.");
  int i;
  for (i = 0; i < max_covered_regions; i++) {
    _covered[i].set_word_size(0);
    _committed[i].set_word_size(0);
  }
  _cur_covered_regions = 0;

  ReservedSpace heap_rs(_byte_map_size);
  if (!heap_rs.is_reserved()) {
    vm_exit_during_initialization("Could not reserve enough space for card marking array");
  }
  // The assember store_check code will do an unsigned shift of the oop, 
  // then add it to byte_map_base, i.e.
  // 
  //   _byte_map = byte_map_base + (uintptr_t(low_bound) >> card_shift)
  _byte_map = (jbyte*) heap_rs.base();
  byte_map_base = _byte_map - (uintptr_t(low_bound) >> card_shift);
  assert(byte_for(low_bound) == &_byte_map[0], "Checking start of map");
  assert(byte_for(high_bound-1) <= &_byte_map[_last_valid_index], "Checking end of map");

  jbyte* guard_card = &_byte_map[_guard_index];
  uintptr_t guard_page = align_size_down((uintptr_t)guard_card, os::vm_page_size());
  _guard_region = MemRegion((HeapWord*)guard_page, os::vm_page_size());
  if (!os::commit_memory((char*)guard_page, os::vm_page_size())) {
    // Do better than this for Merlin
    vm_exit_out_of_memory(os::vm_page_size(), "card table last card");
  }
  *guard_card = last_card;

   _lowest_non_clean =
    NEW_C_HEAP_ARRAY(CardArr, max_covered_regions);
  _lowest_non_clean_chunk_size =
    NEW_C_HEAP_ARRAY(size_t, max_covered_regions);
  _lowest_non_clean_base_chunk_index =
    NEW_C_HEAP_ARRAY(uintptr_t, max_covered_regions);
  _last_LNC_resizing_collection =
    NEW_C_HEAP_ARRAY(int, max_covered_regions);
  if (_lowest_non_clean == NULL
      || _lowest_non_clean_chunk_size == NULL 
      || _lowest_non_clean_base_chunk_index == NULL 
      || _last_LNC_resizing_collection == NULL)
    vm_exit_during_initialization("couldn't allocate an LNC array.");
  for (i = 0; i < max_covered_regions; i++) {
    _lowest_non_clean[i] = NULL;
    _lowest_non_clean_chunk_size[i] = 0;
    _last_LNC_resizing_collection[i] = -1;
  }

  if (TraceCardTableModRefBS) {
    gclog_or_tty->print_cr("CardTableModRefBS::CardTableModRefBS: ");
    gclog_or_tty->print_cr("  "
                  "  &_byte_map[0]: " INTPTR_FORMAT
                  "  &_byte_map[_last_valid_index]: " INTPTR_FORMAT,
                  &_byte_map[0],
                  &_byte_map[_last_valid_index]);
    gclog_or_tty->print_cr("  "
                  "  byte_map_base: " INTPTR_FORMAT,
                  byte_map_base);
  }
}

int CardTableModRefBS::find_covering_region_by_base(HeapWord* base) {
  int i;
  for (i = 0; i < _cur_covered_regions; i++) {
    if (_covered[i].start() == base) return i;
    if (_covered[i].start() > base) break;
  }
  // If we didn't find it, create a new one.
  assert(_cur_covered_regions < _max_covered_regions,
	 "too many covered regions");
  // Move the ones above up, to maintain sorted order.
  for (int j = _cur_covered_regions; j > i; j--) {
    _covered[j] = _covered[j-1];
    _committed[j] = _committed[j-1];
  }
  int res = i;
  _cur_covered_regions++;
  _covered[res].set_start(base);
  _covered[res].set_word_size(0);
  jbyte* ct_start = byte_for(base);
  uintptr_t ct_start_aligned =
    align_size_down((uintptr_t)ct_start, os::vm_page_size());
  _committed[res].set_start((HeapWord*)ct_start_aligned);
  _committed[res].set_word_size(0);
  return res;
}

int CardTableModRefBS::find_covering_region_containing(HeapWord* addr) {
  for (int i = 0; i < _cur_covered_regions; i++) {
    if (_covered[i].contains(addr)) {
      return i;
    }
  }
  assert(0, "address outside of heap?");
  return -1;
}

HeapWord* CardTableModRefBS::largest_prev_committed_end(int ind) const {
  HeapWord* max_end = NULL;
  for (int j = 0; j < ind; j++) {
    HeapWord* this_end = _committed[j].end();
    if (this_end > max_end) max_end = this_end;
  }
  return max_end;
}

MemRegion CardTableModRefBS::committed_unique_to_self(int self, 
                                                      MemRegion mr) const {
  MemRegion result = mr;
  for (int r = 0; r < _cur_covered_regions; r += 1) {
    if (r != self) {
      result = result.minus(_committed[r]);
    }
  }
  // Never include the guard page.
  result = result.minus(_guard_region);
  return result;
}

void CardTableModRefBS::resize_covered_region(MemRegion new_region) {
  // We don't change the start of a region, only the end.
  assert(_whole_heap.contains(new_region), "attempt to cover area not in reserved area");
  int ind = find_covering_region_by_base(new_region.start());
  MemRegion old_region = _covered[ind];
  assert(old_region.start() == new_region.start(), "just checking");
  if (new_region.word_size() != old_region.word_size()) {
    // Commit new or uncommit old pages, if necessary.
    MemRegion cur_committed = _committed[ind];
    // Extend the end of this _commited region 
    // to cover the end of any lower _committed regions.
    // This forms overlapping regions, but never interior regions.
    HeapWord* max_prev_end = largest_prev_committed_end(ind);
    if (max_prev_end > cur_committed.end()) {
      cur_committed.set_end(max_prev_end);
    }
    // Align the end up to a page size (starts are already aligned).
    jbyte* new_end = byte_after(new_region.last());
    HeapWord* new_end_aligned =
      (HeapWord*)align_size_up((uintptr_t)new_end, os::vm_page_size());
    assert(new_end_aligned >= (HeapWord*) new_end,
           "align up, but less");
    if (new_end_aligned > cur_committed.end()) {
      // Must commit new pages.
      MemRegion new_committed =
	MemRegion(cur_committed.end(), new_end_aligned);
      if (!os::commit_memory((char*)new_committed.start(),
			     new_committed.byte_size())) {
	// Do better than this for Merlin
	vm_exit_out_of_memory(new_committed.byte_size(),
			      "card table expansion");
      }
    } else if (new_end_aligned < cur_committed.end()) {
      // Must uncommit pages.
      MemRegion uncommit_region = 
        committed_unique_to_self(ind, MemRegion(new_end_aligned,
                                                cur_committed.end()));
      if (!uncommit_region.is_empty()) {
        if (!os::uncommit_memory((char*)uncommit_region.start(),
			         uncommit_region.byte_size())) {
          // Do better than this for Merlin
          vm_exit_out_of_memory(uncommit_region.byte_size(),
            "card table contraction");
        }
      }
    }
    // In any case, we can reset the end of the current committed entry.
    _committed[ind].set_end(new_end_aligned);

    // The default of 0 is not necessarily clean cards.
    jbyte* entry;
    if (old_region.last() < _whole_heap.start()) {
      entry = byte_for(_whole_heap.start());
    } else {
      entry = byte_after(old_region.last());
    }
    jbyte* end = byte_after(new_region.last());
    // This loop does nothing if we resized downward.
    while (entry < end) { *entry++ = clean_card; }

  }
  // In any case, the covered size changes.
  _covered[ind].set_word_size(new_region.word_size());
  if (TraceCardTableModRefBS) {
    gclog_or_tty->print_cr("CardTableModRefBS::resize_covered_region: ");
    gclog_or_tty->print_cr("  "
                  "  _covered[%d].start(): " INTPTR_FORMAT
                  "  _covered[%d].last(): " INTPTR_FORMAT,
                  ind, _covered[ind].start(), 
                  ind, _covered[ind].last());
    gclog_or_tty->print_cr("  "
                  "  _committed[%d].start(): " INTPTR_FORMAT
                  "  _committed[%d].last(): " INTPTR_FORMAT,
                  ind, _committed[ind].start(),
                  ind, _committed[ind].last());
    gclog_or_tty->print_cr("  "
                  "  byte_for(start): " INTPTR_FORMAT
                  "  byte_for(last): " INTPTR_FORMAT,
                  byte_for(_covered[ind].start()),
                  byte_for(_covered[ind].last()));
    gclog_or_tty->print_cr("  "
                  "  addr_for(start): " INTPTR_FORMAT
                  "  addr_for(last): " INTPTR_FORMAT,
                  addr_for((jbyte*) _committed[ind].start()),
                  addr_for((jbyte*) _committed[ind].last()));
  }
}

// Note that these versions are precise!  The scanning code has to handle the
// fact that the write barrier may be either precise or imprecise.

void CardTableModRefBS::write_ref_field_work(oop* field, oop newVal) {
  inline_write_ref_field(field, newVal);
}

size_t CardTableModRefBS::chunks_to_cover(MemRegion mr) {
  return (size_t)(addr_to_chunk_index(mr.last()) -
		  addr_to_chunk_index(mr.start()) + 1);
}

uintptr_t CardTableModRefBS::addr_to_chunk_index(const void* addr) {
  uintptr_t card = (uintptr_t) byte_for(addr);
  return card / CardsPerStrideChunk;
}

void CardTableModRefBS::non_clean_card_iterate(Space* sp,
					       MemRegion mr,
					       DirtyCardToOopClosure* dcto_cl,
					       MemRegionClosure* cl,
					       bool clear) {
  if (!mr.is_empty()) {
    int n_threads = SharedHeap::heap()->n_par_threads();
    if (n_threads > 0) {
      par_non_clean_card_iterate_work(sp, mr, dcto_cl, cl, clear, n_threads);
    } else {
      non_clean_card_iterate_work(mr, cl, clear);
    }
  }
}

// NOTE: For this to work correctly, it is important that
// we look for non-clean cards below (so as to catch those
// marked precleaned), rather than look explicitly for dirty
// cards (and miss those marked precleaned). In that sense,
// the name precleaned is currently somewhat of a misnomer.
void CardTableModRefBS::non_clean_card_iterate_work(MemRegion mr,
						    MemRegionClosure* cl,
						    bool clear) {
  // Figure out whether we have to worry about parallelism.
  bool is_par = (SharedHeap::heap()->n_par_threads() > 1);
  for (int i = 0; i < _cur_covered_regions; i++) {
    MemRegion mri = mr.intersection(_covered[i]);
    if (mri.word_size() > 0) {
      jbyte* cur_entry = byte_for(mri.last());
      jbyte* limit = byte_for(mri.start());
      while (cur_entry >= limit) {
        jbyte* next_entry = cur_entry - 1;
	if (*cur_entry != clean_card) {
	  size_t non_clean_cards = 1;
	  // Should the next card be included in this range of dirty cards.
          while (next_entry >= limit && *next_entry != clean_card) {
	    non_clean_cards++; 
	    cur_entry = next_entry;
	    next_entry--;
	  }
	  // The memory region may not be on a card boundary.  So that
	  // objects beyond the end of the region are not processed, make
	  // cur_cards precise with regard to the end of the memory region.
	  MemRegion cur_cards(addr_for(cur_entry), 
			      non_clean_cards * card_size_in_words);
	  MemRegion dirty_region = cur_cards.intersection(mri);
	  if (clear) {
            for (size_t i = 0; i < non_clean_cards; i++) {
	      // Clean the dirty cards (but leave the other non-clean
	      // alone.)  If parallel, do the cleaning atomically.
	      jbyte cur_entry_val = cur_entry[i];
	      if (card_is_dirty_wrt_gen_iter(cur_entry_val)) {
		if (is_par) {
		  jbyte res = Atomic::cmpxchg(clean_card, &cur_entry[i], cur_entry_val);
		  assert(res != clean_card,
			 "Dirty card mysteriously cleaned");
		} else {
		  cur_entry[i] = clean_card;
		}
	      }
            }
          }
	  cl->do_MemRegion(dirty_region);
	}
	cur_entry = next_entry;
      }
    }
  }
}

void CardTableModRefBS::par_non_clean_card_iterate_work(Space* sp, MemRegion mr,
                                                        DirtyCardToOopClosure* dcto_cl,
                                                        MemRegionClosure* cl,
                                                        bool clear,
                                                        int n_threads) {
  if (n_threads > 0) {
    assert(n_threads == (int)ParallelGCThreads, "# worker threads != # requested!");
      
      // Make sure the LNC array is valid for the space.
    jbyte**   lowest_non_clean;
    uintptr_t lowest_non_clean_base_chunk_index;
    size_t    lowest_non_clean_chunk_size;
    get_LNC_array_for_space(sp, lowest_non_clean,
                            lowest_non_clean_base_chunk_index,
                            lowest_non_clean_chunk_size);

    int n_strides = n_threads * StridesPerThread;
    SequentialSubTasksDone* pst = sp->par_seq_tasks();
    pst->set_par_threads(n_threads);
    pst->set_n_tasks(n_strides);

    int stride = 0;
    while (!pst->is_task_claimed(/* reference */ stride)) {
      process_stride(sp, mr, stride, n_strides, dcto_cl, cl, clear,
                     lowest_non_clean, 
                     lowest_non_clean_base_chunk_index,
                     lowest_non_clean_chunk_size);
    }
    if (pst->all_tasks_completed()) {
      // Clear lowest_non_clean array for next time.
      intptr_t first_chunk_index = addr_to_chunk_index(mr.start());
      uintptr_t last_chunk_index  = addr_to_chunk_index(mr.last());
      for (uintptr_t ch = first_chunk_index; ch <= last_chunk_index; ch++) {
        intptr_t ind = ch - lowest_non_clean_base_chunk_index;
        assert(0 <= ind && ind < (intptr_t)lowest_non_clean_chunk_size,
               "Bounds error");
        lowest_non_clean[ind] = NULL;
      }
    }
  }
}

void CardTableModRefBS::mod_oop_in_space_iterate(Space* sp,
                                                 OopClosure* cl,
                                                 bool clear,
						 bool before_save_marks) {
  // Note that dcto_cl is resource-allocated, so there is no
  // corresponding "delete".
  DirtyCardToOopClosure* dcto_cl = sp->new_dcto_cl(cl, precision());
  MemRegion used_mr;
  if (before_save_marks) {
    used_mr = sp->used_region_at_save_marks();
  } else {
    used_mr = sp->used_region();
  }
  non_clean_card_iterate(sp, used_mr, dcto_cl, dcto_cl, clear);
}

void CardTableModRefBS::dirty_MemRegion(MemRegion mr) {
  jbyte* cur  = byte_for(mr.start());
  jbyte* last = byte_after(mr.last());
  while (cur < last) {
    *cur = dirty_card;
    cur++;
  }
}

void CardTableModRefBS::invalidate(MemRegion mr) {
  for (int i = 0; i < _cur_covered_regions; i++) {
    MemRegion mri = mr.intersection(_covered[i]);
    if (!mri.is_empty()) dirty_MemRegion(mri);
  }
}

void CardTableModRefBS::clear_MemRegion(MemRegion mr) {
  // Be conservative: only clean cards entirely contained within the
  // region.
  jbyte* cur;
  if (mr.start() == _whole_heap.start()) {
    cur = byte_for(mr.start());
  } else {
    assert(mr.start() > _whole_heap.start(), "mr is not covered.");
    cur = byte_after(mr.start() - 1);
  }
  jbyte* last = byte_after(mr.last());
  while (cur < last) {
    *cur = clean_card;
    cur++;
  }
}

void CardTableModRefBS::clear(MemRegion mr) {
  for (int i = 0; i < _cur_covered_regions; i++) {
    MemRegion mri = mr.intersection(_covered[i]);
    if (!mri.is_empty()) clear_MemRegion(mri);
  }
}

// NOTE: unlike mod_oop_in_space_iterate() above, dirty_card_iterate()
// iterates over dirty cards ranges in increasing address order.
void CardTableModRefBS::dirty_card_iterate(MemRegion mr,
                                           MemRegionClosure* cl) {
  for (int i = 0; i < _cur_covered_regions; i++) {
    MemRegion mri = mr.intersection(_covered[i]);
    if (!mri.is_empty()) {
      jbyte *cur_entry, *next_entry, *limit;
      for (cur_entry = byte_for(mri.start()), limit = byte_for(mri.last());
           cur_entry <= limit;
           cur_entry  = next_entry) {
        next_entry = cur_entry + 1;
        if (*cur_entry == dirty_card) {
          size_t dirty_cards;
          // Accumulate maximal dirty card range, starting at cur_entry
          for (dirty_cards = 1;
               next_entry <= limit && *next_entry == dirty_card;
               dirty_cards++, next_entry++);
          MemRegion cur_cards(addr_for(cur_entry),
                              dirty_cards*card_size_in_words);
          for (size_t i = 0; i < dirty_cards; i++) {
             cur_entry[i] = precleaned_card;
          }
          cl->do_MemRegion(cur_cards);
        }
      }
    }
  }
}

MemRegion CardTableModRefBS::dirty_card_range_after_preclean(MemRegion mr) {
  for (int i = 0; i < _cur_covered_regions; i++) {
    MemRegion mri = mr.intersection(_covered[i]);
    if (!mri.is_empty()) {
      jbyte* cur_entry, *next_entry, *limit;
      for (cur_entry = byte_for(mri.start()), limit = byte_for(mri.last());
           cur_entry <= limit;
           cur_entry  = next_entry) {
        next_entry = cur_entry + 1;
        if (*cur_entry == dirty_card) {
          size_t dirty_cards;
          // Accumulate maximal dirty card range, starting at cur_entry
          for (dirty_cards = 1;
               next_entry <= limit && *next_entry == dirty_card;
               dirty_cards++, next_entry++);
          MemRegion cur_cards(addr_for(cur_entry),
                              dirty_cards*card_size_in_words);
          for (size_t i = 0; i < dirty_cards; i++) {
             cur_entry[i] = precleaned_card;
          }
          return cur_cards;
        }
      }
    }
  }
  return MemRegion(mr.end(), mr.end());
}

// Set all the dirty cards in the given region to "precleaned" state.
void CardTableModRefBS::preclean_dirty_cards(MemRegion mr) {
  for (int i = 0; i < _cur_covered_regions; i++) {
    MemRegion mri = mr.intersection(_covered[i]);
    if (!mri.is_empty()) {
      jbyte *cur_entry, *limit;
      for (cur_entry = byte_for(mri.start()), limit = byte_for(mri.last());
           cur_entry <= limit;
           cur_entry++) {
        if (*cur_entry == dirty_card) {
          *cur_entry = precleaned_card;
        }
      }
    }
  }
}

uintx CardTableModRefBS::ct_max_alignment_constraint() {
  return card_size * os::vm_page_size();
}

void
CardTableModRefBS::
process_stride(Space* sp,
	       MemRegion used,
	       jint stride, int n_strides,
	       DirtyCardToOopClosure* dcto_cl,
	       MemRegionClosure* cl,
	       bool clear,
	       jbyte** lowest_non_clean,
	       uintptr_t lowest_non_clean_base_chunk_index,
	       size_t    lowest_non_clean_chunk_size) {
  // We don't have to go downwards here; it wouldn't help anyway,
  // because of parallelism.

  // Find the first card address of the first chunk in the stride that is
  // at least "bottom" of the used region.
  jbyte*    start_card  = byte_for(used.start());
  jbyte*    end_card    = byte_after(used.last());
  uintptr_t start_chunk = addr_to_chunk_index(used.start());
  uintptr_t start_chunk_stride_num = start_chunk % n_strides;

  jbyte* chunk_card_start;
  if ((uintptr_t)stride >= start_chunk_stride_num) {
    chunk_card_start = (jbyte*)(start_card +
				(stride - start_chunk_stride_num) *
				CardsPerStrideChunk);
  } else {
    // Go ahead to the next chunk group boundary, then to the requested stride.
    chunk_card_start = (jbyte*)(start_card +
				(n_strides - start_chunk_stride_num + stride) *
				CardsPerStrideChunk);
  }

  while (chunk_card_start < end_card) {
    // We don't have to go downwards here; it wouldn't help anyway,
    // because of parallelism.  (We take care with "min_done"; see below.)
    // Invariant: chunk_mr should be fully contained within the "used" region.
    jbyte*    chunk_card_end = chunk_card_start + CardsPerStrideChunk;
    MemRegion chunk_mr       = MemRegion(addr_for(chunk_card_start),
                                         chunk_card_end >= end_card ?
                                           used.end() : addr_for(chunk_card_end));
    assert(chunk_mr.word_size() > 0, "[chunk_card_start > used_end)");
    assert(used.contains(chunk_mr), "chunk_mr should be subset of used");

    // Process the chunk.
    process_chunk_boundaries(sp,
			     dcto_cl,
			     chunk_mr,
			     used,
			     lowest_non_clean,
			     lowest_non_clean_base_chunk_index,
			     lowest_non_clean_chunk_size);

    non_clean_card_iterate_work(chunk_mr, cl, clear);

    // Find the next chunk of the stride.
    chunk_card_start += CardsPerStrideChunk * n_strides;
  }
}

void
CardTableModRefBS::
process_chunk_boundaries(Space* sp,
			 DirtyCardToOopClosure* dcto_cl,
			 MemRegion chunk_mr,
			 MemRegion used,
			 jbyte** lowest_non_clean,
			 uintptr_t lowest_non_clean_base_chunk_index,
			 size_t    lowest_non_clean_chunk_size)
{
  // We must worry about the chunk boundaries.

  // First, set our max_to_do:
  HeapWord* max_to_do = NULL;
  uintptr_t cur_chunk_index = addr_to_chunk_index(chunk_mr.start());
  cur_chunk_index           = cur_chunk_index - lowest_non_clean_base_chunk_index;

  if (chunk_mr.end() < used.end()) {
    // This is not the last chunk in the used region.  What is the last
    // object?
    HeapWord* last_block = sp->block_start(chunk_mr.end());
    assert(last_block <= chunk_mr.end(), "In case this property changes.");
    if (last_block == chunk_mr.end()
	|| !sp->block_is_obj(last_block)) {
      max_to_do = chunk_mr.end();

    } else {
      // It is an object and starts in the current chunk.
      jbyte* last_obj_card = byte_for(last_block);
      if (!card_may_have_been_dirty(*last_obj_card)) {
	// The card containing the head is not dirty.  Any marks in
	// subsequent cards still in this chunk must have been made
	// precisely; we can cap processing at the end.
	max_to_do = chunk_mr.end();
      } else {
	// The last object must be considered dirty, and extends onto the
	// following chunk.  Look for a dirty card in that chunk that will
	// bound our processing.
	jbyte* limit_card = NULL;
	size_t last_block_size = sp->block_size(last_block);
	jbyte* last_card_of_last_obj =
	  byte_for(last_block + last_block_size - 1);
	jbyte* first_card_of_next_chunk = byte_for(chunk_mr.end());
	for (jbyte* cur = first_card_of_next_chunk;
	     cur <= last_card_of_last_obj; cur++) {
	  if (card_will_be_scanned(*cur)) {
	    limit_card = cur; break;
	  }
	}
        assert(0 <= cur_chunk_index+1 &&
               cur_chunk_index+1 < lowest_non_clean_chunk_size,
               "Bounds error.");
        jbyte* lnc_card = lowest_non_clean[cur_chunk_index+1];
	if (limit_card == NULL) {
	  limit_card = lnc_card;
	}
	if (limit_card != NULL) {
	  if (lnc_card != NULL) {
   	    limit_card = (jbyte*)MIN2((intptr_t)limit_card,
				      (intptr_t)lnc_card);
          }
	  max_to_do = addr_for(limit_card);
	} else {
	  max_to_do = last_block + last_block_size;
	}
      }
    }
    assert(max_to_do != NULL, "OOPS!");
  } else {
    max_to_do = used.end();
  }
  // Now we can set the closure we're using so it doesn't to beyond
  // max_to_do.
  dcto_cl->set_min_done(max_to_do);
#ifndef PRODUCT
  dcto_cl->set_last_bottom(max_to_do);
#endif

  // Now we set *our" lowest_non_clean entry.
  // Find the object that spans our boundary, if one exists.
  // Nothing to do on the first chunk.
  if (chunk_mr.start() > used.start()) {
    HeapWord* first_block = sp->block_start(chunk_mr.start());
    if (first_block < chunk_mr.start() &&
	sp->block_is_obj(first_block)) {
      jbyte* first_dirty_card = NULL;
      jbyte* last_card_of_first_obj =
	  byte_for(first_block + sp->block_size(first_block) - 1);
      jbyte* first_card_of_cur_chunk = byte_for(chunk_mr.start());
      for (jbyte* cur = first_card_of_cur_chunk;
	   cur <= last_card_of_first_obj; cur++) {
	if (card_will_be_scanned(*cur)) {
	  first_dirty_card = cur; break;
	}
      }
      if (first_dirty_card != NULL) {
	assert(0 <= cur_chunk_index &&
		 cur_chunk_index < lowest_non_clean_chunk_size,
	       "Bounds error.");
	lowest_non_clean[cur_chunk_index] = first_dirty_card;
      }
    }
  }
}

void
CardTableModRefBS::
get_LNC_array_for_space(Space* sp,
			jbyte**& lowest_non_clean,
			uintptr_t& lowest_non_clean_base_chunk_index,
			size_t& lowest_non_clean_chunk_size) {

  int       i        = find_covering_region_containing(sp->bottom());
  MemRegion covered  = _covered[i];
  size_t    n_chunks = chunks_to_cover(covered);
  
  // Only the first thread to obtain the lock will resize the
  // LNC array for the covered region.  Any later expansion can't affect
  // the used_at_save_marks region.
  // (I observed a bug in which the first thread to execute this would
  // resize, and then it would cause "expand_and_allocates" that would 
  // Increase the number of chunks in the covered region.  Then a second
  // thread would come and execute this, see that the size didn't match,
  // and free and allocate again.  So the first thread would be using a
  // freed "_lowest_non_clean" array.)

  // Do a dirty read here. If we pass the conditional then take the rare
  // event lock and do the read again in case some other thread had already
  // succeeded and done the resize.
  int cur_collection = Universe::heap()->total_collections();
  if (_last_LNC_resizing_collection[i] != cur_collection) {
    MutexLocker x(ParGCRareEvent_lock);
    if (_last_LNC_resizing_collection[i] != cur_collection) {
      if (_lowest_non_clean[i] == NULL ||
	  n_chunks != _lowest_non_clean_chunk_size[i]) {
	
	// Should we delete the old?
	if (_lowest_non_clean[i] != NULL) {
	  assert(n_chunks != _lowest_non_clean_chunk_size[i],
		 "logical consequence");
	  FREE_C_HEAP_ARRAY(CardPtr, _lowest_non_clean[i]);
	  _lowest_non_clean[i] = NULL;
	}
	// Now allocate a new one if necessary.
	if (_lowest_non_clean[i] == NULL) {
	  _lowest_non_clean[i]                  = NEW_C_HEAP_ARRAY(CardPtr, n_chunks);
	  _lowest_non_clean_chunk_size[i]       = n_chunks;
	  _lowest_non_clean_base_chunk_index[i] = addr_to_chunk_index(covered.start());
	  for (int j = 0; j < (int)n_chunks; j++)
	    _lowest_non_clean[i][j] = NULL;
	}
      }
      _last_LNC_resizing_collection[i] = cur_collection;
    }
  }
  // In any case, now do the initialization.
  lowest_non_clean                  = _lowest_non_clean[i];
  lowest_non_clean_base_chunk_index = _lowest_non_clean_base_chunk_index[i];
  lowest_non_clean_chunk_size       = _lowest_non_clean_chunk_size[i];
}

#ifndef PRODUCT
void CardTableModRefBS::verify() {
  guarantee(_byte_map[_guard_index] == last_card,
            "card table guard has been modified");
}

class GuaranteeNotModClosure: public MemRegionClosure {
  CardTableModRefBS* _ct;
public:
  GuaranteeNotModClosure(CardTableModRefBS* ct) : _ct(ct) {}
  void do_MemRegion(MemRegion mr) {
    jbyte* entry = _ct->byte_for(mr.start());
    guarantee(*entry != CardTableModRefBS::clean_card,
	      "Dirty card in region that should be clean");
  }
};

void CardTableModRefBS::verify_clean_region(MemRegion mr) {
  GuaranteeNotModClosure blk(this);
  non_clean_card_iterate_work(mr, &blk, false);
}
#endif

bool CardTableModRefBSForCTRS::card_will_be_scanned(jbyte cv) {
  return
    CardTableModRefBS::card_will_be_scanned(cv) ||
    _rs->is_prev_nonclean_card_val(cv);
};

bool CardTableModRefBSForCTRS::card_may_have_been_dirty(jbyte cv) {
  return
    cv != clean_card &&
    (CardTableModRefBS::card_may_have_been_dirty(cv) ||
     CardTableRS::youngergen_may_have_been_dirty(cv));
};
