#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)runtime_amd64.cpp	1.10 03/12/23 16:35:55 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_runtime_amd64.cpp.incl"

DeoptimizationBlob *SharedRuntime::_deopt_blob;
UncommonTrapBlob   *OptoRuntime::_uncommon_trap_blob;
ExceptionBlob      *OptoRuntime::_exception_blob;
SafepointBlob      *OptoRuntime::_illegal_instruction_handler_blob;
SafepointBlob      *OptoRuntime::_polling_page_safepoint_handler_blob;
SafepointBlob      *OptoRuntime::_polling_page_return_handler_blob;

#define __ masm->

class RegisterSaver {
  // Capture info about frame layout.  Layout offsets are in jint
  // units because compiler frame slots are jints.
#define DEF_XMM_OFFS(regnum) xmm ## regnum ## _off = xmm_off + (regnum)*16/BytesPerInt, xmm ## regnum ## H_off
  enum layout { 
    fpu_state_off = frame::arg_reg_save_area_bytes/BytesPerInt, // fxsave save area
    xmm_off       = fpu_state_off + 160/BytesPerInt,            // offset in fxsave save area
    DEF_XMM_OFFS(0),
    DEF_XMM_OFFS(1),
    DEF_XMM_OFFS(2),
    DEF_XMM_OFFS(3),
    DEF_XMM_OFFS(4),
    DEF_XMM_OFFS(5),
    DEF_XMM_OFFS(6),
    DEF_XMM_OFFS(7),
    DEF_XMM_OFFS(8),
    DEF_XMM_OFFS(9),
    DEF_XMM_OFFS(10),
    DEF_XMM_OFFS(11),
    DEF_XMM_OFFS(12),
    DEF_XMM_OFFS(13),
    DEF_XMM_OFFS(14),
    DEF_XMM_OFFS(15),
    fpu_state_end = fpu_state_off + ((FPUStateSizeInWords-1)*wordSize / BytesPerInt),
    fpu_stateH_end,
    r15_off, r15H_off,
    r14_off, r14H_off,
    r13_off, r13H_off,
    r12_off, r12H_off,
    r11_off, r11H_off,
    r10_off, r10H_off,
    r9_off,  r9H_off,
    r8_off,  r8H_off,
    rdi_off, rdiH_off,
    rsi_off, rsiH_off,
    ignore_off, ignoreH_off,  // extra copy of rbp
    rsp_off, rspH_off,
    rbx_off, rbxH_off,
    rdx_off, rdxH_off,
    rcx_off, rcxH_off,
    rax_off, raxH_off,
    // 16-byte stack alignment fill word: see MacroAssembler::push/pop_IU_state
    align_off, alignH_off,
    flags_off, flagsH_off,
    rbp_off, rbpH_off,        // copy of rbp we will restore
    return_off, returnH_off,  // slot for return address
    reg_save_size             // size in compiler stack slots
  };

 public: 
  static OopMap* save_live_registers(MacroAssembler* masm, int additional_frame_words, int* total_frame_words);
  static void restore_live_registers(MacroAssembler* masm);

  // Offsets into the register save area
  // Used by deoptimization when it is managing result register
  // values on its own

  static int rax_offset_in_bytes(void)    { return BytesPerInt * rax_off; }
  static int r11_offset_in_bytes(void)    { return BytesPerInt * r11_off; }
  static int xmm0_offset_in_bytes(void)   { return BytesPerInt * xmm0_off; }
  static int return_offset_in_bytes(void) { return BytesPerInt * return_off; }

  // During deoptimization only the result registers need to be restored,
  // all the other values have already been extracted.
  static void restore_result_registers(MacroAssembler* masm);
};

