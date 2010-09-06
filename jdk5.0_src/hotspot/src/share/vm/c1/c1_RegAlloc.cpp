#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_RegAlloc.cpp	1.26 03/12/23 16:39:17 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_c1_RegAlloc.cpp.incl"



//-------------------------------------------------------
//                 Helper classes: RInfo2Reg and others
//-------------------------------------------------------

// This class "translates" the RInfo information into register numbers
// as used by c1_AllocTable class
class RInfo2Reg VALUE_OBJ_CLASS_SPEC {
 protected:
  RInfo       _rinfo;
  RegAlloc*   _ra;
 public:
  RInfo2Reg (RInfo rinfo, RegAlloc* ra) : _rinfo(rinfo), _ra(ra) {}

  void do_it() {
    if (_rinfo.is_word()) {
      do_cpu(_rinfo.reg());
    } else if (_rinfo.is_long()) {      
      do_cpu(_rinfo.reg_lo());
      do_cpu(_rinfo.reg_hi());
    } else if (_rinfo.is_float()) {
      do_float(_rinfo.float_reg());
    } else {
      assert(_rinfo.is_double(), "illegal type");
      do_double(_rinfo.double_reg());
    }
  }

  virtual void do_cpu    (int rnr) = 0;
  virtual void do_float  (int rnr) = 0;
  virtual void do_double (int rnr) = 0;
};


// locks registers using RInfo
class LockReg: public RInfo2Reg {
 private:
  Value _instr;
  int   _rc;
 public:
  LockReg(RInfo reg, RegAlloc* ra, Value instr, int rc): 
       RInfo2Reg(reg, ra), _instr(instr), _rc(rc) {}
  void do_cpu    (int rnr) { _ra->set_locked_cpu(rnr, _instr, _rc); }
  void do_float  (int rnr) { _ra->set_locked_fpu(rnr, _instr, _rc); }
  void do_double (int rnr) { _ra->set_locked_double(rnr, _instr, _rc); }
};


// Frees registers using RInfo
class FreeReg: public RInfo2Reg {
 public:
  FreeReg(RInfo reg, RegAlloc* ra): RInfo2Reg(reg, ra) {}
  void do_cpu    (int rnr) { _ra->set_free_cpu(rnr); }
  void do_float  (int rnr) { _ra->set_free_fpu(rnr); }
  void do_double (int rnr) { _ra->set_free_double(rnr); }
};


// Tests register state using RInfo
class IsFreeReg: public RInfo2Reg {
 private:
  bool _is_free;
 public:
  IsFreeReg (RInfo rinfo, const RegAlloc* ra) : RInfo2Reg(rinfo, (RegAlloc*)ra), _is_free(true) {}
  void do_cpu    (int rnr) { _is_free = _is_free && _ra->is_free_cpu(rnr);    }
  void do_float  (int rnr) { _is_free = _is_free && _ra->is_free_fpu(rnr);    }
  void do_double (int rnr) { _is_free = _is_free && _ra->is_free_double(rnr); }
  bool is_free() const { return _is_free; }
};


// Sets ref. count using RInfo
class SetReg: public RInfo2Reg {
 private:
  int   _rc;
  Value _value;
 public:
  SetReg (RInfo rinfo, RegAlloc* ra, int rc, Value value) :
      RInfo2Reg(rinfo, ra), _rc(rc), _value(value) {}
  void do_cpu    (int rnr) { _ra->set_cpu_reg    (rnr, _rc, _value); }
  void do_float  (int rnr) { _ra->set_fpu_reg    (rnr, _rc, _value); }
  void do_double (int rnr) { _ra->set_double_reg (rnr, _rc, _value); }
};


// Returns reference count of RInfo
class GetRefCount: public RInfo2Reg {
 private:
  int _result;
 public:
  GetRefCount(RInfo rinfo, const RegAlloc* ra): RInfo2Reg(rinfo, (RegAlloc*)ra), _result(-1) {}
  void do_cpu    (int rnr) { _result = _ra->get_cpu_rc(rnr); }
  void do_float  (int rnr) { _result = _ra->get_fpu_rc(rnr); }
  void do_double (int rnr) { _result = _ra->get_double_rc(rnr); }
  int get_rc()   const { return _result; }
};


