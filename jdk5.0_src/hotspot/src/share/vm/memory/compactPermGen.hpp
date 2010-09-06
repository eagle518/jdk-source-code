#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)compactPermGen.hpp	1.11 03/12/23 16:40:59 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
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
