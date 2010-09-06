#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)objectMonitor_solaris.cpp	1.66 04/04/19 11:35:38 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_objectMonitor_solaris.cpp.incl"

typedef os::Solaris::Event* event_t;

// A macro is used below because there may already be a pending
// exception which should not abort the execution of the routines
// which use this (which is why we don't put this into check_slow and
// call it with a CHECK argument).
#define CHECK_OWNER() \
  do { \
    if (THREAD != _owner) { \
      if (THREAD->is_lock_owned((address) _owner)) { \
        _owner = THREAD ;  /* Convert from basiclock addr to Thread addr */ \
        _recursions = 0; \
      } else { \
        THROW(vmSymbols::java_lang_IllegalMonitorStateException()); \
      } \
    } \
  } while (false)

class ObjectWaiter : public StackObj {
  // ObjectWaiter serves as a "proxy" or surrogate thread.
public:
  enum TStates { TS_UNDEF, TS_READY, TS_RUN, TS_WAIT, TS_ENTER } ; 
  ObjectWaiter* _next;
  ObjectWaiter* _prev;
  Thread*       _thread;
  volatile intptr_t  _notified;
  volatile TStates TState ; 
  event_t       _event;
public:
  ObjectWaiter(Thread* thread) {
    _next     = NULL;
    _prev     = NULL;
    _thread   = thread;
    _notified = 0;
    _event    = thread->osthread()->interrupt_event() ; 
    TState    = TS_RUN ; 
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
  _succ	      = NULL ; 
  _EntryQueue = NULL ; 
  _QMix	      = 0 ; 
}

ObjectMonitor::~ObjectMonitor() {
}

intptr_t ObjectMonitor::is_busy() const {
  return (_count|_waiters|(intptr_t)_owner);
}

void ObjectMonitor::Recycle () {
  _succ	      = NULL ; 
  _EntryQueue = NULL ; 
  _QMix	      = 0 ; 
}


inline void ObjectMonitor::enqueue(ObjectWaiter* node) {
  assert(node != NULL, "should not dequeue NULL node");
  assert(node->_prev == NULL, "node already in list");
  assert(node->_next == NULL, "node already in list");
  // put node at end of queue (circular doubly linked list)
  if (_queue == NULL) {
    _queue = node;
    node->_prev = node;
    node->_next = node;
  } else {
    ObjectWaiter* head = (ObjectWaiter*)_queue;
    ObjectWaiter* tail = head->_prev;
    assert(tail->_next == head, "invariant check");
    tail->_next = node;
    head->_prev = node;
    node->_next = head;
    node->_prev = tail;
  }
}

inline ObjectWaiter* ObjectMonitor::dequeue() {
  // dequeue the very first waiter
  ObjectWaiter* waiter = (ObjectWaiter*)_queue;
  if (waiter) {
    dequeue2(waiter);
  }
  return waiter;
}

inline void ObjectMonitor::dequeue2(ObjectWaiter* node) {
  assert(node != NULL, "should not dequeue NULL node");
  assert(node->_prev != NULL, "node already removed from list");
  assert(node->_next != NULL, "node already removed from list");
  // when the waiter has woken up because of interrupt,
  // timeout or other suprious wake-up, dequeue the 
  // waiter from waiting list
  ObjectWaiter* next = node->_next;
  if (next == node) {
    assert(node->_prev == node, "invariant check");
    _queue = NULL;
  } else {
    ObjectWaiter* prev = node->_prev;
    assert(prev->_next == node, "invariant check");
    assert(next->_prev == node, "invariant check");
    next->_prev = prev;
    prev->_next = next;
    if (_queue == node) {
      _queue = next;
    }
  }
  node->_next = NULL;
  node->_prev = NULL;
}

void ObjectMonitor::EntryQueue_insert (ObjectWaiter * node, int PrePend) { 
  assert(node != NULL, "should not dequeue NULL node");
  assert(node->_prev == NULL, "node already in list");
  assert(node->_next == NULL, "node already in list");
  debug_only(_mutex.verify_locked();)
  if (_EntryQueue == NULL) {
    _EntryQueue = node;
    node->_prev = node;
    node->_next = node;
  } else {
    ObjectWaiter* head = _EntryQueue ;
    ObjectWaiter* tail = head->_prev;
    assert(tail->_next == head, "invariant check");
    tail->_next = node;
    head->_prev = node;
    node->_next = head;
    node->_prev = tail;
  	if (PrePend) _EntryQueue = node ; 
  }
}

void ObjectMonitor::EntryQueue_unlink (ObjectWaiter* node) {
  assert(node != NULL, "should not dequeue NULL node");
  assert(node->_prev != NULL, "node already removed from list");
  assert(node->_next != NULL, "node already removed from list");
  debug_only(_mutex.verify_locked();)
  ObjectWaiter* next = node->_next;
  if (next == node) {
    assert(node->_prev == node, "invariant check");
    _EntryQueue = NULL;
  } else {
    ObjectWaiter* prev = node->_prev;
    assert(prev->_next == node, "invariant check");
    assert(next->_prev == node, "invariant check");
    next->_prev = prev;
    prev->_next = next;
    if (_EntryQueue == node) {
      _EntryQueue = next;
    }
  }
  node->_next = NULL;
  node->_prev = NULL;
}

// Pick an appropriate successor ("heir presumptive") from the 
// EntryQueue and unlink it. 
// 
// In the future we'll try to bias the selection mechanism
// to preferentially pick a thread that recently ran on 
// a processor element that shares cache with the CPU on which
// the exiting thread is running.   We need access to Solaris'
// schedctl.sc_cpu to make that work.  

ObjectWaiter * ObjectMonitor::EntryQueue_SelectSuccessor () {
  ObjectWaiter * node ; 
  node = _EntryQueue ; 
  if (node == NULL) return NULL ; 

  assert(node->_prev != NULL, "node already removed from list");
  assert(node->_next != NULL, "node already removed from list");
  ObjectWaiter* next = node->_next;
  if (next == node) {
    assert(node->_prev == node, "invariant check");
    _EntryQueue = NULL;
  } else {
    ObjectWaiter* prev = node->_prev;
    assert(prev->_next == node, "invariant check");
    assert(next->_prev == node, "invariant check");
    next->_prev = prev;
    prev->_next = next;
    if (_EntryQueue == node) {
      _EntryQueue = next;
    }
  }
  assert (node != _EntryQueue, "invariant check") ; 
  node->_next = NULL;
  node->_prev = NULL;
  return node ; 
}

// Spin-then-block strategies ...
//
// In some circumstances it makes sense use a brief, bounded spin 
// in EnterI().  We can spin (a) on initial contention only, or (b) 
// in each iteration of the loop in Enter(), prior to blocking. 
//
// Naive "uninformed" spinning is a horrible idea.  It doesn't scale.  
// There are a number of schemes we could use to rationalize spinning:
//
// a.	Use informed-spinning.  Apply the schedctl.sc_state & SC_ONPROC
//	test to _owner.  Spin +only+ if the onwer is ONPROC.  Beware
//	of references to schedctl blocks, however.  Because of fork() issues
//	the schedctl block can disappear without warning.  That means we need
//	to use either non-faulting loads or make the SEGV/BUS signal handler 
//	complicit by way of a "SafeFetchWord()" helper.  In addition, the
//	reader must tolerate fetches of undefined values from sc_state.  
//	Despite those restrictions, schedctl.sc_state is likely the best
//	solution.  
//
// b.	We could also check the _owner's thread_state() value and 
//	only spin if the owner is _thread_in_Java or _thread_in_vm.  
//	Of course the owner might be blocked on a page-fault or 
//	be preempted and sitting on a ready/dispatch queue, so this
//	mechanism is less informed than (a).  In addition _owner is
//	sometimes a BasicLock address and sometimes a Thread address,
//	so we'd need to add logic to differentiate the two (or make
//	the mechanism 'garbage value' tolerant if we were to fetch
//	junk from _owner->_thread_state.  Finally, there are lifecycle
//	and liveness issues concerning the fetched _owner pointer.  
//	_owner might disappear while a spinning holds a pointer to it.
//	Again, we could use SafeFetchWord() or non-faulting LDs to
//	get around that problem.    
//
// c.	Periodically sample getloadavg() and spin only if the system
//	is less than fully saturated.  Alternately, we could mmap()
//	a read-only kernel page that contained a word with the # of 
//	idle CPUs.  (Spinning only if there are idle CPUs is overly
//	conservative).  Without damping this scheme could "ring" or 
//	oscillate - it's a feedback mechanism. 
//
// d.	Use reactive or adaptive spinning.  Some papers suggest varying 
//	the spin count adaptively based on recent successful spin history.  
//	Instead, I've prototyped a version that uses a fixed spin count but
//	varies the spin frequency using a spin-1-out-of-every-N attempts policy. 
//	N varies based on the rate of recent succesful spin attempts.  
//	Initial data showed this to be effective.  (d) is likely the next
//	best choice after (a).  
// 
// Variations:
//
// *	Don't spin if there are more than N = (CPUs/2) threads
//	currently spinning on the monitor.  That is, limit the number
//	of concurrent spinners. 
//
// *	If a spinning thread observes _owner change hands it should
//	abort the spin and block immediately.
// 
// *	Avoid transitive spinning.
//
// *	Use PAUSE (rep:nop) on IA32. 
//
// *	Classically, the spin count is either K*(CPUs-1) or is a
//	simple constant that approximatees the length of a context switch.  
//

void ObjectMonitor::EnterI (TRAPS) {  
  static int AppendFrq = -1 ;  
 
  if (AppendFrq < 0) {  
     AppendFrq = (1 << AppendRatio) - 1 ;   
  }  

  for (;;) {
    // Try the lock - TATAS  
    if (_owner == NULL && Atomic::cmpxchg_ptr (THREAD, &_owner, NULL) == NULL) break ;   
  
    // Consider spinning here ... see comments above ...  
       
    // Enqueue "Self" on ObjectMonitor's EntryQueue 
    ObjectWaiter node(THREAD) ;   
    node.TState = ObjectWaiter::TS_ENTER ;  
    node._event->reset() ; 
    _mutex.lock () ;   
    if (_owner == NULL) {           // optimization - not strictly necessary
        _mutex.unlock() ; 
        continue;  
    } 
    EntryQueue_insert (&node, (++_QMix) & AppendFrq) ;   
    OrderAccess::fence();           // ST _EntryQ; membar #storeload; LD _owner  
    if (_owner == NULL) {           // resample _owner  
        // TODO: Consider CAS() attempt to acquire lock  
        EntryQueue_unlink (&node);  
        _mutex.unlock () ;   
        continue ;   
    }  
    _mutex.unlock () ;   
    node._event->park();           // Park self  
       
  
    // CONSIDER: keep perf stats on futile wakeup counts  
    // Use a double-checked locking idiom on node.TState.  
    // (Doubled-checked locking is flawed in Java's memory  
    // model but is safe in this specific context).    
  
    if (node.TState == ObjectWaiter::TS_ENTER) {   
       _mutex.lock ();  
       if (node.TState == ObjectWaiter::TS_ENTER) {  
          EntryQueue_unlink (&node);                               
       }   
       _mutex.unlock () ;   
    }   
    if (_succ == &node) _succ = NULL ;   
    OrderAccess::fence() ;	      // Not rqrd for TSO/SPO/PO.  
  }  
  guarantee (_owner == THREAD && _succ != THREAD, "invariant violation") ;  
  
  // The park() call above may have consumed a pending unpark() associated  
  // with an interrupt.  Subsequent() wait() and sleep() operations need to test  
  // for pending async exceptions before blocking.  
}

// enter2() - contended entry
		 
void ObjectMonitor::enter2(TRAPS) {
  guarantee (_owner != THREAD, "invariant violation") ; 
  assert(THREAD->is_Java_thread(), "Must be Java thread!");
  JavaThread *jt = (JavaThread *)THREAD;

  Atomic::inc_ptr(&_count);
  {
    // Change java thread status to indicate blocked on monitor enter.
    JavaThreadBlockedOnMonitorEnterState jtbmes(jt, this);

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

      EnterI (THREAD) ; 

      // were we externally suspended while we were waiting?
      threadIsSuspended = jt->handle_special_suspend_equivalent_condition();
      if (threadIsSuspended) {
        //
        // We have acquired the contended monitor, but while we were
        // waiting another thread suspended us. We don't want to enter
        // the monitor while suspended because that would surprise the
        // thread that suspended us.
        //
	_recursions = 0 ; 
	_succ = NULL ; 
	exit (THREAD) ; 
        jt->java_suspend_self();
      }
    } while (threadIsSuspended);
    THREAD->set_current_pending_monitor(NULL);

    // CONSIDER: guarantee _recursions = 0.  
  }

