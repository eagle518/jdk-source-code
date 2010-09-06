#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)assembler_linux_amd64.cpp	1.2 03/12/23 16:38:01 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_assembler_linux_amd64.cpp.incl"

void Assembler::int3()
{
  call(CAST_FROM_FN_PTR(address, os::breakpoint),
       relocInfo::runtime_call_type);
}

void MacroAssembler::get_thread(Register thread)
{
  // call pthread_getspecific
  // void * pthread_getspecific(pthread_key_t key);
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

   movl(rdi, ThreadLocalStorage::thread_index());
   call(CAST_FROM_FN_PTR(address, pthread_getspecific), 
        relocInfo::runtime_call_type);

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

// NOTE: since the linux kernel resides at the low end of
// user address space, no null pointer check is needed.
bool MacroAssembler::needs_explicit_null_check(int offset)
{
  return offset < 0 || offset >= 0x100000;
}
