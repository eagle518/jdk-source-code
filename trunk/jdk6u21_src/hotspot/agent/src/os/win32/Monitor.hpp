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

#ifndef _MONITOR_
#define _MONITOR_

#include <windows.h>

class Monitor {
public:
  Monitor();
  ~Monitor();

  void lock();
  void unlock();
  // Default time is forever (i.e, zero). Returns true if it times-out, otherwise
  // false.
  bool wait(long timeout = 0);
  bool notify();
  bool notifyAll();

private:
  HANDLE owner();
  void setOwner(HANDLE owner);
  bool ownedBySelf();

  HANDLE _owner;
  long   _lock_count;
  HANDLE _lock_event;   // Auto-reset event for blocking in lock()
  HANDLE _wait_event;   // Manual-reset event for notifications
  long _counter;        // Current number of notifications
  long _waiters;        // Number of threads waiting for notification
  long _tickets;        // Number of waiters to be notified
};


#endif  // #defined _MONITOR_
