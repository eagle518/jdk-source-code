#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)extendedPC_sparc.cpp	1.7 03/12/23 16:37:09 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_extendedPC_sparc.cpp.incl"


ExtendedPC ExtendedPC::adjust(address begin, address end, address new_begin) {
  assert(is_contained_in(_pc,  begin, end), "must be contained");
  assert(is_contained_in(_npc, begin, end), "must be contained");

  ExtendedPC addr(new_begin + (_pc  - begin), new_begin + (_npc - begin));
  return addr;
}

address ExtendedPC::contained_pc() { 
  return _npc; 
}

bool ExtendedPC::is_contained_in(address pc, address begin, address end) {
  return pc >= begin && pc < end;
}

bool ExtendedPC::is_contained_in(address begin, address end) {
  // if the npc must be in the range
  return is_contained_in(_npc, begin, end);
}
