#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)stubRoutines_sparc.hpp	1.60 03/12/23 16:37:24 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// This file holds the platform specific parts of the StubRoutines
// definition. See stubRoutines.hpp for a description on how to
// extend it.

enum /* platform_dependent_constants */ {
  // %%%%%%%% May be able to shrink this a lot 
  code_size1 = 20000,                                        // simply increase if too small (assembler will crash if too small)
  code_size2 = 20000                                         // simply increase if too small (assembler will crash if too small)
};

#undef sparc
class sparc {
 friend class StubGenerator;

 public:
  enum { nof_instance_allocators = 10 };

  // allocator lock values
  enum {
    unlocked = 0,
    locked   = 1
  };

  enum { 
    v8_oop_lock_ignore_bits = 2, 
    v8_oop_lock_bits = 4, 
    nof_v8_oop_lock_cache_entries = 1 << (v8_oop_lock_bits+v8_oop_lock_ignore_bits), 
    v8_oop_lock_mask = right_n_bits(v8_oop_lock_bits), 
    v8_oop_lock_mask_in_place = v8_oop_lock_mask << v8_oop_lock_ignore_bits
  };

  static int _v8_oop_lock_cache[nof_v8_oop_lock_cache_entries];

 private:
  static address _test_stop_entry;
  static address _stop_subroutine_entry;
  static address _flush_callers_register_windows_entry;

  static address _copy_words_aligned8_lower_entry;
  static address _copy_words_aligned8_higher_entry;
  static address _set_words_aligned8_entry;
  static address _zero_words_aligned8_entry;

  static address _handler_for_unsafe_access_entry;

  static int _atomic_memory_operation_lock;

  static address _partial_subtype_check;

 public:
  // %%% global lock for everyone who needs to use atomic_compare_and_exchange
  // %%% or atomic_increment -- should probably use more locks for more 
  // %%% scalability-- for instance one for each eden space or group of

  // address of the lock for atomic_compare_and_exchange
  static int* atomic_memory_operation_lock_addr() { return &_atomic_memory_operation_lock; }

  // accessor and mutator for _atomic_memory_operation_lock
  static int atomic_memory_operation_lock() { return _atomic_memory_operation_lock; }
  static void set_atomic_memory_operation_lock(int value) { _atomic_memory_operation_lock = value; }

  // test assembler stop routine by setting registers
  static void (*test_stop_entry()) ()                     { return CAST_TO_FN_PTR(void (*)(void), _test_stop_entry); }

  // a subroutine for debugging assembler code
  static address stop_subroutine_entry_address()          { return (address)&_stop_subroutine_entry; }

  // flushes (all but current) register window
  static intptr_t* (*flush_callers_register_windows_func())() { return CAST_TO_FN_PTR(intptr_t* (*)(void), _flush_callers_register_windows_entry); }

  // fast copies
  static address copy_words_aligned8_lower_entry()        { return _copy_words_aligned8_lower_entry; }
  static address copy_words_aligned8_higher_entry()       { return _copy_words_aligned8_higher_entry; }
  static address set_words_aligned8_entry()               { return _set_words_aligned8_entry; }
  static address zero_words_aligned8_entry()              { return _zero_words_aligned8_entry; }

  static address handler_for_unsafe_access_entry()        { return _handler_for_unsafe_access_entry; }

  static address partial_subtype_check()                  { return _partial_subtype_check; }
};
