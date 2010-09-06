#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)thread.hpp	1.418 04/06/14 11:08:29 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class ThreadSafepointState;
class ThreadProfiler;

class JvmtiThreadState;
class JvmtiGetLoadedClassesClosure;
class ThreadStatistics;

class ciEnv;
class CompileLog;
class CompileTask;
class CompileQueue;
class CompilerCounters;
class RawMonitor;

class Suspend_Callback;
class Resume_Callback;

class DeoptResourceMark;
#ifndef CORE
class jvmtiDeferredLocalVariableSet;
#endif /* CORE */

class GCTaskQueue;
class ThreadClosure;

// Class hierarchy
// - Thread
//   - VMThread
//   - JavaThread
//   - WatcherThread

class Thread: public ThreadShadow {
  friend class VMStructs;
 private:
  // Exception handling
  // (Note: _pending_exception is in ThreadShadow)
  //oop       _pending_exception;                // pending exception for current thread
  const char* _exception_file;                   // file information for exception (debugging only)
  int         _exception_line;                   // line information for exception (debugging only)

  RawMonitor* _rawmonitor_list;                  // list of locked raw monitors (JVMPI support)

  // ***************************************************************
  // Suspend and resume support
  // ***************************************************************
  //
  // External suspend/resume requests come from JVM_SuspendThread,
  // JVM_ResumeThread, JVMTI SuspendThread, JVMTI ResumeThread,
  // JVM/PI SuspendThread, and finally JVM/PI ResumeThread. External
  // suspend requests cause _external_suspend to be set and external
  // resume requests cause _external_suspend to be cleared.
  // External suspend requests do not nest on top of other external
  // suspend requests. The higher level APIs reject suspend requests
  // for already suspended threads.
  //
  // The external_suspend 
  // flag is checked by has_special_runtime_exit_condition() and java thread 
  // will self-suspend when handle_special_runtime_exit_condition() is
  // called. Most uses of the _thread_blocked state in JavaThreads are
  // considered the same as being externally suspended; if the blocking
  // condition lifts, the JavaThread will self-suspend. Other places
  // where VM checks for external_suspend include:
  //   + mutex granting (do not enter monitors when thread is suspended)
  //   + state transitions from _thread_in_native
  //
  // In general, java_suspend() does not wait for an external suspend
  // request to complete. When it returns, the only guarantee is that
  // the _external_suspend field is true. 
  //
  // wait_for_ext_suspend_completion() is used to wait for an external
  // suspend request to complete. External suspend requests are usually
  // followed by some other interface call that requires the thread to
  // be quiescent, e.g., GetCallTrace(). By moving the "wait time" into
  // the interface that requires quiescence, we give the JavaThread a
  // chance to self-suspend before we need it to be quiescent. This
  // improves overall suspend/query performance.
  //
  // VM uses vm_suspend() and vm_resume() internally for safepointing
  // and the flat profiler. Unlike java_suspend(), vm_suspend() suspends
  // target thread non-cooperatively. vm_suspend() and vm_resume() can
  // handle nesting requests. Target thread can resume execution only
  // when the nesting level reaches zero.
  //
  // Note in vm_suspend() target thread might be suspended while holding 
  // important resources. To avoid deadlocking the VM, target thread can 
  // only be suspended for a brief time, and vm_suspend() should only be 
  // used on code paths where we have full control of. Avoid 
  // allocating heap memory, making I/O calls or grabbing mutex between 
  // vm_suspend() and vm_resume() calls.
  // 
  // vm_resume can only resume a thread suspended by vm_suspend. Similarly,
  // java_resume only resumes threads suspended by java_suspend.
  //
  // _suspend_flags controls the behavior of vm_ and java_ suspend/resume.
  // It must be set under the protection of SR_lock. Read from the flag is
  // OK without SR_lock as long as the value is only used as a hint.
  // (e.g., check _external_suspend first without lock and then recheck
  // inside SR_lock and finish the suspension)
  //
  // -------------------------------------------------------------------
  // Notes: 
  // 1. The suspend/resume logic no longer uses ThreadState in OSThread
  // but we still update its value to keep other part of the system (mainly 
  // JVMPI and JVMTI) happy. ThreadState is legacy code (see notes in 
  // osThread.hpp).
  // 
  // 2. It would be more natural if set_external_suspend() is private and
  // part of java_suspend(), but that probably would affect the suspend/query
  // performance. Need more investigation on this.
  //

  // suspend/resume lock; also protects OSThread::ThreadState
  Monitor*    _SR_lock;
  int32_t     _vm_suspend_depth;

 protected:
  enum SuspendFlags {
    // flags for vm_suspend(), vm_suspend() is always non-cooperative.
    _vm_suspended           = 0x00008000, // suspended by vm_suspend()
    _is_baby_thread         = 0x00004000, // baby thread is unstarted thread
                                          // this is similar to _thread_new
                                          // in JavaThread
    _suspended_as_baby      = 0x00002000, // target is suspended before it
                                          // starts to run (e.g. by a profiler)

    // additional flags for java_suspend(), java_suspend() is used to suspend
    // a JavaThread cooperatively, unless it's in state _thread_in_native.
    _external_suspend       = 0x80000000, // thread is asked to self suspend
    _ext_suspended          = 0x40000000, // thread has self-suspended

    _has_async_exception    = 0x00000001  // there is a pending async exception
  };

  // various suspension related flags.
  // overloaded for async exception checking in check_safepoint_and_suspend_for_native_trans.
  volatile uint32_t _suspend_flags;

#ifdef ASSERT
 private:
  int _num_nested_signal;

 public:
  void enter_signal_handler() { _num_nested_signal++; }
  void leave_signal_handler() { _num_nested_signal--; }
  bool is_inside_signal_handler() const  { return _num_nested_signal > 0; }
#endif

 private:
  // Debug tracing
  static void trace(const char* msg, const Thread* const thread) PRODUCT_RETURN;

  // Active_handles points to a block of handles
  JNIHandleBlock* _active_handles;

  // One-element thread local free list
  JNIHandleBlock* _free_handle_block;

  // Point to the last handle mark
  HandleMark* _last_handle_mark;

  // The parity of the last strong_roots iteration in which this thread was 
  // claimed as a task.
  jint _oops_do_parity;

  public:
   void set_last_handle_mark(HandleMark* mark)   { _last_handle_mark = mark; }
    HandleMark* last_handle_mark() const          { return _last_handle_mark; }
  private:

  // debug support for checking if code does allow safepoints or not
  // GC points in the VM can happen because of allocation, invoking a VM operation, or blocking on 
  // mutex, or blocking on an object synchronizer (Java locking).
  // If !allow_safepoint(), then an assertion failure will happen in any of the above cases
  // If !allow_allocation(), then an assertion failure will happen during allocation
  // (Hence, !allow_safepoint() => !allow_allocation()).
  //
  // The two classes No_Safepoint_Verifier and No_Allocation_Verifier are used to set these counters.
  //
  NOT_PRODUCT(int _allow_safepoint_count;)       // If 0, thread allow a safepoint to happen
  debug_only (int _allow_allocation_count;)      // If 0, the thread is allowed to allocate oops.  

  friend class No_Alloc_Verifier;
  friend class No_Safepoint_Verifier;
  friend class ThreadLocalStorage;

  // In order for all threads to be able to use fast locking, we need to know the highest stack
  // address of where a lock is on the stack (stacks normally grow towards lower addresses). This
  // variable is initially set to NULL, indicating no locks are used by the thread. During the thread's
  // execution, it will be set whenever locking can happen, i.e., when we call out to Java code or use
  // an ObjectLocker. The value is never decreased, hence, it will over the lifetime of a thread
  // approximate the real stackbase. 
  address _highest_lock;                         // Highest stack address where a JavaLock exist
  ThreadPriority _unboosted_priority;            // Old priority if boosted, else NoPriority
  
