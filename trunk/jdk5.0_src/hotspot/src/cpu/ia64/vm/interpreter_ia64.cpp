#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)interpreter_ia64.cpp	1.85 04/01/05 16:38:31 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_interpreter_ia64.cpp.incl"

// Generation of Interpreter assembly routines
//
// The InterpreterGenerator generates the interpreter into Interpreter::_code.

#define __ _masm->

static address interpreter_frame_manager        = NULL;
static address frame_manager_specialized_return = NULL;
static address native_entry                     = NULL;

static address interpreter_return_address       = NULL;

address AbstractInterpreter::_remove_activation_preserving_args_entry;

static address unctrap_frame_manager_entry  = NULL;

static address deopt_frame_manager_return_atos  = NULL;
static address deopt_frame_manager_return_btos  = NULL;
static address deopt_frame_manager_return_itos  = NULL;
static address deopt_frame_manager_return_ltos  = NULL;
static address deopt_frame_manager_return_ftos  = NULL;
static address deopt_frame_manager_return_dtos  = NULL;
static address deopt_frame_manager_return_vtos  = NULL;

// Zero value for gcc Itanium bug
double ia64_double_zero = 0.0;

//----------------------------------------------------------------------------------------------------

#ifndef CORE
address AbstractInterpreter::deopt_entry(TosState state, int length) {
  address ret = NULL;
  if (length != 0) {
    switch (state) {
      case atos: ret = deopt_frame_manager_return_atos; break;
      case btos: ret = deopt_frame_manager_return_btos; break;
      case ctos:
      case stos:
      case itos: ret = deopt_frame_manager_return_itos; break;
      case ltos: ret = deopt_frame_manager_return_ltos; break;
      case ftos: ret = deopt_frame_manager_return_ftos; break;
      case dtos: ret = deopt_frame_manager_return_dtos; break;
      case vtos: ret = deopt_frame_manager_return_vtos; break;
    }
  } else {
    // re-execute the bytecode (e.g. uncommon trap, popframe)
    ret = unctrap_frame_manager_entry;
  }
  assert(ret != NULL, "Not initialized"); 
  return ret;
}
#endif

address AbstractInterpreter::return_entry(TosState state, int length) {
  assert(interpreter_return_address != NULL, "Not initialized"); 
  return interpreter_return_address;
}

int AbstractInterpreter::BasicType_as_index(BasicType type) {
  int i = 0;
  switch (type) {
    case T_BOOLEAN: i = 0; break;
    case T_CHAR   : i = 1; break;
    case T_BYTE   : i = 2; break;
    case T_SHORT  : i = 3; break;
    case T_INT    : i = 4; break;
    case T_LONG   : i = 5; break;
    case T_VOID   : i = 6; break;
    case T_FLOAT  : i = 7; break;
    case T_DOUBLE : i = 8; break;
    case T_OBJECT : i = 9; break;
    case T_ARRAY  : i = 9; break;
    default       : ShouldNotReachHere();
  }
  assert(0 <= i && i < AbstractInterpreter::number_of_result_handlers, "index out of bounds");
  return i;
}


// A result handler converts/unboxes a native call result into
// a java interpreter/compiler result. The current frame is an
// interpreter frame.
//
address AbstractInterpreterGenerator::generate_result_handler_for(BasicType type) {
  const PredicateRegister is_false   = PR6_SCRATCH;
  const PredicateRegister is_true    = PR7_SCRATCH;
  const PredicateRegister is_notnull = PR6_SCRATCH;

  address entry = __ emit_fd();

  switch (type) {
  case T_BOOLEAN:
    __ cmp4(is_false, is_true, 0, GR_RET, Assembler::equal);
    __ clr(is_false, GR_RET);
    __ mov(is_true,  GR_RET, 1);           break;
  case T_BYTE:  __ sxt1(GR_RET, GR_RET);   break;
  case T_CHAR:  __ zxt2(GR_RET, GR_RET);   break;
  case T_SHORT: __ sxt2(GR_RET, GR_RET);   break;
  case T_INT:   __ sxt4(GR_RET, GR_RET);   break;
  case T_LONG:                             break;
  case T_OBJECT:
    // unbox result if not null
    __ cmp(is_notnull, PR0, 0, GR_RET, Assembler::notEqual);
    __ ld8(is_notnull, GR_RET, GR_RET);
//  __ verify_oop(GR_RET);
    break;
  case T_FLOAT: __ fnorms(FR_RET, FR_RET); break;
  case T_DOUBLE:                           break;
  case T_VOID:                             break;
  default:      ShouldNotReachHere();
  }

  __ ret();

  // Marker for disassembly: a bundle's worth of zero.
  NOT_PRODUCT(__ emit_addr(); __ emit_addr();)

  return entry;
}

// tosca based result to c++ interpreter stack based result.
// Result goes to address in Lscratch3

address AbstractInterpreterGenerator::generate_tosca_to_stack_converter(BasicType type) {
  // A result is in the native abi result register from a native method call.
  // We need to return this result to the interpreter by pushing the result on the interpreter's
  // stack. This is relatively simple: the destination address is in R32.
  // I.e. R32 is the first free element on the stack.  If we "push" a return value we must
  // adjust R32.
  //
  // on entry:
  //   GR_RET/FR_RET - result to move
  //   GR_I0         - interpreter expression tos
  //
  // on exit:
  //   GR_RET        - new interpreter expression tos (== GR_I0 for T_VOID)

  const PredicateRegister is_false   = PR6_SCRATCH;
  const PredicateRegister is_true    = PR7_SCRATCH;
  const PredicateRegister is_notnull = PR6_SCRATCH;

  const Register res = GR31_SCRATCH;
  const Register tos = GR_I0;

  address entry = __ emit_fd();

  switch (type) {
  case T_BOOLEAN:
    __ cmp(is_false, is_true, 0, GR_RET, Assembler::equal);
    __ clr(is_false, res);
    __ mov(is_true,  res, 1);
    __ st4(tos, res);
    break;
  case T_BYTE:
  case T_CHAR:
  case T_SHORT:
  case T_INT:
    __ st4(tos, GR_RET);
    break;
  case T_LONG:
    __ st8(tos, GR0, -BytesPerWord);   // mark unused slot for debugging
    __ st8(tos, GR_RET);               // long goes to topmost slot
    break;
  case T_OBJECT:
//  __ verify_oop(GR_RET);
    __ st8(tos, GR_RET);
    break;
  case T_FLOAT:
    __ fnorms(FR_RET, FR_RET);
    __ stfs(tos, FR_RET);
    break;
  case T_DOUBLE:
    __ stfs(tos, FR1, -BytesPerWord);  // mark unused slot for debugging
    __ stfd(tos, FR_RET);              // double goes to topmost slot
    break;
  case T_VOID:
    break;
  default:
    ShouldNotReachHere();
  }

  // new expression stack top
  __ sub(GR_RET, tos, type == T_VOID ? 0 : BytesPerWord);

  __ ret();

  return entry;
}

address AbstractInterpreterGenerator::generate_stack_to_stack_converter(BasicType type) {
  // A result is in the java expression stack of the interpreted method that has just
  // returned and is thus just beyond (lower addressed) the current java expression stack
  // top.  Push this result onto the java expression stack of the caller.
  //
  // The current interpreter activation in Lstate is for the method just returning its
  // result.  So we know that the result of this method is on the top of the current
  // execution stack (which is pre-pushed) and will be returned to the top of the caller's
  // stack.  The top of the caller's stack is the bottom of the locals of the current
  // activation.
  //
  // Because of the way activations are managed by the frame manager the value of sp is
  // below both the stack top of the current activation and naturally the stack top
  // of the calling activation.  This enables this routine to do a vanilla return.
  //
  // On entry:
  //   GR_I0  - callee expression tos + BytesPerWord
  //   GR_I1  - caller expression tos [i.e. free location]
  //
  // On exit:
  //   GR_RET - new interpreter expression tos (== GR_I0 for T_VOID)

  const Register from = GR_I0;
  const Register tos  = GR_I1;
  const Register res  = GR31_SCRATCH;
  const Register dum  = GR30_SCRATCH;

  address entry = __ emit_fd();

  // New expression stack top
  __ sub(GR_RET, tos, type == T_VOID ? 0 : BytesPerWord);

  switch (type) {
  case T_BOOLEAN:
  case T_BYTE:
  case T_CHAR:
  case T_SHORT:
  case T_INT:
  case T_FLOAT:
    __ ld4(res, from);
    __ st4(tos, res);
    break;
  case T_LONG:
  case T_DOUBLE:
    // Move both entries for debug purposes even though only one is live
    __ ld8(res, from, BytesPerWord);      // the live entry
    __ ld8(dum, from);                    // the dummy value
    __ st8(tos, dum, -BytesPerWord);      // store dummy value
    __ sub(GR_RET, GR_RET, BytesPerWord); // two slots
    __ st8(tos, res);                     // the live value
    break;
  case T_OBJECT:
    __ ld8(res, from);
//  __ verify_oop(res);
    __ st8(tos, res);
    break;
  case T_VOID:
    break;
  default:
    ShouldNotReachHere();
  }

  __ ret();

  return entry;
}

address AbstractInterpreterGenerator::generate_stack_to_native_abi_converter(BasicType type) {
  // A result is in the java expression stack of the interpreted method that has just
  // returned. Place this result in the native abi that the caller expects.
  // We are in a new frame: registers we set must be in caller (i.e. callstub) frame.
  //
  // Similar to generate_stack_to_stack_converter above. Called at a similar time from the
  // frame manager except in this situation the caller is native code (c1/c2/call_stub)
  // and so rather than return result onto caller's java expression stack we return the
  // result in the expected location based on the native abi.
  //
  // On entry:
  //   GR_I0         - callee expression tos + BytesPerWord
  //
  // On exit:
  //   GR_RET/FR_RET - result in expected output register

  const Register from = GR_I0;

  address entry = __ emit_fd();

  switch (type) {
  case T_BOOLEAN:
    __ ld1(GR_RET, from);
    break;
  case T_BYTE:
    __ ld1(GR_RET, from);
    __ sxt1(GR_RET, GR_RET);
    break;
  case T_CHAR:
    __ ld2(GR_RET, from);
    __ zxt2(GR_RET, GR_RET);
    break;
  case T_SHORT:
    __ ld2(GR_RET, from);
    __ sxt2(GR_RET, GR_RET);
    break;
  case T_INT:
    __ ld4(GR_RET, from);
    __ sxt4(GR_RET, GR_RET);
    break;
  case T_LONG:
    __ ld8(GR_RET, from);
    break;
  case T_OBJECT:
    __ ld8(GR_RET, from);
//  __ verify_oop(GR_RET);
    break;
  case T_FLOAT:
    __ ldfs(FR_RET, from);
    break;
  case T_DOUBLE:
    __ ldfd(FR_RET, from);
    break;
  case T_VOID:
    break;
  default:
    ShouldNotReachHere();
  }

  __ ret();

  return entry;
}


address AbstractInterpreterGenerator::generate_slow_signature_handler() {
  // We get called by the native entry code with our output register area == 8.
  // First we call InterpreterRuntime::slow_signature_handler (see interpreterRT_ia64.cpp)
  // to copy the argument list on the java expression stack into native varargs
  // format on the native stack.  The first entry in the varargs vector contains
  // a signature descriptor in the form of a bit array, two bits per argument.
  // Because the native entry code will pass the object address in GR_I0,
  // it's ok for the signature to occupy the first entry.  Integer arguments in
  // the varargs vector will be sign-extended to 8 bytes by slow_signature_handler.
  // Single-precision fp arguments occupy the low half of a varargs entry.
  //
  // On entry:
  //   GR_O0          - intptr_t*     Address of java argument list in memory.
  //   GR_Oprev_state - cInterpreter* Address of interpreter state for this method
  //   R27 - method DESTROYED ON EXIT
  //
  // On exit (just before return instruction):
  //   GR_I1-GR_I7 and FR_I0-FR_I7 contain the first 8 native arguments.

  const Register argv   = GR31_SCRATCH;
  const Register sig    = GR30_SCRATCH;
  const Register fpcnt  = GR29_SCRATCH;

  const PredicateRegister is_s8  = PR6_SCRATCH;
  const PredicateRegister is_int = PR7_SCRATCH;

  const PredicateRegister is_0   = PR8_SCRATCH;
  const PredicateRegister is_1   = PR9_SCRATCH;
  const PredicateRegister is_2   = PR10_SCRATCH;
  const PredicateRegister is_3   = PR11_SCRATCH;
  const PredicateRegister is_4   = PR12_SCRATCH;
  const PredicateRegister is_5   = PR13_SCRATCH;
  const PredicateRegister is_6   = PR14_SCRATCH;
  const PredicateRegister is_7   = PR15_SCRATCH;

  address entry = __ emit_fd();

  __ push_full_frame();


  __ mov(GR_O0, GR4_thread);
  __ mov(GR_O1, GR27_method);
  __ mov(GR_O2, GR_I0);

  // Copy java parameter list into varargs format at SP+16

  __ add(GR_O3, SP, 16);              // Where it will end up anyway

  // A java frame anchor already exists which is all we need

  __ call_VM_leaf(CAST_FROM_FN_PTR(address, InterpreterRuntime::slow_signature_handler));

  __ add(argv, SP, 16);

  __ ld8(sig, argv);                  // Load signature

  __ clr(fpcnt);                      // No fp args yet
  __ shru(sig, sig, 2);               // Skip first argument

  for (Argument ldarg(1); ldarg.is_register(); ldarg = ldarg.successor()) {

    // Test for integral/float and 4/8-byte.  See Argument definition.
    __ tbit(is_int, PR0, sig, 0, Assembler::equal);
    __ tbit(is_s8,  PR0, sig, 1, Assembler::notEqual);
    __ add(argv, argv, BytesPerWord); // Point to argument to load

    Label done;

    __ ld8(is_int, ldarg.as_register(), argv);
    __ shru(sig, sig, 2);             // Shift in next argument signature
    __ br(is_int, done);

    // is_int is dead.  We know we've got an fp arg.

    __ cmp(is_0, PR0, 0, fpcnt, Assembler::equal);
    __ cmp(is_1, PR0, 1, fpcnt, Assembler::equal);
    __ cmp(is_2, PR0, 2, fpcnt, Assembler::equal);

    __ cmp(is_3, PR0, 3, fpcnt, Assembler::equal);
    __ cmp(is_4, PR0, 4, fpcnt, Assembler::equal);
    __ cmp(is_5, PR0, 5, fpcnt, Assembler::equal);

    Label double_arg;

    __ cmp(is_6, PR0, 6, fpcnt, Assembler::equal);
    __ cmp(is_7, PR0, 7, fpcnt, Assembler::equal);
    __ br(is_s8, double_arg, Assembler::dptk);

    // We know we've got a single-precision arg.

    __ ldfs(is_0, FR_I0, argv);
    __ ldfs(is_1, FR_I1, argv);

    __ ldfs(is_2, FR_I2, argv);
    __ ldfs(is_3, FR_I3, argv);

    __ ldfs(is_4, FR_I4, argv);
    __ ldfs(is_5, FR_I5, argv);
    __ add(fpcnt, 1, fpcnt);

    __ ldfs(is_6, FR_I6, argv);
    __ ldfs(is_7, FR_I7, argv);
    __ br(done);

    __ bind(double_arg);

    // We know we've got a double-precision arg.

    __ ldfd(is_0, FR_I0, argv);
    __ ldfd(is_1, FR_I1, argv);

    __ ldfd(is_2, FR_I2, argv);
    __ ldfd(is_3, FR_I3, argv);

    __ ldfd(is_4, FR_I4, argv);
    __ ldfd(is_5, FR_I5, argv);
    __ add(fpcnt, 1, fpcnt);

    __ ldfd(is_6, FR_I6, argv);
    __ ldfd(is_7, FR_I7, argv);

    __ bind(done);
  }

  __ pop_full_frame();
  __ ret();

  return entry;
}

