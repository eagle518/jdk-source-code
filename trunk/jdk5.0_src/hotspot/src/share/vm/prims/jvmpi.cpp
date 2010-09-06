#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)jvmpi.cpp	1.155 04/04/05 10:43:45 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

# include "incls/_precompiled.incl"
# include "incls/_jvmpi.cpp.incl"


//-------------------------------------------------------

// Unsolved problems:
//
// CPU profiling
// - we need an exact mapping of pc/bci to lineNo; this is not
//   always possible as:
//    - interpreted has not set up the frame completely
//    - compiled code is not at a safepoint

//-------------------------------------------------------


// define raw monitor validity checking
static int PROF_RM_MAGIC = (int)(('P' << 24) | ('I' << 16) | ('R' << 8) | 'M');
#define PROF_RM_CHECK(m)                                 \
    ((m) != NULL && (m)->magic() == PROF_RM_MAGIC) 


unsigned int jvmpi::_event_flags = JVMPI_PROFILING_OFF;
unsigned int jvmpi::_event_flags_array[JVMPI_MAX_EVENT_TYPE_VAL + 1];
JVMPI_Interface jvmpi::jvmpi_interface;
bool jvmpi::slow_allocation = false;

class VM_JVMPIPostObjAlloc: public VM_Operation {
 private:
  static volatile bool _restrict_event_posting;
  static Thread *      _create_thread;

 public:
  VM_JVMPIPostObjAlloc() {
    // local fields are initialized when declared
  }

  ~VM_JVMPIPostObjAlloc() {
    clear_restriction();
  }

  static void clear_restriction();
  static const Thread *create_thread() {
    return _create_thread;
  }
  void doit();
  const char* name() const {
    return "post JVMPI object allocation";
  }
  static void set_create_thread(Thread *thr) {
    _create_thread = thr;
  }
  static void wait_if_restricted();
};

void jvmpi::initialize(int version) {
  // Exit with an error if we are using a jvmpi-incompatible garbage collector,
  // unless explicitly overridden via JVMPICheckGCCompatibility (needed for using
  // Analyzer with these non-jvmpi collectors; see bug 4889433).
  if (JVMPICheckGCCompatibility && 
      (UseTrainGC || UseConcMarkSweepGC || UseParNewGC || UseParallelGC)) {
    vm_exit_during_initialization(
      "JVMPI not supported with this garbage collector; "
      "please refer to the GC/JVMPI documentation");
  }

  // The creating thread requests the VM_JVMPIPostObjAlloc VM operation
  // so it the restriction should not apply to its events (if any).
  VM_JVMPIPostObjAlloc::set_create_thread(ThreadLocalStorage::thread());

  // Enable JVMPI
  _event_flags |= JVMPI_PROFILING_ON;

  // First, initialize all JVMPI defined event notifications 
  // to be not available
  for (int i= 0; i <= JVMPI_MAX_EVENT_TYPE_VAL; i++) {
    _event_flags_array[i] = JVMPI_EVENT_NOT_SUPPORTED;
  }

  // Then, initialize events supported by the HotSpot VM
  // to be initially disabled.
  disable_event(JVMPI_EVENT_CLASS_LOAD);
  disable_event(JVMPI_EVENT_CLASS_UNLOAD);
  disable_event(JVMPI_EVENT_CLASS_LOAD_HOOK);
  disable_event(JVMPI_EVENT_OBJECT_ALLOC);
  disable_event(JVMPI_EVENT_OBJECT_FREE);
  // JVMPI_VERSION_1_1 is upward compatible from JVMPI_VERSION_1 so enable
  // the INSTRUCTION_START event
  disable_event(JVMPI_EVENT_INSTRUCTION_START);
  disable_event(JVMPI_EVENT_THREAD_START);
  disable_event(JVMPI_EVENT_THREAD_END);
  disable_event(JVMPI_EVENT_JNI_GLOBALREF_ALLOC);
  disable_event(JVMPI_EVENT_JNI_GLOBALREF_FREE);
  disable_event(JVMPI_EVENT_JNI_WEAK_GLOBALREF_ALLOC);
  disable_event(JVMPI_EVENT_JNI_WEAK_GLOBALREF_FREE);
  disable_event(JVMPI_EVENT_METHOD_ENTRY);
  disable_event(JVMPI_EVENT_METHOD_ENTRY2);
  disable_event(JVMPI_EVENT_METHOD_EXIT);
  disable_event(JVMPI_EVENT_LOAD_COMPILED_METHOD);
  disable_event(JVMPI_EVENT_UNLOAD_COMPILED_METHOD);
  disable_event(JVMPI_EVENT_JVM_INIT_DONE);
  disable_event(JVMPI_EVENT_JVM_SHUT_DOWN);
  disable_event(JVMPI_EVENT_DUMP_DATA_REQUEST);
  disable_event(JVMPI_EVENT_RESET_DATA_REQUEST);
  disable_event(JVMPI_EVENT_OBJECT_MOVE);
  disable_event(JVMPI_EVENT_ARENA_NEW);
  disable_event(JVMPI_EVENT_DELETE_ARENA);
  disable_event(JVMPI_EVENT_RAW_MONITOR_CONTENDED_ENTER);
  disable_event(JVMPI_EVENT_RAW_MONITOR_CONTENDED_ENTERED);
  disable_event(JVMPI_EVENT_RAW_MONITOR_CONTENDED_EXIT);
  disable_event(JVMPI_EVENT_MONITOR_CONTENDED_ENTER);
  disable_event(JVMPI_EVENT_MONITOR_CONTENDED_ENTERED);
  disable_event(JVMPI_EVENT_MONITOR_CONTENDED_EXIT);
  disable_event(JVMPI_EVENT_MONITOR_WAIT);
  disable_event(JVMPI_EVENT_MONITOR_WAITED);
  disable_event(JVMPI_EVENT_GC_START);
  disable_event(JVMPI_EVENT_GC_FINISH);

  // return highest upward compatible version number
  if (UseSuspendResumeThreadLists) {
    jvmpi_interface.version               = JVMPI_VERSION_1_2;
  } else {
    jvmpi_interface.version               = JVMPI_VERSION_1_1;
  }
  
  // initialize the jvmpi_interface functions
  jvmpi_interface.EnableEvent             = &enable_event;
  jvmpi_interface.DisableEvent            = &disable_event;
  
  jvmpi_interface.RequestEvent            = &request_event;
  jvmpi_interface.GetCallTrace            = &get_call_trace;
  jvmpi_interface.GetCurrentThreadCpuTime = &get_current_thread_cpu_time;
  jvmpi_interface.ProfilerExit            = &profiler_exit;
  jvmpi_interface.RawMonitorCreate        = &raw_monitor_create;
  jvmpi_interface.RawMonitorEnter         = &raw_monitor_enter;
  jvmpi_interface.RawMonitorExit          = &raw_monitor_exit;
  jvmpi_interface.RawMonitorWait          = &raw_monitor_wait;
  jvmpi_interface.RawMonitorNotifyAll     = &raw_monitor_notify_all;
  jvmpi_interface.RawMonitorDestroy       = &raw_monitor_destroy;
  jvmpi_interface.SuspendThread           = &suspend_thread;
  jvmpi_interface.ResumeThread            = &resume_thread;
  jvmpi_interface.GetThreadStatus         = &get_thread_status;
  jvmpi_interface.ThreadHasRun            = &thread_has_run;
  jvmpi_interface.CreateSystemThread      = &create_system_thread;
  jvmpi_interface.SetThreadLocalStorage   = &set_thread_local_storage;
  jvmpi_interface.GetThreadLocalStorage   = &get_thread_local_storage;
  
  jvmpi_interface.DisableGC               = &disable_gc;
  jvmpi_interface.EnableGC                = &enable_gc;
  
  jvmpi_interface.RunGC                   = &run_gc;
  jvmpi_interface.GetThreadObject         = &get_thread_object;
  jvmpi_interface.GetMethodClass          = &get_method_class;

  // JVMPI_VERSION_1_1 is upward compatible from JVMPI_VERSION_1 so set
  // up function pointers for jobjectID2jobject and jobject2jobjectID
  jvmpi_interface.jobjectID2jobject     = &jobjectID_2_jobject;
  jvmpi_interface.jobject2jobjectID     = &jobject_2_jobjectID;

  // JVMPI_VERSION_1_2 is upward compatible from previous versions, but
  // it can be turned disabled via the UseSuspendResumeThreadLists option.
  if (UseSuspendResumeThreadLists) {
    jvmpi_interface.SuspendThreadList     = &suspend_thread_list;
    jvmpi_interface.ResumeThreadList      = &resume_thread_list;
  } else {
    jvmpi_interface.SuspendThreadList     = NULL;
    jvmpi_interface.ResumeThreadList      = NULL;
  }
}


JVMPI_Interface* jvmpi::GetInterface_1(int version) {
  initialize(version);
  return &jvmpi_interface;
}

static void _pass()     { return; }
static void _block()    { 
  while (true) {
    VM_Exit::block_if_vm_exited();

    // VM has not yet reached final safepoint, but it will get there very soon
    Thread *thr = ThreadLocalStorage::get_thread_slow();
    if (thr) os::yield_all(100);     // yield_all() needs a thread on Solaris
  }
}

// disable JVMPI - this is called during VM shutdown, after the
// JVM_SHUT_DOWN event.
void jvmpi::disengage() {
  _event_flags = JVMPI_PROFILING_OFF;
  
  address block_func = CAST_FROM_FN_PTR(address, _block);
  address pass_func  = CAST_FROM_FN_PTR(address, _pass);

  // replace most JVMPI interface functions with infinite loops
  jvmpi_interface.EnableEvent =
      CAST_TO_FN_PTR(jint(*)(jint, void*), block_func);
  jvmpi_interface.DisableEvent =
      CAST_TO_FN_PTR(jint(*)(jint, void*), block_func);
  jvmpi_interface.RequestEvent =
      CAST_TO_FN_PTR(jint(*)(jint, void*), block_func);
  jvmpi_interface.GetCallTrace =
      CAST_TO_FN_PTR(void(*)(JVMPI_CallTrace*, jint), block_func);
  jvmpi_interface.GetCurrentThreadCpuTime =
      CAST_TO_FN_PTR(jlong(*)(void), block_func);
  // allow ProfilerExit to go through
  // jvmpi_interface.ProfilerExit = CAST_TO_FN_PTR(void(*)(jint), block_func);
  jvmpi_interface.RawMonitorCreate =
      CAST_TO_FN_PTR(JVMPI_RawMonitor(*)(char*), block_func);
  jvmpi_interface.RawMonitorEnter =
      CAST_TO_FN_PTR(void(*)(JVMPI_RawMonitor), block_func);
  jvmpi_interface.RawMonitorExit =
      CAST_TO_FN_PTR(void(*)(JVMPI_RawMonitor), block_func);
  jvmpi_interface.RawMonitorWait =
      CAST_TO_FN_PTR(void(*)(JVMPI_RawMonitor, jlong), block_func);
  jvmpi_interface.RawMonitorNotifyAll =
      CAST_TO_FN_PTR(void(*)(JVMPI_RawMonitor), block_func);
  jvmpi_interface.RawMonitorDestroy =
      CAST_TO_FN_PTR(void(*)(JVMPI_RawMonitor), block_func);
  jvmpi_interface.SuspendThread =
      CAST_TO_FN_PTR(void(*)(JNIEnv*), block_func);
  jvmpi_interface.ResumeThread =
      CAST_TO_FN_PTR(void(*)(JNIEnv*), block_func);
  jvmpi_interface.GetThreadStatus = 
      CAST_TO_FN_PTR(jint(*)(JNIEnv*), block_func);
  jvmpi_interface.ThreadHasRun =
      CAST_TO_FN_PTR(jboolean(*)(JNIEnv*), block_func);
  jvmpi_interface.CreateSystemThread =
      CAST_TO_FN_PTR(jint(*)(char*, jint, jvmpi_void_function_of_void), block_func);
  jvmpi_interface.SetThreadLocalStorage =
      CAST_TO_FN_PTR(void(*)(JNIEnv*, void*), block_func);
  jvmpi_interface.GetThreadLocalStorage = 
      CAST_TO_FN_PTR(void*(*)(JNIEnv*), block_func);
  jvmpi_interface.DisableGC =
      CAST_TO_FN_PTR(void(*)(void), block_func);
  jvmpi_interface.EnableGC = 
      CAST_TO_FN_PTR(void(*)(void), block_func);
  jvmpi_interface.RunGC =
      CAST_TO_FN_PTR(void(*)(void), block_func);
  jvmpi_interface.GetThreadObject =
      CAST_TO_FN_PTR(jobjectID(*)(JNIEnv*), block_func);
  jvmpi_interface.GetMethodClass =
      CAST_TO_FN_PTR(jobjectID(*)(jmethodID), block_func);
  jvmpi_interface.jobjectID2jobject = 
      CAST_TO_FN_PTR(jobject(*)(jobjectID), block_func);
  jvmpi_interface.jobject2jobjectID =
      CAST_TO_FN_PTR(jobjectID(*)(jobject), block_func);

  // NotifyEvent() is called from VM, do not block
  jvmpi_interface.NotifyEvent =
      CAST_TO_FN_PTR(void(*)(JVMPI_Event*), pass_func);
}

inline void jvmpi::post_event_common(JVMPI_Event* event) {

  // Check for restrictions related to the VM_JVMPIPostObjAlloc VM
  // operation. JavaThreads will wait here if the VM operation is
  // in process in order to prevent deadlock.
  VM_JVMPIPostObjAlloc::wait_if_restricted();

  // notify profiler agent
  jvmpi_interface.NotifyEvent(event);
}

inline void jvmpi::post_event(JVMPI_Event* event) {
  Thread* thread = Thread::current();
  assert(thread->is_Java_thread(), "expecting a Java thread");

  JavaThread* jthread = (JavaThread*)thread;
  event->env_id = jthread->jni_environment();
  // prepare to call out across JVMPI
  ThreadToNativeFromVM transition(jthread);
  HandleMark  hm(thread);  
  // notify profiler agent
  post_event_common(event);
}

// JVMPI 2.0: should cleanup race condition where calling_thread
// exits before being notified.
inline void jvmpi::post_event_vm_mode(JVMPI_Event* event, JavaThread* calling_thread) {
  Thread* thread = Thread::current();
  if (thread->is_Java_thread()) {
    // JVMPI doesn't do proper transitions on RAW_ENTRY
    // When it does do this can be enabled.
#ifdef PROPER_TRANSITIONS
    assert(((JavaThread*)thread)->thread_state() == _thread_in_vm, "Only vm mode expected");
    post_event(event);
#else
    JavaThread* jthread = (JavaThread*)thread;
    JavaThreadState saved_state = jthread->thread_state();

    if (saved_state == _thread_in_vm) {
      // same as conditions for post_event() so use it
      post_event(event);
      return;
    }

    // We are about to transition to _thread_in_native. See if there
    // is an external suspend requested before we go. If there is,
    // then we do a self-suspend. We don't need to do this for
    // post_event() because it uses ThreadToNativeFromVM.

    if (jthread->is_external_suspend_with_lock()) {
      jthread->java_suspend_self();
    }

    event->env_id = jthread->jni_environment();
    // prepare to call out across JVMPI
    jthread->frame_anchor()->make_walkable(jthread);
    if (saved_state == _thread_in_Java) {
      ThreadStateTransition::transition_from_java(jthread, _thread_in_native);
    } else if (saved_state != _thread_in_native) {
      // Nested events are already in _thread_in_native and don't need
      // to transition again.
      ThreadStateTransition::transition(jthread, saved_state, _thread_in_native);
    }
    HandleMark  hm(thread);  
    // notify profiler agent
    post_event_common(event);
    // restore state prior to posting event
    ThreadStateTransition::transition_from_native(jthread, saved_state); 
#endif /* PROPER_TRANSITIONS */
  } else {
    if (thread->is_VM_thread()) {
      // calling from VM thread

      if (calling_thread == NULL) {
	  calling_thread = JavaThread::active();
      }	  
	    
      assert(calling_thread != NULL && calling_thread->is_Java_thread(),
	     "wrong thread, expecting Java thread");

      event->env_id = (calling_thread != NULL && 
		       calling_thread->is_Java_thread()) ?
		       calling_thread->jni_environment() : NULL;
    } else {
      event->env_id = calling_thread->jni_environment();
    }
    // notify profiler agent
    post_event_common(event);
  }
}


// ----------------------------------------------------------
// Functions called by other parts of the VM to notify events
// ----------------------------------------------------------

void issue_jvmpi_class_load_event(klassOop k) {
  jvmpi::post_class_load_event(Klass::cast(k)->java_mirror());
}


class IssueJVMPIobjAllocEvent: public ObjectClosure {
 public:
  void do_object(oop obj) {
    Universe::jvmpi_object_alloc(obj, obj->size() * wordSize);
  };
};

volatile bool VM_JVMPIPostObjAlloc::_restrict_event_posting = true;
Thread *      VM_JVMPIPostObjAlloc::_create_thread = NULL;

void VM_JVMPIPostObjAlloc::clear_restriction() {
  // See MutexLockerEx comment in wait_if_restricted().
  MutexLockerEx loap(ObjAllocPost_lock, Mutex::_no_safepoint_check_flag);

  // Lower restriction since we are done with the VM operation
  _restrict_event_posting = false;

  // let any waiting threads resume
  ObjAllocPost_lock->notify_all();
}

void VM_JVMPIPostObjAlloc::doit() {
  // Issue object allocation events for all allocated objects
  IssueJVMPIobjAllocEvent blk;

  // make sure the heap's parseable before iterating over it
  Universe::heap()->ensure_parseability();
  Universe::heap()->object_iterate(&blk);
}

