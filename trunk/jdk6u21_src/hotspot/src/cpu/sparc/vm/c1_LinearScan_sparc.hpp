/*
 * Copyright (c) 2005, 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

inline bool LinearScan::is_processed_reg_num(int reg_num) {
  return reg_num < 26 || reg_num > 31;
}

inline int LinearScan::num_physical_regs(BasicType type) {
  // Sparc requires two cpu registers for long
  // and two cpu registers for double
#ifdef _LP64
  if (type == T_DOUBLE) {
#else
  if (type == T_DOUBLE || type == T_LONG) {
#endif
    return 2;
  }
  return 1;
}


inline bool LinearScan::requires_adjacent_regs(BasicType type) {
#ifdef _LP64
  return type == T_DOUBLE;
#else
  return type == T_DOUBLE || type == T_LONG;
#endif
}

inline bool LinearScan::is_caller_save(int assigned_reg) {
  return assigned_reg > pd_last_callee_saved_reg && assigned_reg <= pd_last_fpu_reg;
}


inline void LinearScan::pd_add_temps(LIR_Op* op) {
  // No special case behaviours yet
}


inline bool LinearScanWalker::pd_init_regs_for_alloc(Interval* cur) {
  if (allocator()->gen()->is_vreg_flag_set(cur->reg_num(), LIRGenerator::callee_saved)) {
    assert(cur->type() != T_FLOAT && cur->type() != T_DOUBLE, "cpu regs only");
    _first_reg = pd_first_callee_saved_reg;
    _last_reg = pd_last_callee_saved_reg;
    return true;
  } else if (cur->type() == T_INT || cur->type() == T_LONG || cur->type() == T_OBJECT) {
    _first_reg = pd_first_cpu_reg;
    _last_reg = pd_last_allocatable_cpu_reg;
    return true;
  }
  return false;
}
