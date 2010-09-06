#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)tenuredGeneration.cpp	1.29 04/04/19 08:35:13 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_tenuredGeneration.cpp.incl"

class ParGCAllocBufferWithBOT: public ParGCAllocBuffer {
  BlockOffsetArrayContigSpace _bt;
  BlockOffsetSharedArray*     _bsa;
  HeapWord*                   _true_end;  // end of the whole ParGCAllocBuffer

  static const size_t ChunkSizeInWords;
  static const size_t ChunkSizeInBytes;
  HeapWord* allocate_slow(size_t word_sz);

  void fill_region_with_block(MemRegion mr, bool contig);

public:
  ParGCAllocBufferWithBOT(size_t word_sz, BlockOffsetSharedArray* bsa);

  HeapWord* allocate(size_t word_sz) {
    HeapWord* res = ParGCAllocBuffer::allocate(word_sz);
    if (res != NULL) {
      _bt.alloc_block(res, word_sz);
    } else {
      res = allocate_slow(word_sz);
    }
    return res;
  }

  void undo_allocation(HeapWord* obj, size_t word_sz);

  void set_buf(HeapWord* buf_start) {
    ParGCAllocBuffer::set_buf(buf_start);
    _true_end = _hard_end;
    _bt.set_region(MemRegion(buf_start, word_sz()));
    _bt.initialize_threshold();
  }

  void retire(bool end_of_gc, bool retain);

  MemRegion range() {
    return MemRegion(_top, _true_end);
  }
};

const size_t ParGCAllocBufferWithBOT::ChunkSizeInWords =
MIN2(CardTableModRefBS::par_chunk_heapword_alignment(),
     ((size_t)Generation::GenGrain)/HeapWordSize);
const size_t ParGCAllocBufferWithBOT::ChunkSizeInBytes =
MIN2(CardTableModRefBS::par_chunk_heapword_alignment() * HeapWordSize,
     (size_t)Generation::GenGrain);


TenuredGeneration::TenuredGeneration(ReservedSpace rs,
				     size_t initial_byte_size, int level, 
				     GenRemSet* remset) :
  OneContigSpaceCardGeneration(rs, initial_byte_size,
			       MinHeapDeltaBytes, level, remset, NULL)
{
  HeapWord* bottom = (HeapWord*) _virtual_space.low();
  HeapWord* end    = (HeapWord*) _virtual_space.high();
  _the_space  = new TenuredSpace(_bts, MemRegion(bottom, end));
  _the_space->reset_saved_mark();
  _shrink_factor = 0;
  _capacity_at_prologue = 0;

  if (jvmpi::is_event_enabled(JVMPI_EVENT_ARENA_NEW)) {
    jvmpi::post_arena_new_event(Universe::heap()->addr_to_arena_id(bottom),
				name());
  }

  // initialize performance counters

  const char* gen_name = "old";

  // Generation Counters -- generation 1, 1 subspace
  _gen_counters = new GenerationCounters(gen_name, 1, 1, &_virtual_space);

  _gc_counters = new CollectorCounters("MSC", 1);

  _space_counters = new CSpaceCounters(gen_name, 0,
                                       _virtual_space.reserved_size(),
                                       _the_space, _gen_counters);

  if (UseParNewGC && ParallelGCThreads > 0) {
    typedef ParGCAllocBufferWithBOT* ParGCAllocBufferWithBOTPtr;
    _alloc_buffers = NEW_C_HEAP_ARRAY(ParGCAllocBufferWithBOTPtr,
				      ParallelGCThreads);
    if (_alloc_buffers == NULL) 
      vm_exit_during_initialization("Could not allocate alloc_buffers");
    for (uint i = 0; i < ParallelGCThreads; i++) {
      _alloc_buffers[i] =
	new ParGCAllocBufferWithBOT(ParallelGCOldGenAllocBufferSize, _bts);
      if (_alloc_buffers[i] == NULL) 
	vm_exit_during_initialization("Could not allocate alloc_buffers");
    }
  } else {
    _alloc_buffers = NULL;
  }
}


