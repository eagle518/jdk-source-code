#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_AllocTable.hpp	1.6 03/12/23 16:38:57 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

//-------------------------------------------------------
//                 c1_AllocTable
//-------------------------------------------------------

// This class handles allocation and release of generic registers, 
// which are specified by their number.

class c1_AllocTable VALUE_OBJ_CLASS_SPEC {
 private:
  intx _state;          // bitmap: 0 means all registers free
  intx _or_state;       // or of all states over time
  intx _lockout;        // any register specified in _lockout should never be locked
  intx _none_available; // bitmap when all registers are locked
  intx _size;           // number of valid bits

  enum {
    allFreeState  = 0  // state when all registers are released
  };

  int c1_AllocTable::get_free_helper(intx lock_state);

 public:
  c1_AllocTable(int size);

  bool are_all_free  () const;
  bool has_one_free  () const;
  bool has_two_free  () const;
  bool has_pair_free () const;
  bool has_one_free_masked (c1_RegMask m) const;

  bool is_free             (int rnr) const;
  bool did_use_register    (int rnr) const;
  bool is_pair_free        (int rnr) const;
  bool are_free            (int r1, int r2) const;

  void set_free        (int rnr);
  void set_locked      (int rnr);
  void set_pair_free   (int rnr);
  void set_pair_locked (int rnr);

  int get_free ();
  int get_pair_free ();
  int get_free_masked (c1_RegMask m);

  // used for allocating a register representing
  // a double floating point register.
  bool has_double_free () const    { return pd_has_double_free(); }
  void set_double_locked (int rnr) { pd_set_double_locked(rnr); }
  int get_double_free ()           { return pd_get_double_free(); }

  c1_RegMask used_registers () const;
  c1_RegMask free_registers () const;

  // lock any registers which are locked in other.
  void merge(c1_AllocTable* other);

 private:
# include "incls/_c1_AllocTable_pd.hpp.incl"
};


//
// A simple class for allocating registers 
// 

class RegisterManager: public ResourceObj {
  enum {
    nof_cpu_regs = pd_nof_cpu_regs_frame_map,
    nof_fpu_regs = pd_nof_fpu_regs_frame_map
  };
  c1_AllocTable _cpu;
  c1_AllocTable _fpu;

 public:
  RegisterManager();

  void lock(RInfo reg);
  void free(RInfo reg);
  bool is_free_reg(RInfo reg);

  bool has_free_reg(ValueTag tag);
  RInfo lock_free_reg(ValueTag tag);

  int num_free_cpu();
  void lock_all_fpu();

  // lock any registers which are locked in other.
  void merge(RegisterManager* other) {
    _cpu.merge(&other->_cpu);
    _fpu.merge(&other->_fpu);
  }

  c1_RegMask free_cpu_registers() { return _cpu.free_registers(); }
  c1_RegMask free_fpu_registers() { return _fpu.free_registers(); }

  void print() const PRODUCT_RETURN;
};


