/*
 * Copyright (c) 2000, 2003, Oracle and/or its affiliates. All rights reserved.
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

class ContigPermSpace;
class CardTableModRefBS;
class CompactingPermGenGen;
class PermanentGenerationSpec;

// A PermGen implemented with a contiguous space.
class CompactingPermGen:  public PermGen {
  friend class VMStructs;
protected:
  // The "generation" view.
  OneContigSpaceCardGeneration* _gen;

public:
  CompactingPermGen(ReservedSpace rs, ReservedSpace shared_rs,
                    size_t initial_byte_size, GenRemSet* remset,
                    PermanentGenerationSpec* perm_spec);

  HeapWord* mem_allocate(size_t size);

  void compute_new_size();

  Generation* as_gen() const { return _gen; }

};
