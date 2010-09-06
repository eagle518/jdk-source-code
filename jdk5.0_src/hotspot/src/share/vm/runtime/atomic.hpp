#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)atomic.hpp	1.16 03/12/23 16:43:31 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

class Atomic : AllStatic {
 public:
  // Atomically store to a location
  static void store    (jbyte    store_value, jbyte*    dest);
  static void store    (jshort   store_value, jshort*   dest);
  static void store    (jint     store_value, jint*     dest);
  static void store    (jlong    store_value, jlong*    dest);
  static void store_ptr(intptr_t store_value, intptr_t* dest);
  static void store_ptr(void*    store_value, void*     dest);

  static void store    (jbyte    store_value, volatile jbyte*    dest);
  static void store    (jshort   store_value, volatile jshort*   dest);
  static void store    (jint     store_value, volatile jint*     dest);
  static void store    (jlong    store_value, volatile jlong*    dest);
  static void store_ptr(intptr_t store_value, volatile intptr_t* dest);
  static void store_ptr(void*    store_value, volatile void*     dest);

  // Atomically add to a location, return updated value
  static jint     add    (jint     add_value, volatile jint*     dest);
  static intptr_t add_ptr(intptr_t add_value, volatile intptr_t* dest);
  static void*    add_ptr(intptr_t add_value, volatile void*     dest);

  // Atomically increment location
  static void inc    (volatile jint*     dest);
  static void inc_ptr(volatile intptr_t* dest);
  static void inc_ptr(volatile void*     dest);

  // Atomically decrement a location
  static void dec    (volatile jint*     dest);
  static void dec_ptr(volatile intptr_t* dest);
  static void dec_ptr(volatile void*     dest);

  // Performs atomic exchange of *dest with exchange_value.  Returns old prior value of *dest.
  static jint     xchg    (jint     exchange_value, volatile jint*     dest);
  static intptr_t xchg_ptr(intptr_t exchange_value, volatile intptr_t* dest);
  static void*    xchg_ptr(void*    exchange_value, volatile void*   dest);

  // Performs atomic compare of *dest and compare_value, and exchanges *dest with exchange_value
  // if the comparison succeeded.  Returns prior value of *dest.  Guarantees a two-way memory
  // barrier across the cmpxchg.  I.e., it's really a 'fence_cmpxchg_acquire'.
  static jbyte    cmpxchg    (jbyte    exchange_value, volatile jbyte*    dest, jbyte    compare_value);
  static jint     cmpxchg    (jint     exchange_value, volatile jint*     dest, jint     compare_value);
  static jlong    cmpxchg    (jlong    exchange_value, volatile jlong*    dest, jlong    compare_value);
  static intptr_t cmpxchg_ptr(intptr_t exchange_value, volatile intptr_t* dest, intptr_t compare_value);
  static void*    cmpxchg_ptr(void*    exchange_value, volatile void*     dest, void*    compare_value);
};