  // Thread is back in vm mode
  if (JvmtiExport::should_post_monitor_contended_entered()) {
    JvmtiExport::post_monitor_contended_entered(jt, this);
  }
  if (jvmpi::is_event_enabled(JVMPI_EVENT_MONITOR_CONTENDED_ENTERED)) {
    jvmpi::post_monitor_contended_entered_event(object());
  }

  Atomic::dec_ptr(&_count);
}


void ObjectMonitor::enter(TRAPS) {
  // TODO-FIXME: in 1.5.1 we'll want to reorder this code to avoid
  // RTS->RTO upgrades on SPARC.  Specifically, we'll want to change the
  // ordering to:
  // 1. Try CAS().  Avoid fetching _owner before the CAS(). 
  // 2. Test recursion
  // 3. Try to convert _Owner from a basiclock-ed stack pointer to a Thread pointer.
  // 4. enter2() -- contention. 

  if (THREAD != _owner) {
    if (THREAD->is_lock_owned ((address)_owner)) {
	   assert(_recursions == 0, "internal state error");
       _owner = THREAD ;
       _recursions = 1 ;
	   return ; 
    }
    if (Atomic::cmpxchg_ptr (THREAD, &_owner, NULL) != NULL) {
       enter2(THREAD) ;    // true contention 
    }
    // CONSIDER: it'd seem sensible to either set _recursions to 0
    // or ASSERT _recursions == 0.  That logic isn't present in 
    // the original form, however.  
  } else {
    _recursions++;
  }
  // Invariant: we never return to java-code holding _mutex.  
  assert (_owner == THREAD, "invariant") ; 
}

