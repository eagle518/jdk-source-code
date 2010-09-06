#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_MacroAssembler_sparc.hpp	1.18 03/12/23 16:37:04 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

  void pd_init() { /* nothing to do */ }

 public:
   void try_allocate(
    Register obj,                      // result: pointer to object after successful allocation
    Register var_size_in_bytes,        // object size in bytes if unknown at compile time; invalid otherwise
    int      con_size_in_bytes,        // object size in bytes if   known at compile time
    Register t1,                       // temp register
    Register t2,                       // temp register
    Label&   slow_case                 // continuation point if fast allocation fails
  );

  void initialize_header(Register obj, Register klass, Register len, Register tmp);
  void initialize_body(Register base, Register index);

  // locking/unlocking
  void lock_object  (Register Rmark, Register Roop, Register Rbox, Register Rscratch, Label& slow_case);
  void unlock_object(Register Rmark, Register Roop, Register Rbox,                    Label& slow_case);

  void initialize_object(
    Register obj,                      // result: pointer to object after successful allocation
    Register klass,                    // object klass
    Register var_size_in_bytes,        // object size in bytes if unknown at compile time; invalid otherwise
    int      con_size_in_bytes,        // object size in bytes if   known at compile time
    Register t1,                       // temp register
    Register t2                        // temp register
  );

  // allocation of fixed-size objects
  // (can also be used to allocate fixed-size arrays, by setting
  // hdr_size correctly and storing the array length afterwards)
  void allocate_object(
    Register obj,                      // result: pointer to object after successful allocation
    Register t1,                       // temp register
    Register t2,                       // temp register
    Register t3,                       // temp register
    int      hdr_size,                 // object header size in words
    int      obj_size,                 // object size in words 
    Register klass,                    // object klass
    Label&   slow_case                 // continuation point if fast allocation fails
  );

  // allocation of arrays
  void allocate_array(
    Register obj,                      // result: pointer to array after successful allocation
    Register len,                      // array length
    Register t1,                       // temp register
    Register t2,                       // temp register
    Register t3,                       // temp register
    int      hdr_size,                 // object header size in words
    int      elt_size,                 // element size in bytes 
    Register klass,                    // object klass
    Label&   slow_case                 // continuation point if fast allocation fails
  );

