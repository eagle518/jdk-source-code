#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)concurrentMarkSweepThread.cpp	1.33 03/12/23 16:43:35 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_concurrentMarkSweepThread.cpp.incl"

// ======= Concurrent Mark Sweep Thread ========

// The CMS thread is created when Concurrent Mark Sweep is used in the
// older of two generations in a generational memory system.

ConcurrentMarkSweepThread*
     ConcurrentMarkSweepThread::_first_thread     = NULL;
ConcurrentMarkSweepThread*
     ConcurrentMarkSweepThread::_last_thread      = NULL;
bool ConcurrentMarkSweepThread::_should_terminate = false;
int  ConcurrentMarkSweepThread::_CMS_flag         = CMS_nil;
int  ConcurrentMarkSweepThread::_next_id          = 0;

volatile jint ConcurrentMarkSweepThread::_pending_yields      = 0;
volatile jint ConcurrentMarkSweepThread::_pending_decrements  = 0;

         bool ConcurrentMarkSweepThread::_icms_enabled   = false;
volatile bool ConcurrentMarkSweepThread::_should_run     = false;
volatile bool ConcurrentMarkSweepThread::_should_stop    = false;

SurrogateLockerThread*
     ConcurrentMarkSweepThread::_slt = NULL;
SurrogateLockerThread::SLT_msg_type
     ConcurrentMarkSweepThread::_sltBuffer = SurrogateLockerThread::empty;
Monitor*
     ConcurrentMarkSweepThread::_sltMonitor = NULL;

ConcurrentMarkSweepThread::ConcurrentMarkSweepThread(CMSCollector* collector)
  : NamedThread() {
  assert(UseConcMarkSweepGC,  "UseConcMarkSweepGC should be set");
  _collector = collector;
  _next = NULL;
  collector->_cmsThread = this;
  int id = 0;
  // hook up to the end of the set of CMS threads
  {
    MutexLockerEx x(CMS_lock,
                    Mutex::_no_safepoint_check_flag);
    if (_first_thread == NULL) {
      assert(_last_thread == NULL, "consistency check");
      _first_thread = _last_thread = this;
    } else {
      assert(_last_thread != NULL, "consistency check");
      _last_thread->_next = this;
      _last_thread = this;
    }
    // Someday we could store the id, but for now it's only used for the name
    id = _next_id++;
             
    // For now there's only one CMS thread
    assert(_first_thread == _last_thread, "for now");
  }

  set_name("Concurrent Mark-Sweep GC Thread#%d", id);

  if (os::create_thread(this, os::cms_thread)) {
    // XXX: need to set this to low priority
    // unless "agressive mode" set; priority
    // should be just less than that of VMThread.
    os::set_priority(this, NearMaxPriority);
    if (!DisableStartThread) {
      os::start_thread(this);
    }
  }
  _sltMonitor = SLT_lock;
  set_icms_enabled(CMSIncrementalMode);
}

ConcurrentMarkSweepThread::~ConcurrentMarkSweepThread() {
  free_name();
}

void ConcurrentMarkSweepThread::run() {
  assert(this == first_thread(), "just checking");

  this->record_stack_base_and_size();
  this->initialize_thread_local_storage();
  this->set_active_handles(JNIHandleBlock::allocate_block());
  // From this time Thread::current() should be working.
  assert(this == Thread::current(), "just checking");
  // Wait until Universe::is_fully_initialized()
  {
    CMSLoopCountWarn loopX("CMS::run", "waiting for "
                           "Universe::is_fully_initialized()", 2);
    MutexLockerEx x(CMS_lock, true);
    set_CMS_flag(CMS_cms_wants_token);
    // Wait until Universe is initialized and all initialization is completed.
    while (!is_init_completed() && !Universe::is_fully_initialized() &&
           !_should_terminate) {
      CMS_lock->wait(true, 200);
      loopX.tick();
    }
    // Wait until the surrogate locker thread that will do
    // pending list locking on our behalf has been created.
    // We cannot start the SLT thread ourselves since we need
    // to be a JavaThread to do so.
    CMSLoopCountWarn loopY("CMS::run", "waiting for SLT installation", 2);
    while (_slt == NULL && !_should_terminate) {
      CMS_lock->wait(true, 200);
      loopY.tick();
    }
    clear_CMS_flag(CMS_cms_wants_token);
  }

  while (!_should_terminate) {
    sleepBeforeNextCycle();
    if (_should_terminate) break;
    _collector->collect_in_background(false);  // !clear_all_soft_refs
  }
  assert(_should_terminate, "just checking");
  // Check that the state of any protocol for synchronization
  // between background (CMS) and foreground collector is "clean"
  // (i.e. will not potentially block the foreground collector,
  // requiring action by us).
  verify_ok_to_terminate();
  // Signal that it is terminated
  {
    MutexLockerEx mu(Terminator_lock,
                     Mutex::_no_safepoint_check_flag);
    assert(_first_thread == this, "just checking");
    assert(_first_thread->_next == NULL, "just checking");
    {
      MutexLockerEx   x(CMS_lock,
                        Mutex::_no_safepoint_check_flag);
      _first_thread = _last_thread = NULL;
    }
    Terminator_lock->notify();
  }
  
  // Thread destructor usually does this..
  ThreadLocalStorage::set_thread(NULL);
}

