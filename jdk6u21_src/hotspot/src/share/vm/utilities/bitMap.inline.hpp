/*
 * Copyright (c) 2005, 2009, Oracle and/or its affiliates. All rights reserved.
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

#ifdef ASSERT
inline void BitMap::verify_index(idx_t index) const {
  assert(index < _size, "BitMap index out of bounds");
}

inline void BitMap::verify_range(idx_t beg_index, idx_t end_index) const {
  assert(beg_index <= end_index, "BitMap range error");
  // Note that [0,0) and [size,size) are both valid ranges.
  if (end_index != _size) verify_index(end_index);
}
#endif // #ifdef ASSERT

inline void BitMap::set_bit(idx_t bit) {
  verify_index(bit);
  *word_addr(bit) |= bit_mask(bit);
}

inline void BitMap::clear_bit(idx_t bit) {
  verify_index(bit);
  *word_addr(bit) &= ~bit_mask(bit);
}

inline bool BitMap::par_set_bit(idx_t bit) {
  verify_index(bit);
  volatile idx_t* const addr = word_addr(bit);
  const idx_t mask = bit_mask(bit);
  idx_t old_val = *addr;

  do {
    const idx_t new_val = old_val | mask;
    if (new_val == old_val) {
      return false;     // Someone else beat us to it.
    }
    const idx_t cur_val = (idx_t) Atomic::cmpxchg_ptr((void*) new_val,
                                                      (volatile void*) addr,
                                                      (void*) old_val);
    if (cur_val == old_val) {
      return true;      // Success.
    }
    old_val = cur_val;  // The value changed, try again.
  } while (true);
}

inline bool BitMap::par_clear_bit(idx_t bit) {
  verify_index(bit);
  volatile idx_t* const addr = word_addr(bit);
  const idx_t mask = ~bit_mask(bit);
  idx_t old_val = *addr;

  do {
    const idx_t new_val = old_val & mask;
    if (new_val == old_val) {
      return false;     // Someone else beat us to it.
    }
    const idx_t cur_val = (idx_t) Atomic::cmpxchg_ptr((void*) new_val,
                                                      (volatile void*) addr,
                                                      (void*) old_val);
    if (cur_val == old_val) {
      return true;      // Success.
    }
    old_val = cur_val;  // The value changed, try again.
  } while (true);
}

inline void BitMap::set_range(idx_t beg, idx_t end, RangeSizeHint hint) {
  if (hint == small_range && end - beg == 1) {
    set_bit(beg);
  } else {
    if (hint == large_range) {
      set_large_range(beg, end);
    } else {
      set_range(beg, end);
    }
  }
}

inline void BitMap::clear_range(idx_t beg, idx_t end, RangeSizeHint hint) {
  if (hint == small_range && end - beg == 1) {
    clear_bit(beg);
  } else {
    if (hint == large_range) {
      clear_large_range(beg, end);
    } else {
      clear_range(beg, end);
    }
  }
}

inline void BitMap::par_set_range(idx_t beg, idx_t end, RangeSizeHint hint) {
  if (hint == small_range && end - beg == 1) {
    par_at_put(beg, true);
  } else {
    if (hint == large_range) {
      par_at_put_large_range(beg, end, true);
    } else {
      par_at_put_range(beg, end, true);
    }
  }
}

inline void BitMap::set_range_of_words(idx_t beg, idx_t end) {
  bm_word_t* map = _map;
  for (idx_t i = beg; i < end; ++i) map[i] = ~(uintptr_t)0;
}


inline void BitMap::clear_range_of_words(idx_t beg, idx_t end) {
  bm_word_t* map = _map;
  for (idx_t i = beg; i < end; ++i) map[i] = 0;
}


inline void BitMap::clear() {
  clear_range_of_words(0, size_in_words());
}


inline void BitMap::par_clear_range(idx_t beg, idx_t end, RangeSizeHint hint) {
  if (hint == small_range && end - beg == 1) {
    par_at_put(beg, false);
  } else {
    if (hint == large_range) {
      par_at_put_large_range(beg, end, false);
    } else {
      par_at_put_range(beg, end, false);
    }
  }
}

inline BitMap::idx_t
BitMap::get_next_one_offset_inline(idx_t l_offset, idx_t r_offset) const {
  assert(l_offset <= size(), "BitMap index out of bounds");
  assert(r_offset <= size(), "BitMap index out of bounds");
  assert(l_offset <= r_offset, "l_offset > r_offset ?");

  if (l_offset == r_offset) {
    return l_offset;
  }
  idx_t   index = word_index(l_offset);
  idx_t r_index = word_index(r_offset-1) + 1;
  idx_t res_offset = l_offset;

  // check bits including and to the _left_ of offset's position
  idx_t pos = bit_in_word(res_offset);
  idx_t res = map(index) >> pos;
  if (res != (uintptr_t)NoBits) {
    // find the position of the 1-bit
    for (; !(res & 1); res_offset++) {
      res = res >> 1;
    }
    assert(res_offset >= l_offset &&
           res_offset < r_offset, "just checking");
    return MIN2(res_offset, r_offset);
  }
  // skip over all word length 0-bit runs
  for (index++; index < r_index; index++) {
    res = map(index);
    if (res != (uintptr_t)NoBits) {
      // found a 1, return the offset
      for (res_offset = bit_index(index); !(res & 1); res_offset++) {
        res = res >> 1;
      }
      assert(res & 1, "tautology; see loop condition");
      assert(res_offset >= l_offset, "just checking");
      return MIN2(res_offset, r_offset);
    }
  }
  return r_offset;
}

inline BitMap::idx_t
BitMap::get_next_zero_offset_inline(idx_t l_offset, idx_t r_offset) const {
  assert(l_offset <= size(), "BitMap index out of bounds");
  assert(r_offset <= size(), "BitMap index out of bounds");
  assert(l_offset <= r_offset, "l_offset > r_offset ?");

  if (l_offset == r_offset) {
    return l_offset;
  }
  idx_t   index = word_index(l_offset);
  idx_t r_index = word_index(r_offset-1) + 1;
  idx_t res_offset = l_offset;

  // check bits including and to the _left_ of offset's position
  idx_t pos = res_offset & (BitsPerWord - 1);
  idx_t res = (map(index) >> pos) | left_n_bits((int)pos);

  if (res != (uintptr_t)AllBits) {
    // find the position of the 0-bit
    for (; res & 1; res_offset++) {
      res = res >> 1;
    }
    assert(res_offset >= l_offset, "just checking");
    return MIN2(res_offset, r_offset);
  }
  // skip over all word length 1-bit runs
  for (index++; index < r_index; index++) {
    res = map(index);
    if (res != (uintptr_t)AllBits) {
      // found a 0, return the offset
      for (res_offset = index << LogBitsPerWord; res & 1;
           res_offset++) {
        res = res >> 1;
      }
      assert(!(res & 1), "tautology; see loop condition");
      assert(res_offset >= l_offset, "just checking");
      return MIN2(res_offset, r_offset);
    }
  }
  return r_offset;
}

inline BitMap::idx_t
BitMap::get_next_one_offset_inline_aligned_right(idx_t l_offset,
                                                 idx_t r_offset) const
{
  verify_range(l_offset, r_offset);
  assert(bit_in_word(r_offset) == 0, "r_offset not word-aligned");

  if (l_offset == r_offset) {
    return l_offset;
  }
  idx_t   index = word_index(l_offset);
  idx_t r_index = word_index(r_offset);
  idx_t res_offset = l_offset;

  // check bits including and to the _left_ of offset's position
  idx_t res = map(index) >> bit_in_word(res_offset);
  if (res != (uintptr_t)NoBits) {
    // find the position of the 1-bit
    for (; !(res & 1); res_offset++) {
      res = res >> 1;
    }
    assert(res_offset >= l_offset &&
           res_offset < r_offset, "just checking");
    return res_offset;
  }
  // skip over all word length 0-bit runs
  for (index++; index < r_index; index++) {
    res = map(index);
    if (res != (uintptr_t)NoBits) {
      // found a 1, return the offset
      for (res_offset = bit_index(index); !(res & 1); res_offset++) {
        res = res >> 1;
      }
      assert(res & 1, "tautology; see loop condition");
      assert(res_offset >= l_offset && res_offset < r_offset, "just checking");
      return res_offset;
    }
  }
  return r_offset;
}


// Returns a bit mask for a range of bits [beg, end) within a single word.  Each
// bit in the mask is 0 if the bit is in the range, 1 if not in the range.  The
// returned mask can be used directly to clear the range, or inverted to set the
// range.  Note:  end must not be 0.
inline BitMap::bm_word_t
BitMap::inverted_bit_mask_for_range(idx_t beg, idx_t end) const {
  assert(end != 0, "does not work when end == 0");
  assert(beg == end || word_index(beg) == word_index(end - 1),
         "must be a single-word range");
  bm_word_t mask = bit_mask(beg) - 1;   // low (right) bits
  if (bit_in_word(end) != 0) {
    mask |= ~(bit_mask(end) - 1);       // high (left) bits
  }
  return mask;
}

inline void BitMap::set_large_range_of_words(idx_t beg, idx_t end) {
  memset(_map + beg, ~(unsigned char)0, (end - beg) * sizeof(uintptr_t));
}

inline void BitMap::clear_large_range_of_words(idx_t beg, idx_t end) {
  memset(_map + beg, 0, (end - beg) * sizeof(uintptr_t));
}

inline BitMap::idx_t BitMap::word_index_round_up(idx_t bit) const {
  idx_t bit_rounded_up = bit + (BitsPerWord - 1);
  // Check for integer arithmetic overflow.
  return bit_rounded_up > bit ? word_index(bit_rounded_up) : size_in_words();
}

inline BitMap::idx_t BitMap::get_next_one_offset(idx_t l_offset,
                                          idx_t r_offset) const {
  return get_next_one_offset_inline(l_offset, r_offset);
}

inline BitMap::idx_t BitMap::get_next_zero_offset(idx_t l_offset,
                                           idx_t r_offset) const {
  return get_next_zero_offset_inline(l_offset, r_offset);
}

inline void BitMap2D::clear() {
  _map.clear();
}
