#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)runtime_ia64.cpp	1.23 04/03/08 11:15:08 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_runtime_ia64.cpp.incl"

// Deoptimization

#define __ masm->

UncommonTrapBlob   *OptoRuntime::_uncommon_trap_blob;
DeoptimizationBlob *SharedRuntime::_deopt_blob;
ExceptionBlob      *OptoRuntime::_exception_blob;
SafepointBlob      *OptoRuntime::_illegal_instruction_handler_blob;
SafepointBlob      *OptoRuntime::_polling_page_safepoint_handler_blob;
SafepointBlob      *OptoRuntime::_polling_page_return_handler_blob;

int handle_exception_call_pc_offset = 0;

static const int exception_blob_words = 0;


class RegisterSaver {

  friend class MacroAssembler;

  public:

  static OopMap* save_live_registers(MacroAssembler* masm, int* total_frame_words);
  static void restore_live_registers(MacroAssembler* masm);

  // During deoptimization only the result register need to be restored
  // all the other values have already been extracted.

  static void restore_result_registers(MacroAssembler* masm);
};

#define ADD_CALLEE_TO_MAP(offset, regname, regHname)                                         \
  map->set_callee_saved(SharedInfo::stack2reg((MacroAssembler::offset*wordSize) >> 2),       \
                        frame_size_in_slots, 0, OptoReg::Name(regname));                     \
  map->set_callee_saved(SharedInfo::stack2reg(((MacroAssembler::offset*wordSize) + 4) >> 2), \
                        frame_size_in_slots, 0, OptoReg::Name(regHname));                    \


OopMap* RegisterSaver::save_live_registers(MacroAssembler* masm, int* total_frame_words) {

  int frame_size_in_bytes = MacroAssembler::push_regs_size*wordSize;

  assert(frame_size_in_bytes == round_to(MacroAssembler::push_regs_size*wordSize, 16), 
         "Not 16 byte aligned");
  
  // OopMap frame size is in compiler stack slots (jint's) not bytes or words
  int frame_size_in_slots = frame_size_in_bytes / BytesPerInt;

  int frame_size_in_words = frame_size_in_bytes / wordSize;
  *total_frame_words = frame_size_in_words;

  // We can use GR31_SCRATCH here since we are coming from compiled
  // code.  GR31 is a very local temp register in ia64.ad.
  // If it's use changes, this code will need to change.
  __ mov(GR31_SCRATCH, RP);
  __ push_dummy_full_frame(GR31_SCRATCH);  
  __ push_scratch_regs();

  // Set an oopmap for the call site.  This oopmap will map all
  // oop-registers and debug-info registers as callee-saved.  This
  // will allow deoptimization at this safepoint to find all possible
  // debug-info recordings, as well as let GC find all oops.

  OopMapSet *oop_maps = new OopMapSet();
  OopMap* map = new OopMap(frame_size_in_slots, 0);

  ADD_CALLEE_TO_MAP(gr2_off, GR2_num, GR2H_num);
  ADD_CALLEE_TO_MAP(gr3_off, GR3_num, GR3H_num);
  ADD_CALLEE_TO_MAP(gr8_off, GR8_RET_num, GR8_RETH_num);
  ADD_CALLEE_TO_MAP(gr9_off, GR9_num, GR9H_num);
  ADD_CALLEE_TO_MAP(gr10_off, GR10_num, GR10H_num);
  ADD_CALLEE_TO_MAP(gr11_off, GR11_num, GR11H_num);
  ADD_CALLEE_TO_MAP(gr14_off, GR14_num, GR14H_num);
  ADD_CALLEE_TO_MAP(gr15_off, GR15_num, GR15H_num);
  ADD_CALLEE_TO_MAP(gr16_off, GR16_num, GR16H_num);
  ADD_CALLEE_TO_MAP(gr17_off, GR17_num, GR17H_num);
  ADD_CALLEE_TO_MAP(gr18_off, GR18_num, GR18H_num);
  ADD_CALLEE_TO_MAP(gr19_off, GR19_num, GR19H_num);
  ADD_CALLEE_TO_MAP(gr20_off, GR20_num, GR20H_num);
  ADD_CALLEE_TO_MAP(gr21_off, GR21_num, GR21H_num);
  ADD_CALLEE_TO_MAP(gr22_off, GR22_num, GR22H_num);
  ADD_CALLEE_TO_MAP(gr23_off, GR23_num, GR23H_num);
  ADD_CALLEE_TO_MAP(gr24_off, GR24_num, GR24H_num);
  ADD_CALLEE_TO_MAP(gr25_off, GR25_num, GR25H_num);
  ADD_CALLEE_TO_MAP(gr26_off, GR26_num, GR26H_num);
  ADD_CALLEE_TO_MAP(gr27_off, GR27_num, GR27H_num);
  ADD_CALLEE_TO_MAP(gr28_off, GR28_num, GR28H_num);
  ADD_CALLEE_TO_MAP(gr29_off, GR29_num, GR29H_num);
  ADD_CALLEE_TO_MAP(gr30_off, GR30_num, GR30H_num);
  ADD_CALLEE_TO_MAP(gr31_off, GR31_num, GR31H_num);

  ADD_CALLEE_TO_MAP(fr6_off, FR6_num, FR6H_num);
  ADD_CALLEE_TO_MAP(fr7_off, FR7_num, FR7H_num);
  ADD_CALLEE_TO_MAP(fr8_off, FR8_num, FR8H_num);
  ADD_CALLEE_TO_MAP(fr9_off, FR9_num, FR9H_num);
  ADD_CALLEE_TO_MAP(fr10_off, FR10_num, FR10H_num);
  ADD_CALLEE_TO_MAP(fr11_off, FR11_num, FR11H_num);
  ADD_CALLEE_TO_MAP(fr12_off, FR12_num, FR12H_num);
  ADD_CALLEE_TO_MAP(fr13_off, FR13_num, FR13H_num);
  ADD_CALLEE_TO_MAP(fr14_off, FR14_num, FR14H_num);
  ADD_CALLEE_TO_MAP(fr15_off, FR15_num, FR15H_num);

  return map;
}


