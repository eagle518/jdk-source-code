#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)psPromotionManager.hpp	1.8 04/04/20 13:30:01 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

//
// psPromotionManager is used by a single thread to manage object survival
// during a scavenge. The promotion manager contains thread local data only.
//
// NOTE! Be carefull when allocating the stacks on cheap. If you are going
// to use a promotion manager in more than one thread, the stacks MUST be
// on cheap. This can lead to memory leaks, though, as they are not auto
// deallocated.
//
// FIX ME FIX ME Add a destructor, and don't rely on the user to drain/flush/deallocate!
//

class MutableSpace;
class PSOldGen;

class PSPromotionManager : public CHeapObj {
 private:
  static PSPromotionManager**  _manager_array;
  static OopTaskQueueSet*      _stack_array;
  static PSOldGen*             _old_gen;
  static MutableSpace*         _young_space;
  static int		       _promotion_failure_a_lot_count;
  
  PSYoungPromotionLAB          _young_lab;
  PSOldPromotionLAB            _old_lab;
  bool                         _young_gen_is_full;
  bool                         _old_gen_is_full;
  PrefetchQueue                _prefetch_queue;
  OopTaskQueue                 _claimed_stack;
  GrowableArray<oop>*          _overflow_stack;

  // Accessors
  static PSOldGen* old_gen()              { return _old_gen; }
  static MutableSpace* young_space()      { return _young_space; }
  static OopTaskQueueSet* stack_array()   { return _stack_array; }

  inline static PSPromotionManager* manager_array(int index);
  inline void claim_or_forward_internal(oop* p);

  GrowableArray<oop>* overflow_stack()    { return _overflow_stack; }

 public:
  // Static
  static void initialize();

  static void pre_scavenge();
  static void post_scavenge();

  static PSPromotionManager* gc_thread_promotion_manager(int index);
  static PSPromotionManager* vm_thread_promotion_manager();

  static bool steal(int queue_num, int* seed, Task& t) {
    return stack_array()->steal(queue_num, seed, t);
  }

  PSPromotionManager();

  // Accessors
  OopTaskQueue* claimed_stack()        { return &_claimed_stack; }

  bool young_gen_is_full()             { return _young_gen_is_full; }

  bool old_gen_is_full()               { return _old_gen_is_full; }
  void set_old_gen_is_full(bool state) { _old_gen_is_full = state; }

  // Note that access to _promotion_failure_a_lot_count is
  // not atomic.
  void set_promotion_failure_a_lot_count(int v) {
    _promotion_failure_a_lot_count = v;
  }

  // Pre-decrement and return the new value.
  int dec_promotion_failure_a_lot_count() { 
    return --_promotion_failure_a_lot_count; 
  }

  // Promotion methods
  oop copy_to_survivor_space(oop o);
  oop oop_promotion_failed(oop obj, markOop obj_mark);
  void handle_stack_overflow();

  void reset();

  void flush_labs();
  void drain_stacks();

  void allocate_stacks(int stack_size);
  void deallocate_stacks();

  inline void flush_prefetch_queue();
  inline void claim_or_forward(oop* p);
};



