#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)objectMonitor_linux.hpp	1.11 03/12/23 16:37:32 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

 private:
  void          enter2(TRAPS);
  void          enqueue(ObjectWaiter* waiter);
  ObjectWaiter* dequeue();
  void          dequeue2(ObjectWaiter* waiter);
  void          EntryQueue_unlink (ObjectWaiter* node) ;
  void          EntryQueue_insert (ObjectWaiter * node, int PrePend) ;
  ObjectWaiter * EntryQueue_SelectSuccessor () ; 
  void          EnterI (TRAPS) ; 

 public:
  intptr_t      contentions() const;
  ObjectWaiter* first_waiter();
  ObjectWaiter* next_waiter(ObjectWaiter* o);
  Thread*       thread_of_waiter(ObjectWaiter* o);
  void          Recycle () ; 

 private:
  os::Linux::OSMutex _mutex;        // platform specific mutex
  ObjectWaiter * volatile _EntryQueue ; 
  void * volatile _succ ;          // Heir presumptive for wakeup throttling
  int _QMix ;                      // mostly prepend queue discipline

