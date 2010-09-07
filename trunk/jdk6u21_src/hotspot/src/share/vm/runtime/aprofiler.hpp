/*
 * Copyright (c) 1997, 2003, Oracle and/or its affiliates. All rights reserved.
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

// A simple allocation profiler for Java. The profiler collects and prints
// the number and total size of instances allocated per class, including
// array classes.
//
// The profiler is currently global for all threads. It can be changed to a
// per threads profiler by keeping a more elaborate data structure and calling
// iterate_since_last_scavenge at thread switches.


class AllocationProfiler: AllStatic {
  friend class GenCollectedHeap;
  friend class G1CollectedHeap;
  friend class MarkSweep;
 private:
  static bool _active;                          // tells whether profiler is active
  static GrowableArray<klassOop>* _print_array; // temporary array for printing

  // Utility printing functions
  static void add_class_to_array(klassOop k);
  static void add_classes_to_array(klassOop k);
  static int  compare_classes(klassOop* k1, klassOop* k2);
  static int  average(size_t alloc_size, int alloc_count);
  static void sort_and_print_array(size_t cutoff);

  // Call for collecting allocation information. Called at scavenge, mark-sweep and disengage.
  static void iterate_since_last_gc();

 public:
  // Start profiler
  static void engage();
  // Stop profiler
  static void disengage();
  // Tells whether profiler is active
  static bool is_active()                   { return _active; }
  // Print profile
  static void print(size_t cutoff);   // Cutoff in total allocation size (in words)
};
