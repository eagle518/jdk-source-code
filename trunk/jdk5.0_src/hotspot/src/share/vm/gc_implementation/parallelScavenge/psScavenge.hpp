#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)psScavenge.hpp	1.36 04/03/28 16:19:31 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class GCTaskManager;
class GCTaskQueue;
class OopStack;
class ReferenceProcessor;

class PSScavenge: AllStatic {
  friend class PSIsAliveClosure;
  friend class PSKeepAliveClosure;
  friend class PSPromotionManager;

 enum ScavengeSkippedCause {
   not_skipped = 0,
   to_space_not_empty,
   promoted_too_large,
   full_follows_scavenge
 };

  // Number of consecutive attempts to scavenge that were skipped
  static int		    _consecutive_skipped_scavenges;


 protected:
  // Flags/counters
  static ReferenceProcessor* _ref_processor;        // Reference processor for scavenging.
  static CardTableExtension* _card_table;           // We cache the card table for fast access.
  static bool                _survivor_overflow;    // Overflow this collection
  static int                 _tenuring_threshold;   // tenuring threshold for next scavenge
  static elapsedTimer        _accumulated_time;     // total time spent on scavenge
  static HeapWord*           _young_generation_boundary; // The lowest address possible for the young_gen.
                                                         // This is used to decide if an oop should be scavenged, 
                                                         // cards should be marked, etc.
  static GCTaskManager*          _gc_task_manager;      // The task manager.
  static GrowableArray<markOop>* _preserved_mark_stack; // List of marks to be restored after failed promotion
  static GrowableArray<oop>*     _preserved_oop_stack;  // List of oops that need their mark restored.
  static CollectorCounters*      _counters;         // collector performance counters

  static void clean_up_failed_promotion();

  static bool should_attempt_scavenge();

  // Private accessors
  static CardTableExtension* const card_table()       { assert(_card_table != NULL, "Sanity"); return _card_table; }

 public:
  // Accessors
  static int              tenuring_threshold()  { return _tenuring_threshold; }
  static elapsedTimer*    accumulated_time()    { return &_accumulated_time; }
  static bool             promotion_failed()    
    { return _preserved_mark_stack != NULL; }
  static int		  consecutive_skipped_scavenges() 
    { return _consecutive_skipped_scavenges; }

  // Performance Counters
  static CollectorCounters* counters()           { return _counters; }

  // Used by scavenge_contents && psMarkSweep
  static ReferenceProcessor* const reference_processor() {
    assert(_ref_processor != NULL, "Sanity"); 
    return _ref_processor;
  }
  // Used to add tasks
  static GCTaskManager* const gc_task_manager() {
    assert(_gc_task_manager != NULL, "shouldn't return NULL");
    return _gc_task_manager;
  }
  // The promotion managers tell us if they encountered overflow
  static void set_survivor_overflow(bool state) {
    _survivor_overflow = state; 
  }
  // Adaptive size policy support.  When the young generation/old generation
  // boundary moves, _young_generation_boundary must be reset
  static void set_young_generation_boundary(HeapWord* v) {
    _young_generation_boundary = v;
  }

  // Called by parallelScavengeHeap to init the tenuring threshold
  static void initialize();

  // Scavenge entry point
  static void invoke(bool* notify_ref_lock);
  // Return true is a collection was done.  Return
  // false if the collection was skipped.
  static bool invoke_no_policy(bool* notify_ref_lock);

  // If an attempt to promote fails, this method is invoked
  static void oop_promotion_failed(oop obj, markOop obj_mark);

  inline static bool should_scavenge(oop p);

  inline static void copy_and_push_safe_barrier(PSPromotionManager* pm, oop* p);

  // Is an object in the young generation
  // This assumes that the HeapWord argument is in the heap, 
  // so it only checks one side of the complete predicate.
  inline static bool is_obj_in_young(HeapWord* o) {
    const bool result = (o >= _young_generation_boundary);
    return result;
  }
};
