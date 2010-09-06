#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)threadLS_linux_i486.hpp	1.10 03/12/23 16:38:09 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

  // Processor dependent parts of ThreadLocalStorage

  // map stack pointer to thread pointer - see notes in threadLS_linux_i486.cpp
  #define SP_BITLENGTH  32
  #define PAGE_SHIFT    12
  #define PAGE_SIZE     (1UL << PAGE_SHIFT)
  static Thread* _sp_map[1UL << (SP_BITLENGTH - PAGE_SHIFT)];

public:
  static Thread** sp_map_addr() { return _sp_map; }

  static Thread* thread() {
    uintptr_t sp;
    __asm__ volatile ("movl %%esp, %0" : "=r" (sp));
    return _sp_map[sp >> PAGE_SHIFT];
  }

