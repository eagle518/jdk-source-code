#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)assembler_linux_i486.cpp	1.11 03/12/23 16:38:05 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_assembler_linux_i486.cpp.incl"

void Assembler::int3() {
  call(CAST_FROM_FN_PTR(address, os::breakpoint), relocInfo::runtime_call_type);
}

void MacroAssembler::get_thread(Register thread) {
  movl(thread, esp);
  shrl(thread, PAGE_SHIFT);
  movl(thread, Address(noreg, thread, Address::times_4, (int)ThreadLocalStorage::sp_map_addr()));
}

bool MacroAssembler::needs_explicit_null_check(int offset) {
  // Linux kernel guarantees that the first page is always unmapped. Don't
  // assume anything more than that.
  bool offset_in_first_page =   0 <= offset  &&  offset < os::vm_page_size();
  return !offset_in_first_page;
}

