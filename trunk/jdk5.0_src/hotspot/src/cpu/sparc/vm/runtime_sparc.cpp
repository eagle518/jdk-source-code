#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)runtime_sparc.cpp	1.117 03/12/23 16:37:21 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_runtime_sparc.cpp.incl"

// Deoptimization

#define __ masm->

UncommonTrapBlob   *OptoRuntime::_uncommon_trap_blob;
DeoptimizationBlob *SharedRuntime::_deopt_blob;
ExceptionBlob      *OptoRuntime::_exception_blob;
SafepointBlob      *OptoRuntime::_illegal_instruction_handler_blob;
SafepointBlob      *OptoRuntime::_polling_page_safepoint_handler_blob;
SafepointBlob      *OptoRuntime::_polling_page_return_handler_blob;

static int handle_exception_call_pc_offset = 0;
static const int exception_blob_words = 0;

static const MachRegisterNumbers F_Reg[64] = { 
  // First OopMap for float registers
  R_F0_num,R_F1_num,R_F2_num,R_F3_num,
  R_F4_num,R_F5_num,R_F6_num,R_F7_num, 
  R_F8_num,R_F9_num,R_F10_num,R_F11_num,
  R_F12_num,R_F13_num,R_F14_num,R_F15_num, 
  R_F16_num,R_F17_num,R_F18_num,R_F19_num,
  R_F20_num,R_F21_num,R_F22_num,R_F23_num, 
  R_F24_num,R_F25_num,R_F26_num,R_F27_num,
  R_F28_num,R_F29_num,R_F30_num,R_F31_num,
  // Additional v9 double registers
  R_D32_num,R_D32x_num,
  R_D34_num,R_D34x_num,
  R_D36_num,R_D36x_num,
  R_D38_num,R_D38x_num,
  R_D40_num,R_D40x_num,
  R_D42_num,R_D42x_num,
  R_D44_num,R_D44x_num,
  R_D46_num,R_D46x_num,
  R_D48_num,R_D48x_num,
  R_D50_num,R_D50x_num,
  R_D52_num,R_D52x_num,
  R_D54_num,R_D54x_num,
  R_D56_num,R_D56x_num,
  R_D58_num,R_D58x_num,
  R_D60_num,R_D60x_num,
  R_D62_num,R_D62x_num
};

class RegisterSaver {

  // Used for saving volatile registers. This is Gregs, Fregs, I/L/O.
  // The Oregs are problematic. In the 32bit build the compiler can
  // have O registers live with 64 bit quantities. A window save will
  // cut the heads off of the registers. We have to do a very extensive
  // stack dance to save and restore these properly.

  // Note that the Oregs problem only exists if we block at either a polling
  // page exception a compiled code safepoint that was not originally a call
  // or deoptimize following one of these kinds of safepoints. 

  // Lots of registers to save.  For all builds, a window save will preserve
  // the %i and %l registers.  For the 32-bit longs-in-two entries and 64-bit
  // builds a window-save will preserve the %o registers.  In the LION build
  // we need to save the 64-bit %o registers which requires we save them
  // before the window-save (as then they become %i registers and get their
  // heads chopped off on interrupt).  We have to save some %g registers here
  // as well.
  enum { 
    // This frame's save area.  Includes extra space for the native call:
    // vararg's layout space and the like.  Briefly holds the caller's
    // register save area.
    call_args_area = frame::register_save_words_sp_offset +
		     frame::memory_parameter_word_sp_offset*wordSize,
    // Make sure save locations are always 8 byte aligned.
    // can't use round_to because it doesn't produce compile time constant
    start_of_extra_save_area = ((call_args_area + 7) & ~7),
    g1_offset = start_of_extra_save_area, // g-regs needing saving
    g3_offset = g1_offset+8,
    g4_offset = g3_offset+8,
    g5_offset = g4_offset+8,
    o0_offset = g5_offset+8,
    o1_offset = o0_offset+8,
    o2_offset = o1_offset+8,
    o3_offset = o2_offset+8,
    o4_offset = o3_offset+8,
    o5_offset = o4_offset+8,
    start_of_flags_save_area = o5_offset+8,
    ccr_offset = start_of_flags_save_area,
    fsr_offset = ccr_offset + 8,
    d00_offset = fsr_offset+8,  // Start of float save area
    register_save_size = d00_offset+8*32
  };


  public:

  static OopMap* save_live_registers(MacroAssembler* masm, int additional_frame_words, int* total_frame_words);
  static void restore_live_registers(MacroAssembler* masm);

  // During deoptimization only the result register need to be restored
  // all the other values have already been extracted.

  static void restore_result_registers(MacroAssembler* masm);
};

