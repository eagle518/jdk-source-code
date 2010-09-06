#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)assembler_win32_i486.cpp	1.9 03/12/23 16:38:29 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_assembler_win32_i486.cpp.incl"


void Assembler::int3() {
  emit_byte(0xCC);
}

//  The current scheme to accelerate access to the thread
//  pointer is to store the current thread in the os_exception_wrapper
//  and reference the current thread from stubs and compiled code
//  via the FS register.  FS[0] contains a pointer to the structured
//  exception block which is actually a stack address.  The first time
//  we call the os exception wrapper, we calculate and store the
//  offset from this exception block and use that offset here.
//
//  The last mechanism we used was problematic in that the 
//  the offset we had hard coded in the VM kept changing as Microsoft
//  evolved the OS.
//
// Warning: This mechanism assumes that we only attempt to get the
//          thread when we are nested below a call wrapper.
//
// movl reg, fs:[0]                        Get exeception pointer
// movl reg, [reg + thread_ptr_offset]     Load thread
//
void MacroAssembler::get_thread(Register thread) {
  prefix(FS_segment); movl(thread, Address(0, relocInfo::none));
  assert(ThreadLocalStorage::get_thread_ptr_offset() != 0, 
         "Thread Pointer Offset has not been initialized");
  movl(thread, Address(thread, ThreadLocalStorage::get_thread_ptr_offset()));
}

bool MacroAssembler::needs_explicit_null_check(int offset) {
  return offset < 0 || (int)os::vm_page_size() <= offset;
}