// returns first value that covers the register
class GetValueFor: public RInfo2Reg {
 private:
  Value _result;
 public:
  GetValueFor(RInfo reg, const RegAlloc* ra): RInfo2Reg(reg, (RegAlloc*)ra), _result(NULL) {}
  void do_cpu(int rnr) { 
    if (!_ra->is_free_cpu(rnr) && _result == NULL) _result = _ra->get_check_cpu_val(rnr);
  }
  void do_float(int rnr) { 
    if (!_ra->is_free_fpu(rnr)    && _result == NULL) _result = _ra->get_check_fpu_val(rnr); 
  }
  void do_double(int rnr) { 
    if (!_ra->is_free_double(rnr) && _result == NULL) _result = _ra->get_double_val(rnr); 
  }
  Value value() const { return _result; }
};


class IsSpillLocked: public RInfo2Reg {
 private:
  bool _is_locked;
 public:
  IsSpillLocked(RInfo reg, const RegAlloc* ra): RInfo2Reg(reg, (RegAlloc*)ra), _is_locked(false) {}
  void do_cpu    (int rnr) { _is_locked |= _ra->is_locked_cpu_spill_count(rnr); }
  void do_float  (int rnr) { _is_locked |= _ra->is_locked_fpu_spill_count(rnr); }
  void do_double (int rnr) { _is_locked |= _ra->is_locked_double_spill_count(rnr); }
  bool is_locked () const { return _is_locked; }
};


// increments or decrements spill count
class ChangeSpillCount: public RInfo2Reg {
 private:
  int _delta; // addding this to count
 public:
  ChangeSpillCount(RInfo reg, RegAlloc* ra, bool do_incr): RInfo2Reg(reg, ra) {
    _delta = do_incr ? 1 : -1;
  }
  void do_cpu    (int rnr) { _ra->change_cpu_spill_count   (rnr, _delta); }
  void do_float  (int rnr) { _ra->change_fpu_spill_count   (rnr, _delta); }
  void do_double (int rnr) { _ra->change_double_spill_count(rnr, _delta); }
};


//-------------------------------------------------------
//                 SpillElem
//-------------------------------------------------------

class SpillElem: public CompilationResourceObj {
 private:
  int   _ref_count;
  bool  _is_oop;
  Value _instr;
 public:
   SpillElem(): _ref_count(0), _is_oop(false), _instr(NULL) {}

   void set (Value instr, int rc, bool is_oop) { 
     _instr = instr; _ref_count = rc; _is_oop = is_oop; 
   }

   Value  instr     () const { return _instr;     }
   int    ref_count () const { return _ref_count; }
   bool   is_oop    () const { return _is_oop;    }

   bool   is_free   () const { return _ref_count == 0; }

   void dec_ref_count  () { assert(_ref_count>0,""); _ref_count--; }
   void clear_ref_count() { _ref_count = 0; }

   void set_using(SpillElem* spill) {
     assert(is_free(), "elem must be free");
     _ref_count = spill->_ref_count;
     _is_oop    = spill->_is_oop;
     _instr     = spill->_instr;
   }

   void clear() {
     _ref_count = 0;
     _is_oop    = false;
     _instr     = NULL;
   }
};


//-------------------------------------------------------
//                 RegAlloc
//-------------------------------------------------------


RegAlloc::RegAlloc() : _cpu_alloc_table(nof_cpu_regs), _fpu_alloc_table(nof_fpu_regs) {
  _spill_area = new SpillElemList();
  _32bit_precision = false;
  _locking_locked  = false;
  assert(are_all_registers_free(), "bug in constructors");
  clear();
}


void RegAlloc::set_free_cpu(int rnr) {
  debug_only(cpu_range_check(rnr);)
    //  if (_cpu_rc[rnr] == 0) return;
  assert(_cpu_rc[rnr] > 0, "wrong ref count");
  _cpu_rc[rnr]--;
  if (_cpu_rc[rnr] == 0) _cpu_alloc_table.set_free(rnr);
}


