#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)collectedHeap.hpp	1.29 04/06/15 12:17:33 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// A "CollectedHeap" is an implementation of a java heap for HotSpot.  This
// is an abstract class: there may be many different kinds of heaps.  This
// class defines the functions that a heap must implement, and contains
// infrastructure common to all heaps.

class BarrierSet;
class ThreadClosure;

class CollectedHeap : public CHeapObj {
  friend class VMStructs;
  friend class IsGCActiveMark; // Block structured external access to _is_gc_active

 protected:
  MemRegion _reserved;
  BarrierSet* _barrier_set;
  bool _is_gc_active;
  unsigned int _total_collections;
  size_t _max_heap_capacity;

  // Reason for current garbage collection.  Should be set to
  // a value reflecting no collection between collections.
  GCCause::Cause _gc_cause;
  GCCause::Cause _gc_lastcause;
  PerfStringVariable* _perf_gc_cause;
  PerfStringVariable* _perf_gc_lastcause;

  // Constructor
  CollectedHeap();

  // Create a new tlab
  virtual HeapWord* allocate_new_tlab(size_t size);

  // Fix up tlabs to make the heap well-formed again.
  virtual void fill_all_tlabs();

  // Accumulate statistics on all tlabs.
  virtual void accumulate_statistics_all_tlabs();

  // Reinitialize tlabs before resuming mutators.
  virtual void resize_all_tlabs();

  debug_only(static void check_for_valid_allocation_state();)

 protected:
  // Allocate from the current thread's TLAB, with broken-out slow path.
  inline static HeapWord* allocate_from_tlab(Thread* thread, size_t size);
  static HeapWord* allocate_from_tlab_slow(Thread* thread, size_t size);

  // Allocate an uninitialized block of the given size, or returns NULL if
  // this is impossible.
  inline static HeapWord* common_mem_allocate_noinit(size_t size, bool is_noref, TRAPS);

  // Like allocate_init, but the block returned by a successful allocation
  // is guaranteed initialized to zeros.
  inline static HeapWord* common_mem_allocate_init(size_t size, bool is_noref, TRAPS);

  // Same as common_mem version, except memory is allocated in the permanent area
  // If there is no permanent area, revert to common_mem_allocate_noinit
  inline static HeapWord* common_permanent_mem_allocate_noinit(size_t size, TRAPS);

  // Same as common_mem version, except memory is allocated in the permanent area
  // If there is no permanent area, revert to common_mem_allocate_init
  inline static HeapWord* common_permanent_mem_allocate_init(size_t size, TRAPS);

  // Helper functions for (VM) allocation.
  inline static void post_allocation_setup_common(KlassHandle klass,
						  HeapWord* obj, size_t size); 

  inline static void post_allocation_setup_obj(KlassHandle klass,
					       HeapWord* obj, size_t size);

  inline static void post_allocation_setup_array(KlassHandle klass,
						 HeapWord* obj, size_t size,
						 int length);

  // Clears an allocated object.
  inline static void init_obj(HeapWord* obj, size_t size);

  // Verification functions
  virtual void check_for_bad_heap_word_value(HeapWord* addr, size_t size)
    PRODUCT_RETURN;
  virtual void check_for_non_bad_heap_word_value(HeapWord* addr, size_t size)
    PRODUCT_RETURN;

 public:
  enum Name {
    Abstract,
    SharedHeap,
    GenCollectedHeap,
    ParallelScavengeHeap
  };

  virtual CollectedHeap::Name kind() { return CollectedHeap::Abstract; }

  /**
   * Returns JNI error code JNI_ENOMEM if memory could not be allocated, 
   * and JNI_OK on success.
   */
  virtual jint initialize() = 0;

  // In many heaps, there will be a need to perform some initialization activities
  // after the Universe is fully formed, but before general heap allocation is allowed.
  // This is the correct place to place such initialization methods.
  virtual void post_initialize() = 0;

  MemRegion reserved_region() const { return _reserved; }

  // Return the number of bytes currently reserved, committed, and used,
  // respectively, for holding objects.
  size_t reserved_obj_bytes() const { return _reserved.byte_size(); }

  // Future cleanup here. The following functions should specify bytes or heapwords as
  // part of their signature.
  virtual size_t capacity() const = 0;
  virtual size_t used() const = 0;
  
  virtual size_t permanent_capacity() const = 0;
  virtual size_t permanent_used() const = 0;

  // Support for java.lang.Runtime.maxMemory():  return the maximum amount of
  // memory that the vm could make available for storing 'normal' java objects.
  // This is based on the reserved address space, but should not include space
  // that the vm uses internally for bookkeeping or temporary storage (e.g.,
  // perm gen space or, in the case of the young gen, one of the survivor
  // spaces).
  virtual size_t max_capacity() const = 0;

