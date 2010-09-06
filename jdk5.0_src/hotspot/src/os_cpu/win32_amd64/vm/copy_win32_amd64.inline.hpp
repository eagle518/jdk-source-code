#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)copy_win32_amd64.inline.hpp	1.1 04/04/30 17:03:48 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

static void pd_conjoint_bytes(void* from, void* to, size_t count) {
  (void)memmove(to, from, count);
}

static void pd_conjoint_bytes_atomic(void* from, void* to, size_t count) {
  (void)memmove(to, from, count);
}

static void pd_conjoint_jshorts_atomic(jshort* from, jshort* to, size_t count) {
  // FIXME
  (void)memmove(to, from, count << LogBytesPerShort);
}

static void pd_conjoint_jints_atomic(jint* from, jint* to, size_t count) {
  // FIXME
  (void)memmove(to, from, count << LogBytesPerInt);
}

static void pd_conjoint_jlongs_atomic(jlong* from, jlong* to, size_t count) {
  assert(BytesPerLong == BytesPerOop, "jlongs and oops must be the same size");
  pd_conjoint_oops_atomic((oop*)from, (oop*)to, count);
}

static void pd_conjoint_oops_atomic(oop* from, oop* to, size_t count) {
  // Do better than this: inline memmove body  NEEDS CLEANUP
  if (from > to) {
    while (count-- > 0) {
      // Copy forwards
      *to++ = *from++;
    }
  } else {
    from += count - 1;
    to   += count - 1;
    while (count-- > 0) {
      // Copy backwards
      *to-- = *from--;
    }
  }
}

static void pd_arrayof_conjoint_bytes(HeapWord* from, HeapWord* to, size_t count) {
  pd_conjoint_bytes_atomic(from, to, count);
}

static void pd_arrayof_conjoint_jshorts(HeapWord* from, HeapWord* to, size_t count) {
  pd_conjoint_jshorts_atomic((jshort*)from, (jshort*)to, count);
}

static void pd_arrayof_conjoint_jints(HeapWord* from, HeapWord* to, size_t count) {
  pd_conjoint_jints_atomic((jint*)from, (jint*)to, count);
}

static void pd_arrayof_conjoint_jlongs(HeapWord* from, HeapWord* to, size_t count) {
  pd_conjoint_jlongs_atomic((jlong*)from, (jlong*)to, count);
}

static void pd_arrayof_conjoint_oops(HeapWord* from, HeapWord* to, size_t count) {
  pd_conjoint_oops_atomic((oop*)from, (oop*)to, count);
}
