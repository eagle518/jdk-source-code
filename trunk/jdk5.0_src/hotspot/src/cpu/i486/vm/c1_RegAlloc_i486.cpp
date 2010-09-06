#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_RegAlloc_i486.cpp	1.61 03/12/23 16:36:10 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

# include "incls/_precompiled.incl"
# include "incls/_c1_RegAlloc_i486.cpp.incl"



//---------------------------------------------------------
//          RegAlloc  
//--------------------------------------------------------


void RegAlloc::init_register_allocation() {
  FrameMap::init();
  c1_AllocTable::init_tables();
}


bool RegAlloc::has_free_reg(ValueType* type) const {
  return has_free_reg(type->tag());
}


bool RegAlloc::has_free_reg(ValueTag tag) const {
  switch (tag) {
    case floatTag:
    case doubleTag:
      return _fpu_alloc_table.has_one_free();
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
    rinfo.set_double_reg(_fpu_alloc_table.get_free());
    break;
  default:
    ShouldNotReachHere();
    break;
  };
  return rinfo;
}


bool RegAlloc::is_free_double (int rnr) const {
  return is_free_fpu(rnr);
}


void RegAlloc::set_free_double (int rnr) {
  set_free_fpu(rnr);
}


void RegAlloc::set_locked_double (int rnr, Value instr, int rc) {
  set_locked_fpu(rnr, instr, rc);
}


void RegAlloc::set_double_reg (int rnr, int rc, Value value) {
  set_fpu_reg(rnr, rc, value);
}


int RegAlloc::get_double_rc (int rnr) const {
  return get_fpu_rc(rnr);
}


Value RegAlloc::get_double_val (int rnr) const {
  return get_check_fpu_val(rnr);
}


bool RegAlloc::is_locked_double_spill_count (int rnr) const {
  return is_locked_fpu_spill_count(rnr);
}


void RegAlloc::change_double_spill_count (int rnr, int d) {
  change_fpu_spill_count(rnr, d);
}


Value RegAlloc::get_smallest_value_to_spill(ValueType* type) const {
  bool is_fpu = type->is_float() || type->is_double();
  Value smallest_v = NULL;
  for (int i = 0; i < c1_AllocTable::nofRegs; i++) {
    Value v = NULL;
    if      ( is_fpu && !is_free_fpu(i) && !is_locked_fpu_spill_count(i)) v = get_fpu_value(i);
    else if (!is_fpu && !is_free_cpu(i) && !is_locked_cpu_spill_count(i)) v = get_cpu_value(i);
    // searching for value with smallest root
    // Sometimes we have a value whose item is not a register: this happens when
    // we are just handling the value and therefore do not spill it
    if (v != NULL && v->item() != NULL && v->item()->is_register()) {
      assert(!is_spill_locked(v->item()->get_register()), "is spill-locked");
      if (smallest_v == NULL) smallest_v = v;
      if (smallest_v->bci() > v->bci()) {
        smallest_v = v;
      }
    }
  }
  assert(smallest_v != NULL, "no spillable root found");
  assert(smallest_v->item()->is_register(), "not a register!");
  return smallest_v;
}
