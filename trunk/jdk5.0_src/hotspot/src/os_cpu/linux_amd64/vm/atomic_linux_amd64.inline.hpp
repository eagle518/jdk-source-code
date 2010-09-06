#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)atomic_linux_amd64.inline.hpp	1.2 03/12/23 16:38:01 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Implementation of class atomic

inline void Atomic::store(jbyte store_value, jbyte*dest)
{
  *dest = store_value;
}

inline void Atomic::store(jshort store_value, jshort* dest)
{
  *dest = store_value;
}

inline void Atomic::store(jint store_value, jint* dest)
{
  *dest = store_value;
}

inline void Atomic::store(jlong store_value, jlong* dest)
{
  *dest = store_value;
}

inline void Atomic::store_ptr(intptr_t store_value, intptr_t* dest)
{
  *dest = store_value;
}

inline void Atomic::store_ptr(void* store_value, void* dest)
{
  *(void**) dest = store_value;
}

inline void Atomic::store(jbyte store_value, volatile jbyte* dest)
{
  *dest = store_value;
}

inline void Atomic::store(jshort store_value, volatile jshort* dest)
{
  *dest = store_value;
}

inline void Atomic::store(jint store_value, volatile jint* dest)
{
  *dest = store_value;
}

inline void Atomic::store(jlong store_value, volatile jlong* dest)
{
  *dest = store_value;
}

inline void Atomic::store_ptr(intptr_t store_value, volatile intptr_t* dest)
{
  *dest = store_value;
}

inline void Atomic::store_ptr(void* store_value, volatile void* dest)
{
  *(void* volatile *) dest = store_value;
}

// XXX remove LOCKs on non-mp
// Adding a lock prefix to an instruction on MP machine
// #define LOCK_IF_MP(mp) "cmp $0, " #mp "; je 1f; lock; 1: "

inline jint Atomic::add(jint add_value, volatile jint* dest)
{
  jint addend = add_value;
  __asm__ __volatile__ ("lock ; xaddl %0, (%2)"
                        : "=r" (addend)
                        : "0" (addend), "r" (dest)
                        : "memory");
  return addend + add_value;
}

inline intptr_t Atomic::add_ptr(intptr_t add_value, volatile intptr_t* dest)
{
  intptr_t addend = add_value;
  __asm__ __volatile__ ("lock ; xaddq %0,(%2)"
                        : "=r" (addend)
                        : "0" (addend), "r" (dest)
                        : "memory");
  return addend + add_value;
}

inline void* Atomic::add_ptr(intptr_t add_value, volatile void* dest)
{
  intptr_t addend = add_value;
  __asm__ __volatile__ ("lock ; xaddq %0,(%2)"
                        : "=r" (addend)
                        : "0" (addend), "r" (dest)
                        : "memory");
  return (void*) (addend + add_value);
}

inline void Atomic::inc(volatile jint* dest)
{
  __asm__ __volatile__ ("lock ; incl (%0)" : : "r" (dest) : "memory");
}

inline void Atomic::inc_ptr(volatile intptr_t* dest) 
{
  __asm__ __volatile__ ("lock ; incq (%0)" : : "r" (dest) : "memory");
}

inline void Atomic::inc_ptr(volatile void* dest) 
{
  __asm__ __volatile__ ("lock ; incq (%0)" : : "r" (dest) : "memory");
}

inline void Atomic::dec(volatile jint* dest)
{
  __asm__ __volatile__ ("lock ; decl (%0)" : : "r" (dest) : "memory");
}

inline void Atomic::dec_ptr(volatile intptr_t* dest)
{
  __asm__ __volatile__ ("lock ; decq (%0)" : : "r" (dest) : "memory");
}

inline void Atomic::dec_ptr(volatile void* dest)
{
  __asm__ __volatile__ ("lock ; decq (%0)" : : "r" (dest) : "memory");
}


inline jint Atomic::xchg(jint exchange_value, volatile jint* dest)
{
  __asm__ __volatile__ ("xchgl (%2),%0"
                        : "=r" (exchange_value)
                        : "0" (exchange_value), "r" (dest)
                        : "memory");
  return exchange_value;
}

inline intptr_t Atomic::xchg_ptr(intptr_t exchange_value, 
                                 volatile intptr_t* dest)
{
  __asm__ __volatile__ ("xchgq (%2),%0"
                        : "=r" (exchange_value)
                        : "0" (exchange_value), "r" (dest)
                        : "memory");
  return exchange_value;
}

inline void* Atomic::xchg_ptr(void* exchange_value, volatile void* dest)
{
  __asm__ __volatile__ ("xchgq (%2),%0"
                        : "=r" (exchange_value)
                        : "0" (exchange_value), "r" (dest)
                        : "memory");
  return exchange_value;
}

inline jint Atomic::cmpxchg(jint exchange_value,
                            volatile jint* dest,
                            jint compare_value)
{
  __asm__ __volatile__ ("lock ; cmpxchgl %1,(%3)"
                        : "=a" (exchange_value)
                        : "r" (exchange_value), 
                          "a" (compare_value),
                          "r" (dest)
                        : "memory");
  return exchange_value;
}

inline jlong Atomic::cmpxchg(jlong exchange_value,
                             volatile jlong* dest,
                             jlong compare_value)
{
  __asm__ __volatile__ ("lock ; cmpxchgq %1,(%3)"
                        : "=a" (exchange_value)
                        : "r" (exchange_value), 
                          "a" (compare_value),
                          "r" (dest)
                        : "memory");
  return exchange_value;
}

inline intptr_t Atomic::cmpxchg_ptr(intptr_t exchange_value,
                                    volatile intptr_t* dest,
                                    intptr_t compare_value)
{
  __asm__ __volatile__ ("lock ; cmpxchgq %1,(%3)"
                        : "=a" (exchange_value)
                        : "r" (exchange_value), 
                          "a" (compare_value),
                          "r" (dest)
                        : "memory");
  return exchange_value;
}

inline void* Atomic::cmpxchg_ptr(void* exchange_value,
                                 volatile void* dest,
                                 void* compare_value)
{
  __asm__ __volatile__ ("lock ; cmpxchgq %1,(%3)"
                        : "=a" (exchange_value)
                        : "r" (exchange_value), 
                          "a" (compare_value),
                          "r" (dest)
                        : "memory");
  return exchange_value;
}