OopMap* RegisterSaver::save_live_registers(MacroAssembler* masm, int additional_frame_words, int* total_frame_words) {
  // Record volatile registers as callee-save values in an OopMap so their save locations will be
  // propagated to the caller frame's RegisterMap during StackFrameStream construction (needed for
  // deoptimization; see compiledVFrame::create_stack_value).  The caller's I, L and O registers
  // are saved in register windows - I's and L's in the caller's frame and O's in the stub frame
  // (as the stub's I's) when the runtime routine called by the stub creates its frame.
  int i;
  // Always make the frame size 16 bytr aligned.
  int frame_size = round_to(additional_frame_words + register_save_size, 16);
  // OopMap frame size is in c2 stack slots (sizeof(jint)) not bytes or words
  int frame_size_in_slots = frame_size / sizeof(jint);
  // CodeBlob frame size is in words.
  *total_frame_words = frame_size / wordSize;
  // OopMap* map = new OopMap(*total_frame_words, 0);
  OopMap* map = new OopMap(frame_size_in_slots, 0);

#if !defined(_LP64)

  // Save 64-bit O registers; they will get their heads chopped off on a 'save'.
  __ stx(O0, G2_thread, JavaThread::o_reg_temps_offset_in_bytes()+0*8);
  __ stx(O1, G2_thread, JavaThread::o_reg_temps_offset_in_bytes()+1*8);
  __ stx(O2, G2_thread, JavaThread::o_reg_temps_offset_in_bytes()+2*8);
  __ stx(O3, G2_thread, JavaThread::o_reg_temps_offset_in_bytes()+3*8);
  __ stx(O4, G2_thread, JavaThread::o_reg_temps_offset_in_bytes()+4*8);
  __ stx(O5, G2_thread, JavaThread::o_reg_temps_offset_in_bytes()+5*8);
#endif /* LP64 */

  __ save(SP, -frame_size, SP);

#ifndef _LP64
  // Reload the 64 bit Oregs. Although they are now Iregs we load them
  // to Oregs here to avoid interrupts cutting off their heads

  __ ldx(G2_thread, JavaThread::o_reg_temps_offset_in_bytes()+0*8, O0);
  __ ldx(G2_thread, JavaThread::o_reg_temps_offset_in_bytes()+1*8, O1);
  __ ldx(G2_thread, JavaThread::o_reg_temps_offset_in_bytes()+2*8, O2);
  __ ldx(G2_thread, JavaThread::o_reg_temps_offset_in_bytes()+3*8, O3);
  __ ldx(G2_thread, JavaThread::o_reg_temps_offset_in_bytes()+4*8, O4);
  __ ldx(G2_thread, JavaThread::o_reg_temps_offset_in_bytes()+5*8, O5);

  // Now store them where the oopMap will locate them

  __ stx(O0, SP, o0_offset+STACK_BIAS);
  map->set_callee_saved(SharedInfo::stack2reg((o0_offset)>>2), frame_size_in_slots, 0, OptoReg::Name(R_O0H_num));
  map->set_callee_saved(SharedInfo::stack2reg((o0_offset + 4)>>2), frame_size_in_slots, 0, OptoReg::Name(R_O0_num));

  __ stx(O1, SP, o1_offset+STACK_BIAS);

  map->set_callee_saved(SharedInfo::stack2reg((o1_offset)>>2), frame_size_in_slots, 0, OptoReg::Name(R_O1H_num));
  map->set_callee_saved(SharedInfo::stack2reg((o1_offset + 4)>>2), frame_size_in_slots, 0, OptoReg::Name(R_O1_num));

  __ stx(O2, SP, o2_offset+STACK_BIAS);
  map->set_callee_saved(SharedInfo::stack2reg((o2_offset)>>2), frame_size_in_slots, 0, OptoReg::Name(R_O2H_num));
  map->set_callee_saved(SharedInfo::stack2reg((o2_offset + 4)>>2), frame_size_in_slots, 0, OptoReg::Name(R_O2_num));

  __ stx(O3, SP, o3_offset+STACK_BIAS);
  map->set_callee_saved(SharedInfo::stack2reg((o3_offset)>>2), frame_size_in_slots, 0, OptoReg::Name(R_O3H_num));
  map->set_callee_saved(SharedInfo::stack2reg((o3_offset + 4)>>2), frame_size_in_slots, 0, OptoReg::Name(R_O3_num));

  __ stx(O4, SP, o4_offset+STACK_BIAS);
  map->set_callee_saved(SharedInfo::stack2reg((o4_offset)>>2), frame_size_in_slots, 0, OptoReg::Name(R_O4H_num));
  map->set_callee_saved(SharedInfo::stack2reg((o4_offset + 4)>>2), frame_size_in_slots, 0, OptoReg::Name(R_O4_num));

  __ stx(O5, SP, o5_offset+STACK_BIAS);
  map->set_callee_saved(SharedInfo::stack2reg((o5_offset)>>2), frame_size_in_slots, 0, OptoReg::Name(R_O5H_num));
  map->set_callee_saved(SharedInfo::stack2reg((o5_offset + 4)>>2), frame_size_in_slots, 0, OptoReg::Name(R_O5_num));
#endif /* LP64 */

  // Save the G's
  __ stx(G1, SP, g1_offset+STACK_BIAS);
  map->set_callee_saved(SharedInfo::stack2reg((g1_offset)>>2), frame_size_in_slots, 0, OptoReg::Name(R_G1H_num));
  map->set_callee_saved(SharedInfo::stack2reg((g1_offset + 4)>>2), frame_size_in_slots, 0, OptoReg::Name(R_G1_num));

  __ stx(G3, SP, g3_offset+STACK_BIAS);
  map->set_callee_saved(SharedInfo::stack2reg((g3_offset)>>2), frame_size_in_slots, 0, OptoReg::Name(R_G3H_num));
  map->set_callee_saved(SharedInfo::stack2reg((g3_offset + 4)>>2), frame_size_in_slots, 0, OptoReg::Name(R_G3_num));

  __ stx(G4, SP, g4_offset+STACK_BIAS);
  map->set_callee_saved(SharedInfo::stack2reg((g4_offset)>>2), frame_size_in_slots, 0, OptoReg::Name(R_G4H_num));
  map->set_callee_saved(SharedInfo::stack2reg((g4_offset + 4)>>2), frame_size_in_slots, 0, OptoReg::Name(R_G4_num));

  __ stx(G5, SP, g5_offset+STACK_BIAS);
  map->set_callee_saved(SharedInfo::stack2reg((g5_offset)>>2), frame_size_in_slots, 0, OptoReg::Name(R_G5H_num));
  map->set_callee_saved(SharedInfo::stack2reg((g5_offset + 4)>>2), frame_size_in_slots, 0, OptoReg::Name(R_G5_num));


  // Save the flags
  __ rdccr( G5 );
  __ stx(G5, SP, ccr_offset+STACK_BIAS);
  __ stxfsr(SP, fsr_offset+STACK_BIAS);

  // Save all the FP registers
  int offset = d00_offset;
  for( int i=0; i<64; i+=2 ) {
    __ stf(FloatRegisterImpl::D,  as_FloatRegister(i), SP, offset+STACK_BIAS);
    map->set_callee_saved(SharedInfo::stack2reg(offset>>2), frame_size_in_slots, 0, OptoReg::Name(F_Reg[i]));
    map->set_callee_saved(SharedInfo::stack2reg((offset + sizeof(float))>>2), frame_size_in_slots, 0, OptoReg::Name(F_Reg[i+1]));
    offset += sizeof(double);
  }

  // And we're done.

  return map;
}


