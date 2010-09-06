#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_RegAlloc.hpp	1.24 03/12/23 16:39:17 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class SpillElem;
define_array(SpillElemArray, SpillElem*)
define_stack(SpillElemList, SpillElemArray)


//-------------------------------------------------------
//                 RegAlloc
//-------------------------------------------------------

// This class is used to handle register allocation and spilling for the C1
// It maintains allocated register, the reference counts of registers and the root
// instructions that have locked the register; register locked by non-root have a
// NULL entry there.

class RegAlloc: public CompilationResourceObj {
  friend class LockReg;
  friend class FreeReg;
  friend class IsFreeReg;
  friend class GetRefCount;
  friend class SetReg;
  friend class GetValueFor;
  friend class IsSpillLocked;
  friend class ChangeSpillCount;
  friend class LocalCaching;
  friend class SpillLockReg;
public:
  enum {
    nof_cpu_regs = pd_nof_cpu_regs_reg_alloc,
    nof_fpu_regs = pd_nof_fpu_regs_reg_alloc
  };

private:
  static bool    _init_done;
  c1_AllocTable  _cpu_alloc_table;
  c1_AllocTable  _fpu_alloc_table;
  SpillElemList* _spill_area;        // never copied! the state is always empty
  bool  _32bit_precision;
  bool  _locking_locked;

  // the ref counts contain values only if corresponding register is locked
  // default ref_count is 1 when locked; if >1, a separate call to set_rc must
  // be made
  int   _cpu_rc     [nof_cpu_regs];
  int   _fpu_rc     [nof_fpu_regs];
  int   _cpu_slock  [nof_cpu_regs];  // spill lock counter, 0 means may spill, >0 may not spill
  int   _fpu_slock  [nof_fpu_regs];  // spill lock counter, 0 means may spill, >0 may not spill
  Value _cpu_values [nof_cpu_regs];
  Value _fpu_values [nof_fpu_regs];

#ifndef PRODUCT
  static void cpu_range_check (int rnr)  { assert(0 <= rnr && rnr < nof_cpu_regs, "fpu register number is too big"); }
  static void fpu_range_check (int rnr)  { assert(0 <= rnr && rnr < nof_fpu_regs, "fpu register number is too big"); }
#endif

  void clear();

  bool is_free_cpu       (int rnr) const { return _cpu_alloc_table.is_free(rnr); }
  bool is_free_fpu       (int rnr) const { return _fpu_alloc_table.is_free(rnr); }
  bool is_free_double    (int rnr) const;
  void set_free_cpu      (int rnr);
  void set_free_fpu      (int rnr);
  void set_free_double   (int rnr);
  void set_locked_cpu    (int rnr, Value instr, int rc);
  void set_locked_fpu    (int rnr, Value instr, int rc);
  void set_locked_double (int rnr, Value instr, int rc);
  void set_cpu_reg       (int rnr, int rc, Value value);
  void set_fpu_reg       (int rnr, int rc, Value value);
  void set_double_reg    (int rnr, int rc, Value value);

  void set_cpu_value(int rnr, Value v)           { assert(0 <= rnr && rnr < nof_cpu_regs, "range check"); 
                                                   _cpu_values[rnr] = v;
                                                 }
  Value get_cpu_value(int rnr) const             { assert(0 <= rnr && rnr < nof_cpu_regs, "range check"); return _cpu_values[rnr]; }
  void set_fpu_value(int rnr, Value v)           { assert(0 <= rnr && rnr < nof_fpu_regs, "range check"); 
                                                   _fpu_values[rnr] = v;
                                                 }
  Value get_fpu_value(int rnr) const             { assert(0 <= rnr && rnr < nof_fpu_regs, "range check"); return _fpu_values[rnr]; }

  int   get_cpu_rc     (int rnr) const { assert(!is_free_cpu(rnr), "register not locked"); return _cpu_rc[rnr]; }
  int   get_fpu_rc     (int rnr) const { assert(!is_free_fpu(rnr), "register not locked"); return _fpu_rc[rnr]; }
  int   get_double_rc  (int rnr) const;
  Value get_check_cpu_val(int rnr) const { assert(!is_free_cpu(rnr), "register not locked"); return get_cpu_value(rnr); }
  Value get_check_fpu_val(int rnr) const { assert(!is_free_fpu(rnr), "register not locked"); return get_fpu_value(rnr); }
  Value get_double_val (int rnr) const;

  bool is_none_spilled () const;

  bool is_locked_cpu_spill_count    (int rnr) const { return _cpu_slock[rnr] != 0; }
  bool is_locked_fpu_spill_count    (int rnr) const { return _fpu_slock[rnr] != 0; }
  bool is_locked_double_spill_count (int rnr) const;

  void change_cpu_spill_count    (int rnr, int d) { _cpu_slock[rnr]+=d; assert(_cpu_slock[rnr] >= 0, "illegal"); }
  void change_fpu_spill_count    (int rnr, int d) { _fpu_slock[rnr]+=d; assert(_fpu_slock[rnr] >= 0, "illegal"); }
  void change_double_spill_count (int rnr, int d);