enum { SignalAfter = true } ; 

void ObjectMonitor::exit(TRAPS) {
  if (THREAD != _owner) {
    if (THREAD->is_lock_owned((address) _owner)) {
      // Transmute _owner from a BasicLock pointer to a Thread address.
      // We don't need to hold _mutex for this transition.  
      // Non-null to Non-null is safe as long as all readers can
      // tolerate either flavor.  
      _owner = THREAD ;
      _recursions = 0 ;
    } else {
      // NOTE: we need to handle unbalanced monitor enter/exit
      // in native code by throwing an exception.a
      // TODO: Throw an IllegalMonitorStateException ?
      assert(false, "Non-balanced monitor enter/exit!");
      return;
    }
  }
  if (_recursions != 0) { 
    _recursions--;        // this is simple recursive enter
    return ; 
  }

  // Contended exit ...
  // Pay attention to the Dekker/Lamport duality.  
  // The contended exit path executes  ST _owner; membar;  LD _EntryQueue.
  // The contended enter path executes ST _EntryQueue; membar; LD _owner. 
  //
  // Note that there's a benign race in the exit path.  We can drop the
  // lock, another thread can reacquire the lock immediately, and we can
  // then wake a thread unnecessarily (yet another flavor of futile wakeup).
  // This is benign, and we've structured the code so the windows are short 
  // and the frequency of such futile wakeups is low.   

  _owner = NULL ;                   // Drop the lock
  OrderAccess::fence();             // ST _owner; membar #storeload; LD _EntryQueue and _succ
  intptr_t savedcount = _count ;    // probabilistic contention indication
  if (_EntryQueue != NULL && _succ == NULL) { 
     ObjectWaiter * wakee = NULL ;
     os::Solaris::Event * Trigger = NULL ; 
     // An interesting optimization is to use _mutex.trylock() instead
     // of _mutex.lock().  If trylock() failed to acquire _mutex we know that 
     // either an enqueue or dequeue operation is taking place concurrently.  
     // In either case progess is assured so we could safely bail-out early 
     // from exit().  This avoid traffic on _mutex and avoids waking 
     // a successor.  The interleavings are quite subtle and the resulting
     // code would be brittle so I'm leaving this for future generations ...
     _mutex.lock () ; 

     // The following (_owner == NULL && _succ == NULL) test is purely
     // an optimization.  The mechanism would properly without the refinement.
     // We can skip waking a successor if some other thread grabbed the lock
     // in the interim.  Succession & progress are now that thread's 
     // responsibility.  
     //
     // The _succ variable is critical in reducing futile wakeup frequency.
     // _succ identifies the "heir presumptive" thread that has been made
     // ready (unparked) but that has not yet run.  We only need one successor
     // to guarantee progress.  
     // See http://www.usenix.org/events/jvm01/full_papers/dice/dice.pdf
     // for details. 

     if (_owner == NULL && _succ == NULL) { 
        wakee = EntryQueue_SelectSuccessor () ; 
        if (wakee != NULL) { 
           _succ = wakee ;              // futile wakeup throttle
           OrderAccess::fence();        // ST-ST -- Critical ordering: 
					// ST to _succ must be visible before ST to TState.
           wakee->TState = ObjectWaiter::TS_RUN ; 
           Trigger = wakee->_event ; 
           if (!SignalAfter) {
              Trigger->unpark() ;          // unpark wakee 
           }
        }
     }
     _mutex.unlock () ; 

     // We can signal the condvar associated with the wakee either inside 
     // or after the critial section defined by _mutex.  Signaling inside 
     // is free of node/Thread/Event lifecycle and liveness issues (IE, is the 
     // Thread/node/Event referenced by wakee still alive?) but can result 
     // in lock jams and futile wakeups on the mutex.  If we elect to 
     // signal inside then the path in Enter() should be designed to avoid
     // re-grabbing the mutex (and lock jams).
     // 
     // Signaling after dropping the _mutex is more efficient but requires 
     // that either the ObjectWaiter, Thread, or the Event reside in TSM.  
     // This implementation maintains the Event in TSM - see osThread::pd_initialize().
     // Signaling a stale Event is benign.  If the Event is stale then - by 
     // definition - the associated thread has made progress.  That gives us
     // the requisite progress properties.  
     if (SignalAfter && Trigger != NULL) { 
        Trigger->unpark() ;               // unpark wakee
     }
  }
		
  // Exit is contended only if count had monitor enter requestors
  if (savedcount > 0) {
    if (jvmpi::is_event_enabled(JVMPI_EVENT_MONITOR_CONTENDED_EXIT)) {
      jvmpi::post_monitor_contended_exit_event(object());
    }
  }
}


