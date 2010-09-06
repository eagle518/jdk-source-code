#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)runtime_i486.cpp	1.85 03/12/23 16:36:25 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_runtime_i486.cpp.incl"

DeoptimizationBlob *SharedRuntime::_deopt_blob;
UncommonTrapBlob   *OptoRuntime::_uncommon_trap_blob;
ExceptionBlob      *OptoRuntime::_exception_blob;
SafepointBlob      *OptoRuntime::_illegal_instruction_handler_blob;
SafepointBlob      *OptoRuntime::_polling_page_safepoint_handler_blob;
SafepointBlob      *OptoRuntime::_polling_page_return_handler_blob;

#define __ masm->

class RegisterSaver {
  enum { FPU_regs_used_by_C2 = 7 /*for the FPU stack*/+8/*eight more for XMM registers*/ };
  // Capture info about frame layout
  enum layout { 
                fpu_state_off = 0,
                fpu_state_end = fpu_state_off+FPUStateSizeInWords-1,
		// server compiler can pass debug info in
		// any of the first 7 FPU registers
		// During deopt fpr1_off really holds a potential
		// floating point result and remain 6 are junk
		// During a safepoint poll all 7 may be valid

                fpr1_off, fpr1H_off,
                fpr2_off, fpr2H_off,
                fpr3_off, fpr3H_off,
                fpr4_off, fpr4H_off,
                fpr5_off, fpr5H_off,
                fpr6_off, fpr6H_off,
                fpr7_off, fpr7H_off, // FPU_regs_used_by_C2 = 7

                xmm0_off, xmm0H_off, 
                xmm1_off, xmm1H_off, 
                xmm2_off, xmm2H_off, 
                xmm3_off, xmm3H_off, 
                xmm4_off, xmm4H_off, 
                xmm5_off, xmm5H_off, 
                xmm6_off, xmm6H_off, 
                xmm7_off, xmm7H_off, // FPU_regs_used_by_C2 += 8
                flags_off,
                edi_off,         
                esi_off,
                ignore_off,  // extra copy of ebp
                esp_off,
                ebx_off,
                edx_off,
                ecx_off,
                eax_off,            
		ebp_off,         // copy of ebp we will restore
                return_off,      // slot for return address
	        reg_save_size };

  
  public: 

  static OopMap* save_live_registers(MacroAssembler* masm, int additional_frame_words, int* total_frame_words);
  static void restore_live_registers(MacroAssembler* masm);

  // Offsets into the register save area
  // Used by deoptimization when it is managing result register
  // values on its own

  static int eaxOffset(void) { return eax_off; }
  static int edxOffset(void) { return edx_off; }
  static int ebxOffset(void) { return ebx_off; }
  static int xmm0Offset(void) { return xmm0_off; }
  // This really returns a slot in the fp save area, which one is not important
  static int fpResultOffset(void) { return fpr1_off; }

  // During deoptimization only the result register need to be restored
  // all the other values have already been extracted.

  static void restore_result_registers(MacroAssembler* masm);

};

