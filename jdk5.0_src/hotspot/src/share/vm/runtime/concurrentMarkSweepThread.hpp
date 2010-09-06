#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)concurrentMarkSweepThread.hpp	1.20 04/05/27 11:04:44 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// The SurrogateLockerThread is used by the CMSThread for
// manipulating Java monitors, in particular, currently for
// manipulating the pending_list_lock. XXX
class SurrogateLockerThread: public JavaThread {
  friend class VMStructs;
 public:
  enum SLT_msg_type {
    empty = 0,           // no message
    acquirePLL,          // acquire pending list lock
    releasePLL,          // release pending list lock
    releaseAndNotifyPLL  // notify and release pending list lock
  };
 private:
  // the following are shared with the CMSThread
  static SLT_msg_type* _buffer;  // communication buffer
  static Monitor*      _monitor; // monitor controlling buffer
  
 public:
  SurrogateLockerThread(SLT_msg_type* buffer,
                        Monitor*      monitor);

  // Hide this thread from external view.
  bool is_hidden_from_external_view() const	 { return true; }

  static void          loop(JavaThread* sltThread); // main method
};

class ConcurrentMarkSweepGeneration;
class CMSCollector;

// The Concurrent Mark Sweep GC Thread (could be several in the future).
class ConcurrentMarkSweepThread: public NamedThread {
  friend class VMStructs;
  friend class ConcurrentMarkSweepGeneration;   // XXX should remove friendship
  friend class CMSCollector;
 public:
  virtual void run();

 private:
  static ConcurrentMarkSweepThread*     _first_thread;
  static ConcurrentMarkSweepThread*     _last_thread;
  static SurrogateLockerThread*         _slt;
  static SurrogateLockerThread::SLT_msg_type _sltBuffer;
  static Monitor*                       _sltMonitor;
  static int                            _next_id;

  ConcurrentMarkSweepThread*            _next;
  CMSCollector*                         _collector;

  static bool _should_terminate;

  enum CMS_flag_type {
    CMS_nil             = NoBits,
    CMS_cms_wants_token = nth_bit(0),
    CMS_cms_has_token   = nth_bit(1),
    CMS_vm_wants_token  = nth_bit(2),
    CMS_vm_has_token    = nth_bit(3)
  };

  static int _CMS_flag;

  static bool CMS_flag_is_set(int b)        { return _CMS_flag & b;   }
  static bool set_CMS_flag(int b)           { return _CMS_flag |= b;  }
  static bool clear_CMS_flag(int b)         { return _CMS_flag &= ~b; }
  void sleepBeforeNextCycle();

  // CMS thread should yield for a young gen collection, direct allocation,
  // and iCMS activity.
  static volatile jint _pending_yields;
  static volatile jint _pending_decrements; // decrements to _pending_yields

  // Tracing messages, enabled by CMSTraceThreadState.
  static inline void trace_state(const char* desc);

  static          bool _icms_enabled;	// iCMS enabled?
  static volatile bool _should_run;	// iCMS may run
  static volatile bool _should_stop;	// iCMS should stop

  // debugging
  void verify_ok_to_terminate() const PRODUCT_RETURN;

 public:
  // Constructor
  ConcurrentMarkSweepThread(CMSCollector* collector);
  ~ConcurrentMarkSweepThread();
  static void makeSurrogateLockerThread(TRAPS);
  static void manipulatePLL(SurrogateLockerThread::SLT_msg_type msg);

  // Tester
  bool is_ConcurrentMarkSweep_thread() const          { return true;       }

  static void threads_do(ThreadClosure* tc);

  // Printing
  void print() const;
  static void print_all();

  // Returns the first instance of a CMS Thread (currently there's only one)
  static ConcurrentMarkSweepThread* first_thread()    { return _first_thread; }

  // Returns the next instance of a CMS Thread
  // (currently this always returns NULL because there's only 1 CMS thread)
  ConcurrentMarkSweepThread* next()                   { return _next;       }
  CMSCollector*              collector()              { return _collector;  }

  // Create and start this instance of CMS Thread, or stop it on shutdown
  static void start(CMSCollector* collector);
  static void stop();
  static bool should_terminate() { return _should_terminate; }

  // Synchronization using CMS token
  static void synchronize(bool is_cms_thread);
  static void desynchronize(bool is_cms_thread);
  static bool vm_thread_has_cms_token() {
    return CMS_flag_is_set(CMS_vm_has_token);
  }
  static bool cms_thread_has_cms_token() {
    return CMS_flag_is_set(CMS_cms_has_token);
  }
  static bool vm_thread_wants_cms_token() {
    return CMS_flag_is_set(CMS_vm_wants_token);
  }
  static bool cms_thread_wants_cms_token() {
    return CMS_flag_is_set(CMS_cms_wants_token);
  }

