#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)objectMonitor_win32.cpp	1.66 04/02/12 09:33:37 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_objectMonitor_win32.cpp.incl"

class ObjectWaiter : public StackObj {
 public:
  ObjectWaiter* _next;
  Thread*       _thread;
  HANDLE        _event;
  intptr_t      _notified;
 public:
  ObjectWaiter(Thread* thread, HANDLE event) {
    _next     = NULL;
    _thread   = thread;
    _notified = 0;
    _event    = event;
  }
};


ObjectWaiter* ObjectMonitor::first_waiter() {
  return (ObjectWaiter*)_queue;
}

ObjectWaiter* ObjectMonitor::next_waiter(ObjectWaiter* o) {
  return o->_next;
}

Thread* ObjectMonitor::thread_of_waiter(ObjectWaiter* o) {
  return o->_thread;
}

// initialize the monitor, exception the semaphore, all other fields
// are simple integers or pointers
ObjectMonitor::ObjectMonitor() {
  _header     = NULL;
  _count      = 0;
  _waiters    = 0,
  _recursions = 0;
  _object     = NULL;
  _owner      = NULL;
  _queue      = NULL;
  _semaphore  = ::CreateSemaphore(0, 0, 1, 0);
  assert(_semaphore != NULL, "CreateSemaphore() failed!");
}

ObjectMonitor::~ObjectMonitor() {
  ::CloseHandle(_semaphore);
}

inline void ObjectMonitor::enqueue(void* argwaiter) {
  ObjectWaiter* waiter = (ObjectWaiter*)argwaiter;
  waiter->_next = NULL;
  // put waiter to the end of queue (single linked list)
  if (_queue == NULL) {
    _queue = waiter;
  } else {
    ObjectWaiter* iterator = (ObjectWaiter*)_queue;
    while (iterator->_next) {
      iterator = iterator->_next;
    }
    iterator->_next = waiter;
  }
}

inline ObjectWaiter* ObjectMonitor::dequeue() {
  // dequeue the very first waiter
  ObjectWaiter* waiter = (ObjectWaiter*)_queue;
  if (waiter) {
    _queue = waiter->_next;
    waiter->_next = NULL;
  }
  return waiter;
}

inline void ObjectMonitor::dequeue2(void* argwaiter) {
  ObjectWaiter* waiter = (ObjectWaiter*) argwaiter;
  // when the waiter has woken up because of an interrupt,
  // timeout or other suprious wake-up, dequeue the 
  // waiter from waiting list (use linear search)
  if (_queue == waiter) {
    _queue = waiter->_next;
    waiter->_next = NULL;
  } else {
    ObjectWaiter* iterator = (ObjectWaiter*) _queue;
    while (iterator->_next != waiter) {
      // the waiter must be in the queue, iterator could not be null
      assert(iterator->_next != NULL, "ObjectMonitor::dequeue2");
      iterator = iterator->_next;
    }
    iterator->_next = waiter->_next;
    waiter->_next = NULL;
  }
}

