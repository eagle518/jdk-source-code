#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_AllocTable_i486.cpp	1.6 03/12/23 16:36:02 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

# include "incls/_precompiled.incl"
# include "incls/_c1_AllocTable_i486.cpp.incl"

//--------------------------------------------------------
//               c1_AllocTable
//--------------------------------------------------------


int c1_AllocTable::_reg_mask [c1_AllocTable::nofRegs];
int c1_AllocTable::_free_reg [c1_AllocTable::nofRegsExp2];


c1_AllocTable::c1_AllocTable (int size) {
  assert(nofRegs >= size && nofRegs >= 0, "too many registers");
  _state    = allFreeState;
  _or_state = _state;
  _none_available = (1 << size) - 1;
  _size = size;
}


void c1_AllocTable::init_tables () {
  for (int rnr = 0; rnr < nofRegs; rnr++) {
    _reg_mask[rnr] = nth_bit(rnr);
  }

  _free_reg[0] = FrameMap::cpu_reg2rnr(FrameMap::first_register()); // if all free use esi
  _free_reg[nofRegsExp2 - 1] = nofRegs;
  for (int i = 1; i < nofRegsExp2; i++) {
    for (int r = 0; r < nofRegs; r++) {
      if (!is_set_nth_bit(i, r)) {
        _free_reg[i] = r; break;
      }
    }
  }
}


// lock_state has bits set for each locked register
int c1_AllocTable::get_free_helper(intx lock_state) {
  int rnr = _free_reg[lock_state];
  _or_state = _or_state | _reg_mask[rnr];
  return rnr;
}

  
int c1_AllocTable::get_free () {
  assert(has_one_free(), "no free reg"); 
  return get_free_helper(_state);
}


int c1_AllocTable::get_free_masked (c1_RegMask m) {
  assert(has_one_free_masked(m), "no free reg in mask");
  return get_free_helper(_state | (~m._mask & _none_available));
}


