#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)extendedPC.hpp	1.7 03/12/23 16:43:37 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// An ExtendedPC contains complete flow-of-control information for
// an architecture at any point of program execution.  For instance, on
// SPARC, this includes the PC as well as the nPC.  For IA, this is just
// Eip -- Thread::get_top_frame returns an ExtendedPC

class ExtendedPC VALUE_OBJ_CLASS_SPEC {
 private:
  address _pc;

 public:
  address pc() const { return _pc; }

  ExtendedPC adjust(address begin, address end, address new_begin);
  address contained_pc();
  bool is_contained_in(address begin, address end);

  // Machine dependent stuff
  #include "incls/_extendedPC_pd.hpp.incl"
};