  // Returns "TRUE" if "p" points into the reserved area of the heap.
  bool is_in_reserved(const void* p) const {
    return _reserved.contains(p);
  }

  // Returns "TRUE" if "p" points into the allocated area of the heap.
  virtual bool is_in(const void* p) const = 0;

  bool is_in_or_null(const void* p) const {
    return p == NULL || is_in(p);
  }

  // Returns "TRUE" if "p" is allocated as "permanent" data.
  // If the heap does not use "permanent" data, returns the same
  // value is_in() would return.
  virtual bool is_in_permanent(const void *p) const = 0;

  bool is_in_permanent_or_null(const void *p) const {
    return p == NULL || is_in_permanent(p);
  }

  void set_gc_cause(GCCause::Cause v) {
     if (UsePerfData) {
       _gc_lastcause = _gc_cause;
       _perf_gc_lastcause->set_value(GCCause::to_string(_gc_lastcause));
       _perf_gc_cause->set_value(GCCause::to_string(v));
     }
    _gc_cause = v;
  }
  GCCause::Cause gc_cause() { return _gc_cause; }

  // Preload classes into the shared portion of the heap, and then dump
  // that data to a file so that it can be loaded directly by another
  // VM (then terminate).
  virtual void preload_and_dump(TRAPS) { ShouldNotReachHere(); }

  // General obj/array allocation facilities.
  inline static oop obj_allocate(KlassHandle klass, int size, TRAPS);
  inline static oop array_allocate(KlassHandle klass, int size, int length, TRAPS);
  inline static oop large_typearray_allocate(KlassHandle klass, int size, int length, TRAPS);

  // Special obj/array allocation facilities.
  // Some heaps may want to manage "permanent" data uniquely. These default
  // to the general routines if the heap does not support such handling.
  inline static oop permanent_obj_allocate(KlassHandle klass, int size, TRAPS);
  inline static oop permanent_array_allocate(KlassHandle klass, int size, int length, TRAPS);

  // Raw memory allocation facilities
  // The obj and array allocate methods are covers for these methods.
  // The permanent allocation method should default to mem_allocate if
  // permanent memory isn't supported.
  virtual HeapWord* mem_allocate(size_t size, bool is_noref, bool is_tlab) = 0;
  virtual HeapWord* permanent_mem_allocate(size_t size) = 0;

  // The boundary between a "large" and "small" array of primitives, in words.
  virtual size_t large_typearray_limit() = 0;

  // Some heaps may offer a contiguous region for shared non-blocking
  // allocation, via inlined code (by exporting the address of the top and
  // end fields defining the extent of the contiguous allocation region.)

  // This function returns "true" iff the heap supports this kind of
  // allocation.  (Default is "no".)
  virtual bool supports_inline_contig_alloc() const {
    return false;
  }
  // These functions return the addresses of the fields that define the
  // boundaries of the contiguous allocation area.  (These fields should be
  // physically near to one another.)
  virtual HeapWord** top_addr() const {
    guarantee(false, "inline contiguous allocation not supported");
    return NULL;
  }
  virtual HeapWord** end_addr() const {
    guarantee(false, "inline contiguous allocation not supported");
    return NULL;
  }

  // Some heaps may be in an unparseable state at certain times between
  // collections. This may be necessary for efficient implementation of
  // certain allocation-related activities. Calling this function before
  // attempting to parse a heap ensures that the heap is in a parseable
  // state (provided other concurrent activity does not introduce
  // unparseability). It is normally expected, therefore, that this
  // method is invoked with the world stopped.
  // NOTE: if you override this method, make sure you call
  // super::ensure_parseability so that the non-generational
  // part of the work gets done. See implementation of
  // CollectedHeap::ensure_parseability and, for instance,
  // that of GenCollectedHeap::ensure_parseability().
  virtual void ensure_parseability();

  // Return an estimate of the maximum allocation that could be performed
  // without triggering any collection or expansion activity.  In a
  // generational collector, for example, this is probably the largest
  // allocation that could be supported (without expansion) in the youngest
  // generation.  It is "unsafe" because no locks are taken; the result
  // should be treated as an approximation, not a guarantee, for use in
  // heuristic resizing decisions.
  virtual size_t unsafe_max_alloc() = 0;