OopMap* RegisterSaver::save_live_registers(MacroAssembler* masm, int additional_frame_words, int* total_frame_words) {

  int frame_size_in_bytes =  (reg_save_size + additional_frame_words) * wordSize;
  int frame_words = frame_size_in_bytes / wordSize;
  *total_frame_words = frame_words;

  assert(FPUStateSizeInWords == 27, "update stack layout");

  // save registers, fpu state, and flags  
  // We assume caller has already has return address slot on the stack
  // We push epb twice in this sequence because we want the real ebp
  // to be under the return like a normal enter and we want to use pushad
  // We push by hand instead of pusing push
  __ pushl(ebp);
  __ pushad();
  __ pushfd();        
  __ subl(esp,FPU_regs_used_by_C2*sizeof(jdouble)); // Push FPU registers space
  __ push_FPU_state();          // Save FPU state & init
  __ frstor(Address(esp));      // Restore state (but keep stack copy)

  // Save the FPU registers in de-opt-able form 

  __ fstp_d(Address(esp, fpr1_off*wordSize)); // FPR1
  __ fstp_d(Address(esp, fpr2_off*wordSize)); // FPR2
  __ fstp_d(Address(esp, fpr3_off*wordSize)); // FPR3
  __ fstp_d(Address(esp, fpr4_off*wordSize)); // FPR4
  __ fstp_d(Address(esp, fpr5_off*wordSize)); // FPR5
  __ fstp_d(Address(esp, fpr6_off*wordSize)); // FPR6
  __ fstp_d(Address(esp, fpr7_off*wordSize)); // FPR7
  __ finit();                   // Reset FPU state for following C code

  if( UseSSE == 1 ) {           // Save the XMM state
    __ movss(Address(esp,xmm0_off*wordSize),xmm0);
    __ movss(Address(esp,xmm1_off*wordSize),xmm1);
    __ movss(Address(esp,xmm2_off*wordSize),xmm2);
    __ movss(Address(esp,xmm3_off*wordSize),xmm3);
    __ movss(Address(esp,xmm4_off*wordSize),xmm4);
    __ movss(Address(esp,xmm5_off*wordSize),xmm5);
    __ movss(Address(esp,xmm6_off*wordSize),xmm6);
    __ movss(Address(esp,xmm7_off*wordSize),xmm7);
  } else if( UseSSE == 2 ) {
    __ movsd(Address(esp,xmm0_off*wordSize),xmm0);
    __ movsd(Address(esp,xmm1_off*wordSize),xmm1);
    __ movsd(Address(esp,xmm2_off*wordSize),xmm2);
    __ movsd(Address(esp,xmm3_off*wordSize),xmm3);
    __ movsd(Address(esp,xmm4_off*wordSize),xmm4);
    __ movsd(Address(esp,xmm5_off*wordSize),xmm5);
    __ movsd(Address(esp,xmm6_off*wordSize),xmm6);
    __ movsd(Address(esp,xmm7_off*wordSize),xmm7);
  }

  // Set an oopmap for the call site.  This oopmap will map all
  // oop-registers and debug-info registers as callee-saved.  This
  // will allow deoptimization at this safepoint to find all possible
  // debug-info recordings, as well as let GC find all oops.

  OopMapSet *oop_maps = new OopMapSet();
  OopMap* map =  new OopMap( frame_words, 0 );  
  map->set_callee_saved(SharedInfo::stack2reg(  eax_off + additional_frame_words), frame_words, 0, OptoReg::Name(  EAX_num));
  map->set_callee_saved(SharedInfo::stack2reg(  ecx_off + additional_frame_words), frame_words, 0, OptoReg::Name(  ECX_num));
  map->set_callee_saved(SharedInfo::stack2reg(  edx_off + additional_frame_words), frame_words, 0, OptoReg::Name(  EDX_num));
  map->set_callee_saved(SharedInfo::stack2reg(  ebx_off + additional_frame_words), frame_words, 0, OptoReg::Name(  EBX_num));  
  map->set_callee_saved(SharedInfo::stack2reg(  ebp_off + additional_frame_words), frame_words, 0, OptoReg::Name(  EBP_num));
  map->set_callee_saved(SharedInfo::stack2reg(  esi_off + additional_frame_words), frame_words, 0, OptoReg::Name(  ESI_num));
  map->set_callee_saved(SharedInfo::stack2reg(  edi_off + additional_frame_words), frame_words, 0, OptoReg::Name(  EDI_num));    
  map->set_callee_saved(SharedInfo::stack2reg( fpr1_off + additional_frame_words), frame_words, 0, OptoReg::Name(FPR1L_num));    
  map->set_callee_saved(SharedInfo::stack2reg( fpr2_off + additional_frame_words), frame_words, 0, OptoReg::Name(FPR2L_num)); 
  map->set_callee_saved(SharedInfo::stack2reg( fpr3_off + additional_frame_words), frame_words, 0, OptoReg::Name(FPR3L_num));    
  map->set_callee_saved(SharedInfo::stack2reg( fpr4_off + additional_frame_words), frame_words, 0, OptoReg::Name(FPR4L_num));    
  map->set_callee_saved(SharedInfo::stack2reg( fpr5_off + additional_frame_words), frame_words, 0, OptoReg::Name(FPR5L_num));    
  map->set_callee_saved(SharedInfo::stack2reg( fpr6_off + additional_frame_words), frame_words, 0, OptoReg::Name(FPR6L_num));    
  map->set_callee_saved(SharedInfo::stack2reg( fpr7_off + additional_frame_words), frame_words, 0, OptoReg::Name(FPR7L_num));    
  map->set_callee_saved(SharedInfo::stack2reg(fpr1H_off + additional_frame_words), frame_words, 0, OptoReg::Name(FPR1H_num));    
  map->set_callee_saved(SharedInfo::stack2reg(fpr2H_off + additional_frame_words), frame_words, 0, OptoReg::Name(FPR2H_num)); 
  map->set_callee_saved(SharedInfo::stack2reg(fpr3H_off + additional_frame_words), frame_words, 0, OptoReg::Name(FPR3H_num));    
  map->set_callee_saved(SharedInfo::stack2reg(fpr4H_off + additional_frame_words), frame_words, 0, OptoReg::Name(FPR4H_num));    
  map->set_callee_saved(SharedInfo::stack2reg(fpr5H_off + additional_frame_words), frame_words, 0, OptoReg::Name(FPR5H_num));    
  map->set_callee_saved(SharedInfo::stack2reg(fpr6H_off + additional_frame_words), frame_words, 0, OptoReg::Name(FPR6H_num));    
  map->set_callee_saved(SharedInfo::stack2reg(fpr7H_off + additional_frame_words), frame_words, 0, OptoReg::Name(FPR7H_num));    
  map->set_callee_saved(SharedInfo::stack2reg( xmm0_off + additional_frame_words), frame_words, 0, OptoReg::Name(XMM0a_num));    
  map->set_callee_saved(SharedInfo::stack2reg( xmm1_off + additional_frame_words), frame_words, 0, OptoReg::Name(XMM1a_num));    
  map->set_callee_saved(SharedInfo::stack2reg( xmm2_off + additional_frame_words), frame_words, 0, OptoReg::Name(XMM2a_num));    
  map->set_callee_saved(SharedInfo::stack2reg( xmm3_off + additional_frame_words), frame_words, 0, OptoReg::Name(XMM3a_num));    
  map->set_callee_saved(SharedInfo::stack2reg( xmm4_off + additional_frame_words), frame_words, 0, OptoReg::Name(XMM4a_num));    
  map->set_callee_saved(SharedInfo::stack2reg( xmm5_off + additional_frame_words), frame_words, 0, OptoReg::Name(XMM5a_num));    
  map->set_callee_saved(SharedInfo::stack2reg( xmm6_off + additional_frame_words), frame_words, 0, OptoReg::Name(XMM6a_num));    
  map->set_callee_saved(SharedInfo::stack2reg( xmm7_off + additional_frame_words), frame_words, 0, OptoReg::Name(XMM7a_num));    
  map->set_callee_saved(SharedInfo::stack2reg(xmm0H_off + additional_frame_words), frame_words, 0, OptoReg::Name(XMM0b_num));    
  map->set_callee_saved(SharedInfo::stack2reg(xmm1H_off + additional_frame_words), frame_words, 0, OptoReg::Name(XMM1b_num));    
  map->set_callee_saved(SharedInfo::stack2reg(xmm2H_off + additional_frame_words), frame_words, 0, OptoReg::Name(XMM2b_num));    
  map->set_callee_saved(SharedInfo::stack2reg(xmm3H_off + additional_frame_words), frame_words, 0, OptoReg::Name(XMM3b_num));    
  map->set_callee_saved(SharedInfo::stack2reg(xmm4H_off + additional_frame_words), frame_words, 0, OptoReg::Name(XMM4b_num));    
  map->set_callee_saved(SharedInfo::stack2reg(xmm5H_off + additional_frame_words), frame_words, 0, OptoReg::Name(XMM5b_num));    
  map->set_callee_saved(SharedInfo::stack2reg(xmm6H_off + additional_frame_words), frame_words, 0, OptoReg::Name(XMM6b_num));    
  map->set_callee_saved(SharedInfo::stack2reg(xmm7H_off + additional_frame_words), frame_words, 0, OptoReg::Name(XMM7b_num));    

  return map;

}