// The restriction is true by default to allow wait_if_restricted()
// to query the value without holding the lock. This imposes the
// least overhead on later calls to post_event_common(). Since any
// event handler can request an OBJ_ALLOC event, we have to restrict
// all other events until the VMThread is done with its housekeeping.
//
// There are five cases to consider:
// 1) The VMThread calls wait_if_restricted() as part of its
//    OBJ_ALLOC posting. It will not grab the lock and will not
//    block due to the second if-statement.
// 2) The JavaThread that will eventually make this VM operation
//    request calls wait_if_restricted() before the VM op is created.
//    It will not grab the lock and will not block due to the second
//    if-statement.
//
// The remaining cases apply to JavaThreads that are not making this
// VM operation request.
//
// 3) A JavaThread that calls wait_if_restricted() before the VM
//    op is created will always grab the lock and then enter the
//    check-and-wait() loop.
// 4) A JavaThread that calls wait_if_restricted() after the VM
//    op is created but before it is finished will always grab the
//    lock and then enter the check-and-wait() loop.
// 5) A JavaThread that calls wait_if_restricted() after the VM
//    op is finished will see the false value and will not block.
//
// If the restriction is false by default and then set to true in
// the VM op constructor, then we have to guard the query with a
// lock grab to prevent a race between the JavaThread and the
// VMThread. Without the lock grab, it would be possible for the
// JavaThread to see the "false" value just before the constructor
// sets that the value to true. At that point, the JavaThread
// would be racing to finish its event posting before the VMThread
// blocks it in a safepoint.
//
void VM_JVMPIPostObjAlloc::wait_if_restricted() {
  if (_restrict_event_posting) {
    // a restriction has been raised

    // The restriction does not apply to the VMThread nor does it
    // apply to the thread that makes the VM_JVMPIPostObjAlloc
    // VM operation request.
    Thread *thr = ThreadLocalStorage::thread();
    if (thr != NULL && !thr->is_VM_thread() && thr != create_thread()) {
      // The restriction applies to this thread. We use
      // MutexLockerEx to allow the lock to work just
      // before calling into the agent's code (native).
      MutexLockerEx loap(ObjAllocPost_lock, Mutex::_no_safepoint_check_flag);
      while (_restrict_event_posting) {
        ObjAllocPost_lock->wait(Mutex::_no_safepoint_check_flag, 0);
      }
    }
  }
}

void jvmpi::post_vm_initialization_events() {
  if (Universe::jvmpi_alloc_event_enabled()) {
    // Issue the object allocation events thru a VM operation since
    // it needs to be done at a safepoint
    VM_JVMPIPostObjAlloc op;
    VMThread::execute(&op);
  } else {
    // lift the restriction since we didn't do the VM operation
    VM_JVMPIPostObjAlloc::clear_restriction();
  }

  if (!jvmpi::enabled()) {
    // no agent is attached and the event posting restriction is now
    // lifted so there is nothing more to do
    return;
  }

  assert(!JVMPICheckGCCompatibility ||
         !(UseTrainGC || UseConcMarkSweepGC || UseParNewGC || UseParallelGC),
         "JVMPI-incompactible collector; jvm should have exited during "
         " JVMPI initialization");

  if (jvmpi::is_event_enabled(JVMPI_EVENT_CLASS_LOAD)) {
    // Issue class load events for all loaded classes
    // Note: This must happen _after_ the allocation events, otherwise hprof has problems!
    SystemDictionary::classes_do(&issue_jvmpi_class_load_event);
  }

  if (jvmpi::is_event_enabled(JVMPI_EVENT_THREAD_START)) {
    // Issue thread creation events for all started threads
    int k = 0;
    int threadcount;
    JavaThread** ThreadSnapShot;
    { MutexLocker mu(Threads_lock);
      threadcount = Threads::number_of_threads();
      ThreadSnapShot = NEW_C_HEAP_ARRAY(JavaThread*, threadcount);
      for (JavaThread* tp = Threads::first() ; (tp != NULL) && ( k < threadcount); tp = tp->next(), k++) {
        ThreadSnapShot[k] = tp;
      }
    } // Release Threads_lock before calling up to agent code
    for (k = 0; k<threadcount; k++) {
      jvmpi::post_thread_start_event(ThreadSnapShot[k]);
    }
    FREE_C_HEAP_ARRAY(JavaThread*, ThreadSnapShot);
  }
}


void jvmpi::post_vm_initialized_event() {
  JVMPI_Event event;
  event.event_type = JVMPI_EVENT_JVM_INIT_DONE;
  post_event(&event);
}

void jvmpi::post_vm_death_event() {
  JVMPI_Event event;
  event.event_type = JVMPI_EVENT_JVM_SHUT_DOWN;
  post_event_vm_mode(&event, NULL);
}


void jvmpi::post_instruction_start_event(const frame& f) {
  ResourceMark rm;
  JVMPI_Event event;

  methodOop method = f.interpreter_frame_method();
  address   bcp    = f.interpreter_frame_bcp();

  // fill in generic information
  event.event_type = JVMPI_EVENT_INSTRUCTION_START;
  event.u.instruction.method_id = method->jmethod_id();
  event.u.instruction.offset    = method->bci_from(bcp);

  // debugging
#ifdef ASSERT
  switch (Bytecodes::java_code(Bytecodes::cast(*bcp))) {
    case Bytecodes::_tableswitch : // fall through
    case Bytecodes::_lookupswitch: // fall through
    case Bytecodes::_ifnull      : // fall through
    case Bytecodes::_ifeq        : // fall through
    case Bytecodes::_ifnonnull   : // fall through
    case Bytecodes::_ifne        : // fall through
    case Bytecodes::_iflt        : // fall through
    case Bytecodes::_ifge        : // fall through
    case Bytecodes::_ifgt        : // fall through
    case Bytecodes::_ifle        : assert(f.interpreter_frame_expression_stack_size() >= 1, "stack size must be >= 1"); break;
    case Bytecodes::_if_acmpeq   : // fall through
    case Bytecodes::_if_icmpeq   : // fall through
    case Bytecodes::_if_acmpne   : // fall through
    case Bytecodes::_if_icmpne   : // fall through
    case Bytecodes::_if_icmplt   : // fall through
    case Bytecodes::_if_icmpge   : // fall through
    case Bytecodes::_if_icmpgt   : // fall through
    case Bytecodes::_if_icmple   : assert(f.interpreter_frame_expression_stack_size() >= 2, "stack size must be >= 2"); break;
  }
#endif

  // fill in bytecode-specific information
  //
  // Note: This code is necessary to satisfy the current interface for the jcov
  //       code coverage tool. The interface should be simplified and generalized
  //       to provide expression stack access instead of specific information for
  //       a few bytecodes only. Given expression stack access, the code below
  //       can move into jcov, the interface becomes simpler, more general, and
  //       also more powerful. With the next version/revision of JVMPI this clean
  //       up should be seriously considered (gri 11/18/99).

  int  size  = f.interpreter_frame_expression_stack_size();
  jint tos_0 = size > 0 ? f.interpreter_frame_expression_stack_at(size - 1) : 0;
  jint tos_1 = size > 1 ? f.interpreter_frame_expression_stack_at(size - 2) : 0;

  switch (Bytecodes::java_code(Bytecodes::cast(*bcp))) {
    case Bytecodes::_tableswitch :
      { const Bytecode_tableswitch* s = Bytecode_tableswitch_at(bcp);
        event.u.instruction.u.tableswitch_info.key = tos_0;
        event.u.instruction.u.tableswitch_info.low = s->low_key();
        event.u.instruction.u.tableswitch_info.hi  = s->high_key();
      }
      break;
    case Bytecodes::_lookupswitch:
      { Bytecode_lookupswitch* s = Bytecode_lookupswitch_at(bcp);
        int i;
        for (i = 0; i < s->number_of_pairs() && tos_0 != s->pair_at(i)->match(); i++);
        event.u.instruction.u.lookupswitch_info.chosen_pair_index = i;
        event.u.instruction.u.lookupswitch_info.pairs_total       = s->number_of_pairs();
      }
      break;
    case Bytecodes::_ifnull      : // fall through
    case Bytecodes::_ifeq        : event.u.instruction.u.if_info.is_true = tos_0 == 0; break;
    case Bytecodes::_ifnonnull   : // fall through
    case Bytecodes::_ifne        : event.u.instruction.u.if_info.is_true = tos_0 != 0; break;
    case Bytecodes::_iflt        : event.u.instruction.u.if_info.is_true = tos_0 <  0; break;
    case Bytecodes::_ifge        : event.u.instruction.u.if_info.is_true = tos_0 >= 0; break;
    case Bytecodes::_ifgt        : event.u.instruction.u.if_info.is_true = tos_0 >  0; break;
    case Bytecodes::_ifle        : event.u.instruction.u.if_info.is_true = tos_0 <= 0; break;
    case Bytecodes::_if_acmpeq   : // fall through
    case Bytecodes::_if_icmpeq   : event.u.instruction.u.if_info.is_true = tos_1 == tos_0; break;
    case Bytecodes::_if_acmpne   : // fall through
    case Bytecodes::_if_icmpne   : event.u.instruction.u.if_info.is_true = tos_1 != tos_0; break;
    case Bytecodes::_if_icmplt   : event.u.instruction.u.if_info.is_true = tos_1 <  tos_0; break;
    case Bytecodes::_if_icmpge   : event.u.instruction.u.if_info.is_true = tos_1 >= tos_0; break;
    case Bytecodes::_if_icmpgt   : event.u.instruction.u.if_info.is_true = tos_1 >  tos_0; break;
    case Bytecodes::_if_icmple   : event.u.instruction.u.if_info.is_true = tos_1 <= tos_0; break;
  }
  
  post_event(&event);
}

void jvmpi::post_thread_start_event(JavaThread* thread, jint flag)
{
  ResourceMark rm;
  JVMPI_Event event;
  
  assert(!Threads_lock->owned_by_self(), "must not own threads_lock for notify");

  { MutexLocker mu(Threads_lock);

    // Do not post thread start event for hidden java thread.
    if (thread->is_hidden_from_external_view()) return;

    event.event_type = JVMPI_EVENT_THREAD_START | flag;
  
    event.u.thread_start.thread_name   = (char*)thread->get_thread_name();
    event.u.thread_start.group_name    = (char*)thread->get_threadgroup_name();
    event.u.thread_start.parent_name   = (char*)thread->get_parent_name();
    event.u.thread_start.thread_id     = (jobjectID)thread->threadObj();
    event.u.thread_start.thread_env_id = thread->jni_environment();
  } // Release Threads_lock
  if (TraceJVMPI) {
    tty->cr();
    tty->print_cr("JVMPI: post_thread_start_event for thread id " INTPTR_FORMAT " [thread " INTPTR_FORMAT " <%s>] ",
		  event.u.thread_start.thread_id, thread, event.u.thread_start.thread_name);
  }
  
  GC_locker::lock();
  post_event_vm_mode(&event, NULL);
  GC_locker::unlock();
}

void jvmpi::post_thread_start_event(JavaThread* thread) {
  post_thread_start_event(thread, 0);
}

void jvmpi::post_thread_end_event(JavaThread* thread) {
  ResourceMark rm;
  JVMPI_Event event;
  event.event_type = JVMPI_EVENT_THREAD_END;

  { MutexLocker mu(Threads_lock);

    // Do not post thread end event for hidden java thread.
    if (thread->is_hidden_from_external_view()) return;

    event.u.thread_start.thread_name   = (char* )thread->get_thread_name();
    event.u.thread_start.group_name    = (char*)thread->get_threadgroup_name();
    event.u.thread_start.parent_name   = (char* )thread->get_parent_name();
    event.u.thread_start.thread_id     = (jobjectID)thread->threadObj();
    event.u.thread_start.thread_env_id = thread->jni_environment();
  } // Release Threads_lock

  if (TraceJVMPI) {
    tty->cr();
    tty->print_cr("JVMPI: post_thread_end_event for thread id " INTPTR_FORMAT " [thread " INTPTR_FORMAT " <%s>] ", 
		  event.u.thread_start.thread_id, thread, event.u.thread_start.thread_name);
  }
  post_event(&event);
}

void jvmpi::fillin_array_class_load_event(oop kOop, JVMPI_Event *eventp) {
  Klass *k = Klass::cast(java_lang_Class::as_klassOop(kOop));
  assert(k->oop_is_array(), "must be array classes");

  eventp->event_type                       = JVMPI_EVENT_CLASS_LOAD;
  eventp->u.class_load.class_name          = k->external_name();
  eventp->u.class_load.source_name         = NULL;
  eventp->u.class_load.num_interfaces      = 0; 
  eventp->u.class_load.num_methods         = 0;
  eventp->u.class_load.methods             = NULL;
  eventp->u.class_load.num_static_fields   = 0;
  eventp->u.class_load.statics             = NULL;
  eventp->u.class_load.num_instance_fields = 0;
  eventp->u.class_load.instances           = NULL;
  eventp->u.class_load.class_id            = (jobjectID)kOop;
}

// Note: kOop must be mirror
void jvmpi::fillin_class_load_event(oop kOop, JVMPI_Event *eventp, bool fillin_jni_ids) {
  eventp->event_type = JVMPI_EVENT_CLASS_LOAD;
  instanceKlassHandle k = java_lang_Class::as_klassOop(kOop);
  assert(!k()->klass_part()->oop_is_array(), "must not be array classes");

  instanceKlass* ik = instanceKlass::cast(k());
  // get field info
  int num_statics = 0;
  int num_instances = 0;
  for (FieldStream count_field_st(k, true, true); !count_field_st.eos(); count_field_st.next()) {
    if (count_field_st.access_flags().is_static()) {
      num_statics++;
    } else {
      num_instances++;
    }
  }
  JVMPI_Field* statics = NEW_RESOURCE_ARRAY(JVMPI_Field, num_statics);
  JVMPI_Field* instances = NEW_RESOURCE_ARRAY(JVMPI_Field, num_instances);
  int i_stat = 0;
  int i_inst = 0;
  for (FieldStream field_st(k, true, true); !field_st.eos(); field_st.next()) {
    char* f_name = field_st.name     ()->as_C_string();
    char* f_sig  = field_st.signature()->as_C_string();
    if (field_st.access_flags().is_static()) {
      statics[i_stat].field_name      = f_name;
      statics[i_stat].field_signature = f_sig;
      i_stat++;
    } else {
      instances[i_inst].field_name      = f_name;
      instances[i_inst].field_signature = f_sig;
      i_inst++;
    }
  }
  assert(i_inst == num_instances, "sanity check");
  assert(i_stat == num_statics, "sanity check");
  // get method info
  int num_methods = ik->methods()->length();
  JVMPI_Method* methods = NEW_RESOURCE_ARRAY(JVMPI_Method, num_methods);
  int i_meth = 0;
  for (MethodStream meth_st(k, true, true); !meth_st.eos(); meth_st.next()) {
    methodOop m = meth_st.method();
    methods[i_meth].method_name      = m->name()->as_C_string();
    methods[i_meth].method_signature = m->signature()->as_C_string();
    if (fillin_jni_ids) {
      methods[i_meth].method_id      = m->jmethod_id();
    } else {
      // method_id doesn't mean much after class is unloaded
      methods[i_meth].method_id      = NULL;
    }
    methods[i_meth].start_lineno     = m->line_number_from_bci(0);
    if (m->code_size() > 0) {
      methods[i_meth].end_lineno     = m->line_number_from_bci(m->code_size() - 1);
    } else {
      methods[i_meth].end_lineno     = m->line_number_from_bci(0);
    }
    i_meth++;
  }

  eventp->u.class_load.class_name          = ik->external_name();
  if (ik->source_file_name() == NULL)
    eventp->u.class_load.source_name       = NULL;
  else
    eventp->u.class_load.source_name       = ik->source_file_name()->as_C_string();
  eventp->u.class_load.num_interfaces      = ik->local_interfaces()->length();
  eventp->u.class_load.num_methods         = num_methods;
  eventp->u.class_load.methods             = methods;
  eventp->u.class_load.num_static_fields   = num_statics;
  eventp->u.class_load.statics             = statics;
  eventp->u.class_load.num_instance_fields = num_instances;
  eventp->u.class_load.instances           = instances;
  eventp->u.class_load.class_id            = (jobjectID)ik->java_mirror();
}


// List of classes unloaded for the duration of the CLASS_UNLOAD event
// handler. Populated by save_class_unload_event_info(), queried by both
// post_class_load_event() and post_class_unload_events(), and cleaned
// up by post_class_unload_events().
static GrowableArray<JVMPI_Event*>* unloaded_classes = NULL;

// Note: kOop must be mirror
void jvmpi::post_class_load_event(oop kOop, jint flag) {

  if (flag == JVMPI_REQUESTED_EVENT && unloaded_classes != NULL) {
    // This is a synthesized event and we are in the middle of unloading
    // classes so see if the requested class is one that we unloaded.

    // walk the list of unloaded class event information
    for (int i = 0; i < unloaded_classes->length(); i++) {
      JVMPI_Event *ev = unloaded_classes->at(i);
      if ((oop)(ev->u.class_load.class_id) == kOop) {
        // We are in the event handler for CLASS_UNLOAD event so
        // we don't have to lock out GC. Post the saved event
        // information for the unloaded class to the agent.
        assert(GC_locker::is_active(), "GC must be locked when in event handler");
        post_event_vm_mode(ev, NULL);
        return;
      }
    }
  }

  ResourceMark rm;
  JVMPI_Event event;
  klassOop k = java_lang_Class::as_klassOop(kOop);

  if (k->klass_part()->oop_is_array()) {
    fillin_array_class_load_event(kOop, &event);
  } else {
    fillin_class_load_event(kOop, &event, true /* fillin JNI ids */);
  }
  event.event_type |= flag;
  if (TraceJVMPI) {
    tty->print("JVMPI: post_class_load_event for klass mirror " INTPTR_FORMAT " ", kOop);
    java_lang_Class::as_klassOop(kOop)->print_value();
    tty->print(" ");
    kOop->print_value();
    tty->cr();
  }

  GC_locker::lock();
  post_event_vm_mode(&event, NULL);
  GC_locker::unlock();
}


void jvmpi::post_class_load_event(oop k) {
  post_class_load_event(k, 0);
}


// Wrapper to translate the (32-bit) JVM/PI memory allocation function
// to the HotSpot resource allocation function.
void *jvmpi::jvmpi_alloc(unsigned int bytecnt) {
  return (void *)resource_allocate_bytes((size_t)bytecnt);
}


void jvmpi::post_class_load_hook_event(unsigned char **ptrP,
  unsigned char **end_ptrP, jvmpi_alloc_func_t malloc_f) {
  JVMPI_Event event;

  /* fill event info and notify the profiler */
  event.event_type = JVMPI_EVENT_CLASS_LOAD_HOOK;

  event.u.class_load_hook.class_data = *ptrP;
  event.u.class_load_hook.class_data_len = *end_ptrP - *ptrP;
  event.u.class_load_hook.malloc_f = malloc_f;

  post_event(&event);
    
  *ptrP = event.u.class_load_hook.new_class_data;
  *end_ptrP = *ptrP + event.u.class_load_hook.new_class_data_len;
}


