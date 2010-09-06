#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)permGen.hpp	1.28 04/01/11 09:12:37 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// All heaps contains a "permanent generation," containing permanent
// (reflective) objects.  This is like a regular generation in some ways,
// but unlike one in others, and so is split apart.

class Generation;
class ConcurrentMarkSweepGeneration;
class GenRemSet;
class CSpaceCounters;

// PermGen models the part of the heap 

class PermGen : public CHeapObj {
  friend class VMStructs;
 protected:
  size_t _capacity_expansion_limit;  // maximum expansion allowed without a
				     // full gc occuring

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

class CardTableRS;   // fwd decl

// A PermGen implemented with a CMS space, collected by a CMS collector.
class CMSPermGen:  public PermGen {
  HeapWord* CMSPermGen::check_lock_and_allocate(bool lock_owned, size_t size);
  void CMSPermGen::check_lock_and_collect(bool lock_owned, GCCause::Cause cause);

  friend class VMStructs;
 protected:
  // The "generation" view.
  ConcurrentMarkSweepGeneration* _gen;

 public:
  CMSPermGen(ReservedSpace rs, size_t initial_byte_size,
             CardTableRS* ct, FreeBlockDictionary::DictionaryChoice);

  HeapWord* mem_allocate(size_t size);

  void compute_new_size();

  Generation* as_gen() const { return _gen; }

  // Override of parent's virtual
  void oop_iterate(OopClosure* cl) {
    assert(_gen != NULL, "_gen is NULL");
    _gen->get_locks_and_oop_iterate(cl);
  }

  // Override of parent's virtual
  virtual void object_iterate(ObjectClosure* cl) {
    assert(_gen != NULL, "_gen is NULL");
    _gen->get_locks_and_object_iterate(cl);
  }
};

// This is the "generation" view of a CMSPermGen.
class CMSPermGenGen: public ConcurrentMarkSweepGeneration {
  // Abstractly, this is a subtype that gets access to protected fields.
  friend class CMSPermGen;
public:
  CMSPermGenGen(ReservedSpace rs, size_t initial_byte_size,
                       int level, CardTableRS* ct):
    // See comments in the constructor for CompactibleFreeListSpace
    // regarding not using adaptive free lists for a perm gen.
    ConcurrentMarkSweepGeneration(rs, initial_byte_size, // MinPermHeapExapnsion
      level, ct, false /* use adaptive freelists */,
      (FreeBlockDictionary::DictionaryChoice)CMSDictionaryChoice)
  {}

  void initialize_performance_counters();

  const char* name() const {
    return "concurrent-mark-sweep perm gen";
  }

  const char* short_name() const {
    return "CMS Perm";
  }

  bool must_be_youngest() const { return false; }
  bool must_be_oldest() const { return false; }
};
