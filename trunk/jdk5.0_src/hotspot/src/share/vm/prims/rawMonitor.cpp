#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)rawMonitor.cpp	1.4 03/12/23 16:43:28 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_rawMonitor.cpp.incl"

// Platform independent support for RawMonitors for JVMPI

//
// class RawMonitor
//

RawMonitor::RawMonitor(const char *name, const int magic) {
  _name = strcpy(NEW_C_HEAP_ARRAY(char, strlen(name) + 1), name);
  _magic = magic;
  _rmnext = NULL;
  _rmprev = NULL;
}

RawMonitor::~RawMonitor() {
  if (_rmprev != NULL || _rmnext != NULL) {
     // monitor is in the list;
     remove_from_locked_list();
  }
}

// Note: destroy can be called while monitor is still busy.
int RawMonitor::raw_destroy() {
  if (_owner != NULL) {
     return OM_ILLEGAL_MONITOR_STATE; 
  }
  _magic = 0;
  if (_name) {
      FreeHeap(_name);
  }
  if (_rmprev != NULL || _rmnext != NULL) {
    // monitor is in the list
    remove_from_locked_list();
  }
  delete this;
  return OM_OK;
}


// Support for linked list of RawMonitors for JVMPI dumps
// The last element in list has _rmnext == NULL
// The first element in list has _rmprev == NULL;

// No need to grab Threads_lock as we are locking in the changed thread

// Call this when RawMonitor is locked by _owner
void RawMonitor::add_to_locked_list() {
  Thread* thr = (Thread *)_owner;
  RawMonitor* first = thr->rawmonitor_list();
  assert (_rmprev == NULL && _rmnext == NULL, "sanity check");
  if (first != NULL) {
    _rmnext = first;
    first->_rmprev = this;
  }
  thr->set_rawmonitor_list(this);
}


void RawMonitor::remove_from_locked_list() {
  Thread* thr = (Thread *)_owner;
  if ((thr == NULL) || (_rmprev == NULL && _rmnext == NULL && this != thr->rawmonitor_list())) {
    // This monitor was never added to the list
    return;
  }

  if (_rmprev == NULL) {
    // removing the head of list
    assert(thr->rawmonitor_list() == this, "must be the first element");
    thr->set_rawmonitor_list(_rmnext);
    if (_rmnext != NULL) {
      _rmnext->_rmprev = NULL;
    }
  } else {
    assert(thr->rawmonitor_list() != this, "sanity check");
    assert(_rmprev != NULL && _rmprev->_rmnext == this, "incorrect list structure");
    assert(_rmnext == NULL || _rmnext->_rmprev == this, "incorrect list structure");
    _rmprev->_rmnext = _rmnext;
    if (_rmnext != NULL) {
      _rmnext->_rmprev = _rmprev;
    }
  }
  _rmnext = NULL; _rmprev = NULL; // removed
}