void ObjectMonitor::wait(jlong millis, bool interruptible, TRAPS) {
   assert(THREAD->is_Java_thread(), "Must be Java thread!");
   JavaThread *jt = (JavaThread *)THREAD;
 
   CHECK_OWNER();
   // check interrupt event
   if (interruptible && Thread::is_interrupted(THREAD, true) && !HAS_PENDING_EXCEPTION) {
     THROW(vmSymbols::java_lang_InterruptedException());
   }
 
   jt->set_current_waiting_monitor(this);
 
   // create a node to be put into the queue
   ObjectWaiter node(THREAD);
   node.TState = ObjectWaiter::TS_WAIT ; 
   node._event->reset();
   OrderAccess::fence();          // ST into Event; membar ; LD interrupted-flag 
 
   // enter the waiting queue, which is a circular doubly linked list in this case
   // how it could be a priority queue, or any data structure
   _mutex.lock () ;
   enqueue(&node);
   // Implicit membar here ... we can avoid the real thing
   // as Solaris sync primitives provide barrier-equivalent semantics with
   // at least release consistency (RC). 
   _mutex.unlock () ; 
   
   if (jvmpi::is_event_enabled(JVMPI_EVENT_MONITOR_WAIT)) {
     jvmpi::post_monitor_wait_event((oop)object(), millis);
   }
   hrtime_t start_wait;
   if (jvmpi::is_event_enabled(JVMPI_EVENT_MONITOR_WAITED)) {
     start_wait = os::javaTimeNanos();
   }
 
   intptr_t save = _recursions; // record the old recursion count
   _waiters++;             // increment the number of waiters
   _recursions = 0;        // set the recursion level to be 1
   exit (THREAD) ; 	  // exit the monitor

  // As soon as the ObjectMonitor's ownership is dropped in the exit()
  // call above, another thread can enter() the ObjectMonitor, do the
  // notify(), and exit() the ObjectMonitor. If the other thread's
  // exit() call chooses this thread as the successor and the unpark()
  // call happens to occur while this thread is posting a
  // MONITOR_CONTENDED_EXIT event, then we run the risk of the event
  // handler using RawMonitors and consuming the unpark().
  //
  // To avoid the problem, we re-post the event. This does no harm
  // even if the original unpark() was not consumed because we are the
  // chosen successor for this monitor.
  if (node._notified == 1 && _succ == &node) {
    node._event->unpark();
  }

   int ret = OS_OK ; 
   int WasNotified ; 
 
   {
    OSThread* osthread = THREAD->osthread();
    OSThreadWaitState osts(osthread, true /* is Object.wait() */);
    {
       // external suspend retry for this ThreadBlockInVM is in raw_enter()
       ThreadBlockInVM tbivm(jt);
      // Thread is in thread_blocked state and oop access is unsafe in this block.
       jt->set_suspend_equivalent();
 
       if (interruptible && (Thread::is_interrupted(THREAD, false) || HAS_PENDING_EXCEPTION)) {
          // Intentionally empty 
       } else {
          if (millis <= 0) {
             node._event->park();
          } else {
             ret = node._event->park(millis);
          }
       }
 
       // TODO: in the future it makes sense to use double-checked
       // locking to avoid grabbing _mutex if the current thread
       // is on neither the EntryQueue nor the _queue (wait queue).
       // We can fetch node.TState and grab _mutex IFF TState is 
       // not TS_RUN.  
 
       _mutex.lock () ; 
       if (_succ == &node) _succ = NULL ; 
       WasNotified = node._notified ; 
       if (node.TState == ObjectWaiter::TS_WAIT) { 
          dequeue2 (&node); 			// unlink from WaitQueue
       } else 
       if (node.TState == ObjectWaiter::TS_ENTER) { 
          EntryQueue_unlink (&node) ; 	        // unlink from EntryQueue
       } else { 
          guarantee (node.TState == ObjectWaiter::TS_RUN, "invariant violation") ;
       }
       _mutex.unlock () ;
 
       // were we externally suspended while we were waiting?
       if (jt->handle_special_suspend_equivalent_condition()) {
         jt->java_suspend_self();
       }
    } // End thread_blocked and thread in vm.
 
    // Reentry phase -- reacquire the monitor.   
    // TODO: refactor enter(), enter2(), EnterI(), exit() and the various
    // specialized inline copies of enter() and exit().  
    // retain OBJECT_WAIT state until re-enter successfully completes

    // Thread state is thread_in_vm and oop access is safe.
    // re-enter contended monitor after object.wait().

    // post monitor waited event.  Note that this is past-tense, we are done waiting.
    if (JvmtiExport::should_post_monitor_waited()) {
      JvmtiExport::post_monitor_waited(jt, this, (ret == OS_TIMEOUT)?true:false);
    }

    // Change java thread status to indicate blocked on monitor re-enter.
    JavaThreadBlockedOnMonitorEnterState jtbmes(jt, this);  
    {
      ThreadBlockInVM tbivm(jt);
      // Thread is in thread_blocked state and oop access is unsafe in this block.
        
       bool threadIsSuspended ; 
       do {
           jt->set_suspend_equivalent();
           EnterI (THREAD) ; 
 
 	     threadIsSuspended = jt->handle_special_suspend_equivalent_condition();
 	     if (threadIsSuspended) {
 	        // We have reentered the contended monitor, but while we were
 	        // waiting another thread suspended us. We don't want to reenter
 	        // the monitor while suspended because that would surprise the
 	        // thread that suspended us.
 
 	        // Drop the lock - 
            ObjectWaiter * wakee = NULL ;
            _owner = NULL ; 
            _mutex.lock ();
            _succ  = NULL ; 
            wakee = EntryQueue_SelectSuccessor () ;
            if (wakee != NULL) {
               wakee->TState = ObjectWaiter::TS_RUN ; 
               wakee->_event->unpark() ; 		// unpark wakee
            }
            _mutex.unlock();

 	        jt->java_suspend_self();
 	     }
       } while (threadIsSuspended);
     }  // End thread_blocked, thread no thread_in_vm
   } // End OSThreadWaitState.
 
   jt->set_current_waiting_monitor(NULL);
 
   if (jvmpi::is_event_enabled(JVMPI_EVENT_MONITOR_WAITED)) {
     hrtime_t end_wait = os::javaTimeNanos();
     jvmpi::post_monitor_waited_event((oop)object(), (end_wait - start_wait)/1000000L);
   }
   _recursions = save;     // restore the old recursion count
   _waiters--;             // decrement the number of waiters
 
   // NOTE: this line must be here, because a notification
   // may be right after the timeout, but before re-enter
   // so the event may still be in signalled state
   node._event->reset();
 
   // check if the notification happened
   if (!WasNotified) {
     // no, it could be timeout or Thread.interrupt() or both
     // check for interrupt event, otherwise it is timeout
     if (interruptible && Thread::is_interrupted(THREAD, true) && !HAS_PENDING_EXCEPTION) {
       THROW(vmSymbols::java_lang_InterruptedException());
     }
   }

   // NOTE: Spurious wake up will be consider as timeout.
   // Monitor notify has precedence over thread interrupt.
}


