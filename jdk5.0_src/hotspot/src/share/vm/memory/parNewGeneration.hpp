#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)parNewGeneration.hpp	1.33 03/12/23 16:41:23 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// It would be better if these types could be kept local to the .cpp file,
// but they must be here to allow ParScanClosure::do_oop_work to be defined 
// in genOopClosures.inline.hpp.


typedef OopTaskQueue    ObjToScanQueue;
typedef OopTaskQueueSet ObjToScanQueueSet;

// Enable this to get push/pop/steal stats.
const int PAR_STATS_ENABLED = 0;

// The state needed by thread performing parallel young-gen collection.
class ParScanThreadState: public StackObj {
  ObjToScanQueue *_work_queue;

  ParGCAllocBuffer _to_space_alloc_buffer;

  ParScanWithoutBarrierClosure* _to_space_closure;
  ParScanWithBarrierClosure*    _old_gen_closure;
  OopsInGenClosure*             _to_space_root_closure;
  OopsInGenClosure*             _old_gen_root_closure;

  Space* _to_space;
  Space* to_space() { return _to_space; }

  Generation* _old_gen;
  Generation* old_gen() { return _old_gen; }

  HeapWord *_young_old_boundary;

  int _hash_seed;
  int _thread_num;
  ageTable _ageTable;

  bool _to_space_full;

  int _pushes, _pops, _steals, _steal_attempts, _term_attempts;
  int _overflow_pushes, _overflow_refills, _overflow_refill_objs;

  // Timing numbers.
  double _start;
  double _start_strong_roots;
  double _strong_roots_time;
  double _start_term;
  double _term_time;

  // Helper for trim_queues. Scans subset of an array and makes
  // remainder available for work stealing.
  void scan_partial_array_and_push_remainder(oop obj);

public:
  ParScanThreadState(Space* to_space_, Generation* old_gen_, int thread_num_,
    ObjToScanQueue* work_queue_);

  ageTable* age_table() {return &_ageTable;}
  
  ObjToScanQueue* work_queue() { return _work_queue; }

  ParGCAllocBuffer* to_space_alloc_buffer() {
    return &_to_space_alloc_buffer;
  }
  void set_to_space_closure(ParScanWithoutBarrierClosure* cl) {
    _to_space_closure = cl;
  }
  void set_old_gen_closure(ParScanWithBarrierClosure* cl) {
    _old_gen_closure = cl;
  }
  void set_to_space_root_closure(OopsInGenClosure* cl) {
    _to_space_root_closure = cl;
  }
  void set_old_gen_root_closure(OopsInGenClosure* cl) {
    _old_gen_root_closure = cl;
  }

  // Decrease queue size below "max_size".
  void trim_queues(int max_size);

  // Is new_obj a candidate for scan_partial_array_and_push_remainder method.
  inline bool should_be_partially_scanned(oop new_obj, oop old_obj) const;

  int* hash_seed()  { return &_hash_seed; }
  int  thread_num() { return _thread_num; }

  // Allocate a to-space block of size "sz", or else return NULL.
  HeapWord* alloc_in_to_space_slow(size_t word_sz);

  HeapWord* alloc_in_to_space(size_t word_sz) {
    HeapWord* obj = to_space_alloc_buffer()->allocate(word_sz);
    if (obj != NULL) return obj;
    else return alloc_in_to_space_slow(word_sz);
  }

  HeapWord* young_old_boundary() { return _young_old_boundary; }

  void set_young_old_boundary(HeapWord *boundary) {
    _young_old_boundary = boundary;
  }

  // Undo the most recent allocation ("obj", of "word_sz").
  void undo_alloc_in_to_space(HeapWord* obj, size_t word_sz);

  int pushes() { return _pushes; }
  int pops()   { return _pops; }
  int steals() { return _steals; }
  int steal_attempts() { return _steal_attempts; }
  int term_attempts()  { return _term_attempts; }
  int overflow_pushes() { return _overflow_pushes; }
  int overflow_refills() { return _overflow_refills; }
  int overflow_refill_objs() { return _overflow_refill_objs; }

  void note_push()  { if (PAR_STATS_ENABLED) _pushes++; }
  void note_pop()   { if (PAR_STATS_ENABLED) _pops++; }
  void note_steal() { if (PAR_STATS_ENABLED) _steals++; }
  void note_steal_attempt() { if (PAR_STATS_ENABLED) _steal_attempts++; }
  void note_term_attempt()  { if (PAR_STATS_ENABLED) _term_attempts++; }
  void note_overflow_push() { if (PAR_STATS_ENABLED) _overflow_pushes++; }
  void note_overflow_refill(int objs) {
    if (PAR_STATS_ENABLED) {
      _overflow_refills++;
      _overflow_refill_objs += objs;
    }
  }