  ThreadLocalAllocBuffer _tlab;                  // Thread-local eden

  int   _vm_operation_started_count;             // VM_Operation support
  int   _vm_operation_completed_count;           // VM_Operation support

  ObjectMonitor* _current_pending_monitor;       // ObjectMonitor this thread
                                                 // is waiting to lock
  bool _current_pending_monitor_is_from_java;    // locking is from Java code

  // ObjectMonitor on which this thread called Object.wait()
  ObjectMonitor* _current_waiting_monitor;
  
 public: 
  enum {
    is_definitely_current_thread = true
  };    

  // Constructor
  Thread();
  virtual ~Thread();

  // initializtion
  void initialize_thread_local_storage();

  // thread entry point
  virtual void run();

  // Testers
  virtual bool is_VM_thread()       const            { return false; }
  virtual bool is_Java_thread()     const            { return false; }
  // Remove this ifdef when C1 is ported to the compiler interface.
#ifndef CORE
  virtual bool is_Compiler_thread() const            { return false; }
#endif
  virtual bool is_hidden_from_external_view() const  { return false; }  
  virtual bool is_jvmti_agent_thread() const 	     { return false; }
  // True iff the thread can perform GC operations at a safepoint.
  // Generally will be true only of VM thread and parallel GC WorkGang
  // threads.
  virtual bool is_GC_task_thread() const             { return false; }
  virtual bool is_jvmpi_daemon_thread() const        { return false; }
  virtual bool is_Watcher_thread() const             { return false; }
  virtual bool is_ConcurrentMarkSweep_thread() const { return false; }

  virtual char* name() const { return (char*)"Unknown thread"; }

  // Returns the current thread
  static inline Thread* current();  

  // Common thread operations
  static void set_priority(Thread* thread, ThreadPriority priority);
  static ThreadPriority get_priority(const Thread* const thread);
  static void boost_priority(Thread* thread, ThreadPriority priority);
  static void unboost_priority(Thread* thread);
  static void start(Thread* thread);
  static void interrupt(Thread* thr);
  static bool is_interrupted(Thread* thr, bool clear_interrupted);  

  Monitor* SR_lock() const                       { return _SR_lock; }
  int do_vm_suspend();
  int do_vm_resume();
  int vm_suspend();
  int vm_resume();

  int vm_suspend_depth() const                   { return _vm_suspend_depth; }
  void set_vm_suspend_depth(int depth)           { _vm_suspend_depth = depth;}

  bool is_vm_suspended() const    { return _suspend_flags & _vm_suspended;   }
  bool is_baby_thread() const     { return _suspend_flags & _is_baby_thread; }
  bool is_suspended_as_baby()const{ return _suspend_flags & _suspended_as_baby; }

  bool has_async_exception() const { return _suspend_flags & _has_async_exception; }

  void set_suspend_flag  (SuspendFlags f)        { _suspend_flags |=  f;     }
  void clear_suspend_flag(SuspendFlags f)        { _suspend_flags &= ~f;     }

  void set_vm_suspended()          { set_suspend_flag  (_vm_suspended);      }
  void set_is_baby_thread()        { set_suspend_flag  (_is_baby_thread);    }
  void set_suspended_as_baby()     { set_suspend_flag  (_suspended_as_baby); }
  void clear_vm_suspended()        { clear_suspend_flag(_vm_suspended);      }
  void clear_is_baby_thread()      { clear_suspend_flag(_is_baby_thread);    }
  void clear_suspended_as_baby()   { clear_suspend_flag(_suspended_as_baby); }

  void set_has_async_exception_with_lock() {
    MutexLockerEx ml(SR_lock(), Mutex::_no_safepoint_check_flag);
    set_suspend_flag(_has_async_exception);
  }
  void clear_has_async_exception_with_lock() {
    MutexLockerEx ml(SR_lock(), Mutex::_no_safepoint_check_flag);
    clear_suspend_flag(_has_async_exception);
  }
public:
  // Installs a pending exception to be inserted later
  static void send_async_exception(oop thread_oop, oop java_throwable);

  // Boosted priority
  ThreadPriority unboosted_priority() const      { return _unboosted_priority; }
  void set_unboosted_priority(ThreadPriority priority) { _unboosted_priority = priority; }

  // Resource area
  ResourceArea* resource_area() const            { return _resource_area; }
  void set_resource_area(ResourceArea* area)     { _resource_area = area; }

  OSThread* osthread() const                     { return _osthread;   }
  void set_osthread(OSThread* thread)            { _osthread = thread; }

  // JNI handle support
  JNIHandleBlock* active_handles() const         { return _active_handles; }
  void set_active_handles(JNIHandleBlock* block) { _active_handles = block; }
  JNIHandleBlock* free_handle_block() const      { return _free_handle_block; }
  void set_free_handle_block(JNIHandleBlock* block) { _free_handle_block = block; }

  // Internal handle support
  HandleArea* handle_area() const                { return _handle_area; }
  void set_handle_area(HandleArea* area)         { _handle_area = area; }

  // Exceptions
  void set_pending_exception(oop exception, const char* file, int line);   // use THROW whenever possible!
  void clear_pending_exception();                                          // use CLEAR_PENDING_EXCEPTION whenever possible!
  bool has_pending_exception() const             { return pending_exception() != NULL; }    
  oop  pending_exception() const                 { return _pending_exception; } 
  const char* exception_file() const             { return _exception_file; }
  int  exception_line() const                    { return _exception_line; }

  // Thread-Local Allocation Buffer (TLAB) support
  ThreadLocalAllocBuffer& tlab()                 { return _tlab; }

  // VM operation support
  int vm_operation_ticket()                      { return ++_vm_operation_started_count; }
  int vm_operation_completed_count()             { return _vm_operation_completed_count; }
  void increment_vm_operation_completed_count()  { _vm_operation_completed_count++; }

  // JVMPI support
  RawMonitor* rawmonitor_list() const            { return _rawmonitor_list; }
  void        set_rawmonitor_list(RawMonitor* r) { _rawmonitor_list = r;    }

  // For tracking the heavyweight monitor the thread is pending on.
  ObjectMonitor* current_pending_monitor() {
    return _current_pending_monitor;
  }
  void set_current_pending_monitor(ObjectMonitor* monitor) {
    _current_pending_monitor = monitor;
  }
  void set_current_pending_monitor_is_from_java(bool from_java) {
    _current_pending_monitor_is_from_java = from_java;
  }
  bool current_pending_monitor_is_from_java() {
    return _current_pending_monitor_is_from_java;
  }

  // For tracking the ObjectMonitor on which this thread called Object.wait()
  ObjectMonitor* current_waiting_monitor() {
    return _current_waiting_monitor;
  }
  void set_current_waiting_monitor(ObjectMonitor* monitor) {
    _current_waiting_monitor = monitor;
  }

  // GC support
  // Apply "f->do_oop" to all root oops in "this".
  void oops_do(OopClosure* f);  

  // Handles the parallel case for the method below.
private:
  bool claim_oops_do_par_case(int collection_parity);
public:
  // Requires that "collection_parity" is that of the current strong roots
  // iteration.  If "is_par" is false, sets the parity of "this" to
  // "collection_parity", and returns "true".  If "is_par" is true,
  // uses an atomic instruction to set the current threads parity to
  // "collection_parity", if it is not already.  Returns "true" iff the
  // calling thread does the update, this indicates that the calling thread 
  // has claimed the thread's stack as a root groop in the current
  // collection. 
  bool claim_oops_do(bool is_par, int collection_parity) {
    if (!is_par) {
      _oops_do_parity = collection_parity;
      return true;
    } else {
      return claim_oops_do_par_case(collection_parity);
    }
  }

