#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)mutex_solaris.cpp	1.44 04/04/16 10:52:54 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_mutex_solaris.cpp.incl"

// put OS-includes here
# include <signal.h>

// Implementation of Mutex

// A simple Mutex for VM locking: it is not guaranteed to interoperate with
// the fast object locking, so exclusively use Mutex locking or exclusively
// use fast object locking.

Mutex::Mutex(int rank, const char *name, bool allow_vm_block)
  debug_only( : _rank(rank) )
{
  _lock_event     = new os::Solaris::Event;
  _suppress_signal = false;
  _owner          = INVALID_THREAD;
  _name           = name;

#ifndef PRODUCT
  _lock_count     = -1; // unused in solaris
  _allow_vm_block = allow_vm_block;
  debug_only(_next = NULL;)
  debug_only(_last_owner = INVALID_THREAD;)
#endif
}

Mutex::~Mutex() {  
  os::Solaris::Event* const _Lock_Event = (os::Solaris::Event*)_lock_event;

  assert(_owner == INVALID_THREAD, "Owned Mutex being deleted");
  assert(_lock_count == -1, "Mutex being deleted with non -1 lock count");
  delete _Lock_Event;
}


void Mutex::unlock() {
  os::Solaris::Event* const _Lock_Event = (os::Solaris::Event*)_lock_event;

  assert(_owner == Thread::current(), "Mutex not being unlocked by owner");

  set_owner(INVALID_THREAD);

  if (_suppress_signal) {
    assert(SafepointSynchronize::at_vmthread_safepoint(), "can't sneak");
    _suppress_signal = false;
  }
  else {
    assert(_lock_count >= 0, "Mutex being unlocked without positive lock count");
    debug_only(_lock_count--;)
    _Lock_Event->unlock();
  }
}


// Can be called by non-Java threads (JVM_RawMonitorExit)
void Mutex::jvm_raw_unlock() {
  os::Solaris::Event* const _Lock_Event = (os::Solaris::Event*)_lock_event;
  // Do not call set_owner, as this would break.
  _owner = INVALID_THREAD;
  if (_suppress_signal) {
    assert(SafepointSynchronize::at_vmthread_safepoint(), "can't sneak");
    _suppress_signal = false;
  }
  else {
    debug_only(_lock_count--;)
    _Lock_Event->unlock();
  }
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
      // waiting another thread suspended us. We don't want to enter
      // the Mutex while suspended because that would surprise the
      // thread that suspended us.
      //
      debug_only(_lock_count--;)
      ((os::Solaris::Event*)_lock_event)->unlock();
      thread->java_suspend_self();
    }
  } while (threadIsSuspended);
}


#ifndef PRODUCT
void Mutex::print() const {
  os::Solaris::Event* const _Lock_Event = (os::Solaris::Event*)_lock_event;

  tty->print_cr("Mutex: [0x%lx/0x%lx] %s - owner: 0x%lx", this, _Lock_Event, _name, _owner);
}
#endif


//
// Monitor
//


Monitor::Monitor(int rank, const char *name, bool allow_vm_block) : Mutex(rank, name, allow_vm_block) {
  _event   = NULL;		
  _counter = 0;
  _tickets = 0;
  _waiters = 0;
}


Monitor::~Monitor() {
}  