#ifndef PRODUCT
void ConcurrentMarkSweepThread::verify_ok_to_terminate() const {
  assert(!(CMS_lock->owned_by_self() || cms_thread_has_cms_token() ||
           cms_thread_wants_cms_token()),
         "Must renounce all worldly possessions and desires for nirvana");
  _collector->verify_ok_to_terminate();
}
#endif

// create and start a new ConcurrentMarkSweep Thread for given CMS generation
void ConcurrentMarkSweepThread::start(CMSCollector* collector) {
  if (!_should_terminate) {
    assert(first_thread() == NULL, "Only one CMS thread for now");
    ConcurrentMarkSweepThread* th = new ConcurrentMarkSweepThread(collector);
    assert(first_thread() == th, "Where did the just-created CMS thread go?");
  }
}

void ConcurrentMarkSweepThread::stop() {
  if (CMSIncrementalMode) {
    // Disable incremental mode and wake up the thread so it notices the change.
    disable_icms();
    start_icms();
  }
  // it is ok to take late safepoints here, if needed
  {
    MutexLockerEx x(Terminator_lock);
    _should_terminate = true;  
  }
  { // Now post a notify on CMS_lock so as to nudge
    // CMS thread(s) that might be slumbering in
    // sleepBeforeNextCycle.
    MutexLockerEx x(CMS_lock, Mutex::_no_safepoint_check_flag);
    CMS_lock->notify_all();
  }
  { // Now wait until (all) CMS thread(s) have exited
    MutexLockerEx x(Terminator_lock);
    while(first_thread() != NULL) {
      Terminator_lock->wait();
    }
  }  
}

void ConcurrentMarkSweepThread::threads_do(ThreadClosure* tc) {
  assert(tc != NULL, "Null ThreadClosure");
  ConcurrentMarkSweepThread* thr = _first_thread;
  for (uint i = 0; thr != NULL; i++, thr = thr->_next) {
    tc->do_thread(thr);
  }
}

void ConcurrentMarkSweepThread::print() const {
  tty->print("\"%s\" ", name());
  Thread::print();
  tty->cr();
}

void ConcurrentMarkSweepThread::print_all() {
  ConcurrentMarkSweepThread* thr = _first_thread;
  for (uint i = 0; thr != NULL; i++, thr = thr->_next) {
    thr->print();
    tty->cr();
  }
}

void ConcurrentMarkSweepThread::synchronize(bool is_cms_thread) {
  assert(UseConcMarkSweepGC, "just checking");

  MutexLockerEx x(CMS_lock,
                  Mutex::_no_safepoint_check_flag);
  if (!is_cms_thread) {
    assert(Thread::current()->is_VM_thread(), "Not a VM thread");
    CMSSynchronousYieldRequest yr;
    while (CMS_flag_is_set(CMS_cms_has_token)) {
      // indicate that we want to get the token
      set_CMS_flag(CMS_vm_wants_token);
      CMS_lock->wait(true);
    }
    // claim the token and proceed
    clear_CMS_flag(CMS_vm_wants_token);
    set_CMS_flag(CMS_vm_has_token);
  } else {
    assert(Thread::current()->is_ConcurrentMarkSweep_thread(),
           "Not a CMS thread");
    // The following barrier assumes there's only one CMS thread.
    // This will need to be modified is there are more CMS threads than one.
    while (CMS_flag_is_set(CMS_vm_has_token | CMS_vm_wants_token)) {
      set_CMS_flag(CMS_cms_wants_token);
      CMS_lock->wait(true);
    }
    // claim the token
    clear_CMS_flag(CMS_cms_wants_token);
    set_CMS_flag(CMS_cms_has_token);
  }
}