// Pop the current frame and restore all the registers that we
// saved.
void RegisterSaver::restore_live_registers(MacroAssembler* masm) {

  // Restore all the FP registers
  for( int i=0; i<64; i+=2 ) {
    __ ldf(FloatRegisterImpl::D, SP, d00_offset+i*sizeof(float)+STACK_BIAS, as_FloatRegister(i));
  }

  __ ldx(SP, ccr_offset+STACK_BIAS, G1);
  __ wrccr (G1) ;

  // Restore the G's
  // Note that G2 (AKA GThread) must be saved and restored separately.  
  // TODO-FIXME: save and restore some of the other ASRs, viz., %asi and %gsr.  

  __ ldx(SP, g1_offset+STACK_BIAS, G1);
  __ ldx(SP, g3_offset+STACK_BIAS, G3);
  __ ldx(SP, g4_offset+STACK_BIAS, G4);
  __ ldx(SP, g5_offset+STACK_BIAS, G5);


#if !defined(_LP64)
  // Restore the 64-bit O's.
  __ ldx(SP, o0_offset+STACK_BIAS, O0);
  __ ldx(SP, o1_offset+STACK_BIAS, O1);
  __ ldx(SP, o2_offset+STACK_BIAS, O2);
  __ ldx(SP, o3_offset+STACK_BIAS, O3);
  __ ldx(SP, o4_offset+STACK_BIAS, O4);
  __ ldx(SP, o5_offset+STACK_BIAS, O5);

  // And temporarily place them in TLS

  __ stx(O0, G2_thread, JavaThread::o_reg_temps_offset_in_bytes()+0*8);
  __ stx(O1, G2_thread, JavaThread::o_reg_temps_offset_in_bytes()+1*8);
  __ stx(O2, G2_thread, JavaThread::o_reg_temps_offset_in_bytes()+2*8);
  __ stx(O3, G2_thread, JavaThread::o_reg_temps_offset_in_bytes()+3*8);
  __ stx(O4, G2_thread, JavaThread::o_reg_temps_offset_in_bytes()+4*8);
  __ stx(O5, G2_thread, JavaThread::o_reg_temps_offset_in_bytes()+5*8);
#endif /* LP64 */

  // Restore flags

  __ ldxfsr(SP, fsr_offset+STACK_BIAS);

  __ restore();

#if !defined(_LP64)
  // Now reload the 64bit Oregs after we've restore the window.
  __ ldx(G2_thread, JavaThread::o_reg_temps_offset_in_bytes()+0*8, O0);
  __ ldx(G2_thread, JavaThread::o_reg_temps_offset_in_bytes()+1*8, O1);
  __ ldx(G2_thread, JavaThread::o_reg_temps_offset_in_bytes()+2*8, O2);
  __ ldx(G2_thread, JavaThread::o_reg_temps_offset_in_bytes()+3*8, O3);
  __ ldx(G2_thread, JavaThread::o_reg_temps_offset_in_bytes()+4*8, O4);
  __ ldx(G2_thread, JavaThread::o_reg_temps_offset_in_bytes()+5*8, O5);
#endif /* LP64 */

}

// Pop the current frame and restore the registers that might be holding
// a result.
void RegisterSaver::restore_result_registers(MacroAssembler* masm) {

#if !defined(_LP64)
  // 32bit build returns longs in G1
  __ ldx(SP, g1_offset+STACK_BIAS, G1);

  // Retrieve the 64-bit O's.
  __ ldx(SP, o0_offset+STACK_BIAS, O0);
  __ ldx(SP, o1_offset+STACK_BIAS, O1);
  // and save to TLS
  __ stx(O0, G2_thread, JavaThread::o_reg_temps_offset_in_bytes()+0*8);
  __ stx(O1, G2_thread, JavaThread::o_reg_temps_offset_in_bytes()+1*8);
#endif /* LP64 */

  __ ldf(FloatRegisterImpl::D, SP, d00_offset+STACK_BIAS, as_FloatRegister(0));

  __ restore();

#if !defined(_LP64)
  // Now reload the 64bit Oregs after we've restore the window.
  __ ldx(G2_thread, JavaThread::o_reg_temps_offset_in_bytes()+0*8, O0);
  __ ldx(G2_thread, JavaThread::o_reg_temps_offset_in_bytes()+1*8, O1);
#endif /* LP64 */

}