  // Sweeper support
  void nmethods_do();  
  
  // Fast-locking support
  address highest_lock() const                   { return _highest_lock; }
  void update_highest_lock(address base)         { if (base > _highest_lock) _highest_lock = base; }

  // Tells if adr belong to this thread. This is used
  // for checking if a lock is owned by the running thread.
  // Warning: the method can only be used on the running thread
  // Fast lock support uses these methods
  virtual bool is_in_stack(address adr) const;
  virtual bool is_lock_owned(address adr) const;  

  // Sets this thread as starting thread. Returns failure if thread
  // creation fails due to lack of memory, too many threads etc.
  bool set_as_starting_thread();

 protected:
  // OS data associated with the thread
  OSThread* _osthread;  // Platform-specific thread information 

  // Thread local resource area for temporary allocation within the VM
  ResourceArea* _resource_area;

  // Thread local handle area for allocation of handles within the VM
  HandleArea* _handle_area;

  // Support for stack overflow handling, get_thread, etc.  
  address          _stack_base;
  size_t           _stack_size;
  uintptr_t        _self_raw_id;      // used by get_thread (mutable)

 public:
  // Stack overflow support
  address stack_base() const           { assert(_stack_base != NULL,"Sanity check"); return _stack_base; }
  
  void    set_stack_base(address base) { _stack_base = base; }
  size_t  stack_size() const           { return _stack_size; }
  void    set_stack_size(size_t size)  { _stack_size = size; }
  void    record_stack_base_and_size();

  // Printing
  void print() const;
  virtual void print_on_error(outputStream* st, char* buf, int buflen) const;

  // Debug-only code

#ifdef ASSERT
 private:
  // Deadlock detection support for Mutex locks. List of locks own by thread.
  Mutex *_owned_locks;
  // Mutex::set_owner_implementation is the only place where _owned_locks is modified,
  // thus the friendship
  friend class Mutex;

 public:  
  void print_owned_locks() const;
  Mutex* owned_locks() const			 { return _owned_locks;          }
  bool owns_locks() const                        { return owned_locks() != NULL; }  
  bool owns_locks_but_compiled_lock() const;

  // Deadlock detection
  bool allow_allocation()                        { return _allow_allocation_count == 0; }
#endif

  void check_for_valid_safepoint_state(bool potential_vm_operation) PRODUCT_RETURN;

  // Code generation
  static ByteSize exception_file_offset()        { return byte_offset_of(Thread, _exception_file   ); }
  static ByteSize exception_line_offset()        { return byte_offset_of(Thread, _exception_line   ); }
  static ByteSize active_handles_offset()        { return byte_offset_of(Thread, _active_handles   ); }

  static ByteSize stack_base_offset()            { return byte_offset_of(Thread, _stack_base ); }
  static ByteSize stack_size_offset()            { return byte_offset_of(Thread, _stack_size ); }

#define TLAB_FIELD_OFFSET(name) \
  static ByteSize tlab_##name##_offset()            { return byte_offset_of(Thread, _tlab) + ThreadLocalAllocBuffer::##name##_offset(); }

  TLAB_FIELD_OFFSET(start)
  TLAB_FIELD_OFFSET(end)
  TLAB_FIELD_OFFSET(top)
  TLAB_FIELD_OFFSET(size)
  TLAB_FIELD_OFFSET(refill_waste_limit)
  TLAB_FIELD_OFFSET(number_of_refills)
  TLAB_FIELD_OFFSET(fast_refill_waste)
  TLAB_FIELD_OFFSET(slow_allocations)

#undef TLAB_FIELD_OFFSET

};

// Inline implementation of Thread::current()
// Thread::current is "hot" it's called > 128K times in the 1st 500 msecs of
// startup.
// ThreadLocalStorage::thread is warm -- it's called > 16K times in the same
// period.   This is inlined in thread_<os_family>.inline.hpp.

inline Thread* Thread::current() {
#ifdef ASSERT
// This function is very high traffic. Define PARANOID to enable expensive 
// asserts.
#ifdef PARANOID
  // Signal handler should call ThreadLocalStorage::get_thread_slow()
  Thread* t = ThreadLocalStorage::get_thread_slow();
  assert(t != NULL && !t->is_inside_signal_handler(), 
         "Don't use Thread::current() inside signal handler");
#endif
#endif
  Thread* thread = ThreadLocalStorage::thread();
  assert(thread != NULL, "just checking");
  return thread;
}

// Name support for threads.  non-JavaThread subclasses with multiple
// uniquely named instances should derive from this.
class NamedThread: public Thread {
  friend class VMStructs;
  enum {
    max_name_len = 64
  };
 private:
  char* _name;
 public:
  NamedThread();
  ~NamedThread();
  // He or she who calls set_name() must free the storage with free_name()
  void set_name(const char* format, ...);
  void free_name();
  virtual char* name() const { return _name == NULL ? (char*)"Unknown Thread" : _name; }
};

// A single WatcherThread is used for simulating timer interrupts.
class WatcherThread: public Thread {
  friend class VMStructs;
 public:
  virtual void run();

 private:
  static WatcherThread* _watcher_thread;

  static bool _should_terminate;  
 public:
  enum SomeConstants {
    delay_interval = 10                          // interrupt delay in milliseconds
  };

  // Constructor
  WatcherThread();

  // Tester
  bool is_Watcher_thread() const                 { return true; }

  // Printing
  char* name() const { return (char*)"VM Periodic Task Thread"; }
  void print() const;

  // Returns the single instance of WatcherThread
  static WatcherThread* watcher_thread()         { return _watcher_thread; }

  // Create and start the single instance of WatcherThread, or stop it on shutdown
  static void start();
  static void stop();
};


typedef void (*ThreadFunction)(JavaThread*, TRAPS);

class JavaThread: public Thread {  
  friend class VMStructs;
 private:  
  JavaThread*    _next;                          // The next thread in the Threads list
  oop            _threadObj;                     // The Java level thread object

#ifdef ASSERT
 private:
  int _java_call_counter;

 public:
  int  java_call_counter()                       { return _java_call_counter; }
  void inc_java_call_counter()                   { _java_call_counter++; }
  void dec_java_call_counter() {
    assert(_java_call_counter > 0, "Invalid nesting of JavaCallWrapper");
    _java_call_counter--;
  }
 private:  // restore original namespace restriction
#endif  // ifdef ASSERT

#ifndef PRODUCT
 public:
  enum {
    jump_ring_buffer_size = 16
  };
 private:  // restore original namespace restriction
#endif

  JavaFrameAnchor _anchor;                       // Encapsulation of current java frame and it state
 
  ThreadFunction _entry_point;

  JNIEnv        _jni_environment;
  
  DeoptResourceMark*  _deopt_mark;               // Holds special ResourceMark for deoptimization
  vframeArray*  _vframe_array_head;              // Holds the heap of the active vframeArrays  
  vframeArray*  _vframe_array_last;              // Holds last vFrameArray we popped
#ifndef CORE
  // Because deoptimization is lazy we must save jvmti requests to set locals
  // in compiled frames until we deoptimize and we have an interpreter frame.
  // This holds the pointer to array (yeah like there might be more than one) of
  // description of compiled vframes that have locals that need to be updated.
  GrowableArray<jvmtiDeferredLocalVariableSet*>* _deferred_locals_updates;
#endif /* CORE */
  
  // Oop results of VM runtime calls  
  oop           _vm_result;                      // Used to pass back an oop result into Java code, GC-preserved
  oop           _vm_result_2;                    // Used to pass back an oop result into Java code, GC-preserved

  MonitorChunk* _monitor_chunks;                 // Contains the off stack monitors 
                                                 // allocated during deoptimization
                                                 // and by JNI_MonitorEnter/Exit

