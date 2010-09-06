#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)vm_operations.cpp	1.159 04/04/08 14:16:20 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_vm_operations.cpp.incl"


void VM_Operation::set_calling_thread(Thread* thread, ThreadPriority priority) { 
  _calling_thread = thread; 
  assert(MinPriority <= priority && priority <= MaxPriority, "sanity check");
  _priority = priority;
}  


void VM_Operation::evaluate() {
  ResourceMark rm;
  if (TraceVMOperation) { 
    tty->print("["); 
    NOT_PRODUCT(print();)
  }
  doit();
  if (TraceVMOperation) { 
    tty->print_cr("]"); 
  }
}

// Called by fatal error handler.
void VM_Operation::print_on_error(outputStream* st) const {
  st->print("VM_Operation (" PTR_FORMAT "): ", this);
  st->print("%s", name());

  const char* mode;
  switch(evaluation_mode()) {
    case _safepoint      : mode = "safepoint";       break;
    case _no_safepoint   : mode = "no safepoint";    break;
    case _concurrent     : mode = "concurrent";      break;
    case _async_safepoint: mode = "async safepoint"; break;
    default              : mode = "unknown";         break;
  }
  st->print(", mode: %s", mode);

  if (calling_thread()) {
    st->print(", requested by thread " PTR_FORMAT, calling_thread());
  }
}

void VM_GC_Operation::acquire_pending_list_lock() {
  // we may enter this with pending exception set
  _notify_ref_lock = false;

  instanceRefKlass::acquire_pending_list_lock(&_pending_list_basic_lock);
}


void VM_GC_Operation::release_and_notify_pending_list_lock() {

  instanceRefKlass::release_and_notify_pending_list_lock(_notify_ref_lock,
							 &_pending_list_basic_lock);
}

// Allocations may fail in several threads at about the same time,
// resulting in multiple gc requests.  We only want to do one of them.

bool VM_GC_Operation::gc_count_changed() const { 
  return (_gc_count_before != Universe::heap()->total_collections());
}

bool VM_GC_Operation::doit_prologue() {
  assert(Thread::current()->is_Java_thread(), "just checking");

  acquire_pending_list_lock();
  // If the GC count has changed someone beat us to the collection
  // Get the Heap_lock after the pending_list_lock.
  Heap_lock->lock();
  // Check invocations
  if (gc_count_changed()) {
    // skip collection
    Heap_lock->unlock();
    release_and_notify_pending_list_lock();
    _prologue_succeeded = false;
  } else {
    _prologue_succeeded = true;
  }
  return _prologue_succeeded;
}


void VM_GC_Operation::doit_epilogue() {
  assert(Thread::current()->is_Java_thread(), "just checking");
  // Release the Heap_lock first.
  Heap_lock->unlock();
  release_and_notify_pending_list_lock();
}

void VM_GenCollectForAllocation::doit() {
  JvmtiGCForAllocationMarker jgcm;

  GenCollectedHeap* gch = GenCollectedHeap::heap();
  _res = gch->satisfy_failed_allocation(_size, _large_noref, _tlab, &_notify_ref_lock);
  assert(gch->is_in_or_null(_res), "result not in heap");
} 			

void VM_GenCollectFull::doit() {
  JvmtiGCFullMarker jgcm;

  GenCollectedHeap* gch = GenCollectedHeap::heap();
  gch->do_full_collection(gch->must_clear_all_soft_refs(),
			  _max_level,
                          &_notify_ref_lock);
} 			

// The following methods are used by the parallel scavenge collector
VM_ParallelGCFailedAllocation::VM_ParallelGCFailedAllocation(size_t size, bool is_noref, bool is_tlab, unsigned int gc_count) :
  VM_GC_Operation(gc_count),
  _size(size),
  _is_noref(is_noref),
  _is_tlab(is_tlab),
  _result(NULL)
{
}

void VM_ParallelGCFailedAllocation::doit() {
  JvmtiGCForAllocationMarker jgcm;

  ParallelScavengeHeap* heap = (ParallelScavengeHeap*)Universe::heap();
  assert(heap->kind() == CollectedHeap::ParallelScavengeHeap, "must be a ParallelScavengeHeap");

  _result = heap->failed_mem_allocate(&_notify_ref_lock, _size, _is_noref, _is_tlab);
}

VM_ParallelGCFailedPermanentAllocation::VM_ParallelGCFailedPermanentAllocation(size_t size, unsigned int gc_count) :
  VM_GC_Operation(gc_count),
  _size(size),
  _result(NULL)
{
}

void VM_ParallelGCFailedPermanentAllocation::doit() {
  JvmtiGCFullMarker jgcm;

  ParallelScavengeHeap* heap = (ParallelScavengeHeap*)Universe::heap();
  assert(heap->kind() == CollectedHeap::ParallelScavengeHeap, "must be a ParallelScavengeHeap");

  _result = heap->failed_permanent_mem_allocate(&_notify_ref_lock, _size);
}

// Only used for System.gc() calls
VM_ParallelGCSystemGC::VM_ParallelGCSystemGC(unsigned int gc_count) :
  VM_GC_Operation(gc_count)
{
}