//
// Helpers for commoning out cases in the various type of method entries.
//
#ifndef CORE

// Increment invocation count and check for overflow.
//
void InterpreterGenerator::generate_counter_incr(Label& overflow) {
  const Register invocation_counter    = GR_O0;
  const Register invocation_limit      = GR2_SCRATCH;
  const Register invocation_limit_addr = invocation_limit;

  const PredicateRegister overflowed   = PR15_SCRATCH;

  // Update standard invocation counters
  __ increment_invocation_counter(invocation_counter);

  // Compare against limit
  __ mova(invocation_limit_addr, (address)&InvocationCounter::InterpreterInvocationLimit);
  __ ld4(invocation_limit, invocation_limit_addr);
  __ cmp4(overflowed, PR0, invocation_counter, invocation_limit, Assembler::higherEqual);
  __ br(overflowed, overflow, Assembler::spnt);
}


// Generate code to initiate compilation on invocation counter overflow.
//
void InterpreterGenerator::generate_counter_overflow(Label& retry_entry) {
  // After entering the vm we remove the activation and retry
  // the entry point in case the compilation is complete.

  // InterpreterRuntime::frequency_counter_overflow takes one argument
  // that indicates if the counter overflow occurs at a backwards branch (NULL bcp).
  // We pass zero.
  // The call returns the address of the verified entry point for the method or NULL
  // if the compilation did not complete (either went background or bailed out).

  __ clr(GR_O1);

  // Pass false to call_VM so it doesn't check for pending exceptions, since at this point
  // in the method invocation the exception handler would try to exit the monitor of
  // synchronized methods which haven't been entered yet.

  __ call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::frequency_counter_overflow), GR_O1, false); 
//__ ld8(GR5_method, GR_Lmethod_addr);          // Reload GR5_method, gc may have moved it
  __ ld8(GR27_method, GR_Lmethod_addr);         // Reload GR27_method, call killed it

  // Returns verified_entry_point or NULL, we don't care which.

  // Method has been compiled.  Remove activation frame and recurse.
  __ pop_full_frame();
  __ br(retry_entry);
}


// Compiled code for method exists.  Run it.
//
void InterpreterGenerator::generate_run_compiled_code() {
  // On entry:
  //   GR27_method   - callee methodOop.
  //   GR_Itos      - sender's Java expression tos, passed as is to adapter.
  //   GR31_SCRATCH - callee nmethod.  Must match definition in generate_check_compiled_code.
  
  const Register nmethod_addr              = GR31_SCRATCH;
  const Register entry_point               = GR30_SCRATCH;
  const Register entry_point_addr          = entry_point;
  const Register compiled_code_return_addr = GR29_SCRATCH;
  const Register pending_exception         = GR28_SCRATCH;
  const Register pending_exception_addr    = pending_exception;

  const PredicateRegister is_initial_call      = PR6_SCRATCH;
  const PredicateRegister is_recursive_call    = PR7_SCRATCH;
  const PredicateRegister have_entry_point     = PR8_SCRATCH;
  const PredicateRegister no_pending_exception = PR9_SCRATCH;

  const BranchRegister entry_point_br = BR6_SCRATCH;

  {
    Label setup_jump;
    Label exception_check;

    // if ((entry_point = nmethod->interpreter_entry_point_or_null()) == NULL) {
    //   // Code for the nmethod got generated before the code for the adapter.
    //   // Force an adapter to be generated.  The possibly gc'ed methodOop comes
    //   // back in thread->vm_result(), the adapter address in GR_RET.
    //   // We haven't set up the interpreter state yet so in order to do the call
    //   // we set up a last_Java_frame that makes it look like our caller did the
    //   // the other platforms have to do this same trick because we are at a
    //   // very inconvenient place to call the vm here. Doing this makes the
    //   // our caller (interpreter or compiled) look like the top frame and so
    //   // the argument oops will be gc'd in the callers frame since we don't "own"
    //   // them until an interpreter frame is setup.
    //   entry_point = InterpreterRuntime::nmethod_entry_point(method, nmethod);
    //
    //   // At this point, the nmethod may have been recompiled again.  However, since the
    //   // interpreter entry point is simply the corresponding i2c adapter, which is
    //   // dependent only on the method's signature, we don't care.  If the nmethod has
    //   // been deoptimized, we will end up in a zombie nmethod, which is ok, too.
    // }
    //
    // if (is_recursive_call) {
    //   // If we got here from the frame manager, either directly or via a specialized method
    //   // call, the compiled code should look like a specialized interpreter entry point.
    //   save_rp = frame_manager_specialized_return;
    // } else {
    //   // save_rp contains the return address in the call stub, which is where we want the
    //   // compiled code to return to.
    // }
    //
    // // Pop the current register frame.  Must do this before checking for a pending
    // // exception so we have a consistent view of things.
    // pop_full_frame();
    //
    // if (pending_exception == NULL) {
    //   // We're now in a state that looks like we've just executed a call instruction
    //   // from either the call stub or the frame manager.  If the latter, it looks
    //   // as if we've come from the specialized entry call.  The adapter expects the
    //   // tos in O1.  Jump to the actual entry point.
    //   goto *entry_point;
    // }
    // 
    // // Abort the whole thing.
    // if (is_initial_call) {
    //   // Just return to the call stub
    //   return;
    // }
    //
    // // Fall thru if recursive call.  The C++ interpreter will handle the exception.

    __ add(entry_point_addr, nmethod_addr, nmethod::interpreter_entry_point_offset());

    __ ld8(entry_point, entry_point_addr);

    // The call to nmethod_entry in the vm is especially stressful to stack walking
    // if we are doing SafepointALot then call into the vm even if we already know
    // where we are going.

    if (!SafepointALot) {
      __ cmp(have_entry_point, PR0, 0, entry_point, Assembler::notEqual);
      __ mov(GR_Lstate, GR0); // Make no Lstate stand out in the debugger
      __ br(have_entry_point, setup_jump);
    }

    // Can't use call_VM here because we have not setup a new interpreter state
    // make the call to the vm and make it look like our caller set up the
    // JavaFrameAnchor. The killer here is calculating our caller's BSP which
    // painful enough without worrying about a NAT collection

    __ set_last_Java_frame(GR_Lsave_SP, GR_Lsave_caller_BSP, GR_Lsave_RP);

    // This is not a leaf but we have a JavaFrameAnchor and we will check exceptions
    // afterward so this is ok.
    __ call_VM_leaf(CAST_FROM_FN_PTR(address, InterpreterRuntime::nmethod_entry_point),
		    GR4_thread,
                    GR27_method,
                    nmethod_addr);
    __ reset_last_Java_frame();
    __ get_vm_result(GR27_method);
    __ mov(entry_point, GR_RET);

    __ bind(setup_jump);

    __ cmp(is_initial_call, is_recursive_call, 0, GR_Iprev_state, Assembler::equal);
    __ mov(entry_point_br, entry_point);
    __ add(pending_exception_addr, thread_(pending_exception));

    __ mova(is_recursive_call, compiled_code_return_addr, (address)&frame_manager_specialized_return);

    __ ld8(is_recursive_call, GR_Lsave_RP, compiled_code_return_addr);
    __ ld8(pending_exception, pending_exception_addr);

    __ pop_full_frame();

    __ cmp(no_pending_exception, PR0, 0, pending_exception, Assembler::equal);
    __ br(no_pending_exception, entry_point_br);

    // If initial entry to interpreter and exception is pending now, return to call_stub
    // and let it catch it. Otherwise we just drop into the C++ interpreter and let it
    // handle it
    __ ret(is_initial_call);
  }
}


// Check if compiled code exists.
//
void InterpreterGenerator::generate_check_compiled_code(Label &run_compiled_code) {
  // On exit:
  //   GR31_SCRATCH - callee nmethod.  Must match definition in generate_run_compiled_code.

  const Register nmethod_addr       = GR31_SCRATCH; // Actually, an nmethod
  const Register interp_only_mode   = GR29_SCRATCH;

  const Register compiled_code_addr      = nmethod_addr;
  const Register interp_only_mode_addr   = interp_only_mode;

  const PredicateRegister skip_compiled = PR6_SCRATCH;
  const PredicateRegister run_compiled  = PR7_SCRATCH;

  Label skip_compiled_code;

  __ add(compiled_code_addr, method_(compiled_code));

  __ ld8(nmethod_addr, compiled_code_addr);

  if (JvmtiExport::can_post_interpreter_events()) {
    // JVMTI events, such as single-stepping, are implemented partly by avoiding running
    // compiled code in threads for which the event is enabled.  Check here for
    // interp_only_mode if these events CAN be enabled.

    __ add(interp_only_mode_addr, thread_(interp_only_mode));

    __ ld4(interp_only_mode, interp_only_mode_addr);

    __ cmp4(skip_compiled, PR0, 0, interp_only_mode, Assembler::notEqual);
    __ br(skip_compiled, skip_compiled_code);
  }

  __ cmp(run_compiled, PR0, 0, nmethod_addr, Assembler::notEqual);
  __ br(run_compiled, run_compiled_code);

  if (JvmtiExport::can_post_interpreter_events()) {
    __ bind(skip_compiled_code);
  }
}

#endif /* !CORE */


// End of helpers


