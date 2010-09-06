#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)threadService.cpp	1.29 04/03/21 23:26:14 JVM"
#endif
//
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
//

# include "incls/_precompiled.incl"
# include "incls/_threadService.cpp.incl"

// TODO: we need to define a naming convention for perf counters
// to distinguish counters for:
//   - standard JSR174 use
//   - Hotspot extension (public and committed)
//   - Hotspot extension (private/internal and uncommitted)

// Default is disabled.
bool ThreadService::_thread_monitoring_contention_enabled = false;
bool ThreadService::_thread_cpu_time_enabled = false;

PerfCounter*  ThreadService::_total_threads_count = NULL;
PerfVariable* ThreadService::_live_threads_count = NULL;
PerfVariable* ThreadService::_peak_threads_count = NULL;
PerfVariable* ThreadService::_daemon_threads_count = NULL;
volatile int ThreadService::_exiting_threads_count = 0;
volatile int ThreadService::_exiting_daemon_threads_count = 0;

ThreadDumpResult* ThreadService::_threaddump_list = NULL;

void ThreadService::init() {
  EXCEPTION_MARK;

  // These counters are for java.lang.management API support.
  // They are created even if -XX:-UsePerfData is set and in
  // that case, they will be allocated on C heap.

  _total_threads_count =
                PerfDataManager::create_counter(JAVA_THREADS, "started", 
                                                PerfData::U_Events, CHECK);

  _live_threads_count =
                PerfDataManager::create_variable(JAVA_THREADS, "live",
                                                 PerfData::U_None, CHECK);

  _peak_threads_count =
                PerfDataManager::create_variable(JAVA_THREADS, "livePeak",
                                                 PerfData::U_None, CHECK);

  _daemon_threads_count =
                PerfDataManager::create_variable(JAVA_THREADS, "daemon",
                                                 PerfData::U_None, CHECK);

  if (os::is_thread_cpu_time_supported()) {
    _thread_cpu_time_enabled = true;
  } 
}

void ThreadService::reset_peak_thread_count() {
  // Acquire the lock to update the peak thread count
  // to synchronize with thread addition and removal.
  MutexLockerEx mu(Threads_lock);
  _peak_threads_count->set_value(get_live_thread_count());
}

void ThreadService::add_thread(JavaThread* thread, bool daemon) {
  // Do not count VM internal or JVMTI agent threads
  if (thread->is_hidden_from_external_view() || 
      thread->is_jvmti_agent_thread()) {
    return;
  }

  _total_threads_count->inc();
  _live_threads_count->inc();

  if (_live_threads_count->get_value() > _peak_threads_count->get_value()) {
    _peak_threads_count->set_value(_live_threads_count->get_value());
  }

  if (daemon) {
    _daemon_threads_count->inc();
  }
}

void ThreadService::remove_thread(JavaThread* thread, bool daemon) {
  Atomic::dec((jint*) &_exiting_threads_count);

  if (thread->is_hidden_from_external_view() ||
      thread->is_jvmti_agent_thread()) {
    return;
  }

  _live_threads_count->set_value(_live_threads_count->get_value() - 1);

  if (daemon) {
    _daemon_threads_count->set_value(_daemon_threads_count->get_value() - 1);
    Atomic::dec((jint*) &_exiting_daemon_threads_count);
  }
}

void ThreadService::current_thread_exiting(JavaThread* jt) {
  assert(jt == JavaThread::current(), "Called by current thread");
  Atomic::inc((jint*) &_exiting_threads_count);

  oop threadObj = jt->threadObj();
  if (threadObj != NULL && java_lang_Thread::is_daemon(threadObj)) {
    Atomic::inc((jint*) &_exiting_daemon_threads_count);
  }
}

// FIXME: JVMTI should call this function
Handle ThreadService::get_current_contended_monitor(JavaThread* thread) {
  assert(thread != NULL, "should be non-NULL");
  assert(Threads_lock->owned_by_self(), "must grab Threads_lock or be at safepoint");

  ObjectMonitor *wait_obj = thread->current_waiting_monitor();

  oop obj = NULL;
  if (wait_obj != NULL) {
    // thread is doing an Object.wait() call
    obj = (oop) wait_obj->object();
    assert(obj != NULL, "Object.wait() should have an object");
  } else {
    ObjectMonitor *enter_obj = thread->current_pending_monitor();
    if (enter_obj != NULL) {
      // thread is trying to enter() or raw_enter() an ObjectMonitor.
      obj = (oop) enter_obj->object();
    }
    // If obj == NULL, then ObjectMonitor is raw which doesn't count.
  }  

  Handle h(obj);
  return h;
}

bool ThreadService::set_thread_monitoring_contention(bool flag) {
  MutexLocker m(Management_lock);
  
  bool prev = _thread_monitoring_contention_enabled;
  _thread_monitoring_contention_enabled = flag;

  return prev;
}

