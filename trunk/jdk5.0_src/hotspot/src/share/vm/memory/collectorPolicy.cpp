#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)collectorPolicy.cpp	1.44 04/06/11 12:24:08 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_collectorPolicy.cpp.incl"

GenRemSet* CollectorPolicy::create_rem_set(MemRegion whole_heap,
					   int max_covered_regions) {
  switch (rem_set_name()) {
  case GenRemSet::CardTable: {
    if (barrier_set_name() != BarrierSet::CardTableModRef) 
      vm_exit_during_initialization("Mismatch between RS and BS.");
    CardTableRS* res = new CardTableRS(whole_heap, max_covered_regions);
    return res;
  }
  default:
    guarantee(false, "unrecognized GenRemSet::Name");
    return(NULL);
  }
}

void TwoGenerationCollectorPolicy::initialize_flags() {
  // All space sizes must be multiples of car size in order for the
  // CarTable to work.  Similarly with the should_scavenge region size.
  set_min_alignment(MAX2((uintx) CarSpace::car_size(), (uintx) Generation::GenGrain));

  // The card marking array and the offset arrays for old generations are
  // committed in os pages as well. Make sure they are entirely full (to
  // avoid partial page problems), e.g. if 512 bytes heap corresponds to 1
  // byte entry and the os page size is 4096, the maximum heap size should
  // be 512*4096 = 2MB aligned.
  set_max_alignment(GenRemSet::max_alignment_constraint(rem_set_name()));
  assert(max_alignment() >= min_alignment() && max_alignment() % min_alignment() == 0, "invalid alignment constraints");

  // Adjust max size parameters
  if (NewSize > MaxNewSize) {
    MaxNewSize = NewSize;
  }
  NewSize = align_size_down(NewSize, min_alignment());
  MaxNewSize = align_size_down(MaxNewSize, min_alignment());

  OldSize = align_size_down(OldSize, min_alignment());
  if (NewSize + OldSize > MaxHeapSize) {
    MaxHeapSize = NewSize + OldSize;
  }
  MaxHeapSize = align_size_up(MaxHeapSize, max_alignment());

  if (PermSize > MaxPermSize) {
    MaxPermSize = PermSize;
  }
  PermSize = align_size_down(PermSize, min_alignment());
  MaxPermSize = align_size_down(MaxPermSize, max_alignment());

  MinPermHeapExpansion = align_size_down(MinPermHeapExpansion, min_alignment());
  MaxPermHeapExpansion = align_size_down(MaxPermHeapExpansion, min_alignment());

  MinHeapDeltaBytes = align_size_up(MinHeapDeltaBytes, min_alignment());

  always_do_update_barrier = UseConcMarkSweepGC;
  BlockOffsetArrayUseUnallocatedBlock |= ParallelGCThreads > 0;

  SharedReadOnlySize = align_size_up(SharedReadOnlySize, max_alignment());
  SharedReadWriteSize = align_size_up(SharedReadWriteSize, max_alignment());
  SharedMiscDataSize = align_size_up(SharedMiscDataSize, max_alignment());

  // Check validity of heap flags
  assert(NewSize     % min_alignment() == 0, "eden space alignment");
  assert(MaxNewSize  % min_alignment() == 0, "survivor space alignment");
  assert(OldSize     % min_alignment() == 0, "old space alignment");
  assert(MaxHeapSize % max_alignment() == 0, "maximum heap alignment");
  assert(PermSize    % min_alignment() == 0, "permanent space alignment");
  assert(MaxPermSize % max_alignment() == 0, "maximum permanent space alignment");
  assert(SharedReadOnlySize % max_alignment() == 0, "read-only space alignment");
  assert(SharedReadWriteSize % max_alignment() == 0, "read-write space alignment");
  assert(SharedMiscDataSize % max_alignment() == 0, "misc-data space alignment");

  if (NewSize < 3*min_alignment()) {
     // make sure there room for eden and two survivor spaces
    vm_exit_during_initialization("Too small new size specified");
  }
  if (PermSize < M) {
    vm_exit_during_initialization("Too small initial permanent heap");
  }
  if (SurvivorRatio < 1 || NewRatio < 1) {
    vm_exit_during_initialization("Invalid heap ratio specified");
  }
}