// Common code for initializing the C++ interpreter state
//
void InterpreterGenerator::generate_compute_interpreter_state(Label& stack_overflow_return) {

  // On entry:
  //   GR27_method    - methodOop
  //   GR4_thread     - JavaThread*
  //   GR_Itos        - intptr_t*     sender tos (pre pushed) Lesp
  //   GR_Iprev_state - cInterpreter* previous interpreter state.  NULL if none.
  //   GR_Lsave_SP    - intptr_t*     previous SP
  //
  // On exit: same plus
  //   GR_Lstate      - cInterpreter* address of an interpreter state
  //   GR_Lmethod_addr- methodOop*    address of istate->_method
  //   GR_Ilocals     - intptr_t*     address of receiver
  //   PR6            - 1 if this is     a synchronized method
  //   PR7            - 1 if this is not a synchronized method
  //   PR8            - 1 if this is a native method
  //   PR9            - 1 if this is a java method
  //   PR10           - 1 if this is a static method
  //   PR11           - 1 if this is a dynamic method

  // Stack on entry
  //
  // +------------------+
  // |                  | <--- locals[0]   == first parameter == GR_I0
  // :   parameters     :
  // |                  | <--- locals[n-1] == last parameter
  // +------------------+
  // |                  |
  // + ABI scratch area +
  // |                  | <--- SP
  // +------------------+
  //
  //
  // Stack on exit, non-synchronized method
  //
  // +------------------+
  // |                  | <--- _locals
  // :      locals      :
  // |                  |
  // +------------------+
  // |                  |
  // +   16-byte slop   + 2 slots for return value in case caller
  // |                  | has no parameters or locals
  // +------------------+
  // |   interpreter    |
  // :      state       :
  // |  (cInterpreter)  | <--- _self_link == _monitor_base == _stack_base
  // +------------------+
  // |                  | <--- _stack.Tos
  // : expression stack :
  // |                  |
  // +------------------+
  // |                  | <--- _stack_limit
  // |  alignment slop  | may not exist
  // +------------------+
  // |     16-byte      |
  // + ABI scratch area +
  // |                  | <--- SP == _frame_bottom
  // +------------------+
  //
  //
  // Stack on exit, synchronized method
  //
  // +------------------+
  // |                  | <--- _locals
  // :      locals      :
  // |                  |
  // +------------------+
  // |                  |
  // +   16-byte slop   + 2 slots for return value in case caller
  // |                  | has no parameters or locals
  // +------------------+
  // |   interpreter    |
  // :      state       :
  // |  (cInterpreter)  | <--- _self_link == _monitor_base
  // +------------------+
  // |                  |
  // : initial monitor  :
  // |                  | <--- _stack_base
  // +------------------+
  // |                  | <--- _stack.Tos
  // : expression stack :
  // |                  |
  // +------------------+
  // |                  | <--- _stack_limit
  // |  alignment slop  | may not exist
  // +------------------+
  // |     16-byte      |
  // + ABI scratch area +
  // |                  | <--- SP == _frame_bottom
  // +------------------+

  //=============================================================================
  // Allocate space for locals other than the parameters, the
  // interpreter state, monitors, and the expression stack.

  const Register local_count       = GR_RET1;
  const Register parameter_count   = GR_RET2;
  const Register max_stack         = GR_RET3;
  const Register frame_size        = GR14_SCRATCH;
  const Register access_flags      = GR15_SCRATCH;

  const PredicateRegister is_synced      = PR6_SCRATCH;
  const PredicateRegister is_not_synced  = PR7_SCRATCH;
  const PredicateRegister is_native      = PR8_SCRATCH;
  const PredicateRegister is_java        = PR9_SCRATCH;
  const PredicateRegister is_static      = PR10_SCRATCH;
  const PredicateRegister is_dynamic     = PR11_SCRATCH;

  {
  // Local registers
  const Register page_size         = GR29_SCRATCH;
  const Register frame_count       = GR28_SCRATCH;
  const Register state_offset      = GR25_SCRATCH; // QQQ 27
  const Register overflow_handler  = GR26_SCRATCH;
  const Register mem_stack_limit   = GR24_SCRATCH;

  const PredicateRegister mem_stack_overflow  = PR12_SCRATCH;
  const PredicateRegister reg_stack_overflow  = PR13_SCRATCH;
  const BranchRegister    overflow_handler_br = BR6_SCRATCH;

  // Address registers
  const Register access_flags_addr      = access_flags;      // Address is dead after first use
  const Register max_stack_addr         = max_stack;         // "
  const Register local_count_addr       = local_count;       // "
  const Register parameter_count_addr   = parameter_count;   // "
  const Register mem_stack_limit_addr   = mem_stack_limit;   // "

  __ add(access_flags_addr, method_(access_flags));
  __ add(parameter_count_addr, method_(size_of_parameters));
  __ add(local_count_addr, method_(size_of_locals));

  __ ld4(access_flags, access_flags_addr);           // access_flags = method->access_flags();
  __ ld2(parameter_count, parameter_count_addr);     // parameter_count = method->size_of_parameters();

  __ add(max_stack_addr, method_(max_stack));
  __ add(mem_stack_limit_addr, thread_(memory_stack_limit));
  __ mov(state_offset, round_to(/*2*BytesPerWord +*/ sizeof(cInterpreter), BytesPerWord));
                                                     // We already have the extra two slots for the
                                                     // no-parameter/no-locals method result in the
                                                     // form of the 16-byte ABI scratch area

  __ ld2(local_count, local_count_addr);             // local_count = method->max_locals();
  __ ld2(max_stack, max_stack_addr);                 // max_stack = method->max_stack();

  // Point locals at the first parameter.  Method's locals are the parameters on
  // top of caller's expression stack.
  __ shladd(GR_Ilocals, parameter_count, LogBytesPerWord, GR_Itos);

  __ ld8(mem_stack_limit, mem_stack_limit_addr);     // mem_stack_limit = thread->memory_stack_limit();
  __ tbit(is_native, is_java, access_flags, JVM_ACC_NATIVE_BIT, Assembler::notEqual);

  //
  // frame_size =
  //   round_to((local_count - parameter_count)*BytesPerWord +
  //              2*BytesPerWord +
  //              sizeof(cInterpreter) +
  //              method->is_synchronized() ? sizeof(BasicObjectLock) : 0 +
  //              max_stack*BytesPerWord,
  //            16)
  //
  // Note that this calculation is exactly mirrored by AbstractInterpreter::layout_activation_impl()
  // [ and AbstractInterpreter::size_activation() ]. Which is used by deoptimization so that it
  // can allocate the proper sized frame. This only happens for interpreted frames so the extra
  // notes below about max_stack below are not important. The other thing to note is that
  // for interpreter frames other than the current activation the size of the stack is the
  // size of the live portion of the stack at the particular bcp and NOT the maximum stack
  // that the method might use.
  //
  // If we're calling a native method, we replace max_stack (which is zero)
  // with space for the worst-case signature handler varargs vector, which is:
  //
  //   max_stack = max(Argument::n_register_parameters, parameter_count+2);
  //
  // We add two slots to the parameter_count, one for the jni environment and one for a
  // possible native mirror.  We allocate space for at least the number of ABI registers,
  // even though InterpreterRuntime::slow_signature_handler won't write more than
  // parameter_count+2 words when it creates the varargs vector at the top of the stack.
  // The generated slow signature handler will just load trash into registers beyond
  // the necessary number.  We're still going to cut the stack back by the ABI register
  // parameter count so as to get SP+16 pointing at the ABI outgoing parameter area,
  // so we need to allocate at least that much even though we're going to throw it away.
  // 

  const PredicateRegister need_max_native_parameters = PR15_SCRATCH;

  __ tbit(is_not_synced, is_synced, access_flags, JVM_ACC_SYNCHRONIZED_BIT, Assembler::equal);
  __ tbit(is_static, is_dynamic, access_flags, JVM_ACC_STATIC_BIT, Assembler::notEqual);
  __ add(is_native, max_stack, parameter_count, 2);

  __ tbit(need_max_native_parameters, PR0, access_flags, JVM_ACC_NATIVE_BIT, Assembler::notEqual);
                                                     // Assume we have at least the ABI number of register parameters
  __ sub(GR2_SCRATCH, max_stack, Argument::n_register_parameters);

  __ cmp0(need_max_native_parameters, PR0, GR2_SCRATCH, Assembler::less, Assembler::And_);
                                                     // Do we?

  __ mov(need_max_native_parameters, max_stack, Argument::n_register_parameters);

  __ shl(max_stack, max_stack, LogBytesPerWord);     // max_stack is now in bytes
  __ sub(is_java, local_count, local_count, parameter_count);
                                                     // if (!method->is_native()) local_count = non-parameter local count
                                                     // method->max_locals() == 0 for native methods
  __ shladd(state_offset, local_count, LogBytesPerWord, state_offset);
  __ add(frame_size, state_offset, max_stack);
  __ add(is_synced, frame_size, frame_size, sizeof(BasicObjectLock));
                                                     // Space for method lock
  __ sub(GR_Lstate, GR_Lsave_SP, state_offset);      // Set interpreter state address, though it isn't yet allocated

  __ add(frame_size, frame_size, 16+15);             // Add 16-byte ABI scratch area size
                                                     // and round to stack alignment
  __ and3(frame_size, frame_size, -16);              // frame size in bytes that we need

  // Test that the new memory stack pointer is above the limit 
  // and that the register stack is below the limit.

  __ sub(GR2_SCRATCH, SP, frame_size);
  __ cmp(mem_stack_overflow, PR0, GR2_SCRATCH,  mem_stack_limit,     Assembler::lower);
  __ cmp(reg_stack_overflow, PR0, GR6_caller_BSP, GR7_reg_stack_limit, Assembler::higher);
  __ br(mem_stack_overflow, stack_overflow_return);
  __ br(reg_stack_overflow, stack_overflow_return);
  }

  //=============================================================================
  // frame_size doesn't overflow the stack.  Allocate new frame and initialize interpreter state.

  // Of the scratch registers, GR_RET1-GR_RET3, GR14, GR15, PR6 and PR7 are live.
  // GR_Lstate now contains a pointer to the uninitialized interpreter state.

  // Locals
  const Register bytecode_addr = GR14_SCRATCH;
  const Register constants     = GR15_SCRATCH;
  const Register cache         = GR16_SCRATCH;
  const Register msg           = GR17_SCRATCH;
  const Register tos           = GR18_SCRATCH;
  const Register stack_limit   = GR19_SCRATCH;
  const Register stack_base    = GR20_SCRATCH;

  // Address registers
  const Register method_constants_addr = constants;    // Address is dead after first use
  const Register prev_link_addr        = GR31_SCRATCH;
  const Register self_link_addr        = GR30_SCRATCH;
//  const Register sender_sp_addr        = GR29_SCRATCH;
  const Register thread_addr           = GR28_SCRATCH;
  const Register locals_addr           = GR24_SCRATCH; // QQQ 27
  const Register native_mirror_addr    = GR26_SCRATCH;

  const Register cache_addr            = cache;        // Address is dead after first use
  const Register bcp_addr              = GR31_SCRATCH;
  const Register mdx_addr              = GR30_SCRATCH;
  const Register last_Java_fp_addr     = GR29_SCRATCH;
  const Register msg_addr              = GR28_SCRATCH;
  const Register callee_method_addr    = GR24_SCRATCH; // QQQ 27
  const Register monitor_base_addr     = GR26_SCRATCH;
  const Register frame_bottom_addr     = GR25_SCRATCH;

  const Register stack_base_addr       = GR31_SCRATCH;
  const Register tos_addr              = GR30_SCRATCH;
  const Register state_constants_addr  = GR29_SCRATCH;
  const Register stack_limit_addr      = GR28_SCRATCH;

  // _last_Java_pc just needs to be close enough that we can identify the frame as
  // an interpreted frame. It does not need to be the exact return address from
  // either calling cInterpreter::InterpretMethod or the call to a jni native method
  // So we can initialize it here with a value of a bundle in this code fragment.
  // We only do this initialization for java frames where InterpretMethod needs a
  // a way to get a good pc value to store in the thread state. For interpreter frames
  // used to call jni native code we just zero the value in the state and move an
  // ip as needed in the native entry code.
  //
  const Register last_Java_pc_addr     = GR24_SCRATCH;  // QQQ 27
  const Register last_Java_pc          = GR26_SCRATCH;

  // Must reference stack before setting new SP since Windows
  // will not be able to deliver the exception on a bad SP.
  // Windows also insists that we bang each page one at a time in order
  // for the OS to map in the reserved pages.  If we bang only
  // the final page, Windows stops delivering exceptions to our
  // VectoredExceptionHandler and terminates our program.
  // Linux only requires a single bang but it's rare to have 
  // to bang more than 1 page so the code is enabled for both OS's.

  const PredicateRegister more_pages    = PR12_SCRATCH;
  const PredicateRegister no_more_pages = PR12_SCRATCH;
  const Register page_size              = GR29_SCRATCH;

  Label bang_more, bang_done;

  __ mov(page_size, os::vm_page_size());
  __ mov(GR3_SCRATCH, frame_size);                  // copy frame size
  __ mov(GR2_SCRATCH, SP);
  __ cmp(no_more_pages, PR0, GR3_SCRATCH, page_size, Assembler::less);
  __ br(no_more_pages, bang_done);

  __ bind(bang_more);

  __ sub(GR2_SCRATCH, GR2_SCRATCH, page_size);      // Point one page lower
  __ sub(GR3_SCRATCH, GR3_SCRATCH, page_size);      // Reduce page size
  __ cmp(more_pages, PR0, GR3_SCRATCH, page_size, Assembler::greater);
  __ st8(GR2_SCRATCH, GR0);                         // Test this page
  __ br(more_pages, bang_more);

  __ bind(bang_done);

  __ sub(frame_size, SP, frame_size);               // Calculate new SP
  __ st8(frame_size, GR0);                          // Test this page
  __ mov(SP, frame_size);                           // Allocate new stack frame

  // GR14 and GR15 are dead.

  __ add(is_java, bytecode_addr, method_(const));   // if (!method->is_native()) bytecode_addr = method->const();
  __ ld8(is_java, bytecode_addr, bytecode_addr);    // if (!method->is_native()) bytecode_addr = constMethod;
  __ add(is_java, bytecode_addr, bytecode_addr,     // if (!method->is_native()) bytecode_addr = constMethod->codes();
         in_bytes(constMethodOopDesc::codes_offset()));

  __ add(method_constants_addr, method_(constants));

  __ add(prev_link_addr, state_(_prev_link));

  __ ld8(constants, method_constants_addr);         // constants = method->constants();
  __ add(self_link_addr, state_(_self_link));

  __ add(thread_addr, state_(_thread));
  __ add(GR_Lmethod_addr, state_(_method));         // Local register because we don't want to reload it all the time

  __ st8(prev_link_addr, GR_Iprev_state);           // state->_prev_link = prev_state;
  __ st8(self_link_addr, GR_Lstate);                // state->_self_link = state;
  __ add(locals_addr, state_(_locals));

  __ st8(thread_addr, GR4_thread);                   // state->_thread = thread;
  __ add(native_mirror_addr, state_(_native_mirror));

  __ st8(GR_Lmethod_addr, GR27_method);              // state->_method = method;
  __ st8(locals_addr, GR_Ilocals);                  // state->_locals = locals;
  __ add(cache_addr, constants, constantPoolOopDesc::cache_offset_in_bytes());

  __ ld8(cache, cache_addr);                        // cache = method->constants()->cache();
  __ add(bcp_addr, state_(_bcp));
#ifndef CORE
  __ add(mdx_addr, state_(_mdx));
#endif

  __ st8(native_mirror_addr, GR0);                  // state->_native_mirror = NULL;
  __ add(last_Java_fp_addr, state_(_last_Java_fp));

  __ add(msg_addr, state_(_msg));

  __ mov(is_not_synced, stack_base, GR_Lstate);     // if (!method->synchronized()) stack_base = state;
  __ sub(is_synced, stack_base, GR_Lstate, sizeof(BasicObjectLock));
                                                    // else stack_base = (uintptr_t)state - sizeof(BasicObjectLock);

  __ st8(last_Java_fp_addr, GR6_caller_BSP);        // state->_last_Java_fp = ar.bsp;
  __ st8(is_native, bcp_addr, GR0);                 // if (method->is_native()) state->_bcp = NULL;
  __ st8(is_java, bcp_addr, bytecode_addr);         // else state->_bcp = bytecode_addr;
  __ mov(msg, cInterpreter::method_entry);

#ifndef CORE
  __ st8(mdx_addr, GR0);                            // state->_mdx = NULL;
#endif
  __ add(callee_method_addr, state_(_result._to_call._callee));
  __ add(monitor_base_addr, state_(_monitor_base));

  __ st4(msg_addr, msg);                            // state->_msg = cInterpreter::method_entry;
  __ st8(callee_method_addr, GR0);                  // state->_result._to_call._callee = NULL;
  __ add(frame_bottom_addr, state_(_frame_bottom));

  __ st8(monitor_base_addr, GR_Lstate);             // state->_monitor_base = state;
  __ st8(frame_bottom_addr, SP);                    // state->_frame_bottom = SP;
  __ add(stack_base_addr, state_(_stack_base));
  __ add(last_Java_pc_addr, state_(_last_Java_pc));

  __ add(tos_addr, state_(_stack));
  __ sub(tos, stack_base, BytesPerWord);            // tos = stack_base - 1;
  __ mov_from_ip(is_java, last_Java_pc);            // Just need an ip that identifies us as interpreted.

  __ st8(stack_base_addr, stack_base);              // state->_stack_base = stack_base;
  __ st8(tos_addr, tos);                            // state->_stack.Tos(tos);
  __ sub(stack_limit, tos, max_stack);              // stack_limit = tos - max_stack;
  __ st8(is_java, last_Java_pc_addr, last_Java_pc); // state->_last_Java_pc = <some_ip_in_this_code_buffer>;
  __ st8(is_native, last_Java_pc_addr, GR0);        // state->_last_Java_pc = NULL; (just for neatness)

  __ add(state_constants_addr, state_(_constants));
  __ add(stack_limit_addr, state_(_stack_limit));
  __ sub(local_count, local_count, 1);              // For counted loop

  Label zero_locals;

  __ st8(state_constants_addr, cache);              // state->_constants = method->constants()->cache();
  __ st8(stack_limit_addr, stack_limit);            // state->_stack_limit = stack_limit;
  __ br(is_not_synced, zero_locals);                // if (!method->is_synchronized()) goto zero_locals;

  // Synchronized method, allocate and initialize method object lock

  const Register pool_holder      = GR29_SCRATCH;
  const Register lockee           = GR28_SCRATCH;

  const Register pool_holder_addr = pool_holder;    // Address is dead after use
  const Register mirror_addr      = GR31_SCRATCH;
  const Register obj_addr         = GR30_SCRATCH;

  __ add(pool_holder_addr, constants, constantPoolOopDesc::pool_holder_offset_in_bytes());

  __ ld8(is_static, pool_holder, pool_holder_addr); // if (method->is_static())
                                                    //   pool_holder = method->constants()->pool_holder();

  const int mirror_offset = klassOopDesc::klass_part_offset_in_bytes() + Klass::java_mirror_offset_in_bytes();

  __ add(mirror_addr, pool_holder, mirror_offset);
  __ add(obj_addr, stack_base, BasicObjectLock::obj_offset_in_bytes());

  __ ld8(is_static, lockee, mirror_addr);           // if (method->is_static())
                                                    //   lockee = pool_holder->klass_part()->java_mirror();

  __ ld8(is_dynamic, lockee, GR_Ilocals);           // else
                                                    //   lockee = *(oop*)locals;

  __ st8(obj_addr, lockee);                         // monitor->set_obj(lockee);

  // See if we need to zero the locals

  __ bind(zero_locals);

  Label locals_zeroed;

  __ br(is_native, locals_zeroed);

  if (true /* zerolocals */ || ClearInterpreterLocals) {
    const Register          local_addr        = GR31_SCRATCH;
    const PredicateRegister no_locals_to_zero = PR15_SCRATCH;

    __ cmp(no_locals_to_zero, PR0, 0, local_count, Assembler::greater);
    __ shl(parameter_count, parameter_count, LogBytesPerWord);
    __ sub(local_addr, GR_Ilocals, parameter_count);

    __ mov(AR_LC, local_count);
    __ br(no_locals_to_zero, locals_zeroed, Assembler::dpnt);

    Label zero_slot;
    __ bind(zero_slot);

    __ st8(local_addr, GR0, -BytesPerWord);
    __ cloop(zero_slot, Assembler::sptk, Assembler::few);
  }

  __ bind(locals_zeroed);
}