//------------------------------setup_exception_blob---------------------------
// creates exception blob at the end
// Using exception blob, this code is jumped from a compiled method.
// (see emit_exception_handler in sparc.ad file)
// It is also the default exception destination for c2i and i2c adapters
// 
// Given an exception pc at a call, we may unwind the stack and jump to
// caller's exception handler. We may also continue into the handler
// This code is entered with a jmp.
// 
// Arguments:
//   O0: exception oop
//   O1: exception pc
//
// Results:
//   O0: exception oop
//   O1: exception pc in caller or ???
//   destination: exception handler of caller
// 
// Note: the exception pc MUST be at a call (precise debug information)
//
void OptoRuntime::setup_exception_blob() {
  // allocate space for code
  ResourceMark rm;
  int pad = VerifyThread ? 256 : 0;// Extra slop space for more verify code

  // setup code generation tools
  // Measured 8/7/03 at 256 in 32bit debug build (no VerifyThread)
  // Measured 8/7/03 at 528 in 32bit debug build (VerifyThread)
  CodeBuffer*     buffer   = new CodeBuffer(600+pad, 512, 0, 0, 0, false);
  MacroAssembler* masm     = new MacroAssembler(buffer);

  Label exception_handler_found;
  Label L;

  int start = __ offset();

  __ verify_thread();
  __ st_ptr(Oexception,  Address(G2_thread, 0, in_bytes(JavaThread::exception_oop_offset())));
  __ st_ptr(Oissuing_pc, Address(G2_thread, 0, in_bytes(JavaThread::exception_pc_offset())));

  // This call does all the hard work. It checks if an exception catch
  // exists in the method.  
  // If so, it returns the handler address.
  // If the nmethod has been deoptimized and it had a handler the handler
  // address is the deopt blob unpack_with_exception entry.
  //
  // If no handler exists it prepares for stack-unwinding, restoring the callee-save 
  // registers of the frame being removed.
  //
  __ save_frame(exception_blob_words);

  __ mov(G2_thread, O0);
  __ set_last_Java_frame(SP, noreg);
  __ save_thread(L7_thread_cache);

  // This call can block at exit and nmethod can be deoptimized at that
  // point. If the nmethod had a catch point we would jump to the 
  // now deoptimized catch point and fall thru the vanilla deopt
  // path and lose the exception
  // Sure would be simpler if this call didn't block!
  __ call(CAST_FROM_FN_PTR(address, OptoRuntime::handle_exception_C), relocInfo::runtime_call_type);
  __ delayed()->mov(L7_thread_cache, O0);
  handle_exception_call_pc_offset = __ offset() - start;

  __ bind(L);
  __ restore_thread(L7_thread_cache);
  __ reset_last_Java_frame();

  __ mov(O0, G3_scratch);             // Move handler address to temp
  __ restore();
  __ br_notnull(G3_scratch, false, Assembler::pn, exception_handler_found);
  __ delayed()->nop();

  // No handler in top frame. We must unwind this frame

  // At this point exception_pc will now be the pc we would normally return to
  // sender of the adapter. We will be going to the exception handler. If
  // the sender has been deoptimized then we will windup jumping to the
  // deopt blob unpack_exception entry which knows how to do the correct
  // thing. The key is the fact that the original pc we would have returned
  // to (with no exception) is in the exception_pc.

  __ ld_ptr(Address(G2_thread, 0, in_bytes(JavaThread::exception_oop_offset())),
	Oexception  ->after_save()); // O0
  __ ld_ptr(Address(G2_thread, 0, in_bytes(JavaThread::exception_pc_offset())),
	Oissuing_pc ->after_save()); // O1

  Label endL;

  __ ld_ptr(Address(G2_thread, 0, in_bytes(JavaThread::exception_handler_pc_offset())), G3_scratch);

  // G3_scratch is handler-pc of caller is where we continue 
  // Oexception is live

#ifdef ASSERT
  __ st_ptr(G0, Address(G2_thread, 0, in_bytes(JavaThread::exception_oop_offset())));
  __ st_ptr(G0, Address(G2_thread, 0, in_bytes(JavaThread::exception_handler_pc_offset())));
  __ st_ptr(G0, Address(G2_thread, 0, in_bytes(JavaThread::exception_pc_offset())));
#endif
  // load up any stack adjustment necessary on return
  __ ld(Address(G2_thread, 0, in_bytes(JavaThread::exception_stack_size_offset())), O5);
  __ JMP(G3_scratch, 0);
  __ delayed()->restore(FP, O5, SP); // remove frame and any adjustment done by interpreter

  // Exception handler found.
  // If frame has been deoptimized then handler has been patch to do the deopt properly
  // we need to do is load the registers properly

  __ bind(exception_handler_found);

  Label endL_2;


  // G3_scratch contains handler address
  // Since this may be the deopt blob we must set O7 to look like we returned
  // from the original pc that threw the exception

  __ ld_ptr(Address(G2_thread, 0, in_bytes(JavaThread::exception_pc_offset())), O7);
  __ sub(O7, frame::pc_return_offset, O7);


  assert(Assembler::is_simm13(in_bytes(JavaThread::exception_oop_offset())), "exception offset overflows simm13, following ld instruction cannot be in delay slot");
#ifdef ASSERT
  __ ld_ptr(Address(G2_thread, 0, in_bytes(JavaThread::exception_oop_offset())), Oexception); // O0
  __ st_ptr(G0, Address(G2_thread, 0, in_bytes(JavaThread::exception_handler_pc_offset())));
  __ st_ptr(G0, Address(G2_thread, 0, in_bytes(JavaThread::exception_pc_offset())));
  __ st_ptr(G0, Address(G2_thread, 0, in_bytes(JavaThread::exception_oop_offset())));
  __ JMP(G3_scratch, 0);
  __ delayed()->nop();
#else
  __ JMP(G3_scratch, 0);
  __ delayed()->ld_ptr(Address(G2_thread, 0, in_bytes(JavaThread::exception_oop_offset())), Oexception); // O0
#endif

  // -------------
  // make sure all code is generated
  masm->flush();

  // Fill out other meta info
  OopMapSet *oop_maps = NULL; // will be set later; currently the register stuff is not yet initialized!
  _exception_blob = ExceptionBlob::create(buffer, oop_maps, __ total_frame_size_in_bytes(0)/wordSize);
  address xxxyyy = _exception_blob->instructions_begin();
}

