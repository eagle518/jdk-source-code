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


inline void Prefetch::read (void *loc, intx interval) {
#ifdef AMD64
  __asm__ ("prefetcht0 (%0,%1,1)" : : "r" (loc), "r" (interval));
#endif // AMD64
}

inline void Prefetch::write(void *loc, intx interval) {
#ifdef AMD64

  // Do not use the 3dnow prefetchw instruction.  It isn't supported on em64t.
  //  __asm__ ("prefetchw (%0,%1,1)" : : "r" (loc), "r" (interval));
  __asm__ ("prefetcht0 (%0,%1,1)" : : "r" (loc), "r" (interval));

#endif // AMD64
}