void ObjectMonitor::enter(TRAPS) {
  if (Atomic::cmpxchg_ptr(1, &_count, 0) == 0) {
    // Got the lock.  cmpxchg_ptr is defined to be fence_cmpxchg_ptr_acquire,
    // so the store of THREAD to _owner can't float above it.  We don't
    // bother with a release after the store to _owner.  The worst that
    // could happen is that another thread sees a bogus _owner while we
    // hold the mutex, which would cause it to fail on the cmpxchg, which
    // is harmless.
    _owner = THREAD;
  } else if (_owner == THREAD) {
    // Recursive lock.
    _recursions++;
  } else if (THREAD->is_lock_owned((address) _owner)) {
    assert(_recursions == 0, "check");
    _owner = THREAD;
    _recursions = 1;
  } else {
    if (UseSpinning) {
      for(int j = 0; j < PreBlockSpin; j++) {
        if (PreSpinYield) {
          os::yield();
        }
        // Spinning: retry the fast path again
        for (int i = 0; i < ReadSpinIterations; i++) {
          if (_count == 0) {
            if (Atomic::cmpxchg_ptr(1, &_count, 0) == 0) {
              // Fast path, we got the lock.  cmpxchg_ptr is defined to be
              // fence_cmpxchg_ptr_acquire, so no fence is needed here.
              _owner = THREAD;
              return;
            }
          }
        }
        if (PostSpinYield) {
          os::yield();  
        }
      }
    }
    
    assert(THREAD->is_Java_thread(), "Must be Java thread!");
    JavaThread *jt = (JavaThread *)THREAD;
    bool didContend = false;
    // Increment the number of waiters.  If it ends up one, we acquired the lock,
    // otherwise we must wait on _semaphore.
    if (Atomic::add_ptr(1, &_count) > 1) {
      // the monitor is locked by some other thread
      // Change java thread status to indicate blocked on monitor enter.
      JavaThreadBlockedOnMonitorEnterState jtbmes(jt, this);
      didContend = true;

      if (JvmtiExport::should_post_monitor_contended_enter()) {
        JvmtiExport::post_monitor_contended_enter(jt, this);
      }
      if (jvmpi::is_event_enabled(JVMPI_EVENT_MONITOR_CONTENDED_ENTER)) {
        jvmpi::post_monitor_contended_enter_event(object());
      } 

      OSThreadContendState osts(THREAD->osthread());
        
      ThreadBlockInVM tbivm(jt);

      bool threadIsSuspended;
      THREAD->set_current_pending_monitor(this);
      do {
        jt->set_suspend_equivalent();
        // cleared by handle_special_suspend_equivalent_condition() or java_suspend_self()
        DWORD ret = ::WaitForSingleObject(_semaphore, INFINITE);
        assert(ret == WAIT_OBJECT_0, "WaitForSingleObject() failed!");

        //
        // We have been granted the contended monitor, but while we were
        // waiting another thread externally suspended us. We don't want
        // to enter the monitor while suspended because that would surprise
        // the thread that suspended us.
        //
        threadIsSuspended = jt->handle_special_suspend_equivalent_condition();
        if (threadIsSuspended) {
          BOOL ret = ::ReleaseSemaphore(_semaphore, 1, NULL);
          assert(ret, "ReleaseSemaphore() failed!");
          jt->java_suspend_self();
        }
      } while (threadIsSuspended);
      THREAD->set_current_pending_monitor(NULL);
    }

    // Thread is back in _thread_in_vm

    // Even though there are circumstances where we only need to do an acquire, (e.g., if
    // the add_ptr succeeded and we didn't yield in the spin loop, the cmpxchg_ptr at the
    // start of this method did the fence for us), take the easy way out.
    OrderAccess::fence();

    // There should be no ownership
    assert(_owner == NULL, "Logic error with monitor owner!");
    assert(_recursions == 0, "Logic error with monitor recursions!");

    // If we didn't do the prior fence, we'd have to do a release here to make sure
    // the new value of _owner isn't visible until after the atomic add is.
    _owner = THREAD;
    if (didContend) {
      if (JvmtiExport::should_post_monitor_contended_entered()) {
        JvmtiExport::post_monitor_contended_entered(jt, this);
      }
      if (jvmpi::is_event_enabled(JVMPI_EVENT_MONITOR_CONTENDED_ENTERED)) {
        jvmpi::post_monitor_contended_entered_event(object());
      }
    }
  }
}