//------------------------------fill_in_exception_blob-------------------------
void OptoRuntime::fill_in_exception_blob() {
  int framesize_in_bytes;
  int framesize_in_words;
  int framesize_in_slots;
  { ResourceMark rm;
    MacroAssembler* masm = new MacroAssembler(new CodeBuffer(NULL, NULL));
    framesize_in_bytes = __ total_frame_size_in_bytes(exception_blob_words);
    framesize_in_words = framesize_in_bytes / wordSize;
    framesize_in_slots = framesize_in_bytes / sizeof(jint);
  }

  // Set an oopmap for the call site.  This oopmap will only be used if we
  // are unwinding the stack.  Hence, all locations will be dead.
  // Callee-saved registers will be the same as the frame above (i.e.,
  // handle_exception_stub), since they were restored when we got the
  // exception.
  ResourceMark rm;
  OopMapSet *oop_maps = new OopMapSet();
  // No callee-save registers to save here.
  oop_maps->add_gc_map( handle_exception_call_pc_offset, true, new OopMap(framesize_in_slots, 0));
  exception_blob()->set_oop_maps(oop_maps);

}

static void gen_new_frame(MacroAssembler* masm, bool deopt) {
//
// Common out the new frame generation for deopt and uncommon trap
//
  Register        G3pcs              = G3_scratch; // Array of new pcs (input)
  Register        Oreturn0           = O0;
  Register        Oreturn1           = O1;
  Register        O2UnrollBlock      = O2;
  Register        O3array            = O3;	   // Array of frame sizes (input)
  Register        O4array_size       = O4;	   // number of frames (input)
  Register        O7frame_size       = O7;	   // number of frames (input)

  __ ld_ptr(O3array, 0, O7frame_size);
  __ sub(G0, O7frame_size, O7frame_size);
  __ save(SP, O7frame_size, SP);
  __ ld_ptr(G3pcs, 0, I7);			// load frame's new pc

  #ifdef ASSERT
  // make sure that the frames are aligned properly
#ifndef _LP64
  __ btst(wordSize*2-1, SP);
  __ breakpoint_trap(Assembler::notZero);
#endif
  #endif

  // Deopt needs to pass some extra live values from frame to frame

  if (deopt) {
    __ mov(Oreturn0->after_save(), Oreturn0);
    __ mov(Oreturn1->after_save(), Oreturn1);
  }

  __ mov(O4array_size->after_save(), O4array_size);
  __ sub(O4array_size, 1, O4array_size);
  __ mov(O3array->after_save(), O3array);
  __ mov(O2UnrollBlock->after_save(), O2UnrollBlock);
  __ add(G3pcs, wordSize, G3pcs);		// point to next pc value

  #ifdef ASSERT
  // trash registers to show a clear pattern in backtraces
  __ set(0xDEAD0000, I0);
  __ add(I0,  2, I1);
  __ add(I0,  4, I2);
  __ add(I0,  6, I3);
  __ add(I0,  8, I4);
  // Don't touch I5 could have valuable savedSP
  __ set(0xDEADBEEF, L0);
  __ mov(L0, L1);
  __ mov(L0, L2);
  __ mov(L0, L3);
  __ mov(L0, L4);
  __ mov(L0, L5);

  // trash the return value as there is nothing to return yet
  __ set(0xDEAD0001, O7);
  #endif

  __ mov(SP, IsavedSP->after_restore());
}

static void make_new_frames(MacroAssembler* masm, bool deopt) {
// 
// loop through the UnrollBlock info and create new frames
//
  Register        G3pcs              = G3_scratch;
  Register        Oreturn0           = O0;
  Register        Oreturn1           = O1;
  Register        O2UnrollBlock      = O2;
  Register        O3array            = O3;
  Register        O4array_size       = O4;
  Label           loop;

  __ ld(Address(O2UnrollBlock, 0, Deoptimization::UnrollBlock::number_of_frames_offset_in_bytes()), O4array_size);
  __ ld_ptr(Address(O2UnrollBlock, 0, Deoptimization::UnrollBlock::frame_pcs_offset_in_bytes()), G3pcs);

  __ ld_ptr(Address(O2UnrollBlock, 0, Deoptimization::UnrollBlock::frame_sizes_offset_in_bytes()), O3array);

  // At this point we either have an interpreter frame or an compiled frame on the
  // the stack. If it is a compiled frame we push a new c2i adapter here
  Label adapter_done;
  __ ld(Address(O2UnrollBlock, 0, Deoptimization::UnrollBlock::new_adapter_offset_in_bytes()), O7);
  __ tst(O7);
  __ br(Assembler::zero, false, Assembler::pt, adapter_done);
  __ delayed()-> nop();

  // Allocate the frame, setup the registers

  gen_new_frame(masm, deopt);		           // Allocate the c2i adapter frame

  __ add(O3array, wordSize, O3array);        // point to next frame size

  __ bind(adapter_done);
  
  // Adjust old interpreter frame (or new c2i) to make space for new frame's extra java locals
  // 
  // We capture the original sp for the transition frame only because it is needed in
  // order to properly calculate interpreter_sp_adjustment. Even though in real life
  // every interpreter frame captures a savedSP it is only needed at the transition
  // (fortunately). If we had to have it correct everywhere then we would need to
  // be told the sp_adjustment for each frame we create. If the frame size array
  // were to have twice the frame count entries then we could have pairs [sp_adjustment, frame_size]
  // for each frame we create and keep up the illusion every where. 
  // 
  
  __ ld(Address(O2UnrollBlock, 0, Deoptimization::UnrollBlock::caller_adjustment_offset_in_bytes()), O7);
  __ mov(SP, IsavedSP->after_restore());	// remember initial sender's original sp before adjustment
  __ sub(SP, O7, SP);

  #ifdef ASSERT
  // make sure that there is at least one entry in the array
  __ tst(O4array_size);
  __ breakpoint_trap(Assembler::zero);
  #endif

  // Now push the new interpreter frames
  //

  __ bind(loop);

  // allocate a new frame, fillin the registers

  gen_new_frame(masm, deopt);        // allocate an interpreter frame

  __ tst(O4array_size);
  __ br(Assembler::notZero, false, Assembler::pn, loop);
  __ delayed()->add(O3array, wordSize, O3array);
  __ ld_ptr(G3pcs, 0, O7);                      // load final frame new pc

}


