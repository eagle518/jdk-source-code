#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)safepoint_linux_amd64.cpp	1.3 03/12/23 16:38:03 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_safepoint_linux_amd64.cpp.incl"


bool
SafepointSynchronize::safepoint_safe(JavaThread* thread,
                                     JavaThreadState state)
{
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
void SafepointSynchronize::patch_return_instruction_md(address cb_pc)
{
  // The relocation info. is actually after the ret
  // (i.e., where we are going to patch with an illegal instruction.
  cb_pc -= NativeReturn::instruction_size;
  
  assert(((NativeInstruction*) cb_pc)->is_return(),
         "must be a return instruction");
  
  // In C1 code is part of ret eventually
  NOT_COMPILER1(assert(((NativeInstruction*) (cb_pc + 1))->is_nop(),
                       "must be a nop instruction");)
  NOT_COMPILER1(assert(((NativeInstruction*) (cb_pc + 2))->is_nop(),
                       "must be a nop instruction");)
  assert(NativePopReg::instruction_size + 
         NativeIllegalInstruction::instruction_size == 3,
         "wrong code format");
  // Pop return address (it is getting pushed by the SafepointBlob again)
  NativePopReg::insert(cb_pc, rbx);
  NativeIllegalInstruction::insert(cb_pc + NativePopReg::instruction_size);
}

#ifndef CORE

int CompiledCodeSafepointHandler::pd_thread_code_buffer_size(nmethod* nm)
{
  // Worst case: only calls/jmps (5 bytes), 16 additional bytes for each
  return nm->instructions_size() * 5 + 4 * 128 + 64; // 1 + 4
}

void 
CompiledCodeSafepointHandler::
pd_patch_runtime_calls_with_trampolines(ThreadCodeBuffer* cb,
                                        int offset_of_first_trampoline)
{
  nmethod* nm = cb->method();

  uintptr_t* data = (uintptr_t*) align_size_up((uintptr_t) cb->code_begin() +
                                               offset_of_first_trampoline,
                                               128);
                                    
  assert(((uintptr_t) data) % 128 == 0, "must be aligned");

  __int128_t* float_signflip = (__int128_t*) data;
  *float_signflip = 0x80000000800000008000000080000000;
  data += 2;

  __int128_t* double_signflip = (__int128_t*) data;
  *double_signflip = 0x80000000000000008000000000000000;
  data += 2;

  __int128_t* float_signmask = (__int128_t*) data;
  *float_signmask = 0x7FFFFFFF7FFFFFFF7FFFFFFF7FFFFFFF;
  data += 2;

  __int128_t* double_signmask = (__int128_t*) data;
  *double_signmask = 0x7FFFFFFFFFFFFFFF7FFFFFFFFFFFFFFF;
  data += 2;
  
  RelocIterator iter(nm);
  while (iter.next()) {
    address reloc_pc = iter.addr();
    if (reloc_pc >= nm->exception_end()) { // XXX
      break;
    }
    
    assert(((address) data) - cb->code_begin() < cb->size(),
           "trampolines must fit within ThreadCodeBuffer");

    address cb_pc = cb->code_begin() + (reloc_pc - nm->instructions_begin());
    assert(cb->contains(cb_pc), "pc must be in temp buffer");            

    switch (iter.type()) {
    case relocInfo::external_word_type: {
      if (ShowSafepointMsgs && Verbose) { 
        tty->print_cr("fixing global reference at " INTPTR_FORMAT 
                      " (org. pc: " INTPTR_FORMAT ")", cb_pc, reloc_pc); 
      }
      if (iter.datalen() == 0) {
        address disp = Assembler::locate_operand(reloc_pc,
                                                 Assembler::disp32_operand);
        __int128_t old_value = *(__int128_t*) (disp + *(int*) disp + 4);

        disp = Assembler::locate_operand(cb_pc, Assembler::disp32_operand);
        if (old_value == *float_signflip) {
          *(int*) disp = ((address) float_signflip) - disp - 4;
        } else if (old_value == *double_signflip) {
          *(int*) disp = ((address) double_signflip) - disp - 4;
        } else if (old_value == *float_signmask) {
          *(int*) disp = ((address) float_signmask) - disp - 4;
        } else if (old_value == *double_signmask) {
          *(int*) disp = ((address) double_signmask) - disp - 4;
        } else {
          ShouldNotReachHere();
        }
      }
      break;
    }
    case relocInfo::runtime_call_type: {
      if (ShowSafepointMsgs && Verbose) { 
        tty->print_cr("trampolining runtime call at " INTPTR_FORMAT 
                      " (org. pc: " INTPTR_FORMAT ")", cb_pc, reloc_pc); 
      }
      // Replace offset of call/jmp instruction with that of
      // trampoline, which will fabricate the real 64-bit destination
      // address and jump to it.
      // Contents of trampoline (14 bytes + 2 bytes padding)
      //  0: dest address
      //  8: ff 25 f2 ff ff ff      jmp     *-14(%rip)  #*0
      //  e: 66                     data16
      //  f: 90                     nop
      // Entry point is 8.

      // original call/jmp
      NativeInstruction* ni = nativeInstruction_at(reloc_pc);
      // real dest address
      uintptr_t real_dest_pc;
      if (ni->is_call()) {
        real_dest_pc = (uintptr_t) nativeCall_at(reloc_pc)->destination();
      } else if (ni->is_jump()) {
        real_dest_pc = (uintptr_t) nativeJump_at(reloc_pc)->jump_destination();
      } else if (ni->is_cond_jump()) {
        real_dest_pc =
          (uintptr_t) nativeGeneralJump_at(reloc_pc)->jump_destination();
      } else {
        ShouldNotReachHere();
      }

      // insert real dest address
      *data++ = real_dest_pc;

      // call/jmp we have to patch
      ni = nativeInstruction_at(cb_pc);
      // patch the call/jmp
      if (ni->is_call()) {
        nativeCall_at(cb_pc)->set_destination((address) data);
      } else if (ni->is_jump()) {
        nativeJump_at(cb_pc)->set_jump_destination((address) data);
      } else if (ni->is_cond_jump()) {
        // %%%% kludge this, for now, until we get a jump_destination method
        address old_dest = nativeGeneralJump_at(cb_pc)->jump_destination();
        address disp = Assembler::locate_operand(cb_pc, 
                                                 Assembler::call32_operand);
        *(int*) disp += (((address) data) - old_dest);
      } else { 
        ShouldNotReachHere(); 
      }

      // insert jmp *-14(%rip) ; data16 nop
      *data++ = 0x9066fffffff225ff;
      
      break;
    }
    default:
      break;
    }
  }
}

#endif // !CORE
