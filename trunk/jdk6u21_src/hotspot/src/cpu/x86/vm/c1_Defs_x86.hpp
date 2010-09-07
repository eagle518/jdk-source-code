/*
 * Copyright (c) 2000, 2008, Oracle and/or its affiliates. All rights reserved.
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

// native word offsets from memory address (little endian)
enum {
  pd_lo_word_offset_in_bytes = 0,
  pd_hi_word_offset_in_bytes = BytesPerWord
};

// explicit rounding operations are required to implement the strictFP mode
enum {
  pd_strict_fp_requires_explicit_rounding = true
};


// registers
enum {
  pd_nof_cpu_regs_frame_map = RegisterImpl::number_of_registers,       // number of registers used during code emission
  pd_nof_fpu_regs_frame_map = FloatRegisterImpl::number_of_registers,  // number of registers used during code emission
  pd_nof_xmm_regs_frame_map = XMMRegisterImpl::number_of_registers,    // number of registers used during code emission

#ifdef _LP64
  #define UNALLOCATED 4    // rsp, rbp, r15, r10
#else
  #define UNALLOCATED 2    // rsp, rbp
#endif // LP64

  pd_nof_caller_save_cpu_regs_frame_map = pd_nof_cpu_regs_frame_map - UNALLOCATED,  // number of registers killed by calls
  pd_nof_caller_save_fpu_regs_frame_map = pd_nof_fpu_regs_frame_map,  // number of registers killed by calls
  pd_nof_caller_save_xmm_regs_frame_map = pd_nof_xmm_regs_frame_map,  // number of registers killed by calls

  pd_nof_cpu_regs_reg_alloc = pd_nof_caller_save_cpu_regs_frame_map,  // number of registers that are visible to register allocator
  pd_nof_fpu_regs_reg_alloc = 6,  // number of registers that are visible to register allocator

  pd_nof_cpu_regs_linearscan = pd_nof_cpu_regs_frame_map, // number of registers visible to linear scan
  pd_nof_fpu_regs_linearscan = pd_nof_fpu_regs_frame_map, // number of registers visible to linear scan
  pd_nof_xmm_regs_linearscan = pd_nof_xmm_regs_frame_map, // number of registers visible to linear scan
  pd_first_cpu_reg = 0,
  pd_last_cpu_reg = NOT_LP64(5) LP64_ONLY(11),
  pd_first_byte_reg = 2,
  pd_last_byte_reg = 5,
  pd_first_fpu_reg = pd_nof_cpu_regs_frame_map,
  pd_last_fpu_reg =  pd_first_fpu_reg + 7,
  pd_first_xmm_reg = pd_nof_cpu_regs_frame_map + pd_nof_fpu_regs_frame_map,
  pd_last_xmm_reg =  pd_first_xmm_reg + pd_nof_xmm_regs_frame_map - 1
};


// encoding of float value in debug info:
enum {
  pd_float_saved_as_double = true
};