void ConcurrentMarkSweepThread::desynchronize(bool is_cms_thread) {
  assert(UseConcMarkSweepGC, "just checking");

  MutexLockerEx x(CMS_lock,
                  Mutex::_no_safepoint_check_flag);
  if (!is_cms_thread) {
    assert(Thread::current()->is_VM_thread(), "Not a VM thread");
    assert(CMS_flag_is_set(CMS_vm_has_token), "just checking");
    clear_CMS_flag(CMS_vm_has_token);
    if (CMS_flag_is_set(CMS_cms_wants_token)) {
      // wake-up a waiting CMS thread
      CMS_lock->notify();
    }
    assert(!CMS_flag_is_set(CMS_vm_has_token | CMS_vm_wants_token),
           "Should have been cleared");
  } else {
    assert(Thread::current()->is_ConcurrentMarkSweep_thread(),
           "Not a CMS thread");
    assert(CMS_flag_is_set(CMS_cms_has_token), "just checking");
    clear_CMS_flag(CMS_cms_has_token);
    if (CMS_flag_is_set(CMS_vm_wants_token)) {
      // wake-up a waiting VM thread
      CMS_lock->notify();
    }
    assert(!CMS_flag_is_set(CMS_cms_has_token | CMS_cms_wants_token),
           "Should have been cleared");
  }
}

// Wait until the next synchronous GC or a timeout, whichever is earlier.
void ConcurrentMarkSweepThread::wait_on_cms_lock(long t) {
  MutexLockerEx x(CMS_lock,
                  Mutex::_no_safepoint_check_flag);
  set_CMS_flag(CMS_cms_wants_token);   // to provoke notifies
  CMS_lock->wait(Mutex::_no_safepoint_check_flag, t);
  clear_CMS_flag(CMS_cms_wants_token);
  assert(!CMS_flag_is_set(CMS_cms_has_token | CMS_cms_wants_token),
         "Should not be set");
}

void ConcurrentMarkSweepThread::sleepBeforeNextCycle() {
  while (!_should_terminate) {
    if (CMSIncrementalMode) {
      icms_wait();
      return;
    } else {
      // Wait until the next synchronous GC or a timeout, whichever is earlier
      wait_on_cms_lock(CMSWaitDuration);
    }
    // Check if we should start a CMS collection cycle
    if (_collector->shouldConcurrentCollect()) {
      return;
    }
    // .. collection criterion not yet met, let's go back 
    // and wait some more
  }
}

// Incremental CMS
void ConcurrentMarkSweepThread::start_icms() {
  assert(UseConcMarkSweepGC && CMSIncrementalMode, "just checking");
  MutexLockerEx x(iCMS_lock, Mutex::_no_safepoint_check_flag);
  trace_state("start_icms");
  _should_run = true;
  iCMS_lock->notify_all();
}

void ConcurrentMarkSweepThread::stop_icms() {
  assert(UseConcMarkSweepGC && CMSIncrementalMode, "just checking");
  MutexLockerEx x(iCMS_lock, Mutex::_no_safepoint_check_flag);
  if (!_should_stop) {
    trace_state("stop_icms");
    _should_stop = true;
    _should_run = false;
    asynchronous_yield_request();
    iCMS_lock->notify_all();
  }
}

void ConcurrentMarkSweepThread::icms_wait() {
  assert(UseConcMarkSweepGC && CMSIncrementalMode, "just checking");
  if (_should_stop && icms_enabled()) {
    MutexLockerEx x(iCMS_lock, Mutex::_no_safepoint_check_flag);
    trace_state("pause_icms");
    _collector->stats().stop_cms_timer();
    while(!_should_run && icms_enabled()) {
      iCMS_lock->wait(Mutex::_no_safepoint_check_flag);
    }
    _collector->stats().start_cms_timer();
    _should_stop = false;
    trace_state("pause_icms end");
  }
}

