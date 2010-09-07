/*
 * Copyright (c) 1997, 2006, Oracle and/or its affiliates. All rights reserved.
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
#include "incls/_icBuffer_x86.cpp.incl"

int InlineCacheBuffer::ic_stub_code_size() {
  return NativeMovConstReg::instruction_size +
         NativeJump::instruction_size +
         1;
  // so that code_end can be set in CodeBuffer
  // 64bit 16 = 5 + 10 bytes + 1 byte
  // 32bit 11 = 10 bytes + 1 byte
}



void InlineCacheBuffer::assemble_ic_buffer_code(address code_begin, oop cached_oop, address entry_point) {
  ResourceMark rm;
  CodeBuffer      code(code_begin, ic_stub_code_size());
  MacroAssembler* masm            = new MacroAssembler(&code);
  // note: even though the code contains an embedded oop, we do not need reloc info
  // because
  // (1) the oop is old (i.e., doesn't matter for scavenges)
  // (2) these ICStubs are removed *before* a GC happens, so the roots disappear
  assert(cached_oop == NULL || cached_oop->is_perm(), "must be perm oop");
  masm->lea(rax, OopAddress((address) cached_oop));
  masm->jump(ExternalAddress(entry_point));
}


address InlineCacheBuffer::ic_buffer_entry_point(address code_begin) {
  NativeMovConstReg* move = nativeMovConstReg_at(code_begin);   // creation also verifies the object
  NativeJump*        jump = nativeJump_at(move->next_instruction_address());
  return jump->jump_destination();
}


oop InlineCacheBuffer::ic_buffer_cached_oop(address code_begin) {
  // creation also verifies the object
  NativeMovConstReg* move = nativeMovConstReg_at(code_begin);
  // Verifies the jump
  NativeJump*        jump = nativeJump_at(move->next_instruction_address());
  return (oop)move->data();
}