void RegAlloc::set_free_fpu(int rnr) {
  debug_only(fpu_range_check(rnr);)
  assert(_fpu_rc[rnr] > 0, "wrong ref count");
  _fpu_rc[rnr]--;
  if (_fpu_rc[rnr] == 0) _fpu_alloc_table.set_free(rnr);
}


void RegAlloc::set_locked_cpu(int rnr, Value instr, int rc) {
  assert(!_locking_locked, "cannot lock");
  assert(0 <= rc, " sanity check");
  debug_only(cpu_range_check(rnr);)
  if (rc == 0) rc++;
  // note we allow instr to be NULL: anonymous lock
  _cpu_alloc_table.set_locked(rnr);
  _cpu_rc[rnr]     = rc;
  set_cpu_value(rnr, instr);
}


void RegAlloc::set_locked_fpu(int rnr, Value instr, int rc) {
  assert(!_locking_locked, "cannot lock");
  debug_only(fpu_range_check(rnr);)
  assert(rc > 0, "wrong rc");
  _fpu_alloc_table.set_locked(rnr);
  _fpu_rc[rnr]     = rc;
  set_fpu_value(rnr, instr);
}


void RegAlloc::set_cpu_reg (int rnr, int rc, Value value) {
  assert(!is_free_cpu(rnr), "register not locked");
  assert(0 < rc, " sanity check");
  _cpu_rc[rnr]     = rc;
  set_cpu_value(rnr, value);
}


void RegAlloc::set_fpu_reg (int rnr, int rc, Value value) {
  assert(!is_free_fpu(rnr), "register not locked");
  assert(rc > 0, "would free register");
  _fpu_rc[rnr]     = rc;
  set_fpu_value(rnr, value);
}


// returns true if _spill_area is empty
bool RegAlloc::is_none_spilled() const {
  for (int i = 0; i < _spill_area->length(); i++) {
    if (_spill_area->at(i)->ref_count() > 0) return false;
  }
  return true;
}


RInfoCollection* RegAlloc::regmask2rinfocollection(const c1_RegMask rm) {
  RInfoCollection* c = new RInfoCollection();
  for (int rnr = 0; rnr < nof_cpu_regs; rnr++) {
    if (rm.contains(rnr)) {      
      c->append(RInfo::word_reg(rnr));
    }
  }
  return c;
}


RInfo RegAlloc::get_free_reg (c1_RegMask mask) {
  RInfo r;
  int rnr = _cpu_alloc_table.get_free_masked(mask);
  r.set_word_reg(rnr);
  return r;
}


RInfo RegAlloc::get_lock_reg (Value instr, ValueType* type) {
  RInfo reg = get_free_reg (type);
  // if instr has use_count 0. then it must be a pinned
  // instruction with side effect but with no use; the register
  // will be released in ValueGen::finish_root
  int use_count = (instr == NULL || instr->use_count() == 0) ? 1 : instr->use_count();
  lock_reg (instr, reg, use_count);
  return reg;
}


RInfo RegAlloc::get_lock_reg(Value instr, c1_RegMask mask) {
  RInfo reg = get_free_reg(mask);
  // if instr has use_count 0. then it must be a pinned
  // instruction with side effect but with no use; the register
  // will be released in ValueGen::finish_root
  int use_count = (instr == NULL || instr->use_count() == 0) ? 1 : instr->use_count();
  lock_reg (instr, reg, use_count);
  return reg;
}


RInfo RegAlloc::get_lock_temp (Value instr, ValueType* type) {
  RInfo reg = get_free_reg (type);
  lock_reg (instr, reg, 1);
  return reg;
}


RInfo RegAlloc::get_lock_temp (Value instr, c1_RegMask mask) {
  RInfo reg = get_free_reg (mask);
  lock_reg (instr, reg, 1);
  return reg;
}


// locks reg on behalf of instr (reg must be free, instr may NOT null)
void RegAlloc::lock_register (Value instr, RInfo reg) {
  assert(instr != NULL, "instruction may not be null");
  lock_reg(instr, reg, instr->use_count());
}


