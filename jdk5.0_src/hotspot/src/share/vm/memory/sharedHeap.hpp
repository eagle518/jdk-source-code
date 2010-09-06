#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)sharedHeap.hpp	1.44 04/03/28 16:18:07 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// A "SharedHeap" is an implementation of a java heap for HotSpot.  This
// is an abstract class: there may be many different kinds of heaps.  This
// class defines the functions that a heap must implement, and contains
// infrastructure common to all heaps.

class PermGen;
class Generation;
class BarrierSet;
class GenRemSet;
class Space;
class SpaceClosure;
class OopClosure;
class OopsInGenClosure;
class ObjectClosure;
class SubTasksDone;
class WorkGang;
class CollectorPolicy;
class KlassHandle;

class SharedHeap : public CollectedHeap {
  friend class VMStructs;

protected:
  // There should be only a single instance of "SharedHeap" in a program.
  // This is enforced with the protected constructor below, which will also
  // set the static pointer "_sh" to that instance.
  static SharedHeap* _sh;

  // All heaps contain a "permanent generation."  This is some ways
  // similar to a generation in a generational system, in other ways not.
  // See the "PermGen" class.
  PermGen* _perm_gen;

  // and the Gen Remembered Set, at least one good enough to scan the perm
  // gen.
  GenRemSet* _rem_set;

  // A gc policy, controls global gc resource issues
  CollectorPolicy *_collector_policy;

  // See the discussion below, in the specification of the reader function
  // for this variable.
  int _strong_roots_parity;

  // If we're doing parallel GC, use this gang of threads.
  WorkGang* _workers;

  // Number of parallel threads currently working on GC tasks.
  // O indicates use sequential code; 1 means use parallel code even with
  // only one thread, for performance testing purposes.
  int _n_par_threads;

  // Full initialization is done in a concrete subtype's "initialize"
  // function.
  SharedHeap(CollectorPolicy* policy_);

public:
  static SharedHeap* heap() { return _sh; }

  CollectorPolicy *collector_policy() const { return _collector_policy; }

  void set_barrier_set(BarrierSet* bs);

  // Does operations required after initialization has been done.
  virtual void post_initialize() = 0;

  // Initialization of ("weak") reference processing support
  virtual void ref_processing_init();

  void set_perm(PermGen* perm_gen) { _perm_gen = perm_gen; }

  // A helper function that fills an allocated-but-not-yet-initialized
  // region with a garbage object.
  static void fill_region_with_object(MemRegion mr);

  // This function returns the "GenRemSet" object that allows us to scan
  // generations; at least the perm gen, possibly more in a fully
  // generational heap.
  GenRemSet* rem_set() { return _rem_set; }

  // These function return the "permanent" generation, in which
  // reflective objects are allocated and stored.  Two versions, the second
  // of which returns the view of the perm gen as a generation.
  PermGen* perm() const { return _perm_gen; }
  Generation* perm_gen() const { return _perm_gen->as_gen(); }

  // Iteration functions.
  void oop_iterate(OopClosure* cl) = 0;

  // Same as above, restricted to a memory region.
  virtual void oop_iterate(MemRegion mr, OopClosure* cl) = 0;

  // Iterate over all objects allocated since the last collection, calling
  // "cl->do_object" on each.  The heap must have been initialized properly
  // to support this function, or else this call will fail.
  virtual void object_iterate_since_last_GC(ObjectClosure* cl) = 0;

  // Iterate over all spaces in use in the heap, in an undefined order.
  virtual void space_iterate(SpaceClosure* cl) = 0;

  // A SharedHeap will contain some number of spaces.  This finds the
  // space whose reserved area contains the given address, or else returns
  // NULL.
  virtual Space* space_containing(const void* addr) const = 0;

  bool no_gc_in_progress() { return !is_gc_active(); }

  // Some collectors will perform "process_strong_roots" in parallel.
  // Such a call will involve claiming some fine-grained tasks, such as
  // scanning of threads.  To make this process simpler, we provide the
  // "strong_roots_parity()" method.  Collectors that start parallel tasks
  // whose threads invoke "process_strong_roots" must 
  // call "change_strong_roots_parity" in sequential code starting such a
  // task.  (This also means that a parallel thread may only call
  // process_strong_roots once.)
  // 
  // For calls to process_strong_roots by sequential code, the parity is
  // updated automatically.
  // 
  // The idea is that objects representing fine-grained tasks, such as
  // threads, will contain a "parity" field.  A task will is claimed in the 
  // current "process_strong_roots" call only if its parity field is the
  // same as the "strong_roots_parity"; task claiming is accomplished by
  // updating the parity field to the strong_roots_parity with a CAS.
  // 
  // If the client meats this spec, then strong_roots_parity() will have
  // the following properties:
  //   a) to return a different value than was returned before the last
  //      call to change_strong_roots_parity, and
  //   c) to never return a distinguished value (zero) with which such
  //      task-claiming variables may be initialized, to indicate "never
  //      claimed".
  void change_strong_roots_parity();
  int strong_roots_parity() { return _strong_roots_parity; }

  WorkGang* workers() const { return _workers; }

  // Sets the number of parallel threads that will be doing tasks
  // (such as process strong roots) subsequently.
  virtual void set_par_threads(int t);

  // Number of threads currently working on GC tasks.
  int n_par_threads() { return _n_par_threads; }

  // Like CollectedHeap::collect, but assume that the caller holds the Heap_lock.
  virtual void collect_locked(GCCause::Cause cause) = 0;

  // The functions below are helper functions that a subclass of
  // "SharedHeap" can use in the implementation of its virtual
  // functions.

protected:

  // Do anything common to GC's.
  virtual void gc_prologue(bool full) = 0;
  virtual void gc_epilogue(bool full) = 0;

  //
  // New methods from CollectedHeap
  //

  size_t permanent_capacity() const {
    assert(perm_gen(), "NULL perm gen");
    return perm_gen()->capacity();
  }

  size_t permanent_used() const {
    assert(perm_gen(), "NULL perm gen");
    return perm_gen()->used();
  }

  bool is_in_permanent(const void *p) const {
    assert(perm_gen(), "NULL perm gen");
    return perm_gen()->is_in_reserved(p);
  }

  HeapWord* permanent_mem_allocate(size_t size) {
    assert(perm_gen(), "NULL perm gen");
    return _perm_gen->mem_allocate(size);
  }

  void permanent_oop_iterate(OopClosure* cl) {
    assert(perm_gen(), "NULL perm gen");
    _perm_gen->oop_iterate(cl);
  }

  void permanent_object_iterate(ObjectClosure* cl) {
    assert(perm_gen(), "NULL perm gen");
    _perm_gen->object_iterate(cl);
  }
};


