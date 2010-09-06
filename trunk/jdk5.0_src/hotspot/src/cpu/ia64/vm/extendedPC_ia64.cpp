#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)extendedPC_ia64.cpp	1.4 03/12/23 16:36:39 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_extendedPC_ia64.cpp.incl"


ExtendedPC ExtendedPC::adjust(address begin, address end, address new_begin) {
  assert(is_contained_in(begin, end), "must be contained");

  address new_pc = new_begin + (_pc - begin);      
  return ExtendedPC(new_pc);
}


address ExtendedPC::contained_pc() { 
  return _pc; 
}


bool ExtendedPC::is_contained_in(address begin, address end) {
  return begin <= _pc && _pc < end;
}