  // Async. requests support
  enum AsyncRequests {
    _no_async_condition = 0,
    _async_exception,
    _async_unsafe_access_error
  };
  AsyncRequests _special_runtime_exit_condition; // Enum indicating pending async. request
  oop           _pending_async_exception;

  // Safepoint support
  volatile JavaThreadState _thread_state;
  ThreadSafepointState *_safepoint_state;        // Holds information about a thread during a safepoint
  address               _saved_exception_pc;     // Saved pc of instruction where last implicit exception happened

  // JavaThread termination support
  enum TerminatedTypes {
    _not_terminated = 0xDEAD - 2,
    _thread_exiting,                             // JavaThread::exit() has been called for this thread
    _thread_terminated,                          // JavaThread is removed from thread list
    _vm_exited                                   // JavaThread is still executing native code, but VM is terminated
                                                 // only VM_Exit can set _vm_exited
  };

  // In general a JavaThread's _terminated field transitions as follows:
  //
  //   _not_terminated => _thread_exiting => _thread_terminated
  //
  // _vm_exited is a special value to cover the case of a JavaThread
  // executing native code after the VM itself is terminated.
  TerminatedTypes       _terminated;
  // suspend/resume support
  bool                  _suspend_equivalent;     // Suspend equivalent condition
  jint                  _in_deopt_handler;       // count of deoptimization
                                                 // handlers thread is in
  bool                  _has_called_back;        // Thread has called back the safepoint code (for debugging)
  bool                  _doing_unsafe_access;    // Thread may fault due to unsafe access
  bool                  _do_not_unlock_if_synchronized; // Do not unlock the receiver of a synchronized method (since it was
						 // never locked) when throwing an exception. Used by interpreter only.

 public:
  // State of the stack guard pages for this thread.
  enum StackGuardState {
    stack_guard_unused,		// not needed
    stack_guard_yellow_disabled,// disabled (temporarily) after stack overflow
    stack_guard_enabled		// enabled
  };

 private:

  StackGuardState        _stack_guard_state;

  // Compiler exception handling (NOTE: The _exception_oop is *NOT* the same as _pending_exception. It is
  // used to temp. parsing values into and out of the runtime system during exception handling for compiled
  // code)
  volatile oop     _exception_oop;               // Exception thrown in compiled code
  volatile address _exception_pc;                // PC where exception happened
  volatile address _exception_handler_pc;        // PC for handler of exception
  volatile int     _exception_stack_size;        // Size of frame where exception happened

  // support for compilation
  bool    _is_compiling;                         // is true if a compilation is active inthis thread (one compilation per thread possible)

  // support for JNI critical regions
  jint    _jni_active_critical;                  // count of entries into JNI critical region

  void* _jvmpi_data;     // support for JVM/PI {Get,Set}ThreadLocalStorage()
  uintptr_t  _last_sum;  // support for JVM/PI ThreadHasRun()
                         // support for deferred OBJECT_ALLOC events
  GrowableArray<DeferredObjAllocEvent*>* _deferred_obj_alloc_events;

  // For deadlock detection.
  int _depth_first_number;

  // JVMTI PopFrame support
  // This is set to popframe_pending to signal that top Java frame should be popped immediately
  int _popframe_condition;

#ifndef PRODUCT
  int _jmp_ring_index;
  struct {
      // We use intptr_t instead of address so debugger doesn't try and display strings
      intptr_t _target;
      intptr_t _instruction;
      const char*  _file;
      int _line;
  }   _jmp_ring[ jump_ring_buffer_size ];
#endif /* PRODUCT */

  friend class VMThread;
  friend class ThreadWaitTransition;
  friend class VM_Exit;
  friend class VM_ThreadSuspend;

  void initialize();                             // Initialized the instance variables

 public:
  // Constructor
  JavaThread(); // for main thread
  JavaThread(ThreadFunction entry_point, size_t stack_size = 0); 
  ~JavaThread();
    
  //JNI functiontable getter/setter for JVMTI jni function table interception API.
  void set_jni_functions(struct JNINativeInterface_* functionTable) {
    _jni_environment.functions = functionTable;
  }
  struct JNINativeInterface_* get_jni_functions() {
    return (struct JNINativeInterface_ *)_jni_environment.functions;
  }

  // Executes Shutdown.shutdown()
  void invoke_shutdown_hooks();

  // Cleanup on thread exit
  void exit(bool destroy_vm);

  void cleanup_failed_attach_current_thread();

  // Testers
  virtual bool is_Java_thread() const            { return true;  }

  // compilation
  void set_is_compiling(bool f)                  { _is_compiling = f; }
  bool is_compiling() const                      { return _is_compiling; }

  // Thread chain operations
  JavaThread* next() const                       { return _next; }
  void set_next(JavaThread* p)                   { _next = p; }
  
  // Thread oop. threadObj() can be NULL for initial JavaThread
  // (or for threads attached via JNI)
  oop threadObj() const                          { return _threadObj; }
  void set_threadObj(oop p)                      { _threadObj = p; }

  ThreadPriority java_priority() const;          // Read from threadObj()

  // Prepare thread and add to priority queue.  If a priority is
  // not specified, use the priority of the thread object. Threads_lock
  // must be held while this function is called.
  void prepare(jobject jni_thread, ThreadPriority prio=NoPriority);

  void set_saved_exception_pc(address pc)        { _saved_exception_pc = pc; }
  address saved_exception_pc()                   { return _saved_exception_pc; }
  
    
  ThreadFunction entry_point() const             { return _entry_point; }

  // Allocates a new Java level thread object for this thread. thread_name may be NULL.
  void allocate_threadObj(Handle thread_group, char* thread_name, bool daemon, TRAPS);

  // Last frame anchor routines

  JavaFrameAnchor* frame_anchor(void)                { return &_anchor; }

  // last_Java_sp 
  bool has_last_Java_frame() const                   { return _anchor.has_last_Java_frame(); }
  intptr_t* last_Java_sp() const                     { return _anchor.last_Java_sp(); }

  // last_Java_pc

  address last_Java_pc(void)                         { return _anchor.last_Java_pc(); }

  // scope-desc/oopmap lookup 
  void  set_pc_not_at_call_for_frame(intptr_t* id)   { _anchor.set_pc_not_at_call_for_frame(id); }
  bool  not_at_call_frame_exists() const             { return _anchor.not_at_call_frame_exists(); }
  intptr_t* not_at_call_id() const                   { return _anchor.not_at_call_id(); }
  
  // Safepoint support
  JavaThreadState thread_state() const           { return _thread_state; }
  void set_thread_state(JavaThreadState s)       { _thread_state=s;      }
  ThreadSafepointState *safepoint_state() const  { return _safepoint_state;  }
  void set_safepoint_state(ThreadSafepointState *state) { _safepoint_state = state; }
  bool is_in_compiled_safepoint()                { return _safepoint_state->type() == ThreadSafepointState::_compiled_safepoint; }
  bool is_at_poll_safepoint()                    { return _safepoint_state->is_at_poll_safepoint(); }
  // thread has called JavaThread::exit() or is terminated
  bool is_exiting()                              { return _terminated == _thread_exiting || is_terminated(); }
  // thread is terminated (no longer on the threads list); we compare
  // against the two non-terminated values so that a freed JavaThread
  // will also be considered terminated.
  bool is_terminated()                           { return _terminated != _not_terminated && _terminated != _thread_exiting; }
  void set_terminated(TerminatedTypes t)         { _terminated = t; }
  // special for Threads::remove() which is static:
  void set_terminated_value()                    { _terminated = _thread_terminated; }
  void block_if_vm_exited();

  // Support for safepoint timeout (debugging)
  bool has_called_back()                         { return _has_called_back; }
  void set_has_called_back(bool val)             { _has_called_back = val; }

