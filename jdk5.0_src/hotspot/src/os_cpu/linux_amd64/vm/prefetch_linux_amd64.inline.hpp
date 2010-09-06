#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)prefetch_linux_amd64.inline.hpp	1.3 04/06/04 20:06:42 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

inline void Prefetch::read(void* loc, intx interval) 
{
  __builtin_prefetch((char*) loc + interval, 0); // prefetcht0 (%rsi, %rdi,1)
}

inline void Prefetch::write(void* loc, intx interval)
{
  // Force prefetchw.  The gcc builtin produces prefetcht0 or prefetchw
  // depending on command line switches we don't control here.
  // Use of this method should be gated by VM_Version::has_prefetchw.
  __asm__ ("prefetchw (%0,%1,1)" : : "r" (loc), "r" (interval));
  //  __builtin_prefetch((char*) loc + interval, 1); // prefetcht0/prefetchw (%rsi,%rdi,1)
}