// Pop the current frame and restore all the registers that we
// saved.
void RegisterSaver::restore_live_registers(MacroAssembler* masm) {
  __ pop_scratch_regs();
  // pop_dummy_full_frame is going to use GR2, save it in GR31_SCRATCH
  __ mov(GR31_SCRATCH, GR2);
  __ pop_dummy_full_frame();
  __ mov(GR2, GR31_SCRATCH);
}

// Pop the current frame and restore the registers that might be holding
// a result.
void RegisterSaver::restore_result_registers(MacroAssembler* masm) {

}

//------------------------------setup_exception_blob---------------------------
// creates exception blob at the end
// Using exception blob, this code is jumped from a compiled method.
// (see emit_exception_handler in ia64.ad file)
// It is also the default exception destination for c2i and i2c adapters
// 
// Given an exception pc at a call, we may unwind the stack and jump to
// caller's exception handler. We may also continue into the handler
// This code is entered with a jmp.
// 
// Arguments:
//   GR_exception: exception oop (R8)
//   GR_issuing_pc: exception pc (R9)
//
// Results:
//   GR_exception: exception oop
//   O1: exception pc in caller or ??? QQQ
//   destination: exception handler of caller
// 
// Note: the exception pc MUST be at a call (precise debug information)
//
// NOTE: NO Registers except the above mention argument/results can be modified.
//
void OptoRuntime::setup_exception_blob() {
  // allocate space for code
  ResourceMark rm;

  // setup code generation tools
  CodeBuffer*    buffer   = new CodeBuffer(4096, 512, 0, 0, 0, false);
  MacroAssembler* masm    = new MacroAssembler(buffer);

  Label exception_handler_found;
  Label L;

  address start = __ pc();

//  __ verify_thread();

  // We enter using the window of the compiled code that has just
  // generated an exception. We have just completed a call so any
  // SOC registers are fair game as temps here (just like other
  // platforms)

  // This dummy frame must store the link to the "caller" 
  // QQQ dependent on calling conventions.

  __ push_dummy_full_frame(GR9_issuing_pc); // Get us a new window

  const Register exception_oop_addr        = GR31_SCRATCH;
  const Register exception_pc_addr         = GR30_SCRATCH;
 
  __ add(exception_oop_addr, GR4_thread, in_bytes(JavaThread::exception_oop_offset()));
  __ add(exception_pc_addr,  GR4_thread, in_bytes(JavaThread::exception_pc_offset()));

  __ st8(exception_oop_addr, GR8_exception);
  __ st8(exception_pc_addr, GR9_issuing_pc);

  // This call does all the hard work. It checks if an exception handler
  // exists in the method.  
  // If so, it returns the handler address.
  // If not, it prepares for stack-unwinding, restoring the callee-save 
  // registers of the frame being removed.
  //

  handle_exception_call_pc_offset = __ set_last_Java_frame(SP);

  // Ought to be just a simple call

  __ call_VM_leaf(CAST_FROM_FN_PTR(address, OptoRuntime::handle_exception_C), GR4_thread);
  __ flush_bundle();

  // Reload GR7 which holds the register stack limit. 
  // We might be processing a register stack overflow and raised
  // the limit while we handle the exception.  If we've unwound
  // enough, the reguard_stack routine will set the limit back to
  // normal.  This is where we update the global register.
  __ add(GR7_reg_stack_limit, thread_(register_stack_limit));
  __ ld8(GR7_reg_stack_limit, GR7_reg_stack_limit);

  __ bind(L);
  __ reset_last_Java_frame();

  // We are still in our new frame
  __ pop_dummy_full_frame();

  // Back to original frame. Only use SOC registers here as scratch.

  __ cmp(PR6, PR0, GR_RET, GR0, Assembler::notEqual);
  __ br(PR6, exception_handler_found, Assembler::spnt);

  // No handler found in this frame
  //
  __ add(exception_oop_addr, GR4_thread, in_bytes(JavaThread::exception_oop_offset()));
  __ add(exception_pc_addr,  GR4_thread, in_bytes(JavaThread::exception_pc_offset()));

  __ ld8(GR8_exception, exception_oop_addr);
  __ ld8(GR9_issuing_pc, exception_pc_addr);

  // No handler was found for the current frame so we will pop this frame
  // and try again at thread->exception_handler_pc()

  const Register exception_handler_pc_addr = exception_oop_addr;

  __ add(exception_handler_pc_addr, GR4_thread, in_bytes(JavaThread::exception_handler_pc_offset()));
  __ ld8(GR_Lsave_RP, exception_handler_pc_addr);
  __ pop_thin_frame();
  __ ret();

  // Exception handler found.
  __ bind(exception_handler_found);

  // GR_RET has the handler address (may be deopt blob)

  const BranchRegister handler_br = BR6_SCRATCH;

  __ mov(handler_br, GR_RET);

  __ add(exception_oop_addr, GR4_thread, in_bytes(JavaThread::exception_oop_offset()));
  __ add(exception_pc_addr,  GR4_thread, in_bytes(JavaThread::exception_pc_offset()));

  __ ld8(GR8_exception, exception_oop_addr);
  __ ld8(GR9_issuing_pc, exception_pc_addr);

#ifdef ASSERT
  __ st8(exception_oop_addr, GR0);
  const Register exception_handler_pc_addr2 = exception_oop_addr;
 
  __ add(exception_handler_pc_addr2, GR4_thread, in_bytes(JavaThread::exception_handler_pc_offset()));

  __ st8(exception_handler_pc_addr, GR0);
  __ st8(exception_pc_addr, GR0);
#endif

  __ br(handler_br);

  // -------------
  // make sure all code is generated

  __ flush();

  // Fill out other meta info
  OopMapSet *oop_maps = NULL; // will be set later; currently the register stuff is not yet initialized!
  // _exception_blob = ExceptionBlob::create(buffer, oop_maps,  __ total_frame_size_in_bytes(0)/wordSize);
  _exception_blob = ExceptionBlob::create(buffer, oop_maps,  0);
  address xxxyyy = _exception_blob->instructions_begin();
}

