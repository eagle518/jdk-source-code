#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)threadLS_solaris_sparc.hpp	1.23 04/01/06 14:14:42 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

public:
  // Java Thread  - force inlining
  static inline Thread* thread() ; 

private:
  static Thread* _get_thread_cache[];  // index by [(raw_id>>9)^(raw_id>>20) % _pd_cache_size]
  static Thread* get_thread_via_cache_slowly(uintptr_t raw_id, int index);

  NOT_PRODUCT(static int _tcacheHit;)
  NOT_PRODUCT(static int _tcacheMiss;)

public:

  // Print cache hit/miss statistics
  static void print_statistics() PRODUCT_RETURN;

  enum Constants {
    _pd_cache_size         =  256*2  // projected typical # of threads * 2
  };

  static void set_thread_in_slot (Thread *) ; 

  static uintptr_t pd_raw_thread_id() {
    return _raw_thread_id(); 
  }

  static int pd_cache_index(uintptr_t raw_id) {
    // Hash function: From email from Dave Dice:
    // The hash function deserves an explanation.  %g7 points to libthread's
    // "thread" structure.  On T1 the thread structure is allocated on the
    // user's stack (yes, really!) so the ">>20" handles T1 where the JVM's
    // stack size is usually >= 1Mb.  The ">>9" is for T2 where Roger allocates
    // globs of thread blocks contiguously.  The "9" has to do with the
    // expected size of the T2 thread structure.  If these constants are wrong
    // the worst thing that'll happen is that the hit rate for heavily threaded
    // apps won't be as good as it could be.  If you want to burn another
    // shift+xor you could mix together _all of the %g7 bits to form the hash,
    // but I think that's excessive.  Making the change above changed the
    // T$ miss rate on SpecJBB (on a 16X system) from about 3% to imperceptible.
    uintptr_t ix = (int) (((raw_id >> 9) ^ (raw_id >> 20)) % _pd_cache_size);
    return ix;
  }
