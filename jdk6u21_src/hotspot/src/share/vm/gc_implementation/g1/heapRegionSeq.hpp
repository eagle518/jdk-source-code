/*
 * Copyright (c) 2001, 2009, Oracle and/or its affiliates. All rights reserved.
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

class HeapRegion;
class HeapRegionClosure;

class HeapRegionSeq: public CHeapObj {

  // _regions is kept sorted by start address order, and no two regions are
  // overlapping.
  GrowableArray<HeapRegion*> _regions;

  // The index in "_regions" at which to start the next allocation search.
  // (For efficiency only; private to obj_allocate after initialization.)
  int _alloc_search_start;

  // Attempts to allocate a block of the (assumed humongous) word_size,
  // starting at the region "ind".
  HeapWord* alloc_obj_from_region_index(int ind, size_t word_size);

  // Currently, we're choosing collection sets in a round-robin fashion,
  // starting here.
  int _next_rr_candidate;

  // The bottom address of the bottom-most region, or else NULL if there
  // are no regions in the sequence.
  char* _seq_bottom;

 public:
  // Initializes "this" to the empty sequence of regions.
  HeapRegionSeq(const size_t max_size);

  // Adds "hr" to "this" sequence.  Requires "hr" not to overlap with
  // any region already in "this".  (Will perform better if regions are
  // inserted in ascending address order.)
  void insert(HeapRegion* hr);

  // Given a HeapRegion*, returns its index within _regions,
  // or returns -1 if not found.
  int find(HeapRegion* hr);

  // Requires the index to be valid, and return the region at the index.
  HeapRegion* at(size_t i) { return _regions.at((int)i); }

  // Return the number of regions in the sequence.
  size_t length();

  // Returns the number of contiguous regions at the end of the sequence
  // that are available for allocation.
  size_t free_suffix();

  // Requires "word_size" to be humongous (in the technical sense).  If
  // possible, allocates a contiguous subsequence of the heap regions to
  // satisfy the allocation, and returns the address of the beginning of
  // that sequence, otherwise returns NULL.
  HeapWord* obj_allocate(size_t word_size);

  // Apply the "doHeapRegion" method of "blk" to all regions in "this",
  // in address order, terminating the iteration early
  // if the "doHeapRegion" method returns "true".
  void iterate(HeapRegionClosure* blk);

  // Apply the "doHeapRegion" method of "blk" to all regions in "this",
  // starting at "r" (or first region, if "r" is NULL), in a circular
  // manner, terminating the iteration early if the "doHeapRegion" method
  // returns "true".
  void iterate_from(HeapRegion* r, HeapRegionClosure* blk);

  // As above, but start from a given index in the sequence
  // instead of a given heap region.
  void iterate_from(int idx, HeapRegionClosure* blk);

  // Requires "shrink_bytes" to be a multiple of the page size and heap
  // region granularity.  Deletes as many "rightmost" completely free heap
  // regions from the sequence as comprise shrink_bytes bytes.  Returns the
  // MemRegion indicating the region those regions comprised, and sets
  // "num_regions_deleted" to the number of regions deleted.
  MemRegion shrink_by(size_t shrink_bytes, size_t& num_regions_deleted);

  // If "addr" falls within a region in the sequence, return that region,
  // or else NULL.
  HeapRegion* addr_to_region(const void* addr);

  void print();

  // Prints out runs of empty regions.
  void print_empty_runs();

};
