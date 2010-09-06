#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)safepoint_solaris_sparc.cpp	1.12 03/12/23 16:38:23 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_safepoint_solaris_sparc.cpp.incl"

bool SafepointSynchronize::safepoint_safe(JavaThread *thread, JavaThreadState state) {
  switch(state) {
   case _thread_in_native:
     // native threads are safe if they have no java stack or have walkable stack
     return !thread->has_last_Java_frame() || thread->frame_anchor()->walkable();

   // blocked threads should have already have walkable stack
   case _thread_blocked:
     assert(!thread->has_last_Java_frame() || thread->frame_anchor()->walkable(), "blocked and not walkable");
     return true;

   default:
    return false;
  }
}

#ifndef CORE

// The patching of the return instruction is machine-specific
void SafepointSynchronize::patch_return_instruction_md(address cb_pc) {
  // In the case of SPARC the relocation info. is on the ret instruction
  // which is where we are going to patch
  assert(((NativeInstruction*)cb_pc)->is_return(), "must be a return instruction");
  assert(((NativeInstruction*)(cb_pc+NativeInstruction::nop_instruction_size))->is_nop(), "delay slot must be a nop instruction");
  NativeIllegalInstruction::insert(cb_pc);
}

static const int TRAMPOLINE_SIZE = 8 * jintSize;

int CompiledCodeSafepointHandler::pd_thread_code_buffer_size(nmethod* nm) {
  int scale_factor = 1;
#ifdef _LP64
  // Worst case is that every other instruction is a runtime call.
  // Could alternatively iterate through all relocation information to
  // compute precise size, but not worth it.
  scale_factor += (TRAMPOLINE_SIZE / 2);
#endif

  return scale_factor * nm->instructions_size();
}

void CompiledCodeSafepointHandler::pd_patch_runtime_calls_with_trampolines(ThreadCodeBuffer* cb,
                                                                           int offset_of_first_trampoline) {
#ifdef _LP64
#define __ masm.

  nmethod* nm = cb->method();

  // Round offset up to 8-byte boundary
  int cur_trampoline_offset = (int) align_size_up(offset_of_first_trampoline, wordSize);

  RelocIterator iter(nm);
  while(iter.next()) {
    address reloc_pc = iter.addr();
    if (reloc_pc >= nm->code_end())
    break;
    
    address cb_pc = cb->code_begin() + (reloc_pc - nm->instructions_begin());
    assert(cb->contains(cb_pc), "pc must be in temp. buffer");            

    switch(iter.type()) {	
      case relocInfo::runtime_call_type:
        {
          if (ShowSafepointMsgs && Verbose) { 
            tty->print_cr("trampolining runtime call at " INTPTR_FORMAT " (org. pc: " INTPTR_FORMAT ")", cb_pc, reloc_pc); 
          }
          Register tmp_reg = O7;
          // Replace offset of call instruction with that of trampoline, which
          // will fabricate the real 64-bit destination address and jump to it.
          // Contents of trampoline (32-bit words):
          //   dest address hi
          //   dest address lo
          //   save
          //   sethi %tmp_reg, [dest address offset wrt %i7]
          //   add   %tmp_reg, [dest address offset wrt %i7]
          //   ld    [%i7 + %tmp_reg], [%tmp_reg]
          //   jmpl  %tmp_reg, 0, %g0
          //   restore
          NativeCall* real_runtime_call = nativeCall_at(reloc_pc);
          address real_destination_pc = real_runtime_call->destination();
          NativeCall* patched_runtime_call = nativeCall_at(cb_pc);
          CodeBuffer trampoline_cb(cb->code_begin() + cur_trampoline_offset, TRAMPOLINE_SIZE);
          MacroAssembler masm(&trampoline_cb);
          intptr_t dest_addr_offset_in_bytes = (__ pc()) - cb_pc;
          assert(dest_addr_offset_in_bytes < cb->size(), "must be within ThreadCodeBuffer");
          assert(((uintptr_t) __ pc()) % wordSize == 0, "must be aligned");
          __ emit_data(((uintptr_t) real_destination_pc) >> 32);
          __ emit_data((uintptr_t) real_destination_pc);
          patched_runtime_call->set_destination(trampoline_cb.code_end());
          __ save_frame(0);
          __ sethi(dest_addr_offset_in_bytes & ~0x3ff, tmp_reg);
          __ add(tmp_reg, dest_addr_offset_in_bytes & 0x3ff, tmp_reg);
          __ ldx(I7, tmp_reg, tmp_reg);
          __ jmpl(tmp_reg, 0, G0);
          __ delayed()->restore();
          assert(trampoline_cb.code_end() - (cb->code_begin() + cur_trampoline_offset) == TRAMPOLINE_SIZE,
                 "miscalculation of trampoline size");
          assert(trampoline_cb.code_end() - cb->code_begin() < cb->size(),
                 "trampolines must fit within ThreadCodeBuffer");
          cur_trampoline_offset += TRAMPOLINE_SIZE;
          break;
        }
        
      default:
        break;
    }
  }
#undef __
#endif /* _LP64 */
}

#endif // !CORE
