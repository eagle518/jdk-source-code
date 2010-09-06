#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)copy.hpp	1.6 04/04/30 16:50:13 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

// Assembly code for platforms that need it.
extern "C" {
  void _Copy_conjoint_words(HeapWord* from, HeapWord* to, size_t count);
  void _Copy_disjoint_words(HeapWord* from, HeapWord* to, size_t count);

  void _Copy_conjoint_words_atomic(HeapWord* from, HeapWord* to, size_t count);
  void _Copy_disjoint_words_atomic(HeapWord* from, HeapWord* to, size_t count);

  void _Copy_aligned_conjoint_words(HeapWord* from, HeapWord* to, size_t count);
  void _Copy_aligned_disjoint_words(HeapWord* from, HeapWord* to, size_t count);

  void _Copy_conjoint_bytes(void* from, void* to, size_t count);

  void _Copy_conjoint_bytes_atomic  (void*   from, void*   to, size_t count);
  void _Copy_conjoint_jshorts_atomic(jshort* from, jshort* to, size_t count);
  void _Copy_conjoint_jints_atomic  (jint*   from, jint*   to, size_t count);
  void _Copy_conjoint_jlongs_atomic (jlong*  from, jlong*  to, size_t count);
  void _Copy_conjoint_oops_atomic   (oop*    from, oop*    to, size_t count);

  void _Copy_arrayof_conjoint_bytes  (HeapWord* from, HeapWord* to, size_t count);
  void _Copy_arrayof_conjoint_jshorts(HeapWord* from, HeapWord* to, size_t count);
  void _Copy_arrayof_conjoint_jints  (HeapWord* from, HeapWord* to, size_t count);
  void _Copy_arrayof_conjoint_jlongs (HeapWord* from, HeapWord* to, size_t count);
  void _Copy_arrayof_conjoint_oops   (HeapWord* from, HeapWord* to, size_t count);
}

class Copy : AllStatic {
 public:
  // Block copy methods have four attributes.  We don't define all possibilities.
  //   alignment: aligned according to minimum Java object alignment (MinObjAlignment)
  //   arrayof:   arraycopy operation with both operands aligned on the same
  //              boundary as the first element of an array of the copy unit.
  //              This is currently a HeapWord boundary on all platforms, except
  //              for long and double arrays, which are aligned on an 8-byte
  //              boundary on all platforms.
  //              arraycopy operations are implicitly atomic on each array element.
  //   overlap:   disjoint or conjoint.
  //   copy unit: bytes or words (i.e., HeapWords) or oops (i.e., pointers).
  //   atomicity: atomic or non-atomic on the copy unit.
  //
  // Names are constructed thusly:
  //
  //     [ 'aligned_' | 'arrayof_' ]
  //     ('conjoint_' | 'disjoint_')
  //     ('words' | 'bytes' | 'jshorts' | 'jints' | 'jlongs' | 'oops')
  //     [ '_atomic' ]
  //
  // Except in the arrayof case, whatever the alignment is, we assume we can copy
  // whole alignment units.  E.g., if MinObjAlignment is 2x word alignment, an odd
  // count may copy an extra word.  In the arrayof case, we are allowed to copy
  // only the number of copy units specified.

  // HeapWords

  // Word-aligned words,    conjoint, not atomic on each word
  static void conjoint_words(HeapWord* from, HeapWord* to, size_t count) {
    assert_params_ok(from, to, LogHeapWordSize);
    pd_conjoint_words(from, to, count);
  }

  // Word-aligned words,    disjoint, not atomic on each word
  static void disjoint_words(HeapWord* from, HeapWord* to, size_t count) {
    assert_params_ok(from, to, LogHeapWordSize);
    assert_disjoint(from, to, count);
    pd_disjoint_words(from, to, count);
  }

  // Word-aligned words,    disjoint, atomic on each word
  static void disjoint_words_atomic(HeapWord* from, HeapWord* to, size_t count) {
    assert_params_ok(from, to, LogHeapWordSize);
    assert_disjoint(from, to, count);
    pd_disjoint_words_atomic(from, to, count);
  }