bool Monitor::wait(bool no_safepoint_check, long timeout) {
  os::Solaris::Event* const _Lock_Event = (os::Solaris::Event*)_lock_event;
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

  long c = _counter;

#ifdef ASSERT
  // Don't catch signals while blocked; let the running threads have the signals.
  // (This allows a debugger to break into the running thread.)
  sigset_t oldsigs;
  sigset_t* allowdebug_blocked = os::Solaris::allowdebug_blocked_signals();
  thr_sigsetmask(SIG_BLOCK, allowdebug_blocked, &oldsigs);
#endif

  _waiters++;
  // Loop until condition variable is signaled.  Tickets will
  // reflect the number of threads which have been notified. The counter
  // field is used to make sure we don't respond to notifications that
  // have occurred *before* we started waiting, and is incremented each
  // time the condition variable is signaled.
  // Use a ticket scheme to guard against spurious wakeups.
  int wait_status;

  while (true) {

    if (no_safepoint_check) {

      // conceptually set the owner to INVALID_THREAD in anticipation of yielding the lock in wait
      set_owner(Mutex::INVALID_THREAD);

      // (SafepointTimeout is not implemented)
      if(timeout == 0) {
	wait_status = _Lock_Event->wait();
      }
      else {
	wait_status = _Lock_Event->timedwait(timeout);
      }
    } else {
      JavaThread *jt = (JavaThread *)thread;
      // save thread state around the lock

      // conceptually set the owner to INVALID_THREAD in anticipation of yielding the lock in wait
      set_owner(Mutex::INVALID_THREAD);

      // Enter safepoint region
      ThreadBlockInVM tbivm(jt);
      OSThreadWaitState osts(thread->osthread(), false /* not Object.wait() */);

      jt->set_suspend_equivalent();
      // cleared by handle_special_suspend_equivalent_condition() or java_suspend_self()
      if(timeout == 0) {
        wait_status = _Lock_Event->wait();
      }
      else {
        wait_status = _Lock_Event->timedwait(timeout);
      }

      // were we externally suspended while we were waiting?
      if (jt->handle_special_suspend_equivalent_condition()) {
        //
	// Our event wait has finished and we own the _Lock_Event, but
	// while we were waiting another thread suspended us. We don't
	// want to hold the _Lock_Event while suspended because that
	// would surprise the thread that suspended us.
        //
        _Lock_Event->unlock();
        jt->java_suspend_self();
        _Lock_Event->lock();
      }
    } // if no_safepoint_check

    // conceptually reaquire the lock (the actual Solaris lock is already reacquired after waiting)
    set_owner(thread);

    // We get to this point if either:
    // a) a notify has been executed by some other thread and woke us up
    // b) a signal has been delivered to this thread and terminated wait
    // c) the above two events happened while we were waiting - that is a signal
    //    was delivered while notify was executed by some other thread.

    // Handle cases a) and c) here. We consume one ticket even in case c) when notify
    // and a signal arrive together
    if (_tickets != 0 && _counter != c) {
      break;
    }
    
    // If wait was interrupted by a signal or timeout, do not use up a ticket
    if (wait_status == EINTR || wait_status == ETIME) {
      ++_tickets;		// will be decremented again below
      break;
    }


  }
  _waiters--;
  _tickets--;

#ifdef ASSERT
  thr_sigsetmask(SIG_SETMASK, &oldsigs, NULL);
#endif
   
  // return true if timed out
  return (wait_status == ETIME);
}


// Notify a single thread waiting on this condition variable
bool Monitor::notify() {
  os::Solaris::Event* const _Lock_Event = (os::Solaris::Event*)_lock_event;

  assert(_owner != INVALID_THREAD, "notify on unknown thread");
  assert(_owner == Thread::current(), "notify on Monitor not by owner");

  if (_waiters > _tickets) {
    
    _Lock_Event->signal();
    
    _tickets++;
    _counter++;
  }

  return true;

}


// Notify all threads waiting on this ConditionVariable
bool Monitor::notify_all() {
  os::Solaris::Event* const _Lock_Event = (os::Solaris::Event*)_lock_event;

  assert(_owner != INVALID_THREAD, "notify on unknown thread");
  assert(_owner == Thread::current(), "notify on Monitor not by owner");

  if (_waiters > 0) {

    _Lock_Event->broadcast();

    _tickets = _waiters;
    _counter++;
  }

  return true;
}

// JSR166
// -------------------------------------------------------

/*
 * The solaris and linux implementations of park/unpark are fairly
 * conservative for now, but can be improved. They currently use a
 * mutex/condvar pair, plus a a count (reused from Monitor::_counter)
 * Park decrements count if > 0, else does a condvar wait.  Unpark
 * sets count to 1 and signals condvar.  There are a few
 * simplifications and specialization wrt Monitor::wait though: Only
 * one thread ever waits on the condvar. Contention seen when trying
 * to park implies that someone is unparking you, so don't wait. And
 * spurious returns are fine, so there is no need to track
 * notifications.
 *
 * (I lifted as much code+comments as possible from Monitor::wait
 * here to try to make the correspondances clear - dl)
 */

Parker::Parker() 
  // Even though park can block, we must act as "leaf" wrt rank rules.
  // Otherwise, rank assertion can incorreclty trigger when unpark is
  // called from Thread::interrupt.
  : Monitor(Mutex::leaf, "Park Semaphore", true) {
}