  void lock_reg (Value instr, RInfo reg, int rc);     // locks register setting ref. count to rc

public:
  static void init_register_allocation ();

  RegAlloc ();

  bool is_32bit_precision () const { return _32bit_precision; }

  // ------------------------ registers ---------------------------------
  RInfo get_free_reg (ValueTag tag);
  RInfo get_free_reg (ValueType* type);
  RInfo get_free_reg (c1_RegMask mask);
  bool has_free_reg (ValueType* type) const;
  bool has_free_reg (ValueTag tag) const;
  bool has_free_reg (c1_RegMask mask) const { return _cpu_alloc_table.has_one_free_masked(mask); }
  bool are_all_registers_free () const      { return _cpu_alloc_table.are_all_free() && _fpu_alloc_table.are_all_free(); }
  bool are_all_free () const                { return are_all_registers_free() && is_none_spilled() && are_all_spill_locks_free(); }

  RInfo get_lock_reg  (Value instr, ValueType* type); // finds free register, locks and returns it
  RInfo get_lock_reg  (Value instr, c1_RegMask mask); // finds free register, locks and returns it
  RInfo get_lock_temp (Value instr, ValueType* type); // finds free register, locks and returns it (with ref. count equal to 1)
  RInfo get_lock_temp (Value instr, c1_RegMask mask); // finds free register, locks and returns it (with ref. count equal to 1)

  void lock_register  (Value instr, RInfo reg); // locks reg on behalf of instr (reg must be free, instr may NOT be null)
  void lock_temp      (Value instr, RInfo reg); // locks reg on behalf of instr (reg must be free, instr may be null, rc equal to 1)


  // "reg" is a register locked by some instruction. It is released and a new free register (selected from mask) is assigned to
  // the instruction. reg and mask must be compatible, but reg must not be a member of mask. mask must have at least one free
  // register. The reference count of the new register is the same as reg's reference count. The new assignment is returned.
  RInfo reallocate (RInfo reg, c1_RegMask mask);

  bool  is_free_reg         (RInfo reg) const;
  void  free_reg            (RInfo reg);                          // decrements ref. count of register
  void  set_reg             (RInfo reg, int rc, Value value);
  int   get_register_rc     (RInfo reg) const;                    // returns reference count for given register
  Value get_value_for_rinfo (RInfo reg) const;
  bool  did_use_register    (RInfo reg) const;

  c1_RegMask used_registers () const { return _cpu_alloc_table.used_registers(); }
  c1_RegMask free_registers () const { return _cpu_alloc_table.free_registers(); }

  void lock_locking (bool f) { _locking_locked = f; }

  //------------------------ spilling -----------------------
  // spill locking:  a spill-locked register may not be spilled
  
  bool are_all_spill_locks_free () const;  // true if all registers may be spilled, false otherwise
  bool is_spill_locked (RInfo reg) const;
  void incr_spill_lock (RInfo reg);        // make reg not spillable
  void decr_spill_lock (RInfo reg);        // if the counter reaches zero then reg is spillable again

  // spill handling; spill slots are defined by their index; a two word value
  // with spill_index, occupies slots at spill_index and spill_index+1;
  // 0<= spill_index <= ...
  // Every spill slot has a reference count

  // finds a value that has locked a register and has the smallest bci 
  Value get_smallest_value_to_spill (ValueType* type) const;
  Value get_smallest_value_to_spill (c1_RegMask mask) const;

  int  get_lock_spill (Value instr, int rc);                     // finds a free spill slot, locks and returns it
  void lock_spill     (Value instr, int spill_ix, int rc);       // locks the spill_slot
  void free_spill     (int spill_ix, ValueType* type);           // decrements ref. count of specified spill slot
  bool is_free_spill  (int spill_ix, ValueType* type) const;     // returns true if needed spill_slots are free

  // returns the value that is spilled at specified spill index
  int   get_free_spill_after (int least_spill_ix, ValueType* type);
  Value value_spilled_at (int spill_ix) const;
  int   get_ref_count_at (int spill_ix) const;

  void  move_spills (int to_spill_ix, int from_spill_ix, ValueType* type);
  void  extend_spill_area (int new_lng);

  int   max_spills () const { return _spill_area->length(); }

  intStack* oops_in_spill () const;                              // returns spill indeces of oops in spill area
  RInfoCollection* oops_in_registers () const;                   // returns register numbers that contain oops
  static RInfoCollection* regmask2rinfocollection (const c1_RegMask rm);

  // debugging
  void check_registers () const             PRODUCT_RETURN;
  bool check_spilled (Item* spilled) const  PRODUCT_RETURN0;
  void print () const                       PRODUCT_RETURN;
  void print_values () const                PRODUCT_RETURN;
};