const char* TenuredGeneration::name() const {
  return "tenured generation";
}

int TenuredGeneration::addr_to_arena_id(void* addr) {
  if (_the_space->contains(addr)) {
    return 0;
  } else {
    return -1;
  }
}

void TenuredGeneration::compute_new_size() {
  assert(_shrink_factor <= 100, "invalid shrink factor");
  size_t current_shrink_factor = _shrink_factor;
  _shrink_factor = 0;

  // We don't have floating point command-line arguments
  // Note:  argument processing ensures that MinHeapFreeRatio < 100.
  const double minimum_free_percentage = MinHeapFreeRatio / 100.0;
  const double maximum_used_percentage = 1.0 - minimum_free_percentage;

  // Compute some numbers about the state of the heap.
  const size_t used_after_gc = used();
  const size_t capacity_after_gc = capacity();

  const double min_tmp = used_after_gc / maximum_used_percentage;
  size_t minimum_desired_capacity = MIN2(min_tmp, double(max_uintx));
  // Don't shrink less than the initial generation size
  minimum_desired_capacity = MAX2(minimum_desired_capacity,
				  spec()->init_size());
  assert(used_after_gc <= minimum_desired_capacity, "sanity check");

  if (PrintGC && Verbose) {
    const size_t free_after_gc = free();
    const double free_percentage = ((double)free_after_gc) / capacity_after_gc;
    gclog_or_tty->print_cr("TenuredGeneration::compute_new_size: ");
    gclog_or_tty->print_cr("  "
                  "  minimum_free_percentage: %6.2f"
                  "  maximum_used_percentage: %6.2f",
                  minimum_free_percentage,
                  maximum_used_percentage);
    gclog_or_tty->print_cr("  "
                  "   free_after_gc   : %6.1fK"
                  "   used_after_gc   : %6.1fK"
                  "   capacity_after_gc   : %6.1fK",
                  free_after_gc / (double) K,
                  used_after_gc / (double) K,
                  capacity_after_gc / (double) K);
    gclog_or_tty->print_cr("  "
                  "   free_percentage: %6.2f",
                  free_percentage);
  }

  if (capacity_after_gc < minimum_desired_capacity) {
    // If we have less free space than we want then expand
    size_t expand_bytes = minimum_desired_capacity - capacity_after_gc;
    // Don't expand unless it's significant
    if (expand_bytes >= _min_heap_delta_bytes) {
      expand(expand_bytes, 0); // safe if expansion fails
    }
    if (PrintGC && Verbose) {
      gclog_or_tty->print_cr("    expanding:"
                    "  minimum_desired_capacity: %6.1fK"
                    "  expand_bytes: %6.1fK"
                    "  _min_heap_delta_bytes: %6.1fK",
                    minimum_desired_capacity / (double) K,
                    expand_bytes / (double) K,
                    _min_heap_delta_bytes / (double) K);
    }
    return;
  } 

  // No expansion, now see if we want to shrink
  size_t shrink_bytes = 0;
  // We would never want to shrink more than this
  size_t max_shrink_bytes = capacity_after_gc - minimum_desired_capacity;

  if (MaxHeapFreeRatio < 100) {
    const double maximum_free_percentage = MaxHeapFreeRatio / 100.0;
    const double minimum_used_percentage = 1.0 - maximum_free_percentage;
    const double max_tmp = used_after_gc / minimum_used_percentage;
    size_t maximum_desired_capacity = MIN2(max_tmp, double(max_uintx));
    maximum_desired_capacity = MAX2(maximum_desired_capacity,
				    spec()->init_size());
    if (PrintGC && Verbose) {
      gclog_or_tty->print_cr("  "
			     "  maximum_free_percentage: %6.2f"
			     "  minimum_used_percentage: %6.2f",
			     maximum_free_percentage,
			     minimum_used_percentage);
      gclog_or_tty->print_cr("  "
			     "  _capacity_at_prologue: %6.1fK"
			     "  minimum_desired_capacity: %6.1fK"
			     "  maximum_desired_capacity: %6.1fK",
			     _capacity_at_prologue / (double) K,
			     minimum_desired_capacity / (double) K,
			     maximum_desired_capacity / (double) K);
    }
    assert(minimum_desired_capacity <= maximum_desired_capacity,
	   "sanity check");

    if (capacity_after_gc > maximum_desired_capacity) {
      // Capacity too large, compute shrinking size
      shrink_bytes = capacity_after_gc - maximum_desired_capacity;
      // We don't want shrink all the way back to initSize if people call
      // System.gc(), because some programs do that between "phases" and then
      // we'd just have to grow the heap up again for the next phase.  So we
      // damp the shrinking: 0% on the first call, 10% on the second call, 40%
      // on the third call, and 100% by the fourth call.  But if we recompute
      // size without shrinking, it goes back to 0%.
      shrink_bytes = shrink_bytes / 100 * current_shrink_factor;
      assert(shrink_bytes <= max_shrink_bytes, "invalid shrink size");
      if (current_shrink_factor == 0) {
        _shrink_factor = 10;
      } else {
        _shrink_factor = MIN2(current_shrink_factor * 4, (size_t) 100);
      }
      if (PrintGC && Verbose) {
        gclog_or_tty->print_cr("  "
                      "  shrinking:"
                      "  initSize: %.1fK"
                      "  maximum_desired_capacity: %.1fK",
                      spec()->init_size() / (double) K,
                      maximum_desired_capacity / (double) K);
        gclog_or_tty->print_cr("  "
                      "  shrink_bytes: %.1fK"
                      "  current_shrink_factor: %d"
                      "  new shrink factor: %d"
                      "  _min_heap_delta_bytes: %.1fK",
                      shrink_bytes / (double) K,
                      current_shrink_factor,
                      _shrink_factor,
                      _min_heap_delta_bytes / (double) K);
      }
    }
  }

  if (capacity_after_gc > _capacity_at_prologue) {
    // We might have expanded for promotions, in which case we might want to
    // take back that expansion if there's room after GC.  That keeps us from
    // stretching the heap with promotions when there's plenty of room.
    size_t expansion_for_promotion = capacity_after_gc - _capacity_at_prologue;
    expansion_for_promotion = MIN2(expansion_for_promotion, max_shrink_bytes);
    // We have two shrinking computations, take the largest
    shrink_bytes = MAX2(shrink_bytes, expansion_for_promotion);
    assert(shrink_bytes <= max_shrink_bytes, "invalid shrink size");
    if (PrintGC && Verbose) {
      gclog_or_tty->print_cr("  "
			     "  aggressive shrinking:"
			     "  _capacity_at_prologue: %.1fK"
			     "  capacity_after_gc: %.1fK"
			     "  expansion_for_promotion: %.1fK"
			     "  shrink_bytes: %.1fK",
			     capacity_after_gc / (double) K,
			     _capacity_at_prologue / (double) K,
			     expansion_for_promotion / (double) K,
			     shrink_bytes / (double) K);
    }
  }
  // Don't shrink unless it's significant
  if (shrink_bytes >= _min_heap_delta_bytes) {
    shrink(shrink_bytes);
  }
  assert(used() == used_after_gc && used_after_gc <= capacity(),
	 "sanity check");
}

