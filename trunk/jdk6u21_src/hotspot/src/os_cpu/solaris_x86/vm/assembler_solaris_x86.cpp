/*
 * Copyright (c) 1999, 2008, Oracle and/or its affiliates. All rights reserved.
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

#include "incls/_precompiled.incl"
#include "incls/_assembler_solaris_x86.cpp.incl"


void MacroAssembler::int3() {
  push(rax);
  push(rdx);
  push(rcx);
  call(RuntimeAddress(CAST_FROM_FN_PTR(address, os::breakpoint)));
  pop(rcx);
  pop(rdx);
  pop(rax);
}

#define __  _masm->
#ifndef _LP64
static void slow_call_thr_specific(MacroAssembler* _masm, Register thread) {

  // slow call to of thr_getspecific
  // int thr_getspecific(thread_key_t key, void **value);
  // Consider using pthread_getspecific instead.

__  push(0);                                                            // allocate space for return value
  if (thread != rax) __ push(rax);                                      // save rax, if caller still wants it
__  push(rcx);                                                          // save caller save
__  push(rdx);                                                          // save caller save
  if (thread != rax) {
__    lea(thread, Address(rsp, 3 * sizeof(int)));                       // address of return value
  } else {
__    lea(thread, Address(rsp, 2 * sizeof(int)));                       // address of return value
  }
__  push(thread);                                                       // and pass the address
__  push(ThreadLocalStorage::thread_index());                           // the key
__  call(RuntimeAddress(CAST_FROM_FN_PTR(address, thr_getspecific)));
__  increment(rsp, 2 * wordSize);
__  pop(rdx);
__  pop(rcx);
  if (thread != rax) __ pop(rax);
__  pop(thread);

}
#else
static void slow_call_thr_specific(MacroAssembler* _masm, Register thread) {
  // slow call to of thr_getspecific
  // int thr_getspecific(thread_key_t key, void **value);
  // Consider using pthread_getspecific instead.

  if (thread != rax) {
__    push(rax);
  }
__  push(0); // space for return value
__  push(rdi);
__  push(rsi);
__  lea(rsi, Address(rsp, 16)); // pass return value address
__  push(rdx);
__  push(rcx);
__  push(r8);
__  push(r9);
__  push(r10);
  // XXX
__  mov(r10, rsp);
__  andptr(rsp, -16);
__  push(r10);
__  push(r11);

__  movl(rdi, ThreadLocalStorage::thread_index());
__  call(RuntimeAddress(CAST_FROM_FN_PTR(address, thr_getspecific)));

__  pop(r11);
__  pop(rsp);
__  pop(r10);
__  pop(r9);
__  pop(r8);
__  pop(rcx);
__  pop(rdx);
__  pop(rsi);
__  pop(rdi);
__  pop(thread); // load return value
  if (thread != rax) {
__    pop(rax);
  }
}
#endif //LP64

void MacroAssembler::get_thread(Register thread) {

  int segment = NOT_LP64(Assembler::GS_segment) LP64_ONLY(Assembler::FS_segment);
  // Try to emit a Solaris-specific fast TSD/TLS accessor.
  ThreadLocalStorage::pd_tlsAccessMode tlsMode = ThreadLocalStorage::pd_getTlsAccessMode ();
  if (tlsMode == ThreadLocalStorage::pd_tlsAccessIndirect) {            // T1
     // Use thread as a temporary: mov r, gs:[0]; mov r, [r+tlsOffset]
     emit_byte (segment);
     // ExternalAddress doesn't work because it can't take NULL
     AddressLiteral null(0, relocInfo::none);
     movptr (thread, null);
     movptr(thread, Address(thread, ThreadLocalStorage::pd_getTlsOffset())) ;
     return ;
  } else
  if (tlsMode == ThreadLocalStorage::pd_tlsAccessDirect) {              // T2
     // mov r, gs:[tlsOffset]
     emit_byte (segment);
     AddressLiteral tls_off((address)ThreadLocalStorage::pd_getTlsOffset(), relocInfo::none);
     movptr (thread, tls_off);
     return ;
  }

  slow_call_thr_specific(this, thread);

}