void RegisterSaver::restore_live_registers(MacroAssembler* masm) {

  // Recover XMM & FPU state
  if( UseSSE == 1 ) {
    __ movss(xmm0,Address(esp,xmm0_off*wordSize));
    __ movss(xmm1,Address(esp,xmm1_off*wordSize));
    __ movss(xmm2,Address(esp,xmm2_off*wordSize));
    __ movss(xmm3,Address(esp,xmm3_off*wordSize));
    __ movss(xmm4,Address(esp,xmm4_off*wordSize));
    __ movss(xmm5,Address(esp,xmm5_off*wordSize));
    __ movss(xmm6,Address(esp,xmm6_off*wordSize));
    __ movss(xmm7,Address(esp,xmm7_off*wordSize));
  } else if( UseSSE == 2 ) {
    __ movsd(xmm0,Address(esp,xmm0_off*wordSize));
    __ movsd(xmm1,Address(esp,xmm1_off*wordSize));
    __ movsd(xmm2,Address(esp,xmm2_off*wordSize));
    __ movsd(xmm3,Address(esp,xmm3_off*wordSize));
    __ movsd(xmm4,Address(esp,xmm4_off*wordSize));
    __ movsd(xmm5,Address(esp,xmm5_off*wordSize));
    __ movsd(xmm6,Address(esp,xmm6_off*wordSize));
    __ movsd(xmm7,Address(esp,xmm7_off*wordSize));
  }
  __ pop_FPU_state();
  __ addl(esp,FPU_regs_used_by_C2*sizeof(jdouble)); // Pop FPU registers

  __ popfd();
  __ popad();
  // Get the ebp described in the oopMap
  __ popl(ebp);

}

void RegisterSaver::restore_result_registers(MacroAssembler* masm) {

  // Just restore result register. Only used by deoptimization. By
  // now any callee save register that needs to be restore to a c2
  // caller of the deoptee has been extracted into the vframeArray
  // and will be stuffed into the c2i adapter we create for later
  // restoration so only result registers need to be restored here.
  // 

  __ frstor(Address(esp));      // Restore fpu state 

  // Recover XMM & FPU state
  if( UseSSE == 1 ) {
    __ movss(xmm0, Address(esp, xmm0_off*wordSize));
  } else if( UseSSE == 2 ) {
    __ movsd(xmm0, Address(esp, xmm0_off*wordSize));
  }
  __ fld_d(Address(esp, fpr1_off*wordSize));
  __ movl(eax, Address(esp, eax_off*wordSize));
  __ movl(edx, Address(esp, edx_off*wordSize));
  // Pop all of the register save are off the stack except the return address
  __ addl(esp, return_off * wordSize); 
}

