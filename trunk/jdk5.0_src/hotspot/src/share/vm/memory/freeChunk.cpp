#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)freeChunk.cpp	1.9 03/12/23 16:41:08 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_freeChunk.cpp.incl"

#ifndef PRODUCT

#define baadbabeHeapWord badHeapWordVal
#define deadbeefHeapWord 0xdeadbeef

size_t const FreeChunk::header_size() {
  return sizeof(FreeChunk)/HeapWordSize;
}

void FreeChunk::mangleAllocated(size_t size) {
  // mangle all but the header of a just-allocated block
  // of storage
  assert(size >= MinChunkSize, "smallest size of object");
  // we can't assert that _size == size because this may be an
  // allocation out of a linear allocation block
  assert(sizeof(FreeChunk) % HeapWordSize == 0,
         "shouldn't write beyond chunk");
  HeapWord* addr = (HeapWord*)this;
  size_t hdr = header_size();
  Copy::fill_to_aligned_words(addr + hdr, size - hdr, baadbabeHeapWord);
}

void FreeChunk::mangleFreed(size_t size) {
  assert(baadbabeHeapWord != deadbeefHeapWord, "Need distinct patterns");
  // mangle all but the header of a just-freed block of storage
  // just prior to passing it to the storage dictionary
  assert(size >= MinChunkSize, "smallest size of object");
  assert(size == _size, "just checking");
  HeapWord* addr = (HeapWord*)this;
  size_t hdr = header_size();
  Copy::fill_to_aligned_words(addr + hdr, size - hdr, deadbeefHeapWord);
}

void FreeChunk::verifyList() const {
  FreeChunk* nextFC = next();
  if (nextFC != NULL) {
    assert(this == nextFC->prev(), "broken chain");
    assert(size() == nextFC->size(), "wrong size");
    nextFC->verifyList();
  }
}
#endif
