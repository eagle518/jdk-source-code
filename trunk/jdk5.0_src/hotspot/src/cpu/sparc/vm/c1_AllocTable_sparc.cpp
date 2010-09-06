#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_AllocTable_sparc.cpp	1.6 03/12/23 16:36:57 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

# include "incls/_precompiled.incl"
# include "incls/_c1_AllocTable_sparc.cpp.incl"


//--------------------------------------------------------
//               c1_AllocTable
//--------------------------------------------------------


intx c1_AllocTable::_reg_mask[c1_AllocTable::max];


c1_AllocTable::c1_AllocTable (int size) : _size(size) {
  assert(sizeof(_state) * BitsPerByte >= max, "bit vector must fit in _state");
  assert(max >= size && size >= 0, "too many registers");
  _state    = 0;
  _or_state = 0;
  _none_available = (_size == max) ? ~0 : (1 << _size) - 1;
}


bool c1_AllocTable::is_pair_free (int rnr) const {
  return are_free(rnr, rnr+1);
}


bool c1_AllocTable::has_pair_free () const {   // for doubles
  intx mask = 3;
  assert(mask != 0, "too many registers");
  for (int rnr = 0; rnr < _size - 1; rnr += 2, mask <<= 2) {
    if ((_state & mask) == 0)
      return true;
  }
  return false;
}


void c1_AllocTable::set_pair_free (int rnr) {
  assert(!is_free(rnr) && !is_free(rnr+1), "not locked");
  assert(rnr % 2 == 0, "pairs start at even boundaries");
  _state = _state & ~(_reg_mask[rnr] | _reg_mask[rnr+1]);
}


void c1_AllocTable::set_pair_locked (int rnr) {
  assert(is_free(rnr) && is_free(rnr+1) , "already locked");
  assert(rnr % 2 == 0, "pairs start at even boundaries");
  _state = _state | _reg_mask[rnr] | _reg_mask[rnr+1];
  _or_state |= _state;
}


int c1_AllocTable::get_pair_free () {
  assert(has_pair_free(), "no free pair"); 
  int  rnr;
  intx mask = 3;
  assert(mask != 0, "too many registers");
  for (rnr = 0; rnr < _size; rnr += 2, mask <<= 2) {
    if ((_state & mask) == 0) {
      _or_state = _or_state | _reg_mask[rnr] | _reg_mask[rnr+1];
      return rnr;
    }
  }
  ShouldNotReachHere();
  return -1;
}


// lock_state has bits set for each locked register
int c1_AllocTable::get_free_helper(intx lock_state) {
  for (int rnr = 0; rnr < _size; rnr++) {
    if ((lock_state & _reg_mask[rnr]) == 0) {
      _or_state = _or_state | _reg_mask[rnr];
      return rnr;
    }
  }
  ShouldNotReachHere();
  return -1;
}


int c1_AllocTable::get_free () {
  assert(has_one_free(), "no free reg"); 
  return get_free_helper(_state);
}


int c1_AllocTable::get_free_masked (c1_RegMask m) {
  assert(has_one_free_masked(m), "no free reg in mask");
  return get_free_helper(_state | ~m._mask);
}


void c1_AllocTable::init_tables () {
  for (int i = 0; i < max; i++) {
    _reg_mask[i] = nth_bit(i);
  }
}

