#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)synchronizer.hpp	1.48 03/12/23 16:44:16 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class BasicLock VALUE_OBJ_CLASS_SPEC {
  friend class VMStructs;
 private:  
  volatile markOop _displaced_header;
 public:
  markOop      displaced_header() const               { return _displaced_header; }
  void         set_displaced_header(markOop header)   { _displaced_header = header; }
  
  void print_on(outputStream* st) const;

  // move a basic lock (used during deoptimization
  void move_to(oop obj, BasicLock* dest);

  static int displaced_header_offset_in_bytes()       { return (intptr_t)&((BasicLock*)NULL)->_displaced_header; }  
};

// A BasicObjectLock associates a specific Java object with a BasicLock.
// It is currently embedded in an interpreter frame.

// Because some machines have alignment restrictions on the control stack,
// the actual space allocated by the interpreter may include padding words
// after the end of the BasicObjectLock.  Also, in order to guarantee
// alignment of the embedded BasicLock objects on such machines, we
// put the embedded BasicLock at the beginning of the struct.

class BasicObjectLock VALUE_OBJ_CLASS_SPEC {
  friend class VMStructs;
 private:
  BasicLock _lock;                                    // the lock, must be double word aligned
  oop       _obj;                                     // object holds the lock;

 public:
  // Manipulation
  oop      obj() const                                { return _obj;  }
  void set_obj(oop obj)                               { _obj = obj; } 
  BasicLock* lock()                                   { return &_lock; }

  // Note: Use frame::interpreter_frame_monitor_size() for the size of BasicObjectLocks
  //       in interpreter activation frames since it includes machine-specific padding.
  static int size()                                   { return sizeof(BasicObjectLock)/wordSize; }          

  // GC support
  void oops_do(OopClosure* f) { f->do_oop(&_obj); }

  static int obj_offset_in_bytes()                    { return (intptr_t)&((BasicObjectLock*)NULL)->_obj;  }
  static int lock_offset_in_bytes()                   { return (intptr_t)&((BasicObjectLock*)NULL)->_lock; }
};

class ObjectMonitor;

class ObjectSynchronizer : AllStatic { 
  friend class VMStructs;
 public:
  // exit must be implemented non-blocking, since the compiler cannot easily handle 
  // deoptimization at monitor exit. Hence, it does not take a Handle argument.

  // This is full version of monitor enter and exit. I choose not
  // to use enter() and exit() in order to make sure user be ware
  // of the performance and semantics difference. They are normally
  // used by ObjectLocker etc. The interpreter and compiler use 
  // assembly copies of these routines. Please keep them synchornized.
  static void fast_enter  (Handle obj, BasicLock* lock, TRAPS);
  static void fast_exit   (oop obj,    BasicLock* lock, Thread* THREAD);

  // WARNING: They are ONLY used to handle the slow cases. They should 
  // only be used when the fast cases failed. Use of these functions 
  // without previous fast case check may cause fatal error.
  static void slow_enter  (Handle obj, BasicLock* lock, TRAPS);
  static void slow_exit   (oop obj,    BasicLock* lock, Thread* THREAD);

  // Used only to handle jni locks or other unmatched monitor enter/exit
  // Internally they will use heavy weight monitor.
  static void jni_enter   (Handle obj, TRAPS);
  static void jni_exit    (oop obj,    Thread* THREAD);

  // Handle all interpreter, compiler and jni cases
  static void wait               (Handle obj, jlong millis, TRAPS);
  static void notify             (Handle obj,               TRAPS);
  static void notifyall          (Handle obj,               TRAPS);

  // Inflate light weight monitor to heavy weight monitor
  static ObjectMonitor* inflate(oop obj);
  // This version is only for internal use
  inline static ObjectMonitor* inflate_helper(oop obj);

  // Returns the identity hash value for an oop
  // NOTE: It may cause monitor inflation
  static intptr_t identity_hash_value_for(Handle obj);  
  
  // java.lang.Thread support
  static bool current_thread_holds_lock(JavaThread* thread, Handle h_obj);

  static JavaThread* get_lock_owner(Handle h_obj, bool doLock);

  // GC: we current use aggressive monitor deflation policy
  // Basically we deflate all monitors that are not busy.
  // An adaptive profile-based deflation policy could be used if needed
  static void deflate_idle_monitors();
  static void oops_do(OopClosure* f);

  // JVMPI
  static void monitors_iterate(MonitorClosure* m);

  // debugging
  static void trace_locking(Handle obj, bool is_compiled, bool is_method, bool is_locking) PRODUCT_RETURN;
  static void verify() PRODUCT_RETURN;
  static int  verify_objmon_isinpool(ObjectMonitor *addr) PRODUCT_RETURN0;

 private:
  enum { BLOCK_SIZE = 64 };
  static ObjectMonitor* gBlockList;
  static ObjectMonitor* gFreeList;
};

// ObjectLocker enforced balanced locking and can never thrown an
// IllegalMonitorStateException. However, a pending exception may
// have to pass through, and we must also be able to deal with
// asynchronous exceptions. The caller is responsible for checking
// the threads pending exception if needed.
class ObjectLocker : public StackObj {
 private:
  Thread*   _thread;
  Handle    _obj;
  BasicLock _lock;
 public:
  ObjectLocker(Handle obj, Thread* thread);
  ~ObjectLocker();                                
  
  // Check if the current threads has reentered the same critical section more than once
  bool is_reentrant() const   { return _lock.displaced_header() == NULL; }

  // Monitor behavior
  void wait      (TRAPS)      { ObjectSynchronizer::wait     (_obj, 0, CHECK); } // wait forever
  void notify_all(TRAPS)      { ObjectSynchronizer::notifyall(_obj,    CHECK); } 
};
