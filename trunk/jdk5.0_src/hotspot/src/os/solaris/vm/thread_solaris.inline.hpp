#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)thread_solaris.inline.hpp	1.5 04/01/06 14:10:34 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

// Thread::current is "hot" it's called > 128K times in the 1st 500 msecs of
// startup.
// ThreadLocalStorage::thread is warm -- it's called > 16K times in the same
// period.   Thread::current() now calls ThreadLocalStorage::thread() directly.
// For SPARC, to avoid excessive register window spill-fill faults,
// we aggressively inline these routines.  

inline Thread* ThreadLocalStorage::thread()  {
  // don't use specialized code if +UseMallocOnly -- may confuse Purify et al.
  debug_only(if (UseMallocOnly) return get_thread_slow(););

  uintptr_t raw = pd_raw_thread_id();
  int ix = pd_cache_index(raw);
  Thread *Candidate = ThreadLocalStorage::_get_thread_cache[ix];
  if (Candidate->_self_raw_id == raw) {
    // hit
    return Candidate;
  } else {
    return ThreadLocalStorage::get_thread_via_cache_slowly(raw, ix);
  }
}
