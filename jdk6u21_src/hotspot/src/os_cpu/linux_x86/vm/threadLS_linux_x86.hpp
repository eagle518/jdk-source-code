/*
 * Copyright (c) 1999, 2003, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

  // Processor dependent parts of ThreadLocalStorage

#ifndef AMD64
  // map stack pointer to thread pointer - see notes in threadLS_linux_x86.cpp
  #define SP_BITLENGTH  32
  #define PAGE_SHIFT    12
  #define PAGE_SIZE     (1UL << PAGE_SHIFT)
  static Thread* _sp_map[1UL << (SP_BITLENGTH - PAGE_SHIFT)];
#endif // !AMD64

public:

#ifndef AMD64
  static Thread** sp_map_addr() { return _sp_map; }
#endif // !AMD64

  static Thread* thread() {
#ifdef AMD64
    return (Thread*) os::thread_local_storage_at(thread_index());
#else
    uintptr_t sp;
    __asm__ volatile ("movl %%esp, %0" : "=r" (sp));
    return _sp_map[sp >> PAGE_SHIFT];
#endif // AMD64
  }
