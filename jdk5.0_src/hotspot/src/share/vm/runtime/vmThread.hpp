#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)vmThread.hpp	1.29 03/12/23 16:44:30 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

//
// Prioritized queue of VM operations.
//
// Encapsulates both queue management and
// and priority policy
//
class VMOperationQueue : public CHeapObj {
 private:
  enum Priorities {
     SafepointPriority, // Highest priority (operation executed at a safepoint)
     MediumPriority,    // Medium priority
     nof_priorities 
  };

  // We maintain a doubled linked list, with explicit count.
  int           _queue_length[nof_priorities];
  int           _queue_counter;
  VM_Operation* _queue       [nof_priorities];  
  
  // Double-linked non-empty list insert.
  void insert(VM_Operation* q,VM_Operation* n);
  void unlink(VM_Operation* q);

  // Basic queue manipulation
  bool queue_empty                (int prio);
  void queue_add_front            (int prio, VM_Operation *op);
  void queue_add_back             (int prio, VM_Operation *op);
  VM_Operation* queue_remove_front(int prio);  
  void queue_oops_do(int queue, OopClosure* f);
    
 public:
  VMOperationQueue();

  // Highlevel operations. Encapsulates policy
  bool add(VM_Operation *op);
  VM_Operation* remove_next();                        // Returns next or null
  VM_Operation* remove_next_at_safepoint_priority()   { return queue_remove_front(SafepointPriority); }

  // GC support
  void oops_do(OopClosure* f);

  void verify_queue(int prio) PRODUCT_RETURN;
};


//
// A single VMThread (the primordial thread) spawns all other threads
// and is itself used by other threads to offload heavy vm operations
// like scavenge, garbage_collect etc.
//

class VMThread: public Thread {
 private:
  static ThreadPriority _current_priority;

  static bool _should_terminate;
  static bool _terminated;
  static Monitor * _terminate_lock;

  static bool should_terminate()                 { return _should_terminate; }

  void evaluate_operation(VM_Operation* op);
 public:
  // Constructor
  VMThread();

  // Tester
  bool is_VM_thread() const                      { return true; }
  bool is_GC_thread() const                      { return true; }

  char* name() const { return (char*)"VM Thread"; }
  
  // The ever running loop for the VMThread
  void loop();

  // Called to stop the VM thread
  static void wait_for_vm_thread_exit();
  static bool is_terminated()                     { return _terminated == true; }

  // Execution of vm operation
  static void execute(VM_Operation* op);

  // Returns the current vm operation if any.
  static VM_Operation* vm_operation()             { return _cur_vm_operation;   }  

  // Returns the single instance of VMThread.
  static VMThread* vm_thread()                    { return _vm_thread; }

  // GC support
  void oops_do(OopClosure* f);

  // Debugging
  void print();
  void verify();

  // Entry for starting vm thread
  virtual void run();

  // Creations/Destructions
  static void create();
  static void destroy();

 private:   
  // VM_Operation support  
  static VM_Operation*     _cur_vm_operation;   // Current VM operation
  static VMOperationQueue* _vm_queue;           // Queue (w/ policy) of VM operations
  
  // Pointer to single-instance of VM thread
  static VMThread*     _vm_thread;
};