void TenuredGeneration::gc_prologue(bool full) {
  _capacity_at_prologue = capacity();
  _used_at_prologue = used();
  verify_alloc_buffers_clean();
}

void TenuredGeneration::gc_epilogue(bool full) {
  verify_alloc_buffers_clean();
  OneContigSpaceCardGeneration::gc_epilogue(full);
}


bool TenuredGeneration::should_collect(bool  full,
                                       size_t size,
                                       bool   is_large_noref,
                                       bool   is_tlab) {
  // This should be one big conditional or (||), but I want to be able to tell 
  // why it returns what it returns (without re-evaluating the conditionals 
  // in case they aren't idempotent), so I'm doing it this way.  
  // DeMorgan says it's okay.
  bool result = false;
  if (!result && full) {
    result = true;
    if (PrintGC && Verbose) {
      gclog_or_tty->print_cr("TenuredGeneration::should_collect: because"
                    " full");
    }
  }
  if (!result && should_allocate(size, is_large_noref, is_tlab)) {
    result = true;
    if (PrintGC && Verbose) {
      gclog_or_tty->print_cr("TenuredGeneration::should_collect: because"
                    " should_allocate(" SIZE_FORMAT ")",
                    size);
    }
  }
  // If we don't have very much free space.
  // XXX: 10000 should be a percentage of the capacity!!!
  if (!result && free() < 10000) {
    result = true;
    if (PrintGC && Verbose) {
      gclog_or_tty->print_cr("TenuredGeneration::should_collect: because"
                    " free(): " SIZE_FORMAT,
                    free());
    }
  }
  // If we had to expand to accomodate promotions from younger generations
  if (!result && _capacity_at_prologue < capacity()) {
    result = true;
    if (PrintGC && Verbose) {
      gclog_or_tty->print_cr("TenuredGeneration::should_collect: because"
                    "_capacity_at_prologue: " SIZE_FORMAT " < capacity(): " SIZE_FORMAT,
                    _capacity_at_prologue, capacity());
    }
  }
  return result;
}

