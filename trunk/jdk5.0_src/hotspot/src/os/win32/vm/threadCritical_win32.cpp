#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)threadCritical_win32.cpp	1.10 03/12/23 16:37:59 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_threadCritical_win32.cpp.incl"

// OS-includes here
# include <windows.h>
# include <winbase.h>

//
// See threadCritical.hpp for details of this class.
//

static bool initialized = false;
static volatile jint lock_count = -1;
static HANDLE lock_event;
static DWORD lock_owner = -1;

//
// Note that Microsoft's critical region code contains a race
// condition, and is not suitable for use. A thread holding the
// critical section cannot safely suspend a thread attempting
// to enter the critical region. The failure mode is that both
// threads are permanently suspended.
//
// I experiemented with the use of ordinary windows mutex objects
// and found them ~30 times slower than the critical region code.
//

void ThreadCritical::initialize() {
}

void ThreadCritical::release() {
  assert(lock_owner == -1, "Mutex being deleted while owned.");
  assert(lock_count == -1, "Mutex being deleted while recursively locked");
  assert(lock_event != NULL, "Sanity check");
  CloseHandle(lock_event);
}

ThreadCritical::ThreadCritical() { 
  DWORD current_thread = GetCurrentThreadId();

  if (lock_owner != current_thread) {
    // Grab the lock before doing anything.
    while (Atomic::cmpxchg(0, &lock_count, -1) != -1) {
      if (initialized) {
        DWORD ret = WaitForSingleObject(lock_event,  INFINITE);
        assert(ret == WAIT_OBJECT_0, "unexpected return value from WaitForSingleObject");
      }
    }

    // Make sure the event object is allocated.
    if (!initialized) {
      // Locking will not work correctly unless this is autoreset.
      lock_event = CreateEvent(NULL, false, false, NULL);
      initialized = true;
    }
      
    assert(lock_owner == -1, "Lock acquired illegally.");
    lock_owner = current_thread;
  } else {
    // Atomicity isn't required. Bump the recursion count.
    lock_count++;
  }

  assert(lock_owner == GetCurrentThreadId(), "Lock acquired illegally.");
}

ThreadCritical::~ThreadCritical() {
  assert(lock_owner == GetCurrentThreadId(), "unlock attempt by wrong thread");
  assert(lock_count >= 0, "Attempt to unlock when already unlocked");

  if (lock_count == 0) {
    // We're going to unlock
    lock_owner = -1;
    lock_count = -1;
    // No lost wakeups, lock_event stays signaled until reset.
    DWORD ret = SetEvent(lock_event);
    assert(ret != 0, "unexpected return value from SetEvent");
  } else {
    // Just unwinding a recursive lock;
    lock_count--;
  }
}