// Post CLASS_UNLOAD events and/or release saved memory.
void jvmpi::post_class_unload_events() {
  if (unloaded_classes != NULL) {  // we unloaded some classes
    // walk the list of unloaded class event information
    for (int i = 0; i < unloaded_classes->length(); i++) {
      JVMPI_Event *ev = unloaded_classes->at(i);  // shorthand for saved info

      if (jvmpi::is_event_enabled(JVMPI_EVENT_CLASS_UNLOAD)) {
        // The caller is still interested in the events so post them.
        // Note: by the time we get called, the caller may no longer
        // be interested in the events, but we have to always free
        // the memory below.
        JVMPI_Event event;

        GC_locker::lock();

        // construct a CLASS_UNLOAD event from the saved into
        event.event_type = JVMPI_EVENT_CLASS_UNLOAD;
        event.u.class_unload.class_id = ev->u.class_load.class_id;

        post_event_vm_mode(&event, NULL);

        GC_locker::unlock();
      }
      delete ev;  // done with the saved info
    }

    delete unloaded_classes;
    unloaded_classes = NULL;
  }
}


// GC has caused a class to be unloaded so save CLASS_LOAD information
// just in case there is a RequestEvent(CLASS_LOAD) call from the
// CLASS_UNLOAD event handler.
void jvmpi::save_class_unload_event_info(oop k) {
  JVMPI_Event *ev = new JVMPI_Event();
  fillin_class_load_event(k, ev, false /* don't fillin JNI id values */);
  ev->event_type |= JVMPI_REQUESTED_EVENT;

  if (unloaded_classes == NULL) {
    // first unloaded class so setup initial space for the events
    unloaded_classes = new GrowableArray<JVMPI_Event*>(5);
  }
  unloaded_classes->append(ev);
}


void jvmpi::post_dump_event() {
  if (is_event_enabled(JVMPI_EVENT_DUMP_DATA_REQUEST)) {
    JVMPI_Event event;
    event.event_type = JVMPI_EVENT_DUMP_DATA_REQUEST;
    post_event(&event);
  }
  if (is_event_enabled(JVMPI_EVENT_RESET_DATA_REQUEST)) {
    JVMPI_Event event;
    event.event_type = JVMPI_EVENT_RESET_DATA_REQUEST;
    post_event(&event);
  }
}

// Maintain an array of skipped JNI global refs and those JNI global refs
// are not dumped as GC roots in the heap dumps since they are internal to VM.
static GrowableArray<jobject>* skipped_globalrefs = NULL;

void jvmpi::post_new_globalref_event(jobject ref, oop obj, bool post_jvmpi_event) {
  if (post_jvmpi_event) {
    // post new JNI global ref alloc event
    JVMPI_Event event;
    
    GC_locker::lock();
    
    /* fill event info and notify the profiler */
    event.event_type = JVMPI_EVENT_JNI_GLOBALREF_ALLOC;

    event.u.jni_globalref_alloc.obj_id = (jobjectID)(obj);
    event.u.jni_globalref_alloc.ref_id = ref;

    post_event_vm_mode(&event, NULL);

    GC_locker::unlock();

  } else {
    // Not to post new JNI global ref alloc event;
    // need to save those skipped JNI global ref, which should not be 
    // dumped as GC roots in the heap dump

    MutexLocker ml(JNIGlobalHandle_lock);
    if (skipped_globalrefs == NULL) {
      skipped_globalrefs = new (ResourceObj::C_HEAP) GrowableArray<jobject>(256, true);
    }
    skipped_globalrefs->append(ref);
  }
}


void jvmpi::post_delete_globalref_event(jobject ref, bool post_jvmpi_event) {
  if (post_jvmpi_event) {
    // post JNI global ref free event
    JVMPI_Event event;

    GC_locker::lock();

    /* fill event info and notify the profiler */
    event.event_type = JVMPI_EVENT_JNI_GLOBALREF_FREE;

    event.u.jni_globalref_free.ref_id = ref;

    post_event_vm_mode(&event, NULL);

    GC_locker::unlock();
  } else {
    // remove the JNI global ref from skipped_globalrefs list
    MutexLocker ml(JNIGlobalHandle_lock);

    int length = (skipped_globalrefs != NULL ? skipped_globalrefs->length() : 0);
    int i = 0;

    // we choose not to compact the array when a globalref is destroyed
    // since the number of such calls might not be that many.
    for (i = 0; i < length; i++) {
      if (skipped_globalrefs->at(i) == ref) {
        skipped_globalrefs->at_put(i, NULL);
        break;
      }
    }
    assert(length == 0 || i < length, "JNI global ref");
  }
}

void jvmpi::post_new_weakref_event(jobject ref, oop obj) {
  JVMPI_Event event;
    
  GC_locker::lock();

  /* fill event info and notify the profiler */
  event.event_type = JVMPI_EVENT_JNI_WEAK_GLOBALREF_ALLOC;

  event.u.jni_globalref_alloc.obj_id = (jobjectID)(obj);
  event.u.jni_globalref_alloc.ref_id = ref;

  post_event_vm_mode(&event, NULL);

  GC_locker::unlock();
}


void jvmpi::post_delete_weakref_event(jobject ref) {
  JVMPI_Event event;
    
  GC_locker::lock();

  /* fill event info and notify the profiler */
  event.event_type = JVMPI_EVENT_JNI_WEAK_GLOBALREF_FREE;

  event.u.jni_globalref_free.ref_id = ref;

  post_event_vm_mode(&event, NULL);

  GC_locker::unlock();
}


void jvmpi::post_arena_new_event(int arena_id, const char* arena_name) {
  if (!is_event_enabled(JVMPI_EVENT_ARENA_NEW)) return;
  JVMPI_Event event;

  event.event_type = JVMPI_EVENT_ARENA_NEW;
  event.u.new_arena.arena_id = arena_id;
  event.u.new_arena.arena_name = arena_name;
  post_event_vm_mode(&event, NULL);
}


void jvmpi::post_arena_delete_event(int arena_id) {
  JVMPI_Event event;

  event.event_type = JVMPI_EVENT_DELETE_ARENA;
  event.u.delete_arena.arena_id = arena_id;
  post_event_vm_mode(&event, NULL);
}

/* post_object_alloc_event requires size to be in bytes */
void jvmpi::post_object_alloc_event(oop obj, size_t bytesize, jint arena_id, jint flag) {
  // do not emit the event if the allocation event is not enabled, except if it is
  // requested
  if (!is_event_enabled(JVMPI_EVENT_OBJECT_ALLOC) && flag != JVMPI_REQUESTED_EVENT) return;
  // bailout if obj is undefined
  if (obj == NULL) return;
  // bootstraping problem: Issue object allocation event for the java/lang/Class 
  // mirror with class set to NULL (to avoid infinite recursion).
  bool bootstrap = (obj == Klass::cast(SystemDictionary::class_klass())->java_mirror());
  // determine klass & is_array info
  oop klass;
  int is_array;
  if (bootstrap) {
    klass    = NULL;
    is_array = JVMPI_NORMAL_OBJECT;
  } else if (obj->is_instance()) {
    klass    = Klass::cast(obj->klass())->java_mirror();
    is_array = JVMPI_NORMAL_OBJECT;
  } else if (obj->is_objArray()) {
    klass    = Klass::cast(objArrayKlass::cast(obj->klass())->element_klass())->java_mirror();
    is_array = JVMPI_CLASS;
  } else if (obj->is_typeArray()) {
    klass    = NULL;
    is_array = typeArrayKlass::cast(obj->klass())->type();
  } else {
    klass    = JVMPI_INVALID_CLASS;
    is_array = JVMPI_NORMAL_OBJECT;
  }    
  // post event if ok
  if (klass != JVMPI_INVALID_CLASS) {
    if (!flag) GC_locker::lock();
    /* fill event info and notify the profiler */
    { JVMPI_Event event;
      event.event_type           = JVMPI_EVENT_OBJECT_ALLOC | flag;
      event.u.obj_alloc.arena_id = arena_id;
      event.u.obj_alloc.class_id = (jobjectID)klass;
      event.u.obj_alloc.is_array = is_array;
      event.u.obj_alloc.size     = (int) bytesize; // spec will require 64 bit modifications
      event.u.obj_alloc.obj_id   = (jobjectID)obj;
      if (TraceJVMPI) {
	tty->print_cr("JVMPI: post_object_alloc_event for object " INTPTR_FORMAT " ", obj);
      }
      post_event_vm_mode(&event, NULL);
    }
    if (!flag) GC_locker::unlock();
  }
}


void jvmpi::post_object_free_event(oop obj) {
  JVMPI_Event event;

  // $$$ There used to be an assertion that this was only happening during
  // m/s collections.  Didn't know how to generalize, so I took it out.
  // (DLD, 6/20).

  /* fill event info and notify the profiler */
  event.event_type = JVMPI_EVENT_OBJECT_FREE;

  event.u.obj_free.obj_id = (jobjectID)obj;
    
  post_event_vm_mode(&event, NULL);
}


void jvmpi::post_object_move_event(oop oldobj, int old_arena_id, oop newobj, int new_arena_id) {
  JVMPI_Event event;
    
  assert(Universe::heap()->is_gc_active(), "Should only move objects during GC");

  /* fill event info and notify the profiler */
  event.event_type = JVMPI_EVENT_OBJECT_MOVE;

  event.u.obj_move.obj_id       = (jobjectID)oldobj;
  event.u.obj_move.arena_id     = old_arena_id;
  event.u.obj_move.new_obj_id   = (jobjectID)newobj;
  event.u.obj_move.new_arena_id = new_arena_id;

  post_event_vm_mode(&event, NULL);
}


static jint level = 0;

void jvmpi::post_method_entry2_event(methodOop m, oop receiver) {
  JVMPI_Event event;
  event.event_type = JVMPI_EVENT_METHOD_ENTRY2;
  event.u.method_entry2.method_id = m->jmethod_id();
  event.u.method_entry2.obj_id = (jobjectID)receiver;
  if (TraceJVMPI) {
#if 0
    ResourceMark rm;
    tty->print_cr("%04d %s: method_entry2 %s",
		  level++,
		  ((JavaThread*)get_thread())->get_thread_name(),
		  m->name_and_sig_as_C_string());
#endif
  }
  post_event_vm_mode(&event, NULL);
}


void jvmpi::post_method_entry_event(methodOop m) {
  JVMPI_Event event;
  event.event_type = JVMPI_EVENT_METHOD_ENTRY;
  event.u.method.method_id = m->jmethod_id();
  if (TraceJVMPI) {
#if 0
    ResourceMark rm;
    tty->print_cr("%04d %s: method_entry %s",
		  level++,
		  ((JavaThread*)get_thread())->get_thread_name(),
		  m->name_and_sig_as_C_string());
#endif
  }
  post_event_vm_mode(&event, NULL);
}


void jvmpi::post_method_exit_event(methodOop m) {
  JVMPI_Event event;
  event.event_type = JVMPI_EVENT_METHOD_EXIT;
  event.u.method.method_id = m->jmethod_id();
  if (TraceJVMPI) {
#if 0
    ResourceMark rm;
    tty->print_cr("%04d %s: method_exit  %s",
		  --level,
		  ((JavaThread*)get_thread())->get_thread_name(),
		  m->name_and_sig_as_C_string());
#endif
  }
  post_event_vm_mode(&event, NULL);
}


// use  compiled_method_t so that the line number table can be constructed only
// temporarily and then released after post_compiled_method_load_event terminates
void jvmpi::post_compiled_method_load_event(compiled_method_t *compiled_method_info) {
  JVMPI_Event event;

  event.event_type = JVMPI_EVENT_COMPILED_METHOD_LOAD;
  event.u.compiled_method_load.method_id         = compiled_method_info->method->jmethod_id();
  event.u.compiled_method_load.code_addr         = compiled_method_info->code_addr;
  event.u.compiled_method_load.code_size         = compiled_method_info->code_size;
  event.u.compiled_method_load.lineno_table_size = compiled_method_info->lineno_table_len;
  event.u.compiled_method_load.lineno_table      = compiled_method_info->lineno_table;

  post_event_vm_mode(&event, NULL);
}

void jvmpi::post_compiled_method_unload_event(methodOop method) {
  JVMPI_Event event;

  event.event_type = JVMPI_EVENT_COMPILED_METHOD_UNLOAD;
  event.u.compiled_method_unload.method_id = method->jmethod_id();
  post_event_vm_mode(&event, NULL);
}

void jvmpi::post_monitor_contended_enter_event(void* object) {
  GC_locker::lock();

  JVMPI_Event event;
  event.event_type = JVMPI_EVENT_MONITOR_CONTENDED_ENTER;
  event.u.monitor.object = (jobjectID)object;
  post_event_vm_mode(&event, NULL);

  GC_locker::unlock();
}


void jvmpi::post_monitor_contended_entered_event(void* object) {
  GC_locker::lock();

  JVMPI_Event event;
  event.event_type = JVMPI_EVENT_MONITOR_CONTENDED_ENTERED;
  event.u.monitor.object = (jobjectID)object;
  post_event_vm_mode(&event, NULL);

  GC_locker::unlock();
}


void jvmpi::post_monitor_contended_exit_event(void* object) {
  GC_locker::lock();

  JVMPI_Event event;
  event.event_type = JVMPI_EVENT_MONITOR_CONTENDED_EXIT;
  event.u.monitor.object = (jobjectID)object;
  post_event_vm_mode(&event, NULL);

  GC_locker::unlock();
}


void jvmpi::post_monitor_wait_event(oop obj, jlong millis) {
  GC_locker::lock();

  JVMPI_Event event;
  event.event_type = JVMPI_EVENT_MONITOR_WAIT;
  event.u.monitor_wait.object  = (jobjectID)obj;
  event.u.monitor_wait.timeout = millis;
  post_event_vm_mode(&event, NULL);

  GC_locker::unlock();
}


void jvmpi::post_monitor_waited_event(oop obj, jlong millis) {
  GC_locker::lock();

  JVMPI_Event event;
  event.event_type = JVMPI_EVENT_MONITOR_WAITED;
  event.u.monitor_wait.object  = (jobjectID)obj;
  event.u.monitor_wait.timeout = millis;
  post_event_vm_mode(&event, NULL);

  GC_locker::unlock();
}


void jvmpi::post_raw_monitor_contended_enter_event(RawMonitor* rmon) {
  Thread* tp = Thread::current();
  if (tp->is_VM_thread()) return;
  JVMPI_Event event;

  event.event_type = JVMPI_EVENT_RAW_MONITOR_CONTENDED_ENTER;
  event.u.raw_monitor.name = rmon->name();
  event.u.raw_monitor.id = (JVMPI_RawMonitor)rmon;
  post_event_vm_mode(&event, NULL);
}


void jvmpi::post_raw_monitor_contended_entered_event(RawMonitor* rmon) {
  if (Thread::current()->is_VM_thread()) return;
  JVMPI_Event event;

  event.event_type = JVMPI_EVENT_RAW_MONITOR_CONTENDED_ENTERED;
  event.u.raw_monitor.name = rmon->name();
  event.u.raw_monitor.id = (JVMPI_RawMonitor)rmon;
  post_event_vm_mode(&event, NULL);
}


void jvmpi::post_raw_monitor_contended_exit_event(RawMonitor* rmon) {
  if (Thread::current()->is_VM_thread()) return;
  JVMPI_Event event;

  event.event_type = JVMPI_EVENT_RAW_MONITOR_CONTENDED_EXIT;
  event.u.raw_monitor.name = rmon->name();
  event.u.raw_monitor.id = (JVMPI_RawMonitor)rmon;
  post_event_vm_mode(&event, NULL);
}


void jvmpi::post_gc_start_event() {
  JVMPI_Event event;
  assert(Thread::current()->is_VM_thread(), "wrong thread");

  Thread* calling_thread = JavaThread::active();
  /* fill event info and notify the profiler */
  event.event_type = JVMPI_EVENT_GC_START;

  assert(calling_thread->is_Java_thread(), "wrong thread");
  post_event_vm_mode(&event, (JavaThread*)calling_thread);
}


class CountObjects: public ObjectClosure {
 private:
  int _nof_objects;
 public:
  CountObjects(): _nof_objects(0) {}

  void do_object(oop obj) { _nof_objects++;  };

  int nof_objects() const { return _nof_objects; }
};


void jvmpi::post_gc_finish_event(jlong used_obj_space, jlong total_obj_space) {
  JVMPI_Event event;
  assert(Thread::current()->is_VM_thread(), "wrong thread");
  jlong used_objs = 0;
  // compute number of used objects
  { // Note: this is slow and cumbersome
    CountObjects blk;
    // Although the call to ensure_parseability()
    // is not needed here due to this code running at the end of
    // GC, these have been added here commented out since
    // this code has moved around.
    //  Universe::heap()->ensure_parseability();
    Universe::heap()->permanent_object_iterate(&blk);

    Universe::heap()->object_iterate(&blk);
    used_objs = blk.nof_objects();
  }
  Thread* calling_thread = JavaThread::active();
  /* fill event info and notify the profiler */
  event.event_type = JVMPI_EVENT_GC_FINISH;
  event.u.gc_info.used_objects       = used_objs;
  event.u.gc_info.used_object_space  = used_obj_space;
  event.u.gc_info.total_object_space = total_obj_space;

  assert(calling_thread->is_Java_thread(), "wrong thread");
  post_event_vm_mode(&event, (JavaThread*)calling_thread);
}


void jvmpi::post_trace_instr_event(unsigned char *pc, unsigned char opcode) {
  Unimplemented();
}


void jvmpi::post_trace_if_event(unsigned char *pc, int is_true) {
  Unimplemented();
}


void jvmpi::post_trace_tableswitch_event(unsigned char *pc, int key, int low, int hi) {
  Unimplemented();
}


void jvmpi::post_trace_lookupswitch_event(unsigned char *pc, int chosen_pair_index, int pairs_total) {
  Unimplemented();
}


// heap dumps

// Dump is a helper class for all kinds of dumps that require
// a buffer to hold the dump. 

class Dump: public StackObj {
 private:
  address _begin;                      // the beginning of the dump space, NULL if no space was allocated
  address _end;                        // the current dump position
  address _limit;                      // the limit of the dump space (debugging only)

  void init(int dump_size) {
    assert(dump_size <= 0 || _begin == NULL, "dump buffer already allocated");
    _begin = dump_size > 0 ? NEW_C_HEAP_ARRAY(unsigned char, dump_size) : NULL;
    _end   = _begin;
    _limit = _begin + dump_size;
  }
    
  bool write() const                   { return begin() != NULL; }
  address limit() const                { return _limit; }
  void range_check(int size)           { assert(end() + size <= limit(), "end of dump reached"); }

 public:
  // creation
  Dump()                               { init(0); }
  void enable_write(int dump_size)     { init(dump_size); }
  ~Dump()                              { if (write()) FREE_C_HEAP_ARRAY(unsigned char, begin()); }