void TenuredGeneration::collect(bool   full,
				bool   clear_all_soft_refs,
				size_t size,
				bool   is_large_noref,
				bool   is_tlab) {
  retire_alloc_buffers_before_full_gc();  
  OneContigSpaceCardGeneration::collect(full, clear_all_soft_refs,
					size, is_large_noref,is_tlab);
}


oop TenuredGeneration::par_promote(int thread_num,
				   oop old, markOop m, size_t word_sz) {

  ParGCAllocBufferWithBOT* buf = _alloc_buffers[thread_num];
  HeapWord* obj_ptr = buf->allocate(word_sz);
  bool is_lab = true;
  if (obj_ptr == NULL) {
    NOT_PRODUCT(
       if (PromotionFailureALot && buf->_promotion_failure_a_lot_count-- > 0) {
         return NULL;
       }
    )
    // Slow path:
    if (word_sz * 100 < ParallelGCBufferWastePct * buf->word_sz()) {
      // Is small enough; abandon this buffer and start a new one.
      size_t buf_size = buf->word_sz();
      HeapWord* buf_space =
	TenuredGeneration::par_allocate(buf_size, false, false);
      if (buf_space == NULL) {
	buf_space = expand_and_allocate(buf_size, false, false,
					true /* parallel*/);
      }
      if (buf_space != NULL) {
	buf->retire(false, false);
	buf->set_buf(buf_space);
	obj_ptr = buf->allocate(word_sz);
	assert(obj_ptr != NULL, "Buffer was definitely big enough...");
      }
    };
    // Otherwise, buffer allocation failed; try allocating object
    // individually.
    if (obj_ptr == NULL) {
      obj_ptr = TenuredGeneration::par_allocate(word_sz, false, false);
      if (obj_ptr == NULL) {
	obj_ptr = expand_and_allocate(word_sz, false, false,
				      true /* parallel */);
      }
    }
    if (obj_ptr == NULL) return NULL;
  }
  assert(obj_ptr != NULL, "program logic");
  Copy::aligned_disjoint_words((HeapWord*)old, obj_ptr, word_sz);
  oop obj = oop(obj_ptr);
  // Restore the mark word copied above.
  obj->set_mark(m);
  return obj;
}

