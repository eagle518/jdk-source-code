#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)vm_operations.hpp	1.114 04/03/30 16:51:13 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// The following classes are used for operations
// initiated by a Java thread but that must
// take place in the VMThread.

class VM_Operation: public CHeapObj {
 public:
  enum Mode { _safepoint,       // Requesting thread     blocked,    safepoint, VM_Operation C-Heap allocated
              _no_safepoint,    // Requesting thread     blocked, no safepoint, VM_Operation C-Heap allocated
              _concurrent,      // Requesting thread not blocked, no safepoint, VM_Operation C-Heap allocated
              _async_safepoint  // Requesting thread not blocked,    safepoint, VM_Operation C-Heap allocated 
            };
              
 private:
  Thread*	  _calling_thread;
  ThreadPriority  _priority;
  long            _timestamp;
  VM_Operation*	  _next;  
  VM_Operation*   _prev;
 public:
  VM_Operation()  { _calling_thread = NULL; _next = NULL; _prev = NULL; }
  virtual ~VM_Operation() {}

  // VM operation support (used by VM thread)  
  Thread* calling_thread() const                 { return _calling_thread; }  
  ThreadPriority priority()                      { return _priority; }
  void set_calling_thread(Thread* thread, ThreadPriority priority);
  
  long timestamp() const              { return _timestamp; }
  void set_timestamp(long timestamp)  { _timestamp = timestamp; }  
    
  // Called by VM thread - does in turn invoke doit(). Do not override this
  void evaluate();  
    
  // evaluate() is called by the VMThread and in turn calls doit(). 
  // If the thread invoking VMThread::execute((VM_Operation*) is a JavaThread, 
  // doit_prologue() is called in that thread before transferring control to 
  // the VMThread.
  // If doit_prologue() returns true the VM operation will proceed, and 
  // doit_epilogue() will be called by the JavaThread once the VM operation 
  // completes. If doit_prologue() returns false the VM operation is cancelled.    
  virtual void doit()                            = 0;
  virtual bool doit_prologue()                   { return true; };
  virtual void doit_epilogue()                   {}; // Note: Not called if mode is: _concurrent

  // Type test
  virtual bool is_methodCompiler() const         { return false; }
  
  // Linking
  VM_Operation *next() const			 { return _next; }
  VM_Operation *prev() const                     { return _prev; }
  void set_next(VM_Operation *next)		 { _next = next; }
  void set_prev(VM_Operation *prev)		 { _prev = prev; }
  
  // Configuration. Override these appropriatly in subclasses.
  virtual const char* name() const                = 0;  
  virtual Mode evaluation_mode() const            { return _safepoint; }  
  virtual bool allow_nested_vm_operations() const { return false; }    
  virtual bool is_cheap_allocated() const         { return false; }
  virtual void oops_do(OopClosure* f)              { /* do nothing */ };

  // DO NOT OVERWRITE THE FOLLOWING METHODS (overwrite evaluation_mode)  
  bool evaluate_at_safepoint() const { return evaluation_mode() == _safepoint  || evaluation_mode() == _async_safepoint; }   
  bool evaluate_concurrently() const { return evaluation_mode() == _concurrent || evaluation_mode() == _async_safepoint; }       

  // Debugging
  void print_on_error(outputStream* st) const;
#ifndef PRODUCT
  void print_on(outputStream* st) const { print_on_error(st); }
#endif
};


class VM_GC_Operation: public VM_Operation {
 protected:
  BasicLock     _pending_list_basic_lock;    // for refs pending list notification
  bool          _notify_ref_lock;            // tells whether waiters on reference lock should be notified
  unsigned int  _gc_count_before;            // gc count before acquiring lock
  bool          _prologue_succeeded;         // tells whether doit_prologue succeeded.

  virtual bool gc_count_changed() const;

  // java.lang.ref.Reference support
  void acquire_pending_list_lock();
  void release_and_notify_pending_list_lock();

 public:
  VM_GC_Operation(int gc_count_before) {
    _prologue_succeeded = false;
    _gc_count_before    = gc_count_before;
  }
  ~VM_GC_Operation() {}
  
