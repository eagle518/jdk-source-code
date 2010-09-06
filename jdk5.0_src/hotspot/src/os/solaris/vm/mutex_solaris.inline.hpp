#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)mutex_solaris.inline.hpp	1.11 03/12/23 16:37:44 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

inline bool Mutex::lock_implementation() {
  int status = ((os::Solaris::Event*)_lock_event)->trylock();
  if (status != 0) { 
    debug_only(_lock_count++); 
    return true; 
  } 
  return false;
}

inline bool Mutex::try_lock_implementation() {
  // Same on Solaris.
  return lock_implementation();
}


inline void Mutex::wait_for_lock_implementation() {
  assert(!owned_by_self(), "deadlock");
  ((os::Solaris::Event*)_lock_event)->lock();
  debug_only(_lock_count++;)
}
