/*
 * Copyright (c) 2001, 2007, Oracle and/or its affiliates. All rights reserved.
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

inline HeapRegion* HeapRegionSeq::addr_to_region(const void* addr) {
  assert(_seq_bottom != NULL, "bad _seq_bottom in addr_to_region");
  if ((char*) addr >= _seq_bottom) {
    size_t diff = (size_t) pointer_delta((HeapWord*) addr,
                                         (HeapWord*) _seq_bottom);
    int index = (int) (diff >> HeapRegion::LogOfHRGrainWords);
    assert(index >= 0, "invariant / paranoia");
    if (index < _regions.length()) {
      HeapRegion* hr = _regions.at(index);
      assert(hr->is_in_reserved(addr),
             "addr_to_region is wrong...");
      return hr;
    }
  }
  return NULL;
}
