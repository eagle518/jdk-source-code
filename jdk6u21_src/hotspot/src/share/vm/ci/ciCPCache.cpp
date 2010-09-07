/*
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
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

#include "incls/_precompiled.incl"
#include "incls/_ciCPCache.cpp.incl"

// ciCPCache

// ------------------------------------------------------------------
// ciCPCache::get_f1_offset
size_t ciCPCache::get_f1_offset(int index) {
  // Calculate the offset from the constantPoolCacheOop to the f1
  // field.
  ByteSize f1_offset =
    constantPoolCacheOopDesc::entry_offset(index) +
    ConstantPoolCacheEntry::f1_offset();

  return in_bytes(f1_offset);
}


// ------------------------------------------------------------------
// ciCPCache::print
//
// Print debugging information about the cache.
void ciCPCache::print() {
  Unimplemented();
}
