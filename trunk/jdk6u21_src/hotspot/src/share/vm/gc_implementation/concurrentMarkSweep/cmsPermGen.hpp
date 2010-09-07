/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
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

class CardTableRS;   // fwd decl
class ConcurrentMarkSweepGeneration;

// A PermGen implemented with a CMS space, collected by a CMS collector.
class CMSPermGen:  public PermGen {
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
