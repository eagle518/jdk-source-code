#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_Defs.hpp	1.13 03/12/23 16:39:03 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

// set frame size and return address offset to these values in blobs
// (if the compiled frame uses ebp as link pointer on IA; otherwise,
// the frame size must be fixed)
enum {
  no_frame_size            = -1
};


// identify halves of a Java doubleword, independent of endianess
enum {
  first_java_word,
  second_java_word
};


# include "incls/_c1_Defs_pd.hpp.incl"


// mapping between Java and native doubleword halves;
// defined in terms of first_ and second_java_word above
enum {
  native_lo_word = pd_native_lo_word,
  native_hi_word = pd_native_hi_word
};


// native word offsets from memory address
enum {
  lo_word_offset_in_bytes = pd_lo_word_offset_in_bytes,
  hi_word_offset_in_bytes = pd_hi_word_offset_in_bytes
};


// the processor may require explicit rounding operations to implement the strictFP mode
enum {
  strict_fp_requires_explicit_rounding = pd_strict_fp_requires_explicit_rounding
};


// OopMap and RegisterMap construction
const int REG_COUNT = pd_REG_COUNT;
const int STACK0    = REG_COUNT;


// for debug info: a float value in a register may be saved in double precision by runtime stubs
enum {
  float_saved_as_double = pd_float_saved_as_double
};


// for deoptimization: know how to find the pc-desc
enum {
  jsr_call_offset = pd_jsr_call_offset
};


