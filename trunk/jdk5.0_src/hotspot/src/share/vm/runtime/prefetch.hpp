#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)prefetch.hpp	1.3 03/12/23 16:44:02 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

// If calls to prefetch methods are in a loop, the loop should be cloned
// such that if Prefetch{Scan,Copy}Interval and/or PrefetchFieldInterval
// say not to do prefetching, these methods aren't called.  At the very
// least, they take up a memory issue slot.  They should be implemented
// as inline assembly code: doing an actual call isn't worth the cost.

class Prefetch : AllStatic {
 public:
  enum style {
    do_none,  // Do no prefetching
    do_read,  // Do read prefetching
    do_write  // Do write prefetching
  };

  // Prefetch anticipating read; must not fault, semantically a no-op
  static void read(void* loc, intx interval);

  // Prefetch anticipating write; must not fault, semantically a no-op
  static void write(void* loc, intx interval);
};