  // accessors
  address begin() const                { return _begin; }
  address end() const                  { return _end; }
  int size() const                     { return end() - begin(); }

  // primitive dumpers
  void dump_u1(u1 x)                   { if (write()) { range_check(1); *_end = x;                   } _end += 1; }
  void dump_u2(u2 x)                   { if (write()) { range_check(2); Bytes::put_Java_u2(_end, x); } _end += 2; }
  void dump_u4(u4 x)                   { if (write()) { range_check(4); Bytes::put_Java_u4(_end, x); } _end += 4; }
  void dump_u8(u8 x)                   { if (write()) { range_check(8); Bytes::put_Java_u8(_end, x); } _end += 8; }

  // basic type dumpers
  void dump_bool  (jboolean* x)        { dump_u1(*(u1*)x); }
  void dump_char  (jchar*    x)        { dump_u2(*(u2*)x); }
  void dump_float (jfloat*   x)        { dump_u4(*(u4*)x); }
  void dump_double(jdouble*  x)        { dump_u8(*(u8*)x); }
  void dump_byte  (jbyte*    x)        { dump_u1(*(u1*)x); }
  void dump_short (jshort*   x)        { dump_u2(*(u2*)x); }
  void dump_int   (jint*     x)        { dump_u4(*(u4*)x); }
  void dump_long  (jlong*    x)        { dump_u8(*(u8*)x); }
  void dump_obj   (oop*      x)        { dump_oop(*x); }

  // other dumpers
  //
  // Note: jobjectID (oops) and JNIEnv* are not dumped in Java byte ordering
  //       like all other data types - which is an inconsistency. It should
  //       really be handled like all other data (and mapped to u4 for the
  //       ia32 architecture).
  void dump_oop(oop obj) {
    assert(obj == NULL || (obj->is_oop() && !obj->is_klass()), "not an opp or a klass");
#ifndef _LP64
    if (write()) {
      range_check(4);
      Bytes::put_native_u4(_end, (u4)obj);
    }
    _end += 4;
#else
    if (write()) {
      range_check(8);
      Bytes::put_native_u8(_end, (u8)obj);
    }
    _end += 8;
#endif
  }

#ifndef _LP64
  void dump_thread(JNIEnv* env)        { if (write()) { range_check(4); Bytes::put_native_u4(_end, (u4)env); } _end += 4; }
  void dump_rawmonitor(JVMPI_RawMonitor mon) { if (write()) { range_check(4); Bytes::put_native_u4(_end, (u4)mon); } _end += 4; }
  void dump_char_array(const char* s)        { if (write()) { range_check(4); Bytes::put_native_u4(_end, (u4)s); } _end += 4; }
  void dump_voids(void* x)             { dump_u4((u4)x); }
#else
  void dump_thread(JNIEnv* env)        { if (write()) { range_check(8); Bytes::put_native_u8(_end, (u8)env); } _end += 8; }
  void dump_rawmonitor(JVMPI_RawMonitor mon) { if (write()) { range_check(8); Bytes::put_native_u8(_end, (u8)mon); } _end += 8; }
  void dump_char_array(const char* s)        { if (write()) { range_check(8); Bytes::put_native_u8(_end, (u8)s); } _end += 8; }
  void dump_voids(void* x)             { dump_u8((u8)x); }
#endif
  void dump_type (int type)            { dump_u1((u1)type); }

  // patching
  void patch_u2(address at, u2 x) {
    if (write()) {
      assert(begin() <= at && at + 2 <= limit(), "patching outside dump space");
      Bytes::put_Java_u2(at, x);
    }
  }

  void patch_u4(address at, u4 x) {
    if (write()) {
      assert(begin() <= at && at + 4 <= limit(), "patching outside dump space");
      Bytes::put_Java_u4(at, x);
    }
  }
};


class FieldDumper: public SignatureIterator {
 private:
  Dump*   _dump;
  address _addr;
  bool    _dump_basic_types;

 public:
  FieldDumper(Dump* dump, int level, symbolHandle signature, address addr)
  : SignatureIterator(signature)
  , _dump(dump)
  , _addr(addr)
  { 
    _dump_basic_types = (level == JVMPI_DUMP_LEVEL_2);
    dispatch_field();
  }

  void do_bool  ()                     { if (_dump_basic_types) _dump->dump_bool  ((jboolean*)_addr); }
  void do_char  ()                     { if (_dump_basic_types) _dump->dump_char  ((jchar*   )_addr); }
  void do_float ()                     { if (_dump_basic_types) _dump->dump_float ((jfloat*  )_addr); }
  void do_double()                     { if (_dump_basic_types) _dump->dump_double((jdouble* )_addr); }
  void do_byte  ()                     { if (_dump_basic_types) _dump->dump_byte  ((jbyte*   )_addr); }
  void do_short ()                     { if (_dump_basic_types) _dump->dump_short ((jshort*  )_addr); }
  void do_int   ()                     { if (_dump_basic_types) _dump->dump_int   ((jint*    )_addr); }
  void do_long  ()                     { if (_dump_basic_types) _dump->dump_long  ((jlong*   )_addr); }
  void do_void  ()                     { ShouldNotReachHere();                                        }
  void do_object(int begin, int end)   {                        _dump->dump_obj   ((oop*     )_addr); }
  void do_array (int begin, int end)   {                        _dump->dump_obj   ((oop*     )_addr); }
};


// The ObjectDumper takes care of any heap object to be dumped.
// Note that non java-level objects are filtered out (such as
// klasses, methodOops, etc.) and that mirrors are converted
// into klasses for the dump.

class ObjectDumper: public StackObj {
 private:
  Dump* _dump;
  int   _level;

  void dump_instance(instanceOop instance) {
    if (_level == JVMPI_DUMP_LEVEL_0) {
      // dump type and id only
      _dump->dump_type(JVMPI_NORMAL_OBJECT);
      _dump->dump_oop(instance);
      return;
    }
    // dump header
    _dump->dump_type(JVMPI_GC_INSTANCE_DUMP);
    _dump->dump_oop(instance);
    _dump->dump_oop(Klass::cast(instance->klass())->java_mirror());
    _dump->dump_u4((u4)0);              // reserve space for no. of bytes - patched at the end
    address field_start = _dump->end(); // remember start of field dump
    // dump instance fields
    // (note: dumping in reverse order since the klass load event dumps
    //        the instance field description in reverse order as well.)
    { for (FieldStream s(instanceKlassHandle(instance->klass()), false, true); !s.eos(); s.next()) {
        // ignore static fields as they are not in the instance
        if (!s.access_flags().is_static()) {
          FieldDumper(_dump, _level, s.signature(), (address)instance + s.offset());
        }
      }
    }
    // patch no. of bytes
    _dump->patch_u4(field_start - 4, _dump->end() - field_start);
  }

  void dump_obj_array(objArrayOop array) {
    // Note: Do not dump system object arrays as they are meaningless
    //       for hprof. Furthermore, they contain klasses which should
    //       never go out w/o extra treatment.
    if (array->klass() != Universe::systemObjArrayKlassObj()) {
      if (_level == JVMPI_DUMP_LEVEL_0) {
        // dump type and id only
        _dump->dump_type(JVMPI_CLASS);
        _dump->dump_oop(array);
        return;
      }
      oop klass = Klass::cast(objArrayKlass::cast(array->klass())->element_klass())->java_mirror();
      const int length = array->length();
      // dump header
      _dump->dump_type(JVMPI_GC_OBJ_ARRAY_DUMP);
      _dump->dump_oop(array);
      _dump->dump_u4(length);
      _dump->dump_oop(klass);
      // dump elements
      for (int i = 0; i < length; i++) _dump->dump_oop(array->obj_at(i));
      // debugging
      if (TraceJVMPI) {
        tty->print("JVMPI: dump @ " INTPTR_FORMAT " obj array [%d] (klass = " INTPTR_FORMAT ")", array, length, klass);
        if (Verbose) {
          tty->print(" {");
          for (int i = 0; i < length; i++) {
            if (i > 0) tty->print(", ");
            tty->print(INTPTR_FORMAT, array->obj_at(i));
          }
          tty->print("}");
        }
        tty->cr();
      }
    }
  }

  void dump_type_array(typeArrayOop array) {
    const int length = array->length();
    const BasicType type = typeArrayKlass::cast(array->klass())->type();
    int jvmpi_type = -1;
    switch (type) {
      case T_BOOLEAN: jvmpi_type = JVMPI_BOOLEAN; break;
      case T_CHAR   : jvmpi_type = JVMPI_CHAR   ; break;
      case T_FLOAT  : jvmpi_type = JVMPI_FLOAT  ; break;
      case T_DOUBLE : jvmpi_type = JVMPI_DOUBLE ; break;
      case T_BYTE   : jvmpi_type = JVMPI_BYTE   ; break;
      case T_SHORT  : jvmpi_type = JVMPI_SHORT  ; break;
      case T_INT    : jvmpi_type = JVMPI_INT    ; break;
      case T_LONG   : jvmpi_type = JVMPI_LONG   ; break;
      default       : ShouldNotReachHere();
    }
    if (_level == JVMPI_DUMP_LEVEL_0) {
      // dump type and id only
      _dump->dump_type(jvmpi_type);
      _dump->dump_oop(array);
      return;
    }
    // dump header
    _dump->dump_type(JVMPI_GC_PRIM_ARRAY_DUMP);
    _dump->dump_oop(array);
    _dump->dump_u4(length);
    _dump->dump_type(jvmpi_type);
    // dump elements
    if (_level == JVMPI_DUMP_LEVEL_2) {
      switch (type) {
        case T_BOOLEAN: { for (int i = 0; i < length; i++) _dump->dump_bool  (array->bool_at_addr  (i)); } break;
        case T_CHAR   : { for (int i = 0; i < length; i++) _dump->dump_char  (array->char_at_addr  (i)); } break;
        case T_FLOAT  : { for (int i = 0; i < length; i++) _dump->dump_float (array->float_at_addr (i)); } break;
        case T_DOUBLE : { for (int i = 0; i < length; i++) _dump->dump_double(array->double_at_addr(i)); } break;
        case T_BYTE   : { for (int i = 0; i < length; i++) _dump->dump_byte  (array->byte_at_addr  (i)); } break;
        case T_SHORT  : { for (int i = 0; i < length; i++) _dump->dump_short (array->short_at_addr (i)); } break;
        case T_INT    : { for (int i = 0; i < length; i++) _dump->dump_int   (array->int_at_addr   (i)); } break;
        case T_LONG   : { for (int i = 0; i < length; i++) _dump->dump_long  (array->long_at_addr  (i)); } break;
        default       : ShouldNotReachHere();
      }
    }
    // debugging
    if (TraceJVMPI) {
      tty->print_cr("JVMPI: dump @ " INTPTR_FORMAT " prim array [%d] (type = %d)", array, length, type);
    }
  }

  void dump_klass(klassOop klass) {
    if (Klass::cast(klass)->oop_is_instance()) {
      instanceKlass* k = instanceKlass::cast(klass);
      // Check for level 0 dump
      if (_level == JVMPI_DUMP_LEVEL_0) {
        // dump type and id only
        _dump->dump_type(JVMPI_NORMAL_OBJECT);    // Is this right?
        _dump->dump_oop(k->java_mirror());
        return;
      }
      // dump header
      _dump->dump_type(JVMPI_GC_CLASS_DUMP);
      _dump->dump_oop(k->java_mirror());
      _dump->dump_oop(k->super() == NULL ? NULL : Klass::cast(k->super())->java_mirror());
      _dump->dump_oop(k->class_loader());
      _dump->dump_oop(k->signers());
      _dump->dump_oop(k->protection_domain());
      _dump->dump_oop(StringTable::lookup(k->name())); // NULL if not interned string
      _dump->dump_voids(NULL); // reserved
      _dump->dump_u4(k->size_helper() * BytesPerWord);
      // dump interfaces
      { objArrayOop interfaces = k->local_interfaces();
        for (int i = 0; i < interfaces->length(); i++) {
          oop interf = Klass::cast((klassOop)interfaces->obj_at(i))->java_mirror();
          _dump->dump_oop(interf);
        }
      }
      // dump constant pool
      { address size_loc = _dump->end();    // remember constant pool size location for later patching  
        _dump->dump_u2((u2)0);              // reserve space for constant pool size - patched at the end
        int size = 0;
        const constantPoolOop pool = k->constants();
        for (int i = 1; i < pool->length(); i++) { // index i = 0 is unused!
          address end = _dump->end();
          // for now we ignore all entries
          // eventually we should probably
          // dump at least the oop entries
          /*
          switch (pool->tag_at(i).value()) {
            case JVM_CONSTANT_Class:
            case JVM_CONSTANT_Fieldref:
            ...
          }
          */
          if (end != _dump->end()) size++; // count individual entries
        }
        // patch number of entries
        _dump->patch_u2(size_loc, size);
      }
      // dump static fields
      // (note: dumping in reverse order since the klass load event dumps
      //        the static field description in reverse order as well.)
      {
         instanceKlassHandle kh(klass);
         FieldStream s(kh, true, true);
         for (; !s.eos(); s.next()) { 
           // ignore instance fields as they are not in the klass
           if (s.access_flags().is_static()) {
             FieldDumper(_dump, _level, s.signature(), (address)klass + s.offset());
           }
         }
      }
    } else if (Klass::cast(klass)->oop_is_objArray()) {
      objArrayKlass* k = objArrayKlass::cast(klass);
      // Check for level 0 dump
      if (_level == JVMPI_DUMP_LEVEL_0) {
        // dump type and id only
        _dump->dump_type(JVMPI_NORMAL_OBJECT);    // Is this right?
        _dump->dump_oop(k->java_mirror());
        return;
      }
      // still missing
    }
  }

 public:
  ObjectDumper(Dump* dump, int level, oop obj) : _dump(dump), _level(level) {
    // filter out all klasses
    if (obj->is_klass()) return;
    // convert mirrors
    if (obj->klass() == SystemDictionary::class_klass()) {
      // obj is a mirror - convert into corresponding class if possible
      if (!java_lang_Class::is_primitive(obj)) {
        // obj is not a mirror for a primitive class (basic type)
        // get the corresponding class for dumping
        obj = java_lang_Class::as_klassOop(obj);
        assert(obj != NULL, "class for non-primitive mirror must exist");
      } else {
        // obj is a mirror for a primitice class (basic type)
        // for which we don't have a (VM-level) class => dump
        // mirror as it is.
      }
    }
    // dump object
           if (obj->is_instance ()) { dump_instance  ((instanceOop )obj);
    } else if (obj->is_objArray ()) { dump_obj_array ((objArrayOop )obj);
    } else if (obj->is_typeArray()) { dump_type_array((typeArrayOop)obj);
    } else if (obj->is_klass    ()) { dump_klass     ((klassOop    )obj);
    }
  }
};


class HeapDumper: public ObjectClosure {
 private:
  Dump* _dump;
  int   _level;

 public:
  HeapDumper(Dump* dump, int level) : _dump(dump), _level(level) {
    assert(SafepointSynchronize::is_at_safepoint(), "must be at safepoint");
    // Ensure the heap's parseable before iterating over it
    Universe::heap()->ensure_parseability();
    Universe::heap()->object_iterate(this);
  }

  void do_object(oop obj)              { ObjectDumper(_dump, _level, obj); }
};


// Move this in machine specific part !
class MonitorDumper: public StackObj {
 private:
  Dump* _dump;

  void dump_for_thread(ObjectMonitor* mid, JavaThread* thread) {
    assert(SafepointSynchronize::is_at_safepoint(), "must be at safepoint");
    ResourceMark rm;
    // klassOops may be locked (e.g. due to class initialization). Make sure to skip them.
    if (((oop)mid->object())->is_klass()) return;
    //
    // Solaris implements mid->count() differently than Win32 or Linux so
    // we had to create and use the OS specific contentions() function.
    //
    int n_want_lock = mid->contentions();     // number of threads contending for the monitor
    int n_waiters = mid->waiters();
    // this is an unused monitor so skip it
    if (thread == NULL && n_want_lock == 0 && n_waiters == 0) return;
    // dump header
    _dump->dump_type(JVMPI_MONITOR_JAVA);
    _dump->dump_oop((oop)mid->object());
    _dump->dump_thread(thread == NULL ? NULL : thread->jni_environment());
    _dump->dump_u4(n_want_lock + n_waiters); // entry count
    _dump->dump_u4(n_want_lock); // number of threads waiting to enter
    if (n_want_lock > 0) {
      GrowableArray<JavaThread*>* want_list = Threads::get_pending_threads(
	n_want_lock, (address)mid, false /* no locking needed */);
      for (int i = 0; i < n_want_lock; i++) {
        if (i < want_list->length()) {
          JavaThread* jt = want_list->at(i);
          _dump->dump_thread(jt->jni_environment());
        } else {
          _dump->dump_thread(NULL);
        }
      }
    }
    _dump->dump_u4(n_waiters); // number of threads waiting to be notified
    if (n_waiters > 0) {
      ObjectWaiter* waiter = mid->first_waiter();
      for (int i = 0; i < n_waiters; i++) {
//        assert(waiter != NULL, "wrong number of waiters");
// No guarantee this value doesn't change while dumping
	if (waiter != NULL) {
          Thread* thd = mid->thread_of_waiter(waiter);
          if (thd->is_Java_thread()) {
            _dump->dump_thread(((JavaThread*)thd)->jni_environment());
          } else {
            _dump->dump_thread(NULL);
          }
          waiter = mid->next_waiter(waiter);
	} else {
	  _dump->dump_thread(NULL);
	}
      }
    }
  }

 public:
  MonitorDumper(Dump* dump, ObjectMonitor* mid): _dump(dump) {
    // dump Java lock
    dump_for_thread(mid, Threads::owning_thread_from_monitor_owner(
      (address)mid->owner(), false /* no locking needed */));
  }
};


class JavaMonitorDumper: public MonitorClosure {
 private:
  Dump* _dump;

 public:
  JavaMonitorDumper(Dump* dump) : _dump(dump) {
    assert(SafepointSynchronize::is_at_safepoint(), "must be at safepoint");
    ObjectSynchronizer::monitors_iterate(this);   // first dump the monitor cache
    if (!UseHeavyMonitors) {	// now dump any lightweight monitors
      ResourceMark rm;
      GrowableArray<ObjectMonitor*>* fab_list = Threads::jvmpi_fab_heavy_monitors();
      for (int i = 0; i < fab_list->length(); i++) {
        ObjectMonitor* fab = fab_list->at(i);
        assert(fab != NULL, "Expected fabricated heavyweight monitor");
        MonitorDumper(_dump, fab);
        // ObjectMonitor is a CHeap object, so remember to free it
        delete fab;
      }
    }
  }
  void do_monitor(ObjectMonitor* mid)  { MonitorDumper(_dump, mid); }
};