//------------------------------fill_in_exception_blob-------------------------
void OptoRuntime::fill_in_exception_blob() {
  int framesize = 0;

  assert(handle_exception_stub() != NULL, "exception stub must have been generated");

  // Set an oopmap for the call site.  This oopmap will only be used if we
  // are unwinding the stack.  Hence, all locations will be dead.
  // Callee-saved registers will be the same as the frame above (i.e.,
  // handle_exception_stub), since they were restored when we got the
  // exception.
  ResourceMark rm;
  OopMapSet *oop_maps = new OopMapSet();
  // No callee-save registers to save here.
  oop_maps->add_gc_map( handle_exception_call_pc_offset, true, new OopMap(framesize, 0));
  exception_blob()->set_oop_maps(oop_maps);
}

//------------------------------generate_deopt_blob----------------------------
// Ought to generate an ideal graph & compile, but here's some IA64 ASM
// instead.
void SharedRuntime::generate_deopt_blob() {
  // allocate space for the code
  ResourceMark rm;
  // setup code generation tools
  CodeBuffer*     buffer             = new CodeBuffer(4096, 512, 0, 0, 0, false);
  MacroAssembler* masm               = new MacroAssembler(buffer);
  Register        GR10_exception_tmp     = GR10;
  Label           cont, loop, skip_i2c;

  OopMapSet *oop_maps = new OopMapSet();

  //
  // This is the entry point for code which is returning to a de-optimized
  // frame.
  // The steps taken by this frame are as follows:
  //   - push a dummy "unpack_frame" and save the return values (O0, O1, F0)
  //     which are coming in
  //   - call the C routine: Deoptimization::fetch_unroll_info (this function
  //     returns information about the number and size of interpreter frames
  //     which are equivalent to the frame which is being deoptimized)
  //   - deallocate the unpack frame
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

  address start = __ pc();

  // space to save live return results and abi scratch area
  int framesize = (round_to(sizeof(double)*2, 16) + round_to(sizeof(jlong)*2, 16))/wordSize + 2;

  // We have been "called" by the deoptimized nmethod. RP is wrong by one
  // bundle from what the original RP before we return to the deoptimized
  // method so we correct it and put it in a useful location.

  __ mov(GR9_issuing_pc, RP);
  __ add(GR9_issuing_pc, -BytesPerInstWord, GR9_issuing_pc);
  __ mov(GR10_exception_tmp, Deoptimization::Unpack_deopt);

  // because we have been "called" we are not in the deoptee's register window
  // we must go back to that window. Do a dummy return to remove the zero
  // length window the call created.

  __ return_from_dummy_frame();
  __ br(cont);
  __ flush_bundle();

  int exception_offset = __ pc() - start;

  // We reached here via a branch we are in the deoptee register window

  // GR9 has the original throwing PC 
  __ mov(GR10_exception_tmp, Deoptimization::Unpack_exception);

  __ bind(cont);

  //
  // frames: deoptee, possible_i2c, caller_of_deoptee

  // push a dummy "unpack_frame" taking care to preserve float return values
  // and call Deoptimization::fetch_unroll_info to get the UnrollBlock
  
  // GR9_issuing_pc contains the return point to the deoptimized nmethod 
  // that existed when the nmethod was deoptimized. We push that frame here
  // and the stack walking will think we are just about to return from that
  // call and do the correct thing.

  __ push_dummy_full_frame(GR9_issuing_pc);

  // frames: unpack frame, deoptee, possible_i2c, caller_of_deoptee
  // unpack frame has pushed caller's unat bits so spills here won't
  // cause any surprises

  __ mov(GR_Lstate, GR10_exception_tmp);                    // Save this where it is safe
  __ sub(SP, SP, round_to(sizeof(double)*2, 16));
  __ stfspill(SP, FR_RET);
  __ sub(SP, SP, round_to(sizeof(jlong)*2, 16));
  __ st8spill(SP, GR_RET);
  __ sub(SP, SP, 2*wordSize);				// ABI scratch

  // What about things like LC?  Saved/restored by push/pop_dummy_full_frame

  // Local register assignment

  const Register GR15_frame_sizes  = GR15;   // first the address, then the value
  const Register GR16_frame_count  = GR16;   // first the address, then the value
  const Register GR17_adapter_size = GR17;   // first the address, then the value
  const Register GR18_frame_pcs    = GR18;   // first the address, then the value

  // We need to set last_Java_frame because fetch_unroll_info will call last_Java_frame()
  // However we can't block and no GC will occur so we don't need an oopmap and the
  // value of the pc in the frame is not particularly important (just identify the blob)

  int offset;
  offset = __ set_last_Java_frame(SP);

  __ call_VM_leaf(CAST_FROM_FN_PTR(address, Deoptimization::fetch_unroll_info), GR4_thread);

  // Add an oopmap that describes the above call
  // At this point on ia64 there are no callee save register that need to be
  // described. If c2 ever has live callee save registers we would need to save them
  // before calling fetch_unroll_info and describe them in the oopmap. 
  // At the moment we have none so an empty oopmap is plenty.

  oop_maps->add_gc_map(offset, true, new OopMap( framesize, 0));

  __ reset_last_Java_frame();

  __ add(GR17_adapter_size, GR_RET /* info */, Deoptimization::UnrollBlock::adapter_size_offset_in_bytes());
  __ add(GR18_frame_pcs,    GR_RET /* info */, Deoptimization::UnrollBlock::frame_pcs_offset_in_bytes());
  __ add(GR15_frame_sizes,  GR_RET /* info */, Deoptimization::UnrollBlock::frame_sizes_offset_in_bytes());
  __ add(GR16_frame_count,  GR_RET /* info */, Deoptimization::UnrollBlock::number_of_frames_offset_in_bytes());

  __ mov(GR10_exception_tmp, GR_L8);                    // and restore to where we keep it
  __ add(SP, SP, 2*wordSize);				// ABI area
  __ ld8fill(GR_RET, SP);
  __ add(SP, SP, round_to(sizeof(jlong)*2, 16));
  __ ldffill(FR_RET, SP);
  __ add(SP, SP, round_to(sizeof(double)*2, 16));

  __ pop_dummy_full_frame();

  // frames: deoptee, possible_i2c, caller_of_deoptee

  __ ld4(GR17_adapter_size, GR17_adapter_size);
  __ ld8(GR15_frame_sizes,  GR15_frame_sizes);
  __ ld8(GR18_frame_pcs,    GR18_frame_pcs);
  __ ld4(GR16_frame_count,  GR16_frame_count);

  // Now we must pop the frame we are deoptimizing

  __ pop_dummy_thin_frame();  // Not really a dummy, but junk now.

  // frames: possible_i2c, caller_of_deoptee

  // We now have in LC and UNAT the values that we there on entry to the deoptimized frame
  // We will propagate them to all the frames we create only the original caller of
  // the deoptimized frame will care (and if that caller is an i2c we will pop it and
  // used its LC and UNAT instead).
  // Seems like things like r4-r7 are ok because Java never touches them so they should
  // already be saved/restored by non-Java code and their values here should be correct
  // already.

  __ cmp4(PR6, PR0, 0, GR17_adapter_size, Assembler::equal);
  __ br(PR6, skip_i2c, Assembler::dptk);
  __ pop_dummy_thin_frame();  // Pop the adapter frame

  __ bind(skip_i2c);

  // frames: caller_of_deoptee

  // loop through the UnrollBlock info and create interpreter frames
  // Everything must be done in global registers because the windows
  // will be changing constantly

  #ifdef ASSERT
  // make sure that there is at least one entry in the array
  __ cmp4(PR6, PR0, 0, GR16_frame_count, Assembler::equal);
  __ breaki(PR6, 1);
  #endif

  // QQQ what about adjusting the caller's stack and getting the sender_sp correct!!!

  // We could use LC but then we'd have to shuffle it to save the preserved value.
  // Doesn't seem to be worth the bother

  const Register GR19_pc  = GR19;

  __ bind(loop);

  __ ld8(GR2, GR15_frame_sizes, sizeof(intptr_t));      // Get size of next memory frame to allocate
  __ ld8(GR19_pc, GR18_frame_pcs, sizeof(address));     // Get size of next return address to store
  __ sub(GR16_frame_count, GR16_frame_count, 1);

  __ push_dummy_full_frame(GR19_pc);                     // Allocate register stack side of the frame

  #ifdef ASSERT
    // trash local registers to show a clear pattern in backtraces
    // __ mov(GR_L0, 0xDEAD00); GR_Lsave_SP
    // __ mov(GR_L1, 0xDEAD01); GR_Lsave_caller_BSP
    // __ mov(GR_L2, 0xDEAD02); GR_Lsave_PFS
    // __ mov(GR_L3, 0xDEAD03); GR_Lsave_RP
    // __ mov(GR_L4, 0xDEAD04); GR_Lsave_LC
    // __ mov(GR_L5, 0xDEAD05); GR_Lsave_GP
    // __ mov(GR_L6, 0xDEAD06); GR_Lsave_UNAT
    // __ mov(GR_L7, 0xDEAD07); GR_Lsave_PR
    __ mov(GR_L8, 0xDEAD08);  //GR_Lstate
    __ mov(GR_L9, 0xDEAD09);  //GR_Lscratch0
    __ mov(GR_L10, 0xDEAD10); //GR_Lmethod_addr
    __ mov(GR_L11, 0xDEAD11);
    __ mov(GR_L12, 0xDEAD12);
    __ mov(GR_L13, 0xDEAD13);
    __ mov(GR_L14, 0xDEAD14);
    __ mov(GR_L15, 0xDEAD15);

  #endif
  __ sub(SP, SP, GR2);                                  // Allocate memory stack side of the frame

  __ cmp4(PR0, PR6, 0, GR16_frame_count, Assembler::equal);
  __ br(PR6, loop, Assembler::dptk);

  // frames: new skeletal interpreter frame(s), possible_c2_i,  caller_of_deoptee

  // push a dummy "unpack_frame" taking care of float/int return values and
  // call Deoptimization::unpack_frames to have the unpacker layout
  // information in the interpreter frames just created and then return
  // to the interpreter entry point

  __ ld8(GR19_pc, GR18_frame_pcs, sizeof(address));     // Get return address we want to store in the new frame

  __ push_dummy_full_frame(GR19_pc);                     // this will be a return to the frame manager (somewhere)

  // frames: unpack_frame, new skeletal interpreter frame(s), possible_c2_i,  caller_of_deoptee

  // Flush the windows to the stack. For Itanium2 we will have to also turn off eager filling
  // so that the windows don't get filled while we're in the middle of changing them!

  // We must be sure that the current windows flush so flushrs inline
  // will not be sufficient. The fact that the current window refills
  // is not important since we just want to be certain that the
  // stack is walkable and it will be.

  __ sub(SP, SP, round_to(sizeof(double)*2, 16));
  __ stfspill(SP, FR_RET);
  __ sub(SP, SP, round_to(sizeof(jlong)*2, 16));
  __ st8spill(SP, GR_RET);
  __ sub(SP, SP, 2*wordSize);				// ABI

  __ set_last_Java_frame(SP);
  __ call_VM_leaf((address)StubRoutines::ia64::flush_register_stack());

  __ call_VM_leaf(CAST_FROM_FN_PTR(address, Deoptimization::unpack_frames), GR4_thread, GR10_exception_tmp);
  __ reset_last_Java_frame();

  __ add(SP, SP, 2*wordSize);				// ABI
  __ ld8fill(GR_RET, SP);
  __ add(SP, SP, round_to(sizeof(jlong)*2, 16));
  __ ldffill(FR_RET, SP);
  __ add(SP, SP, round_to(sizeof(double)*2, 16));

  // We should be in the newly constructed top frame of the interpreter (frame_manager)

  __ pop_full_frame();

  // frames: new skeletal interpreter frame(s), possible_c2_i,  caller_of_deoptee
  __ ret();

  __ flush();
  _deopt_blob = DeoptimizationBlob::create(buffer, oop_maps, 0, exception_offset, 0, framesize);
}


