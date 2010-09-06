#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)java.cpp	1.190 04/06/03 09:48:46 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_java.cpp.incl"

#ifndef PRODUCT

#ifndef CORE

// Statistics printing (method invocation histogram)

GrowableArray<methodOop>* collected_invoked_methods;

void collect_invoked_methods(methodOop m) {
  if (m->invocation_count() + m->compiled_invocation_count() >= 1 ) {
    collected_invoked_methods->push(m);
  }
}


GrowableArray<methodOop>* collected_profiled_methods;

void collect_profiled_methods(methodOop m) {
  methodHandle mh(Thread::current(), m);
  if ((m->method_data() != NULL) && 
      (PrintMethodData || CompilerOracle::should_print(mh))) {
    collected_profiled_methods->push(m);
  }
}


int compare_methods(methodOop* a, methodOop* b) {
  // %%% there can be 32-bit overflow here
  return ((*b)->invocation_count() + (*b)->compiled_invocation_count())
       - ((*a)->invocation_count() + (*a)->compiled_invocation_count());
}


void print_method_invocation_histogram() {
  ResourceMark rm;
  HandleMark hm;
  collected_invoked_methods = new GrowableArray<methodOop>(1024);
  SystemDictionary::methods_do(collect_invoked_methods);
  collected_invoked_methods->sort(&compare_methods);
  //
  tty->cr();
  tty->print_cr("Histogram Over MethodOop Invocation Counters (cutoff = %d):", MethodHistogramCutoff);
  tty->cr();
  tty->print_cr("____Count_(I+C)____Method________________________Module_________________");
  unsigned total = 0, int_total = 0, comp_total = 0, static_total = 0, final_total = 0, 
      synch_total = 0, nativ_total = 0, acces_total = 0;
  for (int index = 0; index < collected_invoked_methods->length(); index++) {
    methodOop m = collected_invoked_methods->at(index);
    int c = m->invocation_count() + m->compiled_invocation_count();
    if (c >= MethodHistogramCutoff) m->print_invocation_count();
    int_total  += m->invocation_count();
    comp_total += m->compiled_invocation_count();
    if (m->is_final())        final_total  += c;
    if (m->is_static())       static_total += c;
    if (m->is_synchronized()) synch_total  += c;
    if (m->is_native())       nativ_total  += c;
    if (m->is_accessor())     acces_total  += c;
  }
  tty->cr();
  total = int_total + comp_total;
  tty->print_cr("Invocations summary:");
  tty->print_cr("\t%9d (%4.1f%%) interpreted",  int_total,    100.0 * int_total    / total);
  tty->print_cr("\t%9d (%4.1f%%) compiled",     comp_total,   100.0 * comp_total   / total);
  tty->print_cr("\t%9d (100%%)  total",         total);
  tty->print_cr("\t%9d (%4.1f%%) synchronized", synch_total,  100.0 * synch_total  / total);
  tty->print_cr("\t%9d (%4.1f%%) final",        final_total,  100.0 * final_total  / total);
  tty->print_cr("\t%9d (%4.1f%%) static",       static_total, 100.0 * static_total / total);
  tty->print_cr("\t%9d (%4.1f%%) native",       nativ_total,  100.0 * nativ_total  / total);
  tty->print_cr("\t%9d (%4.1f%%) accessor",     acces_total,  100.0 * acces_total  / total);
  tty->cr();
  COMPILER2_ONLY(OptoRuntime::print_call_statistics(comp_total);)
}

void print_method_profiling_data() {
  ResourceMark rm;
  HandleMark hm;
  collected_profiled_methods = new GrowableArray<methodOop>(1024);
  SystemDictionary::methods_do(collect_profiled_methods);
  collected_profiled_methods->sort(&compare_methods);
  
  int count = collected_profiled_methods->length();
  if (count > 0) {
    for (int index = 0; index < count; index++) {
      methodOop m = collected_profiled_methods->at(index);
      tty->print_cr("------------------------------------------------------------------------");
      //m->print_name(tty);
      m->print_invocation_count();
      tty->cr();
      m->print_codes();
    }
    tty->print_cr("------------------------------------------------------------------------");
  }
}
# endif // CORE