void TenuredGeneration::par_promote_alloc_undo(int thread_num,
					       HeapWord* obj,
					       size_t word_sz) {
  ParGCAllocBufferWithBOT* buf = _alloc_buffers[thread_num];
  if (buf->contains(obj)) {
    guarantee(buf->contains(obj + word_sz - 1),
	      "should contain whole object");
    buf->undo_allocation(obj, word_sz);
  } else {
    SharedHeap::fill_region_with_object(MemRegion(obj, word_sz));
  }
}

void TenuredGeneration::par_promote_alloc_done(int thread_num) {
  ParGCAllocBufferWithBOT* buf = _alloc_buffers[thread_num];
  buf->retire(true, ParallelGCRetainPLAB);
}

void TenuredGeneration::update_gc_stats(int current_level, bool full) {
  // If the next lower level(s) has been collected, gather any statistics
  // that are of interest at this point.
  if (!full && (current_level + 1) == level()) {
    // Calculate size of data promoted from the younger generations
    // before doing the collection.
    size_t used_before_gc = used();

    // If the younger gen collections were skipped, then the
    // number of promoted bytes will be 0 and adding it to the
    // average will incorrectly lessen the average.  It is, however,
    // also possible that no promotion was needed.
    if (used_before_gc >= _used_at_prologue) {
      size_t promoted_in_bytes = used_before_gc - _used_at_prologue;
      gc_stats().avg_promoted()->sample(promoted_in_bytes);
    }
  }
}

void TenuredGeneration::update_counters() {
  if (UsePerfData) {
    _space_counters->update_all();
    _gen_counters->update_all();
  }
}


void TenuredGeneration::retire_alloc_buffers_before_full_gc() {
  if (UseParNewGC) {
    for (uint i = 0; i < ParallelGCThreads; i++) {
      _alloc_buffers[i]->retire(true /*end_of_gc*/, false /*retain*/);
    }
  }
}

#ifndef PRODUCT
// Verify that any retained parallel allocation buffers do not
// intersect with dirty cards.
void TenuredGeneration::verify_alloc_buffers_clean() {
  if (UseParNewGC) {
    for (uint i = 0; i < ParallelGCThreads; i++) {
      _rs->verify_empty(_alloc_buffers[i]->range());
    }
  }
}
#endif

bool TenuredGeneration::promotion_attempt_is_safe(
    size_t max_promotion_in_bytes,
    bool younger_handles_promotion_failure) const {

  bool result = max_contiguous_available() >= max_promotion_in_bytes;

  if (younger_handles_promotion_failure && !result) {
    result = max_contiguous_available() >= 
      (size_t) gc_stats().avg_promoted()->padded_average();
    if (PrintGC && Verbose && result) {
      gclog_or_tty->print_cr("TenuredGeneration::promotion_attempt_is_safe"
                  " contiguous_available: " SIZE_FORMAT
                  " avg_promoted: " SIZE_FORMAT,
                  max_contiguous_available(), 
		  gc_stats().avg_promoted()->padded_average());
    }
  } else {
    if (PrintGC && Verbose) {
      gclog_or_tty->print_cr("TenuredGeneration::promotion_attempt_is_safe"
                  " contiguous_available: " SIZE_FORMAT
                  " promotion_in_bytes: " SIZE_FORMAT,
                  max_contiguous_available(), max_promotion_in_bytes);
    }
  }
  return result;
}

ParGCAllocBufferWithBOT::ParGCAllocBufferWithBOT(size_t word_sz,
						 BlockOffsetSharedArray* bsa) :
  ParGCAllocBuffer(word_sz),
  _bsa(bsa),
  _bt(bsa, MemRegion(_bottom, _hard_end)),
  _true_end(_hard_end)
{}

void ParGCAllocBufferWithBOT::fill_region_with_block(MemRegion mr,
						     bool contig) {
  SharedHeap::fill_region_with_object(mr);
  if (contig) {
    _bt.alloc_block(mr.start(), mr.end());
  } else {
    _bt.BlockOffsetArray::alloc_block(mr.start(), mr.end());
  }
}

