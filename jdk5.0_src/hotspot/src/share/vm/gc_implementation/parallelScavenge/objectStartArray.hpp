#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)objectStartArray.hpp	1.11 03/12/23 16:40:10 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

//
// This class can be used to locate the beginning of an object in the
// covered region.
//

class ObjectStartArray : public CHeapObj {
 private:
  PSVirtualSpace  _virtual_space;
  MemRegion       _reserved_region;
  MemRegion       _covered_region;
  MemRegion       _blocks_region;
  jbyte*          _raw_base;
  jbyte*          _offset_base;

 public:

  enum BlockValueConstants {
    clean_block                  = -1
  };

  enum BlockSizeConstants {
    block_shift                  = 9,
    block_size                   = 1 << block_shift,
    block_size_in_words          = block_size / sizeof(HeapWord)
  };

 protected:

  // Mapping from address to object start array entry
  jbyte* block_for_addr(void* p) { 
    assert(_covered_region.contains(p),
           "out of bounds access to object start array");
    jbyte* result = &_offset_base[uintptr_t(p) >> block_shift];
    assert(_blocks_region.contains(result),
           "out of bounds result in byte_for");
    return result;
  }

  // Mapping from object start array entry to address of first word
  HeapWord* addr_for_block(jbyte* p) { 
    assert(_blocks_region.contains(p),
	   "out of bounds access to object start array");
    size_t delta = pointer_delta(p, _offset_base, sizeof(jbyte));
    HeapWord* result = (HeapWord*) (delta << block_shift);
    assert(_covered_region.contains(result),
           "out of bounds accessor from card marking array");
    return result;
  }

  // Mapping that includes the derived offset.
  // If the block is clean, returns the last address in the covered region.
  // If the block is < index 0, returns the start of the covered region.
  HeapWord* offset_addr_for_block(jbyte* p) {
    // We have to do this before the assert
    if (p < _raw_base) {
      return _covered_region.start();
    }

    assert(_blocks_region.contains(p),
	   "out of bounds access to object start array");

    if (*p == clean_block) {
      return _covered_region.end();
    }

    size_t delta = pointer_delta(p, _offset_base, sizeof(jbyte));
    HeapWord* result = (HeapWord*) (delta << block_shift);
    result += *p;
    
    assert(_covered_region.contains(result),
           "out of bounds accessor from card marking array");

    return result;
  }

 public:
  
  // This method is in lieu of a constructor, so that this class can be
  // embedded inline in other classes.
  void initialize(MemRegion reserved_region);

  void set_covered_region(MemRegion mr);

  void reset();

  MemRegion covered_region() { return _covered_region; }

  void allocate_block(HeapWord* p) {
    assert(_covered_region.contains(p), "Must be in covered region");
    jbyte* block = block_for_addr(p);
    HeapWord* block_base = addr_for_block(block);
    // We need a signed type to correctly assert against cleared blocks
    jbyte offset = (jbyte)pointer_delta(p, block_base, sizeof(HeapWord*));
    assert(offset >= 0 && offset < 128, "Sanity");
    // When doing MT offsets, we can't assert this.
    //assert(offset > *block, "Found backwards allocation");
    *block = offset;

    // tty->print_cr("[%p]", p);
  }

  // Optimized for finding the first object that crosses into
  // a given block. The blocks contain the offset of the last
  // object in that block. Scroll backwards by one, and the first
  // object hit should be at the begining of the block
  HeapWord* object_start(HeapWord* addr) {
    assert(_covered_region.contains(addr), "Must be in covered region");
    jbyte* block = block_for_addr(addr);
    HeapWord* scroll_forward = offset_addr_for_block(block--);
    while (scroll_forward > addr) {
      scroll_forward = offset_addr_for_block(block--);
    }

    HeapWord* next = scroll_forward;
    while (next <= addr) {
      scroll_forward = next;
      next += oop(next)->size();
    }
    assert(scroll_forward <= addr, "wrong order for current and arg");
    assert(addr <= next, "wrong order for arg and next");
    return scroll_forward;
  }

  bool is_block_allocated(HeapWord* addr) {
    assert(_covered_region.contains(addr), "Must be in covered region");
    jbyte* block = block_for_addr(addr);
    if (*block == clean_block)
      return false;

    return true;
  }
};