  bool doing_unsafe_access()                     { return _doing_unsafe_access; }
  void set_doing_unsafe_access(bool val)         { _doing_unsafe_access = val; }
    
  bool do_not_unlock_if_synchronized()             { return _do_not_unlock_if_synchronized; }
  void set_do_not_unlock_if_synchronized(bool val) { _do_not_unlock_if_synchronized = val; }

  
  // Suspend/resume support for JavaThread

 private:
  void set_ext_suspended()       { set_suspend_flag (_ext_suspended);  }
  void clear_ext_suspended()     { clear_suspend_flag(_ext_suspended); }

 public:
  void java_suspend();
  void java_resume();  
  int  java_suspend_self();
  static void check_safepoint_and_suspend_for_native_trans(JavaThread *thread);

// Experimental number: cannot allow wait_for_ext_suspend_completion()
// or is_ext_suspend_completed() to run forever or we could hang.
#define SUSPENDRETRYCOUNT 100
  bool is_ext_suspend_completed(bool called_by_wait, uint32_t *bits);
  bool is_ext_suspend_completed_with_lock(uint32_t *bits) {
    MutexLockerEx ml(SR_lock(), Mutex::_no_safepoint_check_flag);
    // Warning: is_ext_suspend_completed() may temporarily drop the
    // SR_lock to allow the thread to reach a stable thread state if
    // it is currently in a transient thread state.
    return is_ext_suspend_completed(false /*!called_by_wait */, bits);
  }
  bool wait_for_ext_suspend_completion(int count, uint32_t *bits);

  void set_external_suspend()     { set_suspend_flag  (_external_suspend); }
  void clear_external_suspend()   { clear_suspend_flag(_external_suspend); }

  bool is_external_suspend() const { 
    return _suspend_flags & _external_suspend; 
  }
  // Whenever a thread transitions from native to vm/java it must suspend
  // if external suspend is present.
  bool is_suspend_after_native() const { 
    return _suspend_flags & _external_suspend;
  }

  // external suspend request is completed
  bool is_ext_suspended() const { 
    return _suspend_flags & _ext_suspended;
  }

  // target is suspended, note the difference with is_ext_suspended()
  bool is_any_suspended() const {
    return is_ext_suspended() || is_vm_suspended();
  }

  bool is_external_suspend_with_lock() const {
    MutexLockerEx ml(SR_lock(), Mutex::_no_safepoint_check_flag);
    return is_external_suspend();
  }

  // Special method to handle a pending external suspend request
  // when a suspend equivalent condition lifts.
  bool handle_special_suspend_equivalent_condition() {
    assert(is_suspend_equivalent(),
      "should only be called in a suspend equivalence condition");
    MutexLockerEx ml(SR_lock(), Mutex::_no_safepoint_check_flag);
    bool ret = is_external_suspend();
    if (!ret) {
      // not about to self-suspend so clear suspend equivalence
      clear_suspend_equivalent();
    }
    // implied else:
    // We have a pending external suspend request so we leave the
    // suspend_equivalent flag set until java_suspend_self() sets
    // the ext_suspended flag and clears the suspend_equivalent
    // flag. This insures that wait_for_ext_suspend_completion()
    // will return consistent values.
    return ret;
  }

  bool is_any_suspended_with_lock() const {
    MutexLockerEx ml(SR_lock(), Mutex::_no_safepoint_check_flag);
    return is_any_suspended();
  }
  // utility methods to see if we are doing some kind of suspension
  bool is_being_ext_suspended() const            {
    MutexLockerEx ml(SR_lock(), Mutex::_no_safepoint_check_flag);
    return is_ext_suspended() || is_external_suspend();
  }

  bool is_suspend_equivalent() const             { return _suspend_equivalent; }

  void set_suspend_equivalent()                  { _suspend_equivalent = true; };
  void clear_suspend_equivalent()                { _suspend_equivalent = false; };

  // Thread.stop support
  void send_thread_stop(oop throwable);
  AsyncRequests clear_special_runtime_exit_condition() {
    AsyncRequests x = _special_runtime_exit_condition;
    _special_runtime_exit_condition = _no_async_condition;
    return x;
  }

  // Are any async conditions present? 
  bool has_async_condition() { return (_special_runtime_exit_condition != _no_async_condition); }

  void check_and_handle_async_exceptions(bool check_unsafe_error = true);

  // these next two are also used for self-suspension and async exception support
  void handle_special_runtime_exit_condition(bool check_asyncs = true);

  // Return true if JavaThread has an asynchronous condition or
  // if external suspension is requested.
  bool has_special_runtime_exit_condition() {
    // We call is_external_suspend() last since external suspend should
    // be less common. Because we don't use is_external_suspend_with_lock
    // it is possible that we won't see an asynchronous external suspend
    // request that has just gotten started, i.e., SR_lock grabbed but
    // _external_suspend field change either not made yet or not visible
    // yet. However, this is okay because the request is asynchronous and
    // we will see the new flag value the next time through. It's also
    // possible that the external suspend request is dropped after 
    // we have checked is_external_suspend(), we will recheck its value
    // under SR_lock in java_suspend_self().
    return (_special_runtime_exit_condition != _no_async_condition) ||
            is_external_suspend();
  }

  void set_pending_unsafe_access_error()          { _special_runtime_exit_condition = _async_unsafe_access_error; }

  void set_pending_async_exception(oop e) {
    _pending_async_exception = e;
    _special_runtime_exit_condition = _async_exception;
    set_has_async_exception_with_lock();
  }
  
  // Fast-locking support
  bool is_lock_owned(address adr) const;  

  // Accessors for vframe array top
  // The linked list of vframe arrays are sorted on sp. This means when we
  // unpack the head must contain the vframe array to unpack.
  void set_vframe_array_head(vframeArray* value) { _vframe_array_head = value; }
  vframeArray* vframe_array_head() const         { return _vframe_array_head;  }

#ifndef CORE
  // Side structure for defering update of java frame locals until deopt occurs
  GrowableArray<jvmtiDeferredLocalVariableSet*>* deferred_locals() const { return _deferred_locals_updates; }
  void set_deferred_locals(GrowableArray<jvmtiDeferredLocalVariableSet *>* vf) { _deferred_locals_updates = vf; }
#endif /* CORE */

  // These only really exist to make debugging deopt problems simpler

  void set_vframe_array_last(vframeArray* value) { _vframe_array_last = value; }
  vframeArray* vframe_array_last() const         { return _vframe_array_last;  }

  // The special resourceMark used during deoptimization

  void set_deopt_mark(DeoptResourceMark* value)  { _deopt_mark = value; }
  DeoptResourceMark* deopt_mark(void)            { return _deopt_mark; }

  // Oop results of vm runtime calls
  oop  vm_result() const                         { return _vm_result; }
  void set_vm_result  (oop x)                    { _vm_result   = x; }

  oop  vm_result_2() const                       { return _vm_result_2; }
  void set_vm_result_2  (oop x)                  { _vm_result_2   = x; }

  // Exception handling for compiled methods
  oop      exception_oop() const                 { return _exception_oop; }
  int      exception_stack_size() const          { return _exception_stack_size; }
  address  exception_pc() const                  { return _exception_pc; }
  address  exception_handler_pc() const          { return _exception_handler_pc; }
  
  void set_exception_oop(oop o)                  { _exception_oop = o; }
  void set_exception_pc(address a)               { _exception_pc = a; }
  void set_exception_handler_pc(address a)       { _exception_handler_pc = a; }
  void set_exception_stack_size(int size)        { _exception_stack_size = size; }

