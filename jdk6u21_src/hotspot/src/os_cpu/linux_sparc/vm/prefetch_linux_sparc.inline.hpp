/*
 * Copyright (c) 2003, 2008, Oracle and/or its affiliates. All rights reserved.
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

inline void Prefetch::read(void *loc, intx interval) {
  __asm__ volatile("prefetch [%0+%1], 0" : : "r" (loc), "r" (interval) : "memory" );
}

inline void Prefetch::write(void *loc, intx interval) {
  __asm__ volatile("prefetch [%0+%1], 2" : : "r" (loc), "r" (interval) : "memory" );
}

#else

inline void Prefetch::read (void *loc, intx interval) {}
inline void Prefetch::write(void *loc, intx interval) {}

#endif
