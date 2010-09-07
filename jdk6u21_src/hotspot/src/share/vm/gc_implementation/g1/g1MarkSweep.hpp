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

class ReferenceProcessor;

// G1MarkSweep takes care of global mark-compact garbage collection for a
// G1CollectedHeap using a four-phase pointer forwarding algorithm.  All
// generations are assumed to support marking; those that can also support
// compaction.
//
// Class unloading will only occur when a full gc is invoked.


class G1MarkSweep : AllStatic {
  friend class VM_G1MarkSweep;
  friend class Scavenge;

 public:

  static void invoke_at_safepoint(ReferenceProcessor* rp,
                                  bool clear_all_softrefs);

 private:

  // Mark live objects
  static void mark_sweep_phase1(bool& marked_for_deopt,
                                bool clear_all_softrefs);
  // Calculate new addresses
  static void mark_sweep_phase2();
  // Update pointers
  static void mark_sweep_phase3();
  // Move objects to new positions
  static void mark_sweep_phase4();

  static void allocate_stacks();
};
