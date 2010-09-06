#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)blockOffsetTable.cpp	1.48 03/12/23 16:40:53 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

# include "incls/_precompiled.incl"
# include "incls/_blockOffsetTable.cpp.incl"

//////////////////////////////////////////////////////////////////////
// BlockOffsetSharedArray
//////////////////////////////////////////////////////////////////////

BlockOffsetSharedArray::BlockOffsetSharedArray(MemRegion reserved,
					       size_t init_word_size):
  _reserved(reserved), _end(NULL)
{
  size_t size = compute_size(reserved.word_size());
  ReservedSpace rs(size);
  if (!rs.is_reserved()) {
    vm_exit_during_initialization("Could not reserve enough space for heap offset array");
  }
  if (!_vs.initialize(rs, 0)) {
    vm_exit_during_initialization("Could not reserve enough space for heap offset array");
  }
  _offset_array = (u_char*)_vs.low_boundary();
  resize(init_word_size);
  if (TraceBlockOffsetTable) {
    gclog_or_tty->print_cr("BlockOffsetSharedArray::BlockOffsetSharedArray: ");
    gclog_or_tty->print_cr("  "
                  "  rs.base(): " INTPTR_FORMAT
                  "  rs.size(): " INTPTR_FORMAT
                  "  rs end(): " INTPTR_FORMAT,
                  rs.base(), rs.size(), rs.base() + rs.size());
    gclog_or_tty->print_cr("  "
                  "  _vs.low_boundary(): " INTPTR_FORMAT
                  "  _vs.high_boundary(): " INTPTR_FORMAT,
                  _vs.low_boundary(), 
                  _vs.high_boundary());
  }
}

void BlockOffsetSharedArray::resize(size_t new_word_size) {
  assert(new_word_size <= _reserved.word_size(), "Resize larger than reserved");
  size_t new_size = compute_size(new_word_size);
  size_t old_size = _vs.committed_size();
  size_t delta;
  char* high = _vs.high();
  _end = _reserved.start() + new_word_size;
  if (new_size > old_size) {
    delta = ReservedSpace::page_align_size_up(new_size - old_size);
    assert(delta > 0, "just checking");
    if (!_vs.expand_by(delta)) {
      // Do better than this for Merlin
      vm_exit_out_of_memory(delta, "offset table expansion");
    }
    assert(_vs.high() == high + delta, "invalid expansion");
  } else {
    delta = ReservedSpace::page_align_size_down(old_size - new_size);
    if (delta == 0) return;
    _vs.shrink_by(delta);
    assert(_vs.high() == high - delta, "invalid expansion");
  }
}

bool BlockOffsetSharedArray::is_card_boundary(HeapWord* p) const {
  assert(p >= _reserved.start(), "just checking");
  size_t delta = pointer_delta(p, _reserved.start());
  return (delta & right_n_bits(LogN_words)) == NoBits;
}


void BlockOffsetSharedArray::serialize(SerializeOopClosure* soc,
                                       HeapWord* start, HeapWord* end) {
  assert(_offset_array[0] == 0, "objects can't cross covered areas");
  assert(start <= end, "bad address range");
  size_t start_index = index_for(start);
  size_t end_index = index_for(end-1)+1;
  soc->do_region(&_offset_array[start_index],
                 (end_index - start_index) * sizeof(_offset_array[0]));
}


//////////////////////////////////////////////////////////////////////
// BlockOffsetArray
//////////////////////////////////////////////////////////////////////

BlockOffsetArray::BlockOffsetArray(BlockOffsetSharedArray* array,
				   MemRegion mr, bool init_to_zero) :
  BlockOffsetTable(mr.start(), mr.end()),
  _array(array),
  _init_to_zero(init_to_zero)
{
  assert(_bottom <= _end, "arguments out of order");
  if (!_init_to_zero) {
    // initialize cards to point back to mr.start()
    set_aligned_region_to_point_to_start(mr.start() + N_words, mr.end());
    _array->set_offset_array(0, 0);  // set first card to 0
  }
}

