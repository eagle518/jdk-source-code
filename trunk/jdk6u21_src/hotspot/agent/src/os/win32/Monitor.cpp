/*
 * Copyright (c) 2001, Oracle and/or its affiliates. All rights reserved.
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

#include <stdio.h>
#include <assert.h>
#include "Monitor.hpp"

Monitor::Monitor() {
  _lock_count = -1;       // No threads have entered the critical section
  _owner = NULL;
  _lock_event = CreateEvent(NULL, false, false, NULL);
  _wait_event = CreateEvent(NULL, true, false, NULL);
  _counter = 0;
  _tickets = 0;
  _waiters = 0;
}

Monitor::~Monitor() {
  assert(_owner == NULL);    // Otherwise, owned monitor being deleted
  assert(_lock_count == -1); // Otherwise, monitor being deleted with non -1 lock count
  CloseHandle(_lock_event);
  CloseHandle(_wait_event);
}

void
Monitor::lock() {
  if (InterlockedIncrement(&_lock_count) == 0) {
    // Success, we now own the lock
  } else {
    DWORD dwRet = WaitForSingleObject((HANDLE)_lock_event,  INFINITE);
    assert(dwRet == WAIT_OBJECT_0); // Unexpected return value from WaitForSingleObject
  }
  assert(owner() == NULL); // Otherwise, lock count and owner are inconsistent
  setOwner(GetCurrentThread());
}

void
Monitor::unlock() {
  setOwner(NULL);
  if (InterlockedDecrement(&_lock_count) >= 0) {
    // Wake a waiting thread up
    DWORD dwRet = SetEvent(_lock_event);
    assert(dwRet != 0); // Unexpected return value from SetEvent
  }
}

bool
Monitor::wait(long timeout) {
  assert(owner() != NULL);
  assert(owner() == GetCurrentThread());

  // 0 means forever. Convert to Windows specific code.
  DWORD timeout_value = (timeout == 0) ? INFINITE : timeout;
  DWORD which;

  long c = _counter;
  bool retry = false;

  _waiters++;
  // Loop until condition variable is signaled.  The event object is
  // set whenever the condition variable is signaled, and tickets will
  // reflect the number of threads which have been notified. The counter
  // field is used to make sure we don't respond to notifications that
  // have occurred *before* we started waiting, and is incremented each
  // time the condition variable is signaled.

  while (true) {

    // Leave critical region
    unlock();

    // If this is a retry, let other low-priority threads have a chance
    // to run.  Make sure that we sleep outside of the critical section.
    if (retry) {
      Sleep(1);
    } else {
      retry = true;
    }

    which = WaitForSingleObject(_wait_event, timeout_value);
    // Enter critical section
    lock();

    if (_tickets != 0 && _counter != c) break;

    if (which == WAIT_TIMEOUT) {
      --_waiters;
      return true;
    }
  }
  _waiters--;

  // If this was the last thread to be notified, then we need to reset
  // the event object.
  if (--_tickets == 0) {
    ResetEvent(_wait_event);
  }

  return false;
}

// Notify a single thread waiting on this monitor
bool
Monitor::notify() {
  assert(ownedBySelf()); // Otherwise, notify on unknown thread

  if (_waiters > _tickets) {
    if (!SetEvent(_wait_event)) {
      return false;
    }
    _tickets++;
    _counter++;
  }

  return true;
}

// Notify all threads waiting on this monitor
bool
Monitor::notifyAll() {
  assert(ownedBySelf()); // Otherwise, notifyAll on unknown thread

  if (_waiters > 0) {
    if (!SetEvent(_wait_event)) {
      return false;
    }
    _tickets = _waiters;
    _counter++;
  }

  return true;
}

HANDLE
Monitor::owner() {
  return _owner;
}

void
Monitor::setOwner(HANDLE owner) {
  if (owner != NULL) {
    assert(_owner == NULL);                 // Setting owner thread of already owned monitor
    assert(owner == GetCurrentThread());    // Else should not be doing this
  } else {
    HANDLE oldOwner = _owner;
    assert(oldOwner != NULL);               // Removing the owner thread of an unowned mutex
    assert(oldOwner == GetCurrentThread());
  }
  _owner = owner;
}

bool
Monitor::ownedBySelf() {
  return (_owner == GetCurrentThread());
}
