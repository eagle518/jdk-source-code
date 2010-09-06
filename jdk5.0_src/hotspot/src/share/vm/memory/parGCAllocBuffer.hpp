#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)parGCAllocBuffer.hpp	1.15 03/12/23 16:41:22 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// A per-thread allocation buffer used during GC.
class ParGCAllocBuffer: public CHeapObj {
protected:
  char head[32];
  size_t _word_sz;
  HeapWord* _bottom;
  HeapWord* _top;
  HeapWord* _end;       // last allocatable address + 1
  HeapWord* _hard_end;  // _end + AlignmentReserve
  bool      _retained;
  MemRegion _retained_filler;
  char tail[32];
  static const size_t FillerHeaderSize;
  static const size_t AlignmentReserve;

public:
  // Initializes the buffer to be empty, but with the given "word_sz".
  // Must get initialized with "set_space" for an allocation to succeed.
  ParGCAllocBuffer(size_t word_sz);

  // If an allocation of the given "word_sz" can be satisfied within the
  // buffer, do the allocation, returning a pointer to the start of the
  // allocated block.  If the allocation request cannot be satisfied,
  // return NULL.
  HeapWord* allocate(size_t word_sz) {
    HeapWord* res = _top;
    HeapWord* new_top = _top + word_sz;
    if (new_top <= _end) {
      _top = new_top;
      return res;
    } else {
      return NULL;
    }
  }

  // Undo the last allocation in the buffer, which is required to be of the 
  // "obj" of the given "word_sz".
  void undo_allocation(HeapWord* obj, size_t word_sz) {
    assert(_top - word_sz >= _bottom
	   && _top - word_sz == obj,
	   "Bad undo_allocation");
    _top = _top - word_sz;
  }

  // The total (word) size of the buffer, including both allocated and
  // unallocted space.
  size_t word_sz() { return _word_sz; }

  // Should only be done if we are about to reset with a new buffer of the
  // given size.
  void set_word_size(size_t new_word_sz) { _word_sz = new_word_sz; }

  // The number of words of unallocated space remaining in the buffer.
  size_t words_remaining() {
    return pointer_delta(_end, _top, HeapWordSize);
  }

  bool contains(void* addr) {
    return (void*)_bottom <= addr && addr < (void*)_hard_end;
  }

  // Sets the space of the buffer to be [buf, space+word_sz()).
  void set_buf(HeapWord* buf) {
    _bottom   = buf;
    _top      = _bottom;
    _hard_end = _bottom + word_sz();
    _end      = _hard_end - AlignmentReserve;
    NOT_PRODUCT(_promotion_failure_a_lot_count = PromotionFailureALotCount);
  }

  // Force future allocations to fail and queries for contains()
  // to return false
  void invalidate() {
    assert(!_retained, "Shouldn't retain an invalidated buffer.");
    _end    = _hard_end;
    _top    = _end;      // force future allocations to fail
    _bottom = _end;      // force future contains() queries to return false
  }

  // Fills in the unallocated portion of the buffer with a garbage object.
  // If "end_of_gc" is TRUE, is after the last use in the GC.  IF "retain"
  // is true, attempt to re-use the unused portion in the next GC.
  void retire(bool end_of_gc, bool retain);

  void print() PRODUCT_RETURN;

  NOT_PRODUCT(int _promotion_failure_a_lot_count;)
};