Parker::~Parker() {
}  

#define NANOSECS_PER_SEC 1000000000

void Parker::park(bool isAbsolute, jlong time) {
  // First, demultiplex/decode time arguments
  timestruc_t  absTime;
  if (time < 0) { // don't wait at all
    return; 
  }
  else if (time > 0) {
    struct timeval now;
    int todstatus = gettimeofday(&now, NULL);
    assert(todstatus == 0, "gettimeofday");
    
    if (!isAbsolute) {
      // convert to absolute time (could replace with solaris cond_relwait)
      absTime.tv_sec = time / NANOSECS_PER_SEC + now.tv_sec;
      absTime.tv_nsec = (time % NANOSECS_PER_SEC) + now.tv_usec * 1000;
      if (absTime.tv_nsec > NANOSECS_PER_SEC) {
        absTime.tv_nsec -= NANOSECS_PER_SEC;
        ++absTime.tv_sec;
      }
    }
    else {
      absTime.tv_sec = time / 1000;
      absTime.tv_nsec = (time % 1000) * (1000 * 1000);
    }
    
    // Solaris cond_timedwait(3T) says arg must be within 
    // current_time  + 100000000. So we need to
    // check and possibly saturate
    
    if (absTime.tv_sec >= now.tv_sec + 100000000) {
      absTime.tv_sec = now.tv_sec + 100000000;
      absTime.tv_nsec = 0;
    }
  }

  Thread* thread = Thread::current();
  assert(thread->is_Java_thread(), "Must be JavaThread");
  JavaThread *jt = (JavaThread *)thread;
  os::Solaris::Event* const mon = (os::Solaris::Event*)_lock_event;

  // Don't wait if cannot get lock since interference arises from
  // unblocking.  Also. check interrupt before trying wait
  if (Thread::is_interrupted(thread, false) || !try_lock()) {
    return;
  }

#ifdef ASSERT
  // (The following checks lifted from Monitor::wait - dl)
  assert(_owner != INVALID_THREAD, "Wait on unknown thread");
  assert(_owner == thread, "Wait on Monitor not by owner");
  
  // The design rule for use of mutexes of rank special or less is
  // that we are guaranteed not to block while holding such mutexes.
  // Here we verify that the least ranked mutex that we hold,
  // modulo the mutex we are about to relinquish, satisfies that
  // constraint, since we are about to block in a wait.
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

  if (_counter > 0)  { // no wait needed
    _counter = 0;
    unlock();
    return;
  }

#ifdef ASSERT
  // Don't catch signals while blocked; let the running threads have the signals.
  // (This allows a debugger to break into the running thread.)
  sigset_t oldsigs;
  sigset_t* allowdebug_blocked = os::Solaris::allowdebug_blocked_signals();
  thr_sigsetmask(SIG_BLOCK, allowdebug_blocked, &oldsigs);
#endif
  
  // conceptually set the owner to INVALID_THREAD in anticipation of yielding the lock in wait
  set_owner(Mutex::INVALID_THREAD);
  
  // Enter safepoint region
  ThreadBlockInVM tbivm(jt);
  OSThreadWaitState osts(thread->osthread(), false /* not Object.wait() */);
  jt->set_suspend_equivalent();
  // cleared by handle_special_suspend_equivalent_condition() or java_suspend_self()
  
  if (time == 0) {
    mon->wait();
  }
  else {
    mon->timedwait(&absTime);
  }

  // conceptually reaquire the lock (the actual lock is already reacquired after waiting)
  set_owner(thread);

#ifdef ASSERT
  thr_sigsetmask(SIG_SETMASK, &oldsigs, NULL);
#endif

  unlock();
  // If externally suspended while waiting, re-suspend
  if (jt->handle_special_suspend_equivalent_condition()) {
    jt->java_suspend_self();
  }

}

void Parker::unpark() {
  int s;
  lock();
  s = _counter;
  _counter = 1;
  unlock();

  if (s < 1) {
    os::Solaris::Event* const mon = (os::Solaris::Event*)_lock_event;
    mon->signal();
  }
}


//Reconciliation History
// 1.15 99/01/05 11:27:27 mutex_win32.cpp
// 1.20 99/06/28 11:01:40 mutex_win32.cpp
//End