//------------------------------generate_deopt_blob----------------------------
// Ought to generate an ideal graph & compile, but here's some SPARC ASM
// instead.
void SharedRuntime::generate_deopt_blob() {
  // allocate space for the code
  ResourceMark rm;
  // setup code generation tools
  int pad = VerifyThread ? 512 : 0;// Extra slop space for more verify code
#ifdef _LP64
  CodeBuffer*     buffer             = new CodeBuffer(2000+pad, 512, 0, 0, 0, false);
#else
  // Measured 8/7/03 at 1212 in 32bit debug build (no VerifyThread)
  // Measured 8/7/03 at 1396 in 32bit debug build (VerifyThread)
  CodeBuffer*     buffer             = new CodeBuffer(1500+pad, 512, 0, 0, 0, false);
#endif /* _LP64 */
  MacroAssembler* masm               = new MacroAssembler(buffer);
  FloatRegister   Freturn0           = F0;
  Register        Greturn1           = G1;
  Register        Oreturn0           = O0;
  Register        Oreturn1           = O1;
  Register        O2UnrollBlock      = O2;
  Register        O3tmp              = O3;
  Register        I5exception_tmp    = I5;
  Register        G4exception_tmp    = G4_scratch;
  int             frame_size_words;
  Address         saved_Freturn0_addr(FP, 0, -sizeof(double) + STACK_BIAS);
  Address         saved_Greturn1_addr(FP, 0, -sizeof(double) -sizeof(jlong) + STACK_BIAS);
  Label           cont, skip_i2c;

  OopMapSet *oop_maps = new OopMapSet();

  //
  // This is the entry point for code which is returning to a de-optimized
  // frame.
  // The steps taken by this frame are as follows:
  //   - push a dummy "register_save" and save the return values (O0, O1, F0/F1, G1)
  //     and all potentially live registers (at a pollpoint many registers can be live).
  //
  //   - call the C routine: Deoptimization::fetch_unroll_info (this function
  //     returns information about the number and size of interpreter frames
  //     which are equivalent to the frame which is being deoptimized)
  //   - deallocate the unpack frame, restoring only results values. Other
  //     volatile registers will now be captured in the vframeArray as needed.
  //   - deallocate the deoptimization frame
  //   - in a loop using the information returned in the previous step
  //     push new interpreter frames (take care to propagate the return
  //     values through each new frame pushed)
  //   - create a dummy "unpack_frame" and save the return values (O0, O1, F0)
  //   - call the C routine: Deoptimization::unpack_frames (this function
  //     lays out values on the interpreter frame which was just created)
  //   - deallocate the dummy unpack_frame
  //   - ensure that all the return values are correctly set and then do
  //     a return to the interpreter entry point
  //
  // Refer to the following methods for more information:
  //   - Deoptimization::fetch_unroll_info
  //   - Deoptimization::unpack_frames

  OopMap* map = NULL;

  int start = __ offset();

  // restore G2, the trampoline destroyed it
  __ get_thread();

  // On entry we have been called by the deoptimized nmethod with a call that
  // replaced the original call (or safepoint polling location) so the deoptimizing
  // pc is now in O7. Return values are still in the expected places

  map = RegisterSaver::save_live_registers(masm, 0, &frame_size_words);
  __ ba(false, cont);
  __ delayed()->mov(Deoptimization::Unpack_deopt, I5exception_tmp);

  int exception_offset = __ offset() - start;

  // restore G2, the trampoline destroyed it
  __ get_thread();

  // On entry we jave been jumped to by the nmethod's (patched) exception handler
  // Or we have been jumped to by the exception blob.
  // O0 contains the exception oop and O7 contains the original exception pc.
  // So if we push a frame here it will look to the stack walking code (fetch_unroll_info)
  // just like a normal call so state will be extracted normally. We set the
  // exception temp so we remember that an exception is pending and there is
  // no result from the call just an exception oop. 
  // No need to update map as each call to save_live_registers will produce identical oopmap

  (void) RegisterSaver::save_live_registers(masm, 0, &frame_size_words);

  __ mov(Deoptimization::Unpack_exception, I5exception_tmp);

  __ bind(cont);

  __ set_last_Java_frame(SP, noreg);

  // do the call by hand so we can get the oopmap

  __ mov(G2_thread, L7_thread_cache);
  __ call(CAST_FROM_FN_PTR(address, Deoptimization::fetch_unroll_info), relocInfo::runtime_call_type);
  __ delayed()->mov(G2_thread, O0);

  // Set an oopmap for the call site this describes all our saved volatile registers

  oop_maps->add_gc_map( __ offset()-start, true, map);

  __ mov(L7_thread_cache, G2_thread);

  __ reset_last_Java_frame();

  // NOTE: we know that only O0/O1 will be reloaded by restore_result_registers
  // so this move will survive

  __ mov(I5exception_tmp, G4exception_tmp);

  __ mov(O0, O2UnrollBlock->after_save());

  RegisterSaver::restore_result_registers(masm);

  // deallocate the deoptimization frame taking care to preserve the return values
  __ mov(Oreturn0,     Oreturn0->after_save());
  __ mov(Oreturn1,     Oreturn1->after_save());
  __ mov(O2UnrollBlock, O2UnrollBlock->after_save());
  __ restore();

  // Pop an I2C/OSR adapter if desired
  __ ld(Address(O2UnrollBlock, 0, Deoptimization::UnrollBlock::adapter_size_offset_in_bytes()), O3tmp);
  __ tst(O3tmp);
  __ br(Assembler::zero, false, Assembler::pt, skip_i2c);
  __ delayed()-> nop();
  __ mov(Oreturn0,     Oreturn0->after_save());
  __ mov(Oreturn1,     Oreturn1->after_save());
  __ mov(O2UnrollBlock, O2UnrollBlock->after_save());
  //
  // If we only had to pop I2C adapters we'd be able to just do a vanilla restore
  // here and we'd be done. I2C adapters don't extend caller's stack. OSR adapters
  // which are really old interpreter frames do extend them so if we just
  // do a vanilla restore then the frame gets left extended. Of course OSR adapters
  // have the original SP in IsavedSP so we could use that. I2C adapters
  // don't (see bug 4962301) and we can't tell them apart here. However since
  // adapter_size tells us how much to adjust SP to get back to the unextended
  // value we use it in the unusual restore we have here to get things cleaned up.

  __ restore(SP, O3tmp, SP);			// Actually pop I2C/OSR frame here

  __ bind(skip_i2c);

  // Allocate new interpreter frame(s) and possible c2i adapter frame

  make_new_frames(masm, true);

  // push a dummy "unpack_frame" taking care of float return values and
  // call Deoptimization::unpack_frames to have the unpacker layout
  // information in the interpreter frames just created and then return
  // to the interpreter entry point
  __ save(SP, -frame_size_words*wordSize, SP);
  __ stf(FloatRegisterImpl::D, Freturn0, saved_Freturn0_addr);
#if !defined(_LP64)
  // 32-bit 1-register longs return longs in G1
  __ stx(Greturn1, saved_Greturn1_addr);
  __ set_last_Java_frame(SP, noreg);
  __ call_VM_leaf(L7_thread_cache, CAST_FROM_FN_PTR(address, Deoptimization::unpack_frames), G2_thread, G4exception_tmp);
#else
  // LP64 uses g4 in set_last_Java_frame
  __ mov(G4exception_tmp, O1);
  __ set_last_Java_frame(SP, G0);
  __ call_VM_leaf(L7_thread_cache, CAST_FROM_FN_PTR(address, Deoptimization::unpack_frames), G2_thread, O1);
#endif
  __ reset_last_Java_frame();
  __ ldf(FloatRegisterImpl::D, saved_Freturn0_addr, Freturn0);
#if !defined(_LP64)
  // 32-bit 1-register longs return longs in G1, but interpreter wants them 
  // in O0/O1
  Label not_long;
  __ cmp(O0,T_LONG);
  __ br(Assembler::notEqual, false, Assembler::pt, not_long);
  __ delayed()->nop();
  __ ldd(saved_Greturn1_addr,I0);
  __ bind(not_long);
#endif
  __ ret();
  __ delayed()->restore();

  masm->flush();
  _deopt_blob = DeoptimizationBlob::create(buffer, oop_maps, 0, exception_offset, 0, frame_size_words);
}