// locks reg on behalf of instr (reg must be free, instr may be null, rc equal to 1)
void RegAlloc::lock_temp (Value instr, RInfo reg) {
  lock_reg(instr, reg, 1);
}


// "reg" is a register locked by some instruction. It is released and a new free register (selected from mask) is assigned to
// the instruction. reg and mask must be compatible, but reg must not be a member of mask. mask must have at least one free
// register. The reference count of the new register is the same as reg's reference count. The new assignment is returned.

RInfo RegAlloc::reallocate (RInfo reg, c1_RegMask mask) {
  assert(!is_free_reg(reg), "reg must be locked");
  assert(reg.is_word(), "reg and mask must be compatible (so far we only support integer masks)");
  assert(!mask.contains(reg), "reg should not be a member of mask");
  assert(!is_spill_locked(reg), "reg should not be spill locked");
  assert(has_free_reg(mask), "there must be at least one free register in mask");
  assert(get_value_for_rinfo(reg) != NULL, "reg should be locked on behalf of some instruction");

  Value value   = get_value_for_rinfo (reg);
  int   rc      = get_register_rc     (reg);
  RInfo new_reg = get_free_reg        (mask);

  for (int i = 0; i < rc; i++) {
    free_reg(reg);
  }
  assert(is_free_reg(reg), "reg must be free now");
  lock_reg(value, new_reg, rc);

  return new_reg;
}


bool RegAlloc::is_free_reg(RInfo reg) const {
  IsFreeReg ifr(reg, this);
  ifr.do_it();
  return ifr.is_free();
}


void RegAlloc::free_reg(RInfo reg) {
  FreeReg fr(reg, this);
  fr.do_it();
}


void RegAlloc::lock_reg(Value instr, RInfo reg, int rc) {
  assert(is_free_reg(reg), "no free register");
  // instructions with use count 0 still allocate registers, as it is released later;
  // therefore we set ref. count to 1 in such case, as we cannot lock with rc = 0
  if (rc == 0) rc = 1;
  LockReg lr(reg, this, instr, rc);
  lr.do_it();
}


void RegAlloc::set_reg(RInfo reg, int rc, Value value) {
  assert(!is_free_reg(reg), "register is not locked");
  SetReg setreg(reg, this, rc, value);
  setreg.do_it();
}


// returns the reference count for a given register
int RegAlloc::get_register_rc(RInfo reg) const {
  GetRefCount grc(reg, this);
  grc.do_it();
  return grc.get_rc();
}


Value RegAlloc::get_value_for_rinfo (RInfo reg) const {
  GetValueFor gvf(reg, this);
  gvf.do_it();
  assert(gvf.value() != NULL, "not found");
  return gvf.value();
}


bool RegAlloc::did_use_register (RInfo r) const {
  if (r.is_word()) {
    int rnr = r.reg();
    return _cpu_alloc_table.did_use_register(rnr);
  } else {
    Unimplemented();
  }
  return false;
}


void RegAlloc::clear() {
  int i;
  for (i = 0; i < nof_cpu_regs; i++) { _cpu_slock[i] = 0; _cpu_values[i] = NULL; }
  for (i = 0; i < nof_fpu_regs; i++) { _fpu_slock[i] = 0; _fpu_values[i] = NULL; }
}


bool RegAlloc::are_all_spill_locks_free() const {
  int i;
  for (i = 0; i < nof_cpu_regs; i++) {
    if (_cpu_slock[i] > 0) return false;
  }
  for (i = 0; i < nof_fpu_regs; i++) {
    if (_fpu_slock[i] > 0) return false;
  }
  return true;
}


bool RegAlloc::is_spill_locked(RInfo reg) const {
  IsSpillLocked isl(reg, this);
  isl.do_it();
  return isl.is_locked();
}


void RegAlloc::incr_spill_lock(RInfo reg) {
  ChangeSpillCount msc(reg, this, true);
  msc.do_it();
}


void RegAlloc::decr_spill_lock(RInfo reg) {
  ChangeSpillCount msc(reg, this, false);
  msc.do_it();
}


