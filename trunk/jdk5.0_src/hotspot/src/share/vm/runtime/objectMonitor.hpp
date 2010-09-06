#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)objectMonitor.hpp	1.27 03/12/23 16:43:57 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// WARNING:
//   This is a very sensitive and fragile class. DO NOT make any
// change unless you are fully aware of the underlying semantics.

//   This class can not inherit from any other class, because I have
// to let the displaced header be the very first word. Otherwise I
// have to let markOop include this file, which would export the
// monitor data structure to everywhere.
//
// The ObjectMonitor class is used to implement JavaMonitors which have
// transformed from the lightweight structure of the thread stack to a
// heavy weight lock due to contention

// It is also used as RawMonitor by the JVMTI and JVMPI


class ObjectWaiter;

class ObjectMonitor {
 public:
  enum {
    OM_OK,                    // no error
    OM_SYSTEM_ERROR,          // operating system error
    OM_ILLEGAL_MONITOR_STATE, // IllegalMonitorStateException
    OM_INTERRUPTED,           // Thread.interrupt()
    OM_TIMED_OUT              // Object.wait() timed out
  };

 public:
  static int header_offset_in_bytes() { return (intptr_t)&((ObjectMonitor*)NULL)->_header; }
  static int object_offset_in_bytes() { return (intptr_t)&((ObjectMonitor*)NULL)->_object; }
  static int owner_offset_in_bytes()  { return (intptr_t)&((ObjectMonitor*)NULL)->_owner; }
  static int count_offset_in_bytes()  { return (intptr_t)&((ObjectMonitor*)NULL)->_count; }

 public:
  ObjectMonitor();
  ~ObjectMonitor();

  markOop   header() const;
  void      set_header(markOop hdr);

  intptr_t  is_busy() const;
  intptr_t  is_entered(Thread* current) const;
  
  void*     owner() const;
  void      set_owner(void* owner);

  intptr_t  waiters() const;
  void*     queue() const;
  void      set_queue(void* queue);

  intptr_t  count() const;
  void      set_count(intptr_t count);

  // JVM/DI GetMonitorInfo() needs this
  intptr_t  recursions() const { return _recursions; }

  void*     object() const;
  void*     object_addr();
  void      set_object(void* obj);

  bool      check(TRAPS);	// true if the thread owns the monitor.
  void      check_slow(TRAPS);
  void      clear();
#ifndef PRODUCT
  void      verify();
  void      print();
#endif
  
  void      enter(TRAPS);
  void      exit(TRAPS);
  void      wait(jlong millis, bool interruptable, TRAPS);
  void      notify(TRAPS);
  void      notifyAll(TRAPS);

  int       raw_enter(TRAPS, bool isRawMonitor);
  int       raw_exit(TRAPS, bool isRawMonitor);
  int       raw_wait(jlong millis, bool interruptable, TRAPS);
  int       raw_notify(TRAPS);
  int       raw_notifyAll(TRAPS);
  

 private:
  friend class ObjectSynchronizer;
  friend class ObjectWaiter;
  friend class RawMonitor;
  friend class VMStructs;

  // WARNING: this must be the very first word of ObjectMonitor
  volatile markOop   _header;      // displaced object mark

  // All the following fields must be machine word aligned
  // The VM assumes write ordering wrt these fields, which can be
  // read from other threads.
  void*     volatile _object;      // backward object pointer
  void*     volatile _owner;       // pointer to owning thread OR BasicLock
  void*     volatile _queue;       // waiting queue structure
                                   // (also used by ObjectSynchronizer)
  volatile intptr_t  _count;       // number of entering threads
  volatile intptr_t  _waiters;     // number of waiting threads
  volatile intptr_t  _recursions;  // recursion count, 0 for first entry

  // the following contains processor/os specific definitions
  #include "incls/_objectMonitor_pd.hpp.incl"
};


