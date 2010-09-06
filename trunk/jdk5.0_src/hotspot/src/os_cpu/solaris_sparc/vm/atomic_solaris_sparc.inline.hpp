#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)atomic_solaris_sparc.inline.hpp	1.15 03/12/23 16:38:20 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

// Implementation of class atomic

inline void Atomic::store    (jbyte    store_value, jbyte*    dest) { *dest = store_value; }
inline void Atomic::store    (jshort   store_value, jshort*   dest) { *dest = store_value; }
inline void Atomic::store    (jint     store_value, jint*     dest) { *dest = store_value; }
inline void Atomic::store    (jlong    store_value, jlong*    dest) { *dest = store_value; }
inline void Atomic::store_ptr(intptr_t store_value, intptr_t* dest) { *dest = store_value; }
inline void Atomic::store_ptr(void*    store_value, void*     dest) { *(void**)dest = store_value; }

inline void Atomic::store    (jbyte    store_value, volatile jbyte*    dest) { *dest = store_value; }
inline void Atomic::store    (jshort   store_value, volatile jshort*   dest) { *dest = store_value; }
inline void Atomic::store    (jint     store_value, volatile jint*     dest) { *dest = store_value; }
inline void Atomic::store    (jlong    store_value, volatile jlong*    dest) { *dest = store_value; }
inline void Atomic::store_ptr(intptr_t store_value, volatile intptr_t* dest) { *dest = store_value; }
inline void Atomic::store_ptr(void*    store_value, volatile void*     dest) { *(void* volatile *)dest = store_value; }

inline void Atomic::inc    (volatile jint*     dest) { (void)add    (1, dest); }
inline void Atomic::inc_ptr(volatile intptr_t* dest) { (void)add_ptr(1, dest); }
inline void Atomic::inc_ptr(volatile void*     dest) { (void)add_ptr(1, dest); }

inline void Atomic::dec    (volatile jint*     dest) { (void)add    (-1, dest); }
inline void Atomic::dec_ptr(volatile intptr_t* dest) { (void)add_ptr(-1, dest); }
inline void Atomic::dec_ptr(volatile void*     dest) { (void)add_ptr(-1, dest); }


#if defined(COMPILER2) || defined(_LP64)

// This is the interface to the atomic instructions in solaris_sparc.il.
// It's very messy because we need to support v8 and these instructions
// are illegal there.  When sparc v8 is dropped, we can drop out lots of
// this code.  Also compiler2 does not support v8 so the conditional code
// omits the instruction set check.

extern "C" jint     _Atomic_swap32(jint     exchange_value, volatile jint*     dest);
extern "C" intptr_t _Atomic_swap64(intptr_t exchange_value, volatile intptr_t* dest);

extern "C" jint     _Atomic_cas32(jint     exchange_value, volatile jint*     dest, jint     compare_value);
extern "C" intptr_t _Atomic_cas64(intptr_t exchange_value, volatile intptr_t* dest, intptr_t compare_value);
extern "C" jlong    _Atomic_casl (jlong    exchange_value, volatile jlong*    dest, jlong    compare_value);

extern "C" jint     _Atomic_add32(jint     inc,       volatile jint*     dest);
extern "C" intptr_t _Atomic_add64(intptr_t add_value, volatile intptr_t* dest);


inline jint     Atomic::add     (jint    add_value, volatile jint*     dest) {
  return _Atomic_add32(add_value, dest);
}

inline intptr_t Atomic::add_ptr(intptr_t add_value, volatile intptr_t* dest) {
#ifdef _LP64
  return _Atomic_add64(add_value, dest);
#else
  return _Atomic_add32(add_value, dest);
#endif // _LP64
}

inline void*    Atomic::add_ptr(intptr_t add_value, volatile void*     dest) {
  return (void*)add_ptr((intptr_t)add_value, (volatile intptr_t*)dest);
}


inline jint     Atomic::xchg    (jint     exchange_value, volatile jint*     dest) {
  return _Atomic_swap32(exchange_value, dest);
}