//------------------------------generate_uncommon_trap_blob--------------------
// Ought to generate an ideal graph & compile, but here's some SPARC ASM
// instead.
void OptoRuntime::generate_uncommon_trap_blob() {
  // allocate space for the code
  ResourceMark rm;
  // setup code generation tools
  int pad = VerifyThread ? 512 : 0;
#ifdef _LP64
  CodeBuffer*   buffer               = new CodeBuffer(1200+pad, 512, 0, 0, 0, false);
#else
  // Measured 8/7/03 at 660 in 32bit debug build (no VerifyThread)
  // Measured 8/7/03 at 1028 in 32bit debug build (VerifyThread)
  CodeBuffer*   buffer               = new CodeBuffer(2000+pad, 512, 0, 0, 0, false);
#endif
  MacroAssembler* masm               = new MacroAssembler(buffer);
  Register        O2UnrollBlock      = O2;
  Register        O3tmp              = O3;
  Register        O2klass_index      = O2;
  Label           skip_i2c;

  //
  // This is the entry point for all traps the compiler takes when it thinks
  // it cannot handle further execution of compilation code. The frame is
  // deoptimized in these cases and converted into interpreter frames for
  // execution
  // The steps taken by this frame are as follows:
  //   - push a fake "unpack_frame"
  //   - call the C routine Deoptimization::uncommon_trap (this function
  //     packs the current compiled frame into vframe arrays and returns
  //     information about the number and size of interpreter frames which 
  //     are equivalent to the frame which is being deoptimized)
  //   - deallocate the "unpack_frame"
  //   - deallocate the deoptimization frame
  //   - If removing a paired I2C/C2I, deallocate the I2C frame.
  //   - in a loop using the information returned in the previous step
  //     push interpreter frames; includes a C2I frame if needed.
  //   - create a dummy "unpack_frame"
  //   - call the C routine: Deoptimization::unpack_frames (this function
  //     lays out values on the interpreter frame which was just created)
  //   - deallocate the dummy unpack_frame
  //   - return to the interpreter entry point
  //
  //  Refer to the following methods for more information:
  //   - Deoptimization::uncommon_trap
  //   - Deoptimization::unpack_frame

  // the unloaded class index is in O0 (first parameter to this blob)

  // push a dummy "unpack_frame"
  // and call Deoptimization::uncommon_trap to pack the compiled frame into
  // vframe array and return the UnrollBlock information
  __ save_frame(0);
  __ set_last_Java_frame(SP, noreg);
  __ mov(I0, O2klass_index);
  __ call_VM_leaf(L7_thread_cache, CAST_FROM_FN_PTR(address, Deoptimization::uncommon_trap), G2_thread, O2klass_index);
  __ reset_last_Java_frame();
  __ mov(O0, O2UnrollBlock->after_save());
  __ restore();

  // deallocate the deoptimized frame taking care to preserve the return values
  __ mov(O2UnrollBlock, O2UnrollBlock->after_save());
  __ restore();

  // Pop an I2C adapter if desired
  __ ld(Address(O2UnrollBlock, 0, Deoptimization::UnrollBlock::adapter_size_offset_in_bytes()), O3tmp);
  __ tst(O3tmp);
  __ br(Assembler::zero, false, Assembler::pt, skip_i2c); // boolean (not pointer) check
  __ delayed()->nop();
  __ mov(O2UnrollBlock, O2UnrollBlock->after_save());
  //
  // If we only had to pop I2C adapters we'd be able to just do a restore here
  // and we'd be done. I2C adapters don't extend caller's stack. OSR adapters
  // which are really old interpreter frames do extend them so if we just
  // do a restore then the frame gets left extended. Of course OSR adapters
  // have the original SP in IsavedSP so we could use that. I2C adapters
  // don't (see bug xxxx) and we can't tell them apart here. However since
  // adapter_size tells us how much to adjust SP to get back to the unextended
  // value we use it to get things cleaned up.

  __ restore(SP, O3tmp, SP);			// Actually pop I2C/OSR frame here

  __ bind(skip_i2c);

  // Allocate new interpreter frame(s) and possible c2i adapter frame

  make_new_frames(masm, false);

  // push a dummy "unpack_frame" taking care of float return values and
  // call Deoptimization::unpack_frames to have the unpacker layout
  // information in the interpreter frames just created and then return
  // to the interpreter entry point
  __ save_frame(0);
  __ set_last_Java_frame(SP, noreg);
  __ mov(Deoptimization::Unpack_uncommon_trap, O3); // indicate it is the uncommon trap case
  __ call_VM_leaf(L7_thread_cache, CAST_FROM_FN_PTR(address, Deoptimization::unpack_frames), G2_thread, O3);
  __ reset_last_Java_frame();
  __ ret();
  __ delayed()->restore();

  masm->flush();
  _uncommon_trap_blob = UncommonTrapBlob::create(buffer, NULL, __ total_frame_size_in_bytes(0)/wordSize);
}

