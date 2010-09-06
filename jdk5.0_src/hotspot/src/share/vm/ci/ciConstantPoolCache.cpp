#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)ciConstantPoolCache.cpp	1.6 03/12/23 16:39:26 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_ciConstantPoolCache.cpp.incl"

// ciConstantPoolCache
//
// This class caches indexed constant pool lookups.

// ------------------------------------------------------------------
// ciConstantPoolCache::ciConstantPoolCache
ciConstantPoolCache::ciConstantPoolCache(Arena* arena,
                                 int expected_size) {
  _elements =
    new (arena) GrowableArray<void*>(arena, expected_size, 0, 0);
  _keys = new (arena) GrowableArray<intptr_t>(arena, expected_size, 0, 0);
}

// ------------------------------------------------------------------
// ciConstantPoolCache::get
//
// Get the entry at some index
void* ciConstantPoolCache::get(int index) {
  ASSERT_IN_VM;
  int pos = find(index);
  if (pos >= _keys->length() ||
      _keys->at(pos) != index) {
    // This element is not present in the cache.
    return NULL;
  }
  return _elements->at(pos);
}

// ------------------------------------------------------------------
// ciConstantPoolCache::find
//
// Use binary search to find the position of this index in the cache.
// If there is no entry in the cache corresponding to this oop, return
// the position at which the index would be inserted.
int ciConstantPoolCache::find(int key) {
  int min = 0;
  int max = _keys->length()-1;

  while (max >= min) {
    int mid = (max + min) / 2;
    int value = _keys->at(mid);
    if (value < key) {
      min = mid + 1;
    } else if (value > key) {
      max = mid - 1;
    } else {
      return mid;
    }
  }
  return min;
}

// ------------------------------------------------------------------
// ciConstantPoolCache::insert
//
// Insert a ciObject into the table at some index.
void ciConstantPoolCache::insert(int index, void* elem) {
  int i;
  int pos = find(index);
  for (i = _keys->length()-1; i >= pos; i--) {
    _keys->at_put_grow(i+1, _keys->at(i));
    _elements->at_put_grow(i+1, _elements->at(i));
  }
  _keys->at_put_grow(pos, index);
  _elements->at_put_grow(pos, elem);
}

// ------------------------------------------------------------------
// ciConstantPoolCache::print
//
// Print debugging information about the cache.
void ciConstantPoolCache::print() {
  Unimplemented();
}
