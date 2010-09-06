// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// @(#)solaris_sparc.il	1.14 04/07/29 16:36:15
// 

  // Get the raw thread ID from %g7

       .inline  _raw_thread_id, 0
       .register  %g7, #scratch
       mov     %g7, %o0
       .end


  // Support for jint Atomic::xchg(jint exchange_value, volatile jint* dest).
  //
  // Arguments:
  //      exchange_value: O0
  //      dest:           O1
  //
  // Results:
  //     O0: the value previously stored in dest

        .inline _Atomic_swap32, 2
        swap    [%o1],%o0
        .end


  // Support for intptr_t Atomic::xchg_ptr(intptr_t exchange_value, volatile intptr_t * dest).
  //
  // 64-bit
  //
  // Arguments:
  //      exchange_value: O0
  //      dest:           O1
  //
  // Results:
  //     O0: the value previously stored in dest

        .inline _Atomic_swap64, 2
    1:
        mov     %o0, %o3
        ldx     [%o1], %o2
        casx    [%o1], %o2, %o3
        cmp     %o2, %o3
        bne     %xcc, 1b
         nop
        mov     %o2, %o0
        .end


  // Support for jint Atomic::cmpxchg(jint           exchange_value,
  //                                  volatile jint* dest, 
  //                                  jint           compare_value)
  //
  // Arguments:
  //      exchange_value: O0
  //      dest:           O1
  //      compare_value:  O2
  //
  // Results:
  //     O0: the value previously stored in dest

        .inline _Atomic_cas32, 3
        cas     [%o1], %o2, %o0
        .end


  // Support for intptr_t Atomic::cmpxchg_ptr(intptr_t           exchange_value, 
  //                                          volatile intptr_t* dest, 
  //                                          intptr_t           compare_value)
  //
  // 64-bit
  //
  // Arguments:
  //      exchange_value: O0
  //      dest:           O1
  //      compare_value:  O2
  //
  // Results:
  //     O0: the value previously stored in dest

        .inline _Atomic_cas64, 3
        casx    [%o1], %o2, %o0
        .end


  // Support for jlong Atomic::cmpxchg(jlong           exchange_value, 
  //                                   volatile jlong* dest, 
  //                                   jlong           compare_value)
  //
  // 32-bit calling conventions
  //
  // Arguments:
  //      exchange_value: O1:O0
  //      dest:           O2
  //      compare_value:  O4:O3
  //
  // Results:
  //     O1:O0: the value previously stored in dest

        .inline _Atomic_casl, 3
        sllx    %o0, 32, %o0
        srl     %o1, 0, %o1
        or      %o0,%o1,%o0
        sllx    %o3, 32, %o3
        srl     %o4, 0, %o4
        or      %o3,%o4,%o3
        casx    [%o2], %o3, %o0
        srl     %o0, 0, %o1
        srlx    %o0, 32, %o0
        .end


  // Support for jint Atomic::add(jint add_value, volatile jint* dest).
  //
  // Arguments:
  //      add_value: O0   (e.g., +1 or -1)
  //      dest:      O1
  //
  // Results:
  //     O0: the new value stored in dest
  //
  // Overwrites O3

        .inline _Atomic_add32, 2
    2:
        ld      [%o1], %o2
        add     %o0, %o2, %o3
        cas     [%o1], %o2, %o3
        cmp     %o2, %o3
        bne     2b
         nop
        add     %o0, %o2, %o0
        .end


  // Support for intptr_t Atomic::add_ptr(intptr_t add_value, volatile intptr_t* dest)
  //
  // 64-bit
  //
  // Arguments:
  //      add_value: O0   (e.g., +1 or -1)
  //      dest:      O1
  //
  // Results:
  //     O0: the new value stored in dest
  //
  // Overwrites O3

        .inline _Atomic_add64, 2
    3:
        ldx     [%o1], %o2
        add     %o0, %o2, %o3
        casx    [%o1], %o2, %o3
        cmp     %o2, %o3
        bne     %xcc, 3b
         nop
        add     %o0, %o2, %o0
        .end


  // Support for void OrderAccess::acquire()

        .inline _OrderAccess_acquire,0
        ld      [%sp], %g0
        .end


  // Support for void OrderAccess::fence()

        .inline _OrderAccess_fence,0
        membar  #StoreLoad
        .end


  // Support for void Prefetch::read(void *loc, intx interval)
  //
  // Prefetch for several reads.

        .inline _Prefetch_read, 2
        prefetch [%o0+%o1], 0
        .end


  // Support for void Prefetch::write(void *loc, intx interval)
  //
  // Prefetch for several writes.

        .inline _Prefetch_write, 2
        prefetch [%o0+%o1], 2
        .end


  // Support for void Copy::conjoint_jlongs_atomic(jlong* from, jlong* to, size_t count)
  //
  // 32-bit
  //
  // Arguments:
  //      from:  O0
  //      to:    O1
  //      count: O2 treated as signed
  //
  // Clobbers:
  //      long_value: O2, O3
  //      count:      O4
  //
  // if (from > to) {
  //   while (--count >= 0) {
  //     *to++ = *from++;
  //   }
  // } else {
  //   while (--count >= 0) {
  //     to[count] = from[count];
  //   }
  // }
        .inline _Copy_conjoint_jlongs_atomic, 3
        cmp     %o0, %o1
        bleu    4f
        sll     %o2, 3, %o4
        ba      2f
    1:
        subcc   %o4, 8, %o4
        std     %o2, [%o1]
        add     %o0, 8, %o0
        add     %o1, 8, %o1
    2:
        bge,a   1b
        ldd     [%o0], %o2
        ba      5f
        nop
    3:
        std     %o2, [%o1+%o4]
    4:
        subcc   %o4, 8, %o4
        bge,a   3b
        ldd     [%o0+%o4], %o2
    5:
        .end