void print_bytecode_count() {
  if (CountBytecodes || TraceBytecodes || StopInterpreterAt) {
    tty->print_cr("[BytecodeCounter::counter_value = %d]", BytecodeCounter::counter_value());
  }
}

AllocStats alloc_stats;



// General statistics printing (profiling ...)

void print_statistics() {
  
#ifdef ASSERT

  if (CountJNICalls) {
    extern Histogram *JNIHistogram;
    JNIHistogram->print();
  }

  if (CountJVMCalls) {
    extern Histogram *JVMHistogram;
    JVMHistogram->print();
  }

#endif

  if (MemProfiling) {
    MemProfiler::disengage();
  }

#ifndef CORE

  if (CITime) {
    CompileBroker::print_times();
  }

#ifdef COMPILER1
  if ((PrintC1Statistics || LogVMOutput || LogCompilation) && UseCompiler) {
    FlagSetting fs(DisplayVMOutput, DisplayVMOutput && PrintC1Statistics);
    Runtime1::print_statistics();
    Deoptimization::print_statistics();
  }
#endif

#ifdef COMPILER2
  if ((PrintOptoStatistics || LogVMOutput || LogCompilation) && UseCompiler) {
    FlagSetting fs(DisplayVMOutput, DisplayVMOutput && PrintOptoStatistics);
    Parse::print_statistics();
    PhaseCCP::print_statistics();
    PhaseRegAlloc::print_statistics();
    Scheduling::print_statistics();
    Deoptimization::print_statistics();
    OptoRuntime::print_statistics();
    PhasePeephole::print_statistics();
    PhaseIdealLoop::print_statistics();

    os::print_statistics();
  }
  if (TimeLivenessAnalysis) {
    MethodLiveness::print_times();
  }
#ifdef ASSERT
  if (CollectIndexSetStatistics) {
    IndexSet::print_statistics();
  }
#endif // ASSERT
#endif // COMPILER2
  if (CountCompiledCalls) {
    print_method_invocation_histogram();
  }
  if (ProfileInterpreter) {
    print_method_profiling_data();
  }
  if (TimeCompiler) {
    COMPILER2_ONLY(Compile::print_timers();)
  }
  if (TimeCompilationPolicy) {
    CompilationPolicy::policy()->print_time();
  }
#endif // CORE
  if (TimeOopMap) {
    GenerateOopMap::print_time();
  }  
  if (ProfilerCheckIntervals) {
    PeriodicTask::print_intervals();
  }
  if (PrintSymbolTableSizeHistogram) {
    SymbolTable::print_histogram();
  }
  if (CountBytecodes || TraceBytecodes || StopInterpreterAt) {
    BytecodeCounter::print();
  }  
  if (PrintBytecodePairHistogram) {
    BytecodePairHistogram::print();
  }

#ifndef CORE
  if (PrintCodeCache) {
    MutexLockerEx mu(CodeCache_lock, Mutex::_no_safepoint_check_flag);
    CodeCache::print();
  }

  if (PrintCodeCache2) {
    MutexLockerEx mu(CodeCache_lock, Mutex::_no_safepoint_check_flag);
    CodeCache::print_internals();
  }
#endif // CORE

  if (PrintClassStatistics) {
    SystemDictionary::print_class_statistics();
  }
  if (PrintMethodStatistics) {
    SystemDictionary::print_method_statistics();
  }

  if (PrintVtableStats) {
    klassVtable::print_statistics();
    klassItable::print_statistics();
  }
  if (VerifyOops) {
    tty->print_cr("+VerifyOops count: %d", StubRoutines::verify_oop_count());
  }

  print_bytecode_count();
  if (WizardMode) {
    tty->print("allocation stats: ");
    alloc_stats.print();
    tty->cr();
  }

  if (PrintSystemDictionaryAtExit) {
    SystemDictionary::print();
  }

#ifdef ENABLE_ZAP_DEAD_LOCALS
#ifdef COMPILER2
  if (ZapDeadCompiledLocals) {
    tty->print_cr("Compile::CompiledZap_count = %d", Compile::CompiledZap_count);
    tty->print_cr("OptoRuntime::ZapDeadCompiledLocals_count = %d", OptoRuntime::ZapDeadCompiledLocals_count);
  }
#endif // COMPILER2
#endif // ENABLE_ZAP_DEAD_LOCALS
}

