#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)threadService.hpp	1.23 04/03/21 23:26:14 JVM"
#endif
//
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
//

class OopClosure;
class ThreadDumpResult;
class ThreadStackTrace;
class ThreadSnapshot;

// VM monitoring and management support for the thread and 
// synchronization subsystem
//
// Thread contention monitoring is disabled by default.
// When enabled, the VM will begin measuring the accumulated
// elapsed time a thread blocked on synchronization.
//
class ThreadService : public AllStatic {
private:
  // These counters could be moved to Threads class 
  static PerfCounter*  _total_threads_count;
  static PerfVariable* _live_threads_count;
  static PerfVariable* _peak_threads_count;
  static PerfVariable* _daemon_threads_count;

  // These 2 counters are atomically incremented once the thread is exiting.
  // They will be atomically decremented when ThreadService::remove_thread is called.
  static volatile int  _exiting_threads_count;
  static volatile int  _exiting_daemon_threads_count;

  static bool          _thread_monitoring_contention_enabled;
  static bool          _thread_cpu_time_enabled;

  // Need to keep the list of thread dump result that
  // keep references to methodOop since thread dump can be
  // requested by multiple threads concurrently.
  static ThreadDumpResult* _threaddump_list;

public:
  static void init();
  static void add_thread(JavaThread* thread, bool daemon);
  static void remove_thread(JavaThread* thread, bool daemon);
  static void current_thread_exiting(JavaThread* jt);

  static bool set_thread_monitoring_contention(bool flag);
  static bool is_thread_monitoring_contention() {
    return _thread_monitoring_contention_enabled;
  }

  static bool set_thread_cpu_time_enabled(bool flag);
  static bool is_thread_cpu_time_enabled() {
    return _thread_cpu_time_enabled;
  }

  static jlong get_total_thread_count()       { return _total_threads_count->get_value(); }
  static jlong get_peak_thread_count()        { return _peak_threads_count->get_value(); }
  static jlong get_live_thread_count()        { return _live_threads_count->get_value() - _exiting_threads_count; }
  static jlong get_daemon_thread_count()      { return _daemon_threads_count->get_value() - _exiting_daemon_threads_count; }

  static int   exiting_threads_count()        { return _exiting_threads_count; }
  static int   exiting_daemon_threads_count() { return _exiting_daemon_threads_count; }

  // Support for thread dump
  static void   add_thread_dump(ThreadDumpResult* dump);
  static void   remove_thread_dump(ThreadDumpResult* dump);

  static Handle get_current_contended_monitor(JavaThread* thread);

  // This function is called by JVM_DumpThreads.
  static Handle dump_stack_traces(GrowableArray<instanceHandle>* threads, 
                                  int num_threads, TRAPS) {
    return dump_stack_traces(threads, NULL, num_threads, -1, CHECK_0);
  }

  // Dump stack trace of a list of threads and take the thread snapshots
  // if snapshots is non-null. 
  // maxDepth == -1 indicates that all stack traces are dumped.
  static Handle dump_stack_traces(GrowableArray<instanceHandle>* threads, 
                                  ThreadSnapshot** snapshots,
                                  int num_threads, 
                                  int maxDepth, 
                                  TRAPS);

  static void dump_stack_at_safepoint(JavaThread* thread, ThreadStackTrace* stacktrace, int depth);

  static void  reset_peak_thread_count();
  static void  reset_contention_count_stat(JavaThread* thread);
  static void  reset_contention_time_stat(JavaThread* thread);

  // GC support
  static void oops_do(OopClosure* f);
};

// Per-thread Statistics for synchronization
class ThreadStatistics : public CHeapObj {
private:
  // The following contention statistics are only updated by
  // the thread owning these statistics when contention occurs.

  jlong        _contended_enter_count;
  jlong        _monitor_wait_count;
  elapsedTimer _monitor_wait_timer;
  elapsedTimer _contended_enter_timer;

  // These two reset flags are set to true when another thread 
  // requests to reset the statistics.  The actual statistics
  // are reset when the thread contention occurs and attempts
  // to update the statistics.
  bool         _count_pending_reset;
  bool         _timer_pending_reset;

  // Keep accurate times for potentially recursive class operations
  int          _class_init_recursion_count;
  int          _class_verify_recursion_count;

  // utility functions
  void  check_and_reset_count()            { 
                                             if (!_count_pending_reset) return;
                                             _contended_enter_count = 0;
                                             _monitor_wait_count = 0;
                                             _count_pending_reset = 0;
                                           }
  void  check_and_reset_timer()            { 
                                             if (!_timer_pending_reset) return;
                                             _contended_enter_timer.reset();
                                             _monitor_wait_timer.reset();
                                             _timer_pending_reset = 0;
                                           }

public:
  ThreadStatistics();