//------------------------------generate_uncommon_trap_blob--------------------
// Ought to generate an ideal graph & compile, but here's some IA64 ASM
// instead.
void OptoRuntime::generate_uncommon_trap_blob() {
  // allocate space for the code
  ResourceMark rm;
  // setup code generation tools
  CodeBuffer*     buffer             = new CodeBuffer(2048, 512, 0, 0, 0, false);
  MacroAssembler* masm                = new MacroAssembler(buffer);
  Label           cont, loop, skip_i2c;

  const Register GR15_klass_index     = GR15;
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

  enum frame_layout {
    F8_off,
    F8_nat_off,
    framesize
  };

  // the unloaded class index is in O0 (first parameter to this blob)

  // push a dummy "unpack_frame"
  // and call Deoptimization::uncommon_trap to pack the compiled frame into
  // vframe array and return the UnrollBlock information

  // push a dummy "unpack_frame" taking care to preserve float return values
  // and call Deoptimization::fetch_unroll_info to get the UnrollBlock

  // Weird on sparc this frame isn't really a dummy at all

  
  const Register dummy  = GR3_SCRATCH;

  // This is lame but true. We need to allocate space on the stack so that deoptimization
  // will walk passed this frame and find the stack space above this one because
  // the code for deopt does a while loop looking for an sp that matches and then expects
  // the pc to be for the frame we want deoptimized. Since on ia64 we can have a frame
  // that uses no stack space (but register stack space) the code fails. Simplest thing
  // is to just allocate some dummy space and go with the flow.

  __ mov(dummy, RP);            // Make the return address be the sender
  __ save_full_frame(framesize * sizeof(intptr_t));

  // Spill the live registers

  __ stfspill(SP, FR8);		// ACK QQQ in ABI area...

  // Set an oopmap for the call site
  OopMapSet *oop_maps = new OopMapSet();
  OopMap* map =  new OopMap( framesize, 0 );

  int offset;
  offset = __ set_last_Java_frame(SP);

  __ mov(GR15_klass_index, GR_I0);
  __ call_VM_leaf(CAST_FROM_FN_PTR(address, Deoptimization::uncommon_trap), GR4_thread, GR15_klass_index);
  __ reset_last_Java_frame();

  // Add an oopmap that describes the above call

  map->set_callee_saved( SharedInfo::stack2reg(F8_off   ), framesize,0, OptoReg::Name(FR8_num  ) );
  oop_maps->add_gc_map(offset, true, map);

  // Local register assignment

  const Register GR15_frame_sizes   = GR15;   // first the address, then the value
  const Register GR16_frame_count   = GR16;   // first the address, then the value
  const Register GR17_adapter_size  = GR17;   // first the address, then the value
  const Register GR18_frame_pcs     = GR18;   // first the address, then the value

  __ add(GR17_adapter_size, GR_RET /* info */, Deoptimization::UnrollBlock::adapter_size_offset_in_bytes());
  __ add(GR15_frame_sizes,  GR_RET /* info */, Deoptimization::UnrollBlock::frame_sizes_offset_in_bytes());
  __ add(GR18_frame_pcs,    GR_RET /* info */, Deoptimization::UnrollBlock::frame_pcs_offset_in_bytes());
  __ add(GR16_frame_count,  GR_RET /* info */, Deoptimization::UnrollBlock::number_of_frames_offset_in_bytes());
  __ pop_dummy_full_frame();
  // our frame is gone now.

  __ ld4(GR17_adapter_size, GR17_adapter_size);
  __ ld8(GR15_frame_sizes, GR15_frame_sizes);
  __ ld8(GR18_frame_pcs, GR18_frame_pcs);
  __ ld4(GR16_frame_count, GR16_frame_count);

  // %%%%%%% may have to setup some oop maps for the call site if G/O
  //         registers are used in code

  // Now we must pop the frame we are deoptimizing

  __ pop_dummy_thin_frame();  // Not really a dummy, but junk now. (Assumes the pops SP correctly too QQQ)

  // We now have in LC and UNAT the values that we there on entry to the deoptimized frame
  // We will propagate them to all the frames we create only the original caller of
  // the deoptimized frame will care (and if that caller is an i2c we will pop it and
  // used its LC and UNAT instead).
  // Seems like things like r4-r7 are ok because Java never touches them so they should
  // already be saved/restored by non-Java code and their values here should be correct
  // already.

  __ cmp4(PR6, PR0, 0, GR17_adapter_size, Assembler::equal);
  __ br(PR6, skip_i2c, Assembler::dptk);
  __ pop_dummy_thin_frame();  // Pop the adapter frame (Assumes the pops SP correctly too QQQ)

  __ bind(skip_i2c);

  // We are now in the frame that will become an interpreter frame
  // the RP is either the call_stub or the frame manager
  
  // loop through the UnrollBlock info and create interpreter frames
  // Everything must be done in global registers because the windows
  // will be changing constantly

  #ifdef ASSERT
  // make sure that there is at least one entry in the array
  __ cmp4(PR6, PR0, 0, GR16_frame_count, Assembler::equal);
  __ breaki(PR6, 1);
  #endif

  // QQQ what about adjusting the caller's stack and getting the sender_sp correct!!!

  // We could use LC but then we'd have to shuffle it to save the preserved value.
  // Doesn't seem to be worth the bother

  const Register GR19_pc  = GR19_SCRATCH;

  __ bind(loop);

  __ ld8(GR8, GR15_frame_sizes, sizeof(intptr_t));     // Get size of next memory frame to allocate
  __ ld8(GR19_pc, GR18_frame_pcs, sizeof(address));    // Get return address we want to store in the new frame
  __ sub(GR16_frame_count, GR16_frame_count, 1);
  __ push_dummy_full_frame(GR19_pc);            // Allocate register stack side of the frame 

  #ifdef ASSERT
  // trash local registers to show a clear pattern in backtraces
  // __ mov(GR_L0, 0xDEAD00); GR_Lsave_SP
  // __ mov(GR_L1, 0xDEAD01); GR_Lsave_caller_BSP
  // __ mov(GR_L2, 0xDEAD02); GR_Lsave_PFS
  // __ mov(GR_L3, 0xDEAD03); GR_Lsave_RP
  // __ mov(GR_L4, 0xDEAD04); GR_Lsave_LC
  // __ mov(GR_L5, 0xDEAD05); GR_Lsave_GP
  // __ mov(GR_L6, 0xDEAD06); GR_Lsave_UNAT
  // __ mov(GR_L7, 0xDEAD07); GR_Lsave_PR
  __ mov(GR_L8, 0xDEAD08);  //GR_Lstate
  __ mov(GR_L9, 0xDEAD09);  //GR_Lscratch0
  __ mov(GR_L10, 0xDEAD10); //GR_Lmethod_addr
  __ mov(GR_L11, 0xDEAD11);
  __ mov(GR_L12, 0xDEAD12);
  __ mov(GR_L13, 0xDEAD13);
  __ mov(GR_L14, 0xDEAD14);
  __ mov(GR_L15, 0xDEAD15);
  #endif

  __ sub(SP, SP, GR8);                                 // Allocate memory stack side of the frame

  __ cmp4(PR0, PR6, 0, GR16_frame_count, Assembler::equal);
  __ br(PR6, loop, Assembler::dptk);

  // push a dummy "unpack_frame" taking care of float/int return values and
  // call Deoptimization::unpack_frames to have the unpacker layout
  // information in the interpreter frames just created and then return
  // to the interpreter entry point

  // Flush the windows to the stack. For Itanium2 we will have to also turn off eager filling
  // so that the windows don't get filled while we're in the middle of changing them!

  __ ld8(GR19_pc, GR18_frame_pcs, sizeof(address));    // Get return address we want to store in the new frame

  __ push_dummy_full_frame(GR19_pc);                    // Will return to uncommon trap entry point in frame manager
  __ sub(SP, SP, framesize*wordSize);                  // Allocate the proper frame size for this blob
						       // so stack walking will work properly.

  // We must be sure that the current windows flush so flushrs inline
  // will not be sufficient. The fact that the current window refills
  // is not important since we just want to be certain that the
  // stack is walkable and it will be.
  __ call_VM_leaf((address)StubRoutines::ia64::flush_register_stack());

  __ set_last_Java_frame(SP);
  __ mov(GR2, Deoptimization::Unpack_uncommon_trap);
  __ call_VM_leaf(CAST_FROM_FN_PTR(address, Deoptimization::unpack_frames), GR4_thread, GR2);
  __ reset_last_Java_frame();

  // We should be in the newly constructed top frame of the interpreter (frame_manager)

  __ pop_full_frame();
  __ ret();

  __ flush();
  _uncommon_trap_blob = UncommonTrapBlob::create(buffer, oop_maps, framesize);

}


