#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_Defs_sparc.hpp	1.14 03/12/23 16:36:59 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

// mapping between Java and native doubleword halves (big endian; stack grows toward lower addresses)
enum {
  pd_native_lo_word = first_java_word,
  pd_native_hi_word = second_java_word
};


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
  pd_nof_caller_save_cpu_regs_frame_map = 6,  // number of registers killed by calls
  pd_nof_cpu_regs_reg_alloc = 20,  // number of registers that are visible to register allocator
  pd_nof_fpu_regs_frame_map = 32,  // number of registers used during code emission
  pd_nof_fpu_regs_reg_alloc = 32   // number of registers that are visible to register allocator
};


// OopMap and RegisterMap construction
const int pd_REG_COUNT = pd_nof_cpu_regs_frame_map + pd_nof_fpu_regs_frame_map;


// for debug info: a float value in a register is saved in single precision by runtime stubs
enum {
  pd_float_saved_as_double = false
};


// for deoptimization: know how to find the pc-desc
enum {
  pd_jsr_call_offset = 4*BytesPerInstWord
};