#else // PRODUCT MODE STATISTICS

void print_statistics() {

#ifndef CORE

  if (CITime) {
    CompileBroker::print_times();
  }

#endif

}

#endif


// Helper class for registering on_exit calls through JVM_OnExit

extern "C" {
    typedef void (*__exit_proc)(void);
}

class ExitProc : public CHeapObj {
 private:
  __exit_proc _proc;
  // void (*_proc)(void);
  ExitProc* _next;
 public:
  // ExitProc(void (*proc)(void)) {
  ExitProc(__exit_proc proc) {
    _proc = proc;
    _next = NULL;
  }
  void evaluate()               { _proc(); }
  ExitProc* next() const        { return _next; }
  void set_next(ExitProc* next) { _next = next; }
};


// Linked list of registered on_exit procedures

static ExitProc* exit_procs = NULL;


extern "C" {
  void register_on_exit_function(void (*func)(void)) {
    ExitProc *entry = new ExitProc(func);
    // Classic vm does not throw an exception in case the allocation failed, 
    if (entry != NULL) {
      entry->set_next(exit_procs);
      exit_procs = entry;
    }
  }
}

// Note: before_exit() can be executed only once, if more than one threads
//       are trying to shutdown the VM at the same time, only one thread
//       can run before_exit() and all other threads must wait.
void before_exit(JavaThread * thread) {
  #define BEFORE_EXIT_NOT_RUN 0
  #define BEFORE_EXIT_RUNNING 1
  #define BEFORE_EXIT_DONE    2
  static jint volatile _before_exit_status = BEFORE_EXIT_NOT_RUN;

  // Note: don't use a Mutex to guard the entire before_exit(), as JVMPI and
  // JVMTI post_thread_end_event and post_vm_death_event will run native code. 
  // A CAS or OSMutex would work just fine but then we need to manipulate 
  // thread state for Safepoint. Here we use Monitor wait() and notify_all() 
  // for synchronization.
  { MutexLocker ml(BeforeExit_lock);
    switch (_before_exit_status) {
    case BEFORE_EXIT_NOT_RUN:
      _before_exit_status = BEFORE_EXIT_RUNNING;
      break;
    case BEFORE_EXIT_RUNNING:
      while (_before_exit_status == BEFORE_EXIT_RUNNING) {
        BeforeExit_lock->wait();
      }
      assert(_before_exit_status == BEFORE_EXIT_DONE, "invalid state");
      return;
    case BEFORE_EXIT_DONE:
      return;
    }
  }

  // The only difference between this and Win32's _onexit procs is that 
  // this version is invoked before any threads get killed.
  ExitProc* current = exit_procs;
  while (current != NULL) {
    ExitProc* next = current->next();
    current->evaluate();
    delete current;
    current = next;
  }
 
  // Hang forever on exit if we're reporting an error.
  if (ShowMessageBoxOnError && is_error_reported()) {
    os::infinite_sleep();
  }

  // Terminate watcher thread - must before disenrolling any periodic task
  WatcherThread::stop();

  // Print statistics gathered (profiling ...)
  if (Arguments::has_profile()) {    
    FlatProfiler::disengage();
    FlatProfiler::print(10);
  }

  // shut down the StatSampler task
  StatSampler::disengage();
  StatSampler::destroy();

  // stop CMS threads
  if (UseConcMarkSweepGC) {
    ConcurrentMarkSweepThread::stop();
  }

  if (Arguments::has_alloc_profile()) {
    HandleMark hm;
    // Do one last collection to enumerate all the objects 
    // allocated since the last one.
    Universe::heap()->collect(GCCause::_allocation_profiler);
    AllocationProfiler::disengage();
    AllocationProfiler::print(0);
  }

  if (PrintBytecodeHistogram) {
    BytecodeHistogram::print();
  }

  if (JvmtiExport::should_post_thread_life()) {
    JvmtiExport::post_thread_end(thread);
  }
  // Always call even when there are not JVMTI environments yet, since environments
  // may be attached late and JVMTI must track phases of VM execution
  JvmtiExport::post_vm_death();

  if (jvmpi::enabled()) {
    // Notify the profiler this thread is about to die, so it can stop
    // querying us - we don't want to get caught in a suspend-query-resume
    // operation when we disengage JVMPI.
    if (jvmpi::is_event_enabled(JVMPI_EVENT_THREAD_END)) {
      jvmpi::post_thread_end_event(thread);
    }

    if (jvmpi::is_event_enabled(JVMPI_EVENT_JVM_SHUT_DOWN)) {
      jvmpi::post_vm_death_event();
    }

    // Do not accept new JVMPI requests or post new events
    jvmpi::disengage();
  }

  // Terminate the signal thread
  // Note: we don't wait until it actually dies. Do this after JVMPI is 
  // disengaged, see 4513141
  os::terminate_signal_thread();

  print_statistics();
  if (TraceGen0Time) {
    if (UseParallelGC) {
      double time = PSScavenge::accumulated_time()->seconds();
      tty->print_cr("[Accumulated GC generation 0 time %3.7f secs]", time);
    } else {
      GenCollectedHeap* gch = GenCollectedHeap::heap();
      double time = gch->get_gen(0)->stat_record()->accumulated_time.seconds();
      tty->print_cr("[Accumulated GC generation 0 time %3.7f secs, %d GC's, avg GC time %3.7f]", 
        time, gch->get_gen(0)->stat_record()->invocations, 
        gch->get_gen(0)->stat_record()->accumulated_time.seconds() /
        gch->get_gen(0)->stat_record()->invocations);
    }
  }
  if (TraceGen1Time) {
    if (UseParallelGC) {
      double time = PSMarkSweep::accumulated_time()->seconds();
      tty->print_cr("[Accumulated GC generation 1 time %3.7f secs]", time);
    } else {
      GenCollectedHeap* gch = GenCollectedHeap::heap();
      double time = gch->get_gen(1)->stat_record()->accumulated_time.seconds();
      tty->print_cr("[Accumulated GC generation 1 time %3.7f secs, %d GC's, avg GC time %3.7f]", 
        time, gch->get_gen(1)->stat_record()->invocations, 
        gch->get_gen(1)->stat_record()->accumulated_time.seconds() /
        gch->get_gen(1)->stat_record()->invocations);
    }
  }

  VTune::exit();

  { MutexLocker ml(BeforeExit_lock);
    _before_exit_status = BEFORE_EXIT_DONE;
    BeforeExit_lock->notify_all();
  }

  #undef BEFORE_EXIT_NOT_RUN
  #undef BEFORE_EXIT_RUNNING
  #undef BEFORE_EXIT_DONE
}

