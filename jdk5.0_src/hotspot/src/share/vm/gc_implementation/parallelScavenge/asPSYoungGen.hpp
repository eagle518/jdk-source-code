#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)asPSYoungGen.hpp	1.10 04/02/03 16:45:38 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class ASPSYoungGen : public PSYoungGen {
  friend class VMStructs;
 private:
  size_t _gen_size_limit;
 protected:
  virtual size_t available_to_live();

 public:
  ASPSYoungGen(size_t         initial_byte_size, 
               size_t         minimum_byte_size,
	       size_t         byte_size_limit);

  ASPSYoungGen(PSVirtualSpace* vs,
	       size_t         initial_byte_size, 
               size_t         minimum_byte_size,
	       size_t         byte_size_limit);

  void initialize(ReservedSpace rs, size_t alignment);
  void initialize_virtual_space(ReservedSpace rs, size_t alignment);

  size_t gen_size_limit() { return _gen_size_limit; }
  void set_gen_size_limit(size_t v) { _gen_size_limit = v; }

  bool resize_generation(size_t eden_size, size_t survivor_size);
  void resize_spaces(size_t eden_size, size_t survivor_size);

  // Adjust eden to be consistent with the virtual space.
  void reset_after_change();

  // Adaptive size policy support
  // Return number of bytes that the generation can expand/contract.
  size_t available_for_expansion();
  size_t available_for_contraction();

  // Accessors
  void set_reserved(MemRegion v) { _reserved = v; }

  // Printing support
  virtual const char* short_name() const { return "ASPSYoungGen"; }
};
