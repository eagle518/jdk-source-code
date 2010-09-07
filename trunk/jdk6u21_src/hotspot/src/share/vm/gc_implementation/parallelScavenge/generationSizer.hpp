/*
 * Copyright (c) 2001, 2008, Oracle and/or its affiliates. All rights reserved.
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

// There is a nice batch of tested generation sizing code in
// TwoGenerationCollectorPolicy. Lets reuse it!

class GenerationSizer : public TwoGenerationCollectorPolicy {
 public:
  GenerationSizer() {
    // Partial init only!
    initialize_flags();
    initialize_size_info();
  }

  void initialize_flags() {
    // Do basic sizing work
    this->TwoGenerationCollectorPolicy::initialize_flags();

    // If the user hasn't explicitly set the number of worker
    // threads, set the count.
    assert(UseSerialGC ||
           !FLAG_IS_DEFAULT(ParallelGCThreads) ||
           (ParallelGCThreads > 0),
           "ParallelGCThreads should be set before flag initialization");

    // The survivor ratio's are calculated "raw", unlike the
    // default gc, which adds 2 to the ratio value. We need to
    // make sure the values are valid before using them.
    if (MinSurvivorRatio < 3) {
      MinSurvivorRatio = 3;
    }

    if (InitialSurvivorRatio < 3) {
      InitialSurvivorRatio = 3;
    }
  }

  size_t min_young_gen_size() { return _min_gen0_size; }
  size_t young_gen_size()     { return _initial_gen0_size; }
  size_t max_young_gen_size() { return _max_gen0_size; }

  size_t min_old_gen_size()   { return _min_gen1_size; }
  size_t old_gen_size()       { return _initial_gen1_size; }
  size_t max_old_gen_size()   { return _max_gen1_size; }

  size_t perm_gen_size()      { return PermSize; }
  size_t max_perm_gen_size()  { return MaxPermSize; }
};