class RawMonitorDumper: public StackObj {
 private:
  Dump* _dump;
 public:
  RawMonitorDumper(Dump* dump) : _dump(dump) {
    for(JavaThread *thread = Threads::first(); thread; thread = thread->next()) {
      dump_rawmonitors_for(thread);
    }
  }

  void dump_rawmonitors_for(JavaThread* thread) {
    char* no_name = NULL;
    for (RawMonitor* mon = thread->rawmonitor_list(); mon; mon = mon->next_raw()) {
      assert((PROF_RM_CHECK(mon)), "invalid raw monitor");
      _dump->dump_type(JVMPI_MONITOR_RAW);
      _dump->dump_char_array(mon->name());
      _dump->dump_rawmonitor((JVMPI_RawMonitor) mon);
      _dump->dump_thread(thread->jni_environment());
      dump_monitor_info(mon, thread);
    }
  }

  void dump_monitor_info(RawMonitor* mid, JavaThread* thread)
{
    //
    // Solaris implements mid->count() differently than Win32 or Linux so
    // we had to create and use the OS specific contentions() function.
    //
    int n_want_lock = mid->contentions();     // number of threads contending for the monitor
    int n_waiters = mid->waiters();
    // this is an unused monitor so skip it
    if (thread == NULL && n_want_lock == 0 && n_waiters == 0) return;
    // dump header
    _dump->dump_u4(n_want_lock + n_waiters); // entry count
    _dump->dump_u4(n_want_lock); // number of threads waiting to enter
    if (n_want_lock > 0) {
      GrowableArray<JavaThread*>* want_list = Threads::get_pending_threads(
	n_want_lock, (address)mid, false /* no locking needed */);
      for (int i = 0; i < n_want_lock; i++) {
        if (i < want_list->length()) {
          JavaThread* jt = want_list->at(i);
          _dump->dump_thread(jt->jni_environment());
        } else {
          _dump->dump_thread(NULL);
        }
      }
    }
    _dump->dump_u4(n_waiters); // number of threads waiting to be notified
    if (n_waiters > 0) {
      ObjectWaiter* waiter = mid->first_waiter();
      for (int i = 0; i < n_waiters; i++) {
//        assert(waiter != NULL, "wrong number of waiters");
// no guarantee that this is not changing dynamically
          if (waiter != NULL) {
            Thread* thd = mid->thread_of_waiter(waiter);
            if (thd->is_Java_thread()) {
              _dump->dump_thread(((JavaThread*)thd)->jni_environment());
            } else {
              _dump->dump_thread(NULL);
            }
            waiter = mid->next_waiter(waiter);
          } else {
            _dump->dump_thread(NULL);
          }
      }
    }
  }
};

// JVMPI GC Root Collection support

class HeapDumpInfoCollector;
class RootElementForThread;
class RootElementForFrame;

class CollectRootOopsClosure : public OopClosure {
public:
  enum RootType {
    _unknown,
    _jni_handle,
    _stack_frame,
    _system_class,
    _thread_block,
    _monitor_used
  };

 private:
  JavaThread* _thread;
  intptr_t*   _frame_id;
  bool        _is_native_frame;
  bool        _is_entry_frame;
  GrowableArray<RootType>* typesStack;
  HeapDumpInfoCollector*   _rc;

 public:
  CollectRootOopsClosure(HeapDumpInfoCollector *rc) {
    _rc = rc;
    typesStack = new GrowableArray<RootType>(5, true);
    // Support nested begin_iterate and end_iterate calls
    typesStack->push(_unknown);
  }
  ~CollectRootOopsClosure() {
    assert(typesStack->length() == 1, "All types should be popped");
    typesStack->clear_and_deallocate();
  }
  void set_thread(JavaThread* thread) { 
    _thread = thread;
    _frame_id = NULL;
    _is_native_frame = false;
    _is_entry_frame = false;
  }
  void set_frame_type(bool is_native, bool is_entry) {
    _is_native_frame = is_native;
    _is_entry_frame = is_entry;
  }
  void set_frame_id(intptr_t* id) {
    _frame_id = id; 
  }
  void begin_iterate(RootType type) { typesStack->push(type); }
  void end_iterate(RootType type) { 
    RootType t = typesStack->pop();
    assert(t == type, "type doesn't match");
  }
  void do_oop(oop* obj_p);
};

class CallTraceDump: public StackObj {
  jint _num_traces;
  int  _index;
  int  _frame_index;
  JVMPI_CallTrace*  _traces;
  JVMPI_CallFrame** _frames;
public:
  CallTraceDump() { _num_traces = 0; _traces = NULL; _frames = NULL; _index = 0; }
  ~CallTraceDump() {
    for (int i = 0; i < _num_traces; i++) {
      FREE_C_HEAP_ARRAY(JVMPI_CallFrame, _frames[i]);
    }
    FREE_C_HEAP_ARRAY(JVMPI_CallTrace, _traces);
    FREE_C_HEAP_ARRAY(JVMPI_CallFrame*, _frames);
  }
  void set_calltrace(JavaThread* thread, int num_frames) {
    assert(_traces != NULL && _index < _num_traces, "check number of calltraces generated");
    assert(_index == -1 || _frame_index == _traces[_index].num_frames, "Previous call trace is not filled.");
    _index++;
    _frames[_index] = NEW_C_HEAP_ARRAY(JVMPI_CallFrame, num_frames);
    _traces[_index].env_id = thread->jni_environment();;
    _traces[_index].num_frames = num_frames;
    _traces[_index].frames = _frames[_index];
    _frame_index = 0;
  }
  void set_callframe(jint lineno, jmethodID method_id) {
    assert(_traces[_index].frames != NULL, "JVMPI_CallFrames must have been allocated"); 
    assert(_frame_index < _traces[_index].num_frames, "Invalid _frame_index");
    JVMPI_CallFrame* frame = _traces[_index].frames;
    frame[_frame_index].lineno = lineno;
    frame[_frame_index].method_id = method_id;
    _frame_index++;
  }
  void set_num_traces(jint num_traces) { 
    _num_traces = num_traces; 
    _index = -1;
    _frame_index = -1;
    if (num_traces > 0) {
      _traces = NEW_C_HEAP_ARRAY(JVMPI_CallTrace, num_traces);
      _frames = NEW_C_HEAP_ARRAY(JVMPI_CallFrame*, num_traces);
    } else {
      _traces = NULL;
      _frames = NULL;
    }
  }
  jint get_num_traces() { return _num_traces; }
  JVMPI_CallTrace* get_calltraces() { 
    assert(_index == (_num_traces - 1), "Not all call traces are filled");
    assert(_frame_index == _traces[_index].num_frames, "The last call trace is not filled");
    return _traces; 
  }
};

const jint ROOT_JNI_GLOBAL_SIZE   = (1 + BytesPerWord * 2);
const jint ROOT_JNI_LOCAL_SIZE    = (1 + BytesPerWord * 2 + 4);
const jint ROOT_JAVA_FRAME_SIZE   = (1 + BytesPerWord * 2 + 4);
const jint ROOT_NATIVE_STACK_SIZE = (1 + BytesPerWord * 2);
const jint ROOT_STICKY_CLASS_SIZE = (1 + BytesPerWord);
const jint ROOT_THREAD_BLOCK_SIZE = (1 + BytesPerWord * 2);
const jint ROOT_MONITOR_USED_SIZE = (1 + BytesPerWord);
const jint ROOT_UNKNOWN_SIZE      = (1 + BytesPerWord);
const jint INIT_ROOTS_ARRAY_SIZE  = 256;

class HeapDumpInfoCollector: public StackObj {
 private:
  jint                  _num_threads;
  RootElementForThread* _threadRootInfo;
  GrowableArray<oop*>*  _jni_global_roots;
  GrowableArray<oop>*   _sticky_class_roots;
  GrowableArray<oop>*   _monitor_used_roots;
  GrowableArray<oop>*   _unknown_roots;

  void collect_roots();
  void add_root_to_thread(jint root_type, oop root, JavaThread* thread = NULL, intptr_t* sp = NULL, oop* obj_p = NULL);
  void set_curRootThread(JavaThread *thread);
  RootElementForThread* curRootThread;
  bool                  is_collect_roots;

 public:

  // HeapDumpInfoCollector collects call traces and
  // if roots is true, it collects GC root references as well.
  HeapDumpInfoCollector(bool collect_gc_roots);
  ~HeapDumpInfoCollector();

  bool is_jni_local(JavaThread* thread, intptr_t* sp, oop* obj_p);
  void add_root(jint root_type, oop root, JavaThread* thread = NULL, intptr_t* sp = NULL, oop* obj_p = NULL);
  void add_root(jint root_type, oop* root); // JNI global reference
  jlong root_dump_size() const;
  void dump_roots(Dump* dump) const;
  void dump_calltraces(CallTraceDump* traces) const;

  static void sort_roots(GrowableArray<oop>* roots);
};


class RootElementForFrame : public CHeapObj {
 private:
  intptr_t* _frame_id;
  jint      _depth;
  bool      _is_native_method;
  jint      _lineno; 
  jmethodID _method_id;
  GrowableArray<oop>*  _roots;
  GrowableArray<oop>*  _jni_local_roots;
  GrowableArray<oop*>* _jni_local_refs;
  RootElementForFrame* _next;

 public:
  RootElementForFrame(intptr_t* id, bool is_native, jmethodID mid = 0, jint lineno = 0, jint d = 0) {
    _frame_id = id;
    _is_native_method = is_native;
    _method_id = mid;
    _lineno = lineno;
    _depth = d;
    _next = NULL;
    _roots = NULL;
    _jni_local_roots = NULL;
    _jni_local_refs = NULL;
  }
  ~RootElementForFrame() {
    if (_roots != NULL) {
      _roots->clear_and_deallocate();
      FreeHeap(_roots);
    }
    if (_jni_local_roots != NULL) {
      _jni_local_roots->clear_and_deallocate();
      _jni_local_refs->clear_and_deallocate();
      delete _jni_local_roots;
      delete _jni_local_refs;
    }
  };
  RootElementForFrame* next()           { return _next; }
  void set_next(RootElementForFrame* p) { _next = p; }
  void set_depth(jint d)                { _depth = d; }
  jint lineno()                         { return _lineno; } 
  jmethodID method_id()                 { return _method_id; } 
  intptr_t* frame_id()                  { return _frame_id; }
  bool is_jni_local(oop* obj_p) {
    if (_jni_local_refs == NULL) return false;

    int length = _jni_local_refs->length();
    for (int i = 0; i < length; i++) {
      if (_jni_local_refs->at(i) == obj_p) {
        return true;
      }
    }
    return false;
  }
  void add_root(oop obj) {
    if (_roots == NULL) {
      _roots = new (ResourceObj::C_HEAP) GrowableArray<oop>(INIT_ROOTS_ARRAY_SIZE, true);
    }
    _roots->append(obj); 
  }
  void add_jni_local(oop obj, oop* obj_p) {
    assert(obj_p != NULL, "JNI local ref");
    if (_jni_local_roots == NULL) {
      _jni_local_roots = new GrowableArray<oop>(INIT_ROOTS_ARRAY_SIZE, true);
      _jni_local_refs = new GrowableArray<oop*>(INIT_ROOTS_ARRAY_SIZE, true);
    }
    _jni_local_roots->append(obj);
    _jni_local_refs->append(obj_p);
  }
  void sort_roots() {
    HeapDumpInfoCollector::sort_roots(_roots);
    HeapDumpInfoCollector::sort_roots(_jni_local_roots);
  }
  void dump_roots(Dump* dump, JNIEnv* env_id) const;
  jlong root_dump_size() const;
};

class RootElementForThread : public CHeapObj {
 private:
  JavaThread* _thread;
  jint        _num_frames;
  RootElementForFrame*  _frameRootInfo;
  RootElementForFrame*  _empty_java_frame;
  GrowableArray<oop>*   _native_stack_roots;
  GrowableArray<oop>*   _thread_block_roots;
  RootElementForThread* _next;

  void get_stack_trace();
  void add_root_to_frame(jint root_type, oop root, intptr_t* sp, oop* obj_p = NULL);
  RootElementForFrame* curRootFrame;

 public:
  RootElementForThread(JavaThread* t, bool is_collect_roots);
  ~RootElementForThread();

  RootElementForFrame* get_frame(intptr_t* id);
  RootElementForThread* next()           { return _next; }
  void set_next(RootElementForThread* p) { _next = p; }
  JavaThread* thread()                   { return _thread; }
  bool is_jni_local(intptr_t* sp, oop* obj_p);
  void add_root(jint root_type, oop root, intptr_t* sp, oop* obj_p = NULL);
  void sort_roots() {
    if (_num_frames == 0) {
      _empty_java_frame->sort_roots();
    } else {
      for (RootElementForFrame* p = _frameRootInfo; p != NULL; p = p->next()) {
        p->sort_roots();
      }    
    }

    HeapDumpInfoCollector::sort_roots(_native_stack_roots);
    HeapDumpInfoCollector::sort_roots(_thread_block_roots);
  }
  void dump_roots(Dump* dump) const;
  jlong root_dump_size() const;
  void dump_calltrace(CallTraceDump* dump) const;
};

// Implementation of CollectRootOopsClosure::do_oop()
void CollectRootOopsClosure::do_oop(oop* obj_p) {
  oop obj = *obj_p;
  RootType type = typesStack->top();
  bool is_klass = false;

  if (obj == NULL || 
      (type == _system_class && !obj->is_klass()) || // Skip if not a klass for system class roots
      (type != _system_class && !obj->is_instance() && !obj->is_typeArray() && !obj->is_objArray())) { 
      return;
  }

  if (obj->is_klass()) {
    if (obj->blueprint()->oop_is_instanceKlass() || obj->blueprint()->oop_is_typeArrayKlass() || obj->blueprint()->oop_is_objArrayKlass()) {
      obj = Klass::cast((klassOop)obj)->java_mirror();
      is_klass = true;
    }
  }

  switch (type) {
    case _unknown:
      _rc->add_root(JVMPI_GC_ROOT_UNKNOWN, obj);
      break;
    case _jni_handle:
      if (obj == JNIHandles::deleted_handle()) {
        // skip deleted handles
        break;
      }
      if (_thread == NULL) {
        _rc->add_root(JVMPI_GC_ROOT_JNI_GLOBAL, obj_p);
      } else {
        _rc->add_root(JVMPI_GC_ROOT_JNI_LOCAL, obj, _thread, _frame_id, obj_p);
      }
      break;
    case _stack_frame:
      if (_is_native_frame) {
        _rc->add_root(JVMPI_GC_ROOT_NATIVE_STACK, obj, _thread);
      } else if (_is_entry_frame) {
        // JNI local refs in an entry frame have been traversed separately earlier.
        // So skip these JNI local refs when they are traversed again in oops_do()
        // call for this entry frame.

        if (obj != JNIHandles::deleted_handle() && !_rc->is_jni_local(_thread, _frame_id, obj_p)) {
          _rc->add_root(JVMPI_GC_ROOT_JAVA_FRAME, obj, _thread, _frame_id);
        }
      } else {
        _rc->add_root(JVMPI_GC_ROOT_JAVA_FRAME, obj, _thread, _frame_id);
      }
      break;
    case _system_class:
      if (is_klass) {
        _rc->add_root(JVMPI_GC_ROOT_STICKY_CLASS, obj);
      }
      break;
    case _thread_block:
      assert(_thread != NULL, "NULL thread for CollectRootOopsClosure::_thread_block type");
      _rc->add_root(JVMPI_GC_ROOT_THREAD_BLOCK, obj, _thread);
      break;
    case _monitor_used:
      _rc->add_root(JVMPI_GC_ROOT_MONITOR_USED, obj);
      break;
    default:
      ShouldNotReachHere();
  }
}

// Implementation of RootElementForFrame class
void RootElementForFrame::dump_roots(Dump* dump, JNIEnv* env_id) const {
  int length, i;

  length = (_roots != NULL ? _roots->length() : 0);
  for (i = 0; i < length; i++) {
    dump->dump_type(JVMPI_GC_ROOT_JAVA_FRAME);
    dump->dump_oop(_roots->at(i));
    dump->dump_thread(env_id);
    dump->dump_u4(_depth);
  }
  
  length = (_jni_local_roots != NULL ? _jni_local_roots->length() : 0);
  for (i = 0; i < length; i++) {
    dump->dump_type(JVMPI_GC_ROOT_JNI_LOCAL);
    dump->dump_oop(_jni_local_roots->at(i));
    dump->dump_thread(env_id);
    dump->dump_u4(_depth);
  }
}

jlong RootElementForFrame::root_dump_size() const {
  jlong size = (_roots != NULL ? _roots->length() : 0) * ROOT_JAVA_FRAME_SIZE;
  size += (_jni_local_roots != NULL ? _jni_local_roots->length() : 0) * ROOT_JNI_LOCAL_SIZE;

  return size;
};

// Implementation of RootElementForThread class
RootElementForThread::RootElementForThread(JavaThread* t, bool is_collect_roots) {
  _thread = t;
  _next = NULL;
  _frameRootInfo = NULL;
  _empty_java_frame = NULL;
  _thread_block_roots = NULL;
  _native_stack_roots = NULL;
  _num_frames = 0;
  curRootFrame = NULL;

  if (is_collect_roots) {
    // create root arrays for collecting roots
    _native_stack_roots = new GrowableArray<oop>(INIT_ROOTS_ARRAY_SIZE, true);
    _thread_block_roots = new GrowableArray<oop>(INIT_ROOTS_ARRAY_SIZE, true);
  }

  get_stack_trace();
}

RootElementForThread::~RootElementForThread() {
  RootElementForFrame* p = _frameRootInfo;
  while (p != NULL) {
    RootElementForFrame *q = p;
    p = p->next();
    delete(q);
  }
  delete _empty_java_frame;
  if (_native_stack_roots != NULL) {
    _native_stack_roots->clear_and_deallocate();
    delete _native_stack_roots;
  }
  if (_thread_block_roots != NULL) {
    _thread_block_roots->clear_and_deallocate();
    delete _thread_block_roots;
  }
}