//------------------------------generate_deopt_blob----------------------------
// Ought to generate an ideal graph & compile, but here's some Intel ASM
// instead.  Sigh, Cliff.
void SharedRuntime::generate_deopt_blob() {
  // allocate space for the code
  ResourceMark rm;
  // setup code generation tools
  CodeBuffer*   buffer = new CodeBuffer(1024, 1024, 0, 0, 0, false);
  MacroAssembler* masm = new MacroAssembler(buffer);
  int frame_size_in_words;
  OopMap* map = NULL;
  // Account for the extra args we place on the stack
  // by the time we call fetch_unroll_info
  const int additional_words = 2; // deopt kind, thread

  OopMapSet *oop_maps = new OopMapSet();

  // -------------
  // This code enters when returning to a de-optimized nmethod.  A return
  // address has been pushed on the the stack, and return values are in
  // registers. 
  // If we are doing a normal deopt then we were called from the patched 
  // nmethod from the point we returned to the nmethod. So the return
  // address on the stack is wrong by NativeCall::instruction_size
  // We will adjust the value to it looks like we have the original return
  // address on the stack (like when we eagerly deoptimized).
  // In the case of an exception pending with deoptimized then we enter
  // with a return address on the stack that points after the call we patched
  // into the exception handler. We have the following register state:
  //    eax: exception
  //    ebx: exception handler
  //    edx: throwing pc
  // So in this case we simply jam edx into the useless return address and
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
  __ subl(Address(esp, 0), NativeCall::instruction_size);

  // Save everything in sight.

  map = RegisterSaver::save_live_registers(masm, additional_words, &frame_size_in_words);
  // Normal deoptimization
  __ pushl(Deoptimization::Unpack_deopt);
  __ jmp(cont);
  
  int exception_offset = __ pc() - start;

  // Prolog for exception case

  // push throwin pc as return address

  __ pushl(edx);
  // Overwrite the useless return address with throwing pc

  // __ movl(Address(esp, 0), edx);

  // Save everything in sight.

  map = RegisterSaver::save_live_registers(masm, additional_words, &frame_size_in_words);

  // exception deoptimization
  __ pushl(Deoptimization::Unpack_exception); 

  __ bind(cont);

  // Compiled code leaves the floating point stack dirty, empty it.
  __ empty_FPU_stack();


  // Call C code.  Need thread and this frame, but NOT official VM entry
  // crud.  We cannot block on this call, no GC can happen.  
  __ get_thread(ecx);
  __ pushl(ecx);
  // fetch_unroll_info needs to call last_java_frame()
  __ set_last_Java_frame(ecx, noreg, noreg, NULL);
#ifdef ASSERT
  { Label L;
    __ cmpl(Address(ecx,
		    JavaThread::last_Java_fp_offset()),
		    (intptr_t) 0);
    __ jcc(Assembler::equal, L);
    __ stop("OptoRuntime::generate_deopt_blob: last_Java_fp not cleared");
    __ bind(L);
  }
#endif /* ASSERT */
  __ call( CAST_FROM_FN_PTR(address, Deoptimization::fetch_unroll_info), relocInfo::runtime_call_type );

  // Need to have an oopmap that tells fetch_unroll_info where to
  // find any register it might need.

  oop_maps->add_gc_map( __ pc()-start, true, map);

  // Discard arg to fetch_unroll_info
  __ popl(ecx);

  __ get_thread(ecx);
  __ reset_last_Java_frame(ecx, false);

  // Load UnrollBlock into EDI
  __ movl(edi, eax);

  // Move the unpack kind to a safe place in the UnrollBlock because
  // we are very short of registers

  Address unpack_kind(edi, Deoptimization::UnrollBlock::unpack_kind_offset_in_bytes());
  // retrieve the deopt kind from where we left it.
  __ popl(eax);
  __ movl(unpack_kind, eax);                      // save the unpack_kind value

  // Stack is back to only having register save data on the stack.
  // Now restore the result registers. Everything else is either dead or captured
  // in the vframeArray.

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
  // Note: by leaving the return address of sefl-frame on the stack
  // and using the size of frame(s) 2 & 3 to adjust the stack
  // when we are done the return to frame 4 will still be on the stack.

  // Pop deoptimized frame
  __ addl(esp,Address(edi,Deoptimization::UnrollBlock::size_of_deoptimized_frame_offset_in_bytes()));

  // Pop I2C adapter frame, if any
  __ addl(esp,Address(edi,Deoptimization::UnrollBlock::adapter_size_offset_in_bytes()));

  // sp should be pointing at the return address to the caller (4)

  // Load array of frame pcs into ECX
  __ movl(ecx,Address(edi,Deoptimization::UnrollBlock::frame_pcs_offset_in_bytes()));

  __ popl(esi); // trash the old pc

  // Load array of frame sizes into ESI
  __ movl(esi,Address(edi,Deoptimization::UnrollBlock::frame_sizes_offset_in_bytes()));

  Address counter(edi, Deoptimization::UnrollBlock::counter_temp_offset_in_bytes());

  __ movl(ebx, Address(edi, Deoptimization::UnrollBlock::number_of_frames_offset_in_bytes()));
  __ movl(counter, ebx);

  // Pick up the initial fp we should save
  __ movl(ebp, Address(edi, Deoptimization::UnrollBlock::initial_fp_offset_in_bytes()));


  Label adapter_done;

  // Do we need an adapter frame?

  __ cmpl(Address(edi, Deoptimization::UnrollBlock::new_adapter_offset_in_bytes()), 0);
  __ jcc (Assembler::zero, adapter_done);

  // Push a c2i adapter frame
  __ pushl(Address(ecx));                             // push new pc
  __ movl(ebx, Address(esi));                         // get framesize (includes return address)
  __ subl(ebx, wordSize);                             // account for return address
  __ decl(counter);                                   // count the frame
  __ addl(esi, 4);                                    // Bump array pointer (sizes)
  __ addl(ecx, 4);                                    // Bump array pointer (pcs)
  __ subl(esp, ebx);                                  // allocate adapter frame
  __ bind(adapter_done);

  // Now adjust the caller's stack to make up for the extra locals
  // but record the original sp so that we can save it in the skeletal interpreter
  // frame and the stack walking of interpreter_sender will get the unextended sp
  // value and not the "real" sp value.

  Address sp_temp(edi, Deoptimization::UnrollBlock::sender_sp_temp_offset_in_bytes());
  __ movl(sp_temp, esp);
  __ subl(esp, Address(edi, Deoptimization::UnrollBlock::caller_adjustment_offset_in_bytes()));

  // Push interpreter frames in a loop
  Label loop;
  __ bind(loop);
  __ movl(ebx, Address(esi));           // Load frame size
  __ subl(ebx, 2*wordSize);             // we'll push pc and ebp by hand
  __ pushl(Address(ecx));               // save return address
  __ enter();                           // save old & set new ebp
  __ subl(esp, ebx);                    // Prolog!
  __ movl(ebx, sp_temp);                // sender's sp
  __ movl(Address(ebp, frame::interpreter_frame_sender_sp_offset * wordSize), ebx); // Make it walkable
  __ movl(sp_temp, esp);                // pass to next frame
  __ addl(esi, 4);                      // Bump array pointer (sizes)
  __ addl(ecx, 4);                      // Bump array pointer (pcs)
  __ decl(counter);                     // decrement counter
  __ jcc(Assembler::notZero, loop);
  __ pushl(Address(ecx));               // save final return address

  // Re-push self-frame
  __ enter();                           // save old & set new ebp

  //  Return address and ebp are in place
  // We'll push additional args later. Just allocate a full sized
  // register save area 
  __ subl(esp, (frame_size_in_words-additional_words - 2) * wordSize);

  // Restore frame locals after moving the frame
  __ movl(Address(esp, RegisterSaver::eaxOffset()*wordSize), eax);
  __ movl(Address(esp, RegisterSaver::edxOffset()*wordSize), edx);
  __ fstp_d(Address(esp, RegisterSaver::fpResultOffset()*wordSize));   // Pop float stack and store in local
  if( UseSSE==2 ) __ movsd(Address(esp, RegisterSaver::xmm0Offset()*wordSize), xmm0);
  if( UseSSE==1 ) __ movss(Address(esp, RegisterSaver::xmm0Offset()*wordSize), xmm0);

  // Set up the args to unpack_frame

  __ pushl(unpack_kind);                     // get the unpack_kind value
  __ get_thread(ecx);
  __ pushl(ecx);

  // set last_Java_sp, last_Java_fp
  __ set_last_Java_frame(ecx, noreg, ebp, NULL);

  // Call C code.  Need thread but NOT official VM entry
  // crud.  We cannot block on this call, no GC can happen.  Call should
  // restore return values to their stack-slots with the new SP.
  __ call( CAST_FROM_FN_PTR(address, Deoptimization::unpack_frames), relocInfo::runtime_call_type );
  // Set an oopmap for the call site
  oop_maps->add_gc_map( __ pc()-start, true, new OopMap( frame_size_in_words, 0 ));

  // eax contains the return result type
  __ pushl(eax);

  __ get_thread(ecx);
  __ reset_last_Java_frame(ecx, false);

  // Collect return values
  __ movl(eax,Address(esp, (RegisterSaver::eaxOffset() + additional_words + 1)*wordSize));
  __ movl(edx,Address(esp, (RegisterSaver::edxOffset() + additional_words + 1)*wordSize));

  // Clear floating point stack before returning to interpreter
  __ empty_FPU_stack();

  // Check if we should push the float or double return value.
  Label results_done, yes_double_value;
  __ cmpl(Address(esp, 0), T_DOUBLE);
  __ jcc (Assembler::zero, yes_double_value);
  __ cmpl(Address(esp, 0), T_FLOAT);
  __ jcc (Assembler::notZero, results_done);

  // Push float on fpu stack

  if( UseSSE>=1 ) __ fld_s(Address(esp, (RegisterSaver::xmm0Offset() + additional_words + 1)*wordSize)); 
  else            __ fld_d(Address(esp, (RegisterSaver::fpResultOffset() + additional_words + 1)*wordSize));
  __ jmp(results_done);

  // Push double on fpu stack

  __ bind(yes_double_value);
  if( UseSSE==2 ) __ fld_d(Address(esp, (RegisterSaver::xmm0Offset() + additional_words + 1)*wordSize)); 
  else            __ fld_d(Address(esp, (RegisterSaver::fpResultOffset() + additional_words + 1)*wordSize));

  __ bind(results_done);

  // Pop self-frame.
  __ leave();                              // Epilog!

  // Jump to interpreter
  __ ret(0);
  
  // -------------
  // make sure all code is generated
  masm->flush();

  _deopt_blob = DeoptimizationBlob::create( buffer, oop_maps, 0, exception_offset, 0, frame_size_in_words);
}


