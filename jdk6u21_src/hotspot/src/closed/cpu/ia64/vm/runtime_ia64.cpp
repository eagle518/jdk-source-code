/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_runtime_ia64.cpp.incl"

// Deoptimization

#define __ masm->
ExceptionBlob      *OptoRuntime::_exception_blob;


#ifdef ASSERT
// Add this routine to print an oopMap since gdb refuses to cooperate.

extern "C" void prtOopMapSet( OopMapSet* p) { p->print(); }
#endif

//------------------------------generate_exception_blob---------------------------
// creates exception blob at the end
// Using exception blob, this code is jumped from a compiled method.
// (see emit_exception_handler in ia64.ad file)
//
// Given an exception pc at a call we call into the runtime for the
// handler in this method. This handler might merely restore state
// (i.e. callee save registers) unwind the frame and jump to the
// exception handler for the nmethod if there is no Java level handler
// for the nmethod.

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
void OptoRuntime::generate_exception_blob() {
  // allocate space for code
  ResourceMark rm;

  // setup code generation tools
  CodeBuffer buffer("exception_blob", 4096, 512);
  MacroAssembler* masm    = new MacroAssembler(&buffer);

  Label exception_handler_found;
  Label L;

  int framesize = 0;

  address start = __ pc();

//  __ verify_thread();

  // We enter using the window of the compiled code that has just
  // generated an exception. We have just completed a call so any
  // SOC registers are fair game as temps here (just like other
  // platforms)

  // This dummy frame must store the link to the "caller"
  // This is dependent on calling conventions.

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

  int handle_exception_call_pc_offset = __ set_last_Java_frame(SP);

  OopMapSet *oop_maps = new OopMapSet();
  // No callee-save registers to save here.
  oop_maps->add_gc_map( handle_exception_call_pc_offset, new OopMap(framesize, 0));

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

  // GR_RET has the handler address (may be deopt blob)

  const BranchRegister handler_br = BR6_SCRATCH;

  __ mov(handler_br, GR_RET);

  __ add(exception_oop_addr, GR4_thread, in_bytes(JavaThread::exception_oop_offset()));
  __ add(exception_pc_addr,  GR4_thread, in_bytes(JavaThread::exception_pc_offset()));

  __ ld8(GR8_exception, exception_oop_addr);
  __ ld8(GR9_issuing_pc, exception_pc_addr);

  // Clear the exception oop so GC no longer processes it as a root.
  __ st8(exception_oop_addr, GR0);
#ifdef ASSERT
  const Register exception_handler_pc_addr2 = exception_oop_addr;

  __ add(exception_handler_pc_addr2, GR4_thread, in_bytes(JavaThread::exception_handler_pc_offset()));

  __ st8(exception_handler_pc_addr2, GR0);
  __ st8(exception_pc_addr, GR0);
#endif

  __ br(handler_br);

  // -------------
  // make sure all code is generated

  __ flush();

  _exception_blob = ExceptionBlob::create(&buffer, oop_maps,  0);
}