  // Stack overflow support
  inline size_t stack_available(address cur_sp);
  address stack_yellow_zone_base()
    { return (address)(stack_base() - (stack_size() - (stack_red_zone_size() + stack_yellow_zone_size()))); }
  size_t  stack_yellow_zone_size()
    { return StackYellowPages * os::vm_page_size(); }
  address stack_red_zone_base()
    { return (address)(stack_base() - (stack_size() - stack_red_zone_size())); }
  size_t stack_red_zone_size()
    { return StackRedPages * os::vm_page_size(); }
  bool in_stack_yellow_zone(address a)
    { return (a <= stack_yellow_zone_base()) && (a >= stack_red_zone_base()); }
  bool in_stack_red_zone(address a)
    { return (a <= stack_red_zone_base()) && (a >= (address)((intptr_t)stack_base() - stack_size())); }

  void create_stack_guard_pages();
  void remove_stack_guard_pages();

  void enable_stack_yellow_zone();
  void disable_stack_yellow_zone();
  void enable_stack_red_zone();
  void disable_stack_red_zone();

  inline bool stack_yellow_zone_disabled();
  inline bool stack_yellow_zone_enabled();

  // Attempt to reguard the stack after a stack overflow may have occurred.
  // Returns true if (a) guard pages are not needed on this thread, (b) the
  // pages are already guarded, or (c) the pages were successfully reguarded.
  // Returns false if there is not enough stack space to reguard the pages, in
  // which case the caller should unwind a frame and try again.  The argument
  // should be the caller's (approximate) sp.
  bool reguard_stack(address cur_sp);
  // Similar to above but see if current stackpoint is out of the guard area
  // and reguard if possible.
  bool reguard_stack(void);

  // Misc. accessors/mutators
  void set_do_not_unlock(void)                   { _do_not_unlock_if_synchronized = true; }
  void clr_do_not_unlock(void)                   { _do_not_unlock_if_synchronized = false; }
  bool do_not_unlock(void)                       { return _do_not_unlock_if_synchronized; }

#ifndef PRODUCT
  void record_jump(address target, address instr, const char* file, int line);
#endif /* PRODUCT */

  // For assembly stub generation
  static ByteSize threadObj_offset()             { return byte_offset_of(JavaThread, _threadObj           ); }
#ifndef PRODUCT
  static ByteSize jmp_ring_index_offset()        { return byte_offset_of(JavaThread, _jmp_ring_index      ); }
  static ByteSize jmp_ring_offset()              { return byte_offset_of(JavaThread, _jmp_ring            ); }
#endif /* PRODUCT */
  static ByteSize jni_environment_offset()       { return byte_offset_of(JavaThread, _jni_environment     ); }
  static ByteSize last_Java_sp_offset()          {
    return byte_offset_of(JavaThread, _anchor) + JavaFrameAnchor::last_Java_sp_offset();
  }
  static ByteSize last_Java_pc_offset()          {
    return byte_offset_of(JavaThread, _anchor) + JavaFrameAnchor::last_Java_pc_offset();
  }
  static ByteSize frame_anchor_offset()          {
    return byte_offset_of(JavaThread, _anchor);
  }
  static ByteSize frame_anchor_flags_offset()          {
    return byte_offset_of(JavaThread, _anchor) + JavaFrameAnchor::flags_offset();
  }
  static ByteSize vm_result_offset()             { return byte_offset_of(JavaThread, _vm_result           ); }
  static ByteSize vm_result_2_offset()           { return byte_offset_of(JavaThread, _vm_result_2         ); }
  static ByteSize thread_state_offset()          { return byte_offset_of(JavaThread, _thread_state        ); }  
  static ByteSize saved_exception_pc_offset()    { return byte_offset_of(JavaThread, _saved_exception_pc  ); }
  static ByteSize osthread_offset()              { return byte_offset_of(JavaThread, _osthread            ); }
  static ByteSize exception_oop_offset()         { return byte_offset_of(JavaThread, _exception_oop       ); }
  static ByteSize exception_pc_offset()          { return byte_offset_of(JavaThread, _exception_pc        ); }
  static ByteSize exception_handler_pc_offset()  { return byte_offset_of(JavaThread, _exception_handler_pc); }
  static ByteSize exception_stack_size_offset()  { return byte_offset_of(JavaThread, _exception_stack_size); }
  static ByteSize stack_guard_state_offset()     { return byte_offset_of(JavaThread, _stack_guard_state   ); }
  static ByteSize suspend_flags_offset()         { return byte_offset_of(JavaThread, _suspend_flags       ); }

  static ByteSize do_not_unlock_if_synchronized_offset() { return byte_offset_of(JavaThread, _do_not_unlock_if_synchronized); }

  // Returns the jni environment for this thread
  JNIEnv* jni_environment()                      { return &_jni_environment; }

  static JavaThread* thread_from_jni_environment(JNIEnv* env) {
    JavaThread *thread_from_jni_env = (JavaThread*)((intptr_t)env - in_bytes(jni_environment_offset()));
    // Only return NULL if thread is off the thread list; starting to
    // exit should not return NULL.
    if (thread_from_jni_env->is_terminated()) {
       thread_from_jni_env->block_if_vm_exited();
       return NULL;
    } else {
       return thread_from_jni_env;
    }
  }

  // JNI critical regions. These can nest.
  bool in_critical()    { return _jni_active_critical > 0; }
  void enter_critical() { assert(Thread::current() == this,
                                 "this must be current thread");
                          _jni_active_critical++; }
  void exit_critical()  { assert(Thread::current() == this,
                                 "this must be current thread");
                          _jni_active_critical--; 
                          assert(_jni_active_critical >= 0,
                                 "JNI critical nesting problem?"); }

  // For deadlock detection
  int depth_first_number() { return _depth_first_number; }
  void set_depth_first_number(int dfn) { _depth_first_number = dfn; }
  
 private:
  void set_monitor_chunks(MonitorChunk* monitor_chunks) { _monitor_chunks = monitor_chunks; } 

 public:
  MonitorChunk* monitor_chunks() const           { return _monitor_chunks; }
  void add_monitor_chunk(MonitorChunk* chunk);
  void remove_monitor_chunk(MonitorChunk* chunk);
  bool in_deopt_handler() const                  { return _in_deopt_handler > 0; }
  void inc_in_deopt_handler()                    { _in_deopt_handler++; }
  void dec_in_deopt_handler()                    {
    assert(_in_deopt_handler > 0, "mismatched deopt nesting");
    if (_in_deopt_handler > 0) { // robustness
      _in_deopt_handler--;
    }
  }

 private:
  void set_entry_point(ThreadFunction entry_point) { _entry_point = entry_point; }

 public:
     
  // Frame iteration; calls the function f for all frames on the stack 
  void frames_do(void f(frame*, const RegisterMap*));
  
  // Memory operations
  void oops_do(OopClosure* f);

  // Sweeper operations
  void nmethods_do();

  // Memory management operations
  void gc_epilogue();
  void gc_prologue();

  // Misc. operations
  char* name() const { return (char*)get_thread_name(); }
  void print() const;
  void print_value();
  void print_thread_state() const                PRODUCT_RETURN;
  void print_on_error(outputStream* st, char* buf, int buflen) const;
  void verify();
  const char* get_thread_name() const;
  const char* get_threadgroup_name() const;
  const char* get_parent_name() const;

  // Accessing frames
  frame last_frame() { 
    _anchor.make_walkable(this);
    return cook_last_frame(pd_last_frame()); 
  }
  frame cook_last_frame(frame fr); // adjust any patched pc  
  javaVFrame* last_java_vframe(RegisterMap* reg_map);

  // Returns method at 'depth' java or native frames down the stack  
  // Used for security checks
  klassOop security_get_caller_class(int depth);  

  // Fabricate heavyweight monitors for any lightweight monitors that this thread owns
  void jvmpi_fab_heavy_monitors(bool query, int* index, GrowableArray<ObjectMonitor*>* fab_list);

  // Print stack trace in external format
  void print_stack();