void
BlockOffsetArray::
set_aligned_region_to_point_to_start(HeapWord* start, HeapWord* end) {
  if (start >= end) {
    // object doesn't span multiple cards
    return;
  }
#ifdef ASSERT
  size_t card1 = _array->index_for(start);
  size_t card2 = _array->index_for(end-1);
#endif
  assert(start ==_array->address_for_index(card1), "Precondition");
  assert(end ==_array->address_for_index(card2)+N_words, "Precondition");
  // Initialize the whole region to the maximum backskip.
  _array->set_offset_array(start, end, N_words+N_powers-1);
  // Now, for each smaller power, fix up the appropriate prefix.
  for (int i = N_powers-2; i >= 0; i--) {
    HeapWord* reach = start + (power_to_cards_back(i+1) - 1) * N_words;
    _array->set_offset_array(start, MIN2(end, reach), N_words+i);
  }
#define DEBUG_SET_ALIGNED_REGION 0
#if DEBUG_SET_ALIGNED_REGION
  size_t targ_card = _array->index_for(start) - 1;
  guarantee(targ_card >= 0, "Inv");
  for (HeapWord* c = start; c < end; c += N_words) {
    size_t card_ind = _array->index_for(c);
    guarantee(card_ind -
	      entry_to_cards_back(_array->offset_array(card_ind))
	      >= targ_card, "Inv");
  }
#endif  
}

void
BlockOffsetArray::
fix_up_alloced_region(size_t start_card, size_t end_card) {
  if (start_card > end_card) return;
  // Otherwise, fix up.
  for (uint i = 1; i < N_powers; i++) {
    size_t back_by = power_to_cards_back(i);
    size_t range = start_card + back_by - 1;
    range = MIN2(range, end_card);
    while (range >= start_card) {
      uint offset = _array->offset_array(range);
      assert(offset >= (uint)N_words, "or else is not a multi-card obj.");
      if ((offset - N_words) >= i) {
	// points back too far; fix to make it point less far.
	_array->set_offset_array(range, N_words + i - 1);
	range--;
      } else {
	break;
      }
    }
  }
  assert(_array->offset_array(start_card) == N_words, "Inv");
}

void
BlockOffsetArray::alloc_block(HeapWord* blk_start, HeapWord* blk_end) {
  assert(blk_start != NULL && blk_end > blk_start,
         "phantom block");
  single_block(blk_start, blk_end);
}

void
BlockOffsetArray::do_block_internal(HeapWord* blk_start,
                                    HeapWord* blk_end,
                                    Action action) {
  assert(Universe::heap()->is_in_reserved(blk_start),
         "reference must be into the heap");
  assert(Universe::heap()->is_in_reserved(blk_end-1),
         "limit must be within the heap");
  // This is optimized to make the test fast, assuming we only rarely
  // cross boundaries.
  uintptr_t end_ui = (uintptr_t)(blk_end - 1);
  uintptr_t start_ui = (uintptr_t)blk_start;
  // Calculate the last card boundary preceding end of blk
  intptr_t boundary_before_end = (intptr_t)end_ui;
  clear_bits(boundary_before_end, right_n_bits(LogN));
  if (start_ui <= (uintptr_t)boundary_before_end) {
    // blk starts at or crosses a boundary
    // Calculate index of card on which blk begins
    size_t    start_index = _array->index_for(blk_start);
    // Index of card on which blk ends
    size_t    end_index   = _array->index_for(blk_end - 1);
    // Start address of card on which blk begins
    HeapWord* boundary    = _array->address_for_index(start_index);
    assert(boundary <= blk_start, "blk should start at or after boundary");
    if (blk_start != boundary) {
      // blk starts strictly after boundary
      // adjust card boundary and start_index forward to next card
      boundary += N_words;
      start_index++;
    }
    assert(start_index <= end_index, "monotonicity of index_for()");
    assert(boundary <= (HeapWord*)boundary_before_end, "tautology");
    switch (action) {
      case Action_single: {
        _array->set_offset_array(start_index, boundary, blk_start);
        // We have finished marking the "offset card". We need to now
        // mark the subsequent cards that this blk spans.
        if (start_index < end_index) {
	  HeapWord* rem_st = _array->address_for_index(start_index) + N_words;
	  HeapWord* rem_end = _array->address_for_index(end_index) + N_words;
	  set_aligned_region_to_point_to_start(rem_st, rem_end);
	}
        break;
      }
      case Action_mark: {
	if (!init_to_zero()) {
	  fix_up_alloced_region(start_index+1, end_index);
	}
        _array->set_offset_array(start_index, boundary, blk_start);
        break;
      }
      case Action_check: {
        _array->check_offset_array(start_index, boundary, blk_start);
        // We have finished checking the "offset card". We need to now
        // check the subsequent cards that this blk spans.
        for (size_t j = start_index+1; j <= end_index; j++) {
          guarantee(_array->offset_array(j) >= N_words,
                    "Incorrect offset value in non-first card");
          guarantee((j - entry_to_cards_back(_array->offset_array(j)))
                    >= start_index,
                    "Incorrect offset value in non-first card");
        }
        break;
      }
    }
  }
}