void TwoGenerationCollectorPolicy::initialize_size_info() {
  // User inputs from -mx and ms are aligned
  MaxNewSize = align_size_down(MaxNewSize, min_alignment());
  uintx initial_heap_size = align_size_up(Arguments::initial_heap_size(), min_alignment());
  uintx max_heap_size     = align_size_up(MaxHeapSize, max_alignment());

#ifndef COMPILER2    // Don't degrade server performance for footprint
  if (FLAG_IS_DEFAULT(UseMPSS) && max_heap_size < LargePageHeapSizeThreshold) {
    // No need for large granularity pages w/small heaps.
    UseMPSS = false;
  }
#endif

  // Check validity of heap parameters from launcher
  if (initial_heap_size == 0) {
    initial_heap_size = NewSize + OldSize;
  } else {
    Universe::check_alignment(initial_heap_size, min_alignment(), "initial heap");
  }

  // Check heap parameter properties
  if (initial_heap_size < M) {
    vm_exit_during_initialization("Too small initial heap");
  }
  if (initial_heap_size <= NewSize) {
     // make sure there is at least some room in old space
    vm_exit_during_initialization("Too small initial heap for new size specified");
  }
  if (max_heap_size < initial_heap_size) {
    vm_exit_during_initialization("Incompatible initial and maximum heap sizes specified");
  }

  // Minimum sizes of the generations may be different than
  // the initial sizes.
  _min_gen0_size = NewSize;
  _min_gen1_size = OldSize;

  // Parameters are valid, compute area sizes.
  size_t max_new_size = align_size_down(max_heap_size / (NewRatio+1),
					min_alignment());
  max_new_size = MIN2(MAX2(max_new_size, _min_gen0_size), MaxNewSize);

  size_t desired_new_size = align_size_down(initial_heap_size / (NewRatio+1),
					    min_alignment());

  size_t new_size = MIN2(MAX2(desired_new_size, _min_gen0_size), max_new_size);
  // I'm putting this in to make sure the compilation of the code above is
  // correct.  At times, it hasn't been. -dld 04/00.
  guarantee(new_size <= max_new_size &&
	    new_size <= MAX2(desired_new_size, _min_gen0_size),
	    "Or what gives");


  _initial_gen0_size = new_size;
  _max_gen0_size = max_new_size;

  _initial_gen1_size = initial_heap_size - new_size;
  _max_gen1_size = max_heap_size - max_new_size;
}

HeapWord* TwoGenerationCollectorPolicy::mem_allocate_work(size_t size,
                                                          bool is_large_noref,
                                                          bool is_tlab) {
  GenCollectedHeap *gch = GenCollectedHeap::heap();

  debug_only(gch->check_for_valid_allocation_state());
  assert(gch->no_gc_in_progress(), "Allocation during gc not allowed");
  HeapWord* result = NULL;

  // Loop until the allocation is satisified,
  // or unsatisfied after GC.
  for (int try_count = 1; /* return or throw */; try_count += 1) {

    // First allocation attempt is lock-free.
    Generation *gen0 = gch->get_gen(0);
    assert(gen0->supports_inline_contig_alloc(),
      "Otherwise, must do alloc within heap lock");
    if (gen0->should_allocate(size, is_large_noref, is_tlab)) {
      result = gen0->par_allocate(size, is_large_noref, is_tlab);
      if (result != NULL) {
        assert(gch->is_in(result), "result not in heap");
        return result;
      }
    }
    int gc_count_before;  // read inside the Heap_lock locked region
    {
      MutexLocker ml(Heap_lock);
      if (PrintGC && Verbose) {
        gclog_or_tty->print_cr("TwoGenerationCollectorPolicy::mem_allocate_work:"
                      " attempting locked slow path allocation");
      }
      // Note that only large objects get a shot at being
      // allocated in later generations.  If jvmpi slow allocation
      // is enabled, allocate in later generations (since the
      // first generation is always full.
      bool first_only = ! should_try_older_generation_allocation(size);

      result = gch->attempt_allocation(size, 
                                       is_large_noref, 
                                       is_tlab, 
                                       first_only);
      if (result != NULL) {
        assert(gch->is_in(result), "result not in heap");
        return result;
      }

      // Read the gc count while the heap lock is held.
      gc_count_before = Universe::heap()->total_collections();
    }
      
    VM_GenCollectForAllocation op(size,
                                  is_large_noref,
                                  is_tlab,
                                  gc_count_before);
    VMThread::execute(&op);
    if (op.prologue_succeeded()) {
      result = op.result();
      assert(result == NULL || gch->is_in(result), "result not in heap");
      return result;
    }

    // Give a warning if we seem to be looping forever.
    if ((QueuedAllocationWarningCount > 0) &&
        (try_count % QueuedAllocationWarningCount == 0)) {
          warning("TwoGenerationCollectorPolicy::mem_allocate_work retries %d times \n\t"
		  " size=%d %s", try_count, size, is_tlab ? "(TLAB)" : "");
    }
  }
}

