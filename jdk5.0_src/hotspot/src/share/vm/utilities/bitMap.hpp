#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)bitMap.hpp	1.32 03/12/23 16:44:40 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Closure for iterating over BitMaps

class BitMapClosure VALUE_OBJ_CLASS_SPEC {
 public:
  // Callback when bit in map is set
  virtual void do_bit(size_t offset) = 0;
};


// Operations for bitmaps represented as arrays of unsigned 32- or 64-bit
// integers (uintptr_t).
//
// Bit offsets are numbered from 0 to size-1

class BitMap VALUE_OBJ_CLASS_SPEC {
 public:
  typedef size_t idx_t;		// Type used for bit and word indices.

  // Hints for range sizes.
  typedef enum {
    unknown_range, small_range, large_range
  } RangeSizeHint;

 private:
  uintptr_t* _map;     // First word in bitmap
  idx_t      _size;    // Size of bitmap (in bits)

  static idx_t      bit_in_word(idx_t bit)  { return bit & (BitsPerWord - 1); }
  static uintptr_t  bit_mask   (idx_t bit)  { return ((uintptr_t)1) << bit_in_word(bit); }
  static idx_t      word_index (idx_t bit)  { return bit >> LogBitsPerWord; }
  static idx_t      bit_index  (idx_t word) { return word << LogBitsPerWord; }

  uintptr_t* map() const                { return _map; }
  uintptr_t* word_addr(idx_t bit) const { return map() + word_index(bit); }

  void set_word  (idx_t word) { _map[word] = ~(uintptr_t)0; }
  void clear_word(idx_t word) { _map[word] = 0; }

  // Utilities for ranges of bits.  Ranges are half-open [beg, end).

  // Ranges within a single word.
  inline uintptr_t inverted_bit_mask_for_range(idx_t beg, idx_t end) const;
  inline void      set_range_within_word      (idx_t beg, idx_t end);
  inline void      clear_range_within_word    (idx_t beg, idx_t end);
  inline void      par_put_range_within_word  (idx_t beg, idx_t end, bool value);

  // Ranges spanning entire words.
  inline void      set_range_of_words         (idx_t beg, idx_t end);
  inline void      clear_range_of_words       (idx_t beg, idx_t end);

  // The index of the first full word in a range.
  inline idx_t     word_index_round_up        (idx_t bit) const;

  // Verification, statistics.
  void verify_index(idx_t index) const {
    assert(index < _size, "BitMap index out of bounds");
  }

  void verify_range(idx_t beg_index, idx_t end_index) const {
#ifdef ASSERT
    assert(beg_index <= end_index, "BitMap range error");
    // Note that [0,0) and [size,size) are both valid ranges.
    if (end_index != _size)  verify_index(end_index);
#endif
  }

 public:
  // Construction
  BitMap(uintptr_t* map, idx_t size_in_bits);

  // Allocates necessary data structure in resource area
  BitMap(idx_t size_in_bits);

  void set_map(uintptr_t* map)         { _map = map; }
  void set_size(idx_t size_in_bits)    { _size = size_in_bits; }

  // Allocates necessary data structure in resource area.
  // Destroys any state currently in bit map.
  // Does not perform any frees (i.e., of current _map).
  void resize(idx_t size_in_bits);

  // Accessing
  idx_t size() const                    { return _size; }
  idx_t size_in_words() const           {
    return word_index(size() + BitsPerWord - 1);
  }

  bool at(idx_t index) const {
    verify_index(index);
    return (*word_addr(index) & bit_mask(index)) != 0;
  }

  void set_bit(idx_t index) {
    verify_index(index);
    *word_addr(index) |= bit_mask(index);
  }

  void clear_bit(idx_t index) {
    verify_index(index);
    *word_addr(index) &= ~bit_mask(index);
  }

  // Put the given value at the given offset. The paraller version
  // will CAS the value into the bitmap and is quite a bit slower.
  // The parallel version also returns a value indicating if the
  // calling thread was the one that changed the value of the bit.
  void at_put(idx_t index, bool value);
  bool par_at_put(idx_t index, bool value);

  // Update a range of bits.  Ranges are half-open [beg, end).
  void set_range   (idx_t beg, idx_t end);
  void clear_range (idx_t beg, idx_t end);
  void at_put_range(idx_t beg, idx_t end, bool value);
  void par_at_put_range(idx_t beg, idx_t end, bool value);

  // Update a range of bits, using a hint about the size.  Currently only
  // inlines the predominant case of a 1-bit range.  Works best when hint is a
  // compile-time constant.
  inline void set_range  (idx_t beg, idx_t end, RangeSizeHint hint);
  inline void clear_range(idx_t beg, idx_t end, RangeSizeHint hint);
  inline void par_set_range  (idx_t beg, idx_t end, RangeSizeHint hint);

  // Clearing
  void clear();
  
  // Iteration support
  void iterate(BitMapClosure* blk, idx_t leftIndex, idx_t rightIndex);
  inline void iterate(BitMapClosure* blk) {
    // call the version that takes an interval
    iterate(blk, 0, size());
  }

  // Looking for 1's and 0's to the "right"
  idx_t get_next_one_offset (idx_t l_index, idx_t r_index) const;
  idx_t get_next_zero_offset(idx_t l_index, idx_t r_index) const;

  // Set operations.
  void set_union(BitMap bits);
  void set_difference(BitMap bits);
  void set_intersection(BitMap bits);

  // Returns result of whether this map changed
  // during the operation
  bool set_union_with_result(BitMap bits);
  bool set_difference_with_result(BitMap bits);
  bool set_intersection_with_result(BitMap bits);

  void set_from(BitMap bits);

  bool is_same(BitMap bits);

#ifndef PRODUCT
 public:
  // Printing
  void print_on(outputStream* st) const;
#endif
};

inline void BitMap::set_range(idx_t beg, idx_t end, RangeSizeHint hint) {
  if (hint == small_range && end - beg == 1) {
    set_bit(beg);
  } else {
    set_range(beg, end);
  }
}

inline void BitMap::clear_range(idx_t beg, idx_t end, RangeSizeHint hint) {
  if (hint == small_range && end - beg == 1) {
    clear_bit(beg);
  } else {
    clear_range(beg, end);
  }
}

inline void BitMap::par_set_range(idx_t beg, idx_t end, RangeSizeHint hint) {
  if (hint == small_range && end - beg == 1) {
    par_at_put(beg, true);
  } else {
    par_at_put_range(beg, end, true);
  }
}
