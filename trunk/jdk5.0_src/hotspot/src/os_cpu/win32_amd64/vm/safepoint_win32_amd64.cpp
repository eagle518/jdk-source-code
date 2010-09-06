#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)safepoint_win32_amd64.cpp	1.3 03/12/23 16:38:27 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_safepoint_win32_amd64.cpp.incl"

bool SafepointSynchronize::safepoint_safe(JavaThread *thread, JavaThreadState state) {
  switch(state) {
   // blocked threads and native threads are already safepoint safe.
   case _thread_in_native:
   case _thread_blocked:
     return true;

   default:
    return false;
  }
}

// The patching of the return instruction is machine-specific
void SafepointSynchronize::patch_return_instruction_md(address cb_pc) {
  // The relocation info. is actually after the ret (i.e., where we are going to patch with
  // an illegal instruction.
  cb_pc -= NativeReturn::instruction_size;

  assert(((NativeInstruction*)cb_pc)->is_return(), "must be a return instruction");

  // In C1  code is part of ret eventually
  NOT_COMPILER1(assert(((NativeInstruction*)(cb_pc+1))->is_nop(), "must be a nop instruction");)
  NOT_COMPILER1(assert(((NativeInstruction*)(cb_pc+2))->is_nop(), "must be a nop instruction");)
  assert(NativePopReg::instruction_size + NativeIllegalInstruction::instruction_size == 3, "wrong code format");
  NativePopReg::insert(cb_pc, rbx); // Pop return address (it is getting pushed by the SafepointBlob again)
  NativeIllegalInstruction::insert(cb_pc + NativePopReg::instruction_size);        
}

#ifndef CORE

int CompiledCodeSafepointHandler::pd_thread_code_buffer_size(nmethod* nm) {
  return nm->instructions_size();
}

void CompiledCodeSafepointHandler::pd_patch_runtime_calls_with_trampolines(ThreadCodeBuffer* cb,
                                                                           int offset_of_first_trampoline) {
  // Nothing to do
}

#endif // !CORE
