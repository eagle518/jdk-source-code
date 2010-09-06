#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)icBuffer_amd64.cpp	1.4 03/12/23 16:35:48 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_icBuffer_amd64.cpp.incl"

int InlineCacheBuffer::ic_stub_code_size()
{
  return NativeMovConstReg::instruction_size +
         NativeJump::instruction_size +
         1;
  // 16 = 5 + 10 bytes + 1 byte so that code_end can be set in CodeBuffer
}

void InlineCacheBuffer::assemble_ic_buffer_code(address code_begin,
                                                oop cached_oop,
                                                address entry_point) 
{
  ResourceMark rm;
  CodeBuffer* code = new CodeBuffer(code_begin, ic_stub_code_size());
  MacroAssembler* masm = new MacroAssembler(code);
  // note: even though the code contains an embedded oop, we do not
  // need reloc info because
  // (1) the oop is old (i.e., doesn't matter for scavenges)
  // (2) these ICStubs are removed *before* a GC happens, so the roots
  //     disappear
  assert(cached_oop == NULL || cached_oop->is_perm(), "must be perm oop");
  masm->movq(rax, (int64_t) cached_oop);
  masm->jmp(entry_point, relocInfo::none);
}


address InlineCacheBuffer::ic_buffer_entry_point(address code_begin) 
{
  // creation also verifies the object  
  NativeMovConstReg* move = nativeMovConstReg_at(code_begin);
  NativeJump* jump = nativeJump_at(move->next_instruction_address());
  return jump->jump_destination();
}


oop InlineCacheBuffer::ic_buffer_cached_oop(address code_begin) 
{
  // creation also verifies the object  
  NativeMovConstReg* move = nativeMovConstReg_at(code_begin);
  return (oop) move->data();
}