bool ThreadService::set_thread_cpu_time_enabled(bool flag) {
  MutexLocker m(Management_lock);
  
  bool prev = _thread_cpu_time_enabled;
  _thread_cpu_time_enabled = flag;

  return prev;
}

// GC support
void ThreadService::oops_do(OopClosure* f) {
  for (ThreadDumpResult* dump = _threaddump_list; dump != NULL; dump = dump->next()) {
    for (int i = 0; i < dump->num_threads(); i++) {
      ThreadStackTrace* stacktrace = dump->get_stack_trace(i);
      if (stacktrace != NULL) {
        stacktrace->oops_do(f);
      }
    }
  }
}

void ThreadService::add_thread_dump(ThreadDumpResult* dump) {
  MutexLocker ml(Management_lock);
  if (_threaddump_list == NULL) {
    _threaddump_list = dump;
  } else {
    dump->set_next(_threaddump_list);
    _threaddump_list = dump;
  }
}

void ThreadService::remove_thread_dump(ThreadDumpResult* dump) {
  MutexLocker ml(Management_lock);

  ThreadDumpResult* prev = NULL;
  bool found = false;
  for (ThreadDumpResult* d = _threaddump_list; d != NULL; prev = d, d = d->next()) {
    if (d == dump) {
      if (prev == NULL) {
        _threaddump_list = dump->next();
      } else {
        prev->set_next(dump->next());
      }
      found = true;
      break;
    }
  }
  assert(found, "The threaddump result to be removed must exist.");
}

// Perhaps this function should be in JavaThread class.
void ThreadService::dump_stack_at_safepoint(JavaThread* thread, ThreadStackTrace* stacktrace, int maxDepth) {
  assert(SafepointSynchronize::is_at_safepoint(), "all threads are stopped");  

  if (!thread->has_last_Java_frame()) {
    return;
  }

  vframeStream st(thread);
  int count;
  for (count = 0; !st.at_end(); st.next(), count++) {
    if (maxDepth > 0 && count == maxDepth) {
      break;
    }
    stacktrace->add_stack_trace_element(st.method(), st.bci());
  }
}

// Dump stack trace of a list of threads and take the thread snapshots
// if snapshots is non-null. maxDepth == -1 indicates that all stack traces
// are dumped.
//
Handle ThreadService::dump_stack_traces(GrowableArray<instanceHandle>* threads, 
                                        ThreadSnapshot** snapshots, 
                                        int num_threads, int maxDepth, TRAPS) {
  assert(maxDepth == -1 || maxDepth > 0 && num_threads > 0, "just checking");

  // Create a new ThreadDumpResult object and append to the list.
  // If GC happens before this function returns, methodOop
  // in the stack trace will be visited.
  ThreadDumpResult dump_result(num_threads);
  ThreadService::add_thread_dump(&dump_result);

  // TODO: Optimization if only the current thread or maxDepth = 1
  VM_ThreadDump op(threads, snapshots, num_threads, maxDepth, &dump_result, THREAD);
  VMThread::execute(&op);

  // Allocate the resulting StackTraceElement[][] object

  ResourceMark rm(THREAD); 
  klassOop k = SystemDictionary::resolve_or_fail(vmSymbolHandles::java_lang_StackTraceElement_array(), true, CHECK_0);
  objArrayKlassHandle ik (THREAD, k);
  objArrayOop r = oopFactory::new_objArray(ik(), num_threads, CHECK_0);
  objArrayHandle result_obj(THREAD, r);
  for (int i = 0; i < num_threads; i++) {
    ThreadStackTrace* stacktrace = dump_result.get_stack_trace(i);
    if (stacktrace == NULL) {
      result_obj->obj_at_put(i, NULL);
    } else {
      // Construct an array of java/lang/StackTraceElement object for each thread
      Handle backtrace = stacktrace->allocate_fill_stack_trace_element_array(CHECK_0);     
      result_obj->obj_at_put(i, backtrace());
    }
  }

  // Remove the thread dump result before return
  ThreadService::remove_thread_dump(&dump_result);
  return result_obj;
}

void ThreadService::reset_contention_count_stat(JavaThread* thread) {
  ThreadStatistics* stat = thread->get_thread_stat();
  if (stat != NULL) {
    stat->reset_count_stat();
  }
}

void ThreadService::reset_contention_time_stat(JavaThread* thread) {
  ThreadStatistics* stat = thread->get_thread_stat();
  if (stat != NULL) {
    stat->reset_time_stat();
  }
}

ThreadDumpResult::ThreadDumpResult(int num_threads) {
  _num_threads = num_threads;
  _traces = (ThreadStackTrace**) NEW_C_HEAP_ARRAY(ThreadStackTrace*, _num_threads);
  for (int i = 0; i < _num_threads; i++) {
    _traces[i] = NULL;
  }
  _next = NULL;
}