void ObjectMonitor::exit(TRAPS) {
  if (THREAD != _owner) {
    if (THREAD->is_lock_owned((address) _owner)) {
      // Regain ownership of inflated monitor.
      _owner = THREAD;
    } else {
      // NOTE: we need to handle unbalanced monitor enter/exit
      // in native code by throwing an exception.
      assert(false, "Non-balanced monitor enter/exit!");
      return;
    }
  }
  if (_recursions == 0) {
    // Tell the world we'll not own the lock in the very near future.
    // We do a release before the store to _owner because other threads
    // seeing a NULL _owner field will deem the monitor to be unlocked.
    // We want all accesses inside the region guarded by this monitor
    // (i.e., arbitrary java code) to be complete before unlocking.
    // Otherwise, we could end up making stores protected by the
    // monitor visible after making the unlock visible, so those
    // stores would effectively complete outside the guarded region.
    OrderAccess::release_store_ptr(&_owner, NULL);
    // Do a release so NULL value of _owner is visible before the
    // add_ptr that constitutes the unlock.  add_ptr may not do the
    // release for us.  If we didn't first do a release, another thread
    // could see a non-NULL _owner field along with an unlocked monitor.
    // It might then acquire the lock and set _owner to its own thread
    // id.  If at that point we completed our NULL store to _owner, the
    // other thread (which now owns the lock) would see NULL in _owner
    // instead of its own thread id.
    OrderAccess::release();
    // Decrement count atomicly.  If it still more than zero
    // there must be someone else blocking on monitor enter.
    if (Atomic::add_ptr(-1, &_count) > 0) {
      BOOL ret = ::ReleaseSemaphore(_semaphore, 1, NULL);
      assert(ret, "ReleaseSemaphore() failed!");
      if (jvmpi::is_event_enabled(JVMPI_EVENT_MONITOR_CONTENDED_EXIT)) {
        jvmpi::post_monitor_contended_exit_event(object());
      }
    }
  } else {
    // Simple recursive exit.  Decrementing _recursions constitutes an
    // 'unlock'.  We don't do a release beforehand because we still own
    // the lock and no other threads should be depending on what order
    // non-volatile accesses guarded by the lock (i.e., arbitrary java
    // code) become visible while we hold the lock.  It's possible this
    // isn't true, in which case we should replace the decrement with
    //
    // OrderAccess::release_store_ptr(&_recursions, _recursions - 1);
    //
    // and ensure that the interpreter and compiler-generated recursive
    // unlock code do a release.
    _recursions--;
  }
}

