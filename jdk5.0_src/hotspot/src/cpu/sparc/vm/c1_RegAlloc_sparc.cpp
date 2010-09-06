#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_RegAlloc_sparc.cpp	1.33 03/12/23 16:37:05 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_c1_RegAlloc_sparc.cpp.incl"




//---------------------------------------------------------
//          RegAlloc
//--------------------------------------------------------


bool RegAlloc::_init_done = false;


void RegAlloc::init_register_allocation() {
  if (_init_done) return;
  FrameMap::init();
  c1_AllocTable::init_tables();
  _init_done = true;
}


bool RegAlloc::has_free_reg(ValueType* type) const {
  return has_free_reg(type->tag());
}


bool RegAlloc::has_free_reg(ValueTag tag) const {
  switch (tag) {
    case floatTag:
      return _fpu_alloc_table.has_one_free();
    case doubleTag:
      return _fpu_alloc_table.has_pair_free();
    case longTag:
      return _cpu_alloc_table.has_two_free();
    case intTag:
    case objectTag:
    case addressTag:
      return _cpu_alloc_table.has_one_free();
    default:
      ShouldNotReachHere();
      return false;
  }
}


RInfo RegAlloc::get_free_reg (ValueType* type) {
  assert(has_free_reg(type), "no free register");
  return get_free_reg(type->tag());
}

RInfo RegAlloc::get_free_reg (ValueTag tag) {
  assert(has_free_reg(tag), "no free register");
  RInfo rinfo;
  switch (tag) {
  case intTag:
  case objectTag:
  case addressTag:
    rinfo = RInfo::word_reg(_cpu_alloc_table.get_free());
    break;
  case longTag:
    {
      int rnr_lo = _cpu_alloc_table.get_free(); ((RegAlloc*)this)->set_locked_cpu(rnr_lo, NULL, 1); // temp lock
      int rnr_hi = _cpu_alloc_table.get_free(); ((RegAlloc*)this)->set_free_cpu(rnr_lo);
      rinfo.set_long_reg(rnr_lo, rnr_hi);
    }
    break;  
  case floatTag:
    rinfo.set_float_reg(_fpu_alloc_table.get_free());
    break;
  case doubleTag:
    rinfo.set_double_reg(_fpu_alloc_table.get_pair_free());
    break;
  default:
    ShouldNotReachHere();
    break;
  };
  return rinfo;
}


bool RegAlloc::is_free_double (int rnr) const {
  return _fpu_alloc_table.is_pair_free(rnr);
}


void RegAlloc::set_free_double(int rnr) {
  debug_only(fpu_range_check(rnr);)
  assert(_fpu_rc[rnr] > 0, "wrong ref count");
  _fpu_rc[rnr]--;
  _fpu_rc[rnr+1]--;
  if (_fpu_rc[rnr] == 0) _fpu_alloc_table.set_pair_free(rnr);
}


void RegAlloc::set_locked_double(int rnr, Value instr, int rc) {
  debug_only(fpu_range_check(rnr);)
  assert(rc > 0, "wrong rc");
  _fpu_alloc_table.set_pair_locked(rnr);
  _fpu_rc[rnr]     = rc;
  _fpu_rc[rnr+1]   = rc;
  set_fpu_value(rnr, instr);
}


void RegAlloc::set_double_reg (int rnr, int rc, Value value) {
  assert(!is_free_double(rnr), "register not locked");
  assert(rc > 0, "would free register");
  _fpu_rc[rnr]     = rc;
  _fpu_rc[rnr+1]   = rc;
  set_fpu_value(rnr, value);
}


int RegAlloc::get_double_rc (int rnr) const {
  assert(!is_free_double(rnr), "register not locked");
  return _fpu_rc[rnr];
}


Value RegAlloc::get_double_val(int rnr) const {
  assert(!is_free_double(rnr), "register not locked");
  return get_fpu_value(rnr);
}


bool RegAlloc::is_locked_double_spill_count (int rnr) const {
  return _fpu_slock[rnr] != 0 && _fpu_slock[rnr+1] != 0;
}


void RegAlloc::change_double_spill_count (int rnr, int d) {
  _fpu_slock[ rnr ] += d; assert(_fpu_slock[ rnr ] >= 0, "illegal");
  _fpu_slock[rnr+1] += d; assert(_fpu_slock[rnr+1] >= 0, "illegal");
}


Value RegAlloc::get_smallest_value_to_spill(ValueType* type) const {
  Value smallest_v = NULL;

  if (type->is_float()) {
    for (int i = 0; i < nof_fpu_regs; i++) {
      Value v = NULL;
      if (!is_free_fpu(i) && !is_locked_fpu_spill_count(i)) v = get_fpu_value(i);
      // searching for value with smallest root
      // Sometimes we have a value whose item is not a register: this happens when
      // we are just handling the value and therefore do not spill it
      if (v != NULL && v->item() != NULL && v->item()->is_register()) {
        if (smallest_v == NULL || smallest_v->bci() > v->bci()) {
          smallest_v = v;
        }
      }
    }
  }
  else if (type->is_double()) {
    for (int i = 0; i < nof_fpu_regs; i += 2) {
      // Search for a double to spill
      Value v = NULL;
      if (!is_free_double(i) && !is_locked_double_spill_count(i)) v = get_fpu_value(i);
      if (v == NULL || !v->type()->is_double())
        continue;
      // searching for value with smallest root
      // Sometimes we have a value whose item is not a register: this happens when
      // we are just handling the value and therefore do not spill it
      if (v->item() != NULL && v->item()->is_register()) {
        if (smallest_v == NULL || smallest_v->bci() > v->bci()) {
          smallest_v = v;
        }
      }
    }
    if (smallest_v == NULL) {
      // No double item to spill. Spill upto two floats (one
      // per call to this function)
      for (int i = 0; i < nof_fpu_regs; i += 2) {
        // Search for a one/two floats to spill
        Value v = NULL;
        if (!is_free_fpu(i) && !is_locked_fpu_spill_count(i)) {
          v = get_fpu_value(i);
        } else if (!is_free_fpu(i + 1) && !is_locked_fpu_spill_count(i+1)) {
          v = get_fpu_value(i + 1);
        }
        // searching for value with smallest root
        // Sometimes we have a value whose item is not a register: this happens when
        // we are just handling the value and therefore do not spill it
        if (v != NULL && v->item() != NULL && v->item()->is_register()) {
          if (smallest_v == NULL || smallest_v->bci() > v->bci()) {
            smallest_v = v;
          }
        }
      }
    }
  }
  else {
    for (int i = 0; i < nof_cpu_regs; i++) {
      Value v = NULL;
      if (!is_free_cpu(i) && !is_locked_cpu_spill_count(i)) v = get_cpu_value(i);
      // searching for value with smallest root
      // Sometimes we have a value whose item is not a register: this happens when
      // we are just handling the value and therefore do not spill it
      if (v != NULL && v->item() != NULL && v->item()->is_register()) {
        if (smallest_v == NULL || smallest_v->bci() > v->bci()) {
          smallest_v = v;
        }
      }
    }
  }

  assert(smallest_v != NULL, "no spillable root found");
  assert(smallest_v->item()->is_register(), "not a register!");
  assert(!is_spill_locked(smallest_v->item()->get_register()), "is spill-locked");
  return smallest_v;
}
