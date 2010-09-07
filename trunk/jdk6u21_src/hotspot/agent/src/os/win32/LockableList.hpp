/*
 * Copyright (c) 2000, Oracle and/or its affiliates. All rights reserved.
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

#ifndef _LOCKABLE_LIST_
#define _LOCKABLE_LIST_

#include <windows.h>
#include "BasicList.hpp"

template<class T>
class LockableList : public BasicList<T> {
private:
  CRITICAL_SECTION crit;

public:
  LockableList() {
    InitializeCriticalSection(&crit);
  }

  ~LockableList() {
    DeleteCriticalSection(&crit);
  }

  void lock() {
    EnterCriticalSection(&crit);
  }

  void unlock() {
    LeaveCriticalSection(&crit);
  }
};

#endif  // #defined _LOCKABLE_LIST_
