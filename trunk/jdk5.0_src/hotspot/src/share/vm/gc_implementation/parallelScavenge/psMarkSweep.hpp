#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)psMarkSweep.hpp	1.17 04/03/28 16:19:30 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class PSAdaptiveSizePolicy;
class PSYoungGen;
class PSOldGen;

class PSMarkSweep : public MarkSweep {
 private:
  static elapsedTimer        _accumulated_time;
  static unsigned int        _total_invocations;
  static jlong               _time_of_last_gc;   // ms
  static CollectorCounters*  _counters;

  // Closure accessors
  static OopClosure* mark_and_push_closure() { return &MarkSweep::mark_and_push_closure; }
  static VoidClosure* follow_stack_closure() { return (VoidClosure*)&MarkSweep::follow_stack_closure; }
  static OopClosure* adjust_pointer_closure() { return (OopClosure*)&MarkSweep::adjust_pointer_closure; }
  static OopClosure* adjust_root_pointer_closure() { return (OopClosure*)&MarkSweep::adjust_root_pointer_closure; }
  static BoolObjectClosure* is_alive_closure() { return (BoolObjectClosure*)&MarkSweep::is_alive; }

  // Mark live objects
  static void mark_sweep_phase1(bool& marked_for_deopt, bool clear_all_softrefs);
  // Calculate new addresses
  static void mark_sweep_phase2();
  // Update pointers
  static void mark_sweep_phase3();
  // Move objects to new positions
  static void mark_sweep_phase4();

  // Temporary data structures for traversal and storing/restoring marks
  static void allocate_stacks();
  static void deallocate_stacks();

  // If objects are left in eden after a collection, try to move the boundary
  // and absorb them into the old gen.  Returns true if eden was emptied.
  static bool absorb_live_data_from_eden(PSAdaptiveSizePolicy* size_policy,
					 PSYoungGen* young_gen,
					 PSOldGen* old_gen);

  // Reset time since last full gc
  static void reset_millis_since_last_gc();

 public:
  static void invoke(bool* notify_ref_lock, 
		     bool clear_all_softrefs);
  static void invoke_no_policy(bool* notify_ref_lock, 
			       bool clear_all_softrefs);

  static void initialize();

  // Public accessors
  static elapsedTimer* accumulated_time() { return &_accumulated_time; }
  static unsigned int total_invocations() { return _total_invocations; }
  static CollectorCounters* counters()    { return _counters; }

  // Time since last full gc (in milliseconds)
  static jlong millis_since_last_gc();
};

