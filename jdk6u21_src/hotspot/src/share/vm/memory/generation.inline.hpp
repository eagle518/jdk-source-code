/*
 * Copyright (c) 2000, 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

bool OneContigSpaceCardGeneration::is_in(const void* p) const {
  return the_space()->is_in(p);
}


WaterMark OneContigSpaceCardGeneration::top_mark() {
  return the_space()->top_mark();
}

CompactibleSpace*
OneContigSpaceCardGeneration::first_compaction_space() const {
  return the_space();
}

HeapWord* OneContigSpaceCardGeneration::allocate(size_t word_size,
                                                 bool is_tlab) {
  assert(!is_tlab, "OneContigSpaceCardGeneration does not support TLAB allocation");
  return the_space()->allocate(word_size);
}

HeapWord* OneContigSpaceCardGeneration::par_allocate(size_t word_size,
                                                     bool is_tlab) {
  assert(!is_tlab, "OneContigSpaceCardGeneration does not support TLAB allocation");
  return the_space()->par_allocate(word_size);
}

WaterMark OneContigSpaceCardGeneration::bottom_mark() {
  return the_space()->bottom_mark();
}

size_t OneContigSpaceCardGeneration::block_size(const HeapWord* addr) const {
  if (addr < the_space()->top()) return oop(addr)->size();
  else {
    assert(addr == the_space()->top(), "non-block head arg to block_size");
    return the_space()->_end - the_space()->top();
  }
}

bool OneContigSpaceCardGeneration::block_is_obj(const HeapWord* addr) const {
  return addr < the_space()->top();
}
