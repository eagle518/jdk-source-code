#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)osThread.hpp	1.32 04/04/21 10:05:12 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// The OSThread class holds OS-specific thread information.  It is equivalent
// to the sys_thread_t structure of the classic JVM implementation.

// The thread states represented by the ThreadState values are platform-specific
// and are likely to be only approximate, because most OSes don't give you access 
// to precise thread state information.

// Note: the ThreadState is legacy code and is not correctly implemented.
// Uses of ThreadState need to be replaced by the state in the JavaThread.

enum ThreadState {
  ALLOCATED,                    // Memory has been allocated but not initialized
  INITIALIZED,                  // The thread has been initialized but yet started
  RUNNABLE,                     // Has been started and is runnable, but not necessarily running
  MONITOR_WAIT,                 // Waiting on a contended monitor lock
  CONDVAR_WAIT,                 // Waiting on a condition variable
  OBJECT_WAIT,                  // Waiting on an Object.wait() call
  BREAKPOINTED,                 // Suspended at breakpoint 
  SLEEPING,                     // Thread.sleep() 
  ZOMBIE                        // All done, but not reclaimed yet 
};

// I'd make OSThread a ValueObj embedded in Thread to avoid an indirection, but
// the assembler test in java.cpp expects that it can install the OSThread of
// the main thread into its own Thread at will.


class OSThread: public CHeapObj {
  friend class VMStructs;
 private:
  //void* _start_proc;            // Thread start routine
  OSThreadStartFunc _start_proc;  // Thread start routine
  void* _start_parm;              // Thread start routine parameter
  volatile ThreadState _state;    // Thread state *hint*
  jint _interrupted;              // Thread.isInterrupted state

  // Note:  _interrupted must be jint, so that Java intrinsics can access it.
  // The value stored there must be either 0 or 1.  It must be possible
  // for Java to emulate Thread.currentThread().isInterrupted() by performing
  // the double indirection Thread::current()->_osthread->_interrupted.

  // Methods
 public:
  void set_state(ThreadState state)                { _state = state; }
  ThreadState get_state()                          { return _state; }

  // Constructor
  OSThread(OSThreadStartFunc start_proc, void* start_parm);

  // Destructor
  ~OSThread();

  // Accessors
  OSThreadStartFunc start_proc() const              { return _start_proc; }
  void set_start_proc(OSThreadStartFunc start_proc) { _start_proc = start_proc; }
  void* start_parm() const                          { return _start_parm; }
  void set_start_parm(void* start_parm)             { _start_parm = start_parm; }

  bool interrupted() const                          { return _interrupted; }
  void set_interrupted(bool z)                      { _interrupted = z; }

  void print();
 
  // For java intrinsics:
  static ByteSize interrupted_offset()            { return byte_offset_of(OSThread, _interrupted); }

  // Platform dependent stuff
  #include "incls/_osThread_pd.hpp.incl"
};


// Utility class for use with condition variables:
class OSThreadWaitState : public StackObj {
  OSThread*   _osthread;
  ThreadState _old_state;
 public:
  OSThreadWaitState(OSThread* osthread, bool is_object_wait) {
    _osthread  = osthread;
    _old_state = osthread->get_state();
    if (is_object_wait) {
      osthread->set_state(OBJECT_WAIT);
    } else {
      osthread->set_state(CONDVAR_WAIT);
    }
  }
  ~OSThreadWaitState() {
    _osthread->set_state(_old_state);
  }
};


// Utility class for use with contended monitors:
class OSThreadContendState : public StackObj {
  OSThread*   _osthread;
  ThreadState _old_state;
 public:
  OSThreadContendState(OSThread* osthread) {
    _osthread  = osthread;
    _old_state = osthread->get_state();
    osthread->set_state(MONITOR_WAIT);
  }
  ~OSThreadContendState() {
    _osthread->set_state(_old_state);
  }
};
