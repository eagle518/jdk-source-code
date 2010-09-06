#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)mutex_win32.cpp	1.47 04/04/16 10:53:38 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_mutex_win32.cpp.incl"

// put OS-includes here
# include <windows.h>

// Implementation of Mutex

// A simple Mutex for VM locking: it is not guaranteed to interoperate with
// the fast object locking, so exclusively use Mutex locking or exclusively
// use fast object locking.

void Mutex::wait_for_lock_implementation() {
  DWORD dwRet = WaitForSingleObject((HANDLE)_lock_event,  INFINITE);
  assert(dwRet == WAIT_OBJECT_0, "unexpected return value from WaitForSingleObject");
}

void Mutex::wait_for_lock_blocking_implementation(JavaThread *thread) {
  ThreadBlockInVM tbivm(thread);

  bool threadIsSuspended;
  do {
    thread->set_suspend_equivalent();
    // cleared by handle_special_suspend_equivalent_condition() or java_suspend_self()
    wait_for_lock_implementation();

    // were we externally suspended while we were waiting?
    threadIsSuspended = thread->handle_special_suspend_equivalent_condition();
    if (threadIsSuspended) {
      //
      // We have been granted the contended Mutex, but while we were
      // waiting another thread suspended us. We don't want to keep
      // the Mutex while suspended because that would surprise the
      // thread that suspended us. Reset the event because we will
      // be calling wait_for_lock_implementation() again when we
      // get resumed.
      //
      DWORD dwRet = SetEvent((HANDLE)_lock_event);
      assert(dwRet != 0, "unexpected return value from SetEvent");

      thread->java_suspend_self();
    }
  } while (threadIsSuspended);
}

Mutex::Mutex(int rank, const char *name, bool allow_vm_block) debug_only( : _rank(rank)) {
  _lock_count = -1;       // No threads has entered the critical section
  _lock_event = CreateEvent(NULL, false, false, NULL);
  _suppress_signal = false;
  _owner          = INVALID_THREAD;
  _name           = name;

#ifndef PRODUCT
  _allow_vm_block = allow_vm_block;
  debug_only(_next= NULL;)
#endif
}


Mutex::~Mutex() {
  assert(_owner == INVALID_THREAD, "Owned Mutex being deleted");
  assert(_lock_count == -1, "Mutex being deleted with non -1 lock count");
  CloseHandle((HANDLE)_lock_event);
}


void Mutex::unlock() {  
  assert(_owner == Thread::current(), "Mutex not being unlocked by owner");
  assert(_lock_count >= 0, "Mutex being unlocked without positive lock count");
  trace("unlocks");
  set_owner(INVALID_THREAD);  
  if (InterlockedDecrement((long *) &_lock_count) >= 0) {
    // Wake a waiting thread up
    if (!_suppress_signal) {
      DWORD dwRet = SetEvent((HANDLE)_lock_event);
      assert(dwRet != 0, "unexpected return value from SetEvent");
    } else {
      assert(SafepointSynchronize::at_vmthread_safepoint(),
             "can't sneak");
    }
    _suppress_signal = false;
  }    
}

// Can be called by non-Java threads (JVM_RawMonitorExit)
void Mutex::jvm_raw_unlock() {
  assert(rank() == native, "must be called by non-VM locks");
  // Do not call set_owner, as this would break.
  _owner = INVALID_THREAD;  
  if (InterlockedDecrement((long *) &_lock_count) >= 0) {
    // Wake a waiting thread up
    if (!_suppress_signal) {
      DWORD dwRet = SetEvent((HANDLE)_lock_event);
      assert(dwRet != 0, "unexpected return value from SetEvent");
    } else {
      assert(SafepointSynchronize::at_vmthread_safepoint(),
             "can't sneak");
    }
    _suppress_signal = false;
  }    
}


#ifndef PRODUCT
void Mutex::print() const {
  tty->print_cr("Mutex: [0x%x] %s - lock_count: %d", this, _name, _lock_count);
}
#endif


//
// Monitor
//

Monitor::Monitor(int rank, const char *name, bool allow_vm_block) : Mutex(rank, name, allow_vm_block) {
  HANDLE e = CreateEvent(NULL, true, false, NULL);
  if (e == 0) {
    fatal("Could not initialize condition variable");
  }
  _event = e;
  _counter = 0;
  _tickets = 0;
  _waiters = 0;
}


Monitor::~Monitor() {
  if (_event != 0) { // careful in case dtor virtualness changes.
    CloseHandle((HANDLE)_event);
    _event = 0;
  }
}  


