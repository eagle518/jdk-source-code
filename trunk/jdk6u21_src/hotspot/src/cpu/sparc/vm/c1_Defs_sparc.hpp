/*
 * Copyright (c) 2000, 2009, Oracle and/or its affiliates. All rights reserved.
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

// native word offsets from memory address (big endian)
enum {
  pd_lo_word_offset_in_bytes = BytesPerInt,
  pd_hi_word_offset_in_bytes = 0
};


// explicit rounding operations are not required to implement the strictFP mode
enum {
  pd_strict_fp_requires_explicit_rounding = false
};


// registers
enum {
  pd_nof_cpu_regs_frame_map = 32,  // number of registers used during code emission
  pd_nof_caller_save_cpu_regs_frame_map = 10,  // number of cpu registers killed by calls
  pd_nof_cpu_regs_reg_alloc = 20,  // number of registers that are visible to register allocator
  pd_nof_cpu_regs_linearscan = 32,// number of registers visible linear scan
  pd_first_cpu_reg = 0,
  pd_last_cpu_reg = 31,
  pd_last_allocatable_cpu_reg = 19,
  pd_first_callee_saved_reg = 0,
  pd_last_callee_saved_reg = 13,

  pd_nof_fpu_regs_frame_map = 32,  // number of registers used during code emission
  pd_nof_caller_save_fpu_regs_frame_map = 32,  // number of fpu registers killed by calls
  pd_nof_fpu_regs_reg_alloc = 32,  // number of registers that are visible to register allocator
  pd_nof_fpu_regs_linearscan = 32, // number of registers visible to linear scan
  pd_first_fpu_reg = pd_nof_cpu_regs_frame_map,
  pd_last_fpu_reg =  pd_nof_cpu_regs_frame_map + pd_nof_fpu_regs_frame_map - 1,

  pd_nof_xmm_regs_linearscan = 0,
  pd_nof_caller_save_xmm_regs = 0,
  pd_first_xmm_reg = -1,
  pd_last_xmm_reg = -1
};


// for debug info: a float value in a register is saved in single precision by runtime stubs
enum {
  pd_float_saved_as_double = false
};
