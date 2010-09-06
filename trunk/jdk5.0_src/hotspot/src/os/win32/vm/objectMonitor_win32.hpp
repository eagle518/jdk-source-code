#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)objectMonitor_win32.hpp	1.21 03/12/23 16:37:55 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

 private:
  void          enqueue(void* waiter);
  ObjectWaiter* dequeue();
  void          dequeue2(void* waiter);

 public:
  intptr_t      contentions() const;
  ObjectWaiter* first_waiter();
  ObjectWaiter* next_waiter(ObjectWaiter* o);
  Thread*       thread_of_waiter(ObjectWaiter* o);
  void          Recycle () {}

 private:
  HANDLE _semaphore;   // system dependent semaphore
  