// Lock the current method. 
//
void InterpreterGenerator::lock_method(void) {
  // Find preallocated monitor and lock method.
  // Method monitor is the first one and is allocated at (BasicObjectLock*)state - 1.

  const Register monitor  = GR_O1;
  const Register obj      = GR_O2;

  const Register obj_addr = obj; // Address is dead after use

  // Pass address of initial monitor we allocated
  __ sub(monitor, GR_Lstate, sizeof(BasicObjectLock));

  // Address of object address in monitor
  __ sub(obj_addr, GR_Lstate, sizeof(BasicObjectLock) - BasicObjectLock::obj_offset_in_bytes());

  // Pass object address
  __ ld8(obj, obj_addr);

  // Lock method
  __ lock_object(monitor, obj);

  // Reload GR27_method, call killed it
  __ ld8(GR27_method, GR_Lmethod_addr);
}


// Unlock the current method. 
//
void InterpreterGenerator::unlock_method(void) {
  // Find preallocated monitor and unlock method.
  // Method monitor is the first one and is allocated at (BasicObjectLock*)state - 1.

  const Register monitor = GR_O1;

  // Pass address of initial monitor we allocated
  __ sub(monitor, GR_Lstate, sizeof(BasicObjectLock));

  // Unlock method
  __ unlock_object(monitor);

  // Reload GR27_method, call killed it
  __ ld8(GR27_method, GR_Lmethod_addr);
}


// Routine exists to make tracebacks look decent in debugger
// while "shadow" interpreter frames are on stack. It is also
// used to distinguish interpreter frames.
//
extern "C" void InterpretMethodInitialDummy(void)
{
  ShouldNotReachHere(); 
}

extern "C" void InterpretMethodDummy(interpreterState istate)
{
  ShouldNotReachHere(); 
}


// Initial entry to C++ interpreter from the call_stub.
//
// This entry point is called the frame manager since it handles the generation
// of interpreter activation frames via requests directly from the vm (via call_stub)
// and via requests from the interpreter. The requests from the call_stub happen
// directly thru the entry point. Requests from the interpreter happen via returning
// from the interpreter and examining the message the interpreter has returned to
// the frame manager. The frame manager can take the following requests:
//
// more_monitors      - Need a new monitor. Shuffle the expression stack on down and
//                      allocate a new monitor.
// call_method        - Setup a new activation to call a new method. Very similar to
//                      what happens during entry during the entry via the call stub.
// return_from_method - Remove an activation. Return to interpreter or call stub.
// popping_frame      - Remove an activation. Return to interpreter.
//                      popframe api only happens if pop will return it to interpreter.
//                      However the interpreter frame may be a newly created deoptimzed
//                      frame and so we can't depend on it being a recursive return.
// retry_method       -
// throwing_exception -

#ifdef ASSERT
  #define VALIDATE_STATE(self_link, marker)                            \
  {                                                                    \
    const PredicateRegister is_really_me = PR15_SCRATCH;               \
    Label skip;                                                        \
    __ add(self_link, state_(_self_link));                             \
    __ ld8(self_link, self_link);                                      \
    __ cmp(is_really_me, PR0, GR_Lstate, self_link, Assembler::equal); \
    __ br(is_really_me, skip);                                         \
/*  __ breakpoint_trap(); */                                           \
    __ bind(skip);                                                     \
  }
#else
  #define VALIDATE_STATE(self_link, marker)
#endif /* ASSERT */