  jlong contended_enter_count()            { return (_count_pending_reset ? 0 : _contended_enter_count); }
  jlong monitor_wait_count()               { return (_count_pending_reset ? 0 : _monitor_wait_count); }
  jlong contended_enter_ticks()            { return (_timer_pending_reset ? 0 : _contended_enter_timer.active_ticks()); }
  jlong monitor_wait_ticks()               { return (_timer_pending_reset ? 0 : _monitor_wait_timer.active_ticks()); }

  void monitor_wait()                      { check_and_reset_count(); _monitor_wait_count++; }
  void monitor_wait_begin()                { check_and_reset_timer(); _monitor_wait_timer.start(); }
  void monitor_wait_end()                  { _monitor_wait_timer.stop(); check_and_reset_timer(); }

  void contended_enter()                   { check_and_reset_count(); _contended_enter_count++; }

  void contended_enter_begin()             { check_and_reset_timer(); _contended_enter_timer.start(); }
  void contended_enter_end()               { _contended_enter_timer.stop(); check_and_reset_timer(); }
  
  void reset_count_stat()                  { _count_pending_reset = true; }
  void reset_time_stat()                   { _timer_pending_reset = true; }

  int* class_init_recursion_count_addr()   { return &_class_init_recursion_count; }
  int* class_verify_recursion_count_addr() { return &_class_verify_recursion_count; }
};

// Thread snapshot to represent the thread state and statistics 
class ThreadSnapshot : public CHeapObj {
private:
  java_lang_Thread::ThreadStatus _thread_status;
  bool    _is_ext_suspended;
  bool    _is_in_native;

  jlong   _contended_enter_ticks;
  jlong   _contended_enter_count;
  jlong   _monitor_wait_ticks;
  jlong   _monitor_wait_count;
  jobject _blocked_object;       // JNI local handle
  jobject _blocked_object_owner; // JNI local handle

public:
  ThreadSnapshot(JavaThread* thread, Thread* req_thread);
  ~ThreadSnapshot() {
    if (_blocked_object != NULL) {
      JNIHandles::destroy_local(_blocked_object);
    }
  }

  java_lang_Thread::ThreadStatus thread_status() { return _thread_status; }
  bool  is_ext_suspended()       { return _is_ext_suspended; }
  bool  is_in_native()           { return _is_in_native; }

  jlong contended_enter_count()  { return _contended_enter_count; }
  jlong contended_enter_ticks()  { return _contended_enter_ticks; }
  jlong monitor_wait_count()     { return _monitor_wait_count; }
  jlong monitor_wait_ticks()     { return _monitor_wait_ticks; }

  oop   blocked_object() { 
    if (_blocked_object != NULL) {
      return JNIHandles::resolve_non_null(_blocked_object); 
    } else {
      return NULL;
    }
  }
  oop   blocked_object_owner() { 
    if (_blocked_object_owner != NULL) {
      return JNIHandles::resolve_non_null(_blocked_object_owner); 
    } else {
      return NULL;
    }
  }
};

// ThreadStackTrace for keeping methodOop and bci during
// stack walking for later construction of StackTraceElement[]
// Java instances
class ThreadStackTrace : public CHeapObj {
 private:
  enum {
    init_stack_size = 32
  };

  int                       _depth;  // number of stack frames added
  GrowableArray<methodOop>* _methods;
  GrowableArray<int>*       _bcis;

 public:

  ThreadStackTrace(JavaThread* thread);
  ~ThreadStackTrace();

  void add_stack_trace_element(methodOop m, int bci);
  int  get_stack_depth()     { return _depth; }
  methodOop method_at(int i) { return _methods->at(i); }
  int       bci_at(int i)    { return _bcis->at(i); }
  Handle allocate_fill_stack_trace_element_array(TRAPS);

  void oops_do(OopClosure* f);
};

class ThreadDumpResult : public StackObj {
private:
  ThreadStackTrace** _traces;
  int                _num_threads;
  ThreadDumpResult*  _next;
public:
  ThreadDumpResult(int num_threads);
  ~ThreadDumpResult();

  void set_stack_trace(int index, ThreadStackTrace* stacktrace) {
    assert(index >= 0 && index < _num_threads, "out of range");
    _traces[index] = stacktrace;
  }
  ThreadStackTrace* get_stack_trace(int index) {
    assert(index >= 0 && index < _num_threads, "out of range");
    return _traces[index];
  }
  int num_threads()                     { return _num_threads; }
  ThreadDumpResult* next()              { return _next; }
  void set_next(ThreadDumpResult* next) { _next = next; }
};