void VM_ParallelGCSystemGC::doit() {
  JvmtiGCFullMarker jgcm;

  ParallelScavengeHeap* heap = (ParallelScavengeHeap*)Universe::heap();
  assert(heap->kind() == CollectedHeap::ParallelScavengeHeap, 
    "must be a ParallelScavengeHeap");

  PSMarkSweep::invoke(&_notify_ref_lock, false);
}

void VM_ThreadStop::doit() {
  assert(SafepointSynchronize::is_at_safepoint(), "must be at a safepoint");
  JavaThread* target = java_lang_Thread::thread(target_thread());  
  // Note that this now allows multiple ThreadDeath exceptions to be
  // thrown at a thread.
  if (target != NULL) {
    // the thread has run and is not already in the process of exiting
    target->send_thread_stop(throwable());
  }  
}

#ifndef CORE

void VM_Deoptimize::doit() {
  // We do not want any GCs to happen while we are in the middle of this VM operation
  ResourceMark rm;
  DeoptimizationMarker dm;

  // Deoptimize all activations depending on marked nmethods  
  Deoptimization::deoptimize_dependents();

  // Make the dependent methods zombies
  if (SafepointPolling)
    // Work around safepoint polling bug
    CodeCache::make_marked_nmethods_not_entrant();
  else
    CodeCache::make_marked_nmethods_zombies();

}


VM_DeoptimizeFrame::VM_DeoptimizeFrame(JavaThread* thread, intptr_t* id) {
  _thread = thread;
  _id     = id;
}


void VM_DeoptimizeFrame::doit() {
  Deoptimization::deoptimize_frame(_thread, _id);
}


#ifndef PRODUCT

void VM_DeoptimizeAll::doit() {
  DeoptimizationMarker dm;
  // deoptimize all java threads in the system
  if (DeoptimizeALot) {
    for (JavaThread* thread = Threads::first(); thread != NULL; thread = thread->next()) {
      if (thread->has_last_Java_frame()) {
	thread->deoptimize();
      }
    }
  } else if (DeoptimizeRandom) {

    // Deoptimize some selected threads and frames
    int tnum = os::random() & 0x3;
    int fnum =  os::random() & 0x3;
    int tcount = 0;
    for (JavaThread* thread = Threads::first(); thread != NULL; thread = thread->next()) {
      if (thread->has_last_Java_frame()) {
	if (tcount++ == tnum)  {
	tcount = 0;
	  int fcount = 0;
	  // Deoptimize some selected frames.
	  for(StackFrameStream fst(thread, false); !fst.is_done(); fst.next()) {
	    if (fst.current()->can_be_deoptimized()) {
	      if (fcount++ == fnum) {
		fcount = 0;
		Deoptimization::deoptimize(thread, *fst.current());
	      }
	    }
	  }
	}
      }
    }
  }
}


void VM_ZombieAll::doit() {
  JavaThread *thread = (JavaThread *)calling_thread();
  assert(thread->is_Java_thread(), "must be a Java thread");
  thread->make_zombies();
}

#endif // !PRODUCT
#endif // !CORE


#ifndef PRODUCT
void VM_Verify::doit() {
  Universe::verify();
}
#endif // !PRODUCT


void VM_PrintThreads::doit() {
  Threads::print(true, false);
}

VM_FindDeadlocks::VM_FindDeadlocks(bool save_result, bool print_to_tty) {
  if (save_result) {
    _deadlock_threads = new (ResourceObj::C_HEAP) GrowableArray<JavaThread*>(_initial_array_size, true);
  } else {
    _deadlock_threads = NULL;
  }
  _print_to_tty = print_to_tty;
}

VM_FindDeadlocks::~VM_FindDeadlocks() {
  if (_deadlock_threads != NULL) {
    _deadlock_threads->clear_and_deallocate();
    FreeHeap(_deadlock_threads);
  }
}

void VM_FindDeadlocks::doit() {
  Threads::find_deadlocks(_deadlock_threads, _print_to_tty);
}

VM_GC_HeapInspection::VM_GC_HeapInspection() :
  VM_GC_Operation(Universe::heap()->total_collections()) {
}

void VM_GC_HeapInspection::doit() {
  if (Universe::heap()->kind() == CollectedHeap::GenCollectedHeap) {
    HandleMark hm;
    GenCollectedHeap* gch = GenCollectedHeap::heap();
    gch->do_full_collection(false /* clear_all_soft_refs */,
                            gch->n_gens() - 1,
                            &_notify_ref_lock);
    HeapInspection::heap_inspection();
  }
}

VM_ThreadDump::VM_ThreadDump(GrowableArray<instanceHandle>* threads, 
                             ThreadSnapshot** snapshots,
                             int num_threads, 
                             int max_depth, 
                             ThreadDumpResult* result,
                             Thread* req_thread) {
  assert(threads != NULL && num_threads == threads->length(), "Check threads input argument");
  _threads = threads;
  _snapshots = snapshots;
  _num_threads = num_threads;
  _max_depth = max_depth;
  _result = result;
  _req_thread = req_thread;
}