ThreadDumpResult::~ThreadDumpResult() {
  // free all the ThreadStackTrace objects created during
  // the VM_ThreadDump operation
  for (int i = 0; i < _num_threads; i++) {
    delete _traces[i];
  }
  FREE_C_HEAP_ARRAY(ThreadStackTrace*, _traces);
}

ThreadStackTrace::ThreadStackTrace(JavaThread* t) {
  _methods = new (ResourceObj::C_HEAP) GrowableArray<methodOop>(init_stack_size, true);
  _bcis = new (ResourceObj::C_HEAP) GrowableArray<int>(init_stack_size, true);
  _depth = 0;
}

ThreadStackTrace::~ThreadStackTrace() {
  _methods->clear_and_deallocate();
  _bcis->clear_and_deallocate();
}

Handle ThreadStackTrace::allocate_fill_stack_trace_element_array(TRAPS) {
  klassOop k = SystemDictionary::stackTraceElement_klass();
  instanceKlassHandle ik(THREAD, k);

  // Allocate an array of java/lang/StackTraceElement object
  objArrayOop ste = oopFactory::new_objArray(ik(), _depth, CHECK_0);
  objArrayHandle backtrace(THREAD, ste);
  for (int j = 0; j < _depth; j++) {
    methodHandle mh(THREAD, method_at(j));
    oop element = java_lang_StackTraceElement::create(mh, bci_at(j), CHECK_0);
    backtrace->obj_at_put(j, element);
  }
  return backtrace;
}

void ThreadStackTrace::add_stack_trace_element(methodOop m, int bci) {
  _methods->append(m);
  _bcis->append(bci);
  _depth++;
}

void ThreadStackTrace::oops_do(OopClosure* f) {
  int length = _methods->length();
  for (int i = 0; i < length; i++) {
    f->do_oop((oop*) _methods->adr_at(i));
  }
}

ThreadStatistics::ThreadStatistics() {
  _contended_enter_count = 0;
  _monitor_wait_count = 0;
  _class_init_recursion_count = 0;
  _class_verify_recursion_count = 0;
  _count_pending_reset = false;
  _timer_pending_reset = false;
}

ThreadSnapshot::ThreadSnapshot(JavaThread* thread, Thread* cur_thread) {  
  ThreadStatistics* stat = thread->get_thread_stat();
  _contended_enter_ticks = stat->contended_enter_ticks();
  _contended_enter_count = stat->contended_enter_count();
  _monitor_wait_ticks = stat->monitor_wait_ticks();
  _monitor_wait_count = stat->monitor_wait_count();

  _thread_status = java_lang_Thread::get_thread_status(thread->threadObj());
  _is_ext_suspended = thread->is_being_ext_suspended();
  _is_in_native = (thread->thread_state() == _thread_in_native);

  _blocked_object = NULL;
  _blocked_object_owner = NULL;

  if (_thread_status == java_lang_Thread::BLOCKED_ON_MONITOR_ENTER ||
      _thread_status == java_lang_Thread::IN_OBJECT_WAIT ||
      _thread_status == java_lang_Thread::IN_OBJECT_WAIT_TIMED) {

    Handle obj = ThreadService::get_current_contended_monitor(thread);
    if (obj() != NULL) {
      _blocked_object = JNIHandles::make_local(cur_thread, obj());
      JavaThread* owner = ObjectSynchronizer::get_lock_owner(obj, false);
      if (owner != NULL) {
        _blocked_object_owner = JNIHandles::make_local(cur_thread, owner->threadObj());
      }
    } else {
      // Thread now no longer blocks
      _thread_status = java_lang_Thread::RUNNABLE;
    }
  }
}


ThreadsListEnumerator::ThreadsListEnumerator(Thread* cur_thread, bool include_jvmti_agent_threads) {
  assert(cur_thread == Thread::current(), "Check current thread");

  int init_size = ThreadService::get_live_thread_count();
  _threads_array = new GrowableArray<instanceHandle>(init_size);

  MutexLockerEx ml(Threads_lock);

  for (JavaThread* jt = Threads::first(); jt != NULL; jt = jt->next()) {
    // skips JavaThreads in the process of exiting or attached via JNI
    // and also skips VM internal JavaThreads
    // Threads in _thread_new or _thread_new_trans state are included.
    // i.e. threads have been started but not yet running.
    if (jt->threadObj() == NULL   ||
        jt->is_exiting() ||
        !java_lang_Thread::is_alive(jt->threadObj())   ||
        jt->is_hidden_from_external_view()) {
      continue;
    }

    // skip agent threads
    if (!include_jvmti_agent_threads && jt->is_jvmti_agent_thread()) {
      continue;
    }

    instanceHandle h(cur_thread, (instanceOop) jt->threadObj());
    _threads_array->append(h);
  }
}
