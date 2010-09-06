#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)mutableSpace.hpp	1.11 03/12/23 16:40:29 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// A MutableSpace is a subtype of ImmutableSpace that supports the
// concept of allocation. This includes the concepts that a space may
// be only partially full, and the querry methods that go with such
// an assumption.
//
// Invariant: (ImmutableSpace +) bottom() <= top() <= end() 
// top() is inclusive and end() is exclusive.

class MutableSpace: public ImmutableSpace {
  friend class VMStructs;
 protected:
  HeapWord* _top;

 public:
  // Accessors
  HeapWord* top() const                    { return _top;    }
  void set_top(HeapWord* value)            { _top = value; }

  HeapWord** top_addr()                    { return &_top; }
  HeapWord** end_addr()                    { return &_end; }

  void set_bottom(HeapWord* value) { _bottom = value; }
  void set_end(HeapWord* value)    { _end = value; }

  // Returns a subregion containing all objects in this space.
  MemRegion used_region() { return MemRegion(bottom(), top()); }

  // Initialization
  void initialize(MemRegion mr, bool clear_space);
  void clear();

  // Overwrites the unused portion of this space. Note that some collectors
  // may use this "scratch" space during collections.
  virtual void mangle_unused_area() {
    mangle_region(MemRegion(_top, _end));
  }

  void mangle_region(MemRegion mr) {
    debug_only(Copy::fill_to_aligned_words(mr.start(), mr.word_size(), badHeapWord));
  }

  // Boolean querries.
  bool is_empty() const              { return used_in_words() == 0; }
  bool not_empty() const             { return used_in_words() > 0; }
  bool contains(const void* p) const { return _bottom <= p && p < _end; }

  // Size computations.  Sizes are in bytes.
  size_t used_in_bytes() const                { return used_in_words() * HeapWordSize; }
  size_t free_in_bytes() const                { return free_in_words() * HeapWordSize; }

  // Size computations.  Sizes are in heapwords.
  size_t used_in_words() const                { return pointer_delta(top(), bottom()); }
  size_t free_in_words() const                { return pointer_delta(end(),    top()); }

  // Allocation (return NULL if full)
  HeapWord* allocate(size_t word_size);
  HeapWord* cas_allocate(size_t word_size);

  // Iteration.
  virtual void oop_iterate(OopClosure* cl);
  virtual void object_iterate(ObjectClosure* cl);

  // Debugging
  virtual void print() const;
  virtual void print_on(outputStream* st) const;
  virtual void print_short() const;
  virtual void print_short_on(outputStream* st) const;
  virtual void verify(bool allow_dirty) const PRODUCT_RETURN;
};