Value RegAlloc::get_smallest_value_to_spill (c1_RegMask mask) const {
  Value smallest_v = NULL;
  for (int i = 0; i < nof_cpu_regs; i++) {
    if (mask.contains(i)) {
      if (!is_free_cpu(i) && !is_locked_cpu_spill_count(i)) {
        Value v = get_cpu_value(i);
        // searching for value with smallest bci
        if (v != NULL) {
          assert(!is_spill_locked(v->item()->get_register()), "it is spill-locked");
          if (smallest_v == NULL || smallest_v->bci() > v->bci()) {
            smallest_v = v;
          }
        }
      }
    }
  }
  assert(smallest_v != NULL, "no spillable value found");
  assert(smallest_v->item()->is_register(), "not a register!");
  return smallest_v;
}


// Lock a spill-slot (or two); return the spill index
int RegAlloc::get_lock_spill(Value instr, int rc) {
  ValueType* type = instr->type();
  bool is_oop = type->is_object() || type->is_array();
  int lng = _spill_area->length();
  for (int i = 0; i < lng; i++) {
    SpillElem* spill = _spill_area->at(i);
    if (spill->is_free()) {
      if (type->is_double_word()) {
        if (i+1 >= _spill_area->length()) {
          _spill_area->append(new SpillElem());
        }
        SpillElem* spill2 = _spill_area->at(i+1);
        if (spill2->is_free()) {
          spill->set (instr, rc, false);
          spill2->set(instr, rc, false);
          return i;
        }
      } else {
        // one word type
        spill->set(instr, rc, is_oop);
        return i;
      }
    }
  }
  // no free spill slot found: extend spill area
  int res = _spill_area->length();
  SpillElem* spill = new SpillElem();
  spill->set(instr, rc, is_oop);
  _spill_area->append(spill);
  if (type->is_double_word()) {
    spill = new SpillElem();
    spill->set(instr, rc, is_oop);
    _spill_area->append(spill);
  }
  return res;
}


// spill slot spill_ix must be free; lock spill slot 
void RegAlloc::lock_spill(Value instr, int spill_ix, int rc) {
  ValueType* type = instr->type();
  bool is_oop = type->is_object() || type->is_array();
  int required_length = type->is_double_word() ? spill_ix + 2 : spill_ix + 1;
  if (required_length > _spill_area->length()) extend_spill_area(required_length);
  SpillElem* elem = _spill_area->at(spill_ix);
  assert(elem->ref_count() == 0, "already locked?");
  elem->set(instr, rc, is_oop);
  if (type->is_double_word()) {
    elem = _spill_area->at(spill_ix+1);
    assert(elem->ref_count() == 0, "already locked?");
    elem->set(instr, rc, is_oop);
  }
}


void RegAlloc::free_spill(int spill_ix, ValueType* type) {
  SpillElem* spill = _spill_area->at(spill_ix);
  spill->dec_ref_count();
  if (type->is_double_word()) {
    spill = _spill_area->at(spill_ix + 1);
    spill->dec_ref_count();
  }
}


bool RegAlloc::is_free_spill(int spill_ix, ValueType* type) const {
  if (spill_ix >= _spill_area->length()) return true;
  SpillElem* spill = _spill_area->at(spill_ix);
  bool is_free_1 = spill->is_free();
  bool is_free_2 = true;
  if (type->is_double_word() && spill_ix+1 < _spill_area->length()) {
    spill = _spill_area->at(spill_ix + 1);
    is_free_2 = spill->is_free();
  }
  return is_free_1 && is_free_2;
}


