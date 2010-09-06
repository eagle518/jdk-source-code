#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)extendedPC_i486.cpp	1.10 03/12/23 16:36:15 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_extendedPC_i486.cpp.incl"


ExtendedPC ExtendedPC::adjust(address begin, address end, address new_begin) {
  assert(is_contained_in(begin, end), "must be contained");

  // The compiler inserts the nop, nop sequence in loops to provide safepoints.
  // This nop, nop sequence is getting patched to an illegal instruction by
  // the safepoint code. If we stopped at the second nop instruction, the pc
  // will no longer point to a valid instruction.
  address new_pc = _pc;
  while(nativeInstruction_at(new_pc)->is_nop()) {    
    new_pc += NativeInstruction:: nop_instruction_size;       
  }
  new_pc = new_begin + (new_pc - begin);      
  return ExtendedPC(new_pc);
}


address ExtendedPC::contained_pc() { 
  return _pc; 
}


bool ExtendedPC::is_contained_in(address begin, address end) {
  return begin <= _pc && _pc < end;
}
