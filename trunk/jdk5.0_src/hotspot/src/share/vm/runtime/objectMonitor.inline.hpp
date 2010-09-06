#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)objectMonitor.inline.hpp	1.17 03/12/23 16:43:58 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

inline intptr_t ObjectMonitor::is_entered(TRAPS) const {
  if (THREAD == _owner || THREAD->is_lock_owned((address) _owner)) {
    return 1;
  }
  return 0;
}

inline markOop ObjectMonitor::header() const {
  return _header;
}

inline void ObjectMonitor::set_header(markOop hdr) {
  _header = hdr;
}

inline intptr_t ObjectMonitor::count() const {
  return _count;
}

inline void ObjectMonitor::set_count(intptr_t count) {
  _count= count;
}

inline intptr_t ObjectMonitor::waiters() const {
  return _waiters;
}

inline void* ObjectMonitor::queue() const {
  return _queue;
}

inline void ObjectMonitor::set_queue(void* queue) {
  _queue = queue;
}

inline void* ObjectMonitor::owner() const {
  return _owner;
}

inline void ObjectMonitor::clear() {
  assert(_header, "Fatal logic error in ObjectMonitor header!");
  assert(_count == 0, "Fatal logic error in ObjectMonitor count!");
  assert(_waiters == 0, "Fatal logic error in ObjectMonitor waiters!");
  assert(_recursions == 0, "Fatal logic error in ObjectMonitor recursions!");
  assert(_object, "Fatal logic error in ObjectMonitor object!");
  assert(_owner == 0, "Fatal logic error in ObjectMonitor owner!");
  assert(_queue == 0, "Fatal logic error in ObjectMonitor queue!");

  _header = NULL;
  _object = NULL;
}


inline void* ObjectMonitor::object() const {
  return _object;
}

inline void* ObjectMonitor::object_addr() {
  return (void *)(&_object);
}

inline void ObjectMonitor::set_object(void* obj) {
  _object = obj;
}

inline bool ObjectMonitor::check(TRAPS) {
  if (THREAD != _owner) {
    if (THREAD->is_lock_owned((address) _owner)) {
      _owner = THREAD;  // regain ownership of inflated monitor
    } else {
      check_slow(THREAD);
      return false;
    }
  }
  return true;
}

// here are the platform-dependent bodies:

# include "incls/_objectMonitor_pd.inline.hpp.incl"