void ObjectMonitor::notify(TRAPS) {
  CHECK_OWNER();
  ObjectWaiter* iterator;
  if (_queue == NULL) return ; 
  _mutex.lock () ; 
  if ((iterator = dequeue()) != NULL) {
     guarantee (iterator->TState == ObjectWaiter::TS_WAIT, "invariant violation") ; 
     EntryQueue_insert (iterator, 0) ; 
     iterator->TState = ObjectWaiter::TS_ENTER ; 
     iterator->_notified = 1 ; 
  }
  _mutex.unlock () ;
}

void ObjectMonitor::notifyAll(TRAPS) {
  CHECK_OWNER();
  ObjectWaiter* iterator;
  if (_queue == NULL) return ; 
  _mutex.lock() ; 
  while ((iterator = dequeue()) != NULL) {
     guarantee (iterator->TState == ObjectWaiter::TS_WAIT, "invariant violation") ; 
     EntryQueue_insert (iterator, 0) ; 
     iterator->TState = ObjectWaiter::TS_ENTER ; 
     iterator->_notified = 1 ; 
  }
  _mutex.unlock () ; 
}

// Used mainly for JVMTI, JVMPI raw monitor implementation
// Also used for ObjectMonitor::wait().
// Any JavaThread will enter here with state _thread_blocked
int ObjectMonitor::raw_enter(TRAPS, bool isRawMonitor) {
  void * Contended ; 

  // don't enter raw monitor if thread is being externally suspended, it will
  // surprise the suspender if a "suspended" thread can still enter monitor
  JavaThread * jt = (JavaThread *)THREAD;
  if (THREAD->is_Java_thread()) {
    jt->SR_lock()->lock_without_safepoint_check();
    while (jt->is_external_suspend()) {
      jt->SR_lock()->unlock();
      jt->java_suspend_self();
      jt->SR_lock()->lock_without_safepoint_check();
    }
    // guarded by SR_lock to avoid racing with new external suspend requests.
    Contended = Atomic::cmpxchg_ptr (THREAD, &_owner, NULL) ; 
    jt->SR_lock()->unlock();
  } else { 
    Contended = Atomic::cmpxchg_ptr (THREAD, &_owner, NULL) ; 
  }

  if (Contended == THREAD) {
     _recursions ++ ; 
  } else 
  if (Contended == NULL) { 
     guarantee (_owner == THREAD, "invariant") ; 
  } else { 
    // true contention ...
    // It might be easier to call enter2() directly ...
    if (isRawMonitor) {
      Atomic::inc_ptr(&_count);
      if (jvmpi::is_event_enabled(JVMPI_EVENT_RAW_MONITOR_CONTENDED_ENTER)) {
        jvmpi::post_raw_monitor_contended_enter_event((RawMonitor *)this);
      }
    }

    bool threadIsSuspended;
    THREAD->set_current_pending_monitor(this);
    if (!THREAD->is_Java_thread()) {
      // No other non-Java threads besides VM thread would acquire 
      // a raw monitor.
      assert(THREAD->is_VM_thread(), "must be VM thread");
      EnterI (THREAD) ; 
    } else {
      do {
        jt->set_suspend_equivalent();
        // cleared by handle_special_suspend_equivalent_condition() or
        // java_suspend_self()
        EnterI (THREAD) ; 

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

	   // Drop the lock - 
           ObjectWaiter * wakee = NULL ;
           _owner = NULL ; 
           _mutex.lock ();
           _succ  = NULL ; 
           wakee = EntryQueue_SelectSuccessor () ;
           if (wakee != NULL) {
              wakee->TState = ObjectWaiter::TS_RUN ; 
              wakee->_event->unpark() ; 		// unpark wakee
           }
           _mutex.unlock();

	   jt->java_suspend_self();
	}
      } while (threadIsSuspended);
    }
    THREAD->set_current_pending_monitor(NULL);

    assert(_owner == THREAD, "Fatal error with monitor owner!");
    assert(_recursions == 0, "Fatal error with monitor recursions!");
    if (isRawMonitor) {
      if (jvmpi::is_event_enabled(JVMPI_EVENT_RAW_MONITOR_CONTENDED_ENTERED)) {
        jvmpi::post_raw_monitor_contended_entered_event((RawMonitor *)this);
      }
      Atomic::dec_ptr(&_count);
    }
  }

  if (isRawMonitor) {
    if (_recursions == 0) {
      ((RawMonitor *)this)->add_to_locked_list();
    }
  }
  return OM_OK;
}

