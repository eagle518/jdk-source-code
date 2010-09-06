#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_AllocTable.cpp	1.6 03/12/23 16:38:57 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

# include "incls/_precompiled.incl"
# include "incls/_c1_AllocTable.cpp.incl"


bool c1_AllocTable::is_free (int rnr) const {
  debug_only(reg_check(rnr););
  return (_state & _reg_mask[rnr]) == 0;
}

bool c1_AllocTable::did_use_register (int rnr) const {
  debug_only(reg_check(rnr););
  return (_or_state & _reg_mask[rnr]) != 0;
}


bool c1_AllocTable::are_free (int r1, int r2) const {
  return is_free(r1) && is_free(r2);
}


bool c1_AllocTable::has_one_free () const {
  return _state != _none_available;
}


bool c1_AllocTable::has_one_free_masked (c1_RegMask m) const {
  return (_state | (~m._mask & _none_available)) != _none_available;
}


bool c1_AllocTable::has_two_free () const {   // for longs
  if (!has_one_free()) return false;
  c1_AllocTable* self = (c1_AllocTable*)this; // force cast to non-const type
  int r = self->get_free();
  self->set_locked(r);
  bool has_two = self->has_one_free();
  self->set_free(r);
  return has_two;
}


bool c1_AllocTable::are_all_free () const {
    return _state == allFreeState;
}


void c1_AllocTable::set_free (int rnr) {
  assert(!is_free(rnr), "not locked");     
  _state = _state & ~_reg_mask[rnr];
}


void c1_AllocTable::set_locked (int rnr) { 
  assert(is_free(rnr) , "already locked"); 
  _state    = _state | _reg_mask[rnr];  
  _or_state = _or_state | _state;
}


c1_RegMask c1_AllocTable::free_registers () const {
  c1_RegMask rm = c1_RegMask::empty_set();
  for (int rnr = 0; rnr < _size; rnr++) {
    if ((_or_state & _reg_mask[rnr]) == 0) {
      rm.add_reg(rnr);
    }
  }
  return rm;
}


c1_RegMask c1_AllocTable::used_registers () const {
  c1_RegMask rm = c1_RegMask::empty_set();
  for (int rnr = 0; rnr < _size; rnr++) {
    if ((_or_state & _reg_mask[rnr]) != 0) {
      rm.add_reg(rnr);
    }
  }
  return rm;
}


void c1_AllocTable::merge(c1_AllocTable* other) {
  assert(_size == other->_size, "must be same size");
  _state |= other->_state;
  _or_state |= other->_or_state;
}


RegisterManager::RegisterManager() : _cpu(nof_cpu_regs), _fpu(nof_fpu_regs) {
  for (int c = pd_nof_cpu_regs_reg_alloc; c < nof_cpu_regs; c++) {
    _cpu.set_locked(c);
  }
  for (int f = pd_nof_fpu_regs_reg_alloc; f < nof_fpu_regs; f++) {
    _fpu.set_locked(f);
  }
}


void RegisterManager::free(RInfo reg) {
  if (reg.is_single_cpu()) {
    _cpu.set_free(reg.cpu_regnr());
  } else if (reg.is_double_cpu()) {
    _cpu.set_free(reg.cpu_regnrLo());
    _cpu.set_free(reg.cpu_regnrHi());
  } else if (reg.is_single_fpu()) {
    _fpu.set_free(reg.fpu_regnr());
  } else if (reg.is_double_fpu()) {
    _fpu.set_free(reg.fpu_regnrLo());
    _fpu.set_free(reg.fpu_regnrHi());
  } else {
    ShouldNotReachHere();
  }
}



