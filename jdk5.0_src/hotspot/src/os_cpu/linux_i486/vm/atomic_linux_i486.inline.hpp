#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)atomic_linux_i486.inline.hpp	1.20 03/12/23 16:38:06 JVM"
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


// Adding a lock prefix to an instruction on MP machine
#define LOCK_IF_MP(mp) "cmp $0, " #mp "; je 1f; lock; 1: "

inline jint     Atomic::add    (jint     add_value, volatile jint*     dest) {
  jint addend = add_value;
  bool mp = os::is_MP();
  __asm__ volatile (  LOCK_IF_MP(%3) "xaddl %0,(%2)"
		    : "=r" (addend)
		    : "0" (addend), "r" (dest), "r" (mp)
		    : "cc", "memory");
  return addend + add_value;
}

inline intptr_t Atomic::add_ptr(intptr_t add_value, volatile intptr_t* dest) {
  return (intptr_t)Atomic::add((jint)add_value, (volatile jint*)dest);
}

inline void*    Atomic::add_ptr(intptr_t add_value, volatile void*     dest) {
  return (void*)Atomic::add((jint)add_value, (volatile jint*)dest);
}


inline void Atomic::inc    (volatile jint*     dest) {
  bool mp = os::is_MP();
  __asm__ volatile (LOCK_IF_MP(%1) "incl (%0)" :
                    : "r" (dest), "r" (mp) : "cc", "memory");
}

inline void Atomic::inc_ptr(volatile intptr_t* dest) {
  inc((volatile jint*)dest);
}

inline void Atomic::inc_ptr(volatile void*     dest) {
  inc((volatile jint*)dest);
}


inline void Atomic::dec    (volatile jint*     dest) {
  bool mp = os::is_MP();
  __asm__ volatile (LOCK_IF_MP(%1) "decl (%0)" :
                    : "r" (dest), "r" (mp) : "cc", "memory");
}

inline void Atomic::dec_ptr(volatile intptr_t* dest) {
  dec((volatile jint*)dest);
}

inline void Atomic::dec_ptr(volatile void*     dest) {
  dec((volatile jint*)dest);
}


inline jint     Atomic::xchg    (jint     exchange_value, volatile jint*     dest) {
  __asm__ volatile (  "xchgl (%2),%0"
		    : "=r" (exchange_value)
		    : "0" (exchange_value), "r" (dest)
		    : "memory");
  return exchange_value;
}

inline intptr_t Atomic::xchg_ptr(intptr_t exchange_value, volatile intptr_t* dest) {
  return (intptr_t)xchg((jint)exchange_value, (volatile jint*)dest);
}

inline void*    Atomic::xchg_ptr(void*    exchange_value, volatile void*     dest) {
  return (void*)xchg((jint)exchange_value, (volatile jint*)dest);
}


inline jint     Atomic::cmpxchg    (jint     exchange_value, volatile jint*     dest, jint     compare_value) {
  bool mp = os::is_MP();
  __asm__ volatile (LOCK_IF_MP(%4) "cmpxchgl %1,(%3)"
                    : "=a" (exchange_value)
                    : "r" (exchange_value), "a" (compare_value), "r" (dest), "r" (mp)
                    : "cc", "memory");
  return exchange_value;
}

inline jlong    Atomic::cmpxchg    (jlong    exchange_value, volatile jlong*    dest, jlong    compare_value) {
  bool mp = os::is_MP();
  jlong old_value;
  jint lo = (jint)exchange_value;
  jint hi = (jint)(exchange_value >> 32);
  __asm__ volatile ("xchgl %%ebx, %1;"
                    LOCK_IF_MP(%6)
                    "cmpxchg8b (%5); xchgl %%ebx, %1"
                    : "=A" (old_value), "=r" (lo)
                    : "1" (lo), "c" (hi), "A" (compare_value), "r" (dest), "m" (mp)
                    : "%ebx", "cc", "memory");
  return old_value;
}

inline intptr_t Atomic::cmpxchg_ptr(intptr_t exchange_value, volatile intptr_t* dest, intptr_t compare_value) {
  return (intptr_t)cmpxchg((jint)exchange_value, (volatile jint*)dest, (jint)compare_value);
}

inline void*    Atomic::cmpxchg_ptr(void*    exchange_value, volatile void*     dest, void*    compare_value) {
  return (void*)cmpxchg((jint)exchange_value, (volatile jint*)dest, (jint)compare_value);
}