void VM_ThreadDump::doit() {
  for (int i = 0; i < _num_threads; i++) {
    instanceHandle th = _threads->at(i);
    if (th() == NULL) {
      // skip if the thread doesn't exist
      _result->set_stack_trace(i, NULL);
      continue;
    }

    // Dump thread stack only if the thread is alive and not exiting
    // and not VM internal thread.
    JavaThread* java_thread = java_lang_Thread::thread(th());
    if (java_thread == NULL || 
        java_thread->is_exiting() ||
        java_thread->is_hidden_from_external_view())  {
      _result->set_stack_trace(i, NULL);
      continue;
    }

    ThreadStackTrace* stacktrace = new ThreadStackTrace(java_thread);
    ThreadService::dump_stack_at_safepoint(java_thread, stacktrace, _max_depth);
    _result->set_stack_trace(i, stacktrace);
    if (_snapshots != NULL) {
      _snapshots[i] = new ThreadSnapshot(java_thread, _req_thread);
    }
  }
}

volatile bool VM_Exit::_vm_exited = false;
Thread * VM_Exit::_shutdown_thread = NULL;

int VM_Exit::set_vm_exited() {
  Thread * thr_cur = ThreadLocalStorage::get_thread_slow();

  assert(SafepointSynchronize::is_at_safepoint(), "must be at safepoint already");

  int num_active = 0;

  _shutdown_thread = thr_cur;
  _vm_exited = true;                                // global flag
  for(JavaThread *thr = Threads::first(); thr != NULL; thr = thr->next())
    if (thr!=thr_cur && thr->thread_state() == _thread_in_native) {
      ++num_active;
      thr->set_terminated(JavaThread::_vm_exited);  // per-thread flag
    }

  return num_active;
}

int VM_Exit::wait_for_threads_in_native_to_block() {
  // VM exits at safepoint. This function must be called at the final safepoint
  // to wait for threads in _thread_in_native state to be quiescent.
  assert(SafepointSynchronize::is_at_safepoint(), "must be at safepoint already");

  Thread * thr_cur = ThreadLocalStorage::get_thread_slow();
  Monitor timer(Mutex::leaf, "VM_Exit timer", true);

  // Compiler threads need longer wait because they can access VM data directly
  // while in native. If they are active and some structures being used are
  // deleted by the shutdown sequence, they will crash. On the other hand, user 
  // threads must go through native=>Java/VM transitions first to access VM
  // data, and they will be stopped during state transition. In theory, we
  // don't have to wait for user threads to be quiescent, but it's always
  // better to terminate VM when current thread is the only active thread, so
  // wait for user threads too. Numbers are in 10 milliseconds.
  int max_wait_user_thread = 30;                  // at least 300 milliseconds
  NOT_CORE(int max_wait_compiler_thread = 1000;)  // at least 10 seconds

  int max_wait = CORE_ONLY(max_wait_user_thread;)
                 NOT_CORE (max_wait_compiler_thread;)

  int attempts = 0;
  while (true) {
    int num_active = 0;
    NOT_CORE(int num_active_compiler_thread = 0;) 

    for(JavaThread *thr = Threads::first(); thr != NULL; thr = thr->next()) {
      if (thr!=thr_cur && thr->thread_state() == _thread_in_native) {
        num_active++;
        NOT_CORE(if (thr->is_Compiler_thread()) num_active_compiler_thread++;)
      }
    }

    if (num_active == 0) {
       return 0;
    } else if (attempts > max_wait) {
       return num_active;
#ifndef CORE
    } else if (num_active_compiler_thread == 0 && attempts > max_wait_user_thread) {
       return num_active;
#endif
    }

    attempts++;

    MutexLockerEx ml(&timer, Mutex::_no_safepoint_check_flag);
    timer.wait(Mutex::_no_safepoint_check_flag, 10);
  }
}

void VM_Exit::doit() {
  NOT_CORE(CompileBroker::set_should_block());

  // Wait for a short period for threads in native to block. Any thread
  // still executing native code after the wait will be stopped at
  // native==>Java/VM barriers.
  // Among 16276 JCK tests, 94% of them come here without any threads still
  // running in native; the other 6% are quiescent within 250ms (Ultra 80).
  wait_for_threads_in_native_to_block();

  set_vm_exited();

  // cleanup globals resources before exiting. exit_globals() currently
  // cleans up outputStream resources and PerfMemory resources.
  exit_globals();

  // Check for exit hook
  exit_hook_t exit_hook = Arguments::exit_hook();
  if (exit_hook != NULL) {
    // exit hook should exit. 
    exit_hook(_exit_code);
    // ... but if it didn't, we must do it here
    ::exit(_exit_code);
  } else {
    ::exit(_exit_code);
  }
}


void VM_Exit::wait_if_vm_exited() {
  if (_vm_exited && 
      ThreadLocalStorage::get_thread_slow() != _shutdown_thread) {
    // _vm_exited is set at safepoint, and the Threads_lock is never released
    // we will block here until the process dies
    Threads_lock->lock_without_safepoint_check();
    ShouldNotReachHere();
  }
}