//------------------------------generate_uncommon_trap_blob--------------------
// Ought to generate an ideal graph & compile, but here's some Intel ASM
// instead.  Sigh, Cliff.
void OptoRuntime::generate_uncommon_trap_blob() {
  // allocate space for the code
  ResourceMark rm;
  // setup code generation tools
  CodeBuffer*   buffer = new CodeBuffer(512, 512, 0, 0, 0, false);
  MacroAssembler* masm = new MacroAssembler(buffer);

  enum frame_layout {
    arg0_off,      // thread                     sp + 0 // Arg location for 
    arg1_off,      // unloaded_class_index       sp + 1 // calling C
    xmm6_off,      // callee saved register      sp + 2
    xmm6_off_2,    // callee saved register      sp + 3
    xmm7_off,      // callee saved register      sp + 4
    xmm7_off_2,    // callee saved register      sp + 5
    edi_off,       // callee saved register      sp + 6
    esi_off,       // callee saved register      sp + 7
    ebp_off,       // callee saved register      sp + 8
    return_off,    // slot for return address    sp + 9
    framesize
  };
  
  address start = __ pc();
  // Push self-frame.
  __ subl(esp,return_off*wordSize);     // Epilog!

  // Save callee saved registers.  None for UseSSE=0, 
  // floats-only for UseSSE=1, and doubles for UseSSE=2.
  if( OptoRuntimeCalleeSavedFloats ) {
    if( UseSSE == 1 ) {
      __ movss(Address(esp,xmm6_off*wordSize),xmm6);
      __ movss(Address(esp,xmm7_off*wordSize),xmm7);
    } else if( UseSSE == 2 ) {
      __ movsd(Address(esp,xmm6_off*wordSize),xmm6);
      __ movsd(Address(esp,xmm7_off*wordSize),xmm7);
    }
  }
  __ movl(Address(esp,ebp_off  *wordSize),ebp);
  __ movl(Address(esp,edi_off  *wordSize),edi);
  __ movl(Address(esp,esi_off  *wordSize),esi);

  // Clear the floating point exception stack
  __ empty_FPU_stack();

  // set last_Java_sp
  __ get_thread(edx);
  __ set_last_Java_frame(edx, noreg, noreg, NULL);

  // Call C code.  Need thread but NOT official VM entry
  // crud.  We cannot block on this call, no GC can happen.  Call should
  // capture callee-saved registers as well as return values.
  __ movl(Address(esp, arg0_off*wordSize),edx);
  // argument already in ECX 
  __ movl(Address(esp, arg1_off*wordSize),ecx);
  __ call(CAST_FROM_FN_PTR(address, Deoptimization::uncommon_trap), relocInfo::runtime_call_type);

  // Set an oopmap for the call site
  OopMapSet *oop_maps = new OopMapSet();
  OopMap* map =  new OopMap( framesize, 0 );
  // oop_map->set_callee(
  if( OptoRuntimeCalleeSavedFloats ) {
    map->set_callee_saved( SharedInfo::stack2reg(xmm6_off  ), framesize,0, OptoReg::Name(XMM6a_num) );
    map->set_callee_saved( SharedInfo::stack2reg(xmm6_off+1), framesize,0, OptoReg::Name(XMM6b_num) );
    map->set_callee_saved( SharedInfo::stack2reg(xmm7_off  ), framesize,0, OptoReg::Name(XMM7a_num) );
    map->set_callee_saved( SharedInfo::stack2reg(xmm7_off+1), framesize,0, OptoReg::Name(XMM7b_num) );
  }
  map->set_callee_saved( SharedInfo::stack2reg(ebp_off   ), framesize,0, OptoReg::Name(EBP_num  ) );
  map->set_callee_saved( SharedInfo::stack2reg(edi_off   ), framesize,0, OptoReg::Name(EDI_num  ) );
  map->set_callee_saved( SharedInfo::stack2reg(esi_off   ), framesize,0, OptoReg::Name(ESI_num  ) );
  oop_maps->add_gc_map( __ pc()-start, true, map);

  __ get_thread(ecx);

  __ reset_last_Java_frame(ecx, false);

  // Load UnrollBlock into EDI
  __ movl(edi, eax);

  // Pop all the frames we must move/replace. 
  // 
  // Frame picture (youngest to oldest)
  // 1: self-frame (no frame link)
  // 2: deopting frame  (no frame link)
  // 3: possible-i2c-adapter-frame 
  // 4: caller of deopting frame (could be compiled/interpreted. If interpreted we will create an
  //    and c2i here)

  // Pop self-frame.  We have no frame, and must rely only on EAX and ESP.
  __ addl(esp,(framesize-1)*wordSize);     // Epilog!

  // Pop deoptimized frame
  __ addl(esp,Address(edi,Deoptimization::UnrollBlock::size_of_deoptimized_frame_offset_in_bytes()));

  // Pop I2C adapter frame, if any
  __ addl(esp,Address(edi,Deoptimization::UnrollBlock::adapter_size_offset_in_bytes()));

  // sp should be pointing at the return address to the caller (4)

  // Load array of frame pcs into ECX
  __ movl(ecx,Address(edi,Deoptimization::UnrollBlock::frame_pcs_offset_in_bytes()));

  __ popl(esi); // trash the pc

  // Load array of frame sizes into ESI
  __ movl(esi,Address(edi,Deoptimization::UnrollBlock::frame_sizes_offset_in_bytes()));

  Address counter(edi, Deoptimization::UnrollBlock::counter_temp_offset_in_bytes());

  __ movl(ebx, Address(edi, Deoptimization::UnrollBlock::number_of_frames_offset_in_bytes()));
  __ movl(counter, ebx);

  // Pick up the initial fp we should save
  __ movl(ebp, Address(edi, Deoptimization::UnrollBlock::initial_fp_offset_in_bytes()));

  Label adapter_done;

  // Do we need an adapter frame?

  __ cmpl(Address(edi, Deoptimization::UnrollBlock::new_adapter_offset_in_bytes()), 0);
  __ jcc (Assembler::zero, adapter_done);

  // Push a c2i adapter frame
  __ pushl(Address(ecx));                             // push new pc
  __ movl(ebx, Address(esi));                         // get framesize (includes return address)
  __ subl(ebx, wordSize);                             // account for return address
  __ decl(counter);                                   // count the frame
  __ subl(esp, ebx);                                  // allocate adapter frame
  __ addl(esi, 4);                                    // Bump array pointer (sizes)
  __ addl(ecx, 4);                                    // Bump array pointer (pcs)
  __ bind(adapter_done);

  // Now adjust the caller's stack to make up for the extra locals
  // but record the original sp so that we can save it in the skeletal interpreter
  // frame and the stack walking of interpreter_sender will get the unextended sp
  // value and not the "real" sp value.

  Address sp_temp(edi, Deoptimization::UnrollBlock::sender_sp_temp_offset_in_bytes());
  __ movl(sp_temp, esp);
  __ subl(esp, Address(edi, Deoptimization::UnrollBlock::caller_adjustment_offset_in_bytes()));

  // Push interpreter frames in a loop
  Label loop;
  __ bind(loop);
  __ movl(ebx, Address(esi));           // Load frame size
  __ subl(ebx, 2*wordSize);             // we'll push pc and ebp by hand
  __ pushl(Address(ecx));               // save return address
  __ enter();                           // save old & set new ebp
  __ subl(esp, ebx);                    // Prolog!
  __ movl(ebx, sp_temp);                // sender's sp
  __ movl(Address(ebp, frame::interpreter_frame_sender_sp_offset * wordSize), ebx); // Make it walkable
  __ movl(sp_temp, esp);                // pass to next frame
  __ addl(esi, 4);                      // Bump array pointer (sizes)
  __ addl(ecx, 4);                      // Bump array pointer (pcs)
  __ decl(counter);                     // decrement counter
  __ jcc(Assembler::notZero, loop);
  __ pushl(Address(ecx));               // save final return address

  // Re-push self-frame
  __ enter();                           // save old & set new ebp
  __ subl(esp, (framesize-2) * wordSize);   // Prolog!


  // set last_Java_sp, last_Java_fp
  __ get_thread(edi);
  __ set_last_Java_frame(edi, noreg, ebp, NULL);

  // Call C code.  Need thread but NOT official VM entry
  // crud.  We cannot block on this call, no GC can happen.  Call should
  // restore return values to their stack-slots with the new SP.
  __ movl(Address(esp,arg0_off*wordSize),edi);
  __ movl(Address(esp,arg1_off*wordSize), Deoptimization::Unpack_uncommon_trap); 
  __ call(CAST_FROM_FN_PTR(address, Deoptimization::unpack_frames), relocInfo::runtime_call_type);
  // Set an oopmap for the call site
  oop_maps->add_gc_map( __ pc()-start, true, new OopMap( framesize, 0 ) );

  __ get_thread(edi);
  __ reset_last_Java_frame(edi, false);

  // Pop self-frame.
  __ leave();     // Epilog!

  // Jump to interpreter
  __ ret(0);

  // -------------
  // make sure all code is generated
  masm->flush();

  _uncommon_trap_blob = UncommonTrapBlob::create(buffer, oop_maps, framesize);
}