HeapWord* ParGCAllocBufferWithBOT::allocate_slow(size_t word_sz) {
  HeapWord* res = NULL;
  if (_true_end > _hard_end) {
    assert((HeapWord*)align_size_down(intptr_t(_hard_end),
				      ChunkSizeInBytes) == _hard_end,
	   "or else _true_end should be equal to _hard_end");
    assert(_retained, "or else _true_end should be equal to _hard_end");
    assert(_retained_filler.end() <= _top, "INVARIANT");
    SharedHeap::fill_region_with_object(_retained_filler);
    if (_top < _hard_end) {
      fill_region_with_block(MemRegion(_top, _hard_end), true);
    }
    HeapWord* next_hard_end = MIN2(_true_end, _hard_end + ChunkSizeInWords);
    _retained_filler = MemRegion(_hard_end, FillerHeaderSize);
    _bt.alloc_block(_retained_filler.start(), _retained_filler.word_size());
    _top      = _retained_filler.end();
    _hard_end = next_hard_end;
    _end      = _hard_end - AlignmentReserve;
    res       = ParGCAllocBuffer::allocate(word_sz);
    if (res != NULL) {
      _bt.alloc_block(res, word_sz);
    }
  }
  return res;
}

void
ParGCAllocBufferWithBOT::undo_allocation(HeapWord* obj, size_t word_sz) {
  ParGCAllocBuffer::undo_allocation(obj, word_sz);
  // This may back us up beyond the previous threshold, so reset.
  _bt.set_region(MemRegion(_top, _hard_end));
  _bt.initialize_threshold();
}