void RootElementForThread::get_stack_trace(){
  assert(_thread->thread_state() != _thread_in_Java, "All threads must be blocked at safepoint");

  if (!_thread->has_last_Java_frame()) {
    _empty_java_frame = new RootElementForFrame(0, false);
    _empty_java_frame->set_depth(-1);
    return;
  }

  vframeStream vfst(_thread);
  RootElementForFrame* last = NULL;
  int count = 0;

  // Get call trace for this JavaThread
  for (; !vfst.at_end(); vfst.next(), count++) {
    methodOop m = vfst.method(); // The method is not stored GC safe
    int bci     = vfst.bci();
    int lineno  = m->is_native() ? (-3) : m->line_number_from_bci(bci);

    RootElementForFrame* p = new RootElementForFrame(vfst.frame_id(),
                                                     m->is_native(),
                                                     m->jmethod_id(),
                                                     lineno);
    if (last == NULL) {
      _frameRootInfo = p;
    } else {
      last->set_next(p);
    }
    last = p;
  }

  _num_frames = count;
  for (RootElementForFrame* p = _frameRootInfo; p != NULL; p = p->next(), count--) {
    p->set_depth(count);
  }
}

RootElementForFrame* RootElementForThread::get_frame(intptr_t* id) {
  if (_num_frames == 0) {
    return _empty_java_frame;
  }

  if (id == NULL) {
    // set to the top vframe
    return _frameRootInfo;
  } else if (curRootFrame == NULL || curRootFrame->frame_id() != id) {
    // find the one with a matching id
    curRootFrame = NULL;
    for (RootElementForFrame* p = _frameRootInfo; p != NULL; p = p->next()) {
      if (p->frame_id() == id) {
        curRootFrame = p;
        return curRootFrame;
      }
    }
  }
  return curRootFrame;
}

bool RootElementForThread::is_jni_local(intptr_t* id, oop* obj_p) {
  RootElementForFrame* fr = get_frame(id);

  assert(fr != NULL, "Java Frame not found");
  return fr->is_jni_local(obj_p);
}

void RootElementForThread::add_root_to_frame(jint root_type, oop root, intptr_t* id, oop* obj_p) {
  RootElementForFrame* fr = get_frame(id);

  assert(fr != NULL, "Java Frame not found");

  if (root_type == JVMPI_GC_ROOT_JNI_LOCAL) {
    fr->add_jni_local(root, obj_p);
  } else {
    fr->add_root(root);
  }
}


void RootElementForThread::add_root(jint root_type, oop root, intptr_t* id, oop* obj_p) {
  switch (root_type) {
    case JVMPI_GC_ROOT_JNI_LOCAL:
      add_root_to_frame(root_type, root, id, obj_p);
      break;
    case JVMPI_GC_ROOT_JAVA_FRAME:
      add_root_to_frame(root_type, root, id);
      break;
    case JVMPI_GC_ROOT_NATIVE_STACK:
      _native_stack_roots->append(root);
      break;
    case JVMPI_GC_ROOT_THREAD_BLOCK:
      _thread_block_roots->append(root);
      break;
    default:
      ShouldNotReachHere();
  }
}

jlong RootElementForThread::root_dump_size() const {
  jlong size = (_empty_java_frame != NULL ? _empty_java_frame->root_dump_size() : 0) + 
              (_native_stack_roots->length() * ROOT_NATIVE_STACK_SIZE) +
              (_thread_block_roots->length() * ROOT_THREAD_BLOCK_SIZE);

  for (RootElementForFrame* p = _frameRootInfo; p != NULL; p = p->next()) {
    size += p->root_dump_size();
  }

  return size;
};

void RootElementForThread::dump_roots(Dump* dump) const {
  JNIEnv* env_id = _thread->jni_environment();

  if (_num_frames == 0) {
    _empty_java_frame->dump_roots(dump, env_id);
  } else {
    for (RootElementForFrame* p = _frameRootInfo; p != NULL; p = p->next()) {
      p->dump_roots(dump, env_id);
    }
  }  
  
  int length, i;

  length = _native_stack_roots->length();
  for (i = 0; i < length; i++) {
    dump->dump_type(JVMPI_GC_ROOT_NATIVE_STACK);
    dump->dump_oop(_native_stack_roots->at(i));
    dump->dump_thread(env_id);
  }
  length = _thread_block_roots->length();
  for (i = 0; i < length; i++) {
    dump->dump_type(JVMPI_GC_ROOT_THREAD_BLOCK);
    dump->dump_oop(_thread_block_roots->at(i));
    dump->dump_thread(env_id);
  }
}

void RootElementForThread::dump_calltrace(CallTraceDump* dump) const {
  dump->set_calltrace(_thread, _num_frames);
  for (RootElementForFrame* p = _frameRootInfo; p != NULL; p = p->next()) {
    dump->set_callframe(p->lineno(), p->method_id());
  }
}

// Implementation of HeapDumpInfoCollector
HeapDumpInfoCollector::HeapDumpInfoCollector(bool collect_gc_roots) {
  // initialize _threadRootInfo before collecting roots
  RootElementForThread* q = NULL;
  _num_threads = 0;
  for (JavaThread* thread = Threads::first(); thread != NULL ; thread = thread->next()) {
    RootElementForThread* p = new RootElementForThread(thread, collect_gc_roots);
    if (q == NULL) {
      _threadRootInfo = p;
    } else {
      q->set_next(p);
    }
    q = p;
    _num_threads++;
  }

  if (collect_gc_roots) {
    _jni_global_roots = new GrowableArray<oop*>(INIT_ROOTS_ARRAY_SIZE, true);
    _sticky_class_roots = new GrowableArray<oop>(INIT_ROOTS_ARRAY_SIZE, true);
    _monitor_used_roots = new GrowableArray<oop>(INIT_ROOTS_ARRAY_SIZE, true);
    _unknown_roots = new GrowableArray<oop>(INIT_ROOTS_ARRAY_SIZE, true);
    curRootThread = NULL;
    collect_roots(); 
  }
  is_collect_roots = collect_gc_roots;
}

HeapDumpInfoCollector::~HeapDumpInfoCollector() {
  RootElementForThread* p = _threadRootInfo;
  while (p != NULL) {
    RootElementForThread *q = p;
    p = p->next();
    delete(q);
  }
    
  if (is_collect_roots) {
    _jni_global_roots->clear_and_deallocate();
    _sticky_class_roots->clear_and_deallocate();
    _monitor_used_roots->clear_and_deallocate();
    _unknown_roots->clear_and_deallocate();
    // no need to delete these growable array pointers since HeapDumpInfoCollector is a StackObj
  }
}

// Collect roots for heap dump
// Note: the current implemenation of collect_roots() requires explicit knowledge
// about GC strong roots as well as explicit knowledge about frames.  This function
// may need to be modified if future modification to the VM internal structures is
// made.  Watch for future modification to oops_do() methods.
//
// Another way to implement it is to modify OopClosure class to add new methods
// (nop by default) for passing additional profiling information. In addition,
// modify oops_do() method in various classes to call those OopClosure new
// methods to pass the root type information.  However, it is not advised to 
// modify OopClosure to affect its simplicity and its semantics. So we chose
// the current implemenation.
//
void HeapDumpInfoCollector::collect_roots() {
  CollectRootOopsClosure blk(this);

  // Traverse all system classes
  blk.begin_iterate(CollectRootOopsClosure::_system_class);
  SystemDictionary::always_strong_oops_do(&blk);
  blk.end_iterate(CollectRootOopsClosure::_system_class);

  // Traverse all JNI Global references
  blk.set_thread(NULL);
  blk.begin_iterate(CollectRootOopsClosure::_jni_handle);
  JNIHandles::oops_do(&blk);   // Global (strong) JNI handles
  blk.end_iterate(CollectRootOopsClosure::_jni_handle);

  // Traverse all monitor objects
  blk.begin_iterate(CollectRootOopsClosure::_monitor_used);
  ObjectSynchronizer::oops_do(&blk);
  blk.end_iterate(CollectRootOopsClosure::_monitor_used);

  // Traverse JNI locals and frames for all Java threads 
  RootElementForFrame *prev_reff = NULL;
  for (JavaThread* thread = Threads::first(); thread != NULL ; thread = thread->next()) {  
    blk.set_thread(thread);
    set_curRootThread(thread);

    // get all JNI local references for the top frame
    blk.begin_iterate(CollectRootOopsClosure::_jni_handle);
    thread->active_handles()->oops_do(&blk);
    blk.end_iterate(CollectRootOopsClosure::_jni_handle);

    // Traverse the execution stack    
    blk.begin_iterate(CollectRootOopsClosure::_stack_frame);
    if (thread->has_last_Java_frame()) {
      for(StackFrameStream fst(thread); !fst.is_done(); fst.next()) {
        frame* fr = fst.current();

        // skip the first entry frame
        if (fr->is_first_frame()) continue;

        blk.set_frame_type(fr->is_native_frame(), fr->is_entry_frame());
        if (fr->is_entry_frame()) {
          // An entry frame is considered part of the previous Java
          // frame on the stack. Use the id from the previous frame
          // that was found on the RootElementForFrame list.
          assert(prev_reff != NULL, "must have previous frame");
          blk.set_frame_id(prev_reff->frame_id());

          // traverse the JNI local refs stored in JavaCallWrapper for an entry frame
          blk.begin_iterate(CollectRootOopsClosure::_jni_handle);
          fr->entry_frame_call_wrapper()->handles()->oops_do(&blk);
          blk.end_iterate(CollectRootOopsClosure::_jni_handle);

        } else {
          // remember id of the current frame for frame information in the oops traversal.
          blk.set_frame_id(fr->id());
        }
        fr->oops_do(&blk, fst.register_map());

        // If the current frame is found on the RootElementForFrame
        // list, then save it for a possible "entry frame" later.
        RootElementForFrame *reff = curRootThread->get_frame(fr->id());
        if (reff != NULL) {
          prev_reff = reff;
        }
      }
    }
    blk.end_iterate(CollectRootOopsClosure::_stack_frame);
  }

  // sort and remove duplicates
  // no need to sort _jni_global_roots because all JNI global references are
  // traversed only once.
  for (RootElementForThread* p = _threadRootInfo; p != NULL; p = p->next()) {
    p->sort_roots();
  }
  sort_roots(_sticky_class_roots);
  sort_roots(_monitor_used_roots);
  sort_roots(_unknown_roots);
}

static int cmp(oop* x, oop* y) { return *x - *y; }
void HeapDumpInfoCollector::sort_roots(GrowableArray<oop>* roots) {
  if (roots == NULL) return;

  // sort roots
  roots->sort(cmp);

  // remove duplicates by compacting array
  const int len = roots->length();
  oop obj = NULL; // we don't need NULL roots
  int j = 0;
  for (int i = 0; i < len; i++) {
    assert(i >= j, "algorithmic error");
    if (roots->at(i) != obj) {
      obj = roots->at(i);
      roots->at_put(j++, obj);
    }
  }
  roots->trunc_to(j);
  assert(roots->length() == j, "just checking");
}

void HeapDumpInfoCollector::set_curRootThread(JavaThread *thread) {
  if (curRootThread == NULL || curRootThread->thread() != thread) {
    curRootThread = NULL;
    for (RootElementForThread* p = _threadRootInfo; p != NULL; p = p->next()) {
      if (p->thread() == thread) {
        curRootThread = p;
        break;
      }
    }
  }
  assert(curRootThread != NULL, "Thread not found");
}

bool HeapDumpInfoCollector::is_jni_local(JavaThread* thread, intptr_t* sp, oop* obj_p) {
  set_curRootThread(thread);
  return curRootThread->is_jni_local(sp, obj_p);
}

jlong HeapDumpInfoCollector::root_dump_size() const {
  jlong size = (_jni_global_roots->length() * ROOT_JNI_GLOBAL_SIZE) +
              (_sticky_class_roots->length() * ROOT_STICKY_CLASS_SIZE) +
              (_monitor_used_roots->length() * ROOT_MONITOR_USED_SIZE) +
              (_unknown_roots->length() * ROOT_UNKNOWN_SIZE);

  for (RootElementForThread* p = _threadRootInfo; p != NULL; p = p->next()) {
    size += p->root_dump_size();
  }
  return size;
}

void HeapDumpInfoCollector::dump_roots(Dump* dump) const {
  for (RootElementForThread* p = _threadRootInfo; p != NULL; p = p->next()) {
    p->dump_roots(dump);
  }

  int length, i;

  length = _jni_global_roots->length();
  for (i = 0; i < length; i++) {
    oop* handle = _jni_global_roots->at(i);
    oop obj = *handle;

    dump->dump_type(JVMPI_GC_ROOT_JNI_GLOBAL);
    if (obj->is_klass()) {
      obj = Klass::cast((klassOop)obj)->java_mirror();
    }
    dump->dump_oop(obj);
    dump->dump_voids((void*) handle);
  }
  length = _sticky_class_roots->length();
  for (i = 0; i < length; i++) {
    dump->dump_type(JVMPI_GC_ROOT_STICKY_CLASS);
    dump->dump_oop(_sticky_class_roots->at(i));
  }
  length = _monitor_used_roots->length();
  for (i = 0; i < length; i++) {
    dump->dump_type(JVMPI_GC_ROOT_MONITOR_USED);
    dump->dump_oop(_monitor_used_roots->at(i));
  }
  length = _unknown_roots->length();
  for (i = 0; i < length; i++) {
    dump->dump_type(JVMPI_GC_ROOT_UNKNOWN);
    dump->dump_oop(_unknown_roots->at(i));
  }

}

void HeapDumpInfoCollector::add_root_to_thread(jint root_type, oop root, JavaThread* thread, intptr_t* sp, oop* obj_p) {
  set_curRootThread(thread);
  curRootThread->add_root(root_type, root, sp, obj_p);
}

void HeapDumpInfoCollector::add_root(jint root_type, oop* root) {
  assert(root_type == JVMPI_GC_ROOT_JNI_GLOBAL, "Must be JNI globals");

  bool is_root = true;
  int length = (skipped_globalrefs != NULL ? skipped_globalrefs->length() : 0);
  for (int i = 0; i < length; i++) {
    if (skipped_globalrefs->at(i) == (jobject) root) {
      is_root = false;
      break;
    }
  }

  if (is_root) {
    _jni_global_roots->append(root);
  }
}

void HeapDumpInfoCollector::add_root(jint root_type, oop root, JavaThread* thread, intptr_t* sp, oop* obj_p) {
  switch (root_type) {
    case JVMPI_GC_ROOT_UNKNOWN:
      _unknown_roots->append(root);
      break;
    case JVMPI_GC_ROOT_JNI_LOCAL:
      add_root_to_thread(root_type, root, thread, sp, obj_p);
      break;
    case JVMPI_GC_ROOT_JAVA_FRAME:
    case JVMPI_GC_ROOT_NATIVE_STACK:
      add_root_to_thread(root_type, root, thread, sp);
      break;
    case JVMPI_GC_ROOT_STICKY_CLASS:
      _sticky_class_roots->append(root);
      break;
    case JVMPI_GC_ROOT_THREAD_BLOCK:
      add_root_to_thread(root_type, root, thread, sp);
      break;
    case JVMPI_GC_ROOT_MONITOR_USED:
      _monitor_used_roots->append(root);
      break;
    default:
      ShouldNotReachHere();
  }
}

void HeapDumpInfoCollector::dump_calltraces(CallTraceDump* dump) const {
  dump->set_num_traces(_num_threads);
  for (RootElementForThread* p = _threadRootInfo; p != NULL; p = p->next()) {
    p->dump_calltrace(dump);
  }
}

void jvmpi::post_object_dump_event(oop obj, int flag) {
  No_GC_Verifier nogc;
  Dump dump;
  // 1st dump to measure dump size
  { ObjectDumper od(&dump, JVMPI_DUMP_LEVEL_2, obj); }
  // 2nd dump to actually write dump
  dump.enable_write(dump.size());
  { ObjectDumper od(&dump, JVMPI_DUMP_LEVEL_2, obj); }
  // create event
  JVMPI_Event event;
  event.event_type             = JVMPI_EVENT_OBJECT_DUMP | flag;
  event.u.heap_dump.begin      = (char*)dump.begin();
  event.u.heap_dump.end        = (char*)dump.end  ();
  event.u.heap_dump.num_traces = 0;
  event.u.heap_dump.traces     = NULL;
  // post event
  post_event_vm_mode(&event, NULL);
}

class VM_JVMPIPostHeapDump: public VM_Operation {
 private:
  Dump* _dump;
  int   _level;
  int   _flag;
  CallTraceDump* _traces;
 public:
  VM_JVMPIPostHeapDump(Dump* dump, int level, int flag, CallTraceDump *traces) { 
    _dump   = dump; 
    _level  = level; 
    _flag   = flag;  
    _traces = traces;
  }
  void doit() {
    // 1st heap dump to measure dump size for heap objects
    { HeapDumper hd(_dump, _level); }
    // collect VM roots and dump them
    if (_level == JVMPI_DUMP_LEVEL_0) {
      // dump level 0 => no roots
      HeapDumpInfoCollector rd(false);
      _dump->enable_write(_dump->size());
      rd.dump_calltraces(_traces);
    } else {
      // dump level 1 & 2 => include roots
      HeapDumpInfoCollector rd(true);
      debug_only(int heap_dump_size = _dump->size());
      debug_only(int gc_root_dump_size = rd.root_dump_size());

      _dump->enable_write((int) rd.root_dump_size() + _dump->size());
      rd.dump_roots(_dump);
      rd.dump_calltraces(_traces);
      assert((int) rd.root_dump_size() == _dump->size(), "dump size inconsistent");
    }
    // 2nd heap dump to actually write heap objects
    { HeapDumper hd(_dump, _level); }

    // Disable GC to prevent GC from happening before the agent 
    // finishes processing the heap dump.

    GC_locker::lock();
  }
  const char* name() const { return "post JVMPI heap dump"; }
};


void jvmpi::post_heap_dump_event_in_safepoint(int level, int flag) {
  Dump dump;
  CallTraceDump traces;

  {
    // We must acquire the Heap_lock before collecting heap dump 
    MutexLocker ml(Heap_lock);

    // We count and collect the heap information at a safepoint
    VM_JVMPIPostHeapDump op(&dump, level, flag, &traces);
    VMThread::execute(&op);
  }

  // Create and post the event in the JavaThread
  // We don't put this in a doit_epilogue to avoid exposing the Dump class
  //  assert(Thread::current()->is_Java_thread(), "must be in JavaThread");
  JVMPI_Event event;
  event.event_type             = JVMPI_EVENT_HEAP_DUMP | flag;
  event.u.heap_dump.dump_level = level;
  event.u.heap_dump.begin      = (char*)dump.begin();
  event.u.heap_dump.end        = (char*)dump.end  ();
  event.u.heap_dump.num_traces = traces.get_num_traces();
  event.u.heap_dump.traces     = traces.get_calltraces();
  // post event
  post_event_vm_mode(&event, NULL);

  // Enable GC
  GC_locker::unlock();
}