// Utility class to get list of java threads.
class ThreadsListEnumerator : public StackObj {
private:
  GrowableArray<instanceHandle>* _threads_array;
public:
  ThreadsListEnumerator(Thread* cur_thread, bool include_jvmti_agent_threads = false);
  int            num_threads()            { return _threads_array->length(); }
  instanceHandle get_threadObj(int index) { return _threads_array->at(index); }
};


// abstract utility class to set new thread states, and restore previous after the block exits
class JavaThreadStatusChanger : public StackObj {
 private:
  java_lang_Thread::ThreadStatus _old_state;
  JavaThread*  _java_thread;
  bool _is_alive;

  void save_old_state(JavaThread* java_thread) {
    _java_thread  = java_thread;
    _is_alive = (_java_thread != NULL) && (_java_thread->threadObj() != NULL);
    if (is_alive()) {
      _old_state = java_lang_Thread::get_thread_status(_java_thread->threadObj());
    }
  }

 public:
  void set_thread_status(java_lang_Thread::ThreadStatus state) {
    if (is_alive()) {
      java_lang_Thread::set_thread_status(_java_thread->threadObj(), state);
    }
  }
    
  JavaThreadStatusChanger(JavaThread* java_thread,
                          java_lang_Thread::ThreadStatus state) {
    save_old_state(java_thread);
    set_thread_status(state);
  }

  JavaThreadStatusChanger(JavaThread* java_thread) {
    save_old_state(java_thread);
  }

  ~JavaThreadStatusChanger() {
    set_thread_status(_old_state);
  }
  bool is_alive() {
    return _is_alive;
  }
};

// Change status to waiting on an object  (timed or indefinite)
class JavaThreadInObjectWaitState : public JavaThreadStatusChanger {
 private:
  ThreadStatistics* _stat; 
  bool _active;
    
 public:
  JavaThreadInObjectWaitState(JavaThread *java_thread, bool timed) :
    JavaThreadStatusChanger(java_thread, 
                            timed ? java_lang_Thread::IN_OBJECT_WAIT_TIMED : java_lang_Thread::IN_OBJECT_WAIT) {
    if (is_alive()) {
      _stat = java_thread->get_thread_stat();
      _active = ThreadService::is_thread_monitoring_contention();
      _stat->monitor_wait();
      if (_active) {
        _stat->monitor_wait_begin();
      }
    } else {
      _active = false;
    }
  }

  ~JavaThreadInObjectWaitState() {
    if (_active) {
      _stat->monitor_wait_end();
    }
  }
};

// Change status to parked (timed or indefinite)
class JavaThreadParkedState : public JavaThreadStatusChanger {
 private:
  ThreadStatistics* _stat; 
  bool _active;
    
 public:
  JavaThreadParkedState(JavaThread *java_thread, bool timed) :
    JavaThreadStatusChanger(java_thread, 
                            timed ? java_lang_Thread::PARKED_TIMED : java_lang_Thread::PARKED) {
    if (is_alive()) {
      _stat = java_thread->get_thread_stat();
      _active = ThreadService::is_thread_monitoring_contention();
      _stat->monitor_wait();
      if (_active) {
        _stat->monitor_wait_begin();
      }
    } else {
      _active = false;
    }
  }

  ~JavaThreadParkedState() {
    if (_active) {
      _stat->monitor_wait_end();
    }
  }
};

// Change status to blocked on (re-)entering a synchronization block
class JavaThreadBlockedOnMonitorEnterState : public JavaThreadStatusChanger {
 private:
  ThreadStatistics* _stat; 
  bool _active;
 public:
  JavaThreadBlockedOnMonitorEnterState(JavaThread *java_thread, ObjectMonitor *obj_m) :
    JavaThreadStatusChanger(java_thread) {
    assert((java_thread != NULL), "Java thread should not be null here");
    // Change thread status and collect contended enter stats for monitor contended
    // enter done for external java world objects and it is contended. All other cases
    // like for vm internal objects and for external objects which are not contended
    // thread status is not changed and contended enter stat is not collected.
    if (is_alive() && ServiceUtil::visible_oop((oop)obj_m->object()) && obj_m->owner() !=NULL) {
      set_thread_status(java_lang_Thread::BLOCKED_ON_MONITOR_ENTER);
      _stat = java_thread->get_thread_stat();
      _stat->contended_enter();
      _active = ThreadService::is_thread_monitoring_contention();
      if (_active) {
        _stat->contended_enter_begin();
      }
    } else {
      _active = false;
    } 
  }

  ~JavaThreadBlockedOnMonitorEnterState() {
    if (_active) {
      _stat->contended_enter_end();
    }
  }
};

// Change status to sleeping
class JavaThreadSleepState : public JavaThreadStatusChanger {
 public:
  JavaThreadSleepState(JavaThread *java_thread) :
    JavaThreadStatusChanger(java_thread, java_lang_Thread::SLEEPING) {}
};