  void start_strong_roots() {
    _start_strong_roots = os::elapsedTime();
  }
  void end_strong_roots() {
    _strong_roots_time += (os::elapsedTime() - _start_strong_roots);
  }
  double strong_roots_time() { return _strong_roots_time; }
  void start_term_time() {
    note_term_attempt();
    _start_term = os::elapsedTime();
  }
  void end_term_time() {
    _term_time += (os::elapsedTime() - _start_term);
  }
  double term_time() { return _term_time; }

  double elapsed() {
    return os::elapsedTime() - _start;
  }

};

// A Generation that does parallel young-gen collection.

class ParNewGeneration: public DefNewGeneration {
  friend class ParNewGenTask;

  class ParKeepAliveClosure: public DefNewGeneration::KeepAliveClosure {
  public:
    ParKeepAliveClosure(ScanWeakRefClosure* cl);
    void do_oop(oop* p);
  };

  // XXX use a global constant instead of 64!
  struct ObjToScanQueuePadded {
        ObjToScanQueue work_queue;
        char pad[64 - sizeof(ObjToScanQueue)];  // prevent false sharing
  };

  // The per-thread work queues, available here for stealing.
  ObjToScanQueueSet* _task_queues;

  // A list of from-space images of to-be-scanned objects, threaded through 
  // klass-pointers (klass information already copied to the forwarded
  // image.)  Manipulated with CAS.
  oop _overflow_list;

  // If true, older generation does not support promotion undo, so avoid.
  static bool _avoid_promotion_undo;

  static oop real_forwardee_slow(oop obj);
  static void waste_some_time();

  // Preserve the mark of "obj", if necessary, in preparation for its mark 
  // word being overwritten with a self-forwarding-pointer. 
  void preserve_mark_if_necessary(oop obj, markOop m);

public:
  ParNewGeneration(ReservedSpace rs, size_t initial_byte_size, int level);

  ~ParNewGeneration() {
    for (uint i = 0; i < ParallelGCThreads; i++)
        delete _task_queues->queue(i);

    delete _task_queues;
  }

  virtual Generation::Name kind()        { return Generation::ParNew; }
  virtual const char* name() const;
  virtual const char* short_name() const { return "ParNew"; }

  // override
  virtual bool refs_discovery_is_mt()     const {
    assert(UseParNewGC, "ParNewGeneration only when UseParNewGC");
    return ParallelGCThreads > 1;
  }

  // Make the collection virtual.
  virtual void collect(bool   full,
                       bool   clear_all_soft_refs,
                       size_t size, 
		       bool   is_large_noref,
                       bool   is_tlab);

  // This needs to be visible to the closure function.
  // "obj" is the object to be copied, "m" is a recent value of its mark
  // that must not contain a forwarding pointer (though one might be
  // inserted in "obj"s mark word by a parallel thread).
  inline oop copy_to_survivor_space(ParScanThreadState* par_scan_state,
			     oop obj, size_t obj_sz, markOop m,
			     bool jvmpi_slow_alloc) {
    if (_avoid_promotion_undo) {
       return copy_to_survivor_space_avoiding_promotion_undo(par_scan_state,
                                         		     obj, obj_sz, m, jvmpi_slow_alloc);
    }

    return copy_to_survivor_space_with_undo(par_scan_state, obj, obj_sz, m, jvmpi_slow_alloc);
  }

  oop copy_to_survivor_space_avoiding_promotion_undo(ParScanThreadState* par_scan_state,
			     oop obj, size_t obj_sz, markOop m,
			     bool jvmpi_slow_alloc);

  oop copy_to_survivor_space_with_undo(ParScanThreadState* par_scan_state,
			     oop obj, size_t obj_sz, markOop m,
			     bool jvmpi_slow_alloc);

  // Push the given (from-space) object on the global overflow list.
  void push_on_overflow_list(oop from_space_obj);

  // If the global overflow list is non-empty, move some tasks from it
  // onto "work_q" (which must be empty).  No more than 1/4 of the
  // max_elems of "work_q" are moved.
  bool take_from_overflow_list(ParScanThreadState* par_scan_state);

  // The task queues to be used by parallel GC threads.
  ObjToScanQueueSet* task_queues() {
    return _task_queues;
  }

  static oop real_forwardee(oop obj);

  DEBUG_ONLY(static bool is_legal_forward_ptr(oop p);)
};


