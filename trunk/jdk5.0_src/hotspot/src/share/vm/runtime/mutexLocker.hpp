#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)mutexLocker.hpp	1.125 03/12/23 16:43:57 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Mutexes used in the VM.

extern Mutex*   Patching_lock;                   // a lock used to guard code patching of compiled code
extern Mutex*   SystemDictionary_lock;           // a lock on the system dictonary
extern Mutex*   PackageTable_lock;               // a lock on the class loader package table
extern Mutex*   CompiledIC_lock;                 // a lock used to guard compiled IC patching and access
extern Mutex*   InlineCacheBuffer_lock;          // a lock used to guard the InlineCacheBuffer
extern Mutex*   VMStatistic_lock;                // a lock used to guard statistics count increment
extern Mutex*   JNIGlobalHandle_lock;            // a lock on creating JNI global handles
extern Mutex*   JNIHandleBlockFreeList_lock;     // a lock on the JNI handle block free list
extern Mutex*   JNIIdentifier_lock;              // a lock on creating JNI method/static field identifiers
extern Monitor* JNICritical_lock;                // a lock used while entering and exiting JNI critical regions, allows GC to sometimes get in
extern Mutex*   JvmdiFrame_lock;                 // a lock used to protect jframeID counter.
extern Mutex*   JvmdiCachedFrame_lock;           // a lock on modification of jvmdi cached frames.
extern Mutex*   JvmtiThreadState_lock;           // a lock on modification of JVMTI thread data
extern Monitor*	JvmtiPendingEvent_lock;		 // a lock on the JVMTI pending events list
extern Mutex*   Heap_lock;                       // a lock on the heap
extern Mutex*   ExpandHeap_lock;                 // a lock on expanding the heap
extern Mutex*   RecompilationMonitor_lock;       // a lock to guard the RecompilationMonitor
extern Mutex*   SignatureHandlerLibrary_lock;    // a lock on the SignatureHandlerLibrary
extern Mutex*   VtableStubs_lock;                // a lock on the VtableStubs
extern Mutex*   SymbolTable_lock;                // a lock on the symbol table
extern Mutex*   StringTable_lock;                // a lock on the interned string table
extern Mutex*   AdapterCache_lock;               // a lock on the C2I/I2C Adapter cache's
extern Mutex*   CodeCache_lock;                  // a lock on the CodeCache, rank is special, use MutexLockerEx
extern Mutex*   MethodData_lock;                 // a lock on installation of method data
extern Mutex*   RetData_lock;                    // a lock on installation of RetData inside method data
extern Mutex*	DerivedPointerTableGC_lock;	 // a lock to protect the derived pointer table
extern Monitor* VMOperationQueue_lock;	         // a lock on queue of vm_operations waiting to execute
extern Monitor* VMOperationRequest_lock;         // a lock on Threads waiting for a vm_operation to terminate
extern Monitor* Safepoint_lock;                  // a lock used by the safepoint abstraction
extern Monitor* Threads_lock;                    // a lock on the Threads table of active Java threads 
                                                 // (also used by Safepoints too to block threads creation/destruction)
extern Monitor* CMS_lock;                        // used for coordination between 
                                                 // fore- & background GC threads.
extern Monitor* SLT_lock;                        // used in CMS GC for acquiring PLL
extern Monitor* iCMS_lock;                       // CMS incremental mode start/stop notification
extern Mutex*   ParGCRareEvent_lock;             // Synchronizes various (rare) parallel GC ops.
extern Mutex*   Compile_lock;                    // a lock held when Compilation is updating code (used to block CodeCache traversal, CHA updates, etc)
extern Monitor* AdapterCompileQueue_lock;        // a lock held when adapter compilations are enqueued, dequeued
extern Monitor* MethodCompileQueue_lock;         // a lock held when method compilations are enqueued, dequeued
extern Mutex*   CompileTaskAlloc_lock;           // a lock held when CompileTasks are allocated
extern Mutex*   CompileStatistics_lock;          // a lock held when updating compilation statistics
extern Mutex*   MultiArray_lock;                 // a lock used to guard allocation of multi-dim arrays
extern Monitor* Terminator_lock;                 // a lock used to guard termination of the vm
extern Monitor* BeforeExit_lock;                 // a lock used to guard cleanups and shutdown hooks
extern Monitor* Notify_lock;                     // a lock used to synchronize the start-up of the vm
extern Monitor* Interrupt_lock;                  // a lock used for condition variable mediated interrupt processing
extern Monitor* ProfileVM_lock;                  // a lock used for profiling the VMThread
extern Mutex*   ProfilePrint_lock;               // a lock used to serialize the printing of profiles
extern Mutex*   ExceptionCache_lock;             // a lock used to synchronize exception cache updates
extern Monitor* ObjAllocPost_lock;               // a lock used to synchronize VMThread JVM/PI OBJ_ALLOC event posting
extern Mutex*   OsrList_lock;                    // a lock used to serialize access to OSR queues

#ifndef PRODUCT
extern Mutex*   BytecodeTrace_lock;              // a lock to make bytecode tracing MT safe
extern Mutex*   FullGCALot_lock;                 // a lock to make FullGCALot MT safe
#endif
extern Mutex*   Debug1_lock;                     // A bunch of pre-allocated locks that can be used for tracing
extern Mutex*   Debug2_lock;                     // down synchronization related bugs!
extern Mutex*   Debug3_lock;

extern Mutex*   MonitorCache_lock;               // a lock on the global monitor cache
extern Mutex*   PerfDataMemAlloc_lock;           // a lock on the allocator for PerfData memory for performance data
extern Mutex*   PerfDataManager_lock;            // a long on access to PerfDataManager resources
extern Mutex*   OopMapCacheAlloc_lock;           // protects allocation of oop_map caches

