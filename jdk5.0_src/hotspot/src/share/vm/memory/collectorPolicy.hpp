#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)collectorPolicy.hpp	1.19 03/12/23 16:40:58 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// This class (or more correctly, subtypes of this class)
// are used to define global garbage collector attributes.
// This includes initialization of generations and any other
// shared resources they may need. 
//
// In general, all flag adjustment and validation should be
// done in initialize_flags(), which is called prior to
// initialize_size_info().
//
// This class is not fully developed yet. As more collector(s)
// are added, it is expected that we will come across further
// behavior that requires global attention. The correct place
// to deal with those issues is this class. 

class CollectorPolicy : public CHeapObj {
 protected:
  PermanentGenerationSpec *_permanent_generation;
  GenerationSpec **_generations;
  GCPolicyCounters *_gc_policy_counters;

  virtual void initialize_flags() = 0;
  virtual void initialize_size_info() = 0;
  virtual void initialize_generations() = 0;

 public:
  // Identification methods.
  virtual bool is_two_generation_policy()        { return false; }
  virtual bool is_mark_sweep_policy()            { return false; }
  virtual bool is_train_policy()                 { return false; }
  virtual bool is_concurrent_mark_sweep_policy() { return false; }

  virtual int number_of_generations() = 0;

  virtual GenerationSpec **generations()       {
    assert(_generations != NULL, "Sanity check");
    return _generations;
  }

  virtual PermanentGenerationSpec *permanent_generation() {
    assert(_permanent_generation != NULL, "Sanity check");
    return _permanent_generation;
  }

  virtual BarrierSet::Name barrier_set_name() = 0;
  virtual GenRemSet::Name  rem_set_name() = 0;

  // Create the remembered set (to cover the given reserved region,
  // allowing breaking up into at most "max_covered_regions").
  virtual GenRemSet* create_rem_set(MemRegion reserved,
				    int max_covered_regions);

  // This method controls how a collector satisfies a request
  // for a block of memory.
  virtual HeapWord* mem_allocate_work(size_t size,
                                      bool is_large_noref,
                                      bool is_tlab) = 0;
  
  // This method controls how a collector handles one or more
  // of its generations being fully allocated.
  virtual HeapWord *satisfy_failed_allocation(size_t size,
                                              bool is_large_noref,
                                              bool is_tlab,
                                              bool* notify_ref_lock) = 0;
  // Performace Counter support
  GCPolicyCounters* counters()     { return _gc_policy_counters; }
};

// All of hotspot's current collectors are subtypes of this
// class. Currently, these collectors all use the same gen[0],
// but have different gen[1] types. If we add another subtype
// of CollectorPolicy, this class should be broken out into
// its own file.

class TwoGenerationCollectorPolicy : public CollectorPolicy {
 protected:
  size_t _min_alignment;
  size_t _max_alignment;
  size_t _min_gen0_size;
  size_t _initial_gen0_size;
  size_t _max_gen0_size;
  size_t _min_gen1_size;
  size_t _initial_gen1_size;
  size_t _max_gen1_size;

  void set_min_alignment(size_t align)         { _min_alignment = align; }
  size_t min_alignment()                       { return _min_alignment; }
  void set_max_alignment(size_t align)         { _max_alignment = align; }
  size_t max_alignment()                       { return _max_alignment; }

  void initialize_flags();
  void initialize_size_info();
  void initialize_generations()                { ShouldNotReachHere(); }

 private:
  // Return true if an allocation should be attempted in the older
  // generation if it fails in the younger generation.  Return
  // false, otherwise.
  virtual bool should_try_older_generation_allocation(size_t word_size) const;

 public:
  // Inherited methods
  bool is_two_generation_policy()              { return true; }

  int number_of_generations()                  { return 2; }
  BarrierSet::Name barrier_set_name()          { return BarrierSet::CardTableModRef; }
  GenRemSet::Name rem_set_name()               { return GenRemSet::CardTable; }

  HeapWord* mem_allocate_work(size_t size,
                              bool is_large_noref,
                              bool is_tlab);

  HeapWord *satisfy_failed_allocation(size_t size,
                                      bool is_large_noref,
                                      bool is_tlab,
                                      bool* notify_ref_lock);

  // Methods unique to TwoGenerationCollectorPolicy
  virtual size_t large_typearray_limit();
};

class TrainPolicy : public TwoGenerationCollectorPolicy {
 protected:
  void initialize_flags();
  void initialize_generations();

 private:
  virtual bool should_try_older_generation_allocation(size_t word_size) const ;

 public:
  TrainPolicy();
  virtual bool is_train_policy()               { return true; }
  virtual size_t large_typearray_limit();
};

class ConcurrentMarkSweepPolicy : public TwoGenerationCollectorPolicy {
 protected:
  void initialize_generations();

 public:
  ConcurrentMarkSweepPolicy();

  bool is_concurrent_mark_sweep_policy()       { return true; }
};

class MarkSweepPolicy : public TwoGenerationCollectorPolicy {
 protected:
  void initialize_generations();

 public:
  MarkSweepPolicy();

  bool is_mark_sweep_policy()                   { return true; }
};