// Note: this method, although exported by the ConcurrentMarkSweepThread,
// which is a non-JavaThread, can only be called by a JavaThread.
// Currently this is done at vm creation time (post-vm-init) by the
// main/Primordial (Java)Thread.
// XXX Consider changing this in the future to allow the CMS thread
// itself to create this thread?
void ConcurrentMarkSweepThread::makeSurrogateLockerThread(TRAPS) {
  assert(UseConcMarkSweepGC, "SLT thread needed only for CMS GC");
  assert(_slt == NULL, "SLT already created");

  klassOop k =
    SystemDictionary::resolve_or_fail(vmSymbolHandles::java_lang_Thread(),
                                      true, CHECK);
  instanceKlassHandle klass (THREAD, k);
  instanceHandle thread_oop = klass->allocate_instance_handle(CHECK);

  const char thread_name[] = "Surrogate Locker Thread (CMS)";
  Handle string = java_lang_String::create_from_str(thread_name, CHECK);

  // Initialize thread_oop to put it into the system threadGroup
  Handle thread_group (THREAD, Universe::system_thread_group());
  JavaValue result(T_VOID);
  JavaCalls::call_special(&result, thread_oop,
                       klass,
                       vmSymbolHandles::object_initializer_name(),
                       vmSymbolHandles::threadgroup_string_void_signature(),
                       thread_group,
                       string,
                       CHECK);

  {
    MutexLocker mu(Threads_lock);
    assert(_slt == NULL, "SLT already created");
    _slt = new SurrogateLockerThread(&_sltBuffer, SLT_lock);

    // At this point it may be possible that no osthread was created for the
    // JavaThread due to lack of memory. We would have to throw an exception
    // in that case. However, since this must work and we do not allow
    // exceptions anyway, check and abort if this fails.
    if (_slt == NULL || _slt->osthread() == NULL) {
      vm_exit_during_initialization("java.lang.OutOfMemoryError",
                                    "unable to create new native thread");
    }
    java_lang_Thread::set_thread(thread_oop(), _slt);
    java_lang_Thread::set_priority(thread_oop(), NearMaxPriority);
    java_lang_Thread::set_daemon(thread_oop());

    _slt->set_threadObj(thread_oop());
    Threads::add(_slt);
    Thread::start(_slt);
  }
  os::yield(); // This seems to help with initial start-up of SLT
}

void ConcurrentMarkSweepThread::manipulatePLL(
  SurrogateLockerThread::SLT_msg_type msg) {

  MutexLockerEx x(_sltMonitor, Mutex::_no_safepoint_check_flag);
  assert(_sltBuffer == SurrogateLockerThread::empty, "Should be empty");
  assert(msg != SurrogateLockerThread::empty, "empty message");
  _sltBuffer = msg;
  while (_sltBuffer != SurrogateLockerThread::empty) {
    _sltMonitor->notify();
    _sltMonitor->wait(Mutex::_no_safepoint_check_flag);
  }
}

// ======= Surrogate Locker Thread =============

static void _sltLoop(JavaThread* thread, TRAPS) {
  SurrogateLockerThread::loop(thread);
}

SurrogateLockerThread::SLT_msg_type*
  SurrogateLockerThread::_buffer = NULL;
Monitor*
  SurrogateLockerThread::_monitor = NULL;

SurrogateLockerThread::SurrogateLockerThread(
  SLT_msg_type* buffer,
  Monitor*      monitor):
  JavaThread(&_sltLoop) {
  assert(_buffer == NULL && _monitor == NULL, "already initialized?");
  _buffer  = buffer;
  _monitor = monitor;
}

void SurrogateLockerThread::loop(JavaThread* sltThread) {
  BasicLock pll_basic_lock;
  SLT_msg_type msg;
  debug_only(unsigned int owned = 0;)

  while (/* !isTerminated() */ 1) {
    {
      MutexLocker x(_monitor);
      // Since we are a JavaThread, we can't be here at a safepoint.
      assert(!SafepointSynchronize::is_at_safepoint(),
             "SLT is a JavaThread");
      // wait for msg buffer to become non-empty
      while (*_buffer == empty) {
        _monitor->notify();
        _monitor->wait();
      }
      msg = *_buffer;
    }
    switch(msg) {
      case acquirePLL: {
        instanceRefKlass::acquire_pending_list_lock(&pll_basic_lock);
        debug_only(owned++;)
        break;
      }
      case releasePLL: {
        assert(owned > 0, "Don't have PLL");
        instanceRefKlass::release_and_notify_pending_list_lock(
          false, &pll_basic_lock);
        debug_only(owned--;)
        break;
      }
      case releaseAndNotifyPLL: {
        assert(owned > 0, "Don't have PLL");
        instanceRefKlass::release_and_notify_pending_list_lock(
          true, &pll_basic_lock);
        debug_only(owned--;)
        break;
      }
      case empty:
      default: {
        guarantee(false,"Unexpected message in _buffer");
        break;
      }
    }
    {
      MutexLocker x(_monitor);
      // Since we are a JavaThread, we can't be here at a safepoint.
      assert(!SafepointSynchronize::is_at_safepoint(),
             "SLT is a JavaThread");
      assert(msg == *_buffer, "Mailbox protocol violation");
      *_buffer = empty;
      _monitor->notify();
    }
  }
  assert(!_monitor->owned_by_self(), "Should unlock before exit.");
}