//------------------------------fill_in_exception_blob-------------------------
// Ought to generate an ideal graph & compile, but here's some Intel ASM
// instead.  Sigh, Cliff.
int handle_exception_deopt_offset = 0;
int handle_exception_deopt_exception_offset = 0;
int handle_exception_deopt_offset_2 = 0;
int handle_exception_deopt_exception_offset_2 = 0;
int handle_exception_call_pc_offset = 0;

void OptoRuntime::fill_in_exception_blob() {  
  enum layout { 
    thread_off,                 // last_java_sp                
    ebp_off,                
    edi_off,         
    esi_off,
    xmm6_off,                   // callee saved register
    xmm6_off_2,                 // callee saved register
    xmm7_off,                   // callee saved register
    xmm7_off_2,                 // callee saved register
    return_off,                 // slot for return address
    framesize 
  };

   // Patch blob
  assert(handle_exception_stub() != NULL, "exception stub must have been generated");
  assert(handle_exception_call_pc_offset != 0, "");

  // Set an oopmap for the call site.  This oopmap will only be used if we
  // are unwinding the stack.  Hence, all locations will be dead.
  // Callee-saved registers will be the same as the frame above (i.e.,
  // handle_exception_stub), since they were restored when we got the
  // exception.
  ResourceMark rm;
  OopMapSet *oop_maps = new OopMapSet();
  OopMap* map =  new OopMap( framesize, 0 );        
  map->set_callee_saved( SharedInfo::stack2reg(ebp_off   ), framesize, 0, OptoReg::Name(EBP_num  ) );
  map->set_callee_saved( SharedInfo::stack2reg(edi_off   ), framesize, 0, OptoReg::Name(EDI_num  ) );
  map->set_callee_saved( SharedInfo::stack2reg(esi_off   ), framesize, 0, OptoReg::Name(ESI_num  ) );
  if( OptoRuntimeCalleeSavedFloats ) {
    map->set_callee_saved( SharedInfo::stack2reg(xmm6_off  ), framesize, 0, OptoReg::Name(XMM6a_num) );
    map->set_callee_saved( SharedInfo::stack2reg(xmm6_off+1), framesize, 0, OptoReg::Name(XMM6b_num) );
    map->set_callee_saved( SharedInfo::stack2reg(xmm7_off  ), framesize, 0, OptoReg::Name(XMM7a_num) );
    map->set_callee_saved( SharedInfo::stack2reg(xmm7_off+1), framesize, 0, OptoReg::Name(XMM7b_num) );
  }
  oop_maps->add_gc_map( handle_exception_call_pc_offset, true, map);
  exception_blob()->set_oop_maps(oop_maps);
}


//------------------------------setup_exception_blob---------------------------
// creates exception blob at the end
// Using exception blob, this code is jumped from a compiled method.
// (see emit_exception_handler in i486.ad file)
// It is also the default exception destination for c2i and i2c adapters
// 
// Given an exception pc at a call, we may unwind the stack and jump to
// caller's exception handler. We may also continue into the handler
// This code is entered with a jmp.
// 
// Arguments:
//   eax: exception oop
//   edx: exception pc
//
// Results:
//   eax: exception oop
//   edx: exception pc in caller or ???
//   destination: exception handler of caller
// 
// Note: the exception pc MUST be at a call (precise debug information)
//       Only register eax, edx, ecx are not callee saved.
//

