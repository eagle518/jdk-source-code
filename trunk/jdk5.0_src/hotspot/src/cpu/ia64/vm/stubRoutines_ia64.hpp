#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)stubRoutines_ia64.hpp	1.11 04/01/08 01:02:11 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// This file holds the platform specific parts of the StubRoutines
// definition. See stubRoutines.hpp for a description on how to
// extend it.

enum platform_dependent_constants {
  code_size1 =  9000,		// simply increase if too small (assembler will crash if too small)
  code_size2 = 22000		// simply increase if too small (assembler will crash if too small)
};

class ia64 {
 friend class StubGenerator;

 private:
  static address _flush_register_stack_entry;
  static address _handler_for_divide_by_zero_entry;
  static address _handler_for_null_exception_entry;
  static address _handler_for_stack_overflow_entry;  
  static address _get_previous_fp_entry;  
  static address _acquire_entry; // Windows only
  static address _partial_subtype_check;
  static address _get_backing_store_pointer;
  static address _ldffill_entry;

  typedef void (*flush_register_stack_func)(void); // This should be extern C but is harmless as is
  typedef address get_backing_store_pointer_func(void);
  typedef jdouble (*ldffill_func)(address fresult);

 public:
  static flush_register_stack_func flush_register_stack() {
    return CAST_TO_FN_PTR(flush_register_stack_func, _flush_register_stack_entry);
  }

  static ldffill_func ldffill() {
    return CAST_TO_FN_PTR(ldffill_func, _ldffill_entry);
  }

  // Call back points for traps in compiled code
  static address handler_for_divide_by_zero() { return _handler_for_divide_by_zero_entry; }
  static address handler_for_null_exception() { return _handler_for_null_exception_entry; }
  static address handler_for_stack_overflow() { return _handler_for_stack_overflow_entry; }
  static address get_previous_fp_entry()      { return _get_previous_fp_entry; }
  static address acquire_entry()              { return _acquire_entry; } // Windows only
  static address partial_subtype_check()      { return _partial_subtype_check; }

  static address get_backing_store_pointer() {
    return (*CAST_TO_FN_PTR(get_backing_store_pointer_func*, _get_backing_store_pointer))(); 
  }
};