  // Section on thread-local allocation buffers (TLABs)
  // If the heap supports thread-local allocation buffers, it should override
  // the following methods:
  // Returns "true" iff the heap supports thread-local allocation buffers.
  // The default is "no".  
  virtual bool supports_tlab_allocation() const {
    return false;
  }
  // The amount of space available for thread-local allocation buffers.
  virtual size_t tlab_capacity() const {
    guarantee(false, "thread-local allocation buffers not supported");
    return 0;
  }
  // An estimate of the maximum allocation that could be performed
  // for thread-local allocation buffers without triggering any
  // collection or expansion activity.
  virtual size_t unsafe_max_tlab_alloc() const {
    guarantee(false, "thread-local allocation buffers not supported");
    return 0;
  }
  
  // Perform a collection of the heap; intended for use in implementing
  // "System.gc".  This probably implies as full a collection as the
  // "CollectedHeap" supports.
  virtual void collect(GCCause::Cause cause) = 0;

  // Returns the barrier set for this heap
  BarrierSet* barrier_set() { return _barrier_set; }

  // Returns "true" iff there is a stop-world GC in progress.  (I assume
  // that it should answer "false" for the concurrent part of a concurrent
  // collector -- dld).
  bool is_gc_active() { return _is_gc_active; }

  // Total number of GC collections
  unsigned int total_collections() { return _total_collections; }

  // Increment total number of GC collections
  // Should be protected but used by PSMarkSweep - cleanup for 1.4.2
  void increment_total_collections() { _total_collections++; }

  // Iterate over all the ref-containing fields of all objects, calling
  // "cl.do_oop" on each. This includes objects in permanent memory.
  virtual void oop_iterate(OopClosure* cl) = 0;

  // Iterate over all objects, calling "cl.do_object" on each.
  // This includes objects in permanent memory.
  virtual void object_iterate(ObjectClosure* cl) = 0;

  // Behaves the same as oop_iterate, except only traverses
  // interior pointers contained in permanent memory. If there
  // is no permanent memory, does nothing.
  virtual void permanent_oop_iterate(OopClosure* cl) = 0;

  // Behaves the same as object_iterate, except only traverses
  // object contained in permanent memory. If there is no
  // permanent memory, does nothing.
  virtual void permanent_object_iterate(ObjectClosure* cl) = 0;

  // NOTE! There is no requirement that a collector implement these
  // functions.
  //
  // A CollectedHeap is divided into a dense sequence of "blocks"; that is,
  // each address in the (reserved) heap is a member of exactly
  // one block.  The defining characteristic of a block is that it is
  // possible to find its size, and thus to progress forward to the next
  // block.  (Blocks may be of different sizes.)  Thus, blocks may
  // represent Java objects, or they might be free blocks in a
  // free-list-based heap (or subheap), as long as the two kinds are 
  // distinguishable and the size of each is determinable.

  // Returns the address of the start of the "block" that contains the
  // address "addr".  We say "blocks" instead of "object" since some heaps
  // may not pack objects densely; a chunk may either be an object or a
  // non-object. 
  virtual HeapWord* block_start(const void* addr) const = 0;

  // Requires "addr" to be the start of a chunk, and returns its size.
  // "addr + size" is required to be the start of a new chunk, or the end
  // of the active area of the heap.
  virtual size_t block_size(const HeapWord* addr) const = 0;

  // Requires "addr" to be the start of a block, and returns "TRUE" iff
  // the block is an object.
  virtual bool block_is_obj(const HeapWord* addr) const = 0;

  // Returns the longest time (in ms) that has elapsed since the last
  // time that any part of the heap was examined by a garbage collection.
  virtual jlong millis_since_last_gc() = 0;

  // Perform any cleanup actions necessary before allowing a verification.
  virtual void prepare_for_verify() = 0;

  virtual void print() const = 0;
  virtual void print_on(outputStream* st) const = 0;
  
  // Print all GC threads (other than the VM thread)
  // used by this heap.
  virtual void print_gc_threads() const = 0;
  // Iterator for all GC threads (other than VM thread)
  virtual void gc_threads_do(ThreadClosure* tc) const = 0;

  // Non product verification and debugging.
#ifndef PRODUCT
  virtual void verify(bool allow_dirty, bool silent) = 0;
#endif

  // If "addr" is a pointer into the (reserved?) heap, returns a positive
  // number indicating the "arena" within the heap in which "addr" falls.
  // Or else returns 0.
  virtual int addr_to_arena_id(void* addr) = 0;
};

// Class to set and reset the GC cause for a CollectedHeap.

class GCCauseSetter : StackObj {
  CollectedHeap* _heap;
  GCCause::Cause _previous_cause;
 public:
  GCCauseSetter(CollectedHeap* heap, GCCause::Cause cause) {
    _heap = heap;
    _previous_cause = _heap->gc_cause();
    _heap->set_gc_cause(cause);
  }
  ~GCCauseSetter() {
    _heap->set_gc_cause(_previous_cause);
  }
};