void RegisterManager::lock(RInfo reg) {
  if (reg.is_single_cpu()) {
    int rnr = reg.cpu_regnr();
    if (_cpu.is_free(rnr)) _cpu.set_locked(rnr);
  } else if (reg.is_double_cpu()) {
    int rnr = reg.cpu_regnrLo();
    if (_cpu.is_free(rnr)) _cpu.set_locked(rnr);
    rnr = reg.cpu_regnrHi();
    if (_cpu.is_free(rnr)) _cpu.set_locked(rnr);
  } else if (reg.is_single_fpu()) {
    int rnr = reg.fpu_regnr();
    if (_fpu.is_free(rnr)) _fpu.set_locked(rnr);
  } else if (reg.is_double_fpu()) {
    int rnr = reg.fpu_regnrLo();
    if (_fpu.is_free(rnr)) _fpu.set_locked(rnr);
    rnr = reg.fpu_regnrHi();
    if (_fpu.is_free(rnr)) _fpu.set_locked(rnr);
  } else {
    ShouldNotReachHere();
  }
}



bool RegisterManager::is_free_reg(RInfo reg) {
  if (reg.is_single_cpu()) {
    return _cpu.is_free(reg.cpu_regnr());
  } else if (reg.is_double_cpu()) {
    return _cpu.is_free(reg.cpu_regnrLo()) &&
      _cpu.is_free(reg.cpu_regnrHi());
  } else if (reg.is_single_fpu()) {
    return _fpu.is_free(reg.fpu_regnr());
  } else if (reg.is_double_fpu()) {
    return _fpu.is_free(reg.fpu_regnrLo()) && 
      _fpu.is_free(reg.fpu_regnrHi());
  }
  ShouldNotReachHere();
  return false;
}


void RegisterManager::lock_all_fpu() {
  for (int i = 0; i < nof_fpu_regs; i++) {
    if (_fpu.is_free(i)) {
      _fpu.set_locked(i);
    }
  }
  assert(!_fpu.has_one_free(), "must all be locked");
}


bool RegisterManager::has_free_reg(ValueTag tag) {
  switch (tag) {
  case intTag:
  case objectTag:
    return _cpu.has_one_free();
  case longTag: 
    return _cpu.has_two_free();
  case floatTag:
    return _fpu.has_one_free();
  case doubleTag:
    return _fpu.has_double_free();
  }
  ShouldNotReachHere();
}


RInfo RegisterManager::lock_free_reg(ValueTag tag) {
  RInfo result;
  switch (tag) {
  case intTag:
  case objectTag: {
    int rnr = _cpu.get_free();
    _cpu.set_locked(rnr);
    result.set_word_reg(rnr);
    break;
  }
  case longTag: {
    int rnr_lo = _cpu.get_free();
    _cpu.set_locked(rnr_lo);
    int rnr_hi = _cpu.get_free();
    _cpu.set_locked(rnr_hi);
    result.set_long_reg(rnr_lo, rnr_hi);
    break;
  }
  case floatTag: {
    int rnr = _fpu.get_free();
    _fpu.set_locked(rnr);
    result.set_float_reg(rnr);
    break;
  }
    
  case doubleTag: {
    int rnr = _fpu.get_double_free();
    _fpu.set_double_locked(rnr);
    result.set_double_reg(rnr);
    break;
  }
  default:
    ShouldNotReachHere();
  }
  assert(!is_free_reg(result), "must be locked");
  return result;
}


int RegisterManager::num_free_cpu() {
  int free = 0;
  for (int i = 0; i < nof_cpu_regs; i++) {
    if (_cpu.is_free(i)) {
      free++;
    }
  }
  return free;
}


#ifndef PRODUCT
void RegisterManager::print() const {
  tty->print(" cpu: ");
  int i;
  for (i = 0; i < nof_cpu_regs; i++) {
    if (_cpu.is_free(i)) {
      RInfo r;
      r.set_word_reg(i);
      r.print();
      tty->print(", ");
    }
  }
  tty->cr();
  tty->print(" fpu: ");
  if (_fpu.are_all_free()) {
    tty->print(" all free ");
  } else {
    for (i = 0; i < nof_fpu_regs; i++) {
      if (_fpu.is_free(i)) {
        RInfo r;
        r.set_float_reg(i);
        r.print();
        tty->print(", ");
      }
    }
  }
  tty->cr();
}
#endif