address InterpreterGenerator::generate_interpreter_frame_manager(void) {
  // On entry:
  //   GR27_method - methodOop
  //   GR4_thread - JavaThread*
  //   GR_Otos    - intptr_t*    sender tos (pre pushed) Lesp

  // A single frame manager is plenty as we don't specialize for synchronized.

  if (interpreter_frame_manager != NULL) {
    return interpreter_frame_manager;
  }

  address entry = __ emit_fd();

  __ push_full_frame();

  __ clr(GR_Iprev_state); // No previous state on initial activation


  //=============================================================================
  // Dispatch an instance of the interpreter.  Recursive activations come here.

  Label re_dispatch;
  __ bind(re_dispatch);

  // At this point GR_Itos contains the corresponding sender tos and
  // GR_Iprev_state the address of the calling method's interpreter state.

#ifndef CORE
  // Generate check for compiled code
  Label run_compiled_code;
  if (!CompileTheWorld) {
    generate_check_compiled_code(run_compiled_code);
  }  
#endif


  //=============================================================================
  // Allocate new frame and initialize interpreter state.

  Label stack_overflow_return;

  generate_compute_interpreter_state(stack_overflow_return);


  //=============================================================================
  // Interpreter dispatch.  All scratch registers are dead here.

  Label call_interpreter;
  __ bind(call_interpreter);

  // We declare all but very-local registers here because otherwise it's
  // too hard to keep track of what's declared where.

  // Thread fields.
  const Register pending_exception = GR24_SCRATCH;

  // Interpreter state fields.
  const Register tos               = GR23_SCRATCH;
  const Register stack_limit       = GR22_SCRATCH;

  const Register msg               = GR21_SCRATCH;

  // MethodOop fields.
  const Register parameter_count   = GR15_SCRATCH;
  const Register result_index      = GR14_SCRATCH;

  // Thread field addresses.
  const Register pending_exception_addr = pending_exception; // Address is dead after first use
  const Register last_interpreter_fp_addr  = GR2_SCRATCH;

  // Interpreter state field addresses.
  const Register callee_method_addr     = GR31_SCRATCH;
  const Register locals_addr            = GR30_SCRATCH;
  const Register frame_bottom_addr      = GR29_SCRATCH;
  const Register tos_addr               = GR28_SCRATCH;
  const Register stack_limit_addr       = GR16_SCRATCH; // QQ 27
  const Register msg_addr               = GR26_SCRATCH;

  // MethodOop field addresses.
  const Register parameter_count_addr   = parameter_count;   // Address is dead after first use
  const Register result_index_addr      = result_index;      // "

  // Address of various interpreter stubs.
  const Register stub_addr              = GR25_SCRATCH;

  // Branch register used to call various interpreter stubs.
  const BranchRegister stub_br = BR6_SCRATCH;

  const PredicateRegister is_initial_call   = PR15_SCRATCH;
  const PredicateRegister is_recursive_call = PR14_SCRATCH;

  __ flush_bundle();

  // uncommon trap needs to jump to here to enter the interpreter (re-execute current bytecode)
  unctrap_frame_manager_entry  = __ pc();

  // If we are profiling, store our fp in the thread so we can 
  // find it during a tick.
  if ( Arguments::has_profile() ) {
    __ add(last_interpreter_fp_addr, thread_(last_interpreter_fp));
    __ st8(last_interpreter_fp_addr, GR6_caller_BSP);
  }

  __ call_VM_leaf(CAST_FROM_FN_PTR(
                    address,
                    JvmtiExport::can_post_interpreter_events() || jvmpi::enabled() ? 
                                                           cInterpreter::InterpretMethodWithChecks
                                                         : cInterpreter::InterpretMethod),
                  GR_Lstate);

  __ flush_bundle();
  interpreter_return_address  = __ pc();

  // If we are profiling, clear the fp in the thread to tell
  // the profiler that we are no longer in the interpreter.
  if ( Arguments::has_profile() ) {
    __ add(last_interpreter_fp_addr, thread_(last_interpreter_fp));
    __ st8(last_interpreter_fp_addr, GR0);
  }

  // Reload L_method_addr since deopt entry does not set it.

  __ add(GR_Lmethod_addr, state_(_method));         // Local register because we don't want to reload it all the time

  __ ld8(GR27_method, GR_Lmethod_addr);         // Reload method, it is scratch register


  __ add(msg_addr, state_(_msg));

  __ ld4(msg, msg_addr);

  Label more_monitors;
  Label return_from_native;
  Label return_from_native_common;
  Label return_from_interpreted_method;
  Label return_from_recursive_activation;
  Label unwind_recursive_activation;
  Label resume_interpreter;
  Label return_to_initial_caller;
  Label unwind_initial_activation_pending_exception;
  Label call_method;
  Label call_special;
  Label retry_method;
  Label popping_frame;
  Label throwing_exception;

  // Very-local scratch registers.

  const PredicateRegister do_call_method        = PR15_SCRATCH;
  const PredicateRegister do_return_from_method = PR14_SCRATCH;
  const PredicateRegister do_throwing_exception = PR13_SCRATCH;
  const PredicateRegister do_retry_method       = PR12_SCRATCH;
  const PredicateRegister do_more_monitors      = PR11_SCRATCH;
  const PredicateRegister do_pop_frame          = PR10_SCRATCH;

  __ cmp4(do_call_method,        PR0, cInterpreter::call_method,        msg, Assembler::equal);
  __ cmp4(do_return_from_method, PR0, cInterpreter::return_from_method, msg, Assembler::equal);
  __ br(do_call_method, call_method);

  __ cmp4(do_throwing_exception, PR0, cInterpreter::throwing_exception, msg, Assembler::equal);
  __ cmp4(do_retry_method,       PR0, cInterpreter::retry_method,       msg, Assembler::equal);
  __ br(do_return_from_method, return_from_interpreted_method);

  __ cmp4(do_more_monitors,      PR0, cInterpreter::retry_method,       msg, Assembler::equal);
  __ cmp4(do_pop_frame,          PR0, cInterpreter::popping_frame,      msg, Assembler::equal);
  __ br(do_throwing_exception, throwing_exception);

  __ br(do_retry_method, retry_method);
  __ br(do_more_monitors, more_monitors);
  __ br(do_pop_frame, popping_frame);

  __ stop("bad message from interpreter");


  //=============================================================================
  // Add a monitor just below the existing one(s).  state->_stack_base
  // points to the lowest existing one, so we insert the new one just
  // below it and shuffle the expression stack down.  Ref. the above
  // stack layout picture, we must update _stack_base, _stack, _stack_limit
  // and _frame_bottom in the interpreter state.

  __ bind(more_monitors);

  // Very-local scratch registers.

  const Register          old_tos         = tos;
  const Register          new_tos         = GR2_SCRATCH;
  const Register          obj_addr        = GR3_SCRATCH;
  const Register          slot            = GR_RET1;
  const Register          stack_base_addr = GR_RET2;
  const Register          stack_base      = GR_RET3;
  const PredicateRegister move_slot       = PR15_SCRATCH;

  // Load up relevant interpreter state.

  __ add(stack_base_addr, state_(_stack_base));
  __ add(tos_addr, state_(_stack));
  __ add(stack_limit_addr, state_(_stack_limit));

  __ ld8(stack_base, stack_base_addr);                       // Old stack_base
  __ ld8(old_tos, tos_addr);                                 // Old tos
  __ add(frame_bottom_addr, state_(_frame_bottom));

  __ ld8(stack_limit, stack_limit_addr);                     // Old stack_limit
  __ sub(SP, SP, round_to(sizeof(BasicObjectLock), 16));     // Allocate space for new monitor
  __ sub(stack_base, stack_base, sizeof(BasicObjectLock));
                                                             // Point stack_base at it
  __ sub(new_tos, old_tos, sizeof(BasicObjectLock));         // New tos

  __ st8(stack_base_addr, stack_base);                       // Update stack_base
  __ st8(tos_addr, new_tos);                                 // Update tos
  __ sub(stack_limit, stack_limit, sizeof(BasicObjectLock)); // New stack_limit

  __ st8(stack_limit_addr, stack_limit);                     // Update stack_limit
  __ add(obj_addr, stack_base, BasicObjectLock::obj_offset_in_bytes());

  __ st8(frame_bottom_addr, SP);                             // Update frame_bottom
  __ mov(msg, cInterpreter::got_monitors);                   // Tell interpreter we allocated the lock

  __ st4(msg_addr, msg);

  // Shuffle expression stack down.  Recall that stack_base points just above
  // the new expression stack bottom.  old_tos and new_tos are used to scan
  // thru the old and new expression stacks.

  __ add(old_tos, old_tos, BytesPerWord);                    // First old location
  __ add(new_tos, new_tos, BytesPerWord);                    // First new location

  Label copy_slot;
  __ bind(copy_slot);

  __ ld8(slot, old_tos, BytesPerWord);                       // slot = *old_tos++;
  __ cmp(move_slot, PR0, stack_base, new_tos, Assembler::higher);

  __ st8(move_slot, new_tos, slot, BytesPerWord);            // *new_tos++ = slot;
  __ br(move_slot, copy_slot, Assembler::dptk);

  // Restart interpreter

  __ st8(obj_addr, GR0);                                     // Mark lock as unused
  __ br(call_interpreter);


  //=============================================================================
  // Returning from a compiled method into a deopted method. The bytecode at the
  // bcp has completed. The result of the bytecode is in the native abi (the tosca
  // for the template based interpreter). Any stack space that was used by the
  // bytecode that has completed has been removed (e.g. parameters for an invoke)
  // so all that we have to do is place any pending result on the expression stack
  // and resume execution on the next bytecode.

  Label return_from_deopt_common;

  // deopt needs to jump to here to enter the interpreter (return a result)
  __ flush_bundle();
  deopt_frame_manager_return_atos  = __ pc();

  // GR_RET (GR8) and FR_RET (FR8) are live here!
  __ mov(result_index, AbstractInterpreter::BasicType_as_index(T_OBJECT));    // Result stub address array index
  __ br(return_from_deopt_common);


  // deopt needs to jump to here to enter the interpreter (return a result)
  __ flush_bundle();
  deopt_frame_manager_return_btos  = __ pc();

  // GR_RET (GR8) and FR_RET (FR8) are live here!
  __ mov(result_index, AbstractInterpreter::BasicType_as_index(T_BOOLEAN));    // Result stub address array index
  __ br(return_from_deopt_common);

  // deopt needs to jump to here to enter the interpreter (return a result)
  __ flush_bundle();
  deopt_frame_manager_return_itos  = __ pc();

  // GR_RET (GR8) and FR_RET (FR8) are live here!
  __ mov(result_index, AbstractInterpreter::BasicType_as_index(T_INT));    // Result stub address array index
  __ br(return_from_deopt_common);

  // deopt needs to jump to here to enter the interpreter (return a result)
  __ flush_bundle();

  deopt_frame_manager_return_ltos  = __ pc();
  // GR_RET (GR8) and FR_RET (FR8) are live here!
  __ mov(result_index, AbstractInterpreter::BasicType_as_index(T_LONG));    // Result stub address array index
  __ br(return_from_deopt_common);

  // deopt needs to jump to here to enter the interpreter (return a result)
  __ flush_bundle();

  deopt_frame_manager_return_ftos  = __ pc();
  // GR_RET (GR8) and FR_RET (FR8) are live here!
  __ mov(result_index, AbstractInterpreter::BasicType_as_index(T_FLOAT));    // Result stub address array index
  __ br(return_from_deopt_common);

  // deopt needs to jump to here to enter the interpreter (return a result)
  __ flush_bundle();
  deopt_frame_manager_return_dtos  = __ pc();

  // GR_RET (GR8) and FR_RET (FR8) are live here!
  __ mov(result_index, AbstractInterpreter::BasicType_as_index(T_DOUBLE));    // Result stub address array index
  __ br(return_from_deopt_common);

  // deopt needs to jump to here to enter the interpreter (return a result)
  __ flush_bundle();
  deopt_frame_manager_return_vtos  = __ pc();

  // GR_RET (GR8) and FR_RET (FR8) are live here!
  __ mov(result_index, AbstractInterpreter::BasicType_as_index(T_VOID));

  // Deopt return common 
  // an index is present that lets us move any possible result being
  // return to the interpreter's stack
  //
  __ bind(return_from_deopt_common);

  __ add(tos_addr, state_(_stack));
  __ ld8(tos, tos_addr);                      // Current tos includes no parameter slots
  __ mov(msg, cInterpreter::deopt_resume);
  __ br(return_from_native_common);


// We are sent here when we are unwinding from a native method or
// adapter with an exception pending.  We need to notify the interpreter
// that there is an exception to process.

  Label return_from_native_with_exception;

  __ flush_bundle();

  AbstractInterpreter::_rethrow_exception_entry = __ pc();

  __ add(pending_exception_addr, thread_(pending_exception));
  __ st8(pending_exception_addr, GR_RET);
  __ mov(msg, cInterpreter::method_resume /*rethrow_exception*/);
  //
  // NOTE: the interpreter frame as setup be deopt does NOT include
  // any parameter slots (good thing since we have no callee here
  // and couldn't remove them) so we don't have to do any calculations
  // here to figure it out. 
  //
  __ add(tos_addr, state_(_stack));
  __ ld8(tos, tos_addr);                      
  __ br(return_from_native_common);

  //=============================================================================
  // Returning from a native method.  Result is in the native abi location so
  // we must move it to the java expression stack.

  __ bind(return_from_native);

  // GR_RET (GR8) and FR_RET (FR8) are live here!

  // Very-local scratch registers.

  const PredicateRegister have_pending_exception = PR15_SCRATCH;

  __ add(callee_method_addr, state_(_result._to_call._callee));
  __ add(tos_addr, state_(_stack));
  __ ld8(GR27_method, callee_method_addr);     // Load callee methodOop, gc may have moved it

  __ add(parameter_count_addr, method_(size_of_parameters));
  __ ld2(parameter_count, parameter_count_addr);
                                              // Callee parameter count

  __ ld8(tos, tos_addr);                      // Current tos includes parameter slots
  __ shladd(tos, parameter_count, LogBytesPerWord, tos);
                                              // New tos = old tos + size of callee parameters
  __ add(result_index_addr, method_(result_index));

  __ ld4(result_index, result_index_addr);    // Result stub address array index
  __ mov(msg, cInterpreter::method_resume);

  __ bind(return_from_native_common);

  __ add(pending_exception_addr, thread_(pending_exception));
  __ ld8(pending_exception, pending_exception_addr);
                                              // Pending exception, if any

  __ cmp(have_pending_exception, PR0, 0, pending_exception, Assembler::notEqual);

  // If there's a pending exception, we really have no result, so GR_RET is dead.
  // resume_interpreter assumes the new tos is in GR_RET.

  __ mov(have_pending_exception, GR_RET, tos);
  __ br(have_pending_exception, resume_interpreter);

  // No pending exception, copy method result from native ABI register to tos

  // Address of stub descriptor address array
  __ mova(stub_addr, AbstractInterpreter::tosca_result_to_stack());

  __ mov(GR_O0, tos);                         // Pass tos to stub

  // Address of stub descriptor address
  __ shladd(stub_addr, result_index, LogBytesPerOop, stub_addr);

  __ ld8(stub_addr, stub_addr);               // Stub descriptor address

  __ ld8(stub_addr, stub_addr);               // Stub entry point

  __ mov(stub_br, stub_addr);                 // Call the stub
  __ call(stub_br);			// DOES NOT KILL R27


  // new tos = result of call in GR_RET

  __ br(resume_interpreter);


  //=============================================================================
  // We encountered an exception while computing the interpreter state, so GR_Lstate
  // isn't valid.  Act as if we just returned from the callee method with a pending
  // exception.

  __ bind(stack_overflow_return);

  Label recursive_stack_overflow;

  __ mov(GR_RET, GR_Ilocals);                 // Save new tos across pop_dummy_full_frame
  __ cmp(is_recursive_call, PR0, 0, GR_Iprev_state, Assembler::notEqual);
  __ mov(GR_Lstate, GR_Iprev_state);          // call_VM must know where the interpreter frame is
  __ br(is_recursive_call, recursive_stack_overflow);

  // Can't use call_VM here because we have not setup a new interpreter state.  Make
  // the call to the vm and make it look like our caller set up the JavaFrameAnchor. 

  __ set_last_Java_frame(GR_Lsave_SP, GR_Lsave_caller_BSP, GR_Lsave_RP);

  // This is not a leaf but we have a JavaFrameAnchor now and we will check
  // exceptions afterward so this is ok.  The various throw_xxxError methods
  // all store the exception oop in the thread's pending_exception field.

  __ call_VM_leaf(CAST_FROM_FN_PTR(address, InterpreterRuntime::throw_StackOverflowError), GR4_thread);

  __ br(unwind_initial_activation_pending_exception);

  __ bind(recursive_stack_overflow);

  __ pop_dummy_full_frame();                  // Pop the dummy frame we were trying to set up
  __ mov(GR_Lscratch0, GR_RET);               // Save new tos across call_VM

  // Create exception oop and make it pending.
  // Since our caller was an interpreter frame we have a valid interpreter
  // frame to use for call_VM.

  __ call_VM(GR8_exception, CAST_FROM_FN_PTR(address, InterpreterRuntime::throw_StackOverflowError));

  __ mov(GR_RET, GR_Lscratch0);               // Reload new tos
  __ mov(msg, cInterpreter::method_resume);
  __ br(resume_interpreter);


  //=============================================================================
  // We have popped a frame from an interpreted call. We are assured of returning to an interpreted
  // call by the popframe abi. We have no return value all we have to do is pop the current
  // frame and then make sure that the top of stack (of the caller) gets set to where
  // it was when we entered the callee (i.e. the args are still in place).
  // Or we are returning to the interpreter. In the first case we must extract result (if any)
  // from the java expression stack and store it in the location the native abi would expect
  // for a call returning this type. In the second case we must simply do a stack to stack
  // move as we unwind.

  __ bind(popping_frame);

  __ ld8(GR27_method, GR_Lmethod_addr);           // Reload callee method, gc may have moved it

  // We may be returning to a deoptimized frame in which case the
  // usual assumption of a recursive return is not true.

  __ cmp(is_recursive_call, PR0, 0, GR_Iprev_state, Assembler::notEqual);

  // resume_interpreter expects the original tos in GR_RET.
  __ mov(is_recursive_call, GR_RET, GR_Itos);

  // We're done.
  __ mov(is_recursive_call, msg, cInterpreter::popping_frame);

  __ br(is_recursive_call, unwind_recursive_activation);

  // Popping frame to a deoptimized frame we will be returning to the c2i and
  // then to the deopt blob. We have no results just pop initial frame and return.

  __ pop_full_frame();
  __ ret();

  //=============================================================================
  // We have finished a interpreted call. We are either returning to native (call_stub/c2)
  // or we are returning to the interpreter. In the first case we must extract result (if any)
  // from the java expression stack and store it in the location the native abi would expect
  // for a call returning this type. In the second case we must simply do a stack to stack
  // move as we unwind.

  __ bind(return_from_interpreted_method);

  __ ld8(GR27_method, GR_Lmethod_addr);        // Reload callee method, gc may have moved it

  // Check if this is the initial invocation of the frame manager.
  // If so, previous interpreter state in GR_Iprev_state will be null.

  __ add(tos_addr, state_(_stack));
  __ add(locals_addr, state_(_locals));

  __ cmp(is_initial_call, PR0, 0, GR_Iprev_state, Assembler::equal);
  __ br(is_initial_call, return_to_initial_caller);

  // Nope.  Use a stub to copy result from callee's to caller's expression stack.

  __ add(result_index_addr, method_(result_index));

  __ ld4(result_index, result_index_addr);

  // Address of stub descriptor address array
  __ mova(stub_addr, AbstractInterpreter::stack_result_to_stack());

  __ ld8(GR_O0, tos_addr);                    // Pass callee's tos
  __ ld8(GR_O1, locals_addr);                 // Pass caller's tos == callee's locals address

  __ add(GR_O0, GR_O0, BytesPerWord);         // Bias callee's tos for stub

  // Address of stub descriptor address
  __ shladd(stub_addr, result_index, LogBytesPerOop, stub_addr);

  __ ld8(stub_addr, stub_addr);               // Stub descriptor address

  __ ld8(stub_addr, stub_addr);               // Stub entry point

  __ mov(stub_br, stub_addr);                 // Call the stub
  __ call(stub_br);				// DOES NOT KILL R27

  // new tos = result of call in GR_RET

  // Get the message for the interpreter
  __ mov(msg, cInterpreter::method_resume);

  // And fall thru


  //=============================================================================
  // Get new tos across register frame boundary by 'returning' it from this frame.
  // New tos is in GR_RET.  New state is in the previous frame's GR_Lstate.

  __ bind(unwind_recursive_activation);

  __ pop_dummy_full_frame();

  // And fall thru


  //=============================================================================
  // Resume the interpreter after a call.  Reload GR27_method, restore the stack
  // frame from state->_frame_bottom, and the tos from GR_RET.

  // NOTE:: the message to the interpreter should already be in "msg"

  __ bind(resume_interpreter);

  __ add(frame_bottom_addr, state_(_frame_bottom));
  __ add(tos_addr, state_(_stack));
  __ add(msg_addr, state_(_msg));

  __ ld8(GR27_method, GR_Lmethod_addr);        // Restore current methodOop
  __ ld8(SP, frame_bottom_addr);              // Restore stack frame

  __ st8(tos_addr, GR_RET);                   // Restore tos

  __ st4(msg_addr, msg);                      // Note resumption after call
  __ br(call_interpreter);


  //=============================================================================
  // Interpreter returning to native code (call_stub/c1/c2) from
  // initial activation.  Convert stack result and unwind activation.
  // We get here only from return_from_interpreted_method: GR27_method
  // contains the callee methodOop.

  __ bind(return_to_initial_caller);

  // If we have an expection pending we have no result and we
  // must figure out where to really return to.
  //
  __ add(pending_exception_addr, thread_(pending_exception));
  __ ld8(pending_exception, pending_exception_addr);
  __ cmp(PR6, PR0, 0, pending_exception, Assembler::notEqual);
  __ br(PR6, unwind_initial_activation_pending_exception);

  __ add(result_index_addr, method_(result_index));

  __ ld4(result_index, result_index_addr);

  // Address of stub descriptor address array
  __ mova(stub_addr, AbstractInterpreter::stack_result_to_native());

  __ ld8(GR_O0, tos_addr);                    // Pass tos

  __ add(GR_O0, GR_O0, BytesPerWord);         // Bias tos for stub

  // Address of stub descriptor address
  __ shladd(stub_addr, result_index, LogBytesPerOop, stub_addr);

  __ ld8(stub_addr, stub_addr);               // Stub descriptor address

  __ ld8(stub_addr, stub_addr);               // Stub entry point

  __ mov(stub_br, stub_addr);                 // Call the stub
  __ call(stub_br);				// DOES NOT KILL R27

  //=============================================================================
  // Unwind from initial activation. No exception is pending
  // Pop initial frame and return.

  __ pop_full_frame();
  __ ret();

  // And fall thru


  //=============================================================================
  // Unwind from initial activation. An exception is pending

  __ bind(unwind_initial_activation_pending_exception);

  {
    Label L;
    __ call_VM_leaf(CAST_FROM_FN_PTR(address, SharedRuntime::exception_handler_for_return_address), GR_Lsave_RP);
    __ add(pending_exception_addr, thread_(pending_exception));
    __ mov(GR10, GR_RET);
    __ ld8(GR8_exception, pending_exception_addr);
    __ st8(pending_exception_addr, GR0);
    __ mov(GR9_issuing_pc, GR_Lsave_RP);
    __ mov(GR_Lsave_RP, GR10);
    __ bind(L);
  }

  // Pop initial frame and return to exception handler

  __ pop_full_frame();
  __ ret();


  //=============================================================================
  // Call a new method. Compute new args and trim the expression stack to only what
  // we are currently using and then recurse.

  __ bind(call_method);

  // Very-local scratch registers.

  const Register new_sp = GR2_SCRATCH;
  const Register dummy  = GR3_SCRATCH;
  const Register self   = GR_RET1;

  const PredicateRegister is_call_to_self    = PR15_SCRATCH;
  const PredicateRegister is_call_to_special = PR14_SCRATCH;

  __ add(callee_method_addr, state_(_result._to_call._callee));
  __ add(tos_addr, state_(_stack));

  __ ld8(GR27_method, callee_method_addr);             // Callee methodOop
  __ ld8(GR_Otos, tos_addr);                           // Load outgoing tos

  // Trim the native stack back to tos.  tos points just beyond the last
  // parameter, so we only have to add the difference between the ABI
  // scratch area size and BytesPerWord, then round up.

#if 0
  __ sub(new_sp, GR_Otos, (16 - BytesPerWord) + 15);

  __ and3(SP, new_sp, -16);
#else
  // Since interpreter tos points at a free location we only need one
  // more free location to meet the native abi scratch are of 16 bytes
  // so we can subtract one location and round down to meet the abi requirements
  __ sub(new_sp, GR_Otos, BytesPerWord);
  __ and3(SP, new_sp, -16);

#endif

#if 0
  // Point outgoing locals at the first parameter

  __ add(parameter_count_addr, method_(size_of_parameters));
  __ ld2(parameter_count, parameter_count_addr);

  // Callee method's locals are the parameters on top of caller's expression stack.
  // Pass their address across to new frame.

  __ shladd(GR_Olocals, parameter_count, LogBytesPerWord, GR_Otos);
#endif
  __ mov(GR_Oprev_state, GR_Lstate);                  // Pass current state across to new frame
  __ mova(dummy, CAST_FROM_FN_PTR(address, InterpretMethodDummy));

  // Load the callee's entry point and push a dummy frame whose RP is
  // InterpretMethodDummy.  If the callee is the frame manager, tail recurse.

  __ add(stub_addr, state_(_result._to_call._callee_entry_point));
  __ mova(self, (address)&interpreter_frame_manager);

  __ ld8(dummy, dummy);                               // InterpretMethodDummy address

  __ ld8(stub_addr, stub_addr);                       // Callee's function descriptor address
  __ ld8(self, self);                                 // Frame manager's function descriptor address

  __ cmp(is_call_to_self, is_call_to_special, self, stub_addr, Assembler::equal);

  __ ld8(is_call_to_special, stub_addr, stub_addr);   // Specialized entry point
  __ br(is_call_to_special, call_special);

  // Push a dummy frame and recurse

  __ push_dummy_full_frame(dummy);

  // GR_Oprev_state and GR_Otos are now
  // GR_Iprev_state and GR_Itos, respectively.

  __ br(re_dispatch);

  __ bind(call_special);

  // Call the specialized entry.

  __ mov(stub_br, stub_addr);
  __ call(stub_br);				// DOES NOT KILL R27

  // Mark return from specialized entry for generate_native_entry.

  frame_manager_specialized_return = __ pc();

  // Handle the return.

  __ br(return_from_native);


  //=============================================================================
  // Interpreted method asked to run compiled code instead.  Revert to state just
  // prior to call (see call_method above) and recurse.

  __ bind(retry_method);

  const Register saved_sp_addr = GR2_SCRATCH;

  __ ld8(GR27_method, GR_Lmethod_addr);        // Reload GR27_method, call killed it
  __ mov(SP, GR_Lsave_SP);                    // Restore caller's SP
  __ br(re_dispatch);


  //=============================================================================
  // Interpreted method "returned" with an exception, pass it on.
  // Pass no result, unwind activation and continue/return to interpreter/call_stub/c2.

  __ bind(throwing_exception);

  // Check if this is the initial invocation of the frame manager.
  // If so, previous interpreter state in GR_Iprev_state will be null.

  __ add(locals_addr, state_(_locals));

  __ ld8(GR_RET, locals_addr);                // New tos is our first parameter address

  __ cmp(is_initial_call, PR0, 0, GR_Iprev_state, Assembler::equal);
  __ br(is_initial_call, unwind_initial_activation_pending_exception);

  // send resume message, interpreter will see the exception first

  __ mov(msg, cInterpreter::method_resume);
  __ br(unwind_recursive_activation);


#ifndef CORE

  //=============================================================================
  // Run compiled code.  Looks to the frame manager like a specialized method call.
  // Adapter will take care of parameter list conversion.

  __ bind(run_compiled_code);
  generate_run_compiled_code();

  // We get here if there's an exception pending and we're not returning to the
  // call stub.  We're in the caller's context.  Need to pass the current tos to
  // resume_interpreter.

  __ add(tos_addr, state_(_stack));
  __ ld8(GR_RET, tos_addr);
  __ br(resume_interpreter);

#endif // !CORE


  //=============================================================================
  // Push the last instruction out to the code buffer.

  __ flush_bundle();

  interpreter_frame_manager = entry;
  return entry;
}


