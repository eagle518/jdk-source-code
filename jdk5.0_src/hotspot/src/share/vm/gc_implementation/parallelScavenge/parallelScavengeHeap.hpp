#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)parallelScavengeHeap.hpp	1.33 04/06/15 12:17:32 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class PSAdaptiveSizePolicy;

class ParallelScavengeHeap : public CollectedHeap {
  friend class VMStructs;
 private:
  static PSYoungGen* _young_gen;
  static PSOldGen*   _old_gen;
  static PSPermGen*  _perm_gen;

  // Sizing policy for entire heap
  static PSAdaptiveSizePolicy* _size_policy;
  static PSGCAdaptivePolicyCounters*   _gc_policy_counters;

  static ParallelScavengeHeap* _psh;

  // Byte size of the reserved space for the heap
  size_t _reserved_byte_size;

  size_t _generation_alignment;
  inline void set_generation_alignment(size_t val);

  // Collection of generations that are adjacent in the
  // space reserved for the heap.
  AdjoiningGenerations* _gens;

  // Private accessors
  size_t reserved_byte_size() const { return _reserved_byte_size; }

 protected:

  HeapWord* allocate_new_tlab(size_t size);
  void fill_all_tlabs();

 public:
  ParallelScavengeHeap() : CollectedHeap() {
    set_generation_alignment(intra_generation_alignment());
  }

  // For use by VM operations
  enum CollectionType {
    Scavenge,
    MarkSweep
  };

  ParallelScavengeHeap::Name kind()  { return CollectedHeap::ParallelScavengeHeap; }

  static PSYoungGen* young_gen()     { return _young_gen; }
  static PSOldGen* old_gen()         { return _old_gen; }
  static PSPermGen* perm_gen()       { return _perm_gen; }

  static PSAdaptiveSizePolicy* size_policy() { return _size_policy; }

  static PSGCAdaptivePolicyCounters* gc_policy_counters() { return _gc_policy_counters; }

  static ParallelScavengeHeap* heap();

  AdjoiningGenerations* gens() { return _gens; }

  // Returns JNI_OK on success
  virtual jint initialize();

  void post_initialize();
  void update_counters();

  // The alignment used for generations.
  size_t generation_alignment() const { return _generation_alignment; }

  // The alignment used for eden and survivors within the young gen.
  size_t intra_generation_alignment() const { return 64 * K; }

  size_t capacity() const;
  size_t used() const;
  
  size_t permanent_capacity() const;
  size_t permanent_used() const;

  size_t max_capacity() const;

  bool is_in(const void* p) const;
  bool is_in_permanent(const void *p) const {
    return perm_gen()->reserved().contains(p);
  }

  static bool is_in_young(oop *p);
  static bool is_in_old_or_perm(oop *p);

  // Memory allocation
  HeapWord* mem_allocate(size_t size, bool is_noref, bool is_tlab);
  HeapWord* failed_mem_allocate(bool* notify_ref_lock,
                                size_t size,
                                bool is_noref,
                                bool is_tlab);

  HeapWord* permanent_mem_allocate(size_t size);
  HeapWord* failed_permanent_mem_allocate(bool* notify_ref_lock, size_t size);

  // Support for System.gc()
  void collect(GCCause::Cause cause);

  size_t large_typearray_limit() { return FastAllocateSizeLimit; }

  bool supports_inline_contig_alloc() const { return true; }
  HeapWord** top_addr() const { return young_gen()->top_addr(); }
  HeapWord** end_addr() const { return young_gen()->end_addr(); }

  void ensure_parseability();
  void accumulate_statistics_all_tlabs();
  void resize_all_tlabs();

  size_t unsafe_max_alloc();

  bool supports_tlab_allocation() const { return true; }

  size_t tlab_capacity() const;
  size_t unsafe_max_tlab_alloc() const;

  void oop_iterate(OopClosure* cl);
  void object_iterate(ObjectClosure* cl);
  void permanent_oop_iterate(OopClosure* cl);
  void permanent_object_iterate(ObjectClosure* cl);

  HeapWord* block_start(const void* addr) const;
  size_t block_size(const HeapWord* addr) const;
  bool block_is_obj(const HeapWord* addr) const;

  jlong millis_since_last_gc();

  void prepare_for_verify();
  void print() const;
  void print_on(outputStream* st) const;
  virtual void print_gc_threads() const;
  virtual void gc_threads_do(ThreadClosure* tc) const;

#ifndef PRODUCT
  void verify(bool allow_dirty, bool silent);
#endif

  void print_heap_change(size_t prev_used);

  int addr_to_arena_id(void* addr);

  // Resize the young generation.  The reserved space for the
  // generation may be expanded in preparation for the resize.
  void resize_young_gen(size_t eden_size, size_t survivor_size);

  // Resize the old generation.  The reserved space for the
  // generation may be expanded in preparation for the resize.
  void resize_old_gen(size_t desired_free_space);
};

inline void ParallelScavengeHeap::set_generation_alignment(size_t val) {
  assert(align_size_up_(val, os::vm_page_size()) == val, "not aligned");
  _generation_alignment = val;
}