  // Object-aligned words,  conjoint, not atomic on each word
  static void aligned_conjoint_words(HeapWord* from, HeapWord* to, size_t count) {
    assert_params_aligned(from, to);
    assert_non_zero(count);
    pd_aligned_conjoint_words(from, to, count);
  }

  // Object-aligned words,  disjoint, not atomic on each word
  static void aligned_disjoint_words(HeapWord* from, HeapWord* to, size_t count) {
    assert_params_aligned(from, to);
    assert_disjoint(from, to, count);
    assert_non_zero(count);
    pd_aligned_disjoint_words(from, to, count);
  }

  // bytes, jshorts, jints, jlongs, oops

  // bytes,                 conjoint, not atomic on each byte (not that it matters)
  static void conjoint_bytes(void* from, void* to, size_t count) {
    assert_non_zero(count);
    pd_conjoint_bytes(from, to, count);
  }

  // bytes,                 conjoint, atomic on each byte (not that it matters)
  static void conjoint_bytes_atomic(void* from, void* to, size_t count) {
    assert_non_zero(count);
    pd_conjoint_bytes(from, to, count);
  }

  // jshorts,               conjoint, atomic on each jshort
  static void conjoint_jshorts_atomic(jshort* from, jshort* to, size_t count) {
    assert_params_ok(from, to, LogBytesPerShort);
    assert_non_zero(count);
    pd_conjoint_jshorts_atomic(from, to, count);
  }

  // jints,                 conjoint, atomic on each jint
  static void conjoint_jints_atomic(jint* from, jint* to, size_t count) {
    assert_params_ok(from, to, LogBytesPerInt);
    assert_non_zero(count);
    pd_conjoint_jints_atomic(from, to, count);
  }

  // jlongs,                conjoint, atomic on each jlong
  static void conjoint_jlongs_atomic(jlong* from, jlong* to, size_t count) {
    assert_params_ok(from, to, LogBytesPerLong);
    assert_non_zero(count);
    pd_conjoint_jlongs_atomic(from, to, count);
  }

  // oops,                  conjoint, atomic on each oop
  static void conjoint_oops_atomic(oop* from, oop* to, size_t count) {
    assert_params_ok(from, to, LogBytesPerOop);
    assert_non_zero(count);
    pd_conjoint_oops_atomic(from, to, count);
  }

  // bytes,                 conjoint array, atomic on each byte (not that it matters)
  static void arrayof_conjoint_bytes(HeapWord* from, HeapWord* to, size_t count) {
    assert_non_zero(count);
    pd_arrayof_conjoint_bytes(from, to, count);
  }

  // jshorts,               conjoint array, atomic on each jshort
  static void arrayof_conjoint_jshorts(HeapWord* from, HeapWord* to, size_t count) {
    assert_params_ok(from, to, LogBytesPerShort);
    assert_non_zero(count);
    pd_arrayof_conjoint_jshorts(from, to, count);
  }

  // jints,                 conjoint array, atomic on each jint
  static void arrayof_conjoint_jints(HeapWord* from, HeapWord* to, size_t count) {
    assert_params_ok(from, to, LogBytesPerInt);
    assert_non_zero(count);
    pd_arrayof_conjoint_jints(from, to, count);
  }

  // jlongs,                conjoint array, atomic on each jlong
  static void arrayof_conjoint_jlongs(HeapWord* from, HeapWord* to, size_t count) {
    assert_params_ok(from, to, LogBytesPerLong);
    assert_non_zero(count);
    pd_arrayof_conjoint_jlongs(from, to, count);
  }

  // oops,                  conjoint array, atomic on each oop
  static void arrayof_conjoint_oops(HeapWord* from, HeapWord* to, size_t count) {
    assert_params_ok(from, to, LogBytesPerOop);
    assert_non_zero(count);
    pd_arrayof_conjoint_oops(from, to, count);
  }

  // Known overlap methods

  // Copy word-aligned words from higher to lower addresses, not atomic on each word
  inline static void conjoint_words_to_lower(HeapWord* from, HeapWord* to, size_t byte_count) {
    // byte_count is in bytes to check its alignment
    assert_params_ok(from, to, LogHeapWordSize);
    assert_byte_count_ok(byte_count, HeapWordSize);

    size_t count = (size_t)round_to(byte_count, HeapWordSize) >> LogHeapWordSize;
    assert(to <= from || from + count <= to, "do not overwrite source data");

    while (count-- > 0) {
      *to++ = *from++;
    }
  }