// The range [blk_start, blk_end) represents a single contiguous block
// of storage; modify the block offset table to represent this
// information; Right-open interval: [blk_start, blk_end)
// NOTE: this method does _not_ adjust _unallocated_block.
void
BlockOffsetArray::single_block(HeapWord* blk_start,
                               HeapWord* blk_end) {
  do_block_internal(blk_start, blk_end, Action_single);
}


//////////////////////////////////////////////////////////////////////
// BlockOffsetArrayNonContigSpace
//////////////////////////////////////////////////////////////////////

// The block [blk_start, blk_end) has been allocated;
// adjust the block offset table to represent this information;
// NOTE: Clients of BlockOffsetArrayNonContigSpace: consider using
// the somewhat more lightweight mark_block() (potentially followed
// by mark_block()) wherever possible.
// right-open interval: [blk_start, blk_end)
void
BlockOffsetArrayNonContigSpace::alloc_block(HeapWord* blk_start,
                                            HeapWord* blk_end) {
  assert(blk_start != NULL && blk_end > blk_start,
         "phantom block");
  single_block(blk_start, blk_end);
  allocated(blk_start, blk_end);
}

// Adjust BOT to show that a previously whole block has been split
// into two.
void BlockOffsetArrayNonContigSpace::split_block(HeapWord* blk,
                                                 size_t blk_size,
				                 size_t left_blk_size) {
  // Verify that the BOT shows [blk, blk + blk_size) to be one block.
  verify_single_block(blk, blk_size);
  // Update the BOT to indicate that [blk + left_blk_size, blk + blk_size)
  // is one single block.
  mark_block(blk + left_blk_size, blk + blk_size);
}

// Mark the BOT such that if [blk_start, blk_end) straddles a card
// boundary, the card following the first such boundary is marked
// with the appropriate offset.
// NOTE: this method does _not_ adjust _unallocated_block or
// any cards subsequent to the first one.
void
BlockOffsetArrayNonContigSpace::mark_block(HeapWord* blk_start,
                                           HeapWord* blk_end) {
  do_block_internal(blk_start, blk_end, Action_mark);
}

HeapWord* BlockOffsetArrayNonContigSpace::block_start_unsafe(
  const void* addr) const {
  assert(_array->offset_array(0) == 0, "objects can't cross covered areas");

  assert(_bottom <= addr && addr < _end,
         "addr must be covered by this Array");
  // Must read this exactly once because it can be modified by parallel
  // allocation.
  HeapWord* ub = _unallocated_block;
  if (BlockOffsetArrayUseUnallocatedBlock && addr >= ub) {
    assert(ub < _end, "tautology (see above)");
    return ub;
  }

  // Otherwise, find the block start using the table.
  size_t index = _array->index_for(addr);
  HeapWord* q = _array->address_for_index(index);

  uint offset = _array->offset_array(index);	// Extend u_char to uint.
  while (offset >= N_words) {
    // The excess of the offset from N_words indicates a power of Base
    // to go back by.
    int n_cards_back = entry_to_cards_back(offset);
    q -= (N_words * n_cards_back);
    assert(q >= _sp->bottom(), "Went below bottom!");
    index -= n_cards_back;
    offset = _array->offset_array(index);
  }
  index--;
  q -= offset;
  HeapWord* n = q;

  while (n <= addr) {
    debug_only(HeapWord* last = q);   // for debugging
    q = n;
    n += _sp->block_size(n);
  }
  assert(q <= addr, "wrong order for current and arg");
  assert(addr <= n, "wrong order for arg and next");
  return q;
}