class VM_JVMPIPostMonitorDump: public VM_Operation {
 private:
  Dump* _dump;
  int   _flag;
 public:
  VM_JVMPIPostMonitorDump(Dump* dump, int flag) { _dump = dump; _flag = flag; }
  void doit() {
    // 1st dump to measure dump size
    { JavaMonitorDumper md(_dump); 
      RawMonitorDumper rmd(_dump);
    }
    // 2nd dump to actually write dump
    _dump->enable_write(_dump->size());
    { JavaMonitorDumper md(_dump); 
      RawMonitorDumper rmd(_dump);
    }
  }
  const char* name() const { return "post JVMPI monitor dump"; }
};


void jvmpi::post_monitor_dump_event_in_safepoint(int flag) {
  Dump dump;
  // We count and collect the monitor information at a safepoint
  VM_JVMPIPostMonitorDump op(&dump, flag);
  VMThread::execute(&op);
  // Create and post the event in the JavaThread
  // We don't put this in a doit_epilogue to avoid exposing the Dump class
//  assert(Thread::current()->is_Java_thread(), "must be in JavaThread");
  JVMPI_Event event;
  event.event_type = JVMPI_EVENT_MONITOR_DUMP | flag;
  event.u.monitor_dump.begin          = (char*)dump.begin();
  event.u.monitor_dump.end            = (char*)dump.end  ();
  event.u.monitor_dump.num_traces     = 0;
  event.u.monitor_dump.threads_status = 0;
  // post event
  post_event_vm_mode(&event, NULL);
}


bool should_invalidate_nmethods(jint event_type) {
  switch (event_type) {
    case JVMPI_EVENT_METHOD_ENTRY : // fall through
    case JVMPI_EVENT_METHOD_ENTRY2: // fall through
    case JVMPI_EVENT_METHOD_EXIT  : return true;
  }
  return false;
}


void invalidate_nmethods() {
  // need do deoptimize all frames; for the moment we just make all methods
  // non-entrant
}


bool needs_slow_allocation(jint event_type) {
  switch(event_type) {
    case JVMPI_EVENT_OBJECT_ALLOC       : // fall through
    case JVMPI_EVENT_OBJECT_MOVE        : // fall through
    case JVMPI_EVENT_OBJECT_FREE        : // fall through
    case JVMPI_EVENT_ARENA_NEW          : // fall through
    case JVMPI_EVENT_DELETE_ARENA       : // fall through
    case JVMPI_EVENT_JNI_GLOBALREF_ALLOC: // fall through
    case JVMPI_EVENT_JNI_GLOBALREF_FREE : return true;
  }
  return false;
}

void jvmpi::reset_jvmpi_allocation() {
  bool use_jvmpi_allocation = (is_event_enabled(JVMPI_EVENT_OBJECT_ALLOC) ||
                               is_event_enabled(JVMPI_EVENT_OBJECT_MOVE)  ||
                               is_event_enabled(JVMPI_EVENT_OBJECT_FREE)  ||
                               is_event_enabled(JVMPI_EVENT_ARENA_NEW)    ||
                               is_event_enabled(JVMPI_EVENT_DELETE_ARENA) ||
                               is_event_enabled(JVMPI_EVENT_JNI_GLOBALREF_ALLOC) ||
                               is_event_enabled(JVMPI_EVENT_JNI_GLOBALREF_FREE));

  if (use_jvmpi_allocation && !slow_allocation) {
    // Enable slow allocation

    slow_allocation = true;
    Universe::set_jvmpi_alloc_event_enabled(Universe::_jvmpi_enabled);

    // Note:  I think disabling GC-events should be done only 
    // during startup time.  When the agent is ready to handle
    // GC-events, we should report it.  As this piece of code
    // has been there for a while, I just leave it as it is but 
    // we should look into it in jvmpi 2.0.

    // it is too early to report GC-events
    bool old_gc_start = is_event_enabled(JVMPI_EVENT_GC_START);
    bool old_gc_finish = is_event_enabled(JVMPI_EVENT_GC_FINISH);
    disable_event(JVMPI_EVENT_GC_START);
    disable_event(JVMPI_EVENT_GC_FINISH);

    // ensure that the heap is initialized the way we want it to be;
    // in particular, the new generation must be filled so we always
    // perform slow allocations
    Universe::heap()->collect(GCCause::_java_lang_system_gc);

    if (old_gc_start) enable_event(JVMPI_EVENT_GC_START);
    if (old_gc_finish) enable_event(JVMPI_EVENT_GC_FINISH);

  } else if (!use_jvmpi_allocation && slow_allocation) {
    // Disable slow allocation

    slow_allocation = false;

    // Do a GC to enable the heap for fast allocation since the new generation
    // was filled up for slow allocation.  
    // Note that fast allocation is not immediately turned on until a GC 
    // is completed.  If GC is disabled (due to some other jvmpi events),  
    // this GC is cancelled and the new generation is still filled up.

    Universe::set_jvmpi_alloc_event_enabled(Universe::_jvmpi_disabling);
    Universe::heap()->collect(GCCause::_java_lang_system_gc);
  }
}

// ----------------------------------------------
// Functions exported through the JVMPI interface
// ----------------------------------------------

JVMPI_ENTRY(jint, jvmpi::enable_event(jint event_type, void *arg))
  if (!is_event_supported(event_type)) {
    return JVMPI_NOT_AVAILABLE;
  }

  enable_event(event_type);
  if (should_invalidate_nmethods(event_type)) {
    invalidate_nmethods();
  }
  if (event_type == JVMPI_EVENT_OBJECT_MOVE) {
    Universe::set_jvmpi_move_event_enabled(true);
  } else if (event_type == JVMPI_EVENT_METHOD_ENTRY || event_type == JVMPI_EVENT_METHOD_ENTRY2) {
  // Missing disabling of inlining
    // Inline flag is a constant in product mode
    // Inline = false;
  } else if (event_type == JVMPI_EVENT_JNI_GLOBALREF_ALLOC) {
    Universe::set_jvmpi_jni_global_alloc_event_enabled(true);
  } else if (event_type == JVMPI_EVENT_JNI_GLOBALREF_FREE) {
    Universe::set_jvmpi_jni_global_free_event_enabled(true);
  } else if (event_type == JVMPI_EVENT_JNI_WEAK_GLOBALREF_ALLOC) {
    Universe::set_jvmpi_jni_weak_global_alloc_event_enabled(true);
  } else if (event_type == JVMPI_EVENT_JNI_WEAK_GLOBALREF_FREE) {
    Universe::set_jvmpi_jni_weak_global_free_event_enabled(true);
  }

  // enable slow allocation, if necessary 
  if (!slow_allocation && needs_slow_allocation(event_type)) {
    reset_jvmpi_allocation();
  }
  return JVMPI_SUCCESS;
JVMPI_END


JVMPI_ENTRY(jint, jvmpi::disable_event(jint event_type, void *arg))
  if (!is_event_supported(event_type)) {
    return JVMPI_NOT_AVAILABLE;
  }

  if (should_invalidate_nmethods(event_type)) {
    invalidate_nmethods();
  }
  disable_event(event_type);

  if (event_type == JVMPI_EVENT_OBJECT_MOVE) {
    Universe::set_jvmpi_move_event_enabled(false);
  } else if (event_type == JVMPI_EVENT_JNI_GLOBALREF_ALLOC) {
    Universe::set_jvmpi_jni_global_alloc_event_enabled(false);
  } else if (event_type == JVMPI_EVENT_JNI_GLOBALREF_FREE) {
    Universe::set_jvmpi_jni_global_free_event_enabled(false);
  } else if (event_type == JVMPI_EVENT_JNI_WEAK_GLOBALREF_ALLOC) {
    Universe::set_jvmpi_jni_weak_global_alloc_event_enabled(false);
  } else if (event_type == JVMPI_EVENT_JNI_WEAK_GLOBALREF_FREE) {
    Universe::set_jvmpi_jni_weak_global_free_event_enabled(false);
  }

  // disable slow allocation and use fast allocation, if necessary 
  if (slow_allocation && needs_slow_allocation(event_type)) {
    reset_jvmpi_allocation();
  }
  return JVMPI_SUCCESS;
JVMPI_END


JVMPI_ENTRY(void, jvmpi::disable_gc())
  GC_locker::lock();
JVMPI_END


JVMPI_ENTRY(void, jvmpi::enable_gc())
  GC_locker::unlock();
JVMPI_END

inline bool is_valid_method(methodOop method) {
  if (method == NULL || 
      !method->is_perm() || 
      oop(method)->klass() != Universe::methodKlassObj() ||
      !method->is_method()) {
    return false;   // doesn't look good
  }
  return true;      // hopefully this is a method indeed
}

// Return the top-most frame that can be used for vframeStream
// This frame will be skipped by vframeStream for stack walking.
frame is_walkable_frame(JavaThread* thread, frame* fr, methodOop* method_p, int* bci_p) {
  methodOop method = NULL;
  int bci = -1;
  frame walkframe;

  if (fr->is_interpreted_frame()) {
    // top frame is an interpreted frame 
    // check if it is walkable (i.e. valid methodOop and valid bci)
    if (fr->is_interpreted_frame_valid()) {
      if (fr->fp() != NULL) {
        // access address in order not to trigger asserts that
        // are built in interpreter_frame_method function
        method = *fr->interpreter_frame_method_addr();
        if (is_valid_method(method)) {
          intptr_t bcx = fr->interpreter_frame_bcx();
          if (!method->is_native() && (frame::is_bci(bcx) || method->contains((address) bcx))) {
            bci = fr->interpreter_frame_bci();
            // code_size() may return 0 and we allow 0 here
            if (!(bci == 0 || bci >= 0 && bci < method->code_size())) {
              // reset bci to -1 if not a valid bci
              bci = -1;
            }
          }
          walkframe = *fr;
        } else {
          method = NULL;
        }
      }
    }

  } else {
    method = NULL;
    walkframe = *fr;
#ifndef CORE
    // Determine if this top frame is executing a Java method.
    if (CodeCache::contains(fr->pc())) {
      // top frame is a compiled frame or stub routines
      CodeBlob* cb = CodeCache::find_blob(fr->pc());
      if (cb->is_nmethod()) {
        method = ((nmethod *)cb)->method();
      }
    }
#endif 
  }

  if (method_p != NULL) {
    *method_p = method;
  }
  if(bci_p != NULL) {
    *bci_p = bci;
  }
  return walkframe;
}

// The thread we are examining must be suspended
void fill_call_trace_at_safepoint(JavaThread* thd, JVMPI_CallTrace* trace, int depth) {
  vframeStream st(thd); 

  int count = 0;
  // collect the rest
  for (;!st.at_end() && count < depth; st.next(), count++) {    
    methodOop m = st.method(); // The method is not stored GC safe
    int bci     = st.bci();
    int lineno  = m->is_native() ? (-3) : m->line_number_from_bci(bci);
    trace->frames[count].method_id = m->jmethod_id();
    trace->frames[count].lineno = lineno;
  }

  trace->num_frames = count;
  if (TraceJVMPI) {
    tty->cr();
    tty->print_cr("JVMPI: fill_call_trace_at_safepoint return, thread: " INTPTR_FORMAT ", trace->num_frames = %d\n",
		  thd, trace->num_frames);
  }
  return; 
}

void fill_call_trace_given_top(JavaThread* thd, JVMPI_CallTrace* trace, int depth, frame top_frame) {
  frame walkframe;
  methodOop method;
  int bci;
  int count;

  count = 0;
  assert(trace->frames != NULL, "trace->frames must be non-NULL");

  walkframe = is_walkable_frame(thd, &top_frame, &method, &bci);
  if (method != NULL) {
    count++;
    trace->num_frames = count;
    trace->frames[0].method_id = method->jmethod_id();
    if (!method->is_native()) {
      trace->frames[0].lineno = method->line_number_from_bci(bci);
    } else {
      trace->frames[0].lineno = -3;
    }
  } 

  // return if no walkable frame is found
  if (walkframe.sp() == NULL) {
    return;
  }

  // check has_last_Java_frame() after looking at the top frame
  // which may be an interpreted Java frame.
  if (!thd->has_last_Java_frame() && count == 0) {
    trace->num_frames = 0;
    return;
  }

  vframeStream st(thd, walkframe);
  for (; !st.at_end() && count < depth; st.next(), count++) {
    bci = st.bci();
    method = st.method(); // The method is not stored GC safe

    trace->frames[count].method_id = method->jmethod_id();
    if (!method->is_native()) {
      trace->frames[count].lineno = method->line_number_from_bci(bci);
    } else {
      trace->frames[count].lineno = -3;
    }
  }
  trace->num_frames = count;
  return;
}

JVMPI_ENTRY(void, jvmpi::get_call_trace(JVMPI_CallTrace *trace, jint depth))
  JavaThread* thd;
  ResourceMark rm;

  trace->num_frames = 0;
  if (!((trace->env_id) && (thd = JavaThread::thread_from_jni_environment(trace->env_id))))  {
    return;
  }

  // ensure thread suspension completed for other threads
  // Note: need to ensure hprof agent actually suspends threads
  // May need to temporarily suspend thread for the caller
  uint32_t debug_bits = 0;
  if (thd != Thread::current()) {
    if (!thd->wait_for_ext_suspend_completion(SUSPENDRETRYCOUNT, &debug_bits)) {
      return;
    }
  }

  switch (thd->thread_state()) {
    // The thread is either in the VM or in native code so use information
    // from the last Java frame.
    case _thread_blocked:
    case _thread_in_native:
    case _thread_in_vm:    
      if (thd->has_last_Java_frame()) {
        fill_call_trace_at_safepoint(thd, trace, depth);
      }
      break;
    case _thread_in_Java:  
      { frame fr;
        trace->num_frames = 0;
        // profile_last_Java_frame sets up the frame 'fr' and returns true;
        if (thd->profile_last_Java_frame(&fr)) {
          fill_call_trace_given_top(thd, trace, depth, fr);
        }
      }
      break;
    default: break;
  }
JVMPI_END


JVMPI_ENTRY(jlong, jvmpi::get_current_thread_cpu_time())
  return os::current_thread_cpu_time();
JVMPI_END


JVMPI_RAW_ENTRY(JVMPI_RawMonitor, jvmpi::raw_monitor_create(char *lock_name))
  RawMonitor * monitor = new RawMonitor(lock_name, PROF_RM_MAGIC);
  return (JVMPI_RawMonitor)monitor;
JVMPI_RAW_END


JVMPI_RAW_ENTRY(void, jvmpi::raw_monitor_enter(JVMPI_RawMonitor lock_id))
  RawMonitor *monitor = (RawMonitor *)lock_id;
  if (!(PROF_RM_CHECK(monitor))) {
      return;
  }
  if (TraceJVMPI) {
    tty->cr();
    tty->print_cr("JVMPI: raw_monitor_enter for thread id " INTPTR_FORMAT " lock_id = " INTPTR_FORMAT " ", THREAD, lock_id);
  }
  // JVMPI can't do proper transitions on RAW_ENTRY
  // Because VM thread posting events can deadlock. When
  // vmthread posting is fixed enable this code
  if (THREAD && THREAD->is_Java_thread()) {
#ifdef PROPER_TRANSITIONS
    ThreadInVMfromUnknown __tiv;
    {
      ThreadBlockInVM __tbivm((JavaThread*)THREAD);
      monitor->raw_enter(THREAD, true);
    }
#else

    /* Transition to thread_blocked without entering vm state          */
    /* This is really evil. Normally you can't undo _thread_blocked    */
    /* transitions like this because it would cause us to miss a       */
    /* safepoint but since the thread was already in _thread_in_native */
    /* the thread is not leaving a safepoint safe state and it will    */
    /* block when it tries to return from native. We can't safepoint   */
    /* block in here because we could deadlock the vmthread. Blech.    */

    JavaThread* jt = (JavaThread*) THREAD;
    JavaThreadState state = jt->thread_state();
    assert(state == _thread_in_native, "Must be _thread_in_native");
    // frame should already be walkable since we are in native
    assert(!jt->has_last_Java_frame() || jt->frame_anchor()->walkable(), "Must be walkable");
    jt->set_thread_state(_thread_blocked);

    monitor->raw_enter(THREAD, true);

    // restore state, still at a safepoint safe state
    jt->set_thread_state(state);
#endif /* PROPER_TRANSITIONS */
  } else {
    monitor->raw_enter(THREAD, true);
  }

JVMPI_RAW_END


JVMPI_RAW_ENTRY(void, jvmpi::raw_monitor_exit(JVMPI_RawMonitor lock_id))
  RawMonitor *monitor = (RawMonitor *)lock_id;
  if (!(PROF_RM_CHECK(monitor))) {
      return;
  }
  if (TraceJVMPI) {
    tty->cr();
    tty->print_cr("JVMPI: raw_monitor_exit for thread id " INTPTR_FORMAT " lock_id = " INTPTR_FORMAT " ", THREAD, lock_id);
  }
  // JVMPI can't do proper transitions on RAW_ENTRY
  // Because VM thread posting events can deadlock. When
  // vmthread posting is fixed enable this code
#ifdef PROPER_TRANSITIONS
  if (THREAD && THREAD->is_Java_thread()) {
    ThreadInVMfromUnknown __tiv;
    monitor->raw_exit(THREAD, true);
  } else {
    monitor->raw_exit(THREAD, true);
  }
#else
  // Doesn't block so we don't need to do anything special here
  monitor->raw_exit(THREAD, true);
#endif /* PROPER_TRANSITIONS */

JVMPI_RAW_END


JVMPI_RAW_ENTRY(void, jvmpi::raw_monitor_destroy(JVMPI_RawMonitor lock_id))
  RawMonitor *monitor = (RawMonitor *)lock_id;
  if (!(PROF_RM_CHECK(monitor))) {
      return;
  }
  if (TraceJVMPI) {
    tty->cr();
    tty->print_cr("JVMPI: raw_monitor_destroy for thread id " INTPTR_FORMAT " lock_id = " INTPTR_FORMAT " ", THREAD, lock_id);
  }
  // JVMPI can't do proper transitions on RAW_ENTRY
  // Because VM thread posting events can deadlock. When
  // vmthread posting is fixed enable this code