extern Mutex*   Management_lock;                 // a lock used to serialize JVM management
extern Monitor* LowMemory_lock;                  // a lock used for low memory detection

// A MutexLocker provides mutual exclusion with respect to a given mutex
// for the scope which contains the locker.  The lock is an OS lock, not
// an object lock, and the two do not interoperate.  Do not use Mutex-based
// locks to lock on Java objects, because they will not be respected if a
// that object is locked using the Java locking mechanism.
//
//                NOTE WELL!!
//
// See orderAccess.hpp.  We assume throughout the VM that MutexLocker's
// and friends constructors do a fence, a lock and an acquire *in that
// order*.  And that their destructors do a release and unlock, in *that*
// order.  If their implementations change such that these assumptions
// are violated, a whole lot of code will break.

// Print all mutexes/monitors that are currently owned by a thread; called
// by fatal error handler.
void print_owned_locks_on_error(outputStream* st);

char *lock_name(Mutex *mutex);

class MutexLocker: StackObj {
 private:
  Mutex* _mutex;
 public:
  MutexLocker(Mutex* mutex) {
    assert(mutex->rank() != Mutex::special,
      "Special ranked mutex should only use MutexLockerEx");
    _mutex = mutex;
    _mutex->lock();
  }

  // Overloaded constructor passing current thread
  MutexLocker(Mutex* mutex, Thread *thread) {
    assert(mutex->rank() != Mutex::special,
      "Special ranked mutex should only use MutexLockerEx");
    _mutex = mutex;
    _mutex->lock(thread);
  }

  ~MutexLocker() {    
    _mutex->unlock();
  }

};

// for debugging: check that we're already owning this lock (or are at a safepoint)
#ifdef ASSERT
void assert_locked_or_safepoint(const Mutex* lock);
void assert_lock_strong(const Mutex* lock);
#else
#define assert_locked_or_safepoint(lock)
#define assert_lock_strong(lock)
#endif

// A MutexLockerEx behaves like a MutexLocker when its constructor is
// called with a Mutex.  Unlike a MutexLocker, its constructor can also be
// called with NULL, in which case the MutexLockerEx is a no-op.  There
// is also a corresponding MutexUnlockerEx.  We want to keep the
// basic MutexLocker as fast as possible.  MutexLockerEx can also lock
// without safepoint check.

class MutexLockerEx: StackObj {
 private:
  Mutex* _mutex;
 public:
  MutexLockerEx(Mutex* mutex, bool no_safepoint_check = !Mutex::_no_safepoint_check_flag) {
    _mutex = mutex;
    if (_mutex != NULL) {
      assert(mutex->rank() > Mutex::special || no_safepoint_check,
	"Mutexes with rank special or lower should not do safepoint checks");
      if (no_safepoint_check)
	_mutex->lock_without_safepoint_check();
      else
	_mutex->lock();
    }
  }

  ~MutexLockerEx() {
    if (_mutex != NULL) {
      _mutex->unlock();
    }
  }
};

// A GCMutexLocker is usually initialized with a mutex that is
// automatically acquired in order to do GC.  The function that
// synchronizes using a GCMutexLocker may be called both during and between
// GC's.  Thus, it must acquire the mutex if GC is not in progress, but not
// if GC is in progress (since the mutex is already held on its behalf.)

class GCMutexLocker: public StackObj {
private:
  Mutex* _mutex;
  bool _locked;
public:
  GCMutexLocker(Mutex* mutex);
  ~GCMutexLocker() { if (_locked) _mutex->unlock(); }
};



// A MutexUnlocker temporarily exits a previously
// entered mutex for the scope which contains the unlocker.

class MutexUnlocker: StackObj {
 private:
  Mutex* _mutex;

 public:
  MutexUnlocker(Mutex* mutex) {
    _mutex = mutex;
    _mutex->unlock();
  }

  ~MutexUnlocker() {
    _mutex->lock();
  }
};

// A MutexUnlockerEx temporarily exits a previously
// entered mutex for the scope which contains the unlocker.

class MutexUnlockerEx: StackObj {
 private:
  Mutex* _mutex;
  bool _no_safepoint_check;

 public:
  MutexUnlockerEx(Mutex* mutex, bool no_safepoint_check = !Mutex::_no_safepoint_check_flag) {
    _mutex = mutex;
    _no_safepoint_check = no_safepoint_check;
    _mutex->unlock();
  }

  ~MutexUnlockerEx() {
    if (_no_safepoint_check == Mutex::_no_safepoint_check_flag) {
      _mutex->lock_without_safepoint_check();
    } else {
      _mutex->lock();
    }
  }
};

#ifndef PRODUCT
//
// A special MutexLocker that allows:
//   - reentrant locking
//   - locking out of order
//
// Only too be used for verify code, where we can relaxe out dead-lock
// dection code a bit (unsafe, but probably ok). This code is NEVER to
// be included in a product version.
//
class VerifyMutexLocker: StackObj {
 private:
  Mutex* _mutex;
  bool   _reentrant;
 public:
  VerifyMutexLocker(Mutex* mutex) {
    _mutex     = mutex;
    _reentrant = mutex->owned_by_self();
    if (!_reentrant) {
      // We temp. diable strict safepoint checking, while we require the lock
      FlagSetting fs(StrictSafepointChecks, false);    
      _mutex->lock();
    }
  }
  
  ~VerifyMutexLocker() {
    if (!_reentrant) {     
      _mutex->unlock();
    }
  }
};

#endif