// Various method entries


// Math function, frame manager must set up an interpreter state, etc.
address InterpreterGenerator::generate_math_entry(AbstractInterpreter::MethodKind kind) {
  return NULL;
}


// Abstract method entry.
//
address InterpreterGenerator::generate_abstract_entry(void) {
  address entry = __ emit_fd();

  // We could be called from a call stub, compiled code, or the frame manager.
  // If from a call stub or compiled code, we set up the call stub as the last
  // java frame and we're all set.  The frame manager treats a call to this
  // stub as a specialized call (i.e., similar to a native call), so we don't
  // have to differentiate between a recursive interpreter call and a call from
  // the call stub.

  __ push_full_frame();

  // Can't use call_VM here because we have not setup a new interpreter state.  Make
  // the call to the vm and make it look like our caller set up the JavaFrameAnchor. 

  __ set_last_Java_frame(GR_Lsave_SP, GR_Lsave_caller_BSP, GR_Lsave_RP);

  __ call_VM_leaf(CAST_FROM_FN_PTR(address, InterpreterRuntime::throw_AbstractMethodError), GR4_thread);

  __ pop_full_frame();
  __ ret();

  return entry;
}


// Empty method, generate a very fast return.
//
address InterpreterGenerator::generate_empty_entry(void) {
  // On entry:
  //   GR_Otos    - intptr_t*   sender tos (pre pushed) Lesp
  //   GR27_method - methodOop
  //   GR4_thread - JavaThread*
  //
  // Do nothing for empty methods (do not even increment invocation counter)

  // Use normal interpreter entry  to get method events
  if (!UseFastEmptyMethods || jvmpi::enabled()) return NULL;

  address entry = __ emit_fd();

//__ push_full_frame();

//__ verify_oop(GR27_method);

//__ pop_full_frame();

  __ ret();

  return entry;
}


// Call an accessor method (assuming it is resolved, otherwise drop into vanilla (slow path) entry
//
address InterpreterGenerator::generate_accessor_entry(void) {

  // Use normal interpreter entry  to get method events
  if (!UseFastEmptyMethods || jvmpi::enabled()) return NULL;
#if 0
  address entry = __ emit_fd();

//__ verify_oop(GR27_method);

  if (UseFastAccessorMethods) {
    // Do fastpath for resolved accessor methods
    ShouldNotReachHere();
  }
#endif
  // Default to frame manager
  return NULL;
}