void ObjectMonitor::wait(jlong millis, bool interruptable, TRAPS) {
  assert(THREAD->is_Java_thread(), "Must be Java thread!");
  JavaThread *jt = (JavaThread *)THREAD;

  check(CHECK);
  // check interrupt event
  if (interruptable && Thread::is_interrupted(THREAD, true) && !HAS_PENDING_EXCEPTION) {
    THROW(vmSymbols::java_lang_InterruptedException());
  }

  jt->set_current_waiting_monitor(this);

  // create a node to be put into the queue
  ObjectWaiter node(THREAD, THREAD->osthread()->interrupt_event());
  // enter the waiting queue, which is single linked list in this case
  // how it could be a priority queue, or any data structure
  enqueue(&node);
  if (jvmpi::is_event_enabled(JVMPI_EVENT_MONITOR_WAIT)) {
    jvmpi::post_monitor_wait_event((oop)object(), millis);
  }
  DWORD start_wait;
  if (jvmpi::is_event_enabled(JVMPI_EVENT_MONITOR_WAITED)) {
    start_wait = GetTickCount();
  }
  intptr_t save = _recursions; // record the old recursion count
  _waiters++;             // increment the number of waiters
  _recursions = 0;        // set the recursion level to be 1
  bool timedout;
  
  exit(THREAD);           // exit the monitor

  // As soon as the ObjectMonitor's ownership is dropped in the exit()
  // call above, another thread can enter() the ObjectMonitor, do the
  // notify(), and exit() the ObjectMonitor. If the other thread's
  // exit() call chooses this thread as the successor and the SetEvent()
  // call happens to occur while this thread is posting a
  // MONITOR_CONTENDED_EXIT event, then we run the risk of the event
  // handler using RawMonitors and consuming the SetEvent().
  //
  // To avoid the problem, we re-post the event. This does no harm
  // even if the original SetEvent() was not consumed because we are the
  // chosen successor for this monitor.
  if (node._notified == 1) {
    BOOL ret = ::SetEvent(node._event);
    assert(ret, "SetEvent() failed!");
  }

  {
    OSThreadWaitState osts(THREAD->osthread(), true /* is Object.wait() */);
    {
      // external suspend retry for this ThreadBlockInVM is in raw_enter()
      ThreadBlockInVM tbivm(jt);
      // Thread is in thread_blocked state and oop access is unsafe in this block.
      
      jt->set_suspend_equivalent();
      // cleared by handle_special_suspend_equivalent_condition() or java_suspend_self()
      timedout = EventWait(node._event, millis);

      // were we externally suspended while we were waiting?
      if (jt->handle_special_suspend_equivalent_condition()) {
        //
        // If we were suspended while we were waiting in EventWait() above,
        // then we don't want to re-enter the monitor while suspended
        // because that would surprise the thread that suspended us.
        // The raw_enter() call below handles the contended monitor
        // case. However, if the thread that notified us has already
        // released the monitor, then we may complete the raw_enter()
        // without contention and without self-suspending.
        //
        jt->java_suspend_self();
      }
    } // End thread_blocked. 

    // Thread state is thread_in_vm and oop access is safe.
    // re-enter contended monitor after object.wait().
    // Post monitor waited event.
    if (JvmtiExport::should_post_monitor_waited()) {
      JvmtiExport::post_monitor_waited(jt, this, timedout);
    }
    // Change java thread status to indicate blocked on monitor re-enter.
    JavaThreadBlockedOnMonitorEnterState jtbmes(jt, this);
    
    {
      // Thread is in blocked state and oop access is unsafe. 
      ThreadBlockInVM tbivm(jt);
      // retain OSThread OBJECT_WAIT state until re-enter successfully completes (legacy code).
      raw_enter(THREAD, false);          // re-enter the monitor
    }
  } // End of OSThreadWaitState

  jt->set_current_waiting_monitor(NULL);

  if (jvmpi::is_event_enabled(JVMPI_EVENT_MONITOR_WAITED)) {
    DWORD end_wait = GetTickCount();
    // the following code is to handle the GetTickCount()
    // wrap as documented by MSDN
    long millis_wait = (end_wait >= start_wait) ? (end_wait - start_wait) :
      (ULONG_MAX - start_wait + end_wait);
    jvmpi::post_monitor_waited_event((oop)object(), millis_wait);
  }
  _recursions = save;     // restore the old recursion count
  _waiters--;             // decrement the number of waiters

  // NOTE: this line must be here, because a notification
  // may be right after the timeout, but before re-enter
  // so the event may still be in signed state
  BOOL ret = ::ResetEvent(node._event);
  assert(ret, "ResetEvent() failed!");
  // check if the notification happened
  if (node._notified == 0) {
    // no, it could be timeout or Thread.interrupt() or both
    // dequeue the the node from the single linked list
    dequeue2(&node);
    // check for interrupt event, otherwise it is timeout
    if (interruptable && Thread::is_interrupted(THREAD, true) && !HAS_PENDING_EXCEPTION) {
      THROW(vmSymbols::java_lang_InterruptedException());
    }
  }
  // NOTES: Suprious wake up will be consider as timeout.
  // Monitor notify has precedence over thread interrupt.
}

void ObjectMonitor::notify(TRAPS) {
  check(CHECK);
  ObjectWaiter* iterator;
  if ((iterator = dequeue()) != 0) {
    iterator->_notified = 1;
    BOOL ret = ::SetEvent(iterator->_event);
    assert(ret, "SetEvent() failed!");
  }
}

void ObjectMonitor::notifyAll(TRAPS) {
  check(CHECK);
  ObjectWaiter* iterator;
  while ((iterator = dequeue()) != 0) {
    iterator->_notified = 1;
    BOOL ret = ::SetEvent(iterator->_event);
    assert(ret, "SetEvent() failed!");
  }
}

