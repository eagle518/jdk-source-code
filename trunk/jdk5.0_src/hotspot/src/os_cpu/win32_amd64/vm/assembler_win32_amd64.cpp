#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)assembler_win32_amd64.cpp	1.5 03/12/23 16:38:25 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_assembler_win32_amd64.cpp.incl"


void Assembler::int3() {
  emit_byte(0xCC);
}

// call (Thread*)TlsGetValue(thread_index());
void MacroAssembler::get_thread(Register thread) {
   if (thread != rax) {
     pushq(rax);
   } 
   pushq(rdi);
   pushq(rsi);
   pushq(rdx);
   pushq(rcx);
   pushq(r8);
   pushq(r9);
   pushq(r10);
   // XXX
   movq(r10, rsp);
   andq(rsp, -16);
   pushq(r10);
   pushq(r11);

   movl(rarg0, ThreadLocalStorage::thread_index());
   call((address)TlsGetValue, relocInfo::none);

   popq(r11);
   popq(rsp);
   popq(r10);
   popq(r9);
   popq(r8);
   popq(rcx);
   popq(rdx);
   popq(rsi);
   popq(rdi);
   if (thread != rax) {
       movq(thread, rax);
       popq(rax);
   }
}

bool MacroAssembler::needs_explicit_null_check(int offset) {
  return offset < 0 || (int)os::vm_page_size() <= offset;
}