// Call a native method. (C++ interpreter)
// This sets up a somewhat different looking stack for calling the native method
// than the typical interpreter frame setup.
//
address InterpreterGenerator::generate_native_entry(void) {
  // On entry:
  //   GR27_method     - methodOop
  //   GR4_thread     - JavaThread*
  //   GR_Otos    - intptr_t*    sender tos (pre pushed) Lesp
  //   GR_Oprev_state - cInterpreter* address of interpreter state for this method

  if (native_entry != NULL) {
    return native_entry;
  }

  const bool inc_counter = UseCompiler || CountCompiledCalls;

  address entry = __ emit_fd();

  Label retry_entry;
  __ bind(retry_entry);

  __ push_full_frame();

//__ verify_oop(GR27_method);

#ifndef CORE
  // Generate check for compiled code
  Label run_compiled_code;
  if (!PreferInterpreterNativeStubs && !CompileTheWorld) {
    generate_check_compiled_code(run_compiled_code);
  }  
#endif


  //=============================================================================
  // See if GR_Iprev_state is valid.  I.e., see if we got here from the frame manager.
  // If we didn't, clear GR_Iprev_state.

  const Register          frame_manager_return = GR_RET;
  const PredicateRegister no_prev_state        = PR15_SCRATCH;

  __ mova(frame_manager_return, (address)&frame_manager_specialized_return);
  __ ld8(frame_manager_return, frame_manager_return);
  __ cmp(no_prev_state, PR0, GR_Lsave_RP, frame_manager_return, Assembler::notEqual);
  __ clr(no_prev_state, GR_Iprev_state);


  //=============================================================================
  // Allocate new frame and initialize interpreter state.

  Label exception_return;
  Label stack_overflow_return;

  generate_compute_interpreter_state(stack_overflow_return);

  // Stuff live-on-exit from compute_interpreter_state.

  const PredicateRegister is_not_synced = PR7_SCRATCH;

  // We declare all but very-local registers here because otherwise it's
  // too hard to keep track of what's declared where.

  const Register pending_exception = GR31_SCRATCH;
  const Register access_flags      = GR30_SCRATCH;
  const Register constants         = GR29_SCRATCH;
  const Register pool_holder       = GR28_SCRATCH;
  const Register mirror            = GR17_SCRATCH; // QQ 27
  const Register active_handles    = GR26_SCRATCH;
  const Register last_native_pc    = GR25_SCRATCH;
  const Register thread_state      = GR24_SCRATCH;
  const Register native_method     = GR23_SCRATCH;

  const Register pending_exception_addr = pending_exception; // Address is dead after use
  const Register access_flags_addr      = access_flags;      // "
  const Register method_constants_addr  = constants;         // "
  const Register active_handles_addr    = active_handles;    // "
  const Register pool_holder_addr       = pool_holder;       // "
  const Register mirror_addr            = mirror;            // "
  const Register next_handle_addr       = active_handles;    // active_handles is dead after use
  const Register jni_environment_addr   = GR22_SCRATCH;
  const Register last_native_pc_addr    = GR21_SCRATCH;
  const Register thread_state_addr      = GR20_SCRATCH;
  const Register native_method_addr     = GR19_SCRATCH;
  const Register native_GP              = GR18_SCRATCH;


#ifndef CORE
  //=============================================================================
  // Increment invocation counter and check for overflow
  //
  Label invocation_counter_overflow; 
  if (inc_counter) {
    generate_counter_incr(invocation_counter_overflow);
  }
#endif // !CORE


  // Check for synchronized methods.  Must happen AFTER invocation counter
  // check, so method is not locked if counter overflows.

  Label lock_check_done;
  __ br(is_not_synced, lock_check_done);

  lock_method();

  __ bind(lock_check_done);


  // jvmti/jvmpi support

  __ notify_method_entry();


  // Get and call the signature handler

  const Register signature_handler_addr = GR_Lscratch0; 
  const Register signature_handler      = GR2_SCRATCH;

  __ add(signature_handler_addr, method_(signature_handler));
  __ ld8(signature_handler, signature_handler_addr);

  Label call_signature_handler;

  const PredicateRegister have_signature_handler = PR15_SCRATCH;

  __ cmp(have_signature_handler, PR0, 0, signature_handler, Assembler::notEqual);
  __ br(have_signature_handler, call_signature_handler);

  // Method has never been called.  Either generate a specialized handler or point to the slow one.
  __ call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::prepare_native_call), GR27_method);
  __ ld8(GR27_method, GR_Lmethod_addr);         // Reload method, call killed it

  // Check for an exception while looking up the target method.  
  // If we incurred one, bail.

  __ add(pending_exception_addr, thread_(pending_exception));

  __ ld8(pending_exception, pending_exception_addr);

  const PredicateRegister have_pending_exception = PR15_SCRATCH;

  __ cmp(have_pending_exception, PR0, 0, pending_exception, Assembler::notEqual);
  __ br(have_pending_exception, exception_return);

  __ bind(call_signature_handler);

  // Before we call the signature handler we push a new frame to protect the
  // interpreter frame volatile registers when we return from jni but before
  // we can get back to Java

  // First set the frame anchor while the SP/FP registers are convenient
  // and the slow signature handler can use this same frame anchor

  (void)
  __ set_last_Java_frame(SP);                       // set last_Java_pc, last_Java_fp, last_Java_sp

  // Now save some registers we need to cross the frame we push

  assert(GR_Ilocals == GR_I2, "change this code");

  __ mov(GR_O2, GR_Ilocals);                       // Pass locals directly in expected register
  __ mov(GR_O0, GR_Lmethod_addr);
  __ mov(GR_O3, GR_Lstate);
  __ mov(GR_Lsave_GP, GP);                          // Save current GP 
  __ push_dummy_full_frame(GR0);

  // IN NEW FRAME, Interpreter Frame is up one level
  // Special registers for this jni transition frame

  const Register GR_Lsave_RSC   = GR_L15;
  const Register GR_Lmod_RSC    = GR_L14;
  const Register GR_Lsave_UNAT  = GR_L13;

  // Setup RSC for jni calls

  __ mov(GR_Lsave_RSC, AR_RSC);           // save original value of RSC
  __ movl(GR_Lmod_RSC, CONST64(0xFFFFFFFFC000FFFC));    // mask tear point to zero, rse to lazy

  // Make registers look like interpreter frame we just left (what we care about anyway)

  __ mov(GR_Lmethod_addr, GR_I0);
  __ mov(GR_Lstate, GR_I3);
  __ and3(GR_Lmod_RSC, GR_Lmod_RSC, GR_Lsave_RSC);


  __ flushrs();

  // Make sure eager RSE is off while we are in native

  __ mov(AR_RSC, GR_Lmod_RSC); 
  __ loadrs();					// invalidate lower frames

  const Register flags_addr             = GR2_SCRATCH;
  const Register flushed                = GR3_SCRATCH;

  __ add(flushed, GR0, JavaFrameAnchor::flushed);
  __ add(flags_addr, GR4_thread, in_bytes(JavaThread::frame_anchor_offset() + JavaFrameAnchor::flags_offset()));


  __ st4(flags_addr, flushed, Assembler::ordered_release);
  
  // We don't need a fence here because state is still Java we will fence after any state change
  // and that is enough


  // Now the interpreter frame (and its call chain) have been invalidated and flushed.
  // We are now protected against Eager being enabled in native code.
  // Even if it goes eager the registers will be reloaded as clean
  // and we will invalidate after the call so no spurious flush should
  // be possible.


  // Reload signature_handler_addr in the new frame
  __ add(signature_handler_addr, method_(signature_handler));
  __ ld8(signature_handler, signature_handler_addr);    // get descriptor

  const BranchRegister signature_handler_br = BR6_SCRATCH;

  __ ld8(signature_handler, signature_handler);    // Load entry point from descriptor


  __ mov(GR_O0, GR_Ilocals);                       // Pass locals address to signature handler
  __ mov(signature_handler_br, signature_handler);
  __ call(signature_handler_br);
  __ ld8(GR27_method, GR_Lmethod_addr);         // Reload method, call killed it

  // Remove the register parameter varargs slots we allocated in compute_interpreter_state.
  // SP+16 ends up pointing to the ABI outgoing argument area.
  __ add(SP, SP, Argument::n_register_parameters*BytesPerWord);

  const Register result_handler_addr = GR_Lscratch0;

  __ mov(result_handler_addr, GR_RET);             // Save across call to native method


  // Set up fixed parameters and call the native method.
  // If the method is static, get mirror into GR_O1.

  const PredicateRegister is_static = PR15_SCRATCH;

  __ add(access_flags_addr, method_(access_flags));
  __ add(method_constants_addr, method_(constants));
  __ add(active_handles_addr, thread_(active_handles));

  __ ld4(access_flags, access_flags_addr);          // access_flags = method->access_flags();
  __ ld8(constants, method_constants_addr);         // constants = method->constants();

  __ ld8(active_handles, active_handles_addr); 
  __ tbit(is_static, PR0, access_flags, JVM_ACC_STATIC_BIT, Assembler::notEqual);
  __ add(pool_holder_addr, constants, constantPoolOopDesc::pool_holder_offset_in_bytes());

  __ add(is_static, GR_O1, state_(_native_mirror)); // if (method->is_static()) {
                                                    //   GR_O1 = &state->_native_mirror;
  __ ld8(is_static, pool_holder, pool_holder_addr); //   pool_holder = method->constants()->pool_holder();

  const int mirror_offset = klassOopDesc::klass_part_offset_in_bytes() + Klass::java_mirror_offset_in_bytes();

  __ add(is_static, mirror_addr, pool_holder, mirror_offset);
  __ add(thread_state_addr, thread_(thread_state));
  __ add(next_handle_addr, active_handles, JNIHandleBlock::top_offset_in_bytes());

  __ ld8(is_static, mirror, mirror_addr);           //   mirror = pool_holder->klass_part()->java_mirror();
  __ mov(thread_state, _thread_in_native);
  __ add(native_method_addr, method_(native_function));  // address of pointer to descriptor

  __ st8(is_static, GR_O1, mirror);                 //   state->_native_mirror = mirror;
                                                    // }

  __ ld8(native_method_addr, native_method_addr);       // Now points to the descriptor

  // At this point, arguments have been copied off the stack into their
  // JNI positions.  Oops are boxed in-place on the stack, with handles
  // copied to arguments.  The result handler address is in a local register.

  __ st8(next_handle_addr, GR0);                    // thread->active_handles()->clear();

  __ add(GR_O0, thread_(jni_environment));          // Pass JNIEnv address

  // Transition from _thread_in_Java to _thread_in_native. As soon as we make this change
  // the safepoint code needs to be certain that the last Java frame we established is good.
  // The pc in that frame just need to be near here not an actual return address.

  __ st4(thread_state_addr, thread_state, Assembler::ordered_release);

  // Fence because acquire is not ordered wrt release
  __ mf();


  __ ld8(native_method, native_method_addr, in_bytes(FuncDesc::gp_offset())); // get func ptr, adjust pointer to gp
  __ ld8(GP, native_method_addr);                  // Native method GP

  // Call the native method
  const BranchRegister native_method_br = BR6_SCRATCH;
  __ mov(native_method_br, native_method);
  __ call(native_method_br);			// off we go

  const Register sync_state         = GR_I0;
  const Register sync_state_addr    = sync_state;    // Address is dead after use
  const Register suspend_flags      = GR_I1;
  const Register suspend_flags_addr = suspend_flags; // Address is dead after use

  // Block, if necessary, before resuming in _thread_in_Java state.
  // In order for GC to work, don't clear the last_Java_sp until after blocking.

  // Switch thread to "native transition" state before reading the synchronization state.
  // This additional state is necessary because reading and testing the synchronization
  // state is not atomic w.r.t. GC, as this scenario demonstrates:
  //     Java thread A, in _thread_in_native state, loads _not_synchronized and is preempted.
  //     VM thread changes sync state to synchronizing and suspends threads for GC.
  //     Thread A is resumed to finish this native method, but doesn't block here since it
  //     didn't see any synchronization is progress, and escapes.

  __ add(thread_state_addr, thread_(thread_state));
  __ add(suspend_flags_addr, thread_(suspend_flags));
  __ mov(thread_state, _thread_in_native_trans);
  __ st4(thread_state_addr, thread_state, Assembler::ordered_release);
  __ mova(sync_state_addr, SafepointSynchronize::address_of_state());

  // Fence because acquire is not ordered wrt release.
  __ mf();

  // Now invalidate the lower frames once again so that if Eager happened
  // behind our back we will still be safe and reload when we go back to
  // Java mode.

  __ mov(AR_RSC, GR_Lmod_RSC); 
  __ loadrs();					// invalidate lower frames

  // Now before we return to java we must look for a current safepoint
  // (a new safepoint can not start since we entered native_trans).
  // We must check here because a current safepoint could be modifying
  // the callers registers right this moment.

  // Acquire isn't strictly necessary here because of the fence, but
  // sync_state is declared to be volatile, so we do it anyway.

  const PredicateRegister not_synced = PR6;

  __ ld4(sync_state, sync_state_addr, Assembler::acquired);
  __ ld4(suspend_flags, suspend_flags_addr, Assembler::acquired);

  Label sync_check_done;

  __ cmp(not_synced, PR0, SafepointSynchronize::_not_synchronized, sync_state, Assembler::equal, Assembler::Unc);
  __ cmp(not_synced, PR0, suspend_flags, GR0, Assembler::equal, Assembler::And_);
  __ br(not_synced, sync_check_done);

  // Block.  We do the call directly and leave the current last_Java_frame setup undisturbed
  // We must save any possible native result acrosss the call. No oop is present

  __ mov(GR_Lsave_UNAT, AR_UNAT);         // save caller's UNAT

  __ sub(GR2, SP, 16);		// address of 1st spill
  __ sub(SP, SP, 16 + 32 + 32);	// 1 int register spill, 1 float spill, abi scratch area
  __ st8spill(GR2, GR_RET, -32);
  __ stfspill(GR2, FR_RET);

  __ mov(GR_O0, GR4_thread);

  // Get target address and gp 
  address target = CAST_FROM_FN_PTR(FuncDesc*, JavaThread::check_safepoint_and_suspend_for_native_trans)->entry();
  address target_gp = CAST_FROM_FN_PTR(FuncDesc*, JavaThread::check_safepoint_and_suspend_for_native_trans)->gp();

  __ mova(GR_RET, target);
  __ mova(GP, target_gp);

  __ mov(BR6_SCRATCH, GR_RET);
  __ call(BR6_SCRATCH);

  // Restore any possible native result

  __ add(GR2, SP, 32);
  __ ldffill(FR_RET, GR2, 32);
  __ ld8fill(GR_RET, GR2);
  __ add(SP, SP, 16 + 32 + 32);       // 1 int register spill, 1 float spill, abi scratch area

  __ mov(AR_UNAT, GR_Lsave_UNAT);     // restore caller's UNAT

  __ bind(sync_check_done);

  // Put RSC back to original state. 
  __ mov(AR_RSC, GR_Lsave_RSC);

  // windows are no longer flushed

  __ add(flags_addr, GR4_thread, in_bytes(JavaThread::frame_anchor_offset() + JavaFrameAnchor::flags_offset()));
  __ st4(flags_addr, GR0, Assembler::ordered_release);

  __ mov(GR_I0, result_handler_addr);             // pass result handler across frame pop
  __ pop_dummy_full_frame();
  __ mov(GP, GR_Lsave_GP);

  __ mov(result_handler_addr, GR_O0);             // put result handler in expected local

  // <<<<<< Back in Interpreter Frame >>>>>

  __ ld8(GR27_method, GR_Lmethod_addr);         // Reload method, call killed it


  // We are in thread_in_native_trans here and back in the normal interpreter frame.
  // We don't have to do anything special about safepoints and we can switch
  // to Java mode anytime we are ready. 

  // Save native method result.  A bit of trickiness getting the dresult address
  // 16-byte aligned: _native_fresult is a double[3].  See interp_masm_ia64.hpp.

  // Note: frame::interpreter_frame_result has a dependency on how the 
  // method result is saved across the call to post_method_exit. For native
  // methods it assumes that the non-FPU/non-void result is saved in
  // _native_lresult and a FPU result in _native_fresult. If this changes
  // then the interpreter_frame_result implementation will need to be 
  // updated too.

  const Register native_lresult_addr = GR2_SCRATCH;
  const Register native_fresult_addr = GR3_SCRATCH;

  __ add(native_lresult_addr, state_(_native_lresult));
  __ add(native_fresult_addr, state_(_native_fresult) + 0x8);

  __ and3(native_fresult_addr, native_fresult_addr, ~0xF);
  __ st8(native_lresult_addr, GR_RET);

  __ stfspill(native_fresult_addr, FR_RET);

  __ add(thread_state_addr, thread_(thread_state));
  __ mov(thread_state, _thread_in_Java);

  __ st4(thread_state_addr, thread_state, Assembler::ordered_release);

  // Fence because acquire is not ordered wrt release
  __ mf();

  __ reset_last_Java_frame();

  // JVMDI jframeIDs are invalidated on exit from native method.
  // JVMTI does not use jframeIDs, this whole mechanism must be removed when JVMDI is removed.
  if (JvmtiExport::must_purge_jvmdi_frames_on_native_exit()) { 
    __ call_VM_leaf(CAST_FROM_FN_PTR(address, JvmtiExport::thread_leaving_native_code));
  }

  // Reload GR27_method, call killed it.  We can't look at state->_method until we're
  // back in java state because in java state gc can't happen until we get to a safepoint.

  __ ld8(GR27_method, GR_Lmethod_addr);


  // jvmdi/jvmpi support.  Whether we've got an exception pending or not, and whether
  // unlocking throws an exception or not, we notify on native method exit.
  // If we do have an exception, we'll end up in the caller's context to handle it,
  // so if we don't do the notify here, we'll drop it on the floor.

  __ notify_method_exit();


  // Handle exceptions

  Label exception_check_done;

  __ add(pending_exception_addr, thread_(pending_exception));
  __ add(access_flags_addr, method_(access_flags));

  __ ld8(pending_exception, pending_exception_addr);
  __ ld4(access_flags, access_flags_addr);          // access_flags = method->access_flags();

  const PredicateRegister no_pending_exception = PR15_SCRATCH;

  __ cmp(no_pending_exception, PR0, 0, pending_exception, Assembler::equal);
  __ br(no_pending_exception, exception_check_done);

  // We just leave it pending, caller will do the correct thing.
  // See if we must unlock.

  {
    Label unlock_check_done;

    __ tbit(is_not_synced, PR0, access_flags, JVM_ACC_SYNCHRONIZED_BIT, Assembler::equal);
    __ br(is_not_synced, unlock_check_done);

    unlock_method();

    __ bind(unlock_check_done);
  }

  __ bind(exception_return);

  // An exception is pending.  We call into the runtime only if the caller
  // was not interpreted.  If it was interpreted the interpreter will do
  // the correct thing.  If it isn't interpreted (call stub/compiled code)
  // we will change our return and continue.
  {
  const PredicateRegister is_not_initial_call = PR15_SCRATCH;
  const Register          continuation        = GR10;
  Label continuation_known;

  __ cmp(is_not_initial_call, PR0, GR_Iprev_state, GR0, Assembler::notEqual);
  __ br(is_not_initial_call, continuation_known);

  __ call_VM_leaf(CAST_FROM_FN_PTR(address, SharedRuntime::exception_handler_for_return_address), GR_Lsave_RP);
  __ add(pending_exception_addr, thread_(pending_exception));
  __ mov(continuation, GR_RET);
  __ ld8(GR8_exception, pending_exception_addr);
  __ st8(pending_exception_addr, GR0);
  __ mov(GR9_issuing_pc, GR_Lsave_RP);
  __ mov(GR_Lsave_RP, continuation);

  __ bind(continuation_known);
  }

  __ pop_full_frame();

  __ ret();

  __ bind(exception_check_done);


  // No exception pending.  See if we must unlock.

  {
  Label unlock_check_done;

  __ add(access_flags_addr, method_(access_flags));

  __ ld4(access_flags, access_flags_addr);          // access_flags = method->access_flags();

  __ tbit(is_not_synced, PR0, access_flags, JVM_ACC_SYNCHRONIZED_BIT, Assembler::equal);
  __ br(is_not_synced, unlock_check_done);

  unlock_method();

  __ bind(unlock_check_done);
  }


  // Move native method result back into proper registers and return.
  // Invoke result handler (may unbox/promote).

  __ add(native_lresult_addr, state_(_native_lresult));
  __ add(native_fresult_addr, state_(_native_fresult) + 0x8);

  __ and3(native_fresult_addr, native_fresult_addr, ~0xF);
  __ ld8(GR_RET, native_lresult_addr);

  __ ldffill(FR_RET, native_fresult_addr);

  const BranchRegister result_handler_br = BR6_SCRATCH;

  __ ld8(result_handler_addr, result_handler_addr); // Entry point
  __ mov(result_handler_br, result_handler_addr);
  __ call(result_handler_br);			    // DOES NOT KILL GR27_method

  __ pop_full_frame();

  __ ret();


  //=============================================================================
  // We encountered an exception while computing the interpreter state, so GR_Lstate
  // isn't valid.  Act as if we just returned from the callee method with a pending
  // exception.

  __ bind(stack_overflow_return);

  // Create exception oop and make it pending.

  // Can't use call_VM here because we have not setup a new interpreter state
  // make the call to the vm and make it look like our caller set up the
  // JavaFrameAnchor. 

  __ set_last_Java_frame(GR_Lsave_SP, GR_Lsave_caller_BSP, GR_Lsave_RP);

  // This is not a leaf but we have a JavaFrameAnchor now and we will
  // check (create) exceptions afterward so this is ok.

  __ call_VM_leaf(CAST_FROM_FN_PTR(address, InterpreterRuntime::throw_StackOverflowError), GR4_thread);

  // Unwind to caller's interpreter state.
  __ pop_full_frame();
  __ ret();


#ifndef CORE

  //=============================================================================
  // Counter overflow.

  if (inc_counter) {
    // Handle invocation counter overflow
    __ bind(invocation_counter_overflow);
    generate_counter_overflow(retry_entry);
  }


  //=============================================================================
  // Run compiled code.  Compiled code signature handler will take care of parameter
  // list conversion.

  __ bind(run_compiled_code);
  generate_run_compiled_code();

  // We get here if there's an exception pending and we're not returning to the
  // call stub.  We're in the caller's context.
  
  __ ret();

#endif // !CORE


  native_entry = entry;
  return entry;
}


// Generate code for various sorts of method entries
//
address AbstractInterpreterGenerator::generate_method_entry(AbstractInterpreter::MethodKind kind) {
  address entry_point = NULL;

  switch (kind) {    
    case Interpreter::zerolocals             :                                                                         break;
    case Interpreter::zerolocals_synchronized:                                                                         break;
    case Interpreter::native                 : // Fall thru
    case Interpreter::native_synchronized    : entry_point = ((InterpreterGenerator*)this)->generate_native_entry();   break;
    case Interpreter::empty                  : entry_point = ((InterpreterGenerator*)this)->generate_empty_entry();    break;
    case Interpreter::accessor               : entry_point = ((InterpreterGenerator*)this)->generate_accessor_entry(); break;
    case Interpreter::abstract               : entry_point = ((InterpreterGenerator*)this)->generate_abstract_entry(); break;
    case Interpreter::java_lang_math_sin     :                                                                         break;
    case Interpreter::java_lang_math_cos     :                                                                         break;
    case Interpreter::java_lang_math_sqrt    :                                                                         break;
    default                                  : ShouldNotReachHere();                                                   break;
  }

  if (entry_point) {
    return entry_point;
  }
  return ((InterpreterGenerator*)this)->generate_interpreter_frame_manager();
}


// returns the activation size.
static int size_activation_helper(int extra_locals_size, int monitor_size) {
  return (extra_locals_size +                  // the addition space for locals
          2*BytesPerWord +                     // slop factor/abi space
          sizeof(cInterpreter) +               // interpreterState
          monitor_size);                       // monitors
}

