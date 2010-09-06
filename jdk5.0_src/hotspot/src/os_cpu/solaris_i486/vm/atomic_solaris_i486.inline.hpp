#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)atomic_solaris_i486.inline.hpp	1.16 03/12/23 16:38:15 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

// Implementation of class atomic

inline void Atomic::store    (jbyte    store_value, jbyte*    dest) { *dest = store_value; }
inline void Atomic::store    (jshort   store_value, jshort*   dest) { *dest = store_value; }
inline void Atomic::store    (jint     store_value, jint*     dest) { *dest = store_value; }
//inline void Atomic::store    (jlong    store_value, jlong*    dest) { *dest = store_value; }
inline void Atomic::store_ptr(intptr_t store_value, intptr_t* dest) { *dest = store_value; }
inline void Atomic::store_ptr(void*    store_value, void*     dest) { *(void**)dest = store_value; }

inline void Atomic::store    (jbyte    store_value, volatile jbyte*    dest) { *dest = store_value; }
inline void Atomic::store    (jshort   store_value, volatile jshort*   dest) { *dest = store_value; }
inline void Atomic::store    (jint     store_value, volatile jint*     dest) { *dest = store_value; }
//inline void Atomic::store    (jlong    store_value, volatile jlong*    dest) { *dest = store_value; }
inline void Atomic::store_ptr(intptr_t store_value, volatile intptr_t* dest) { *dest = store_value; }
inline void Atomic::store_ptr(void*    store_value, volatile void*     dest) { *(void* volatile *)dest = store_value; }

inline void Atomic::inc    (volatile jint*     dest) { (void)add    (1, dest); }
inline void Atomic::inc_ptr(volatile intptr_t* dest) { (void)add_ptr(1, dest); }
inline void Atomic::inc_ptr(volatile void*     dest) { (void)add_ptr(1, dest); }

inline void Atomic::dec    (volatile jint*     dest) { (void)add    (-1, dest); }
inline void Atomic::dec_ptr(volatile intptr_t* dest) { (void)add_ptr(-1, dest); }
inline void Atomic::dec_ptr(volatile void*     dest) { (void)add_ptr(-1, dest); }

// This is the interface to the atomic instructions in solaris_i486.il.
extern "C" jint _Atomic_add(jint add_value, volatile jint* dest, bool mp);
extern "C" jint _Atomic_xchg(jint exchange_value, volatile jint* dest);
extern "C" jint _Atomic_cmpxchg(jint exchange_value, volatile jint* dest, jint compare_value, bool mp);
extern "C" jlong _Atomic_cmpxchg_long(jlong exchange_value, volatile jlong* dest, jlong compare_value, bool mp);


inline jint     Atomic::add    (jint     add_value, volatile jint*     dest) {
  return _Atomic_add(add_value, dest, os::is_MP());
}

inline intptr_t Atomic::add_ptr(intptr_t add_value, volatile intptr_t* dest) {
  return (intptr_t)add((jint)add_value, (volatile jint*)dest);
}

inline void*    Atomic::add_ptr(intptr_t add_value, volatile void*     dest) {
  return (void*)add((jint)add_value, (volatile jint*)dest);
}


inline jint     Atomic::xchg    (jint     exchange_value, volatile jint*     dest) {
  // We noticed a CC5.5 bug (4894807), so keep calling the stub just to be safe.
  // Will use the inline template version after 4894807 is fixed.
  // return _Atomic_xchg(exchange_value, dest);
  return (*os::atomic_xchg_func)(exchange_value, dest);
}

inline intptr_t Atomic::xchg_ptr(intptr_t exchange_value, volatile intptr_t* dest) {
  return (intptr_t)xchg((jint)exchange_value, (volatile jint*)dest);
}

inline void*    Atomic::xchg_ptr(void*    exchange_value, volatile void*     dest) {
  return (void*)xchg((jint)exchange_value, (volatile jint*)dest);
}


inline jint     Atomic::cmpxchg    (jint     exchange_value, volatile jint*     dest, jint     compare_value) {
  return _Atomic_cmpxchg(exchange_value, dest, compare_value, os::is_MP());
}

inline jlong    Atomic::cmpxchg    (jlong    exchange_value, volatile jlong*    dest, jlong    compare_value) {
  return _Atomic_cmpxchg_long(exchange_value, dest, compare_value, os::is_MP());
}

inline intptr_t Atomic::cmpxchg_ptr(intptr_t exchange_value, volatile intptr_t* dest, intptr_t compare_value) {
  return (intptr_t)cmpxchg((jint)exchange_value, (volatile jint*)dest, (jint)compare_value);
}

inline void*    Atomic::cmpxchg_ptr(void*    exchange_value, volatile void*     dest, void*    compare_value) {
  return (void*)cmpxchg((jint)exchange_value, (volatile jint*)dest, (jint)compare_value);
}
