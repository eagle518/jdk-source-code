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

// An ImmutableSpace is a viewport into a contiguous range
// (or subrange) of previously allocated objects.

// Invariant: bottom() and end() are on page_size boundaries and
// bottom() <= end()

class ImmutableSpace: public CHeapObj {
  friend class VMStructs;
 protected:
  HeapWord* _bottom;
  HeapWord* _end;

 public:
  ImmutableSpace()                   { _bottom = NULL; _end = NULL;  }
  HeapWord* bottom() const           { return _bottom;               }
  HeapWord* end() const              { return _end;                  }

  MemRegion region() const { return MemRegion(bottom(), end()); }

  // Initialization
  void initialize(MemRegion mr);

  bool contains(const void* p) const { return _bottom <= p && p < _end; }

  // Size computations.  Sizes are in bytes.
  size_t capacity_in_bytes() const            { return capacity_in_words() * HeapWordSize; }

  // Size computations.  Sizes are in heapwords.
  size_t capacity_in_words() const                { return pointer_delta(end(), bottom()); }
  virtual size_t capacity_in_words(Thread*) const { return capacity_in_words(); }

  // Iteration.
  virtual void oop_iterate(OopClosure* cl);
  virtual void object_iterate(ObjectClosure* cl);

  // Debugging
  virtual void print() const            PRODUCT_RETURN;
  virtual void print_short() const      PRODUCT_RETURN;
  virtual void verify(bool allow_dirty);
};