void vm_exit(int code) {  
  Thread* thread = ThreadLocalStorage::thread_index() == -1 ? NULL
    : ThreadLocalStorage::get_thread_slow();
  if (thread == NULL) {
    // we have serious problems -- just exit
    ::exit(code);
  }

  if (VMThread::vm_thread() != NULL) {
    // Fire off a VM_Exit operation to bring VM to a safepoint and exit
    VM_Exit op(code);
    if (thread->is_Java_thread())
      ((JavaThread*)thread)->set_thread_state(_thread_in_vm);
    VMThread::execute(&op);
    // should never reach here; but in case something wrong with VM Thread.
    ::exit(code);
  } else {
    // VM thread is gone, just exit
    ::exit(code);
  }
  ShouldNotReachHere();
}

void vm_perform_shutdown_actions() {
  // Warning: do not call 'exit_globals()' here. All threads are still running.
  // Calling 'exit_globals()' will disable thread-local-storage and cause all
  // kinds of assertions to trigger in debug mode.
  if (is_init_completed()) {
    Thread* thread = Thread::current();
    if (thread->is_Java_thread()) {
      // We are leaving the VM, set state to native (in case any OS exit
      // handlers call back to the VM)
      JavaThread* jt = (JavaThread*)thread;
      // Must always be walkable or have no last_Java_frame when in
      // thread_in_native
      jt->frame_anchor()->make_walkable(jt);
      jt->set_thread_state(_thread_in_native);
    }
  }
}