// used for JVMTI/JVMPI raw monitor implementation
// Also used by ObjectMonitor::wait().
// Any JavaThread will enter here with state _thread_blocked
int ObjectMonitor::raw_enter(TRAPS, bool isRawMonitor) {
  bool uncontended;

  // don't enter raw monitor if thread is being externally suspended, it will
  // surprise the suspender if a "suspended" thread can still enter monitor
  if (THREAD->is_Java_thread()) {
    JavaThread * jt = (JavaThread *)THREAD;
    jt->SR_lock()->lock_without_safepoint_check();
    while (jt->is_external_suspend()) {
      jt->SR_lock()->unlock();

      jt->java_suspend_self();

      jt->SR_lock()->lock_without_safepoint_check();
    }

    // guarded by SR_lock to avoid racing with new external suspend requests.
    uncontended = (Atomic::cmpxchg_ptr(1, &_count, 0) == 0);

    jt->SR_lock()->unlock();
  } else {
    uncontended = (Atomic::cmpxchg_ptr(1, &_count, 0) == 0);
  }

  // cmpxchg_ptr is defined to do a fence.

  if (uncontended) {
    _owner = THREAD;
  } else if (_owner == THREAD) {
    _recursions++;
  } else {
    if (isRawMonitor) {
      if (jvmpi::is_event_enabled(JVMPI_EVENT_RAW_MONITOR_CONTENDED_ENTER)) {
        jvmpi::post_raw_monitor_contended_enter_event((RawMonitor *)this);
      }
    }
    if (Atomic::add_ptr(1, &_count) > 1) {
      THREAD->set_current_pending_monitor(this);
      if (!THREAD->is_Java_thread()) {
        // No other non-Java threads besides VM thread should acquire
        // a raw monitor.
        assert(THREAD->is_VM_thread(), "must be VM thread");
        DWORD ret = ::WaitForSingleObject(_semaphore, INFINITE);
        assert(ret == WAIT_OBJECT_0, "WaitForSingleObject() failed!");
      } else {
        bool threadIsSuspended;
        JavaThread * jt = (JavaThread *)THREAD;
        do {
          jt->set_suspend_equivalent();
          // cleared by handle_special_suspend_equivalent_condition() or
          // java_suspend_self()

	  DWORD ret = ::WaitForSingleObject(_semaphore, INFINITE);
	  assert(ret == WAIT_OBJECT_0, "WaitForSingleObject() failed!");

          // were we externally suspended while we were waiting?
          threadIsSuspended = jt->handle_special_suspend_equivalent_condition();
          if (threadIsSuspended) {
            //
            // This logic isn't needed for JVMTI or JVM/PI raw monitors,
            // but doesn't hurt just in case the suspend rules change. This
            // logic is needed for the ObjectMonitor.wait() reentry phase.
            // We have reentered the contended monitor, but while we were
            // waiting another thread suspended us. We don't want to reenter
            // the monitor while suspended because that would surprise the
            // thread that suspended us.
            //
            BOOL ret = ::ReleaseSemaphore(_semaphore, 1, NULL);
            assert(ret, "ReleaseSemaphore() failed!");
	    jt->java_suspend_self();
          }
        } while (threadIsSuspended);
      }
      THREAD->set_current_pending_monitor(NULL);
    }
    // there should be no ownership
    assert(_owner == NULL, "Fatal error with monitor owner!");
    assert(_recursions == 0, "Fatal error with monitor recursions!");
    // prevent out of order writes
    OrderAccess::release_store_ptr(&_owner, THREAD);
    if (isRawMonitor) {
      if (jvmpi::is_event_enabled(JVMPI_EVENT_RAW_MONITOR_CONTENDED_ENTERED)) {
        jvmpi::post_raw_monitor_contended_entered_event((RawMonitor *)this);
      }
    }
  }
  return OM_OK;
}

// Used for JVMTI, JVMPI raw monitor implementation.
int ObjectMonitor::raw_exit(TRAPS, bool isRawMonitor) {
  if (THREAD != _owner) {
    return OM_ILLEGAL_MONITOR_STATE;
  }
  if (_recursions == 0) {
    if (isRawMonitor) {
      ((RawMonitor*)this)->remove_from_locked_list();
    }
    OrderAccess::release_store_ptr(&_owner, NULL);
    OrderAccess::release();
    if (Atomic::add_ptr(-1, &_count) > 0) {
      BOOL ret = ::ReleaseSemaphore(_semaphore, 1, NULL);
      assert(ret, "ReleaseSemaphore() failed!");
      if (isRawMonitor) {
        if (jvmpi::is_event_enabled(JVMPI_EVENT_RAW_MONITOR_CONTENDED_EXIT)) {
          jvmpi::post_raw_monitor_contended_exit_event((RawMonitor *)this);
        }
      }
    }
  } else {
    _recursions--;
  }
  return OM_OK;
}

