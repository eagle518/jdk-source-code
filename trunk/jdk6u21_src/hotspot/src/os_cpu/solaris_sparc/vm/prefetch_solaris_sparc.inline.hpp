/*
 * Copyright (c) 2003, 2007, Oracle and/or its affiliates. All rights reserved.
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

#if defined(COMPILER2) || defined(_LP64)

// For Sun Studio inplementation is in solaris_sparc.il
// For gcc inplementation is just below
extern "C" void _Prefetch_read (void *loc, intx interval);
extern "C" void _Prefetch_write(void *loc, intx interval);

inline void Prefetch::read(void *loc, intx interval) {
  _Prefetch_read(loc, interval);
}

inline void Prefetch::write(void *loc, intx interval) {
  _Prefetch_write(loc, interval);
}

#ifdef _GNU_SOURCE
extern "C" {
  inline void _Prefetch_read (void *loc, intx interval) {
    __asm__ volatile
      ("prefetch [%0+%1], 0" : : "r" (loc), "r" (interval) : "memory" );
  }
  inline void _Prefetch_write(void *loc, intx interval) {
    __asm__ volatile
      ("prefetch [%0+%1], 2" : : "r" (loc), "r" (interval) : "memory" );
  }
}
#endif // _GNU_SOURCE

#else  // defined(COMPILER2) || defined(_LP64)

inline void Prefetch::read (void *loc, intx interval) {}
inline void Prefetch::write(void *loc, intx interval) {}

#endif // defined(COMPILER2) || defined(_LP64)
