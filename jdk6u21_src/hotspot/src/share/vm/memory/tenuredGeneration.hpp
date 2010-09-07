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

// TenuredGeneration models the heap containing old (promoted/tenured) objects.

class ParGCAllocBufferWithBOT;

class TenuredGeneration: public OneContigSpaceCardGeneration {
  friend class VMStructs;
 protected:
  // current shrinking effect: this damps shrinking when the heap gets empty.
  size_t _shrink_factor;
  // Some statistics from before gc started.
  // These are gathered in the gc_prologue (and should_collect)
  // to control growing/shrinking policy in spite of promotions.
  size_t _capacity_at_prologue;
  size_t _used_at_prologue;

#ifndef SERIALGC
  // To support parallel promotion: an array of parallel allocation
  // buffers, one per thread, initially NULL.
  ParGCAllocBufferWithBOT** _alloc_buffers;
#endif // SERIALGC

  // Retire all alloc buffers before a full GC, so that they will be
  // re-allocated at the start of the next young GC.
  void retire_alloc_buffers_before_full_gc();

  GenerationCounters*   _gen_counters;
  CSpaceCounters*       _space_counters;

 public:
  TenuredGeneration(ReservedSpace rs, size_t initial_byte_size, int level,
                    GenRemSet* remset);

  Generation::Name kind() { return Generation::MarkSweepCompact; }

  // Printing
  const char* name() const;
  const char* short_name() const { return "Tenured"; }
  bool must_be_youngest() const { return false; }
  bool must_be_oldest() const { return true; }

  // Does a "full" (forced) collection invoked on this generation collect
  // all younger generations as well? Note that this is a
  // hack to allow the collection of the younger gen first if the flag is
  // set. This is better than using th policy's should_collect_gen0_first()
  // since that causes us to do an extra unnecessary pair of restart-&-stop-world.
  virtual bool full_collects_younger_generations() const {
    return !CollectGen0First;
  }

  // Mark sweep support
  void compute_new_size();

  virtual void gc_prologue(bool full);
  virtual void gc_epilogue(bool full);
  bool should_collect(bool   full,
                      size_t word_size,
                      bool   is_tlab);

  virtual void collect(bool full,
                       bool clear_all_soft_refs,
                       size_t size,
                       bool is_tlab);

#ifndef SERIALGC
  // Overrides.
  virtual oop par_promote(int thread_num,
                          oop obj, markOop m, size_t word_sz);
  virtual void par_promote_alloc_undo(int thread_num,
                                      HeapWord* obj, size_t word_sz);
  virtual void par_promote_alloc_done(int thread_num);
#endif // SERIALGC

  // Performance Counter support
  void update_counters();

  // Statistics

  virtual void update_gc_stats(int level, bool full);

  virtual bool promotion_attempt_is_safe(size_t max_promoted_in_bytes,
    bool younger_handles_promotion_failure) const;

  void verify_alloc_buffers_clean();
};