int RegAlloc::get_free_spill_after(int least_spill_ix, ValueType* type) {
  // search forward for an empty slot of the right size.
  for (int i = least_spill_ix; i < _spill_area->length(); i++) {
    SpillElem* spill = _spill_area->at(i);
    assert(spill != NULL, "");
    if (spill->is_free()) {
      if (type->is_double_word()) {
        // make sure the following slot is free, otherwise continue searching
        if (i+1 < _spill_area->length()) {
          if (_spill_area->at(i + 1)->is_free()) {
            return i;
          }
        } else {
          // there's one free slot at the end, so extend the spill area by one
          // word and return this slot.
          extend_spill_area(_spill_area->length() + 1);
          assert(_spill_area->at(i+1)->is_free(), "must be free");
          return i;
        }
      } else {
        // found a single free slot, so return it.
        return i;
      }
    }
  }

  // no free slot found, so extend the spill area
  int old_length = MAX2(_spill_area->length(), least_spill_ix);
  extend_spill_area(old_length + type->size()); // room enough for this type
#ifdef ASSERT
  SpillElem* spill = _spill_area->at(old_length);
  SpillElem* spill2 = type->is_double_word() ? _spill_area->at(old_length + 1) : NULL;
  assert(spill != NULL && spill->is_free(), "must be free");
  assert((spill2 == NULL && !type->is_double_word()) || spill2->is_free(), "must be free");
#endif

  return old_length;
}


Value RegAlloc::value_spilled_at(int spill_ix) const {
  SpillElem* spill = _spill_area->at(spill_ix);
  assert(spill->ref_count() > 0, "this is a released spill");
  assert(spill->instr() != NULL, "wrong type");
  return spill->instr();
}


int RegAlloc::get_ref_count_at(int spill_ix) const {
  return _spill_area->at(spill_ix)->ref_count();
}


void RegAlloc::move_spills(int to_spill_ix, int from_spill_ix, ValueType* type) {
  // from-spill must exist
  // update item information
  Value val = _spill_area->at(from_spill_ix)->instr();
  if (val->type()->size() == 1) {
    assert(val->item()->get_spilled_index() == from_spill_ix, "spill indeces do not match");
  } else {
    assert(val->item()->get_spilled_index() == from_spill_ix ||
           val->item()->get_spilled_index() == from_spill_ix - 1, "spill indeces do not match");
  }
  val->item()->set_spill_ix(to_spill_ix);

  // update spill area information
  int tst_lng = type->is_double_word() ? to_spill_ix + 2 : to_spill_ix + 1;
  if (tst_lng >= _spill_area->length()) extend_spill_area(tst_lng);
  for (int i = 0; i < type->size(); i++) {
    SpillElem* from = _spill_area->at(from_spill_ix + i);
    SpillElem* to   = _spill_area->at(to_spill_ix + i);
    to->set_using(from);
    from->clear();
  }  
  if (PrintC1RegAlloc) {
    tty->print_cr("   Moving spill from %d to %d", from_spill_ix, to_spill_ix);
  }
#ifdef ASSERT
  if (type->size() == 2) {
    assert(to_spill_ix + 1 < _spill_area->length(), "not enough room for spill");
    assert(from_spill_ix + 1 < _spill_area->length(), "not enough room for spill");
  } else {
    assert(to_spill_ix < _spill_area->length(), "not enough room for spill");
    assert(from_spill_ix < _spill_area->length(), "not enough room for spill");
  }
#endif
}


void RegAlloc::extend_spill_area(int new_lng) {
  int delta = new_lng - _spill_area->length();
  for (int i = 0; i < delta; i++) {
    _spill_area->append(new SpillElem());
  }
}


intStack* RegAlloc::oops_in_spill() const {
  if (_spill_area->length() == 0) return NULL;
  NEEDS_CLEANUP; // do not allocate stack if there are no oops in spill area (performance, footprint improvements)
  intStack* result = new intStack();
  int lng = _spill_area->length() - 1;
  for (;lng >= 0; lng--) {
    SpillElem* elem = _spill_area->at(lng);
    assert(elem != NULL, "no spill element");
    if (!elem->is_free() && elem->is_oop()) {
      result->append(lng);
    }
  }
  return result;
}


RInfoCollection* RegAlloc::oops_in_registers() const {
  RInfoCollection* result = new RInfoCollection();
  for (int i = 0; i < nof_cpu_regs; i++) {
    if (!is_free_cpu(i)) {
      if (get_cpu_value(i) != NULL && get_cpu_value(i)->type()->is_object()) {
        result->append(RInfo::word_reg(i));
      }
    }
  }
  return result;
}


