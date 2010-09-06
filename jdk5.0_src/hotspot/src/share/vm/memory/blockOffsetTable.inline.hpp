#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)blockOffsetTable.inline.hpp	1.14 03/12/23 16:40:53 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

//////////////////////////////////////////////////////////////////////////
// BlockOffsetTable inlines
//////////////////////////////////////////////////////////////////////////
inline HeapWord* BlockOffsetTable::block_start(const void* addr) const {
  if (addr >= _bottom && addr < _end) {
    return block_start_unsafe(addr);
  } else {
    return NULL;
  }
}

//////////////////////////////////////////////////////////////////////////
// BlockOffsetSharedArray inlines
//////////////////////////////////////////////////////////////////////////
inline size_t BlockOffsetSharedArray::index_for(const void* p) const {
  char* pc = (char*)p;
  assert(pc >= (char*)_reserved.start() &&
	 pc <  (char*)_reserved.end(),
	 "p not in range.");
  size_t delta = pointer_delta(pc, _reserved.start(), sizeof(char));
  size_t result = delta >> LogN;
  assert(result < _vs.committed_size(), "bad index from address");
  return result;
}

inline HeapWord* BlockOffsetSharedArray::address_for_index(size_t index) const {
  assert(index < _vs.committed_size(), "bad index");
  HeapWord* result = _reserved.start() + (index << LogN_words);
  assert(result >= _reserved.start() && result < _reserved.end(),
         "bad address from index");
  return result;
}
    

//////////////////////////////////////////////////////////////////////////
// BlockOffsetArrayNonContigSpace inlines
//////////////////////////////////////////////////////////////////////////
inline void BlockOffsetArrayNonContigSpace::freed(HeapWord* blk_start,
                                                  HeapWord* blk_end) {
  // Verify that the BOT shows [blk_start, blk_end) to be one block.
  verify_single_block(blk_start, blk_end);
  // adjust _unallocated_block upward or downward
  // as appropriate
  if (BlockOffsetArrayUseUnallocatedBlock) {
    assert(_unallocated_block <= _end,
           "Inconsistent value for _unallocated_block");
    if (blk_end >= _unallocated_block && blk_start <= _unallocated_block) {
      // CMS-specific note: a block abutting _unallocated_block to
      // its left is being freed, a new block is being added or
      // we are resetting following a compaction
      _unallocated_block = blk_start;
    }
  }
}
