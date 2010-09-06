#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)icBuffer_ia64.cpp	1.8 03/12/23 16:36:41 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_icBuffer_ia64.cpp.incl"


// movl inline_cache_reg, cached_oop
// movl method_oop_reg, target_address
// mov  branch_reg, method_oop_reg
// br   branch_reg
int InlineCacheBuffer::ic_stub_code_size() {
  return (NativeMovConstReg::instruction_size +  NativeJump::instruction_size ) + 1;          
}


void InlineCacheBuffer::assemble_ic_buffer_code(address code_begin, oop cached_oop, address entry_point) {

  ResourceMark rm;
  CodeBuffer*     code            = new CodeBuffer(code_begin, ic_stub_code_size());
  MacroAssembler masm(code);
  // note: even though the code contains an embedded oop, we do not need reloc info
  // because
  // (1) the oop is old (i.e., doesn't matter for scavenges)
  // (2) these ICStubs are removed *before* a GC happens, so the roots disappear

  assert(((GR27_inline_cache_reg != GR2_SCRATCH) &&
          (GR30_compiler_method_oop_reg != GR2_SCRATCH)),
         "conflicting registers in InlineCacheBuffer::assemble_ic_buffer_code" );

  masm.movl(GR27_inline_cache_reg, (int64_t)cached_oop);
  masm.movl(GR2_SCRATCH, (int64_t)entry_point);
  masm.mov(BR7_SCRATCH, GR2_SCRATCH);
  masm.br(BR7_SCRATCH);
  masm.flush();
}


address InlineCacheBuffer::ic_buffer_entry_point(address code_begin) {

  // creation also verifies the object
  NativeMovConstReg* move = nativeMovConstReg_at(code_begin);   
  NativeJump*        jump = nativeJump_at(move->next_instruction_address());
  return jump->jump_destination();
}


oop InlineCacheBuffer::ic_buffer_cached_oop(address code_begin) {

  // creation also verifies the object
  NativeMovConstReg* move = nativeMovConstReg_at(code_begin);   
  NativeJump*        jump = nativeJump_at(move->next_instruction_address());
  return (oop)move->data();
}