HeapWord* TwoGenerationCollectorPolicy::satisfy_failed_allocation(size_t size,
                                                                  bool   is_large_noref,
                                                                  bool   is_tlab,
                                                                  bool*  notify_ref_lock) {
  GenCollectedHeap *gch = GenCollectedHeap::heap();
  GCCauseSetter x(gch, GCCause::_allocation_failure);
  HeapWord* result = NULL;
  
  // The gc_prologues have not executed yet.  The value
  // for incremental_collection_will_fail() is the remanent 
  // of the last collection.
  if (!gch->incremental_collection_will_fail()) {
    // Do an incremental collection.
    gch->do_collection(false            /* full */,
                       false            /* clear_all_soft_refs */,
                       size             /* size */,
                       is_large_noref   /* is_large_noref */,
                       is_tlab          /* is_tlab */,
                       number_of_generations() - 1 /* max_level */,
                       notify_ref_lock  /* notify_ref_lock */);
  } else {
    // The incremental_collection_will_fail flag is set if the
    // next incremental collection will not succeed (e.g., the
    // DefNewGeneration didn't think it had space to promote all
    // its objects). However, that last incremental collection
    // continued, allowing all older generations to collect (and
    // perhaps change the state of the flag).
    // 
    // If we reach here, we know that an incremental collection of
    // all generations left us in the state where incremental collections
    // will fail, so we just try allocating the requested space. 
    // If the allocation fails everywhere, force a full collection.
    // We're probably very close to being out of memory, so forcing many
    // collections now probably won't help.
    if (PrintGC && Verbose) {
      gclog_or_tty->print_cr("TwoGenerationCollectorPolicy::satisfy_failed_allocation:"
                    " attempting allocation anywhere before full collection");
    }
    result = gch->attempt_allocation(size, 
                                     is_large_noref, 
                                     is_tlab, 
                                     false /* first_only */);
    if (result != NULL) {
      assert(gch->is_in(result), "result not in heap");
      return result;
    }

    // Allocation request hasn't yet been met; try a full collection.
    gch->do_collection(true             /* full */, 
                       false            /* clear_all_soft_refs */, 
                       size             /* size */, 
                       is_large_noref   /* is_large_noref */,
                       is_tlab          /* is_tlab */,
                       number_of_generations() - 1 /* max_level */, 
                       notify_ref_lock  /* notify_ref_lock */);
  }
  
  result = gch->attempt_allocation(size, is_large_noref, is_tlab, false /*first_only*/);
  
  if (result != NULL) {
    assert(gch->is_in(result), "result not in heap");
    return result;
  }
  
  // OK, collection failed, try expansion.
  for (int i = number_of_generations() - 1 ; i>= 0; i--) {
    Generation *gen = gch->get_gen(i);
    if (gen->should_allocate(size, is_large_noref, is_tlab)) {
      result = gen->expand_and_allocate(size, is_large_noref, is_tlab);
      if (result != NULL) {
        assert(gch->is_in(result), "result not in heap");
        return result;
      }
    }
  }
  
  // If we reach this point, we're really out of memory. Try every trick
  // we can to reclaim memory. Force collection of soft references. Force
  // a complete compaction of the heap. Any additional methods for finding
  // free memory should be here, especially if they are expensive. If this
  // attempt fails, an OOM exception will be thrown.
  {
    IntFlagSetting flag_change(MarkSweepAlwaysCompactCount, 1); // Make sure the heap is fully compacted

    gch->do_collection(true             /* full */,
                       true             /* clear_all_soft_refs */,
                       size             /* size */,
                       is_large_noref   /* is_large_noref */,
                       is_tlab          /* is_tlab */,
                       number_of_generations() - 1 /* max_level */,
                       notify_ref_lock  /* notify_ref_lock */);
  }

  result = gch->attempt_allocation(size, is_large_noref, is_tlab, false /* first_only */);
  if (result != NULL) {
    assert(gch->is_in(result), "result not in heap");
    return result;
  }
  
  // What else?  We might try synchronous finalization later.  If the total
  // space available is large enough for the allocation, then a more
  // complete compaction phase than we've tried so far might be
  // appropriate.
  return NULL;
}

size_t TwoGenerationCollectorPolicy::large_typearray_limit() {
  return FastAllocateSizeLimit;
}

// Return true if
//  The allocation won't fit into the maximum young gen heap
//  jvmpi_slow_allocation
bool TwoGenerationCollectorPolicy::should_try_older_generation_allocation(
	size_t word_size) const {
  return (word_size > heap_word_size(_max_gen0_size)) ||
    Universe::jvmpi_slow_allocation();
}

//
// TrainPolicy methods
//

TrainPolicy::TrainPolicy() {
  initialize_flags();
  initialize_size_info();
  initialize_generations();
}

void TrainPolicy::initialize_flags() {
  // Increase the default heap size a little.
  // The train generally takes up more room to store the same
  // amount of objects as the default collector, due to fragmentation
  // caused by unused areas at the ends of cars.
  MaxHeapSize += 4*M;

  // Do basic sizing work
  this->TwoGenerationCollectorPolicy::initialize_flags();

  // disable newgen expansion for train gc
  MaxNewSize = NewSize;
}