void OptoRuntime::setup_exception_blob() {
  assert(!OptoRuntime::is_callee_saved_register(EDX_num), "");
  assert(!OptoRuntime::is_callee_saved_register(EAX_num), "");
  assert(!OptoRuntime::is_callee_saved_register(ECX_num), "");

  // Capture info about frame layout  
  enum layout { 
    thread_off,                 // last_java_sp                
    ebp_off,                
    edi_off,         
    esi_off,
    xmm6_off,                   // callee saved register
    xmm6_off_2,                 // callee saved register
    xmm7_off,                   // callee saved register
    xmm7_off_2,                 // callee saved register
    return_off,                 // slot for return address
    framesize 
  };

  // allocate space for the code
  ResourceMark rm;
  // setup code generation tools  
  CodeBuffer*   buffer = new CodeBuffer(512, 512, 0, 0, 0, false);
  MacroAssembler* masm = new MacroAssembler(buffer);


  Label exception_handler_found;

  address start = __ pc();  

  __ pushl(edx);
  __ subl(esp, return_off * wordSize);   // Prolog!

  // Save callee saved registers.  None for UseSSE=0, 
  // floats-only for UseSSE=1, and doubles for UseSSE=2.
  if( OptoRuntimeCalleeSavedFloats ) {
    if( UseSSE == 1 ) {
      __ movss(Address(esp,xmm6_off*wordSize),xmm6);
      __ movss(Address(esp,xmm7_off*wordSize),xmm7);
    } else if( UseSSE == 2 ) {
      __ movsd(Address(esp,xmm6_off*wordSize),xmm6);
      __ movsd(Address(esp,xmm7_off*wordSize),xmm7);
    }
  }
  __ movl(Address(esp,ebp_off  *wordSize),ebp);
  __ movl(Address(esp,edi_off  *wordSize),edi);
  __ movl(Address(esp,esi_off  *wordSize),esi);
          
  // Store exception in Thread object. We cannot pass any arguments to the
  // handle_exception call, since we do not want to make any assumption
  // about the size of the frame where the exception happened in.
  __ get_thread(ecx);
  __ movl(Address(ecx, JavaThread::exception_oop_offset()), eax);
  __ movl(Address(ecx, JavaThread::exception_pc_offset()),  edx);

  // This call does all the hard work.  It checks if an exception handler
  // exists in the method.  
  // If so, it returns the handler address.
  // If not, it prepares for stack-unwinding, restoring the callee-save 
  // registers of the frame being removed.
  //  
  __ movl(Address(esp, thread_off * wordSize), ecx); // Thread is first argument
  __ set_last_Java_frame(ecx, noreg, noreg, NULL);
  __ call(CAST_FROM_FN_PTR(address, OptoRuntime::handle_exception_C), relocInfo::runtime_call_type);
  handle_exception_call_pc_offset = __ pc() - start;
  __ get_thread(ecx);
  __ reset_last_Java_frame(ecx, false);

  // Restore callee-saved registers
  if( OptoRuntimeCalleeSavedFloats ) {
    if( UseSSE == 1 ) {
      __ movss(xmm6,Address(esp,xmm6_off*wordSize));
      __ movss(xmm7,Address(esp,xmm7_off*wordSize));
    } else if( UseSSE == 2 ) {
      __ movsd(xmm6,Address(esp,xmm6_off*wordSize));
      __ movsd(xmm7,Address(esp,xmm7_off*wordSize));
    }
  }
  __ movl(ebp, Address(esp, ebp_off * wordSize));
  __ movl(edi, Address(esp, edi_off * wordSize));
  __ movl(esi, Address(esp, esi_off * wordSize));
  __ addl(esp, return_off * wordSize);   // Epilog!
  __ popl(edx); // Exception pc


  // eax: 0 if no exception handler for given <exception oop/exception pc>
  // found
  __ testl(eax, eax);
  __ jcc(Assembler::notZero, exception_handler_found);    

  // No exception handler found. We unwind stack one level.
  // All information is stored in thread.
  // thread->exception_oop
  // thread->exception_handler_pc
  // thread->exception_pc;
  // thread->exception_stack_size

  // The registers ebp, edi and esi contain the addresses of their values
  // load the values into the callee saved registers (only if register is non-zero)

  Label skipEBP, skipEDI, skipESI;
  __ testl(ebp, ebp);
  __ jcc  (Assembler::zero, skipEBP);
  __ movl (ebp, Address(ebp));
  __ bind (skipEBP);

  __ testl(edi, edi);
  __ jcc  (Assembler::zero, skipEDI);
  __ movl (edi, Address(edi));
  __ bind (skipEDI);

  __ testl(esi, esi);
  __ jcc  (Assembler::zero, skipESI);
  __ movl (esi, Address(esi));
  __ bind (skipESI);

  // now we continue at JavaThread::exception_handler_pc_offset_in_bytes.
  // The caller frame state is defined by its return address.

  __ get_thread (ecx);    
  __ movl(edx, Address(ecx, JavaThread::exception_pc_offset()));
  __ movl(eax, Address(ecx, JavaThread::exception_stack_size_offset()));
  //   edx: exception-pc
  //   eax: stack-size
  __ addl (esp, eax);      // remove frame (including return address)
  __ pushl(Address(ecx,JavaThread::exception_handler_pc_offset()));

  __ movl(eax, Address(ecx, JavaThread::exception_oop_offset()));   // exception oop 
#ifdef ASSERT
  __ movl(Address(ecx, JavaThread::exception_oop_offset()), 0);
  __ movl(Address(ecx, JavaThread::exception_handler_pc_offset()), 0);
  __ movl(Address(ecx, JavaThread::exception_pc_offset()), 0);
#endif

  // eax: exception oop
  // edx: exception pc
  // return to handler
 
  __ ret(0);

  // Exception handler found.       
  __ bind (exception_handler_found);  
  
  Label deoptL_2, endL_2;

  // We have a handler in eax (could be deopt blob)
  // edx - throwing pc, deopt blob will need it.

  __ pushl(eax); 

  // ecx contains handler address

  __ get_thread(ecx);           // TLS
  // Get the exception
  __ movl(eax, Address(ecx, JavaThread::exception_oop_offset()));
  // Get the exception pc in case we are deoptimized
  __ movl(edx, Address(ecx, JavaThread::exception_pc_offset()));
#ifdef ASSERT
  __ movl(Address(ecx, JavaThread::exception_handler_pc_offset()), 0);
  __ movl(Address(ecx, JavaThread::exception_pc_offset()), 0); 
  __ movl(Address(ecx, JavaThread::exception_oop_offset()), 0);
#endif

  __ popl(ecx);

  // eax: exception oop
  // ecx: exception handler
  // edx: exception pc
  __ jmp (ecx);

  // -------------
  // make sure all code is generated
  masm->flush();  

  // Set exception blob
  OopMapSet *oop_maps = NULL; // will be set later; currently the register stuff is not yet initialized!
  _exception_blob = ExceptionBlob::create(buffer, oop_maps, framesize);  
}