// Used for JVMTI, JVMPI raw monitor implementation.
// All JavaThreads will enter here with state _thread_blocked
int ObjectMonitor::raw_wait(jlong millis, bool interruptable, TRAPS) {
  if (THREAD != _owner) {
    return OM_ILLEGAL_MONITOR_STATE;
  }
  // check interrupt event
  if (interruptable && Thread::is_interrupted(THREAD, true)) {
    return OM_INTERRUPTED;
  }
  // create a node to be put into the queue
  ObjectWaiter node(THREAD, THREAD->osthread()->interrupt_event());
  // enter the waiting queue, which is single linked list in this case
  // how it could be a priority queue, or any data structure
  enqueue(&node);

  intptr_t save = _recursions; // record the old recursion count
  _waiters++;             // increment the number of waiters
  _recursions = 0;        // set the recursion level to be 1

  if (THREAD->is_Java_thread()) {
    ((JavaThread *)THREAD)->set_suspend_equivalent();
  }

  raw_exit(THREAD, false);// exit the monitor

  EventWait(node._event, millis);

  // were we externally suspended while we were waiting?
  if (THREAD->is_Java_thread() &&
     ((JavaThread *)THREAD)->handle_special_suspend_equivalent_condition()) {
      //
      // If we were suspended while we were waiting in EventWait() above,
      // then we don't want to re-enter the monitor while suspended
      // because that would surprise the thread that suspended us.
      // The raw_enter() call below handles the contended monitor
      // case. However, if the thread that notified us has already
      // released the monitor, then we may complete the raw_enter()
      // without contention and without self-suspending.
      //
      ((JavaThread*) THREAD)->java_suspend_self();
  }

  raw_enter(THREAD, false);      // re-enter the monitor
  _recursions = save;     // restore the old recursion count
  _waiters--;             // decrement the number of waiters

  // NOTE: this line must be here, because a notification
  // may be right after the timeout, but before re-enter
  // so the event may still be in signed state
  BOOL ret = ::ResetEvent(node._event);
  assert(ret, "ResetEvent() failed!");
  // check if the notification happened
  if (node._notified == 0) {
    // no, it could be timeout or Thread.interrupt() or both
    // dequeue the the node from the single linked list
    dequeue2(&node);
    // check for interrupt event, otherwise it is timeout
    if (interruptable && Thread::is_interrupted(THREAD, true)) {
      return OM_INTERRUPTED;
    }
  }
  // NOTES: Suprious wake up will be consider as timeout.
  // Monitor notify has precedence over thread interrupt.
  return OM_OK;
}

int ObjectMonitor::raw_notify(TRAPS) {
  if (THREAD != _owner) {
    return OM_ILLEGAL_MONITOR_STATE;
  }
  ObjectWaiter* iterator;
  if ((iterator = dequeue()) != 0) {
    iterator->_notified = 1;
    BOOL ret = ::SetEvent(iterator->_event);
    assert(ret, "SetEvent() failed!");
  }
  return OM_OK;
}

int ObjectMonitor::raw_notifyAll(TRAPS) {
  if (THREAD != _owner) {
    return OM_ILLEGAL_MONITOR_STATE;
  }
  ObjectWaiter* iterator;
  while ((iterator = dequeue()) != 0) {
    iterator->_notified = 1;
    BOOL ret = ::SetEvent(iterator->_event);
    assert(ret, "SetEvent() failed!");
  }
  return OM_OK;
}

// %%% should move this to objectMonitor.cpp
void ObjectMonitor::check_slow(TRAPS) {
  // called only from check()
  assert(THREAD != _owner && !THREAD->is_lock_owned((address) _owner), "must not be owner");
  THROW_MSG(vmSymbols::java_lang_IllegalMonitorStateException(), "current thread not owner");
}

#ifndef PRODUCT
void ObjectMonitor::verify() {
}

void ObjectMonitor::print() {
}
#endif