  // Acquire the reference synchronization lock
  virtual bool doit_prologue();
  // Do notifyAll (if needed) and release held lock
  virtual void doit_epilogue();

  virtual bool allow_nested_vm_operations() const  { return true; }
  bool prologue_succeeded() const { return _prologue_succeeded; }
};

// VM operation to invoke a collection of the heap as a
// GenCollectedHeap heap.
class VM_GenCollectFull: public VM_GC_Operation {
 private:
  int _max_level;
 public:
  VM_GenCollectFull(int gc_count_before, int max_level) 
    : VM_GC_Operation(gc_count_before) , _max_level(max_level) {}
  ~VM_GenCollectFull() {}
  virtual void doit();
  virtual const char* name() const { 
    return "full generation collection"; 
  }
};

class VM_GenCollectForAllocation: public VM_GC_Operation {
 private:
  HeapWord*   _res;
  size_t      _size;                       // size of object to be allocated.
  bool        _large_noref;                // alloc is large and will contain no refs.
  bool        _tlab;                       // alloc is of a tlab.
 public:
  VM_GenCollectForAllocation(size_t size,
                             bool large_noref,
                             bool tlab,
                             int gc_count_before)
    : VM_GC_Operation(gc_count_before),
      _size(size),
      _large_noref(large_noref),
      _tlab(tlab) {
    _res = NULL;
  }
  ~VM_GenCollectForAllocation()        {}
  virtual void doit();
  virtual const char* name() const { 
    return "generation collection for allocation"; 
  }
  HeapWord* result() const { return _res; }
};


class VM_ParallelGCFailedAllocation: public VM_GC_Operation {
 private:
  size_t    _size;
  bool      _is_noref;
  bool      _is_tlab;
  HeapWord* _result;

 public:
  VM_ParallelGCFailedAllocation(size_t size, bool is_noref, bool is_tlab, unsigned int gc_count);

  virtual void doit();
  virtual const char* name() const { 
    return "parallel gc failed allocation"; 
  }

  HeapWord* result() const { return _result; }
};

class VM_ParallelGCFailedPermanentAllocation: public VM_GC_Operation {
 private:
  size_t    _size;
  HeapWord* _result;

 public:
  VM_ParallelGCFailedPermanentAllocation(size_t size, unsigned int gc_count);

  virtual void doit();
  virtual const char* name() const { 
    return "parallel gc failed permanent allocation"; 
  }

  HeapWord* result() const { return _result; }
};

class VM_ParallelGCSystemGC: public VM_GC_Operation {
 public:
  VM_ParallelGCSystemGC(unsigned int gc_count);

  virtual void doit();
  virtual const char* name() const { 
    return "parallel gc system gc"; 
  }
};


class VM_ThreadStop: public VM_Operation {
 private:  
  oop     _thread;        // The Thread that the Throwable is thrown against
  oop     _throwable;     // The Throwable thrown at the target Thread  
 public:
  // All oops are passed as JNI handles, since there is no guarantee that a GC might happen before the
  // VM operation is executed.
  VM_ThreadStop(oop thread, oop throwable) {    
    _thread    = thread;  
    _throwable = throwable;
  }
  oop target_thread() const                      { return _thread; }
  oop throwable() const                          { return _throwable;}
  void doit();
  const char* name() const                       { return "thread stop"; }
  // We deoptimize if top-most frame is compiled - this might require a C2I adapter to be generated
  bool allow_nested_vm_operations() const        { return true; }
  Mode evaluation_mode() const                   { return _async_safepoint; }
  bool is_cheap_allocated() const                { return true; }

  // GC support
  void oops_do(OopClosure* f) {
    f->do_oop(&_thread); f->do_oop(&_throwable);
  }
};

// dummy vm op, evaluated just to force a safepoint
class VM_ForceSafepoint: public VM_Operation {
 public:
  VM_ForceSafepoint() { }  
  void doit() {}
  const char* name() const                       { return "VM_ForceSafepoint"; }  
};