OopMap* RegisterSaver::save_live_registers(MacroAssembler* masm, int additional_frame_words, int* total_frame_words) {

  // Always make the frame size 16-byte aligned
  int frame_size_in_bytes = round_to(additional_frame_words*wordSize +
                                     reg_save_size*BytesPerInt, 16);
  // OopMap frame size is in compiler stack slots (jint's) not bytes or words
  int frame_size_in_slots = frame_size_in_bytes / BytesPerInt;
  // The caller will allocate additional_frame_words
  int additional_frame_slots = additional_frame_words*wordSize / BytesPerInt;
  // CodeBlob frame size is in words.
  int frame_size_in_words = frame_size_in_bytes / wordSize;
  *total_frame_words = frame_size_in_words;

  // Save registers, fpu state, and flags.
  // We assume caller has already pushed the return address onto the
  // stack, so rsp is 8-byte aligned here.
  // We push rpb twice in this sequence because we want the real rbp
  // to be under the return like a normal enter.

  __ pushq(rbp);       // rsp becomes 16-byte aligned here
  __ push_CPU_state(); // Push a multiple of 16 bytes
  __ subq(rsp, frame::arg_reg_save_area_bytes);
                       // Allocate argument register save area

  // Set an oopmap for the call site.  This oopmap will map all
  // oop-registers and debug-info registers as callee-saved.  This
  // will allow deoptimization at this safepoint to find all possible
  // debug-info recordings, as well as let GC find all oops.

  OopMapSet *oop_maps = new OopMapSet();
  OopMap* map = new OopMap(frame_size_in_slots, 0);
  map->set_callee_saved(SharedInfo::stack2reg( rax_off  + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name( RAX_num));
  map->set_callee_saved(SharedInfo::stack2reg( raxH_off + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name( RAX_H_num));
  map->set_callee_saved(SharedInfo::stack2reg( rcx_off  + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name( RCX_num));
  map->set_callee_saved(SharedInfo::stack2reg( rcxH_off + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name( RCX_H_num));
  map->set_callee_saved(SharedInfo::stack2reg( rdx_off  + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name( RDX_num));
  map->set_callee_saved(SharedInfo::stack2reg( rdxH_off + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name( RDX_H_num));
  map->set_callee_saved(SharedInfo::stack2reg( rbx_off  + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name( RBX_num));
  map->set_callee_saved(SharedInfo::stack2reg( rbxH_off + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name( RBX_H_num));
  map->set_callee_saved(SharedInfo::stack2reg( rbp_off  + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name( RBP_num));
  map->set_callee_saved(SharedInfo::stack2reg( rbpH_off + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name( RBP_H_num));
  map->set_callee_saved(SharedInfo::stack2reg( rsi_off  + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name( RSI_num));
  map->set_callee_saved(SharedInfo::stack2reg( rsiH_off + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name( RSI_H_num));
  map->set_callee_saved(SharedInfo::stack2reg( rdi_off  + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name( RDI_num));
  map->set_callee_saved(SharedInfo::stack2reg( rdiH_off + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name( RDI_H_num));
  map->set_callee_saved(SharedInfo::stack2reg( r8_off   + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name( R8_num));
  map->set_callee_saved(SharedInfo::stack2reg( r8H_off  + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name( R8_H_num));
  map->set_callee_saved(SharedInfo::stack2reg( r9_off   + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name( R9_num));
  map->set_callee_saved(SharedInfo::stack2reg( r9H_off  + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name( R9_H_num));
  map->set_callee_saved(SharedInfo::stack2reg( r10_off  + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name( R10_num));
  map->set_callee_saved(SharedInfo::stack2reg( r10H_off + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name( R10_H_num));
  map->set_callee_saved(SharedInfo::stack2reg( r11_off  + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name( R11_num));
  map->set_callee_saved(SharedInfo::stack2reg( r11H_off + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name( R11_H_num));
  map->set_callee_saved(SharedInfo::stack2reg( r12_off  + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name( R12_num));
  map->set_callee_saved(SharedInfo::stack2reg( r12H_off + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name( R12_H_num));
  map->set_callee_saved(SharedInfo::stack2reg( r13_off  + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name( R13_num));
  map->set_callee_saved(SharedInfo::stack2reg( r13H_off + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name( R13_H_num));
  map->set_callee_saved(SharedInfo::stack2reg( r14_off  + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name( R14_num));
  map->set_callee_saved(SharedInfo::stack2reg( r14H_off + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name( R14_H_num));
  map->set_callee_saved(SharedInfo::stack2reg( r15_off  + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name( R15_num));
  map->set_callee_saved(SharedInfo::stack2reg( r15H_off + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name( R15_H_num));
  map->set_callee_saved(SharedInfo::stack2reg(xmm0_off  + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name(XMM0_num));
  map->set_callee_saved(SharedInfo::stack2reg(xmm0H_off + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name(XMM0_H_num));
  map->set_callee_saved(SharedInfo::stack2reg(xmm1_off  + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name(XMM1_num));
  map->set_callee_saved(SharedInfo::stack2reg(xmm1H_off + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name(XMM1_H_num));
  map->set_callee_saved(SharedInfo::stack2reg(xmm2_off  + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name(XMM2_num));
  map->set_callee_saved(SharedInfo::stack2reg(xmm2H_off + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name(XMM2_H_num));
  map->set_callee_saved(SharedInfo::stack2reg(xmm3_off  + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name(XMM3_num));
  map->set_callee_saved(SharedInfo::stack2reg(xmm3H_off + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name(XMM3_H_num));
  map->set_callee_saved(SharedInfo::stack2reg(xmm4_off  + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name(XMM4_num));
  map->set_callee_saved(SharedInfo::stack2reg(xmm4H_off + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name(XMM4_H_num));
  map->set_callee_saved(SharedInfo::stack2reg(xmm5_off  + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name(XMM5_num));
  map->set_callee_saved(SharedInfo::stack2reg(xmm5H_off + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name(XMM5_H_num));
  map->set_callee_saved(SharedInfo::stack2reg(xmm6_off  + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name(XMM6_num));
  map->set_callee_saved(SharedInfo::stack2reg(xmm6H_off + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name(XMM6_H_num));
  map->set_callee_saved(SharedInfo::stack2reg(xmm7_off  + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name(XMM7_num));
  map->set_callee_saved(SharedInfo::stack2reg(xmm7H_off + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name(XMM7_H_num));
  map->set_callee_saved(SharedInfo::stack2reg(xmm8_off  + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name(XMM8_num));
  map->set_callee_saved(SharedInfo::stack2reg(xmm8H_off + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name(XMM8_H_num));
  map->set_callee_saved(SharedInfo::stack2reg(xmm9_off  + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name(XMM9_num));
  map->set_callee_saved(SharedInfo::stack2reg(xmm9H_off + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name(XMM9_H_num));
  map->set_callee_saved(SharedInfo::stack2reg(xmm10_off + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name(XMM10_num));
  map->set_callee_saved(SharedInfo::stack2reg(xmm10H_off+ additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name(XMM10_H_num));
  map->set_callee_saved(SharedInfo::stack2reg(xmm11_off + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name(XMM11_num));
  map->set_callee_saved(SharedInfo::stack2reg(xmm11H_off+ additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name(XMM11_H_num));
  map->set_callee_saved(SharedInfo::stack2reg(xmm12_off + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name(XMM12_num));
  map->set_callee_saved(SharedInfo::stack2reg(xmm12H_off+ additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name(XMM12_H_num));
  map->set_callee_saved(SharedInfo::stack2reg(xmm13_off + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name(XMM13_num));
  map->set_callee_saved(SharedInfo::stack2reg(xmm13H_off+ additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name(XMM13_H_num));
  map->set_callee_saved(SharedInfo::stack2reg(xmm14_off + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name(XMM14_num));
  map->set_callee_saved(SharedInfo::stack2reg(xmm14H_off+ additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name(XMM14_H_num));
  map->set_callee_saved(SharedInfo::stack2reg(xmm15_off + additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name(XMM15_num));
  map->set_callee_saved(SharedInfo::stack2reg(xmm15H_off+ additional_frame_slots), frame_size_in_slots, 0, OptoReg::Name(XMM15_H_num));

  return map;
}

void RegisterSaver::restore_live_registers(MacroAssembler* masm) {
  // Pop arg register save area
  __ addq(rsp, frame::arg_reg_save_area_bytes);
  // Recover CPU state
  __ pop_CPU_state();
  // Get the rbp described in the oopMap
  __ popq(rbp);
}

void RegisterSaver::restore_result_registers(MacroAssembler* masm) {

  // Just restore result register. Only used by deoptimization. By
  // now any callee save register that needs to be restored to a c2
  // caller of the deoptee has been extracted into the vframeArray
  // and will be stuffed into the c2i adapter we create for later
  // restoration so only result registers need to be restored here.

  // Restore fp result register
  __ movlpd(xmm0, Address(rsp, xmm0_offset_in_bytes()));
  // Restore integer result register
  __ movq(rax, Address(rsp, rax_offset_in_bytes()));
  // Pop all of the register save are off the stack except the return address
  __ addq(rsp, return_offset_in_bytes());
}

//------------------------------generate_deopt_blob----------------------------
// Ought to generate an ideal graph & compile, but here's some Intel ASM
// instead.  Sigh, Cliff.
void SharedRuntime::generate_deopt_blob()
{
  // Allocate space for the code
  ResourceMark rm;
  // Setup code generation tools
  CodeBuffer*   buffer = new CodeBuffer(2048, 1024, 0, 0, 0, false);
  MacroAssembler* masm = new MacroAssembler(buffer);
  int frame_size_in_words;
  OopMap* map = NULL;
  OopMapSet *oop_maps = new OopMapSet();

  // -------------
  // This code enters when returning to a de-optimized nmethod.  A return
  // address has been pushed on the the stack, and return values are in
  // registers. 
  // If we are doing a normal deopt then we were called from the patched 
  // nmethod from the point we returned to the nmethod. So the return
  // address on the stack is wrong by NativeCall::instruction_size
  // We will adjust the value so it looks like we have the original return
  // address on the stack (like when we eagerly deoptimized).
  // In the case of an exception pending when deoptimizing, we enter
  // with a return address on the stack that points after the call we patched
  // into the exception handler. We have the following register state from,
  // e.g., the forward exception stub (see stubGenerator_amd64.cpp).
  //    rax: exception oop
  //    rbx: exception handler
  //    rdx: throwing pc
  // So in this case we simply jam rdx into the useless return address and
  // the stack looks just like we want.
  //
  // At this point we need to de-opt.  We save the argument return
  // registers.  We call the first C routine, fetch_unroll_info().  This
  // routine captures the return values and returns a structure which
  // describes the current frame size and the sizes of all replacement frames.
  // The current frame is compiled code and may contain many inlined
  // functions, each with their own JVM state.  We pop the current frame, then
  // push all the new frames.  Then we call the C routine unpack_frames() to
  // populate these frames.  Finally unpack_frames() returns us the new target
  // address.  Notice that callee-save registers are BLOWN here; they have
  // already been captured in the vframeArray at the time the return PC was
  // patched.
  address start = __ pc();
  Label cont;

  // Prolog for non exception case!
  // Correct the return address we were given.
  __ subq(Address(rsp, 0), NativeCall::instruction_size);

  // Save everything in sight.
  map = RegisterSaver::save_live_registers(masm, 0, &frame_size_in_words);

  // Normal deoptimization.  Save exec mode for unpack_frames.
  __ movl(r12, Deoptimization::Unpack_deopt); // callee-saved
  __ jmp(cont);
  
  int exception_offset = __ pc() - start;

  // Prolog for exception case

  // Push throwing pc as return address
  __ pushq(rdx);

  // Save everything in sight.
  map = RegisterSaver::save_live_registers(masm, 0, &frame_size_in_words);

  // Deopt during an exception.  Save exec mode for unpack_frames.
  __ movl(r12, Deoptimization::Unpack_exception); // callee-saved

  __ bind(cont);

  // Call C code.  Need thread and this frame, but NOT official VM entry
  // crud.  We cannot block on this call, no GC can happen.  
  //
  // UnrollBlock* fetch_unroll_info(JavaThread* thread)

  // fetch_unroll_info needs to call last_java_frame().

  __ set_last_Java_frame(noreg, noreg, noreg, NULL);
#ifdef ASSERT
  { Label L;
    __ cmpq(Address(r15_thread,
                    JavaThread::last_Java_fp_offset()),
            0);
    __ jcc(Assembler::equal, L);
    __ stop("OptoRuntime::generate_deopt_blob: last_Java_fp not cleared");
    __ bind(L);
  }
#endif // ASSERT
  __ movq(rarg0, r15_thread);
  __ call(CAST_FROM_FN_PTR(address, Deoptimization::fetch_unroll_info),
          relocInfo::runtime_call_type);

  // Need to have an oopmap that tells fetch_unroll_info where to
  // find any register it might need.
  oop_maps->add_gc_map(__ pc() - start, true, map);

  __ reset_last_Java_frame(noreg, false);

  // Load UnrollBlock* into rdi
  __ movq(rdi, rax);

  // Only register save data is on the stack.
  // Now restore the result registers.  Everything else is either dead
  // or captured in the vframeArray.
  RegisterSaver::restore_result_registers(masm);

  // All of the register save area has been popped of the stack. Only the
  // return address remains.

  // Pop all the frames we must move/replace. 
  // 
  // Frame picture (youngest to oldest)
  // 1: self-frame (no frame link)
  // 2: deopting frame  (no frame link)
  // 3: possible-i2c-adapter-frame 
  // 4: caller of deopting frame (could be compiled/interpreted. If interpreted we will create an
  //    and c2i here)
  // 
  // Note: by leaving the return address of self-frame on the stack
  // and using the size of frame(s) 2 & 3 to adjust the stack
  // when we are done the return to frame 4 will still be on the stack.

  // Pop deoptimized frame
  __ movl(rcx, Address(rdi, Deoptimization::UnrollBlock::size_of_deoptimized_frame_offset_in_bytes()));
  __ addq(rsp, rcx);

  // Pop I2C adapter frame, if any
  __ movl(rcx, Address(rdi, Deoptimization::UnrollBlock::adapter_size_offset_in_bytes()));
  __ addq(rsp, rcx);

  // rsp should be pointing at the return address to the caller (4)

  // Load address of array of frame pcs into rcx
  __ movq(rcx, Address(rdi, Deoptimization::UnrollBlock::frame_pcs_offset_in_bytes()));

  // Trash the old pc
  __ addq(rsp, wordSize);

  // Load address of array of frame sizes into rsi
  __ movq(rsi, Address(rdi, Deoptimization::UnrollBlock::frame_sizes_offset_in_bytes()));

  // Load counter into rdx
  __ movl(rdx, Address(rdi, Deoptimization::UnrollBlock::number_of_frames_offset_in_bytes()));

  // Pick up the initial fp we should save
  __ movq(rbp, Address(rdi, Deoptimization::UnrollBlock::initial_fp_offset_in_bytes()));


  Label adapter_done;

  // Do we need an adapter frame?
  __ cmpl(Address(rdi,
                  Deoptimization::UnrollBlock::new_adapter_offset_in_bytes()),
          0);
  __ jcc (Assembler::zero, adapter_done);

  // Push a c2i adapter frame
  __ pushq(Address(rcx));               // Push new pc
  __ movq(rbx, Address(rsi));           // Get framesize (includes return address)
  __ subq(rbx, wordSize);               // Account for return address
  __ decl(rdx);                         // Count the frame
  __ addq(rsi, wordSize);               // Bump array pointer (sizes)
  __ addq(rcx, wordSize);               // Bump array pointer (pcs)
  __ subq(rsp, rbx);                    // Allocate adapter frame
  __ bind(adapter_done);

  // Now adjust the caller's stack to make up for the extra locals
  // but record the original sp so that we can save it in the skeletal interpreter
  // frame and the stack walking of interpreter_sender will get the unextended sp
  // value and not the "real" sp value.

  const Register sender_sp = r8;

  __ movq(sender_sp, rsp);
  __ movl(rbx, Address(rdi,
                       Deoptimization::UnrollBlock::
                       caller_adjustment_offset_in_bytes()));
  __ subq(rsp, rbx);

  // Push interpreter frames in a loop
  Label loop;
  __ bind(loop);
  __ movq(rbx, Address(rsi));           // Load frame size
  __ subq(rbx, 2*wordSize);             // We'll push pc and ebp by hand
  __ pushq(Address(rcx));               // Save return address
  __ enter();                           // Save old & set new ebp
  __ subq(rsp, rbx);                    // Prolog
  __ movq(Address(rbp, frame::interpreter_frame_sender_sp_offset * wordSize),
          sender_sp);                   // Make it walkable
  __ movq(sender_sp, rsp);              // Pass sender_sp to next frame
  __ addq(rsi, wordSize);               // Bump array pointer (sizes)
  __ addq(rcx, wordSize);               // Bump array pointer (pcs)
  __ decl(rdx);                         // Decrement counter
  __ jcc(Assembler::notZero, loop);
  __ pushq(Address(rcx));               // Save final return address

  // Re-push self-frame
  __ enter();                           // Save old & set new ebp

  // Allocate a full sized register save area.
  // Return address and rbp are in place, so we allocate two less words.
  __ subq(rsp, (frame_size_in_words - 2) * wordSize);

  // Restore frame locals after moving the frame
  __ movsd(Address(rsp, RegisterSaver::xmm0_offset_in_bytes()), xmm0);
  __ movq(Address(rsp, RegisterSaver::rax_offset_in_bytes()), rax);

  // Call C code.  Need thread but NOT official VM entry
  // crud.  We cannot block on this call, no GC can happen.  Call should
  // restore return values to their stack-slots with the new SP.
  //
  // void Deoptimization::unpack_frames(JavaThread* thread, int exec_mode)

  __ set_last_Java_frame(noreg, noreg, rbp, NULL);

  __ movq(rarg0, r15_thread);
  __ movl(rarg1, r12); // second arg: exec_mode
  __ call(CAST_FROM_FN_PTR(address, Deoptimization::unpack_frames),
          relocInfo::runtime_call_type);

  // Set an oopmap for the call site
  oop_maps->add_gc_map(__ pc() - start,
                       true,
                       new OopMap( frame_size_in_words, 0 ));

  __ reset_last_Java_frame(noreg, false);

  // Collect return values
  __ movlpd(xmm0, Address(rsp, RegisterSaver::xmm0_offset_in_bytes()));
  __ movq(rax, Address(rsp, RegisterSaver::rax_offset_in_bytes()));

  // Pop self-frame.
  __ leave();                           // Epilog

  // Jump to interpreter
  __ ret(0);
  
  // Make sure all code is generated
  masm->flush();

  _deopt_blob = DeoptimizationBlob::create( buffer, oop_maps, 0, exception_offset, 0, frame_size_in_words);
}

//------------------------------generate_uncommon_trap_blob--------------------
// Ought to generate an ideal graph & compile, but here's some Intel ASM
// instead.  Sigh, Cliff.
void OptoRuntime::generate_uncommon_trap_blob()
{
  // Allocate space for the code
  ResourceMark rm;
  // Setup code generation tools
  CodeBuffer* buffer = new CodeBuffer(2048, 1024, 0, 0, 0, false);
  MacroAssembler* masm = new MacroAssembler(buffer);

  // Offsets are for compiler stack slots, which are jint's.
  enum layout {
    r13_off = frame::arg_reg_save_area_bytes/BytesPerInt,
                r13_off2,
    r14_off,    r14_off2,
    rbp_off,    rbp_off2,
    return_off, return_off2,
    framesize
  };
  assert(framesize % 4 == 0, "sp not 16-byte aligned");
  
  address start = __ pc();

  // Push self-frame.  We get here with a return address on the
  // stack, so rsp is 8-byte aligned until we allocate our frame.
  __ subq(rsp, return_off << LogBytesPerInt); // Epilog!

  // Save callee-saved registers.  See amd64.ad.
  __ movq(Address(rsp, rbp_off << LogBytesPerInt), rbp);
  __ movq(Address(rsp, r13_off << LogBytesPerInt), r13);
  __ movq(Address(rsp, r14_off << LogBytesPerInt), r14);

  // compiler left unloaded_class_index in rcx, which is rarg0 on Windows.
  __ movl(rarg1, rcx);

  __ set_last_Java_frame(noreg, noreg, noreg, NULL);

  // Call C code.  Need thread but NOT official VM entry
  // crud.  We cannot block on this call, no GC can happen.  Call should
  // capture callee-saved registers as well as return values.
  // Thread is in rdi already.
  //
  // UnrollBlock* uncommon_trap(JavaThread* thread, jint unloaded_class_index);

  __ movq(rarg0, r15_thread);
  __ call(CAST_FROM_FN_PTR(address, Deoptimization::uncommon_trap),
          relocInfo::runtime_call_type);

  // Set an oopmap for the call site
  OopMapSet* oop_maps = new OopMapSet();
  OopMap* map = new OopMap(framesize, 0);

  map->set_callee_saved(SharedInfo::stack2reg(r13_off),
                        framesize, 0,
                        OptoReg::Name(R13_num));
  map->set_callee_saved(SharedInfo::stack2reg(r13_off2),
                        framesize, 0,
                        OptoReg::Name(R13_H_num));
  map->set_callee_saved(SharedInfo::stack2reg(r14_off),
                        framesize, 0,
                        OptoReg::Name(R14_num));
  map->set_callee_saved(SharedInfo::stack2reg(r14_off2),
                        framesize, 0,
                        OptoReg::Name(R14_H_num));
  map->set_callee_saved(SharedInfo::stack2reg(rbp_off),
                        framesize, 0,
                        OptoReg::Name(RBP_num));
  map->set_callee_saved(SharedInfo::stack2reg(rbp_off2),
                        framesize, 0,
                        OptoReg::Name(RBP_H_num));

  oop_maps->add_gc_map(__ pc() - start, true, map);

  __ reset_last_Java_frame(noreg, false);

  // Load UnrollBlock* into rdi
  __ movq(rdi, rax);

  // Pop all the frames we must move/replace. 
  // 
  // Frame picture (youngest to oldest)
  // 1: self-frame (no frame link)
  // 2: deopting frame  (no frame link)
  // 3: possible-i2c-adapter-frame 
  // 4: caller of deopting frame (could be compiled/interpreted. If
  //    interpreted we will create an and c2i here)

  // Pop self-frame.  We have no frame, and must rely only on rax and rsp.
  __ addq(rsp, (framesize - 2) << LogBytesPerInt); // Epilog!

  // Pop deoptimized frame (int)
  __ movl(rcx, Address(rdi,
                       Deoptimization::UnrollBlock::
                       size_of_deoptimized_frame_offset_in_bytes()));
  __ addq(rsp, rcx);

  // Pop I2C adapter frame, if any (int)
  __ movl(rcx, Address(rdi,
                       Deoptimization::UnrollBlock::
                       adapter_size_offset_in_bytes()));
  __ addq(rsp, rcx);

  // rsp should be pointing at the return address to the caller (4)

  // Load address of array of frame pcs into rcx (address*)
  __ movq(rcx, 
          Address(rdi,
                  Deoptimization::UnrollBlock::frame_pcs_offset_in_bytes()));

  // Trash the return pc
  __ addq(rsp, wordSize);

  // Load address of array of frame sizes into rsi (intptr_t*)
  __ movq(rsi, Address(rdi,
                       Deoptimization::UnrollBlock::
                       frame_sizes_offset_in_bytes()));

  // Counter
  __ movl(rdx, Address(rdi, 
                       Deoptimization::UnrollBlock::
                       number_of_frames_offset_in_bytes())); // (int)

  // Pick up the initial fp we should save
  __ movq(rbp, 
          Address(rdi, 
                  Deoptimization::UnrollBlock::initial_fp_offset_in_bytes()));

  Label adapter_done;

  // Do we need an adapter frame?
  __ cmpl(Address(rdi, 
                  Deoptimization::UnrollBlock::new_adapter_offset_in_bytes()),
          0); // (int)
  __ jcc(Assembler::zero, adapter_done);

  // Push a c2i adapter frame
  __ pushq(Address(rcx));     // Push new pc
  __ movq(rbx, Address(rsi)); // Get framesize (includes return address)
  __ subq(rbx, wordSize);     // Account for return address
  __ decl(rdx);               // Count the frame
  __ subq(rsp, rbx);          // Allocate adapter frame
  __ addq(rsi, wordSize);     // Bump array pointer (sizes)
  __ addq(rcx, wordSize);     // Bump array pointer (pcs)
  __ bind(adapter_done);

  // Now adjust the caller's stack to make up for the extra locals but
  // record the original sp so that we can save it in the skeletal
  // interpreter frame and the stack walking of interpreter_sender
  // will get the unextended sp value and not the "real" sp value.

  const Register sender_sp = r8;

  __ movq(sender_sp, rsp);
  __ movl(rbx, Address(rdi, 
                       Deoptimization::UnrollBlock::
                       caller_adjustment_offset_in_bytes())); // (int)
  __ subq(rsp, rbx);

  // Push interpreter frames in a loop
  Label loop;
  __ bind(loop);
  __ movq(rbx, Address(rsi)); // Load frame size
  __ subq(rbx, 2 * wordSize); // We'll push pc and rbp by hand
  __ pushq(Address(rcx));     // Save return address
  __ enter();                 // Save old & set new rbp
  __ subq(rsp, rbx);          // Prolog
  __ movq(Address(rbp, frame::interpreter_frame_sender_sp_offset * wordSize),
          sender_sp);         // Make it walkable
  __ movq(sender_sp, rsp);    // Pass sender_sp to next frame
  __ addq(rsi, wordSize);     // Bump array pointer (sizes)
  __ addq(rcx, wordSize);     // Bump array pointer (pcs)
  __ decl(rdx);               // Decrement counter
  __ jcc(Assembler::notZero, loop);
  __ pushq(Address(rcx));     // Save final return address

  // Re-push self-frame
  __ enter();                 // Save old & set new rbp
  __ subq(rsp, (framesize - 4) << LogBytesPerInt); 
                              // Prolog

  __ set_last_Java_frame(noreg, noreg, rbp, NULL);

  // Call C code.  Need thread but NOT official VM entry
  // crud.  We cannot block on this call, no GC can happen.  Call should
  // restore return values to their stack-slots with the new SP.
  // Thread is in rdi already.
  //
  // BasicType unpack_frames(JavaThread* thread, int exec_mode);

  __ movq(rarg0, r15_thread);
  __ movl(rarg1, Deoptimization::Unpack_uncommon_trap);
  __ call(CAST_FROM_FN_PTR(address, Deoptimization::unpack_frames),
          relocInfo::runtime_call_type);

  // Set an oopmap for the call site
  oop_maps->add_gc_map(__ pc() - start, true, new OopMap(framesize, 0));

  __ reset_last_Java_frame(noreg, false);

  // Pop self-frame.
  __ leave();                 // Epilog

  // Jump to interpreter
  __ ret(0);

  // Make sure all code is generated
  masm->flush();

  _uncommon_trap_blob = UncommonTrapBlob::create(buffer, oop_maps, 
                                                 framesize >> 1);
}


//------------------------------fill_in_exception_blob-------------------------
// Ought to generate an ideal graph & compile, but here's some Intel ASM
// instead.  Sigh, Cliff.
int handle_exception_deopt_offset = 0;
int handle_exception_deopt_exception_offset = 0;
int handle_exception_deopt_offset_2 = 0;
int handle_exception_deopt_exception_offset_2 = 0;
int handle_exception_call_pc_offset = 0;

void OptoRuntime::fill_in_exception_blob()
{
  // Offsets are for compiler stack slots, which are jints.
  enum layout {
    r13_off = frame::arg_reg_save_area_bytes/BytesPerInt,
                r13_off2,
    r14_off,    r14_off2,
    rbp_off,    rbp_off2,
    return_off, return_off2,
    framesize 
  };
  assert(framesize % 4 == 0, "sp not 16-byte aligned");

   // Patch blob
  assert(handle_exception_stub() != NULL, 
         "exception stub must have been generated");
  assert(handle_exception_call_pc_offset != 0, "");

  // Set an oopmap for the call site.  This oopmap will only be used if we
  // are unwinding the stack.  Hence, all locations will be dead.
  // Callee-saved registers will be the same as the frame above (i.e.,
  // handle_exception_stub), since they were restored when we got the
  // exception.
  ResourceMark rm;
  OopMapSet* oop_maps = new OopMapSet();
  OopMap* map = new OopMap(framesize, 0);        

  map->set_callee_saved(SharedInfo::stack2reg(r13_off),
                        framesize, 0,
                        OptoReg::Name(R13_num));
  map->set_callee_saved(SharedInfo::stack2reg(r13_off2),
                        framesize, 0,
                        OptoReg::Name(R13_H_num));
  map->set_callee_saved(SharedInfo::stack2reg(r14_off),
                        framesize, 0,
                        OptoReg::Name(R14_num));
  map->set_callee_saved(SharedInfo::stack2reg(r14_off2),
                        framesize, 0,
                        OptoReg::Name(R14_H_num));
  map->set_callee_saved(SharedInfo::stack2reg(rbp_off),
                        framesize, 0, 
                        OptoReg::Name(RBP_num));
  map->set_callee_saved(SharedInfo::stack2reg(rbp_off2),
                        framesize, 0, 
                        OptoReg::Name(RBP_H_num));

  oop_maps->add_gc_map(handle_exception_call_pc_offset, true, map);
  exception_blob()->set_oop_maps(oop_maps);
}

//------------------------------setup_exception_blob---------------------------
// creates exception blob at the end
// Using exception blob, this code is jumped from a compiled method.
// (see emit_exception_handler in amd64.ad file)
// It is also the default exception destination for c2i and i2c adapters
// 
// Given an exception pc at a call, we may unwind the stack and jump to
// caller's exception handler. We may also continue into the handler
// This code is entered with a jmp.
// 
// Arguments:
//   rax: exception oop
//   rdx: exception pc
//
// Results:
//   rax: exception oop
//   rdx: exception pc in caller or ???
//   destination: exception handler of caller
// 
// Note: the exception pc MUST be at a call (precise debug information)
//       Registers rax, rdx, rcx, rsi, rdi, r8-r11 are not callee saved.
//

void OptoRuntime::setup_exception_blob()
{
  assert(!OptoRuntime::is_callee_saved_register(RDX_num), "");
  assert(!OptoRuntime::is_callee_saved_register(RAX_num), "");
  assert(!OptoRuntime::is_callee_saved_register(RCX_num), "");

  // Offsets are for compiler stack slots, which are jints.
  enum layout {
    r13_off = frame::arg_reg_save_area_bytes/BytesPerInt,
                r13_off2,
    r14_off,    r14_off2,
    rbp_off,    rbp_off2,
    return_off, return_off2,
    framesize 
  };
  assert(framesize % 4 == 0, "sp not 16-byte aligned");

  // Allocate space for the code
  ResourceMark rm;
  // Setup code generation tools  
  CodeBuffer* buffer = new CodeBuffer(2048, 1024, 0, 0, 0, false);
  MacroAssembler* masm = new MacroAssembler(buffer);


  Label exception_handler_found;

  address start = __ pc();  

  // Exception pc is 'return address' for stack walker
  __ pushq(rdx);
  __ subq(rsp, return_off << LogBytesPerInt); // Prolog

  // Save callee-saved registers.  See amd64.ad.
  __ movq(Address(rsp, rbp_off << LogBytesPerInt), rbp);
  __ movq(Address(rsp, r13_off << LogBytesPerInt), r13);
  __ movq(Address(rsp, r14_off << LogBytesPerInt), r14);
          
  // Store exception in Thread object. We cannot pass any arguments to the
  // handle_exception call, since we do not want to make any assumption
  // about the size of the frame where the exception happened in.
  // rarg0 is either rdi (Linux) or rcx (Windows).
  __ movq(Address(r15_thread, JavaThread::exception_oop_offset()),rax);
  __ movq(Address(r15_thread, JavaThread::exception_pc_offset()), rdx);

  // This call does all the hard work.  It checks if an exception handler
  // exists in the method.  
  // If so, it returns the handler address.
  // If not, it prepares for stack-unwinding, restoring the callee-save 
  // registers of the frame being removed.
  //
  // address OptoRuntime::handle_exception_C(JavaThread* thread)

  __ set_last_Java_frame(noreg, noreg, noreg, NULL);
  __ movq(rarg0, r15_thread);
  __ call(CAST_FROM_FN_PTR(address, OptoRuntime::handle_exception_C), 
          relocInfo::runtime_call_type);

  handle_exception_call_pc_offset = __ pc() - start;

  __ reset_last_Java_frame(noreg, false);

  // Restore callee-saved registers
  __ movq(rbp, Address(rsp, rbp_off << LogBytesPerInt));
  __ movq(r13, Address(rsp, r13_off << LogBytesPerInt));
  __ movq(r14, Address(rsp, r14_off << LogBytesPerInt));

  __ addq(rsp, return_off << LogBytesPerInt); // Epilog
  __ popq(rdx);                  // No need for exception pc anymore

  // rax: 
  // NULL if no exception handler for given <exception oop/exception pc> found
  __ testq(rax, rax);
  __ jcc(Assembler::notZero, exception_handler_found);    

  // No exception handler found. We unwind stack one level.
  // All information is stored in thread.
  // thread->exception_oop
  // thread->exception_handler_pc
  // thread->exception_pc;
  // thread->exception_stack_size

  // The callee-saved registers contain the addresses of their
  // values (see pd_unwind_stack below).  Load the values into the
  // callee saved registers (only if register is non-zero).

  Label skip_rbp, skip_r13, skip_r14;
  __ testq(rbp, rbp);
  __ jcc(Assembler::zero, skip_rbp);
  __ movq(rbp, Address(rbp));
  __ bind(skip_rbp);

  __ testq(r13, r13);
  __ jcc(Assembler::zero, skip_r13);
  __ movq(r13, Address(r13));
  __ bind(skip_r13);

  __ testq(r14, r14);
  __ jcc(Assembler::zero, skip_r14);
  __ movq(r14, Address(r14));
  __ bind(skip_r14);

  // Now we continue at JavaThread::exception_handler_pc_offset_in_bytes.
  // The caller frame state is defined by its return address.

  __ movq(rdx, Address(r15_thread, JavaThread::exception_pc_offset()));         // address
  __ movl(rax, Address(r15_thread, JavaThread::exception_stack_size_offset())); // int

  // rax: stack-size
  // rdx: exception-pc
              
  // Remove frame (including return address)
  __ addq(rsp, rax);

  // Return to the handler
  __ pushq(Address(r15_thread, JavaThread::exception_handler_pc_offset()));

  // Load exception oop
  __ movq(rax, Address(r15_thread, JavaThread::exception_oop_offset()));
#ifdef ASSERT
  __ movq(Address(r15_thread, JavaThread::exception_oop_offset()), 0);
  __ movq(Address(r15_thread, JavaThread::exception_handler_pc_offset()), 0);
  __ movq(Address(r15_thread, JavaThread::exception_pc_offset()), 0);
#endif

  // rax: exception oop
  // rdx: exception pc
  // Return to handler
 
  __ ret(0);

  // Exception handler found.
  __ bind (exception_handler_found);

  Label deoptL_2, endL_2;

  // We have a handler in rax (could be deopt blob).
  __ movq(r8, rax);

  // Get the exception oop
  __ movq(rax, Address(r15_thread, JavaThread::exception_oop_offset()));
  // Get the exception pc in case we are deoptimized
  __ movq(rdx, Address(r15_thread, JavaThread::exception_pc_offset()));
#ifdef ASSERT
  __ movq(Address(r15_thread, JavaThread::exception_handler_pc_offset()), 0);
  __ movq(Address(r15_thread, JavaThread::exception_pc_offset()), 0); 
  __ movq(Address(r15_thread, JavaThread::exception_oop_offset()), 0);
#endif

  // rax: exception oop
  // r8:  exception handler
  // rdx: exception pc
  // Jump to handler

  __ jmp(r8);

  // Make sure all code is generated
  masm->flush();  

  // Set exception blob
  OopMapSet* oop_maps = NULL; // Will be set later; currently the
                              // register stuff is not yet
                              // initialized!
  _exception_blob = ExceptionBlob::create(buffer, oop_maps, framesize >> 1);
}

//------------------------------generate_illegal_instruction_handler_blob------
//
// Generate a special Compile2Runtime blob that saves all registers, 
// and setup oopmap.
//
SafepointBlob* OptoRuntime::generate_handler_blob(address call_ptr, bool cause_return)
{
  assert(StubRoutines::forward_exception_entry() != NULL,
         "must be generated before");  

  ResourceMark rm;
  OopMapSet *oop_maps = new OopMapSet();
  OopMap* map;

  // Allocate space for the code.  Setup code generation tools.
  CodeBuffer*   buffer = new CodeBuffer(2048, 1024, 0, 0, 0, false);
  MacroAssembler* masm = new MacroAssembler(buffer);
  
  address start   = __ pc();  
  address call_pc = NULL;  
  int frame_size_in_words;

  // Make room for return address (or push it again)
  if (!cause_return)
    __ pushq(rbx);

  // Save registers, fpu state, and flags  
  map = RegisterSaver::save_live_registers(masm, 0, &frame_size_in_words);
  
  // The following is basically a call_VM.  However, we need the precise
  // address of the call in order to generate an oopmap. Hence, we do all the
  // work outselves.

  __ set_last_Java_frame(noreg, noreg, noreg, NULL);

  // Do the call
  __ movq(rarg0, r15_thread);
  __ call(call_ptr, relocInfo::runtime_call_type);

  // Set an oopmap for the call site.  This oopmap will map all
  // oop-registers and debug-info registers as callee-saved.  This
  // will allow deoptimization at this safepoint to find all possible
  // debug-info recordings, as well as let GC find all oops.

  oop_maps->add_gc_map( __ pc() - start, true, map);

  Label noException;

  __ reset_last_Java_frame(noreg, false);

  __ cmpq(Address(r15_thread, Thread::pending_exception_offset()), NULL);
  __ jcc(Assembler::equal, noException);

  // Exception pending

  RegisterSaver::restore_live_registers(masm);

  __ jmp(StubRoutines::forward_exception_entry(), relocInfo::runtime_call_type);

  // No exception case
  Label continueL;
  __ bind(noException);
  __ testl(rax, rax); // (int) Continue call?
  __ jcc(Assembler::negative, continueL);

  // Normal exit, restore registers and exit.
  RegisterSaver::restore_live_registers(masm);

  __ ret(0);

  // We have deoptimized at a blocked call, we may not reexecute the
  // instruction as we would skip the call in interpreter; therefore
  // execute the destination of the call; the destination is valid
  // because the receiver was already consumed.
  //
  // rax holds the destination of the call.
  __ bind(continueL);

  // Move rax (continuation address) to where r11 will be loaded from.
  // Need a register that's not callee-saved and not an argument register.
  __ movq(Address(rsp, RegisterSaver::r11_offset_in_bytes()), rax);

  RegisterSaver::restore_live_registers(masm);

  // Everything is just like we were at entry (except r11)
  // original return address is still there too (we deopt on return)
  // just continue with the call.
  __ jmp(r11);

  // Make sure all code is generated
  masm->flush();  

  // Fill-out other meta info
  return SafepointBlob::create(buffer, oop_maps, frame_size_in_words);    
}


//------------------------------generate_illegal_instruction_handler_blob------
void OptoRuntime::generate_illegal_instruction_handler_blob()
{
  _illegal_instruction_handler_blob =
    generate_handler_blob(CAST_FROM_FN_PTR(address, 
                  SafepointSynchronize::handle_illegal_instruction_exception), false);
}

//------------------------------generate_polling_page_safepoint_handler_blob-------------
void OptoRuntime::generate_polling_page_safepoint_handler_blob()
{
  _polling_page_safepoint_handler_blob =
    generate_handler_blob(CAST_FROM_FN_PTR(address,
                         SafepointSynchronize::handle_polling_page_exception), false);
}


//------------------------------generate_polling_page_return_handler_blob-------------
void OptoRuntime::generate_polling_page_return_handler_blob()
{
  _polling_page_return_handler_blob =
    generate_handler_blob(CAST_FROM_FN_PTR(address,
                         SafepointSynchronize::handle_polling_page_exception), true);
}


//------------------------------pd_unwind_stack--------------------------------
void OptoRuntime::pd_unwind_stack(JavaThread* thread, 
                                  frame fr, 
                                  RegisterMap* reg_map)
{
  // Update callee-saved registers across frame
  intptr_t* reg_locs[REG_COUNT];
  // Copy values of all callee-saved registers out of frame
  for (int i = 0; i < REG_COUNT; i++) {
    intptr_t* loc = (intptr_t*) reg_map->location(VMReg::Name(i));    
    reg_locs[i] = loc;
  }
  
  // Find oopmap for stub, and update it stack with callee-saved
  // reg. info for next frame.
  frame stub_frame = thread->last_frame();
  CodeBlob* blob = CodeCache::find_blob(stub_frame.pc());
  assert(blob->is_exception_stub(), "sanity check");
  OopMap* map = blob->oop_maps()->singular_oop_map();
  // Update callee-saved registers in stub
  OopMapValue omv;
  for (OopMapStream oms(map, OopMapValue::callee_saved_value); 
       !oms.is_done(); 
       oms.next()) {
    omv = oms.current();
    assert(omv.is_stack_loc(), "sanity check");
    intptr_t* location = 
      (intptr_t*) stub_frame.oopmapreg_to_location(omv.reg(), NULL);
    VMReg::Name reg = omv.content_reg();
    assert(reg >= 0 && reg < REG_COUNT, "reg out of range");
    intptr_t** location2 = (intptr_t**) location;
    *location2 = reg_locs[reg];
  }
}
