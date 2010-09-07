/*
 * Copyright (c) 2003, Oracle and/or its affiliates. All rights reserved.
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

extern "C" {
  void _Prefetch_read (void *loc, intx interval);
  void _Prefetch_write(void *loc, intx interval);
}

inline void Prefetch::read (void *loc, intx interval) {
#ifdef AMD64
  _Prefetch_read(loc, interval);
#endif // AMD64
}

// Use of this method should be gated by VM_Version::has_prefetchw.
inline void Prefetch::write(void *loc, intx interval) {
#ifdef AMD64
  _Prefetch_write(loc, interval);
#endif // AMD64
}