  // Wait on CMS lock until the next synchronous GC 
  // or given timeout, whichever is earlier.
  void    wait_on_cms_lock(long t); // milliseconds

  // The CMS thread will yield during the work portion of it's cycle
  // only when requested to.  Both synchronous and asychronous requests
  // are provided.  A synchronous request is used for young gen
  // collections and direct allocations.  The requesting thread increments
  // pending_yields at the beginning of an operation, and decrements it when
  // the operation is completed.  The CMS thread yields when pending_yields
  // is positive.  An asynchronous request is used by iCMS in the stop_icms()
  // operation. A single yield satisfies the outstanding asynch yield requests.
  // The requesting thread increments both pending_yields and pending_decrements.
  // After yielding, the CMS thread decrements both by the amount in
  // pending_decrements.
  static void increment_pending_yields()   {
    Atomic::inc(&_pending_yields);
    assert(_pending_yields > 0, "can't be zero or negative");
    assert(_pending_yields >= _pending_decrements, "more decrements than yield requests");
  }
  static void decrement_pending_yields()   {
    Atomic::dec(&_pending_yields);
    assert(_pending_yields >= 0, "can't be negative");
    assert(_pending_yields >= _pending_decrements, "more decrements than yield requests");
  }
  static void asynchronous_yield_request() {
    increment_pending_yields();
    Atomic::inc(&_pending_decrements);
    assert(_pending_decrements > 0, "can't be zero or negative");
    assert(_pending_yields >= _pending_decrements, "more decrements than yield requests");
  }
  static void acknowledge_yield_request() {
    jint decrement = _pending_decrements;
    if (decrement > 0) {
      // Order important to preserve: _pending_yields >= _pending_decrements
      Atomic::add(-decrement, &_pending_decrements);
      Atomic::add(-decrement, &_pending_yields);
      assert(_pending_decrements >= 0, "can't be negative");
      assert(_pending_yields >= _pending_decrements, "more decrements than yield requests");
    }
  }
  static bool should_yield()   { return _pending_yields > 0; }

  // CMS incremental mode.
  static void start_icms(); // notify thread to start a quantum of work
  static void stop_icms();  // request thread to stop working
  void icms_wait();	    // if asked to stop, wait until notified to start

  // Incremental mode is enabled globally by the flag CMSIncrementalMode.  It
  // must also be enabled/disabled dynamically to allow foreground collections.
  static inline void enable_icms()              { _icms_enabled = true; }
  static inline void disable_icms()             { _icms_enabled = false; }
  static inline void set_icms_enabled(bool val) { _icms_enabled = val; }
  static inline bool icms_enabled()             { return _icms_enabled; } 
};

inline void ConcurrentMarkSweepThread::trace_state(const char* desc) {
  if (CMSTraceThreadState) {
    char buf[128];
    TimeStamp& ts = gclog_or_tty->time_stamp();
    if (!ts.is_updated()) {
      ts.update();
    }
    jio_snprintf(buf, sizeof(buf), " [%.3f:  CMSThread %s] ",
		 ts.seconds(), desc);
    buf[sizeof(buf) - 1] = '\0';
    gclog_or_tty->print(buf);
  }
}

// For scoped increment/decrement of yield requests
class CMSSynchronousYieldRequest: public StackObj {
 public:
  CMSSynchronousYieldRequest() {
    ConcurrentMarkSweepThread::increment_pending_yields();
  }
  ~CMSSynchronousYieldRequest() {
    ConcurrentMarkSweepThread::decrement_pending_yields();
  }
};

// Used to emit a warning in case of unexpectedly excessive
// looping (in "apparently endless loops") in CMS code.
class CMSLoopCountWarn: public StackObj {
  friend class ConcurrentMarkSweepThread;
  friend class ConcurrentMarkSweepGeneration;

 private:
  const char* _src;
  const char* _msg;
  const intx  _threshold;
  intx        _ticks;

 protected:
  inline CMSLoopCountWarn(const char* src, const char* msg,
                          const intx threshold) :
    _src(src), _msg(msg), _threshold(threshold), _ticks(0) { }

  inline void tick() {
    _ticks++;
    if (CMSLoopWarn && _ticks % _threshold == 0) {
      warning("%s has looped %d times %s", _src, _ticks, _msg);
    }
  }
};
