#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)icBuffer_sparc.cpp	1.26 03/12/23 16:37:12 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_icBuffer_sparc.cpp.incl"

int InlineCacheBuffer::ic_stub_code_size() {
#ifdef _LP64
  if (TraceJumps) return 600 * wordSize;
  return (NativeMovConstReg::instruction_size +  // sethi;add
	  NativeJump::instruction_size +          // sethi; jmp; delay slot
	  (1*BytesPerInstWord) + 1); 		// flush + 1 extra byte
#else
  if (TraceJumps) return 300 * wordSize;
  return (2+2+ 1) * wordSize + 1; // set/jump_to/nop + 1 byte so that code_end can be set in CodeBuffer
#endif
}

void InlineCacheBuffer::assemble_ic_buffer_code(address code_begin, oop cached_oop, address entry_point) {
  ResourceMark rm;
  CodeBuffer*     code            = new CodeBuffer(code_begin, ic_stub_code_size()); 
  MacroAssembler* masm            = new MacroAssembler(code);
  // note: even though the code contains an embedded oop, we do not need reloc info
  // because
  // (1) the oop is old (i.e., doesn't matter for scavenges)
  // (2) these ICStubs are removed *before* a GC happens, so the roots disappear
  assert(cached_oop == NULL || cached_oop->is_perm(), "must be old oop");
  Address cached_oop_addr(G5_inline_cache_reg, address(cached_oop));
  // Force the sethi to generate the fixed sequence so next_instruction_address works 
  masm->sethi(cached_oop_addr, true /* ForceRelocatable */ );
  masm->add(cached_oop_addr, G5_inline_cache_reg);
  assert(G3_scratch != G5_method, "Do not clobber the method oop in the transition stub");
  assert(G3_scratch != G5_inline_cache_reg, "Do not clobber the inline cache register in the transition stub");
  // masm->jump_to(Address(G3_scratch, entry_point));
  masm->JUMP(Address(G3_scratch, entry_point), 0);
  masm->delayed()->nop();
  masm->flush();
}


address InlineCacheBuffer::ic_buffer_entry_point(address code_begin) {
  NativeMovConstReg* move = nativeMovConstReg_at(code_begin);   // creation also verifies the object  
  NativeJump*        jump = nativeJump_at(move->next_instruction_address());
  return jump->jump_destination();
}


oop InlineCacheBuffer::ic_buffer_cached_oop(address code_begin) {
  NativeMovConstReg* move = nativeMovConstReg_at(code_begin);   // creation also verifies the object  
  NativeJump*        jump = nativeJump_at(move->next_instruction_address());
  return (oop)move->data();
}



//Reconciliation History
// 1.2 97/12/05 18:12:56 icBuffer_i486.cpp
// 1.4 97/12/14 13:56:51 icBuffer_i486.cpp
// 1.5 98/01/26 15:02:24 icBuffer_i486.cpp
// 1.5 98/01/31 00:15:31 icBuffer_i486.cpp
// 1.7 98/02/27 16:48:37 icBuffer_i486.cpp
// 1.7 98/03/05 17:17:02 icBuffer_i486.cpp
// 1.8 98/05/04 17:03:44 icBuffer_i486.cpp
// 1.9 98/06/01 12:43:05 icBuffer_i486.cpp
// 1.10 98/11/12 11:58:39 icBuffer_i486.cpp
// 1.12 99/06/22 16:37:41 icBuffer_i486.cpp
//End
