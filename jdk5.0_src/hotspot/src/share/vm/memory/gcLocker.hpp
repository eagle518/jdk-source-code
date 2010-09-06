#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)gcLocker.hpp	1.45 03/12/23 16:41:09 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// The direct lock/unlock calls do not force a collection if an unlock
// decrements the count to zero. Avoid calling these if at all possible.

class GC_locker: public AllStatic {
 private:
  static volatile jint _jni_lock_count;  // number of jni active instances
  static volatile jint _lock_count;      // number of other active instances
  static volatile jint _needs_gc;        // heap is filling, we need a GC
                                         // note: bool is typedef'd as jint

  // Accessors
  static bool is_jni_active()  { return _jni_lock_count > 0;              }

  // The following 4 accessor functions all require the JNICritical_lock
  // be held.
  static bool needs_gc()       { assert_lock_strong(JNICritical_lock);
                                 return _needs_gc;                        }
  static void clear_needs_gc() { assert_lock_strong(JNICritical_lock);
                                 _needs_gc = false;                       }

  static void jni_lock()       { assert_lock_strong(JNICritical_lock);
                                 _jni_lock_count++;
                                 assert(Universe::heap() == NULL ||
                                        !Universe::heap()->is_gc_active(), 
                                         "locking failed");               }

  static void jni_unlock()     { assert_lock_strong(JNICritical_lock);
                                 _jni_lock_count--;                       }
    
 public:
  // Accessors
  static bool is_active() { return _lock_count > 0 || _jni_lock_count > 0; }

  // The heap expanded, in lieu of a GC
  //
  // Note that it would be best if this routine took the JNICritical_lock,
  // as it might write _needs_gc. However, this is difficult because
  // of lock ordering issues.
  // As a result, lock_critical() makes an additional check that we've
  // got an active jni critical region before blocking any threads.
  // (The case we're concerned about is if we determine there are JNI
  // active regions here, but are interrupted and they all exit before
  // we manage to set _needs_gc -- thus we'd have set _needs_gc when there
  // are actually no JNI active regions!)
  static void heap_expanded() {
    if (is_jni_active()) {
      // cast away volatile
      Atomic::store(true, &_needs_gc);
    }
  }

  // Non-structured GC locking: currently needed for JNI. Use with care!
  inline static void lock();
  static void unlock() {
    // cast away volatile
    Atomic::dec(&_lock_count);
  }
  
  // The following two methods are used for JNI critical regions.
  // If we find that we failed to perform a GC because the GC_locker
  // was active, arrange for one as soon as possible by allowing
  // all threads in critical regions to complete, but not allowing
  // other critical regions to be entered.
  // Note that critical regions can be nested in a single thread, so
  // we must allow threads already in critical regions to continue.
  //
  // JNI critical regions are the only participants in this scheme
  // because they are, by spec, well bounded while in a critical region.
  // JVMPI is the other GC_locker user, and they are not necessarily
  // well bounded, as they lock GC while interacting with an external
  // agent.
  //
  // If excessive OutOfMemory errors occur during JVMPI execution
  // because of the GC lock being held too long, it's probably more
  // appropriate to simply increase the heap size, rather than attempt
  // to block threads.
  static void lock_critical(JavaThread* thread) {
    MutexLocker mu(JNICritical_lock);
    // Block entering threads if we know at least one thread is in a
    // JNI critical region, we need a GC, and this thread isn't already
    // in a critical region. 
    // We check that at least one thread is in a critical region before
    // blocking because blocked threads are woken up by a thread exiting
    // a JNI critical region.
    while (is_jni_active() && needs_gc() && !thread->in_critical()) {
      JNICritical_lock->wait();
    }
    jni_lock();
    thread->enter_critical();
  }

  inline static void unlock_critical(JavaThread* thread);
};


// A No_GC_Verifier object can be placed in methods where one assumes that
// no garbage collection will occur. The destructor will verify this property
// unless the constructor is called with argument false (not verifygc).
//
// The check will only be done in debug mode and if verifygc true.

class No_GC_Verifier: public StackObj {
 protected:
  bool _verifygc;  
  unsigned int _old_invocations;

 public:
#ifdef ASSERT
  No_GC_Verifier(bool verifygc = true);
  ~No_GC_Verifier();   
#else
  No_GC_Verifier(bool verifygc = true) {}
  ~No_GC_Verifier() {}
#endif
};


// A No_Safepoint_Verifier object will throw an assertion failure if
// the current thread passes a possible safepoint while this object is
// instantiated. A safepoint, will either be: an oop allocation, blocking
// on a Mutex or JavaLock, or executing a VM operation.
//
// If StrictSafepointChecks is turned off, it degrades into a No_GC_Verifier
//
class No_Safepoint_Verifier : public No_GC_Verifier {
 private:  
  bool _activated;
  Thread *_thread;
 public:
#ifdef ASSERT
  No_Safepoint_Verifier(bool activated = true, bool verifygc = true ) : No_GC_Verifier(verifygc) {      
    _thread = Thread::current();
    if (_activated) {
      _thread->_allow_allocation_count++;
      _thread->_allow_safepoint_count++;
    }
  }

  ~No_Safepoint_Verifier() {
    if (_activated) {
      _thread->_allow_allocation_count--;
      _thread->_allow_safepoint_count--;
    }
  }
#else
  No_Safepoint_Verifier(bool activated = true, bool verifygc = true) : No_GC_Verifier(verifygc){}
  ~No_Safepoint_Verifier() {}
#endif
};

// JRT_LEAF currently can be called from either _thread_in_Java or
// _thread_in_native mode. In _thread_in_native, it is ok
// for another thread to trigger GC. The rest of the JRT_LEAF
// rules apply.
class JRT_Leaf_Verifier : public No_Safepoint_Verifier {
  static bool should_verify_GC();
 public:
#ifdef ASSERT
  JRT_Leaf_Verifier();
  ~JRT_Leaf_Verifier();
#else
  JRT_Leaf_Verifier() {}
  ~JRT_Leaf_Verifier() {}
#endif
};

// A No_Alloc_Verifier object can be placed in methods where one assumes that
// no allocation will occur. The destructor will verify this property
// unless the constructor is called with argument false (not activated).
//
// The check will only be done in debug mode and if activated.
// Note: this only makes sense at safepoints (otherwise, other threads may
// allocate concurrently.)

class No_Alloc_Verifier : public StackObj {
 private:
  bool  _activated;

 public:
#ifdef ASSERT
  No_Alloc_Verifier(bool activated = true) { 
    _activated = activated;
    if (_activated) Thread::current()->_allow_allocation_count++;
  }

  ~No_Alloc_Verifier() {
    if (_activated) Thread::current()->_allow_allocation_count--;
  }
#else
  No_Alloc_Verifier(bool activated = true) {}
  ~No_Alloc_Verifier() {}
#endif
};




