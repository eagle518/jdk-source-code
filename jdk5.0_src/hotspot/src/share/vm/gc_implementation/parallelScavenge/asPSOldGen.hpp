#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)asPSOldGen.hpp	1.8 04/02/03 16:45:35 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class ASPSOldGen : public PSOldGen {
  friend class VMStructs;
  size_t _gen_size_limit;  // Largest size the generation's reserved size
			   // can grow.
 public:
  ASPSOldGen(size_t initial_byte_size,
             size_t minimum_byte_size, 
	     size_t byte_size_limit,
             const char* gen_name, int level);
  ASPSOldGen(PSVirtualSpace* vs, 
	     size_t initial_byte_size,
             size_t minimum_byte_size, 
	     size_t byte_size_limit,
             const char* gen_name, int level);
  size_t gen_size_limit() 		{ return _gen_size_limit; }
  size_t max_gen_size() 		{ return _reserved.byte_size(); }
  void set_gen_size_limit(size_t v) 	{ _gen_size_limit = v; }

  // After a shrink or expand reset the generation
  void reset_after_change();

  // Return number of bytes that the virtual space in the generation is willing
  // to expand or contract.  The results from these methods should feed into the
  // decisions about adjusting the virtual space.
  size_t available_for_expansion();
  size_t available_for_contraction();

  // Accessors
  void set_reserved(MemRegion v) { _reserved = v; }

  // Debugging support
  virtual const char* short_name() const { return "ASPSOldGen"; }
};
