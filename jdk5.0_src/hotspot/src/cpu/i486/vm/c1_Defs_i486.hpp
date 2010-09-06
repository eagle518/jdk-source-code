#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_Defs_i486.hpp	1.14 03/12/23 16:36:05 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

// mapping between Java and native doubleword halves (little endian; stack grows toward lower addresses)
enum {
  pd_native_hi_word = first_java_word,
  pd_native_lo_word = second_java_word
};


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
  pd_nof_cpu_regs_frame_map = 8,  // number of registers used during code emission
  pd_nof_caller_save_cpu_regs_frame_map = 6,  // number of registers killed by calls
  pd_nof_cpu_regs_reg_alloc = 6,  // number of registers that are visible to register allocator
  pd_nof_fpu_regs_frame_map = 8,  // number of registers used during code emission
  pd_nof_fpu_regs_reg_alloc = 6   // number of registers that are visible to register allocator
};


// OopMap and RegisterMap construction need REG_COUNT, the maximum number of words of
// register context saved and recorded by a stub before it calls a non-leaf runtime function;
// FPU stack values are saved in double-precision format
const int pd_REG_COUNT = pd_nof_cpu_regs_frame_map + (2 * pd_nof_fpu_regs_frame_map);


// encoding of float value in debug info: 
enum {
  pd_float_saved_as_double = true
};


// for deoptimization: know how to find the pc-desc
enum {
  pd_jsr_call_offset = 5
};