//------------------------------generate_handler_blob-------------------
//
// Generate a special Compile2Runtime blob that saves all registers, and sets
// up an OopMap.
//
// This blob is jumped to (via a breakpoint and the signal handler) from a
// safepoint in a thread code buffer.  On entry to this blob, RP contains the
// address in the original nmethod at which we should resume normal execution.
// Thus, this blob looks like a subroutine which must preserve lots of
// registers and return normally.  Note that RP is never register-allocated,
// so it is guaranteed to be free here.
//

SafepointBlob* OptoRuntime::generate_handler_blob(address call_ptr, bool cause_return) {
  assert (StubRoutines::forward_exception_entry() != NULL, "must be generated before");  

  const Register pending_exception             = GR2_SCRATCH;
  const Register pending_exception_addr        = pending_exception;
  const Register continuation                  = pending_exception;
  const PredicateRegister is_pending_exception = PR6_SCRATCH;
  const PredicateRegister is_null              = is_pending_exception;
  const BranchRegister continuation_br         = BR6_SCRATCH;

  // allocate space for the code
  ResourceMark rm;
  CodeBuffer*     buffer              = new CodeBuffer(3000, 512, 0, 0, 0, false);
  MacroAssembler* masm                = new MacroAssembler(buffer);
  int             frame_size_words;
  OopMapSet *oop_maps = new OopMapSet();
  OopMap* map = NULL;
  

  // If this causes a return before the processing, pop the frame off
  if (cause_return) {
    __ pop_dummy_thin_frame();
  }

  map = RegisterSaver::save_live_registers(masm, &frame_size_words);

  // setup last_Java_sp and last_Java_pc
  int offset;
  offset = __ set_last_Java_frame(SP);

  // call into the runtime to handle the safepoint poll 
  __ call_VM_leaf(call_ptr, GR4_thread);

  // Set an oopmap for the call site.
  // We need this not only for callee-saved registers, but also for volatile
  // registers that the compiler might be keeping live across a safepoint.

  oop_maps->add_gc_map(offset, true, map);

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

  __ add(pending_exception_addr, thread_(pending_exception));
  __ ld8(pending_exception, pending_exception_addr);
  __ cmp(PR0, is_pending_exception, 0, pending_exception, Assembler::equal);
  __ br(is_pending_exception, pending);

  Label skip;

  // Did the VM give us a continuation address to jump to.
  // IA64 should never get a non zero continuation.

  __ cmp(is_null, PR0, 0, GR_RET, Assembler::equal);
  __ br(is_null, skip);

  __ stop("Non null return from handle safepoint poll exception");

  // No continuation or exceptions pending just return to a compiled frame

  __ bind(skip);

  RegisterSaver::restore_live_registers(masm);

  // We are back the the original state on entry and ready to go.

  __ br(RP);

  // Pending exception after the safepoint

  __ bind(pending);

  RegisterSaver::restore_live_registers(masm);

  // We are back the the original state on entry.

  // Tail-call forward_exception_entry, with the issuing PC in GR_Lsave_RP
  // so it looks like the original nmethod called forward_exception_entry.
  __ mova(continuation, StubRoutines::forward_exception_entry());
  __ mov(continuation_br, continuation);
  __ br(continuation_br);

  // -------------
  // make sure all code is generated
  __ flush();  

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


#ifdef ASSERT
// Add this routine to print an oopMap since gdb refuses to cooperate.

extern "C" void prtOopMapSet( OopMapSet* p) { p->print(); }
#endif