void ParGCAllocBufferWithBOT::retire(bool end_of_gc, bool retain) {
  assert(!retain || end_of_gc, "Can only retain at GC end.");
  if (_retained) {
    // We're about to make the retained_filler into a block.
    _bt.BlockOffsetArray::alloc_block(_retained_filler.start(),
				      _retained_filler.end());
  }
  // Reset _hard_end to _true_end (and update _end)
  if (retain && _hard_end != NULL) {
    assert(_hard_end <= _true_end, "Invariant.");
    _hard_end = _true_end;
    _end      = MAX2(_top, _hard_end - AlignmentReserve);
    assert(_end <= _hard_end, "Invariant.");
  }
  _true_end = _hard_end;
  HeapWord* pre_top = _top;

  ParGCAllocBuffer::retire(end_of_gc, retain);
  // Now any old _retained_filler is cut back to size, the free part is
  // filled with a filler object, and top is past the header of that
  // object.

  if (retain && _top < _end) {
    assert(end_of_gc && retain, "Or else retain should be false.");
    // If the lab does not start on a card boundary, we don't want to
    // allocate onto that card, since that might lead to concurrent
    // allocation and card scanning, which we don't support.  So we fill
    // the first card with a garbage object.
    size_t first_card_index = _bsa->index_for(pre_top);
    HeapWord* first_card_start = _bsa->address_for_index(first_card_index);
    if (first_card_start < pre_top) {
      HeapWord* second_card_start =
	_bsa->address_for_index(first_card_index + 1);

      // Ensure enough room to fill with the smallest block
      second_card_start = MAX2(second_card_start, pre_top + AlignmentReserve);

      // If the end is already in the first card, don't go beyond it!
      // Or if the remainder is too small for a filler object, gobble it up.
      if (_hard_end < second_card_start ||
          pointer_delta(_hard_end, second_card_start) < AlignmentReserve) {
        second_card_start = _hard_end;
      }
      if (pre_top < second_card_start) {
	MemRegion first_card_suffix(pre_top, second_card_start);
	fill_region_with_block(first_card_suffix, true);
      }
      pre_top = second_card_start;
      _top = pre_top;
      _end = MAX2(_top, _hard_end - AlignmentReserve);
    }

    // If the lab does not end on a card boundary, we don't want to
    // allocate onto that card, since that might lead to concurrent
    // allocation and card scanning, which we don't support.  So we fill
    // the last card with a garbage object.
    size_t last_card_index = _bsa->index_for(_hard_end);
    HeapWord* last_card_start = _bsa->address_for_index(last_card_index);
    if (last_card_start < _hard_end) {

      // Ensure enough room to fill with the smallest block
      last_card_start = MIN2(last_card_start, _hard_end - AlignmentReserve);

      // If the top is already in the last card, don't go back beyond it!
      // Or if the remainder is too small for a filler object, gobble it up.
      if (_top > last_card_start ||
          pointer_delta(last_card_start, _top) < AlignmentReserve) {
        last_card_start = _top;
      }
      if (last_card_start < _hard_end) {
	MemRegion last_card_prefix(last_card_start, _hard_end);
	fill_region_with_block(last_card_prefix, false);
      }
      _hard_end = last_card_start;
      _end      = MAX2(_top, _hard_end - AlignmentReserve);
      _true_end = _hard_end;
      assert(_end <= _hard_end, "Invariant.");
    }

    // At this point:
    //   1) we had a filler object from the original top to hard_end.
    //   2) We've filled in any partial cards at the front and back.
    if (pre_top < _hard_end) {
      // Now we can reset the _bt to do allocation in the given area.
      MemRegion new_filler(pre_top, _hard_end);
      fill_region_with_block(new_filler, false);
      _top = pre_top + ParGCAllocBuffer::FillerHeaderSize;
      // If there's no space left, don't retain.
      if (_top >= _end) {
	_retained = false;
        invalidate();
	return;
      } 
      _retained_filler = MemRegion(pre_top, _top);
      _bt.set_region(MemRegion(_top, _hard_end));
      _bt.initialize_threshold();
      assert(_bt.threshold() > _top, "initialize_threshold failed!");

      // There may be other reasons for queries into the middle of the
      // filler object.  When such queries are done in parallel with
      // allocation, bad things can happen, if the query involves object
      // iteration.  So we ensure that such queries do not involve object
      // iteration, by putting another filler object on the boundaries of
      // such queries.  One such is the object spanning a parallel card
      // chunk boundary.

      // "chunk_boundary" is the address of the first chunk boundary less
      // than "hard_end".
      HeapWord* chunk_boundary = 
        (HeapWord*)align_size_down(intptr_t(_hard_end-1), ChunkSizeInBytes); 
      assert(chunk_boundary < _hard_end, "Or else above did not work."); 
      assert(pointer_delta(_true_end, chunk_boundary) >= AlignmentReserve,
             "Consequence of last card handling above.");
 
      if (_top <= chunk_boundary) { 
	assert(_true_end == _hard_end, "Invariant.");
	while (_top <= chunk_boundary) {
          assert(pointer_delta(_hard_end, chunk_boundary) >= AlignmentReserve,
                 "Consequence of last card handling above.");
	  MemRegion chunk_portion(chunk_boundary, _hard_end);
	  _bt.BlockOffsetArray::alloc_block(chunk_portion.start(),
                                            chunk_portion.end());
	  SharedHeap::fill_region_with_object(chunk_portion);
	  _hard_end = chunk_portion.start();
	  chunk_boundary -= ChunkSizeInWords;
	}
        _end = _hard_end - AlignmentReserve;
        assert(_top <= _end, "Invariant.");
	// Now reset the initial filler chunk so it doesn't overlap with
	// the one(s) inserted above.
	MemRegion new_filler(pre_top, _hard_end);
	fill_region_with_block(new_filler, false);
      }
    } else {
      _retained = false;
      invalidate();
    }
  } else {
    assert(!end_of_gc ||
	   (!_retained && _true_end == _hard_end), "Checking.");
  }
  assert(_end <= _hard_end, "Invariant.");
  assert(_top < _end || _top == _hard_end, "Invariant");
}
