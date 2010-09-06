#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)atomic_linux_ia64.inline.hpp	1.9 03/12/23 16:38:11 JVM"
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


inline jint     Atomic::add    (jint     add_value, volatile jint*     dest) {
  jint sum;

  if (add_value == 1) {
    __asm__ volatile ("mf;;fetchadd4.acq %0=[%1],1;;" : "=r" (sum) : "r" (dest) : "memory");
    ++sum;
  } else if (add_value == -1) {
    __asm__ volatile ("mf;;fetchadd4.acq %0=[%1],-1;;" : "=r" (sum) : "r" (dest) : "memory");
    --sum;
  } else {
    jint compare_value;
    jint exchange_value;
    __asm__ volatile (  "mf;;0: ld4 %0=[%7];;add %1=%6,%3;;mov ar.ccv=%3;zxt4 %1=%4;;cmpxchg4.acq %1=[%7],%4,ar.ccv;;"
                        "cmp4.ne p15,p0=%3,%4;add %2=%6,%3;(p15) br.cond.spnt 0b;;sxt4 %2=%5;;"
                      : "=r" (compare_value), "=r" (exchange_value), "=r" (sum)
                      : "0" (compare_value), "1" (exchange_value), "2" (sum), "r" (add_value), "r" (dest)
                      : "p15", "memory");
  }
  return sum;
}

inline intptr_t Atomic::add_ptr(intptr_t add_value, volatile intptr_t* dest) {
  intptr_t sum;

  if (add_value == 1) {
    __asm__ volatile ("mf;;fetchadd8.acq %0=[%1],1;;" : "=r" (sum) : "r" (dest) : "memory");
    ++sum;
  } else if (add_value == -1) {
    __asm__ volatile ("mf;;fetchadd8.acq %0=[%1],-1;;" : "=r" (sum) : "r" (dest) : "memory");
    --sum;
  } else {
    intptr_t compare_value;
    intptr_t exchange_value;
    __asm__ volatile (  "mf;;0: ld8 %0=[%6];;add %1=%5,%3;mov ar.ccv=%3;;cmpxchg8.acq %1=[%6],%4,ar.ccv;;"
                        "cmp.ne p15,p0=%3,%4;add %2=%5,%3;(p15) br.cond.spnt 0b;;"
                      : "=r" (compare_value), "=r" (exchange_value), "=r" (sum)
                      : "0" (compare_value), "1" (exchange_value), "r" (add_value), "r" (dest)
                      : "p15", "memory");
  }
  return sum;
}

inline void*    Atomic::add_ptr(intptr_t add_value, volatile void*     dest) {
  return (void*)add_ptr(add_value, (volatile intptr_t*)dest);
}


inline void Atomic::inc    (volatile jint*     dest) {
  jint exchange_value;
  __asm__ volatile ("mf;;fetchadd4.acq %0=[%1],1;;" : "=r" (exchange_value) : "r" (dest) : "memory");
}

inline void Atomic::inc_ptr(volatile intptr_t* dest) {
  intptr_t exchange_value;
  __asm__ volatile ("mf;;fetchadd8.acq %0=[%1],1;;" : "=r" (exchange_value) : "r" (dest) : "memory");
}

inline void Atomic::inc_ptr(volatile void*     dest) {
  inc_ptr((volatile intptr_t*)dest);
}


inline void Atomic::dec    (volatile jint*     dest) {
  jint exchange_value;
  __asm__ volatile ("mf;;fetchadd4.acq %0=[%1],-1;;" : "=r" (exchange_value) : "r" (dest) : "memory");
}

inline void Atomic::dec_ptr(volatile intptr_t* dest) {
  intptr_t exchange_value;
  __asm__ volatile ("mf;;fetchadd8.acq %0=[%1],-1;;" : "=r" (exchange_value) : "r" (dest) : "memory");
}

inline void Atomic::dec_ptr(volatile void*     dest) {
  dec_ptr((volatile intptr_t*)dest);
}


inline jint     Atomic::xchg    (jint     exchange_value, volatile jint*     dest) {
  __asm__ volatile (  "mf;;xchg4 %0=[%2],%1;;sxt4 %0=%1;;"
                    : "=r" (exchange_value)
                    : "0" (exchange_value), "r" (dest)
                    : "memory");
  return exchange_value;
}

inline intptr_t Atomic::xchg_ptr(intptr_t exchange_value, volatile intptr_t* dest) {
  __asm__ volatile (  "mf;;xchg8 %0=[%2],%1;;"
                    : "=r" (exchange_value)
                    : "0" (exchange_value), "r" (dest)
                    : "memory");
  return exchange_value;
}

inline void*    Atomic::xchg_ptr(void*    exchange_value, volatile void*     dest) {
  return (void*)xchg_ptr((intptr_t)exchange_value, (volatile intptr_t*)dest);
}


inline jint     Atomic::cmpxchg    (jint     exchange_value, volatile jint*     dest, jint     compare_value) {
  __asm__ volatile (  "mf;;zxt4 %1=%3;;mov ar.ccv=%3;;cmpxchg4.acq %0=[%4],%2,ar.ccv;;sxt4 %0=%2;;"
		    : "=r" (exchange_value), "=r" (compare_value)
		    : "0" (exchange_value), "1" (compare_value), "r" (dest)
                    : "memory");
  return exchange_value;
}

inline jlong    Atomic::cmpxchg    (jlong    exchange_value, volatile jlong*    dest, jlong    compare_value) {
  __asm__ volatile (  "mf;;mov ar.ccv=%2;;cmpxchg8.acq %0=[%3],%1,ar.ccv;;"
		    : "=r" (exchange_value)
		    : "0" (exchange_value), "r" (compare_value), "r" (dest)
                    : "memory");
  return exchange_value;
}

inline intptr_t Atomic::cmpxchg_ptr(intptr_t exchange_value, volatile intptr_t* dest, intptr_t compare_value) {
  return (intptr_t)cmpxchg((jlong)exchange_value, (volatile jlong*)dest, (jlong)compare_value);
}

inline void*    Atomic::cmpxchg_ptr(void*    exchange_value, volatile void*     dest, void*    compare_value) {
  return (void*)cmpxchg((jlong)exchange_value, (volatile jlong*)dest, (jlong)compare_value);
}
