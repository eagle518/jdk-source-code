#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)genMarkSweep.hpp	1.4 03/12/23 16:41:11 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class GenMarkSweep : public MarkSweep {
  friend class VM_MarkSweep;
 public:
  static void invoke_at_safepoint(int level, ReferenceProcessor* rp,
                                  bool clear_all_softrefs);

 private:

  // Mark live objects
  static void mark_sweep_phase1(int level, bool& marked_for_deopt, 
				bool clear_all_softrefs);
  // Calculate new addresses
  static void mark_sweep_phase2();
  // Update pointers
  static void mark_sweep_phase3(int level);
  // Move objects to new positions
  static void mark_sweep_phase4();

  // Temporary data structures for traversal and storing/restoring marks
  static void allocate_stacks();
  static void deallocate_stacks();
};
