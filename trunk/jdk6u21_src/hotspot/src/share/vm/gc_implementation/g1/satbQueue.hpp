/*
 * Copyright (c) 2001, 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

class ObjectClosure;
class JavaThread;

// A ptrQueue whose elements are "oops", pointers to object heads.
class ObjPtrQueue: public PtrQueue {
public:
  ObjPtrQueue(PtrQueueSet* qset_, bool perm = false) :
    PtrQueue(qset_, perm, qset_->is_active()) { }
  // Apply the closure to all elements, and reset the index to make the
  // buffer empty.
  void apply_closure(ObjectClosure* cl);

  // Apply the closure to all elements of "buf", down to "index" (inclusive.)
  static void apply_closure_to_buffer(ObjectClosure* cl,
                                      void** buf, size_t index, size_t sz);

  void verify_oops_in_buffer() NOT_DEBUG_RETURN;
};



class SATBMarkQueueSet: public PtrQueueSet {
  ObjectClosure* _closure;
  ObjectClosure** _par_closures;  // One per ParGCThread.

  ObjPtrQueue _shared_satb_queue;

  // Utility function to support sequential and parallel versions.  If
  // "par" is true, then "worker" is the par thread id; if "false", worker
  // is ignored.
  bool apply_closure_to_completed_buffer_work(bool par, int worker);

#ifdef ASSERT
  void dump_active_values(JavaThread* first, bool expected_active);
#endif // ASSERT

public:
  SATBMarkQueueSet();

  void initialize(Monitor* cbl_mon, Mutex* fl_lock,
                  int process_completed_threshold,
                  Mutex* lock);

  static void handle_zero_index_for_thread(JavaThread* t);

  // Apply "set_active(b)" to all Java threads' SATB queues. It should be
  // called only with the world stopped. The method will assert that the
  // SATB queues of all threads it visits, as well as the SATB queue
  // set itself, has an active value same as expected_active.
  void set_active_all_threads(bool b, bool expected_active);

  // Register "blk" as "the closure" for all queues.  Only one such closure
  // is allowed.  The "apply_closure_to_completed_buffer" method will apply
  // this closure to a completed buffer, and "iterate_closure_all_threads"
  // applies it to partially-filled buffers (the latter should only be done
  // with the world stopped).
  void set_closure(ObjectClosure* closure);
  // Set the parallel closures: pointer is an array of pointers to
  // closures, one for each parallel GC thread.
  void set_par_closure(int i, ObjectClosure* closure);

  // If there is a registered closure for buffers, apply it to all entries
  // in all currently-active buffers.  This should only be applied at a
  // safepoint.  (Currently must not be called in parallel; this should
  // change in the future.)
  void iterate_closure_all_threads();
  // Parallel version of the above.
  void par_iterate_closure_all_threads(int worker);

  // If there exists some completed buffer, pop it, then apply the
  // registered closure to all its elements, and return true.  If no
  // completed buffers exist, return false.
  bool apply_closure_to_completed_buffer() {
    return apply_closure_to_completed_buffer_work(false, 0);
  }
  // Parallel version of the above.
  bool par_apply_closure_to_completed_buffer(int worker) {
    return apply_closure_to_completed_buffer_work(true, worker);
  }

  ObjPtrQueue* shared_satb_queue() { return &_shared_satb_queue; }

  // If a marking is being abandoned, reset any unprocessed log buffers.
  void abandon_partial_marking();

};