// Used mainly for JVMTI, JVMPI raw monitor implementation
// Also used for ObjectMonitor::wait().
int ObjectMonitor::raw_exit(TRAPS, bool isRawMonitor) {
  if (THREAD != _owner) {
    return OM_ILLEGAL_MONITOR_STATE;
  }
  if (_recursions == 0) {
    if (isRawMonitor) {
      ((RawMonitor *)this)->remove_from_locked_list();
    }
	
    // What follows is a slightly simplified form of 
    // ObjectMonitor::exit().  We avoid the optimizations in 
    // exit() as debuggers often embody odd & undocumented
    // dependencies on the behaviour of the synchronization
    // primitives.  (Better to be conservative).  
    intptr_t savedcount = _count;
    ObjectWaiter * wakee = NULL ;
    _owner = NULL ; 
    _mutex.lock ();
    _succ  = NULL ; 
    wakee = EntryQueue_SelectSuccessor () ;
    if (wakee != NULL) {
       wakee->TState = ObjectWaiter::TS_RUN ; 
       wakee->_event->unpark() ; 		// unpark wakee
    }
    _mutex.unlock();

    if (isRawMonitor) {
      if (savedcount > 0) {
        if (jvmpi::is_event_enabled(JVMPI_EVENT_RAW_MONITOR_CONTENDED_EXIT)) {
          jvmpi::post_raw_monitor_contended_exit_event((RawMonitor*)this);
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
int ObjectMonitor::raw_wait(jlong millis, bool interruptible, TRAPS) {
  if (THREAD != _owner) {
    return OM_ILLEGAL_MONITOR_STATE;
  }
  // check interrupt event
  if (interruptible && Thread::is_interrupted(THREAD, true)) {
    return OM_INTERRUPTED;
  }
  // create a node to be put into the queue
  ObjectWaiter node(THREAD);
  node.TState = ObjectWaiter::TS_WAIT ; 
  // enter the waiting queue, which is a circular doubly linked list in this case
  // how it could be a priority queue, or any data structure
  _mutex.lock ();  
  enqueue(&node);
  _mutex.unlock () ; 

  intptr_t save = _recursions; // record the old recursion count
  _waiters++;             // increment the number of waiters
  _recursions = 0;        // set the recursion level to be 1

  if (THREAD->is_Java_thread()) {
    ((JavaThread *)THREAD)->set_suspend_equivalent();
  }
  int ret = OS_OK ; 
  raw_exit(THREAD, false);   // exit the monitor

  if (millis <= 0) {
     node._event->park();
  } else {
     ret = node._event->park(millis);
  }

  _mutex.lock () ; 
  int WasNotified = node._notified ; 
  if (node.TState == ObjectWaiter::TS_WAIT) { 
     dequeue2 (&node); 			// unlink from WaitQueue
  } else 
  if (node.TState == ObjectWaiter::TS_ENTER) { 
     EntryQueue_unlink (&node) ; 	// unlink from EntryQueue
  } else { 
     guarantee (node.TState == ObjectWaiter::TS_RUN, "invariant violation") ;
  }
  if (_succ == &node) _succ = NULL ; 
  _mutex.unlock () ;

  // were we externally suspended while we were waiting?
  if (THREAD->is_Java_thread() &&
     ((JavaThread *)THREAD)->handle_special_suspend_equivalent_condition()) {
      //
      // If we were suspended while we were waiting in park() above,
      // then we don't want to re-enter the monitor while suspended
      // because that would surprise the thread that suspended us.
      // The raw_enter() call below handles the contended monitor
      // case. However, if the thread that notified us has already
      // released the monitor, then we may complete the raw_enter()
      // without contention and without self-suspending.
      //
      ((JavaThread *)THREAD)->java_suspend_self();
  }

  raw_enter(THREAD, false);      // re-enter the monitor
  _recursions = save;     // restore the old recursion count
  _waiters--;             // decrement the number of waiters

  // NOTE: this line must be here, because a notification
  // may be right after the timeout, but before re-enter
  // so the event may still be in signed state
  node._event->reset();
  // check if the notification happened
  if (!WasNotified) {
    // no, it could be timeout or Thread.interrupt() or both
    // check for interrupt event, otherwise it is timeout
    if (interruptible && Thread::is_interrupted(THREAD, true)) {
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
  notify (THREAD) ; 
  return OM_OK;
}

int ObjectMonitor::raw_notifyAll(TRAPS) {
  if (THREAD != _owner) {
    return OM_ILLEGAL_MONITOR_STATE;
  }
  notifyAll (THREAD);  
  return OM_OK;
}

// %%% should move this to objectMonitor.cpp
void ObjectMonitor::check_slow(TRAPS) {
  // called only from check() (NOTE: unused in this implementation)
  assert(THREAD != _owner && !THREAD->is_lock_owned((address) _owner), "must not be owner");
  THROW_MSG(vmSymbols::java_lang_IllegalMonitorStateException(), "current thread not owner");
}

#ifndef PRODUCT
void ObjectMonitor::verify() {
}

void ObjectMonitor::print() {
}
#endif