  // Print stack traces in various internal formats
  void trace_stack()                             PRODUCT_RETURN;
  void trace_stack_from(vframe* start_vf)        PRODUCT_RETURN;
  void trace_frames()                            PRODUCT_RETURN;

  // Returns the number of stack frames on the stack
  int depth() const;

  // Function for testing deoptimization
  void deoptimize();
  void make_zombies();

  void deoptimized_wrt_marked_nmethods();

  // Profiling and safepoint operation (see fprofile.cpp and safepoint.cpp)
 public:
   bool get_top_frame(frame* fr, ExtendedPC* addr = NULL, bool for_profile_only = false);
   bool profile_last_Java_frame(frame* fr);

 public:
  // support for JVM/PI {Get,Set}ThreadLocalStorage()
  void* jvmpi_data() const                       { return _jvmpi_data; }
  void set_jvmpi_data(void* data)                { _jvmpi_data = data; }

  // support for JVM/PI ThreadHasRun()
  unsigned int last_sum() const                  { return _last_sum; }
  void set_last_sum(unsigned int s)              { _last_sum = s; }

  // support for deferred OBJECT_ALLOC events
  GrowableArray<DeferredObjAllocEvent *>* deferred_obj_alloc_events() { return _deferred_obj_alloc_events; }
  void set_deferred_obj_alloc_events(GrowableArray<DeferredObjAllocEvent *>* a) { _deferred_obj_alloc_events = a; }

 private:
   ThreadProfiler* _thread_profiler;
 private:
   friend class FlatProfiler;                    // uses both [gs]et_thread_profiler.
   friend class FlatProfilerTask;                // uses get_thread_profiler.
   friend class ThreadProfilerMark;              // uses get_thread_profiler.
   ThreadProfiler* get_thread_profiler()         { return _thread_profiler; }
   ThreadProfiler* set_thread_profiler(ThreadProfiler* tp) {
     ThreadProfiler* result = _thread_profiler;
     _thread_profiler = tp;
     return result;
   }

  // Static operations
 public:
  // Returns the running thread as a JavaThread
  static inline JavaThread* current();

  // Returns the active Java thread.  Do not use this if you know you are calling
  // from a JavaThread, as it's slower than JavaThread::current.  If called from
  // the VMThread, it also returns the JavaThread that instigated the VMThread's
  // operation.  You may not want that either.
  static JavaThread* active();

 public:
  virtual void run();
  void thread_main_inner();

 private:
  // PRIVILEGED STACK
  PrivilegedElement*  _privileged_stack_top;
  GrowableArray<oop>* _array_for_gc; 
 public:
    
  // Returns the privileged_stack information.
  PrivilegedElement* privileged_stack_top() const       { return _privileged_stack_top; }
  void set_privileged_stack_top(PrivilegedElement *e)   { _privileged_stack_top = e; }
  void register_array_for_gc(GrowableArray<oop>* array) { _array_for_gc = array; }

 public:
  // Thread local information maintained by JVMTI. 
  void set_jvmti_thread_state(JvmtiThreadState *value)                           { _jvmti_thread_state = value; }
  JvmtiThreadState *jvmti_thread_state() const                                   { return _jvmti_thread_state; }
  static ByteSize jvmti_thread_state_offset()                                    { return byte_offset_of(JavaThread, _jvmti_thread_state); }
  void set_jvmti_get_loaded_classes_closure(JvmtiGetLoadedClassesClosure* value) { _jvmti_get_loaded_classes_closure = value; }
  JvmtiGetLoadedClassesClosure* get_jvmti_get_loaded_classes_closure() const     { return _jvmti_get_loaded_classes_closure; }

  // JVMTI PopFrame support
  // Setting and clearing popframe_condition
  // All of these enumerated values are bits. popframe_pending
  // indicates that a PopFrame() has been requested and not yet been
  // completed. popframe_processing indicates that that PopFrame() is in
  // the process of being completed. popframe_force_deopt_reexecution_bit
  // indicates that special handling is required when returning to a
  // deoptimized caller. popframe_compiled_return_bit indicates that
  // the PopFrame() request occurred while returning from compiled code
  // and that special handling might be required.
  enum PopCondition {
    popframe_inactive                      = 0x00,
    popframe_pending_bit                   = 0x01,
    popframe_processing_bit                = 0x02,
    popframe_force_deopt_reexecution_bit   = 0x04,
    popframe_compiled_return_bit           = 0x10
  };
  PopCondition popframe_condition()                   { return (PopCondition) _popframe_condition; }
  void set_popframe_condition(PopCondition c)         { _popframe_condition = c; }
  void set_popframe_condition_bit(PopCondition c)     { _popframe_condition |= c; }
  void clear_popframe_condition()                     { _popframe_condition = popframe_inactive; }
  static ByteSize popframe_condition_offset()         { return byte_offset_of(JavaThread, _popframe_condition); }
  bool has_pending_popframe()                         { return (popframe_condition() & popframe_pending_bit) != 0; }
  bool popframe_forcing_deopt_reexecution()           { return (popframe_condition() & popframe_force_deopt_reexecution_bit) != 0; }
  void clear_popframe_forcing_deopt_reexecution()     { _popframe_condition &= ~popframe_force_deopt_reexecution_bit; }
  bool was_popframe_compiled_return()                 { return popframe_condition() == (popframe_pending_bit | popframe_compiled_return_bit); }
#ifdef CC_INTERP
  bool pop_frame_pending(void)                        { return ((_popframe_condition & popframe_pending_bit) != 0); }
  void clr_pop_frame_pending(void)                    { _popframe_condition = popframe_inactive; }
  bool pop_frame_in_process(void)                     { return ((_popframe_condition & popframe_processing_bit)!= 0); }
  void set_pop_frame_in_process(void)                 { _popframe_condition |= popframe_processing_bit; }
  void clr_pop_frame_in_process(void)                 { _popframe_condition &= ~popframe_processing_bit; }
#endif

#ifndef CORE
 private:
  // Saved incoming arguments to popped frame.
  // Used only when popped interpreted frame returns to deoptimized frame.
  void*    _popframe_preserved_args;
  int      _popframe_preserved_args_size;

 public:
  void  popframe_preserve_args(ByteSize size_in_bytes, void* start);
  void* popframe_preserved_args();
  ByteSize popframe_preserved_args_size();
  WordSize popframe_preserved_args_size_in_words();
  void  popframe_free_preserved_args();
#endif

 private:
  JvmtiThreadState *_jvmti_thread_state;
  JvmtiGetLoadedClassesClosure* _jvmti_get_loaded_classes_closure;

  // Used by the interpreter in fullspeed mode for frame pop, method
  // entry, method exit and single stepping support. This field is
  // only set to non-zero by the VM_EnterInterpOnlyMode VM operation.
  // It can be set to zero asynchronously (i.e., without a VM operation
  // or a lock) so we have to be very careful.
  int               _interp_only_mode;

 public:
  // used by the interpreter for fullspeed debugging support (see above)
  static ByteSize interp_only_mode_offset() { return byte_offset_of(JavaThread, _interp_only_mode); }
  bool is_interp_only_mode()                { return (_interp_only_mode != 0); }
  int get_interp_only_mode()                { return _interp_only_mode; }
  void increment_interp_only_mode()         { ++_interp_only_mode; }
  void decrement_interp_only_mode()         { --_interp_only_mode; }

 private:
  ThreadStatistics *_thread_stat;

 public:
  ThreadStatistics* get_thread_stat() const    { return _thread_stat; }

 private:
  static size_t _stack_size_at_create;
 
 public:
  static inline size_t stack_size_at_create(void) { 
    return _stack_size_at_create; 
  }
  static inline void set_stack_size_at_create(size_t value) { 
    _stack_size_at_create = value;
  }
  