// dummy vm op, evaluated just to force a safepoint
class VM_ForceAsyncSafepoint: public VM_Operation {
 public:
  VM_ForceAsyncSafepoint() { }  
  void doit() {}
  const char* name() const                       { return "VM_ForceAsyncSafepoint"; }
  Mode evaluation_mode() const                   { return _async_safepoint; }
  bool is_cheap_allocated() const                { return true; }
};

#ifndef CORE

class VM_Deoptimize: public VM_Operation {
 public:
  VM_Deoptimize() {}
  void doit();
  const char* name() const                       { return "deoptimize"; }
  bool allow_nested_vm_operations() const        { return true; }
};

class VM_DeoptimizeFrame: public VM_Operation {
 private:
  JavaThread* _thread;
  intptr_t*   _id;
 public:
  VM_DeoptimizeFrame(JavaThread* thread, intptr_t* id);
  void doit();
  const char* name() const                       { return "deoptimize frame"; }
  bool allow_nested_vm_operations() const        { return true;  }
};

#ifndef PRODUCT
class VM_DeoptimizeAll: public VM_Operation {
 private:
  KlassHandle _dependee;
 public:
  VM_DeoptimizeAll() {}
  void doit();
  const char* name() const                       { return "deoptimize-all"; }
  bool allow_nested_vm_operations() const        { return true; }
};


class VM_ZombieAll: public VM_Operation {
 public:
  VM_ZombieAll() {}
  void doit();
  const char* name() const                       { return "zombie-all"; }
  bool allow_nested_vm_operations() const        { return true; }
};
#endif // PRODUCT
#endif // CORE


#ifndef PRODUCT
class VM_Verify: public VM_Operation {
 private:
  KlassHandle _dependee;
 public:
  VM_Verify() {}
  void doit();
  const char* name() const                       { return "verify"; }
};
#endif // PRODUCT


class VM_PrintThreads: public VM_Operation {
 public:
  VM_PrintThreads() {}  
  void doit();
  const char* name() const { return "print threads"; }
};

class VM_FindDeadlocks: public VM_Operation {
 private:
  GrowableArray<JavaThread*>* _deadlock_threads;
  bool      _print_to_tty;
  enum {
    _initial_array_size = 10
  };

 public:
  VM_FindDeadlocks() : _deadlock_threads(NULL), _print_to_tty(true) {}
  VM_FindDeadlocks(bool save_result, bool print_to_tty);
  ~VM_FindDeadlocks();

  GrowableArray<JavaThread*>* result() { return _deadlock_threads; }

  void doit();
  const char* name() const { return "find deadlocks"; }
};

class VM_GC_HeapInspection: public VM_GC_Operation {
 public:
  VM_GC_HeapInspection();
  ~VM_GC_HeapInspection() {}
  virtual void doit();
  virtual const char* name() const {
    return "heap inspection";
  }
};

class ThreadDumpResult;
class ThreadSnapshot;
class VM_ThreadDump : public VM_Operation {
private:
  GrowableArray<instanceHandle>* _threads;
  ThreadSnapshot**               _snapshots;
  int                            _num_threads;
  int                            _max_depth;
  ThreadDumpResult*              _result;
  Thread*                        _req_thread;
public:
  VM_ThreadDump(GrowableArray<instanceHandle>* threads, 
                ThreadSnapshot** snapshots, 
                int num_threads, 
                int max_depth, 
                ThreadDumpResult* result,
                Thread* req_thread);
  void doit();
  const char* name() const    { return "thread dump"; }
};


class VM_Exit: public VM_Operation {
 private:
  int  _exit_code;
  static volatile bool _vm_exited;
  static Thread * _shutdown_thread;
  static void wait_if_vm_exited();
 public:
  VM_Exit(int exit_code) {
    _exit_code = exit_code;
  }
  static int wait_for_threads_in_native_to_block();
  static int set_vm_exited();
  static bool vm_exited()                      { return _vm_exited; }
  static void block_if_vm_exited() {
    if (_vm_exited) {
      wait_if_vm_exited();
    }
  }
  void doit();
  const char* name() const                     { return "exit"; }
};