void TrainPolicy::initialize_generations() {
  _generations = new GenerationSpecPtr[number_of_generations()];
  if (_generations == NULL)
    vm_exit_during_initialization("Unable to allocate gen spec");
  
  _generations[0] = new GenerationSpec(Generation::DefNew, _initial_gen0_size, _max_gen0_size);
  _generations[1] = new GenerationSpec(Generation::TrainGen, _initial_gen1_size, _max_gen1_size);

  _permanent_generation = new PermanentGenerationSpec(
                                  PermGen::MarkSweepCompact, PermSize,
                                  MaxPermSize, SharedReadOnlySize,
                                  SharedReadWriteSize, SharedMiscDataSize,
                                  SharedMiscCodeSize);

  if (_generations[0] == NULL || _generations[1] == NULL || _permanent_generation == NULL)
    vm_exit_during_initialization("Unable to allocate gen spec");

  // initialize the policy counters - 2 collectors, 3 generations
  _gc_policy_counters = new GCPolicyCounters("Copy:Train", 2, 3);
}

bool TrainPolicy::should_try_older_generation_allocation(size_t word_size) const {
  assert(!Universe::jvmpi_slow_allocation(), 
    "The incremental collector does not support JVMPI");
  return word_size > CarSpace::car_size_in_words();
}

size_t TrainPolicy::large_typearray_limit() {
  return CarSpace::car_size_in_words();
}

//
// ConcurrentMarkSweepPolicy methods
//

ConcurrentMarkSweepPolicy::ConcurrentMarkSweepPolicy() {
  initialize_flags();
  initialize_size_info();
  initialize_generations();
}

void ConcurrentMarkSweepPolicy::initialize_generations() {
  _generations = new GenerationSpecPtr[number_of_generations()];
  if (_generations == NULL)
    vm_exit_during_initialization("Unable to allocate gen spec");
  
  if (UseParNewGC && ParallelGCThreads > 0) {
    _generations[0] = new GenerationSpec(Generation::ParNew,
                                         _initial_gen0_size, _max_gen0_size);
  } else {
    _generations[0] = new GenerationSpec(Generation::DefNew,
                                         _initial_gen0_size, _max_gen0_size);
  }
  _generations[1] = new GenerationSpec(Generation::ConcurrentMarkSweep,
                          _initial_gen1_size, _max_gen1_size);

  _permanent_generation = new PermanentGenerationSpec(
                                PermGen::ConcurrentMarkSweep,
                                PermSize, MaxPermSize,
                                SharedReadOnlySize,
                                SharedReadWriteSize,
                                SharedMiscDataSize,
                                SharedMiscCodeSize);

  if (_generations[0] == NULL || _generations[1] == NULL
                              || _permanent_generation == NULL) {
    vm_exit_during_initialization("Unable to allocate gen spec");
  }

  // initialize the policy counters - 2 collectors, 3 generations
  if (UseParNewGC && ParallelGCThreads > 0) {
    _gc_policy_counters = new GCPolicyCounters("ParNew:CMS", 2, 3);
  }
  else {
    _gc_policy_counters = new GCPolicyCounters("Copy:CMS", 2, 3);
  }
}

//
// MarkSweepPolicy methods
//

MarkSweepPolicy::MarkSweepPolicy() {
  initialize_flags();
  initialize_size_info();
  initialize_generations();
}

void MarkSweepPolicy::initialize_generations() {
  _generations = new GenerationSpecPtr[number_of_generations()];
  if (_generations == NULL)
    vm_exit_during_initialization("Unable to allocate gen spec");
  
  if (UseParNewGC && ParallelGCThreads > 0) {
    _generations[0] = new GenerationSpec(Generation::ParNew, _initial_gen0_size, _max_gen0_size);
  } else {
    _generations[0] = new GenerationSpec(Generation::DefNew, _initial_gen0_size, _max_gen0_size);
  }
  _generations[1] = new GenerationSpec(Generation::MarkSweepCompact, _initial_gen1_size, _max_gen1_size);

  _permanent_generation = new PermanentGenerationSpec(
                                  PermGen::MarkSweepCompact, PermSize,
                                  MaxPermSize, SharedReadOnlySize,
                                  SharedReadWriteSize, SharedMiscDataSize,
                                  SharedMiscCodeSize);

  if (_generations[0] == NULL || _generations[1] == NULL || _permanent_generation == NULL)
    vm_exit_during_initialization("Unable to allocate gen spec");

  // initialize the policy counters - 2 collectors, 3 generations
  if (UseParNewGC && ParallelGCThreads > 0) {
    _gc_policy_counters = new GCPolicyCounters("ParNew:MSC", 2, 3);
  }
  else {
    _gc_policy_counters = new GCPolicyCounters("Copy:MSC", 2, 3);
  }
}