inline intptr_t Atomic::xchg_ptr(intptr_t exchange_value, volatile intptr_t* dest) {
#ifdef _LP64
  return _Atomic_swap64(exchange_value, dest);
#else
  return _Atomic_swap32(exchange_value, dest);
#endif // _LP64
}

inline void*    Atomic::xchg_ptr(void*    exchange_value, volatile void*     dest) {
  return (void*)xchg_ptr((intptr_t)exchange_value, (volatile intptr_t*)dest);
}


inline jint     Atomic::cmpxchg    (jint     exchange_value, volatile jint*     dest, jint     compare_value) {
  return _Atomic_cas32(exchange_value, dest, compare_value);
}

inline jlong    Atomic::cmpxchg    (jlong    exchange_value, volatile jlong*    dest, jlong    compare_value) {
#ifdef _LP64
  // Return 64 bit value in %o0
  return _Atomic_cas64((intptr_t)exchange_value, (intptr_t *)dest, (intptr_t)compare_value);
#else
  assert (VM_Version::v9_instructions_work(), "only supported on v9");
  // Return 64 bit value in %o0,%o1 by hand
  return _Atomic_casl(exchange_value, dest, compare_value);
#endif
}

inline intptr_t Atomic::cmpxchg_ptr(intptr_t exchange_value, volatile intptr_t* dest, intptr_t compare_value) {
#ifdef _LP64
  return _Atomic_cas64(exchange_value, dest, compare_value);
#else
  return _Atomic_cas32(exchange_value, dest, compare_value);
#endif // _LP64
}

inline void*    Atomic::cmpxchg_ptr(void*    exchange_value, volatile void*     dest, void*    compare_value) {
  return (void*)cmpxchg_ptr((intptr_t)exchange_value, (volatile intptr_t*)dest, (intptr_t)compare_value);
}


#else // _LP64 || COMPILER2


// 32-bit compiler1 only

inline jint     Atomic::add    (jint     add_value, volatile jint*     dest) {
  return (*os::atomic_add_func)(add_value, dest);
}

inline intptr_t Atomic::add_ptr(intptr_t add_value, volatile intptr_t* dest) {
  return (intptr_t)add((jint)add_value, (volatile jint*)dest);
}

inline void*    Atomic::add_ptr(intptr_t add_value, volatile void*     dest) {
  return (void*)add((jint)add_value, (volatile jint*)dest);
}


inline jint     Atomic::xchg    (jint     exchange_value, volatile jint*     dest) {
  return (*os::atomic_xchg_func)(exchange_value, dest);
}

inline intptr_t Atomic::xchg_ptr(intptr_t exchange_value, volatile intptr_t* dest) {
  return (intptr_t)xchg((jint)exchange_value, (volatile jint*)dest);
}

inline void*    Atomic::xchg_ptr(void*    exchange_value, volatile void*     dest) {
  return (void*)xchg((jint)exchange_value, (volatile jint*)dest);
}


inline jint     Atomic::cmpxchg    (jint     exchange_value, volatile jint*     dest, jint     compare_value) {
  return (*os::atomic_cmpxchg_func)(exchange_value, dest, compare_value);
}

inline jlong    Atomic::cmpxchg    (jlong    exchange_value, volatile jlong*    dest, jlong    compare_value) {
  return (*os::atomic_cmpxchg_long_func)(exchange_value, dest, compare_value);
}

inline intptr_t Atomic::cmpxchg_ptr(intptr_t exchange_value, volatile intptr_t* dest, intptr_t compare_value) {
  return (intptr_t)cmpxchg((jint)exchange_value, (volatile jint*)dest, (jint)compare_value);
}

inline void*    Atomic::cmpxchg_ptr(void*    exchange_value, volatile void*     dest, void*    compare_value) {
  return (void*)cmpxchg((jint)exchange_value, (volatile jint*)dest, (jint)compare_value);
}

#endif // _LP64 || COMPILER2
