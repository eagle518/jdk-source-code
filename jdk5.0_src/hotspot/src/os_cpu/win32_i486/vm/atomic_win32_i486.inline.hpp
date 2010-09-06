#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)atomic_win32_i486.inline.hpp	1.12 03/12/23 16:38:29 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

// The following alternative implementations are needed because
// Windows 95 doesn't support (some of) the corresponding Windows NT
// calls. Furthermore, these versions allow inlining in the caller.
// (More precisely: The documentation for InterlockedExchange says
// it is supported for Windows 95. However, when single-stepping
// through the assembly code we cannot step into the routine and
// when looking at the routine address we see only garbage code.
// Better safe then sorry!). Was bug 7/31/98 (gri).
//
// Performance note: On uniprocessors, the 'lock' prefixes are not
// necessary (and expensive). We should generate separate cases if
// this becomes a performance problem.

#pragma warning(disable: 4035) // Disables warnings reporting missing return statement

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
// VC++ doesn't like the lock prefix to be on a single line
// so we can't insert a label after the lock prefix.
// By emitting a lock prefix, we can define a label after it.
#define LOCK_IF_MP(mp) __asm cmp mp, 0  \
                       __asm je L0      \
                       __asm _emit 0xF0 \
                       __asm L0:

inline jint     Atomic::add    (jint     add_value, volatile jint*     dest) {
  bool mp = os::is_MP();
  __asm {
    mov edx, dest;
    mov eax, add_value;
    mov ecx, eax;
    LOCK_IF_MP(mp)
    xadd dword ptr [edx], eax;
    add eax, ecx;
  }
}

inline intptr_t Atomic::add_ptr(intptr_t add_value, volatile intptr_t* dest) {
  return (intptr_t)add((jint)add_value, (volatile jint*)dest);
}

inline void*    Atomic::add_ptr(intptr_t add_value, volatile void*     dest) {
  return (void*)add((jint)add_value, (volatile jint*)dest);
}


inline void Atomic::inc    (volatile jint*     dest) {
  // alternative for InterlockedIncrement
  bool mp = os::is_MP();
  __asm {
    mov edx, dest;
    LOCK_IF_MP(mp)
    inc dword ptr [edx];
  }
}

inline void Atomic::inc_ptr(volatile intptr_t* dest) {
  inc((volatile jint*)dest);
}

inline void Atomic::inc_ptr(volatile void*     dest) {
  inc((volatile jint*)dest);
}


inline void Atomic::dec    (volatile jint*     dest) {
  // alternative for InterlockedDecrement
  bool mp = os::is_MP();
  __asm {
    mov edx, dest;
    LOCK_IF_MP(mp)
    dec dword ptr [edx];
  }
}

inline void Atomic::dec_ptr(volatile intptr_t* dest) {
  dec((volatile jint*)dest);
}

inline void Atomic::dec_ptr(volatile void*     dest) {
  dec((volatile jint*)dest);
}


inline jint     Atomic::xchg    (jint     exchange_value, volatile jint*     dest) {
  // alternative for InterlockedExchange
  __asm {
    mov eax, exchange_value;
    mov ecx, dest;
    xchg eax, dword ptr [ecx];
  }
}

inline intptr_t Atomic::xchg_ptr(intptr_t exchange_value, volatile intptr_t* dest) {
  return (intptr_t)xchg((jint)exchange_value, (volatile jint*)dest);
}

inline void*    Atomic::xchg_ptr(void*    exchange_value, volatile void*     dest) {
  return (void*)xchg((jint)exchange_value, (volatile jint*)dest);
}


inline jint     Atomic::cmpxchg    (jint     exchange_value, volatile jint*     dest, jint     compare_value) {
  // alternative for InterlockedCompareExchange
  bool mp = os::is_MP();
  __asm {
    mov edx, dest
    mov ecx, exchange_value
    mov eax, compare_value
    LOCK_IF_MP(mp)
    cmpxchg dword ptr [edx], ecx   
  }
}

inline jlong    Atomic::cmpxchg    (jlong    exchange_value, volatile jlong*    dest, jlong    compare_value) {
  bool mp = os::is_MP();
  jint ex_lo  = (jint)exchange_value;
  jint ex_hi  = *( ((jint*)&exchange_value) + 1 );
  jint cmp_lo = (jint)compare_value;
  jint cmp_hi = *( ((jint*)&compare_value) + 1 );
  __asm {
    push ebx
    push edi
    mov eax, cmp_lo
    mov edx, cmp_hi
    mov edi, dest
    mov ebx, ex_lo
    mov ecx, ex_hi
    LOCK_IF_MP(mp)
    cmpxchg8b qword ptr [edi]
    pop edi
    pop ebx
  }
}

inline intptr_t Atomic::cmpxchg_ptr(intptr_t exchange_value, volatile intptr_t* dest, intptr_t compare_value) {
  return (intptr_t)cmpxchg((jint)exchange_value, (volatile jint*)dest, (jint)compare_value);
}

inline void*    Atomic::cmpxchg_ptr(void*    exchange_value, volatile void*     dest, void*    compare_value) {
  return (void*)cmpxchg((jint)exchange_value, (volatile jint*)dest, (jint)compare_value);
}

#pragma warning(default: 4035) // Enables warnings reporting missing return statement
