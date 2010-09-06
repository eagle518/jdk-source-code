#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)psYoungGen.hpp	1.37 04/06/16 07:53:39 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class PSMarkSweepDecorator;

class PSYoungGen : public CHeapObj {
  friend class VMStructs;
  friend class ParallelScavengeHeap;
  friend class AdjoiningGenerations;

 protected:
  MemRegion       _reserved;
  PSVirtualSpace* _virtual_space;

  // Spaces
  MutableSpace* _eden_space;
  MutableSpace* _from_space;
  MutableSpace* _to_space;


  // MarkSweep Decorators
  PSMarkSweepDecorator* _eden_mark_sweep;
  PSMarkSweepDecorator* _from_mark_sweep;
  PSMarkSweepDecorator* _to_mark_sweep;

  // Sizing information, in bytes, set in constructor
  const size_t _init_gen_size;
  const size_t _min_gen_size;
  const size_t _max_gen_size;

  // Performance counters
  PSGenerationCounters*     _gen_counters;
  SpaceCounters*            _eden_counters;
  SpaceCounters*            _from_counters;
  SpaceCounters*            _to_counters;

  // Initialize the space boundaries 
  void compute_initial_space_boundaries();

  // Space boundary helper
  void set_space_boundaries(size_t eden_size, size_t survivor_size);

  virtual bool resize_generation(size_t eden_size, size_t survivor_size);
  virtual void resize_spaces(size_t eden_size, size_t survivor_size);

  // Adjust the spaces to be consistent with the virtual space.
  void post_resize();

  // Return number of bytes that the generation can change.
  // These should not be used by PSYoungGen
  virtual size_t available_for_expansion(); 
  virtual size_t available_for_contraction();

  // Given a desired shrinkage in the size of the young generation,
  // return the actual size available for shrinkage.
  virtual size_t limit_gen_shrink(size_t desired_change);
  // returns the number of bytes available from the current size
  // down to the minimum generation size.
  size_t available_to_min_gen();
  // Return the number of bytes available for shrinkage considering
  // the location the live data in the generation.
  virtual size_t available_to_live();

 public:
  // Initialize the generation.
  PSYoungGen(size_t        initial_byte_size, 
             size_t        minimum_byte_size,
             size_t        maximum_byte_size);
  void initialize_work();
  virtual void initialize(ReservedSpace rs, size_t alignment);
  virtual void initialize_virtual_space(ReservedSpace rs, size_t alignment);

  MemRegion reserved()               { return _reserved; }

  bool is_in(const void* p) const   {
      return _virtual_space->contains((void *)p);
  }

  MutableSpace*   eden_space() const    { return _eden_space; }
  MutableSpace*   from_space() const    { return _from_space; }
  MutableSpace*   to_space() const      { return _to_space; }
  PSVirtualSpace* virtual_space() const { return _virtual_space; }

  // For Adaptive size policy
  size_t min_gen_size() { return _min_gen_size; }

  // MarkSweep support
  PSMarkSweepDecorator* eden_mark_sweep() const    { return _eden_mark_sweep; }
  PSMarkSweepDecorator* from_mark_sweep() const    { return _from_mark_sweep; }
  PSMarkSweepDecorator* to_mark_sweep() const      { return _to_mark_sweep;   }

  void precompact();
  void adjust_pointers();
  void compact();

  // Called during/after gc
  void swap_spaces();

  // Resize generation using suggested free space size and survivor size
  // NOTE:  "eden_size" and "survivor_size" are suggestions only. Current
  //        heap layout (particularly, live objects in from space) might
  //        not allow us to use these values.
  void resize(size_t eden_size, size_t survivor_size);

  // Size info
  size_t capacity_in_bytes() const;
  size_t used_in_bytes() const;
  size_t free_in_bytes() const;

  size_t capacity_in_words() const;
  size_t used_in_words() const;
  size_t free_in_words() const;

  // The max this generation can grow to
  size_t max_size() const            { return _reserved.byte_size(); }

  // The max this generation can grow to if the boundary between
  // the generations are allowed to move.
  size_t gen_size_limit() const { return _max_gen_size; }

  // Allocation
  HeapWord* allocate(size_t word_size, bool is_large_noref, bool is_tlab) {
    HeapWord* result = eden_space()->cas_allocate(word_size);
    return result;
  }

  HeapWord** top_addr() const   { return eden_space()->top_addr(); }
  HeapWord** end_addr() const   { return eden_space()->end_addr(); }

  // Iteration.
  void oop_iterate(OopClosure* cl);
  void object_iterate(ObjectClosure* cl);

  virtual void reset_after_change();
  virtual void reset_survivors_after_shrink();

  // Performance Counter support
  void update_counters();

  // Debugging - do not use for time critical operations
  void print() const;
  void print_on(outputStream* st) const;
  void print_used_change(size_t prev_used) const;
  virtual const char* name() const { return "PSYoungGen"; }
 
  void verify(bool allow_dirty) PRODUCT_RETURN;

  // Space boundary invariant checker
  void space_invariants() PRODUCT_RETURN;
};
