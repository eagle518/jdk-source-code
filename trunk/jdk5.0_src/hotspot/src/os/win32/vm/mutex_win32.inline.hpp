#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)mutex_win32.inline.hpp	1.12 03/12/23 16:37:54 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

inline bool Mutex::lock_implementation() {
  return InterlockedIncrement((long *) &_lock_count)==0;
}


inline bool Mutex::try_lock_implementation() {
  // We can only get the lock, if we can atomicly increase the _lock_count 
  // from -1 to 0. Hence, this is like lock_implementation, except that we
  // only count if the initial value is -1.
  return (Atomic::cmpxchg(0, &_lock_count, -1) == -1);
}