//------------------------------generate_handler_blob-------------------
//
// Generate a special Compile2Runtime blob that saves all registers, and sets
// up an OopMap.
//
// This blob is jumped to (via a breakpoint and the signal handler) from a
// safepoint in a thread code buffer.  On entry to this blob, O7 contains the
// address in the original nmethod at which we should resume normal execution.
// Thus, this blob looks like a subroutine which must preserve lots of
// registers and return normally.  Note that O7 is never register-allocated,
// so it is guaranteed to be free here.
//

// The hardest part of what this blob must do is to save the 64-bit %o
// registers in the 32-bit build.  A simple 'save' turn the %o's to %i's and
// an interrupt will chop off their heads.  Making space in the caller's frame
// first will let us save the 64-bit %o's before save'ing, but we cannot hand
// the adjusted FP off to the GC stack-crawler: this will modify the caller's
// SP and mess up HIS OopMaps.  So we first adjust the caller's SP, then save
// the 64-bit %o's, then do a save, then fixup the caller's SP (our FP).
// Tricky, tricky, tricky...

SafepointBlob* OptoRuntime::generate_handler_blob(address call_ptr, bool cause_return) {
  assert (StubRoutines::forward_exception_entry() != NULL, "must be generated before");  

  // allocate space for the code
  ResourceMark rm;
  // setup code generation tools  
  // Measured 8/7/03 at 896 in 32bit debug build (no VerifyThread)
  // Measured 8/7/03 at 1080 in 32bit debug build (VerifyThread)
  // even larger with TraceJumps
  int pad = TraceJumps ? 512 : 0;
  CodeBuffer*     buffer              = new CodeBuffer(1600 + pad, 512, 0, 0, 0, false);
  MacroAssembler* masm                = new MacroAssembler(buffer);
  int             frame_size_words;
  OopMapSet *oop_maps = new OopMapSet();
  OopMap* map = NULL;
  
  int start = __ offset();

  // If this causes a return before the processing, then do a "restore"
  if (cause_return) {
    __ restore();
  }

  map = RegisterSaver::save_live_registers(masm, 0, &frame_size_words);

  // setup last_Java_sp (blows G4)
  __ set_last_Java_frame(SP, noreg);

  // call into the runtime to handle illegal instructions exception
  // Do not use call_VM_leaf, because we need to make a GC map at this call site.
  __ mov(G2_thread, O0);
  __ save_thread(L7_thread_cache);
  __ call(call_ptr);
  __ delayed()->nop();

  // Set an oopmap for the call site.
  // We need this not only for callee-saved registers, but also for volatile
  // registers that the compiler might be keeping live across a safepoint.

  oop_maps->add_gc_map( __ offset() - start, true, map);

  __ restore_thread(L7_thread_cache);
  // clear last_Java_sp
  __ reset_last_Java_frame();

  // If the VM result is non-zero then it is the destination to continue at.
  // This happens when the original nmethod was deoptimized and we safepointed
  // at a call (i.e. non polling safepoint ) since the original call instruction
  // could have been overwritten the vm provides us with the destination
  // address. However if there is an exception pending it takes precedence
  // and we don't execute call. 

  // Check for exceptions
  Label pending;

  __ ld_ptr(G2_thread, in_bytes(Thread::pending_exception_offset()), O1);
  __ tst(O1);
  __ brx(Assembler::notEqual, true, Assembler::pn, pending);
  __ delayed()->nop();

  Label skip;

  __ tst(O0);
  __ brx(Assembler::notEqual, true, Assembler::pn, skip);
  __ delayed()->sub(O0, frame::pc_return_offset, I7);
  __ bind(skip);

  RegisterSaver::restore_live_registers(masm);

  // We are back the the original state on entry and ready to go.

  __ retl();
  __ delayed()->nop();

  // Pending exception after the safepoint

  __ bind(pending);

  RegisterSaver::restore_live_registers(masm);

  // We are back the the original state on entry.

  // Tail-call forward_exception_entry, with the issuing PC in O7,
  // so it looks like the original nmethod called forward_exception_entry.
  __ set((intptr_t)StubRoutines::forward_exception_entry(), O0);
  __ JMP(O0, 0);
  __ delayed()->nop();

  // -------------
  // make sure all code is generated
  masm->flush();  


  // return exception blob
  return SafepointBlob::create(buffer, oop_maps, frame_size_words);
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

void OptoRuntime::pd_unwind_stack(JavaThread *thread, frame fr, RegisterMap* reg_map) {
}