HeapWord* BlockOffsetArrayNonContigSpace::block_start_careful(
  const void* addr) const {
  assert(_array->offset_array(0) == 0, "objects can't cross covered areas");

  assert(_bottom <= addr && addr < _end,
         "addr must be covered by this Array");
  // Must read this exactly once because it can be modified by parallel
  // allocation.
  HeapWord* ub = _unallocated_block;
  if (BlockOffsetArrayUseUnallocatedBlock && addr >= ub) {
    assert(ub < _end, "tautology (see above)");
    return ub;
  }

  // Otherwise, find the block start using the table, but taking
  // care (cf block_start_unsafe() above) not to parse any objects/blocks
  // on the cards themsleves.
  size_t index = _array->index_for(addr);
  assert(_array->address_for_index(index) == addr,
         "arg should be start of card");

  HeapWord* q = (HeapWord*)addr;
  uint offset;
  do {
    offset = _array->offset_array(index);
    if (offset < N_words) {
      q -= offset;
    } else {
      int n_cards_back = entry_to_cards_back(offset);
      q -= (n_cards_back * N_words);
      index -= n_cards_back;
    }
  } while (offset >= N_words);
  assert(q <= addr, "block start should be to left of arg");
  return q;
}

//////////////////////////////////////////////////////////////////////
// BlockOffsetArrayContigSpace
//////////////////////////////////////////////////////////////////////

HeapWord* BlockOffsetArrayContigSpace::block_start_unsafe(const void* addr) const {
  assert(_array->offset_array(0) == 0, "objects can't cross covered areas");

  // Otherwise, find the block start using the table.
  assert(_bottom <= addr && addr < _end,
	 "addr must be covered by this Array");
  size_t index = _array->index_for(addr);
  // We must make sure that the offset table entry we use is valid.  If
  // "addr" is past the end, start at the last known one and go forward.
  index = MIN2(index, _next_offset_index-1);
  HeapWord* q = _array->address_for_index(index);

  uint offset = _array->offset_array(index--);	// Extend u_char to uint.
  while (offset >= N_words) {
    assert(offset == N_words, "Contig spaces don't use Base encoding.");
    q -= N_words;
    offset = _array->offset_array(index--);
  }
  q -= offset;
  HeapWord* n = q;

  while (n <= addr) {
    q = n;
    n += _sp->block_size(n);
  }
  assert(q <= addr, "wrong order for current and arg");
  assert(addr <= n, "wrong order for arg and next");
  return q;
}

void BlockOffsetArrayContigSpace::alloc_block_work(HeapWord* blk_start,
					HeapWord* blk_end) {
  assert(blk_start != NULL && blk_end > blk_start,
         "phantom block");
  assert(blk_end > _next_offset_threshold,
	 "should be past threshold");
  assert(blk_start <= _next_offset_threshold,
	 "blk_start should be at or before threshold")
  assert(pointer_delta(_next_offset_threshold, blk_start) <= N_words,
	 "offset should be <= BlockOffsetSharedArray::N");
  assert(Universe::heap()->is_in_reserved(blk_start),
	 "reference must be into the heap");
  assert(Universe::heap()->is_in_reserved(blk_end-1),
	 "limit must be within the heap");
  assert(_next_offset_threshold ==
	 _array->_reserved.start() + _next_offset_index*N_words,
	 "index must agree with threshold");
  _array->set_offset_array(_next_offset_index++,
                           _next_offset_threshold,
                           blk_start);
  _next_offset_threshold += N_words; // update threshold

  while (_next_offset_threshold < blk_end) {
    // fill in sentinel values if object spans more than one card
    _array->set_offset_array(_next_offset_index++, N_words);
    _next_offset_threshold += N_words;
  }
  // If the current allocation ends a card, then the next offset
  // will definitely be zero.
  if (_next_offset_threshold == blk_end) {
    _array->set_offset_array(_next_offset_index++, 0);
    _next_offset_threshold += N_words;
  }
}

HeapWord* BlockOffsetArrayContigSpace::initialize_threshold() {
  assert(!Universe::heap()->is_in_reserved(_array->_offset_array),
         "just checking");
  _next_offset_index = _array->index_for(_bottom);
  _next_offset_index++;
  _next_offset_threshold =
    _array->address_for_index(_next_offset_index);
  return _next_offset_threshold;
}

void BlockOffsetArrayContigSpace::zero_bottom_entry() {
  assert(!Universe::heap()->is_in_reserved(_array->_offset_array),
         "just checking");
  size_t bottom_index = _array->index_for(_bottom);
  _array->set_offset_array(bottom_index, 0);
}


void BlockOffsetArrayContigSpace::serialize(SerializeOopClosure* soc) {
  if (soc->reading()) {
    // Null these values so that the serializer won't object to updating them.
    _next_offset_threshold = NULL;
    _next_offset_index = 0;
  }
  soc->do_ptr(&_next_offset_threshold);
  soc->do_size_t(&_next_offset_index);
}