  // Copy word-aligned words from lower to higher addresses, not atomic on each word
  inline static void conjoint_words_to_higher(HeapWord* from, HeapWord* to, size_t byte_count) {
    // byte_count is in bytes to check its alignment
    assert_params_ok(from, to, LogHeapWordSize);
    assert_byte_count_ok(byte_count, HeapWordSize);

    size_t count = (size_t)round_to(byte_count, HeapWordSize) >> LogHeapWordSize;
    assert(from <= to || to + count <= from, "do not overwrite source data");

    from += count - 1;
    to   += count - 1;
    while (count-- > 0) {
      *to-- = *from--;
    }
  }

  // Fill methods

  // Fill word-aligned words, not atomic on each word
  // set_words
  static void fill_to_words(HeapWord* to, size_t count, juint value = 0) {
    assert_params_ok(to, LogHeapWordSize);
    pd_fill_to_words(to, count, value);
  }

  static void fill_to_aligned_words(HeapWord* to, size_t count, juint value = 0) {
    assert_params_aligned(to);
    pd_fill_to_aligned_words(to, count, value);
  }

  // Fill bytes
  static void fill_to_bytes(void* to, size_t count, jubyte value = 0) {
    pd_fill_to_bytes(to, count, value);
  }

  // Zero-fill methods

  // Zero word-aligned words, not atomic on each word
  static void zero_to_words(HeapWord* to, size_t count) {
    assert_params_ok(to, LogHeapWordSize);
    pd_zero_to_words(to, count);
  }

  // Zero bytes
  static void zero_to_bytes(void* to, size_t count) {
    pd_zero_to_bytes(to, count);
  }

 private:
  static bool params_disjoint(HeapWord* from, HeapWord* to, size_t count) {
    if (from < to) {
      return pointer_delta(to, from) >= count;
    }
    return pointer_delta(from, to) >= count;
  }

  // These methods raise a fatal if they detect a problem.

  static void assert_disjoint(HeapWord* from, HeapWord* to, size_t count) {
#ifdef ASSERT
    if (!params_disjoint(from, to, count))
      basic_fatal("source and dest overlap");
#endif
  }

  static void assert_params_ok(void* from, void* to, intptr_t log_align) {
#ifdef ASSERT
    if (mask_bits((uintptr_t)from, right_n_bits(log_align)) != 0)
      basic_fatal("not aligned");
    if (mask_bits((uintptr_t)to, right_n_bits(log_align)) != 0)
      basic_fatal("not aligned");
#endif
  }

  static void assert_params_ok(HeapWord* to, uintptr_t log_align) {
#ifdef ASSERT
    if (mask_bits((uintptr_t)to, right_n_bits(log_align)) != 0)
      basic_fatal("not word aligned");
#endif
  }

  static void assert_params_aligned(HeapWord* from, HeapWord* to) {
#ifdef ASSERT
    if (mask_bits((uintptr_t)from, MinObjAlignment-1) != 0)
      basic_fatal("not object aligned");
    if (mask_bits((uintptr_t)to, MinObjAlignment-1) != 0)
      basic_fatal("not object aligned");
#endif
  }

  static void assert_params_aligned(HeapWord* to) {
#ifdef ASSERT
    if (mask_bits((uintptr_t)to, MinObjAlignment-1) != 0)
      basic_fatal("not object aligned");
#endif
  }

  static void assert_non_zero(size_t count) {
#ifdef ASSERT
    if (count == 0) {
      basic_fatal("count must be non-zero");
    }
#endif
  }

  static void assert_byte_count_ok(size_t byte_count, size_t unit_size) {
#ifdef ASSERT
    if ((size_t)round_to(byte_count, unit_size) != byte_count) {
      basic_fatal("byte count must be aligned");
    }
#endif
  }

  // Platform dependent implementations of the above methods.
  #include "incls/_copy_pd.hpp.incl"
};
