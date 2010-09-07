/*
 * Copyright (c) 2000, 2008, Oracle and/or its affiliates. All rights reserved.
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

// All heaps contains a "permanent generation," containing permanent
// (reflective) objects.  This is like a regular generation in some ways,
// but unlike one in others, and so is split apart.

class Generation;
class GenRemSet;
class CSpaceCounters;

// PermGen models the part of the heap

class PermGen : public CHeapObj {
  friend class VMStructs;
 protected:
  size_t _capacity_expansion_limit;  // maximum expansion allowed without a
                                     // full gc occurring

  HeapWord* mem_allocate_in_gen(size_t size, Generation* gen);

 public:
  enum Name {
    MarkSweepCompact, MarkSweep, ConcurrentMarkSweep
  };

  // Permanent allocation (initialized)
  virtual HeapWord* mem_allocate(size_t size) = 0;

  // Mark sweep support
  virtual void compute_new_size() = 0;

  // Ideally, we would use MI (IMHO) but we'll do delegation instead.
  virtual Generation* as_gen() const = 0;

  virtual void oop_iterate(OopClosure* cl) {
    Generation* g = as_gen();
    assert(g != NULL, "as_gen() NULL");
    g->oop_iterate(cl);
  }

  virtual void object_iterate(ObjectClosure* cl) {
    Generation* g = as_gen();
    assert(g != NULL, "as_gen() NULL");
    g->object_iterate(cl);
  }

  // Performance Counter support
  virtual void update_counters() {
    Generation* g = as_gen();
    assert(g != NULL, "as_gen() NULL");
    g->update_counters();
  }
};