  // Machine dependent stuff
  #include "incls/_thread_pd.hpp.incl"

#ifndef CORE
 public:
  void set_blocked_on_compilation(bool value) {
    _blocked_on_compilation = value;
  }

  bool blocked_on_compilation() {
    return _blocked_on_compilation;
  }
 protected:
  bool         _blocked_on_compilation;

  // This state should be only in CompilerThread.  TDT FIX!!!!!

  ciEnv*        _env;
  void*         _compile_data;
  CompileLog*   _log;
  CompileTask*  _task;
  CompileQueue* _queue;

#endif

  // JSR166 per-thread parker
private:
  Parker*    _parker;
public:
  Parker*     parker() { return _parker; }
};

// Inline implementation of JavaThread::current
inline JavaThread* JavaThread::current() {
  Thread* thread = ThreadLocalStorage::thread();
  assert(thread != NULL && thread->is_Java_thread(), "just checking");
  return (JavaThread*)thread;
}

inline bool JavaThread::stack_yellow_zone_disabled() {
  return _stack_guard_state == stack_guard_yellow_disabled;
}

inline bool JavaThread::stack_yellow_zone_enabled() {
#ifdef ASSERT
  if (os::uses_stack_guard_pages()) {
    assert(_stack_guard_state != stack_guard_unused, "guard pages must be in use");
  }
#endif
    return _stack_guard_state == stack_guard_enabled;
}

inline size_t JavaThread::stack_available(address cur_sp) {
  // This code assumes java stacks grow down
  address low_addr; // Limit on the address for deepest stack depth
  if ( _stack_guard_state == stack_guard_unused) {
    low_addr =  stack_base() - stack_size();
  } else {
    low_addr = stack_yellow_zone_base();
  }
  return cur_sp > low_addr ? cur_sp - low_addr : 0;
}

// JVMPIDaemonThread are created when a profiling agent calls jvmpi::create_system_thread
extern "C" {
typedef void (*JVMPIDaemonFunction)(void*);
}

class JVMPIDaemonThread: public JavaThread {  
  friend class VMStructs;
 private:
  JVMPIDaemonFunction _func;
 public:
  JVMPIDaemonThread(ThreadFunction entry_point, JVMPIDaemonFunction f) : JavaThread(entry_point), _func(f) {}
  JVMPIDaemonFunction function() const           { return _func; }
  virtual bool is_jvmpi_daemon_thread() const    { return true;  }
};

// A JavaThread for low memory detection support
class LowMemoryDetectorThread : public JavaThread {
  friend class VMStructs;
public:
  LowMemoryDetectorThread(ThreadFunction entry_point) : JavaThread(entry_point) {};

  // Hide this thread from external view.
  bool is_hidden_from_external_view() const      { return true; }
};

#ifndef CORE
// A thread used for Compilation.
class CompilerThread : public JavaThread {
  friend class VMStructs;
 private:
  CompilerCounters* _counters;
 public:
  CompilerThread(CompileQueue* queue, CompilerCounters* counters);

  bool is_Compiler_thread() const                { return true; }
  // Hide this compiler thread from external view.  
  bool is_hidden_from_external_view() const	 { return true; }

  CompileQueue* queue()                          { return _queue; }        
  CompilerCounters* counters()                   { return _counters; }        

  // Get/set the thread's compilation environment.
  ciEnv*        env()                            { return _env; }
  void          set_env(ciEnv* env)              { _env = env; }

  // Get/set the thread's logging information
  CompileLog*   log()                            { return _log; }
  void          init_log(CompileLog* log) {
    // Set once, for good.
    assert(_log == NULL, "set only once");
    _log = log;
  }

  // Get/set the thread's current task
  CompileTask*  task()                           { return _task; }
  void          set_task(CompileTask* task)      { _task = task; }

  void*         data()                           { return _compile_data; }
  void          set_data(void* data)             { _compile_data = data; }
};
#endif // !CORE


// The active thread queue. It also keeps track of the current used
// thread priorities.
class Threads: AllStatic {
  friend class VMStructs;
 private:
  static JavaThread* _thread_list;
  static int         _number_of_threads;
  static int         _number_of_non_daemon_threads;
  static int         _return_code;

 public:
  // Thread management
  // force_daemon is a concession to JNI, where we may need to add a
  // thread to the thread list before allocating its thread object
  static void add(JavaThread* p, bool force_daemon = false);
  static void remove(JavaThread* p);
  static bool includes(JavaThread* p);
  static JavaThread* first()                     { return _thread_list; }
  static void threads_do(ThreadClosure* tc);
  
  // Initializes the vm and creates the vm thread
  static jint create_vm(JavaVMInitArgs* args, bool* canTryAgain);
  static void convert_vm_init_libraries_to_agents();
  static void create_vm_init_libraries();
  static void create_vm_init_agents();
  static bool destroy_vm();
  // Supported VM versions via JNI
  // Includes JNI_VERSION_1_1
  static jboolean is_supported_jni_version_including_1_1(jint version);
  // Does not include JNI_VERSION_1_1
  static jboolean is_supported_jni_version(jint version);

  // Garbage collection
  static void follow_other_roots(void f(oop*));

  // Apply "f->do_oop" to all root oops in all threads.
  // This version may only be called by sequential code.
  static void oops_do(OopClosure* f);
  // This version may be called by sequential or parallel code.
  static void possibly_parallel_oops_do(OopClosure* f);  
  // This creates a list of GCTasks, one per thread.
  static void create_thread_roots_tasks(GCTaskQueue* q);

#ifndef CORE
  // Apply "f->do_oop" to roots in all threads that
  // are part of compiled frames
  static void compiled_frame_oops_do(OopClosure* f);
#endif  // !CORE

  static void convert_hcode_pointers();
  static void restore_hcode_pointers();

  // Sweeper
  static void nmethods_do();

  static void gc_epilogue();
  static void gc_prologue();

  // Verification
  static void verify();
  static void print(bool print_stacks, bool internal_format);
  static void print_on_error(outputStream* st, Thread* current, char* buf, int buflen);
  static void find_deadlocks(GrowableArray<JavaThread*>* deadlock_threads, bool print_to_tty);

  // Fabricate heavyweight monitor info for each lightweight monitor.
  static GrowableArray<ObjectMonitor*>* Threads::jvmpi_fab_heavy_monitors();

  // Get Java threads that are waiting to enter a monitor. If doLock
  // is true, then Threads_lock is grabbed as needed. Otherwise, the
  // VM needs to be at a safepoint.
  static GrowableArray<JavaThread*>* get_pending_threads(int count,
    address monitor, bool doLock);

  // Get owning Java thread from the monitor's owner field. If doLock
  // is true, then Threads_lock is grabbed as needed. Otherwise, the
  // VM needs to be at a safepoint.
  static JavaThread *owning_thread_from_monitor_owner(address owner,
    bool doLock);

  // Number of threads on the active threads list
  static int number_of_threads()                 { return _number_of_threads; }
  // Number of non-daemon threads on the active threads list
  static int number_of_non_daemon_threads()      { return _number_of_non_daemon_threads; }

  // Deoptimizes all frames tied to marked nmethods
  static void deoptimized_wrt_marked_nmethods();

};


// Thread iterator
class ThreadClosure: public StackObj {
 public:
  virtual void do_thread(Thread* thread) = 0;
};

class SignalHandlerMark: public StackObj {
#ifndef ASSERT
public:
  SignalHandlerMark(Thread* t) {};

#else
private:
  Thread* _thread;
public:
  SignalHandlerMark(Thread* t) {
    _thread = t;
    if (_thread) _thread->enter_signal_handler();
  }
  ~SignalHandlerMark() {
    if (_thread) _thread->leave_signal_handler();
    _thread = NULL;
  }
#endif
};

