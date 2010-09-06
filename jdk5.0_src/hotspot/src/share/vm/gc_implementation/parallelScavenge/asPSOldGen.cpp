#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)asPSOldGen.cpp	1.8 03/12/23 16:40:06 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_asPSOldGen.cpp.incl"

// Whereas PSOldGen takes the maximum size of the generation
// (which doesn't change in the case of PSOldGen) as a parameter, 
// ASPSOldGen takes the upper limit on the size of
// the generation as a parameter.  In ASPSOldGen the
// maximum size of the generation can change as the boundary
// moves.  The "maximum size of the generation" is still a valid
// concept since the generation can grow and shrink within that
// maximum.  There are lots of useful checks that use that
// maximum.  In PSOldGen the method max_gen_size() returns
// _max_gen_size (as set by the PSOldGen constructor).  This
// is how it always worked.  In ASPSOldGen max_gen_size()
// returned the size of the reserved space for the generation.
// That can change as the boundary moves.  Below the limit of
// the size of the generation is passed to the PSOldGen constructor
// for "_max_gen_size" (have to pass something) but it is not used later.
// 
ASPSOldGen::ASPSOldGen(size_t initial_size,
                       size_t min_size, 
		       size_t size_limit,
                       const char* gen_name, 
		       int level) :
  PSOldGen(initial_size, min_size, size_limit, gen_name, level),
  _gen_size_limit(size_limit)

{
  assert(UseAdaptiveSizePolicy && UseAdaptiveGCBoundary, "Runtime check");
}

ASPSOldGen::ASPSOldGen(PSVirtualSpace* vs, 
		       size_t initial_size,
                       size_t min_size, 
		       size_t size_limit,
                       const char* gen_name, 
		       int level) :
  PSOldGen(initial_size, min_size, size_limit, gen_name, level),
  _gen_size_limit(size_limit)

{
  assert(UseAdaptiveSizePolicy && UseAdaptiveGCBoundary, "Runtime check");

  _virtual_space = vs;
}

void ASPSOldGen::reset_after_change() {
  _reserved = MemRegion((HeapWord*)virtual_space()->low_boundary(),
                        (HeapWord*)virtual_space()->high_boundary());
  post_resize();
}


size_t ASPSOldGen::available_for_expansion() {
  assert(UseAdaptiveSizePolicy && UseAdaptiveGCBoundary, "Runtime check");
  assert(virtual_space()->is_aligned(gen_size_limit()), "not aligned");
  assert(gen_size_limit() >= virtual_space()->committed_size(), "bad gen size");

  return gen_size_limit() - virtual_space()->committed_size();
}

size_t ASPSOldGen::available_for_contraction() {
  assert(UseAdaptiveSizePolicy && UseAdaptiveGCBoundary, "Runtime check");

  size_t uncommitted_bytes = virtual_space()->uncommitted_size();
  if (uncommitted_bytes != 0) {
    return uncommitted_bytes;
  }

  const size_t alignment = virtual_space()->alignment();
  const size_t used = align_size_up(used_in_bytes(), alignment);
  return reserved().byte_size() - MAX2(used, min_gen_size());
}