//------------------------------generate_handler_blob------
//
// Generate a special Compile2Runtime blob that saves all registers, 
// and setup oopmap.
//
SafepointBlob* OptoRuntime::generate_handler_blob(address call_ptr, bool cause_return) {

  // Account for thread arg in our frame
  const int additional_words = 1; 
  int frame_size_in_words;

  assert (StubRoutines::forward_exception_entry() != NULL, "must be generated before");  

  ResourceMark rm;
  OopMapSet *oop_maps = new OopMapSet();
  OopMap* map;

  // allocate space for the code
  // setup code generation tools  
  CodeBuffer*   buffer = new CodeBuffer(1024, 512, 0, 0, 0, false);
  MacroAssembler* masm = new MacroAssembler(buffer);
  
  const Register java_thread = edi; // callee-saved for VC++
  address start   = __ pc();  
  address call_pc = NULL;  

  // If cause_return is true we are at a poll_return and there is
  // the return address on the stack to the caller on the nmethod
  // that is safepoint. We can leave this return on the stack and
  // effectively complete the return and safepoint in the caller.
  // Otherwise we push space for a return address that the safepoint
  // handler will install later to make the stack walking sensible.
  if( !cause_return )
    __ pushl(ebx);                // Make room for return address (or push it again)

  map = RegisterSaver::save_live_registers(masm, additional_words, &frame_size_in_words);
  
  // The following is basically a call_VM. However, we need the precise
  // address of the call in order to generate an oopmap. Hence, we do all the
  // work outselvs.

  // Push thread argument and setup last_Java_sp
  __ get_thread(java_thread);
  __ pushl(java_thread);
  __ set_last_Java_frame(java_thread, noreg, noreg, NULL);

  // do the call
  __ call(call_ptr, relocInfo::runtime_call_type);

  // Set an oopmap for the call site.  This oopmap will map all
  // oop-registers and debug-info registers as callee-saved.  This
  // will allow deoptimization at this safepoint to find all possible
  // debug-info recordings, as well as let GC find all oops.

  oop_maps->add_gc_map( __ pc() - start, true, map);

  // Discard arg
  __ popl(ecx);

  Label noException;

  // Clear last_Java_sp again
  __ get_thread(java_thread);
  __ reset_last_Java_frame(java_thread, false);

  __ cmpl(Address(java_thread, Thread::pending_exception_offset()), NULL);
  __ jcc(Assembler::equal, noException);

  // Exception pending

  RegisterSaver::restore_live_registers(masm);

  __ jmp(StubRoutines::forward_exception_entry(), relocInfo::runtime_call_type);

  // No exception case
  Label continueL;
  __ bind(noException);
  __ testl (eax, eax);		// continue call?
  __ jcc(Assembler::negative, continueL);

  // Normal exit, register restoring and exit  
  RegisterSaver::restore_live_registers(masm);

  __ ret(0);
  
  // we have deoptimized at a blocked call, we may not reexecute the
  // instruction as we would skip the call in interpreter; therefore
  // execute the destination of the call; the destination is valid
  // because the receiver was already consumed
  // ecx holds the destination of the call
  __ bind(continueL);
  // Move eax (continuation address) to where ebx will be loaded from
  __ movl(Address(esp, RegisterSaver::ebxOffset()*wordSize), eax);

  RegisterSaver::restore_live_registers(masm);

  // Everything is just like we were at entry (except ebx)
  // original return address is still there too (we deopt on return)
  // just continue with the call.
  __ jmp(ebx);

  // -------------
  // make sure all code is generated
  masm->flush();  

  // Fill-out other meta info
  return SafepointBlob::create(buffer, oop_maps, frame_size_in_words);    
}

//------------------------------generate_illegal_instruction_handler_blob------
void OptoRuntime::generate_illegal_instruction_handler_blob() {
  _illegal_instruction_handler_blob =
    generate_handler_blob(CAST_FROM_FN_PTR(address, SafepointSynchronize::handle_illegal_instruction_exception), false);
}
  
//------------------------------generate_polling_page_safepoint_handler_blob-------------
void OptoRuntime::generate_polling_page_safepoint_handler_blob() {
  _polling_page_safepoint_handler_blob =
    generate_handler_blob(CAST_FROM_FN_PTR(address, SafepointSynchronize::handle_polling_page_exception), false);
}

//------------------------------generate_polling_page_return_handler_blob-------------
void OptoRuntime::generate_polling_page_return_handler_blob() {
  _polling_page_return_handler_blob =
    generate_handler_blob(CAST_FROM_FN_PTR(address, SafepointSynchronize::handle_polling_page_exception), true);
}

//------------------------------pd_unwind_stack--------------------------------
void OptoRuntime::pd_unwind_stack(JavaThread *thread, frame fr, RegisterMap* reg_map) {
  // Update callee-saved registers across frame
  jint* reg_locs[REG_COUNT];
  // Copy values of all callee-saved registers out of frame
  for(int i = 0; i < REG_COUNT; i++) {
    jint* loc = (jint*)reg_map->location(VMReg::Name(i));    
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
  for(OopMapStream oms(map,OopMapValue::callee_saved_value); !oms.is_done(); oms.next()) {
    omv = oms.current();
    assert(omv.is_stack_loc(), "sanity check");
    jint* location = (jint*)stub_frame.oopmapreg_to_location(omv.reg(),NULL);
    VMReg::Name reg = omv.content_reg();
    assert(reg >= 0 && reg < REG_COUNT, "reg out of range");
    jint** location2 = (jint**)location;
    *location2 = reg_locs[reg];
  }
}