bool Monitor::wait(bool no_safepoint_check, long timeout) {

  Thread* thread = Thread::current();

  assert(_owner != INVALID_THREAD, "Wait on unknown thread");
  assert(_owner == thread, "Wait on Monitor not by owner");

  // The design rule for use of mutexes of rank special or less is
  // that we are guaranteed not to block while holding such mutexes.
  // Here we verify that the least ranked mutex that we hold,
  // modulo the mutex we are about to relinquish, satisfies that
  // constraint, since we are about to block in a wait.
  #ifdef ASSERT
    Mutex* least = get_least_ranked_lock_besides_this(thread->owned_locks());
    assert(least != this, "Specification of get_least_... call above");
    if (least != NULL && least->rank() <= special) {
      tty->print("Attempting to wait on monitor %s/%d while holding"
                 " lock %s/%d -- possible deadlock",
                 name(), rank(), least->name(), least->rank());
      assert(false,
             "Shouldn't block(wait) while holding a lock of rank special");
    }
  #endif // ASSERT

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
      os::yield_all();
    } else {
      retry = true;
    }
         
    if (no_safepoint_check) {
      // Need to leave state as SUSPENDED for wait on SR_lock
      // OSThreadWaitState osts(thread->osthread(), false /* not Object.wait() */);
      which = WaitForSingleObject((HANDLE)_event,  timeout_value);
      // Enter critical section (this will also put us in an unblocked state)    
      lock_without_safepoint_check();
    } else { 
      { 
        assert(thread->is_Java_thread(), "Must be JavaThread");
        JavaThread *jt = (JavaThread *)thread;
        ThreadBlockInVM tbivm(jt);
        OSThreadWaitState osts(thread->osthread(), false /* not Object.wait() */);
        jt->set_suspend_equivalent();
        // cleared by handle_special_suspend_equivalent_condition() or java_suspend_self()
        which = WaitForSingleObject((HANDLE)_event,  timeout_value);

        // were we externally suspended while we were waiting?
        if (jt->handle_special_suspend_equivalent_condition()) {
          //
          // Our event wait has finished and we are ready to relock the
          // Monitor, but while we were waiting another thread suspended
          // us. We don't want to hold the Monitor while suspended because
          // that would surprise the thread that suspended us. We don't
          // need to reset the event because we don't need to loop
          // around for another WaitForSingleObject() call.
          //
          jt->java_suspend_self();
        }
      }
      // Enter critical section (this will also put us in an unblocked state)    
      lock();
    }        

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
    ResetEvent((HANDLE)_event);
  }  

  return false;
}


// Notify a single thread waiting on this ConditionVariable
bool Monitor::notify() {
  assert(owned_by_self(), "notify on unknown thread");
  
  if (_waiters > _tickets) {
    if (!SetEvent((HANDLE)_event)) {
      return false;
    }
    _tickets++;
    _counter++;
  }

  return true;

}


// Notify all threads waiting on this ConditionVariable
bool Monitor::notify_all() {
  assert(owned_by_self(), "notify_all on unknown thread");
  
  if (_waiters > 0) {
    if (!SetEvent((HANDLE)_event)) {
      return false;
    }
    _tickets = _waiters;
    _counter++;
  }

  return true;
}

// JSR166
// -------------------------------------------------------

/*
 * The Windows implementation of Park is very straightforward: Basic
 * operations on Win32 Events turn out to have the right semantics to
 * use them directly. We opportunistically resuse the event inherited
 * from Monitor.
 */

Parker::Parker() 
  : Monitor(Mutex::nonleaf, "Park Semaphore", true) {
}

Parker::~Parker() { // needed since non-virtual?
  if (_event != 0) {
    CloseHandle((HANDLE)_event);
    _event = 0;
  }
}  

void Parker::park(bool isAbsolute, jlong time) {
  // First, demultiplex/decode time arguments
  if (time < 0) { // don't wait
    return;  
  }
  else if (time == 0) {
    time = INFINITE;
  }
  else if  (isAbsolute) {
    time -= os::javaTimeMillis(); // convert to relative time
    if (time <= 0) // already elapsed
      return;
  }
  else { // relative
    time /= 1000000; // Must coarsen from nanos to millis
    if (time == 0)   // Wait for the minimal time unit if zero
      time = 1;
  } 

  JavaThread* thread = (JavaThread*)(Thread::current());
  assert(thread->is_Java_thread(), "Must be JavaThread");
  JavaThread *jt = (JavaThread *)thread;

  // Don't wait if interrupted or already triggered
  if (Thread::is_interrupted(thread, false) || 
      WaitForSingleObject((HANDLE)_event, 0) == WAIT_OBJECT_0) {
    ResetEvent((HANDLE)_event);
    return;
  }
  else {
    ThreadBlockInVM tbivm(jt);
    OSThreadWaitState osts(thread->osthread(), false /* not Object.wait() */);
    jt->set_suspend_equivalent();
    
    WaitForSingleObject((HANDLE)_event,  time);
    ResetEvent((HANDLE)_event);
    
    // If externally suspended while waiting, re-suspend
    if (jt->handle_special_suspend_equivalent_condition()) {
      jt->java_suspend_self();
    }
  }
}

void Parker::unpark() {
  SetEvent((HANDLE)_event);
}