#ifndef PRODUCT

void RegAlloc::check_registers() const {
  int i;
  for (i = 0; i < nof_cpu_regs; i++) {
    if (_cpu_alloc_table.is_free(i)) {
    } else {
      assert(_cpu_rc[i] >= 0,"");
      if (_cpu_rc[i] == 0) {
        assert(_cpu_alloc_table.is_free(i), "inconsistent information");
      }
      Value val = get_cpu_value(i);
      if (val != NULL) {
        assert(val->item() == NULL || val->item()->is_register(), "is not register");
      }
    }
  }
  for (i = 0; i < nof_fpu_regs; i++) {
    if (_fpu_alloc_table.is_free(i)) {
    } else {
      assert(_fpu_rc[i] >= 0, "inconsistent information");
      if (_fpu_rc[i] == 0) {
        assert(_fpu_alloc_table.is_free(i), "inconsistent information");
      }
    }
  }
  for (i = 0; i < _spill_area->length(); i++) {
    SpillElem* spill = _spill_area->at(i);
    assert(spill != NULL, "entry is not set");
  }
}


bool RegAlloc::check_spilled(Item* spilled) const {
  assert(spilled->is_spilled(), "wrong item");
  assert(spilled->get_spilled_index() < _spill_area->length(), "wrong spilled index");
  if (spilled->type()->size() == 2) {
    assert(spilled->get_spilled_index() + 1 < _spill_area->length(), "wrong spilled index");
  }
  return true;
} 


void RegAlloc::print () const {
  tty->print(" CPU[");
  int i;
  for (i = 0; i < nof_cpu_regs; i++) {
    if (is_free_cpu(i)) {
      tty->print("0");
    } else {
      assert(_cpu_rc[i] >= 0,"");
      tty->print("%d", _cpu_rc[i]);
      if (_cpu_rc[i] == 0) {
        assert(is_free_cpu(i), "inconsistent information");
      }
    }
  }
  tty->print("]  SCC[");
  for (i = 0; i < nof_cpu_regs; i++) {
    tty->print("%d", _cpu_slock[i]);
  }
  tty->print(" ]  FPU[");
  if (_fpu_alloc_table.are_all_free()) {
    tty->print(" all free ");
  } else {
    for (i = 0; i < nof_fpu_regs; i++) {
      if (is_free_fpu(i)) {
        // we do not preinitialize the _fpu_rc, therefore cannot test it;
        // it is set only after an fpu is allocated
        // assert(_fpu_rc[i] == 0, "inconsistent information");
        tty->print("0");
      } else {
        assert(_fpu_rc[i] >= 0, "inconsistent information");
        tty->print("%d", _fpu_rc[i]);
        if (_fpu_rc[i] == 0) {
          assert(is_free_fpu(i), "inconsistent information");
        }
      }
    }
  }
  tty->print("]  SCF[");
  for (i = 0; i < nof_fpu_regs; i++) {
    tty->print("%d", _fpu_slock[i]);
  }
  tty->print("]  SCC[");
  for (i = 0; i < nof_cpu_regs; i++) {
    tty->print("%d", _cpu_slock[i]);
  }
  tty->print("]  SPILL[");
  for (i = 0; i < _spill_area->length(); i++) {
    SpillElem* spill = _spill_area->at(i);
    tty->print("%d",spill == NULL ? -99 : spill->ref_count());
  }
  tty->print("] ");
  tty->cr();
}


void RegAlloc::print_values () const {
  for (int i=0; i < nof_cpu_regs; i++) {
    int lock_count = _cpu_alloc_table.is_free(i) ? 0 : _cpu_rc[i];
    int bci = get_cpu_value(i) == NULL ? -1 : get_cpu_value(i)->bci();
    tty->print_cr("[CPU%d 0x%x lock:%d bci:%d] ", i, get_cpu_value(i), lock_count, bci);
  }
  for (int j=0; j < nof_fpu_regs; j++) {
    tty->print_cr("[FPU%d 0x%x]", j, get_fpu_value(j));
  }
}

#endif //!PRODUCT