// How much stack an topmost interpreter method activation needs in words.

int AbstractInterpreter::size_top_interpreter_activation(methodOop method) {
  // Computation is in bytes not words to match layout_activation_impl
  // below, but the return is in words.

  // might need to lock one monitor if synchronized
  const int monitor_size = method->is_synchronized() ?
                                                   sizeof(BasicObjectLock) : 0;
  const int locals_size = method->max_locals() * BytesPerWord;
  const int short_frame_size = size_activation_helper(locals_size, monitor_size);
  // This is a topmost frame so count the full frame.
  const int full_frame_size = round_to(short_frame_size + method->max_stack() * BytesPerWord, 16);
  return (full_frame_size/BytesPerWord);
}


#ifndef CORE
// This method tells the deoptimizer how big an interpreted frame must be:
int AbstractInterpreter::size_activation(methodOop method, 
					 int tempcount, 
					 int moncount, 
					 int callee_param_size, 
					 int callee_locals,
					 bool is_top_frame) {
  return layout_activation_impl(method,
				tempcount,
				moncount,
				callee_param_size,
				callee_locals,
				(frame*)NULL,
				(frame*)NULL,
				is_top_frame);
}

void cInterpreter::layout_interpreterState(interpreterState to_fill,
					   frame* caller,
					   frame* current,
					   methodOop method,
					   intptr_t* locals,
					   intptr_t* stack,
					   intptr_t* stack_base,
					   intptr_t* monitor_base,
					   intptr_t* frame_bottom,
					   bool is_top_frame
					   )
{
  // What about any vtable?
  //
  to_fill->_thread = JavaThread::current();
  // This gets filled in later but make it something recognizable for now
  to_fill->_bcp = method->code_base();
  to_fill->_locals = locals;
  to_fill->_constants = method->constants()->cache();
  to_fill->_method = method;
  to_fill->_mdx = NULL;
  to_fill->_stack = stack;
  if (is_top_frame && JavaThread::current()->popframe_forcing_deopt_reexecution() ) {
    to_fill->_msg = deopt_resume2; 
  } else {
    to_fill->_msg = method_resume; 
  }
  to_fill->_result._to_call._bcp_advance = 0;
  to_fill->_result._to_call._callee_entry_point = NULL; // doesn't matter to anyone
  to_fill->_result._to_call._callee = NULL; // doesn't matter to anyone
  to_fill->_prev_link = NULL;

  // Fill in the registers for the frame
  *current->register_addr(GR_Lmethod_addr) = (intptr_t) method;
  *current->register_addr(GR_Lstate) = (intptr_t) to_fill;
  *current->register_addr(GR_Iprev_state) = (intptr_t) NULL;
  *current->register_addr(GR_Ilocals) = (intptr_t) locals;
  //   GR_Itos    - intptr_t*    sender tos (pre pushed) Lesp
  // __ shladd(GR_Ilocals, parameter_count, LogBytesPerWord, GR_Itos);
  *current->register_addr(GR_Itos) = (intptr_t) (locals - method->size_of_parameters());

  if (caller->is_interpreted_frame()) {
    interpreterState prev  = caller->get_interpreterState();
    to_fill->_prev_link = prev;
    *current->register_addr(GR_Iprev_state) = (intptr_t) prev;
    // Make the prev callee look proper
    prev->_result._to_call._callee = method;
    if (*prev->_bcp == Bytecodes::_invokeinterface) {
      prev->_result._to_call._bcp_advance = 5;
    } else {
      prev->_result._to_call._bcp_advance = 3;
    }
  }
  to_fill->_native_mirror = NULL;
  to_fill->_stack_base = stack_base;
  // Need +1 here because stack_base points to the word just above the first expr stack entry
  // and stack_limit is supposed to point to the word just below the last expr stack entry.
  // See generate_compute_interpreter_state.
  to_fill->_stack_limit = stack_base - (method->max_stack() + 1);
  to_fill->_monitor_base = (BasicObjectLock*) monitor_base;
  
  // ia64 specific
  to_fill->_last_Java_pc = NULL;
  to_fill->_last_Java_fp = NULL;
  to_fill->_frame_bottom = frame_bottom;
  to_fill->_self_link = to_fill;
#ifdef ASSERT
  to_fill->_native_fresult[0] = 123456.789;
  to_fill->_native_fresult[1] = 123456.789;
  to_fill->_native_fresult[2] = 123456.789;
  to_fill->_native_lresult = CONST64(0xdeadcafedeafcafe);
#endif
}

void cInterpreter::pd_layout_interpreterState(interpreterState istate, address last_Java_pc, intptr_t* last_Java_fp) {
  istate->_last_Java_pc = last_Java_pc;
  istate->_last_Java_fp = last_Java_fp;
}


int AbstractInterpreter::layout_activation_impl(methodOop method, 
						int tempcount, // Number of slots on java expression stack in use
						int moncount,  // Number of active monitors
						int callee_param_size, 
						int callee_locals_size,
						frame* caller,
						frame* interpreter_frame,      
						bool is_top_frame) {
  // NOTE this code must exactly mimic what InterpreterGenerator::generate_compute_interpreter_state()
  // does as far as allocating an interpreter frame.
  // However there is an exception. With the C++ based interpreter only the top most frame has a
  // full sized expression stack
  // The 16 byte slop factor is both the abi scratch area and a place to hold
  // a result from a callee on its way to the callers stack


  int extra_locals_size = (method->max_locals() - method->size_of_parameters()) * BytesPerWord;
  int monitor_size = sizeof(BasicObjectLock) * moncount;

  int short_frame_size = size_activation_helper(extra_locals_size, 
                                                monitor_size);

  int full_frame_size = round_to(short_frame_size + method->max_stack() * BytesPerWord, 16);
  short_frame_size = round_to(short_frame_size + tempcount * BytesPerWord, 16);

  // the size the activation is right now. Only top frame is full size
  int frame_size = (is_top_frame ? full_frame_size : short_frame_size);
  
  /*
    if we actually have a frame to layout we must now fill in all the pieces. This means both
    the interpreterState (which is in the memory stack) and the registers (in the register stack).
    We only have to fill in a few registers into the register stack
  */
  if (interpreter_frame != NULL) {

    // MUCHO HACK



    intptr_t* frame_bottom = (intptr_t*) ((intptr_t)interpreter_frame->sp() - (full_frame_size - frame_size));

    /* Now fillin the interpreterState object */

    // We know that the caller has given us 16 bytes above the sp (the abi area)
    // However we don't know if there was alignment slop or not so we cannot 
    // determine our locals pointer only the caller of this frame can tell us.
    // Worse

    // QQQ should be easier now 
    // not really interpreter_frame->get_interpreterState() requires GR_Lstate to be set which
    // it isn't until below using this very calculation of cur_state.

    interpreterState cur_state = (interpreterState) ((intptr_t)caller->sp() - (extra_locals_size + sizeof(cInterpreter)));


    intptr_t* locals;
    if (caller->is_interpreted_frame()) {
      // locals must agree with the caller because it will be used to set the 
      // caller's tos when we return. 
      interpreterState prev  = caller->get_interpreterState();
      // stack() is prepushed.
      locals = prev->stack() + method->size_of_parameters();
    } else {
      // +2 for the ABI scratch area
      // stack alignment fools us here...
      // Assuming that stack
      // See note below this value can be off by 8 bytes
      // The extra killer is that locals must be correct now because our locals will get stored right after we
      // complete.
      // With the call_stub or c2i adapter if locals is wrong (off by alignment slop that is) it is harmless
      // because the value of locals will not be used once we return. 
      locals = (intptr_t*) ((intptr_t)caller->sp() + ( (method->size_of_parameters() - 1) + 2) * BytesPerWord) ;
    }
    // END MUCHO HACK

    intptr_t* monitor_base = (intptr_t*) cur_state;
    intptr_t* stack_base = (intptr_t*) ((intptr_t) monitor_base - monitor_size);
    /* +1 because stack is always prepushed */
    intptr_t* stack = (intptr_t*) ((intptr_t) stack_base - (tempcount + 1) * BytesPerWord);
			     

    cInterpreter::layout_interpreterState(cur_state, 
					  caller,
					  interpreter_frame,
					  method, 
					  locals, 
					  stack, 
					  stack_base, 
					  monitor_base, 
					  frame_bottom,
					  is_top_frame);

    cInterpreter::pd_layout_interpreterState(cur_state, interpreter_return_address, interpreter_frame->fp());
    /* Now save the new contents of the registers */

    assert(*interpreter_frame->register_addr(GR_Lsave_SP) == (intptr_t) caller->sp(), "Ack");
//    assert(*interpreter_frame->register_addr(GR_Lsave_caller_BSP) == (intptr_t) interpreter_frame->fp(), "Ack");
    // assert(*interpreter_frame->register_addr(GR_Lsave_RP) == (intptr_t) interpreter_return_address, "Ack");

    /*
       the deopt blob stores default values for these registers. So we don't have to

       *interpreter_frame->register_addr(GR_Lsave_LC) = ???;
       *interpreter_frame->register_addr(GR_Lsave_UNAT) = ???;
    */
#if 0

    *interpreter_frame->register_addr(GR_Lstate) = (intptr_t) cur_state;
    *interpreter_frame->register_addr(GR_Iprev_state) = (intptr_t) NULL;
    //
    // Problem here is that locals will become the caller's tos when  this
    // activation returns. Locals was calculated based on the caller's sp
    // and not from the top of the caller's expression stack. It MUST
    // be calculated based on the callers expression stack (or else
    // caller's expression must be shifted based on locals [doable]).
    //
    // The problem is that the calculation here assumes that the caller
    // expression top is a sp + 16 when depending on the amount of
    // stuff on the stack (unknowable here) it can be a sp + 16 or
    // sp + 24.
    //
    *interpreter_frame->register_addr(GR_Ilocals) = (intptr_t) locals; // ???
    //   GR_Itos    - intptr_t*    sender tos (pre pushed) Lesp
    // __ shladd(GR_Ilocals, parameter_count, LogBytesPerWord, GR_Itos);
    *interpreter_frame->register_addr(GR_Itos) = (intptr_t) locals - (method->size_of_parameters() ) * BytesPerWord; // ???
    // QQQ Yuck
    *interpreter_frame->register_addr(GR_Lmethod_addr) = (intptr_t) method;

#endif
#ifdef ASSERT
    /* QQQ would be nice to do some double checking of the layout here */
#endif // ASSERT

  }
  return frame_size/BytesPerWord;
}
#endif // not CORE


// --------------------------------------------------------------------------------

// Part of popframe stuff & FullSpeedJVMDI
static void removeActivation(void) {
  fatal("Remove activation");
}

InterpreterGenerator::InterpreterGenerator(StubQueue* code) 
 : AbstractInterpreterGenerator(code) {
   generate_all(); // down here so it can be "virtual"
   address* temp =  CAST_FROM_FN_PTR(address*, removeActivation);
   AbstractInterpreter::_remove_activation_preserving_args_entry = *temp;
}

// --------------------------------------------------------------------------------
// Non-product code
#ifndef PRODUCT

static char* C_msg(cInterpreter::messages msg) {
  switch (msg) {
     case cInterpreter::no_request:  return("no_request");
     case cInterpreter::initialize:  return("initialize");
     // status message to C++ interpreter
     case cInterpreter::method_entry:  return("method_entry");
     case cInterpreter::method_resume:  return("method_resume");
     case cInterpreter::got_monitors:  return("got_monitors");
     case cInterpreter::rethrow_exception:  return("rethrow_exception");
     // requests to frame manager from C++ interpreter
     case cInterpreter::call_method:  return("call_method");
     case cInterpreter::return_from_method:  return("return_from_method");
     case cInterpreter::retry_method:  return("retry_method");
     case cInterpreter::more_monitors:  return("more_monitors");
     case cInterpreter::throwing_exception:  return("throwing_exception");
     case cInterpreter::popping_frame:  return("popping_frame");
     // deopt
     case cInterpreter::deopt_resume:  return("deopt_resume");
     case cInterpreter::deopt_resume2:  return("deopt_resume2");
     default: return("BAD MSG");
  }
}
void 
cInterpreter::print() {
  tty->print_cr("thread: " INTPTR_FORMAT, (uintptr_t) this->_thread);
  tty->print_cr("bcp: " INTPTR_FORMAT, (uintptr_t) this->_bcp);
  tty->print_cr("locals: " INTPTR_FORMAT, (uintptr_t) this->_locals);
  tty->print_cr("constants: " INTPTR_FORMAT, (uintptr_t) this->_constants);
  {
    ResourceMark rm;
    char *method_name = _method->name_and_sig_as_C_string();
    tty->print_cr("method: " INTPTR_FORMAT "[ %s ]",  (uintptr_t) this->_method, method_name);
  }
#ifndef CORE
  tty->print_cr("mdx: " INTPTR_FORMAT, (uintptr_t) this->_mdx);
#endif
  tty->print_cr("stack: " INTPTR_FORMAT, (uintptr_t) this->_stack);
  tty->print_cr("msg: %s", C_msg(this->_msg));
  tty->print_cr("result_to_call._callee: " INTPTR_FORMAT, (uintptr_t) this->_result._to_call._callee);
  tty->print_cr("result_to_call._callee_entry_point: " INTPTR_FORMAT, (uintptr_t) this->_result._to_call._callee_entry_point);
  tty->print_cr("result_to_call._bcp_advance: %d ", this->_result._to_call._bcp_advance);
  tty->print_cr("result_return_kind 0x%x ", (int) this->_result._return_kind);
  tty->print_cr("prev_link: " INTPTR_FORMAT, (uintptr_t) this->_prev_link);
  tty->print_cr("native_mirror: " INTPTR_FORMAT, (uintptr_t) this->_native_mirror);
  tty->print_cr("stack_base: " INTPTR_FORMAT, (uintptr_t) this->_stack_base);
  tty->print_cr("stack_limit: " INTPTR_FORMAT, (uintptr_t) this->_stack_limit);
  tty->print_cr("monitor_base: " INTPTR_FORMAT, (uintptr_t) this->_monitor_base);
  tty->print_cr("last_Java_pc: " INTPTR_FORMAT, (uintptr_t) this->_last_Java_pc);
  tty->print_cr("last_Java_fp: " INTPTR_FORMAT, (uintptr_t) this->_last_Java_fp);
  tty->print_cr("frame_bottom: " INTPTR_FORMAT, (uintptr_t) this->_frame_bottom);
  tty->print_cr("self_link: " INTPTR_FORMAT, (uintptr_t) this->_self_link);
  tty->print_cr("&native_fresult: " INTPTR_FORMAT, (uintptr_t) &this->_native_fresult);
  tty->print_cr("native_lresult: " INTPTR_FORMAT, (uintptr_t) this->_native_lresult);
}

extern "C" {
    void PI(uintptr_t arg) {
	((cInterpreter*)arg)->print();
    }
}
#endif
