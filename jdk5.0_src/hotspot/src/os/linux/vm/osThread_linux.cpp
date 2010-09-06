#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)osThread_linux.cpp	1.18 04/04/19 18:35:52 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// do not include  precompiled  header file
# include "incls/_osThread_linux.cpp.incl"


// Events associated with threads via "interrupt_event" must
// reside in a TSM (type-stable memory) pool.  
// The relationship between the interrupt_event and a thread
// must be stable for the lifetime of the thread.  
//
// A slightly better implementation would be to subclass Event
// with a "TSMEvent" that added the FreeNext field.  
 
static os::Linux::Event * EventFreeList = NULL ;     
static pthread_mutex_t EventFreeLock = PTHREAD_MUTEX_INITIALIZER ;
 
void OSThread::pd_initialize() {
  assert(this != NULL, "check");
  _thread_id        = 0;
  _pthread_id       = 0;
  _siginfo = NULL;
  _ucontext = NULL;
  _expanding_stack = 0;
  _alt_sig_stack = NULL;

  sigemptyset(&_caller_sigmask);

  // Try to allocate an Event from the TSM list, otherwise
  // instantiate a new Event.
  pthread_mutex_lock (&EventFreeLock) ;
  os::Linux::Event * ie = EventFreeList ;
  if (ie != NULL) {
     guarantee (ie->Immortal, "invariant") ;
     EventFreeList = ie->FreeNext ;
  }
  pthread_mutex_unlock (&EventFreeLock) ;
  if (ie == NULL) {
     ie = new os::Linux::Event();
  } else { 
     ie->reset () ;
  }
  ie->FreeNext = (os::Linux::Event *) 0xBAD ;
  ie->Immortal = 1 ;
  _interrupt_event = ie ;

  _startThread_lock = new Monitor(Mutex::event, "startThread_lock", true);
  assert(_startThread_lock !=NULL, "check");
}

void OSThread::pd_destroy() {
  os::Linux::Event * ie = _interrupt_event ;
  _interrupt_event = NULL ;
  guarantee (ie != NULL, "invariant") ;
  guarantee (ie->Immortal, "invariant") ;
  pthread_mutex_lock (&EventFreeLock) ;
  ie->FreeNext = EventFreeList ;
  EventFreeList = ie ;
  pthread_mutex_unlock (&EventFreeLock) ;

  delete _startThread_lock;
}