#ifdef PROPER_TRANSITIONS
  if (THREAD && THREAD->is_Java_thread()) {
    ThreadInVMfromUnknown __tiv;
    monitor->raw_exit(THREAD, true);
    monitor->raw_destroy();
  } else {
    monitor->raw_exit(THREAD, true);
    monitor->raw_destroy();
  }
#else
  // Doesn't block so we don't need to do anything special here
  monitor->raw_exit(THREAD, true);
  monitor->raw_destroy();
#endif /* PROPER_TRANSITIONS */

JVMPI_RAW_END


JVMPI_RAW_ENTRY(void, jvmpi::raw_monitor_wait(JVMPI_RawMonitor lock_id, jlong ms))
  RawMonitor *monitor = (RawMonitor *)lock_id;
  if (!(PROF_RM_CHECK(monitor))) {
      return;
  }
  if (TraceJVMPI) {
    tty->cr();
    tty->print_cr("JVMPI: raw_monitor_wait for thread id " INTPTR_FORMAT " lock_id = " INTPTR_FORMAT " ", THREAD, lock_id);
  }
  // JVMPI can't do proper transitions on RAW_ENTRY
  // Because VM thread posting events can deadlock. When
  // vmthread posting is fixed enable this code
  if (THREAD && THREAD->is_Java_thread()) {
#ifdef PROPER_TRANSITIONS
    ThreadInVMfromUnknown __tiv;
    {
      ThreadBlockInVM __tbivm((JavaThread*) THREAD);
      monitor->raw_wait(ms, true, THREAD);
    }
#else
    /* Transition to thread_blocked without entering vm state          */
    /* This is really evil. Normally you can't undo _thread_blocked    */
    /* transitions like this because it would cause us to miss a       */
    /* safepoint but since the thread was already in _thread_in_native */
    /* the thread is not leaving a safepoint safe state and it will    */
    /* block when it tries to return from native. We can't safepoint   */
    /* block in here because we could deadlock the vmthread. Blech.    */

    JavaThread* jt = (JavaThread*) THREAD;
    JavaThreadState state = jt->thread_state();
    assert(state == _thread_in_native, "Must be _thread_in_native");
    // frame should already be walkable since we are in native
    assert(!jt->has_last_Java_frame() || jt->frame_anchor()->walkable(), "Must be walkable");
    jt->set_thread_state(_thread_blocked);

    monitor->raw_wait(ms, true, THREAD);
    // restore state, still at a safepoint safe state
    jt->set_thread_state(state);

#endif /* PROPER_TRANSITIONS */
  } else {
    monitor->raw_wait(ms, true, THREAD);
  }

JVMPI_RAW_END


JVMPI_RAW_ENTRY(void, jvmpi::raw_monitor_notify_all(JVMPI_RawMonitor lock_id))
  RawMonitor *monitor = (RawMonitor *)lock_id;
  if (!(PROF_RM_CHECK(monitor))) {
      return;
  }
  if (TraceJVMPI) {
    tty->cr();
    tty->print_cr("JVMPI: raw_monitor_notify_all for thread id " INTPTR_FORMAT " lock_id = " INTPTR_FORMAT " ", THREAD, lock_id);
  }
  // JVMPI can't do proper transitions on RAW_ENTRY
  // Because VM thread posting events can deadlock. When
  // vmthread posting is fixed enable this code
#ifdef PROPER_TRANSITIONS
  if (THREAD && THREAD->is_Java_thread()) {
    ThreadInVMfromUnknown __tiv;
    monitor->raw_notifyAll(THREAD);
  } else {
    monitor->raw_notifyAll(THREAD);
  }
#else
  // Doesn't block so we don't need to do anything special here
  monitor->raw_notifyAll(THREAD);
#endif /* PROPER_TRANSITIONS */

JVMPI_RAW_END

// Use shared java_suspend.
JVMPI_ENTRY(void, jvmpi::suspend_thread(JNIEnv *env))
  if (env == NULL) return;
  JavaThread *java_thread = JavaThread::thread_from_jni_environment(env);
  if (java_thread == NULL) return;
  // the thread has not yet run or has exited (not on threads list)
  if (java_thread->threadObj() == NULL) return;
  if (java_lang_Thread::thread(java_thread->threadObj()) == NULL) return;

  // don't allow hidden thread suspend request.
  if (java_thread->is_hidden_from_external_view()) {
    return;
  }

  // Don't allow self-suspension, hprof agent expects to keep
  // running so as to process resumes of all threads.
  if (Thread::current() == (Thread *)java_thread) {
    return;
  }

  {
    MutexLockerEx ml(java_thread->SR_lock(), Mutex::_no_safepoint_check_flag);
    if (java_thread->is_external_suspend()) {
      // Don't allow nested external suspend requests. We can't return
      // an error from this interface so just ignore the problem.
      return;
    }
    if (java_thread->is_exiting()) { // thread is in the process of exiting
      return;
    }
    java_thread->set_external_suspend();
  }

  //
  // If a thread in state _thread_in_native is not immediately
  // suspended, then a blocked RawMonitorEnter() call may enter
  // the RawMonitor even if RawMonitorExit() is called after
  // SuspendThread() returns. java_suspend() will catch threads
  // in the process of exiting and will ignore them.
  //
  java_thread->java_suspend();

  // It would be nice to have the following assertion in all the time,
  // but it is possible for a racing resume request to have resumed
  // this thread right after we suspended it. Temporarily enable this
  // assertion if you are chasing a different kind of bug.
  //
  // assert(java_lang_Thread::thread(java_thread->threadObj()) == NULL ||
  //   java_thread->is_being_ext_suspended(), "thread is not suspended");
JVMPI_END

// Use shared java_suspend.
JVMPI_ENTRY(void, jvmpi::suspend_thread_list(jint reqCnt, JNIEnv **reqList, jint *results))

  if (reqCnt <= 0 || reqList == NULL || results == NULL) {
    // parameter problem so bail out
    return;
  }

  int needSafepoint = 0;  // > 0 if we need a safepoint

  for (int i = 0; i < reqCnt; i++) {
    if (reqList[i] == NULL) {
      results[i] = 10; // same as JVMDI_ERROR_INVALID_THREAD
      continue;
    }
    JavaThread *java_thread = JavaThread::thread_from_jni_environment(reqList[i]);
    if (java_thread == NULL) {
      results[i] = 10; // same as JVMDI_ERROR_INVALID_THREAD
      continue;
    }
    // the thread has not yet run or has exited (not on threads list)
    if (java_thread->threadObj() == NULL) {
      results[i] = 10; // same as JVMDI_ERROR_INVALID_THREAD
      continue;
    }
    if (java_lang_Thread::thread(java_thread->threadObj()) == NULL) {
      results[i] = 10; // same as JVMDI_ERROR_INVALID_THREAD
      continue;
    }
    // don't allow hidden thread suspend request.
    if (java_thread->is_hidden_from_external_view()) {
      results[i] = 0;  // indicate successful suspend
      continue;
    }

    // Don't allow self-suspension, hprof agent expects to keep
    // running so as to process resumes of all threads.
    if (Thread::current() == (Thread *)java_thread) {
      results[i] = 10; // same as JVMDI_ERROR_INVALID_THREAD
      continue;
    }

    {
      MutexLockerEx ml(java_thread->SR_lock(), Mutex::_no_safepoint_check_flag);
      if (java_thread->is_external_suspend()) {
        // Don't allow nested external suspend requests. We can't return
        // an error from this interface so just ignore the problem.
        results[i] = 14; // same as JVMDI_ERROR_THREAD_SUSPENDED
        continue;
      }
      if (java_thread->is_exiting()) { // thread is in the process of exiting
        results[i] = 10; // same as JVMDI_ERROR_INVALID_THREAD
        continue;
      }
      java_thread->set_external_suspend();
    }

    if (java_thread->thread_state() == _thread_in_native) {
      // We need to try and suspend native threads here. Threads in
      // other states will self-suspend on their next transition.
      // java_suspend() will catch threads in the process of exiting
      // and will ignore them.
      java_thread->java_suspend();
    } else {
      needSafepoint++;
    }

    results[i] = 0;  // indicate successful suspend
  }

  if (needSafepoint > 0) {
    VM_ForceSafepoint vfs;
    VMThread::execute(&vfs);
  }
JVMPI_END

// Use shared java_resume. Requires owning the Threads lock.
JVMPI_ENTRY(void, jvmpi::resume_thread(JNIEnv *env))
  JavaThread *java_thread;
  if ((env) && (java_thread = JavaThread::thread_from_jni_environment(env))) {
    MutexLocker ml(Threads_lock);

    // don't allow hidden thread resume request.
    if (java_thread->is_hidden_from_external_view()) {
      return;
    }

    java_thread->java_resume();
  }
JVMPI_END

// Use shared java_resume. Requires owning the Threads lock.
JVMPI_ENTRY(void, jvmpi::resume_thread_list(jint reqCnt, JNIEnv **reqList, jint *results))

  if (reqCnt <= 0 || reqList == NULL || results == NULL) {
    // parameter problem so bail out
    return;
  }

  for (int i = 0; i < reqCnt; i++) {
    if (reqList[i] == NULL) {
      results[i] = 10; // same as JVMDI_ERROR_INVALID_THREAD
      continue;
    }
    JavaThread *java_thread = JavaThread::thread_from_jni_environment(reqList[i]);
    if (java_thread == NULL) {
      results[i] = 10; // same as JVMDI_ERROR_INVALID_THREAD
      continue;
    }
    // don't allow hidden thread resume request.
    if (java_thread->is_hidden_from_external_view()) {
      results[i] = 0;  // indicate successful resume
      continue;
    }

    {
      MutexLocker ml(Threads_lock);
      java_thread->java_resume();
    }

    results[i] = 0;  // indicate successful resume
  }
JVMPI_END

// 2.0: redesign to match jvmdi. handle errors and more states
JVMPI_ENTRY(jint, jvmpi::get_thread_status(JNIEnv *env))
  jint res = JVMPI_THREAD_RUNNABLE;
  JavaThread *tp;
  { MutexLocker mu(Threads_lock);
    if ((env) && (tp = JavaThread::thread_from_jni_environment(env))) {
      JavaThreadState state;
      ThreadState t_state;
      if ((state = tp->thread_state()) && (tp->osthread()) && (t_state = tp->osthread()->get_state())) {

          if (state == _thread_blocked|| state == _thread_blocked_trans) {
              switch (t_state) {
                  case CONDVAR_WAIT: 
                  case OBJECT_WAIT: 
                      res = JVMPI_THREAD_CONDVAR_WAIT; 
                      break;
                  case MONITOR_WAIT: 
                      res = JVMPI_THREAD_MONITOR_WAIT; 
                      break;
                  case SLEEPING:
                  case ZOMBIE:
                  case RUNNABLE    : // fall through
                      res = JVMPI_THREAD_RUNNABLE;
                      break;
                  default:
                      break;
              }
           }
          if (tp->is_being_ext_suspended()) {
              // internal suspend doesn't count for this flag
              res = res | JVMPI_THREAD_SUSPENDED;
          }
          if (tp->osthread()->interrupted()) {
              res = res | JVMPI_THREAD_INTERRUPTED;
          }
      }
    }
  } // release Threads_lock
  return res;
JVMPI_END


// There is no provision in VM to check that; assume yes
// Do NOT call thread_is_running - this calls thr_getstate
// which only works if you have called thr_suspend.
JVMPI_ENTRY(jboolean, jvmpi::thread_has_run(JNIEnv *env))
  JavaThread* java_thread;
  if ((env) && (java_thread = JavaThread::thread_from_jni_environment(env)))  {
    return JNI_TRUE;
  } else {
    return JNI_FALSE;
  }
JVMPI_END


JVMPI_ENTRY(void, jvmpi::run_gc())
  Universe::heap()->collect(GCCause::_java_lang_system_gc);
JVMPI_END


JVMPI_ENTRY(void, jvmpi::profiler_exit(jint exit_code))
  vm_exit(exit_code /*user_exit == true*/); 
  ShouldNotReachHere();
JVMPI_END


static void jvmpi_daemon_thread_entry(JavaThread* thread, TRAPS) {
  assert(thread->is_jvmpi_daemon_thread(), "wrong thread");
  JVMPIDaemonThread* daemon_thread = (JVMPIDaemonThread*)thread;

  // ThreadToNativeFromVM takes care of changing thread_state, so safepoint code knows that
  // we have left the VM
  { JavaThread* thread = (JavaThread*) THREAD;
    ThreadToNativeFromVM ttn(thread);
    HandleMark hm(thread);

    daemon_thread->function()(NULL);
  }
}


JVMPI_ENTRY(jint, jvmpi::create_system_thread(char *name, jint priority, JVMPIDaemonFunction f))
  const int invalid_res = JNI_ERR;
  klassOop k = SystemDictionary::resolve_or_fail(vmSymbolHandles::java_lang_Thread(), true, CHECK_(invalid_res));
  instanceKlassHandle klass (THREAD, k);
  instanceHandle thread_oop = klass->allocate_instance_handle(CHECK_(invalid_res));
  Handle string = java_lang_String::create_from_str(name, CHECK_(invalid_res));    

  // Initialize thread_oop to put it into the system threadGroup    
  Handle thread_group (THREAD, Universe::system_thread_group());
  JavaValue result(T_VOID);
  JavaCalls::call_special(&result, thread_oop, 
                         klass, 
                         vmSymbolHandles::object_initializer_name(), 
                         vmSymbolHandles::threadgroup_string_void_signature(), 
                         thread_group, 
                         string, 
                         CHECK_(invalid_res));  
  
  { MutexLocker mu(Threads_lock);
    JVMPIDaemonThread* daemon_thread = new JVMPIDaemonThread(&jvmpi_daemon_thread_entry, f);

    // At this point it may be possible that no osthread was created for the
    // JavaThread due to lack of memory.
    if (daemon_thread == NULL || daemon_thread->osthread() == NULL) {
      if (daemon_thread) delete daemon_thread;
      return JNI_ERR;
    }
   
    ThreadPriority thread_priority = NoPriority;
    switch (priority) {
      case JVMPI_MINIMUM_PRIORITY: thread_priority = MinPriority ; break;
      case JVMPI_MAXIMUM_PRIORITY: thread_priority = MaxPriority ; break;
      case JVMPI_NORMAL_PRIORITY : thread_priority = NormPriority; break;
      default: ShouldNotReachHere();
    }

    java_lang_Thread::set_thread(thread_oop(), daemon_thread);      
    java_lang_Thread::set_priority(thread_oop(), thread_priority);
    java_lang_Thread::set_daemon(thread_oop());

    daemon_thread->set_threadObj(thread_oop());
    Threads::add(daemon_thread);  
    Thread::start(daemon_thread);

  } // Release Threads_lock before calling up to agent code
  // post_thread_start_event called from "run"
  
  return JNI_OK;
JVMPI_END


JVMPI_ENTRY(jint, jvmpi::request_event(jint event_type, void *arg))
  switch (event_type) {
    case JVMPI_EVENT_OBJECT_ALLOC:
      post_object_alloc_event((oop)arg, ((oop)arg)->size() * HeapWordSize,
			      Universe::heap()->addr_to_arena_id(arg),
			      JVMPI_REQUESTED_EVENT);
      return JVMPI_SUCCESS;
    case JVMPI_EVENT_THREAD_START:
      post_thread_start_event(java_lang_Thread::thread((oop)arg), JVMPI_REQUESTED_EVENT);
      return JVMPI_SUCCESS;
    case JVMPI_EVENT_CLASS_LOAD:
      post_class_load_event((oop)arg, JVMPI_REQUESTED_EVENT);
      return JVMPI_SUCCESS;
    case JVMPI_EVENT_OBJECT_DUMP:
      post_object_dump_event((oop)arg, JVMPI_REQUESTED_EVENT);
      return JVMPI_SUCCESS;
    case JVMPI_EVENT_HEAP_DUMP: {
      int heap_dump_level;

      if (arg == NULL) {
        heap_dump_level = JVMPI_DUMP_LEVEL_2;
      } else {
        heap_dump_level = ((JVMPI_HeapDumpArg*)arg)->heap_dump_level;
      }

      post_heap_dump_event_in_safepoint(heap_dump_level, JVMPI_REQUESTED_EVENT);
      return JVMPI_SUCCESS;
    }
    case JVMPI_EVENT_MONITOR_DUMP:
      post_monitor_dump_event_in_safepoint(JVMPI_REQUESTED_EVENT);
      return JVMPI_SUCCESS;
    default:
      return JVMPI_NOT_AVAILABLE;
  }
JVMPI_END


// Using JVMPI_RAW_ENTRY() to allow this API to be called from a
// SIGPROF signal handler. ThreadInVMFromUnknown's use of a
// HandleMarkCleaner will cleanup unexpected Handles when called
// from a signal handler.
JVMPI_RAW_ENTRY(void, jvmpi::set_thread_local_storage(JNIEnv *env, void *ptr))
  if (env != NULL) {
    JavaThread* jt = JavaThread::thread_from_jni_environment(env);
    if (jt != NULL) {
      jt->set_jvmpi_data(ptr);
    }
  }
JVMPI_END


// See set_thread_local_storage comment above.
JVMPI_RAW_ENTRY(void*, jvmpi::get_thread_local_storage(JNIEnv *env))
  if (env == NULL) return NULL;
  JavaThread* jt = JavaThread::thread_from_jni_environment(env);
  if (jt == NULL) return NULL;
  return jt->jvmpi_data();
JVMPI_END


JVMPI_ENTRY(jobjectID, jvmpi::get_thread_object(JNIEnv *env))
  if (env == NULL) return NULL;
  return (jobjectID) JavaThread::thread_from_jni_environment(env)->threadObj();
JVMPI_END


JVMPI_ENTRY(jobjectID, jvmpi::get_method_class(jmethodID mid))
  return (jobjectID) Klass::cast(jniIdSupport::to_klass_oop(mid))->java_mirror();
JVMPI_END


JVMPI_ENTRY(jobject, jvmpi::jobjectID_2_jobject(jobjectID jid))
  assert(GC_locker::is_active(), "jobjectID_2_jobject may be called only with disabled GC");
  Thread* thd = Thread::current();
  assert(thd->is_Java_thread(), "call to jobjectID_2_jobject can only happen in a Java thread");

  JavaThread* jthread = (JavaThread*)thd;

  JNIEnv* env = jthread->jni_environment();
  return JNIHandles::make_local(env, (oop)jid);
JVMPI_END


JVMPI_ENTRY(jobjectID, jvmpi::jobject_2_jobjectID(jobject jobj))
  assert(GC_locker::is_active(), "jobject_2_jobjectID may be called only with disabled GC");
  return (jobjectID)JNIHandles::resolve(jobj);
JVMPI_END

