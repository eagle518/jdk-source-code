#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)objectMonitor_win32.inline.hpp	1.14 03/12/23 16:37:56 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

//  NOTE:
//  The following code will be re-packaged in the future.
//  Please don't complain about the syntax and convention.

// return the number of threads contending for this monitor
inline intptr_t ObjectMonitor::contentions() const {
  // don't count the owner of the monitor
  return ((_count > 0) ? _count - 1 : 0);
}

inline intptr_t ObjectMonitor::is_busy() const {
  return (_count | _waiters);
}

inline void ObjectMonitor::set_owner(void* owner) {
  _owner = owner;
  _recursions = 0;
  _count = 1;
  if (owner == NULL) {
    _count = 0;
  }
}

inline bool EventWait(HANDLE event, int64_t millis) {
  // On Windows millis values greater than max_millis_value
  // have special semantics. This value has been found by
  // painful debugging.
  const DWORD max_millis_value = 0xFFFF0000;
  DWORD ms = (millis == 0 || (millis >= max_millis_value))
           ? INFINITE
           : (DWORD) millis;
  intptr_t ret = ::WaitForSingleObject(event, ms);
  assert(ret != WAIT_FAILED, "Unexpected error in event wait!");

  return (ret == WAIT_TIMEOUT);
}