void vm_shutdown()
{
  vm_perform_shutdown_actions();
  os::shutdown();
}

void vm_abort() {
  vm_perform_shutdown_actions();
  os::abort(PRODUCT_ONLY(false));
  ShouldNotReachHere();
}

void vm_notify_during_shutdown(const char* error, const char* message) {
  if (error != NULL) { 
    tty->print_cr("Error occurred during initialization of VM");
    tty->print("%s", error);
    if (message != NULL) {
      tty->print_cr(": %s", message);
    }
    else {
      tty->cr();
    }
  }
  if (ShowMessageBoxOnError && WizardMode) {
    fatal("Error occurred during initialization of VM");
  }
}

void vm_exit_during_initialization(Handle exception) {
  tty->print_cr("Error occurred during initialization of VM");
  java_lang_Throwable::print(exception, tty);
  tty->cr();
  vm_notify_during_shutdown(NULL, NULL);
  vm_abort();
}

void vm_exit_during_initialization(symbolHandle ex, const char* message) {
  ResourceMark rm;
  vm_notify_during_shutdown(ex->as_C_string(), message);
  vm_abort();
}

void vm_exit_during_initialization(const char* error, const char* message) {
  vm_notify_during_shutdown(error, message);
  vm_abort();
}

void vm_shutdown_during_initialization(const char* error, const char* message) {
  vm_notify_during_shutdown(error, message);
  vm_shutdown();
}


// Used by vm_exit_out_of_memory to detect recursion.
static jint _exiting_out_of_mem = 0;

void vm_exit_out_of_memory(size_t size, const char* name) {
  // We try to gather additional information for the first out of memory 
  // error only; gathering additional data might cause an allocation and a 
  // recursive out_of_memory condition.
  //
  // We use print_raw to avoid using more stack space (we know we're
  // in a low resource situation).
  // This buffer needs to be large enough to hold a size_t converted
  // to ascii, plus one for the null.
  char buffer[24];

  const jint exiting = 1;
  // If we succeed in changing the value, we're the first one in.
  bool first_time_here = Atomic::xchg(exiting, &_exiting_out_of_mem) != exiting;

  // We only print data for the first thread in.
  if (first_time_here) {
    tty->print_raw("\nException ");
    Thread* thread = ThreadLocalStorage::get_thread_slow();
    if (thread != NULL && 
        thread->is_Java_thread() && 
        ((JavaThread*) thread)->threadObj() != NULL) {
      ResourceMark rm;
      tty->print_raw("in thread \"");
      tty->print_raw(((JavaThread*) thread)->get_thread_name());
      tty->print_raw("\" ");
    }

    tty->print_raw("java.lang.OutOfMemoryError: requested ");
    sprintf(buffer,"%d",size);
    tty->print_raw(buffer);
    tty->print_raw(" bytes");
    if (name != NULL) {
      tty->print_raw(" for ");
      tty->print_raw(name);
    }
    tty->print_raw(". Out of swap space?");
    tty->cr();
  }

  vm_abort();
}
