#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)interpreter_sparc.cpp	1.227 04/03/31 11:47:58 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_interpreter_sparc.cpp.incl"

#ifndef FAST_DISPATCH
#define FAST_DISPATCH 1
#endif
#undef FAST_DISPATCH


// Generation of Interpreter
//
// The InterpreterGenerator generates the interpreter into Interpreter::_code.


#define __ _masm->


//----------------------------------------------------------------------------------------------------


void InterpreterGenerator::save_native_result(void) {
  // result potentially in O0/O1: save it across calls
  const Address& l_tmp = InterpreterMacroAssembler::l_tmp;

  // result potentially in F0/F1: save it across calls
  const Address& d_tmp = InterpreterMacroAssembler::d_tmp;

  // save and restore any potential method result value around the unlocking operation
  __ stf(FloatRegisterImpl::D, F0, d_tmp);
#ifdef _LP64
  __ stx(O0, l_tmp);
#else
  __ std(O0, l_tmp);
#endif
}

void InterpreterGenerator::restore_native_result(void) {
  const Address& l_tmp = InterpreterMacroAssembler::l_tmp;
  const Address& d_tmp = InterpreterMacroAssembler::d_tmp;

  // Restore any method result value
  __ ldf(FloatRegisterImpl::D, d_tmp, F0);
#ifdef _LP64
  __ ldx(l_tmp, O0);
#else
  __ ldd(l_tmp, O0);
#endif
}

address AbstractInterpreterGenerator::generate_exception_handler_common(const char* name, const char* message, bool pass_oop) {
  assert(!pass_oop || message == NULL, "either oop or message but not both");
  address entry = __ pc();
  // expression stack must be empty before entering the VM if an exception happened
  __ empty_expression_stack();
  // load exception object
  __ set((intptr_t)name, G3_scratch);
  if (pass_oop) {
    __ call_VM(Oexception, CAST_FROM_FN_PTR(address, InterpreterRuntime::create_klass_exception), G3_scratch, Otos_i);
  } else {
    __ set((intptr_t)message, G4_scratch);
    __ call_VM(Oexception, CAST_FROM_FN_PTR(address, InterpreterRuntime::create_exception), G3_scratch, G4_scratch);
  }
  // throw exception
  assert(Interpreter::throw_exception_entry() != NULL, "generate it first");
  __ jump_to (Address(G3_scratch, Interpreter::throw_exception_entry()));
  __ delayed()->nop();
  return entry;
}


address AbstractInterpreterGenerator::generate_ArrayIndexOutOfBounds_handler(const char* name) {
  address entry = __ pc();
  // expression stack must be empty before entering the VM if an exception happened
  __ empty_expression_stack();
  // convention: expect aberrant index in register G3_scratch, then shuffle the 
  // index to G4_scratch for the VM call
  __ mov(G3_scratch, G4_scratch);
  __ set((intptr_t)name, G3_scratch);
  __ call_VM(Oexception, CAST_FROM_FN_PTR(address, InterpreterRuntime::throw_ArrayIndexOutOfBoundsException), G3_scratch, G4_scratch);
  __ should_not_reach_here();
  return entry;
}


address AbstractInterpreterGenerator::generate_StackOverflowError_handler() {
  address entry = __ pc();
  // expression stack must be empty before entering the VM if an exception happened
  __ empty_expression_stack();
  __ call_VM(Oexception, CAST_FROM_FN_PTR(address, InterpreterRuntime::throw_StackOverflowError));
  __ should_not_reach_here();
  return entry;
}


address AbstractInterpreterGenerator::generate_return_entry_for(TosState state, int step) {
  address entry = __ pc();

  // The callee returns with the stack in the same state as before the
  // call. In particular, SP and all interpreter local registers are
  // untouched. Any result is passed back in the O0/O1 or float
  // registers. Before continuing, the arguments must be popped from
  // the java expression stack; i.e., Lesp must be adjusted. (gri
  // 2/25/2000)

  
  const Register cache = G3_scratch;
  const Register size  = G1_scratch;
  __ get_cache_and_index_at_bcp(cache, G1_scratch, 1);
  __ ld_ptr(Address(cache, 0, in_bytes(constantPoolCacheOopDesc::base_offset()) +
                    in_bytes(ConstantPoolCacheEntry::flags_offset())), size);
  __ and3(size, 0xFF, size);                   // argument size in words
  __ sll(size, LogBytesPerWord, size);         // argument size in bytes
  __ add(Lesp, size, Lesp);                    // pop arguments
  __ dispatch_next(state, step);

  return entry;
}

  
#ifndef CORE
address AbstractInterpreterGenerator::generate_deopt_entry_for(TosState state, int step) {
  address entry = __ pc();
  __ get_constant_pool_cache(LcpoolCache); // load LcpoolCache
  { Label L;
    Address exception_addr (G2_thread, 0, in_bytes(Thread::pending_exception_offset()));

    __ ld_ptr(exception_addr, Gtemp);
    __ tst(Gtemp);
    __ brx(Assembler::equal, false, Assembler::pt, L);
    __ delayed()->nop();
    // The following line has been deleted from the Intel version:
    __ stop("new exception propagation across deoptimization has not been tested yet");
    __ call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::throw_pending_exception));
    __ should_not_reach_here();
    __ bind(L);              
  }
  __ dispatch_next(state, step);
  return entry;
}
#endif // CORE


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
// interpreter frame. The activation frame unwind code must be
// consistent with that of TemplateTable::_return(...). In the
// case of native methods, the caller's SP was not modified.
address AbstractInterpreterGenerator::generate_result_handler_for(BasicType type) {
  address entry = __ pc();
  Register Itos_i  = Otos_i ->after_save();
  Register Itos_l  = Otos_l ->after_save();
  Register Itos_l1 = Otos_l1->after_save();
  Register Itos_l2 = Otos_l2->after_save();
  Register rs = G0, rd = G0;   // hook for using the last restore
  switch (type) {
    case T_BOOLEAN: __ subcc(G0, O0, G0); __ addc(G0, 0, Itos_i); break; // !0 => true; 0 => false
    case T_CHAR   : __ sll(O0, 16, O0); __ srl(O0, 16, Itos_i);   break; // cannot use and3, 0xFFFF too big as immediate value!
    case T_BYTE   : __ sll(O0, 24, O0); __ sra(O0, 24, Itos_i);   break;
    case T_SHORT  : __ sll(O0, 16, O0); __ sra(O0, 16, Itos_i);   break;
    case T_LONG   : 
#ifndef _LP64
                    __ mov(O1, Itos_l2);  // move other half of long
#endif              // ifdef or no ifdef, fall through to the T_INT case
    case T_INT    : rs = O0; rd = Itos_i;                       break;
    case T_VOID   : /* nothing to do */                         break;
    case T_FLOAT  : assert(F0 == Ftos_f, "fix this code" );     break;
    case T_DOUBLE : assert(F0 == Ftos_d, "fix this code" );     break;
    case T_OBJECT :
      { Label L;
        __ addcc(G0, O0, Itos_i);
        __ brx(Assembler::notZero, true, Assembler::pt, L);     // if result is not NULL:
        __ delayed()->ld_ptr(O0, 0, Itos_i);                    // unbox it
        __ bind(L);
      }
      __ verify_oop(Itos_i);
      break;
    default       : ShouldNotReachHere();
  }
  rd = rd->after_restore();
  __ ret();                           // return from interpreter activation
  __ delayed()->restore(G0, rs, rd);  // remove interpreter frame
  NOT_PRODUCT(__ emit_long(0);)       // marker for disassembly
  return entry;
}

#ifndef _LP64
address AbstractInterpreterGenerator::generate_slow_signature_handler() {
  address entry = __ pc();
  Argument argv(0, true);

  // We are in the jni transition frame. Save the last_java_frame corresponding to the
  // outer interpreter frame
  //
  __ set_last_Java_frame(FP, noreg);
  // make sure the interpreter frame we've pushed has a valid return pc
  __ mov(O7, I7);
  __ mov(Lmethod, G3_scratch);
  __ mov(Llocals, G4_scratch);
  __ save_frame(0);
  __ mov(G2_thread, L7_thread_cache);
  __ add(argv.address_in_frame(), O3);
  __ mov(G2_thread, O0);
  __ mov(G3_scratch, O1);
  __ call(CAST_FROM_FN_PTR(address, InterpreterRuntime::slow_signature_handler), relocInfo::runtime_call_type);
  __ delayed()->mov(G4_scratch, O2);
  __ mov(L7_thread_cache, G2_thread);
  __ reset_last_Java_frame();

  // load the register arguments (the C code packed them as varargs)
  for (Argument ldarg = argv.successor(); ldarg.is_register(); ldarg = ldarg.successor()) {
      __ ld_ptr(ldarg.address_in_frame(), ldarg.as_register());
  }
  __ ret();
  __ delayed()->
     restore(O0, 0, Lscratch);  // caller's Lscratch gets the result handler
  return entry;
}


#else
// LP64 passes floating point arguments in F1, F3, F5, etc. instead of
// O0, O1, O2 etc..
// Doubles are passed in D0, D2, D4
// We store the signature of the first 16 arguments in the first argument
// slot because it will be overwritten prior to calling the native
// function, with the pointer to the JNIEnv.
// If LP64 there can be up to 16 floating point arguments in registers
// or 6 integer registers.
address AbstractInterpreterGenerator::generate_slow_signature_handler() {

  enum {
    non_float  = 0,
    float_sig  = 1,
    double_sig = 2,
    sig_mask   = 3
  };

  address entry = __ pc();
  Argument argv(0, true);

  // We are in the jni transition frame. Save the last_java_frame corresponding to the
  // outer interpreter frame
  //
  __ set_last_Java_frame(FP, noreg);
  // make sure the interpreter frame we've pushed has a valid return pc
  __ mov(O7, I7);
  __ mov(Lmethod, G3_scratch);
  __ mov(Llocals, G4_scratch);
  __ save_frame(0);
  __ mov(G2_thread, L7_thread_cache);
  __ add(argv.address_in_frame(), O3);
  __ mov(G2_thread, O0);
  __ mov(G3_scratch, O1);
  __ call(CAST_FROM_FN_PTR(address, InterpreterRuntime::slow_signature_handler), relocInfo::runtime_call_type);
  __ delayed()->mov(G4_scratch, O2);
  __ mov(L7_thread_cache, G2_thread);
  __ reset_last_Java_frame();


  // load the register arguments (the C code packed them as varargs)
  Address Sig = argv.address_in_frame();        // Argument 0 holds the signature
  __ ld_ptr( Sig, G3_scratch );                   // Get register argument signature word into G3_scratch
  __ mov( G3_scratch, G4_scratch);
  __ srl( G4_scratch, 2, G4_scratch);             // Skip Arg 0
  Label done;
  for (Argument ldarg = argv.successor(); ldarg.is_float_register(); ldarg = ldarg.successor()) {
    Label NonFloatArg;
    Label LoadFloatArg;
    Label LoadDoubleArg;
    Label NextArg;
    Address a = ldarg.address_in_frame();
    __ andcc(G4_scratch, sig_mask, G3_scratch);
    __ br(Assembler::zero, false, Assembler::pt, NonFloatArg);
    __ delayed()->nop();

    __ cmp(G3_scratch, float_sig );
    __ br(Assembler::equal, false, Assembler::pt, LoadFloatArg);
    __ delayed()->nop();

    __ cmp(G3_scratch, double_sig );
    __ br(Assembler::equal, false, Assembler::pt, LoadDoubleArg);
    __ delayed()->nop();

    __ bind(NonFloatArg);
    // There are only 6 integer register arguments!
    if ( ldarg.is_register() ) 
      __ ld_ptr(ldarg.address_in_frame(), ldarg.as_register());
    else {
    // Optimization, see if there are any more args and get out prior to checking
    // all 16 float registers.  My guess is that this is rare.  
    // If is_register is false, then we are done the first six integer args.
      __ tst(G4_scratch);
      __ brx(Assembler::zero, false, Assembler::pt, done);
      __ delayed()->nop();

    }
    __ ba(false, NextArg);
    __ delayed()->srl( G4_scratch, 2, G4_scratch );

    __ bind(LoadFloatArg);
    __ ldf( FloatRegisterImpl::S, a, ldarg.as_float_register(), 4);
    __ ba(false, NextArg);
    __ delayed()->srl( G4_scratch, 2, G4_scratch );

    __ bind(LoadDoubleArg);
    __ ldf( FloatRegisterImpl::D, a, ldarg.as_double_register() );
    __ ba(false, NextArg);
    __ delayed()->srl( G4_scratch, 2, G4_scratch );

    __ bind(NextArg);
 
  }

  __ bind(done);
  __ ret();
  __ delayed()->
     restore(O0, 0, Lscratch);  // caller's Lscratch gets the result handler
  return entry;
}
#endif


address AbstractInterpreterGenerator::generate_safept_entry_for(TosState state, address runtime_entry) {
  address entry = __ pc();
  __ push(state);
  __ call_VM(noreg, runtime_entry);
  __ dispatch_via(vtos, Interpreter::normal_table(vtos));
  return entry;
}


address AbstractInterpreterGenerator::generate_continuation_for(TosState state) {
  address entry = __ pc();
  __ dispatch_next(state);
  return entry;
}

//
// Helpers for commoning out cases in the various type of method entries.
//
#ifndef CORE

// increment invocation count & check for overflow
//
// Note: checking for negative value instead of overflow
//       so we have a 'sticky' overflow test
//
// Lmethod: method
// ??: invocation counter
//
void InterpreterGenerator::generate_counter_incr(Label* overflow, Label* profile_method, Label* profile_method_continue) {
  // Update standard invocation counters
  __ increment_invocation_counter(O0, G3_scratch);
#ifdef COMPILER2
  if (ProfileInterpreter) {  // %%% Merge this into methodDataOop
    Address interpreter_invocation_counter(Lmethod, 0, in_bytes(methodOopDesc::interpreter_invocation_counter_offset()));
    __ ld(interpreter_invocation_counter, G3_scratch);
    __ inc(G3_scratch);
    __ st(G3_scratch, interpreter_invocation_counter);
  }
#endif // COMPILER2

  if (ProfileInterpreter && profile_method != NULL) {
    // Test to see if we should create a method data oop
    Address profile_limit(G3_scratch, (address)&InvocationCounter::InterpreterProfileLimit);
    __ sethi(profile_limit);
    __ ld(profile_limit, G3_scratch);
    __ cmp(O0, G3_scratch);
    __ br(Assembler::lessUnsigned, false, Assembler::pn, *profile_method_continue);
    __ delayed()->nop();

    // if no method data exists, go to profile_method
    __ test_method_data_pointer(*profile_method);
  }

  Address invocation_limit(G3_scratch, (address)&InvocationCounter::InterpreterInvocationLimit);
  __ sethi(invocation_limit);
  __ ld(invocation_limit, G3_scratch);
  __ cmp(O0, G3_scratch);
  __ br(Assembler::greaterEqualUnsigned, false, Assembler::pn, *overflow);
  __ delayed()->nop();

}

void InterpreterGenerator::generate_counter_overflow(bool native_call, Label& Lentry) {

  // Generate code to initiate compilation on the counter overflow.
  // After entering the vm we remove the activation and retry
  // the entry point in case the compilation is complete

  // InterpreterRuntime::frequency_counter_overflow takes two arguments,
  // the first indicates if the counter overflow occurs at a backwards branch (NULL bcp)
  // and the second is only used when the first is true.  We pass zero for both.
  // The call returns the address of the verified entry point for the method or NULL
  // if the compilation did not complete (either went background or bailed out).
  __ set((int)false, O2);
  __ call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::frequency_counter_overflow), O2, O2, true); 
  // returns verified_entry_point or NULL

  // restore O0 (must contain receiver in case of virtual calls)
  __ ld_ptr(Llocals, 0, O0->after_save());          // is this needed?
  
  // restore G5_method & Gargs
  __ add(Llocals, BytesPerWord, Gargs);
  __ mov(Lmethod, G5_method);
  
  // determine arguments
  __ lduh(G5_method, in_bytes(methodOopDesc::size_of_parameters_offset()), G3_scratch);
  __ sll(G3_scratch, LogBytesPerWord, G1_scratch);       // parameter size in bytes
  __ sub(Gargs, G1_scratch, Gargs);                 // points to last argument

  // determine extra space for non-argument locals
  // G3_scratch: parameter size in words
  if (native_call) {
    // the caller's SP was not adjusted => simply unwind and redo method entry
    __ ba(false, Lentry);
    __ delayed()->restore();
  } else {
    // the caller's SP was adjusted upon method entry to accomodate
    // the callee's non-argument locals => determine the adjustment
    __ lduh(G5_method, in_bytes(methodOopDesc::size_of_locals_offset()), G1_scratch);
    __ compute_extra_locals_size_in_bytes(G3_scratch, G1_scratch, G1_scratch);
    // restore register window & adjust SP, then redo method entry
    __ ba(false, Lentry);                      // redo method entry
    __ delayed()->restore(FP, G1_scratch, SP);      // restore registers & reset caller's SP
  }

}

void InterpreterGenerator::generate_run_compiled_code(void) {

  // compiled code for method exists
  // G5_method: methodOop
  // G3_scratch: nmethod

  
  assert_different_registers(G5_method, G3_scratch, G1_scratch, Gargs);
  __ ld_ptr(Address(G3_scratch, 0, nmethod::interpreter_entry_point_offset()), G1_scratch);

#ifdef COMPILER2
  // make sure interpreter entry point exists
  {
    Label L;

    // The call to nmethod_entry in the vm is especially stressful to stack walking
    // if we are doing SafepointALot then call into the vm even if we already know
    // where we are going.

    if (!SafepointALot) {
      __ tst(G1_scratch);
      __ brx(Assembler::notZero, false, Assembler::pn, L);
      __ delayed()->nop();
    }
    
    // Do not use __ call_VM here, since it will call to the InterpreterMacroAssembler, which
    // assumes that the current frame (the caller) is interpreted (but it could be compiled)
    __ save_frame(0);
    __ super_call_VM(L7_thread_cache, 
                     G5_method, 
                     FP, 
                     CAST_FROM_FN_PTR(address, InterpreterRuntime::nmethod_entry_point), 
                     G5_method, 
                     G3_scratch, 
                     false);
    __ mov(O0, G1_scratch);
    __ restore();
    
    // it is safe to handle exception now.
    __ check_and_forward_exception(G3_scratch);
    
    // G5_method: methodOop
    // G1_scratch: interpreter nmethod entry
#ifdef ASSERT
    { Label L;
    __ tst(G1_scratch);
    __ brx(Assembler::notZero, false, Assembler::pn, L);
    __ delayed()->nop();
    __ stop("interpreter entry point == NULL");
    __ bind(L);
    }
#endif // ASSERT
    // Note: At this point, the nmethod may have been recompiled once more; i.e., if one would
    //       look at the nmethod's interpreter entry point, it could be NULL! However, since
    //       the interpreter entry point is simply the corresponding i2c adapter, which is
    //       dependent of the method's signature only, we don't care. If the nmethod has been
    //       deoptimized, we will end up in a zombie nmethod, which is ok, too.
    __ bind(L);
  }
#endif // COMPILER2

  // G5_method: compiled code expects methodOop in G5_method 
  // G1_scratch: interpreter nmethod entry
  __ JMP(G1_scratch, 0);                // enter compiled code via interpreter entry point
  __ delayed()->nop();
}

void  InterpreterGenerator::generate_check_compiled_code(Label &run_compiled_code) {
  // check if compiled code exists
  const Address compiled_code     (G5_method, 0, in_bytes(methodOopDesc::compiled_code_offset()));
  Label skip_compiled_code;
  if (JvmtiExport::can_post_interpreter_events()) {
    // JVMTI events, such as single-stepping, are implemented partly by avoiding running
    // compiled code in threads for which the event is enabled.  Check here for
    // interp_only_mode if these events CAN be enabled.
    __ verify_thread();

    const Address interp_only       (G2_thread, 0, in_bytes(JavaThread::interp_only_mode_offset()));

    __ ld(interp_only, G3_scratch);
    __ tst(G3_scratch);
    __ br(Assembler::notZero, false, Assembler::pn, skip_compiled_code);
    __ delayed()->nop();
  }

  __ ld_ptr(compiled_code, G3_scratch);
  __ tst(G3_scratch);
  __ brx(Assembler::notZero, false, Assembler::pt, run_compiled_code);
  __ delayed()->nop();

  if (JvmtiExport::can_post_interpreter_events()) {
    __ bind(skip_compiled_code);
  }
}

#endif /* !CORE */

// Allocate monitor and lock method (asm interpreter)
// ebx - methodOop
// 
void InterpreterGenerator::lock_method(void) {
  const Address access_flags      (Lmethod, 0, in_bytes(methodOopDesc::access_flags_offset()));
  __ ld(access_flags, O0);

#ifdef ASSERT
 { Label ok;
   __ btst(JVM_ACC_SYNCHRONIZED, O0);
   __ br( Assembler::notZero, false, Assembler::pt, ok);
   __ delayed()->nop();
   __ stop("method doesn't need synchronization");
   __ bind(ok);
  }
#endif // ASSERT

  // get synchronization object to O0
  { Label done;
    const int mirror_offset = klassOopDesc::klass_part_offset_in_bytes() + Klass::java_mirror_offset_in_bytes();
    __ btst(JVM_ACC_STATIC, O0);
    __ br( Assembler::zero, true, Assembler::pt, done);
    __ delayed()->ld_ptr(Llocals, 0, O0); // get receiver for not-static case

    __ ld_ptr( Lmethod, in_bytes(methodOopDesc::constants_offset()), O0);
    __ ld_ptr( O0, constantPoolOopDesc::pool_holder_offset_in_bytes(), O0);

    // lock the mirror, not the klassOop
    __ ld_ptr( O0, mirror_offset, O0);

#ifdef ASSERT
    __ tst(O0);
    __ breakpoint_trap(Assembler::zero);
#endif // ASSERT

    __ bind(done);
  }

  __ add_monitor_to_stack(true, noreg, noreg);  // allocate monitor elem
  __ st_ptr( O0, Lmonitors, BasicObjectLock::obj_offset_in_bytes());   // store object
  // __ untested("lock_object from method entry");
  __ lock_object(Lmonitors, O0);
}


void InterpreterGenerator::generate_stack_overflow_check(Register Rframe_size,
                                                         Register Rscratch,
                                                         Register Rscratch2) {
  const int page_size = os::vm_page_size();
  Address saved_exception_pc(G2_thread, 0,
                             in_bytes(JavaThread::saved_exception_pc_offset()));
  Label after_frame_check;

  assert_different_registers(Rframe_size, Rscratch, Rscratch2);

  __ set( page_size,   Rscratch );
  __ cmp( Rframe_size, Rscratch );

  __ br( Assembler::lessEqual, false, Assembler::pt, after_frame_check );
  __ delayed()->nop();

  // get the stack base, and in debug, verify it is non-zero
  __ ld_ptr( G2_thread, in_bytes(Thread::stack_base_offset()), Rscratch );
#ifdef ASSERT
  Label base_not_zero;
  __ cmp( Rscratch, G0 );
  __ brx( Assembler::notEqual, false, Assembler::pn, base_not_zero );
  __ delayed()->nop();
  __ stop("stack base is zero in generate_stack_overflow_check");
  __ bind(base_not_zero);
#endif

  // get the stack size, and in debug, verify it is non-zero
  assert( sizeof(size_t) == sizeof(intptr_t), "wrong load size" );
  __ ld_ptr( G2_thread, in_bytes(Thread::stack_size_offset()), Rscratch2 );
#ifdef ASSERT
  Label size_not_zero;
  __ cmp( Rscratch2, G0 );
  __ brx( Assembler::notEqual, false, Assembler::pn, size_not_zero );
  __ delayed()->nop();
  __ stop("stack size is zero in generate_stack_overflow_check");
  __ bind(size_not_zero);
#endif

  // compute the beginning of the protected zone minus the requested frame size
  __ sub( Rscratch, Rscratch2,   Rscratch );
  __ set( (StackRedPages+StackYellowPages) * page_size, Rscratch2 );
  __ add( Rscratch, Rscratch2,   Rscratch );

  // Add in the size of the frame (which is the same as subtracting it from the
  // SP, which would take another register
  __ add( Rscratch, Rframe_size, Rscratch );

  // the frame is greater than one page in size, so check against
  // the bottom of the stack
  __ cmp( SP, Rscratch );
  __ brx( Assembler::greater, false, Assembler::pt, after_frame_check );
  __ delayed()->nop();

  // Save the return address as the exception pc
  __ st_ptr(O7, saved_exception_pc);

  // the stack will overflow, throw an exception
  __ call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::throw_StackOverflowError));

  // if you get to here, then there is enough stack space
  __ bind( after_frame_check );
}


//
// Generate a fixed interpreter frame. This is identical setup for interpreted
// methods and for native methods hence the shared code.

void InterpreterGenerator::generate_fixed_frame(bool native_call) {
  //
  //
  // The entry code sets up a new interpreter frame in 4 steps:
  //
  // 1) Increase caller's SP by for the extra local space needed:
  //    (check for overflow)
  //    Efficient implementation of xload/xstore bytecodes requires
  //    that arguments and non-argument locals are in a contigously
  //    addressable memory block => non-argument locals must be
  //    allocated in the caller's frame.
  //    
  // 2) Create a new stack frame and register window:
  //    The new stack frame must provide space for the standard
  //    register save area, the maximum java expression stack size,
  //    the monitor slots (0 slots initially), and some frame local
  //    scratch locations.
  //
  // 3) The following interpreter activation registers must be setup:
  //    Lesp       : expression stack pointer
  //    Lbcp       : bytecode pointer
  //    Lmethod    : method
  //    Llocals    : locals pointer
  //    Lmonitors  : monitor pointer
  //    LcpoolCache: constant pool cache
  //
  // 4) Initialize the non-argument locals if necessary:
  //    Non-argument locals may need to be initialized to NULL
  //    for GC to work. If the oop-map information is accurate
  //    (in the absence of the JSR problem), no initialization
  //    is necessary.
  //
  // (gri - 2/25/2000)


  const Address size_of_parameters(G5_method, 0, in_bytes(methodOopDesc::size_of_parameters_offset()));
  const Address size_of_locals    (G5_method, 0, in_bytes(methodOopDesc::size_of_locals_offset()));
  const Address max_stack         (G5_method, 0, in_bytes(methodOopDesc::max_stack_offset()));
  int rounded_vm_local_words = round_to( frame::interpreter_frame_vm_local_words, WordsPerLong );

  const int extra_space =
    rounded_vm_local_words +                   // frame local scratch space
    frame::memory_parameter_word_sp_offset +   // register save area
    (native_call ? frame::interpreter_frame_extra_outgoing_argument_words : 0);

  const Register Glocals_size = G3;
  const Register Otmp1 = O3;
  const Register Otmp2 = O4;
  const Register OsavedSP = IsavedSP->after_restore();
  // Lscratch can't be used as a temporary because the call_stub uses
  // it to assert that the stack frame was setup correctly.

  __ lduh( size_of_parameters, Glocals_size);

  // Gargs points to first local + BytesPerWord
  // Set the saved SP after the register window save
  //
  __ mov(SP, OsavedSP);
  assert_different_registers(Gargs, Glocals_size, Gframe_size);
  __ sll(Glocals_size, LogBytesPerWord, Otmp1);
  __ add(Gargs, Otmp1, Gargs);
  
  if (native_call) {
    __ calc_mem_param_words( Glocals_size, Gframe_size );
    __ add( Gframe_size,  extra_space, Gframe_size);
    __ round_to( Gframe_size, WordsPerLong );
    __ sll( Gframe_size, LogBytesPerWord, Gframe_size );
  } else {

    //
    // Compute number of locals in method apart from incoming parameters
    //
    __ lduh( size_of_locals, Otmp1 );
    __ sub( Otmp1, Glocals_size, Glocals_size );
    __ round_to( Glocals_size, WordsPerLong );
    __ sll( Glocals_size, LogBytesPerWord, Glocals_size );

    // see if the frame is greater than one page in size. If so,
    // then we need to verify there is enough stack space remaining
    // Frame_size = (max_stack + extra_space) * BytesPerWord;
    __ lduh( max_stack, Gframe_size );
    __ add( Gframe_size, extra_space, Gframe_size );
    __ round_to( Gframe_size, WordsPerLong );
    __ sll( Gframe_size, LogBytesPerWord, Gframe_size);

    // Add in java locals size for stack overflow check only
    __ add( Gframe_size, Glocals_size, Gframe_size );

    const Register Otmp2 = O4;
    assert_different_registers(Otmp1, Otmp2, OsavedSP);
    generate_stack_overflow_check(Gframe_size, Otmp1, Otmp2);

    __ sub( Gframe_size, Glocals_size, Gframe_size);

    //
    // bump SP to accomodate the extra locals
    //
    __ sub( SP, Glocals_size, SP );
  }

  //
  // now set up a stack frame with the size computed above
  //
  __ neg( Gframe_size );
  __ save( SP, Gframe_size, SP );

  //
  // now set up all the local cache registers
  //
  // NOTE: At this point, Lbyte_code/Lscratch has been modified. Note
  // that all present references to Lbyte_code initialize the register
  // immediately before use
  if (native_call) {
    __ mov(G0, Lbcp);
  } else {
    __ ld_ptr(Address(G5_method, 0, in_bytes(methodOopDesc::const_offset())), Lbcp );
    __ add(Address(Lbcp, 0, in_bytes(constMethodOopDesc::codes_offset())), Lbcp );
  }
  __ mov( G5_method, Lmethod);                 // set Lmethod
  __ get_constant_pool_cache( LcpoolCache );   // set LcpoolCache
  __ sub(FP, rounded_vm_local_words * BytesPerWord, Lmonitors ); // set Lmonitors
#ifdef _LP64
  __ add( Lmonitors, STACK_BIAS, Lmonitors );   // Account for 64 bit stack bias
#endif
  __ sub(Lmonitors, BytesPerWord, Lesp);       // set Lesp

  // setup interpreter activation registers
  __ sub(Gargs, BytesPerWord, Llocals);        // set Llocals

#ifndef CORE
  if (ProfileInterpreter) {
#ifdef FAST_DISPATCH
    // FAST_DISPATCH and ProfileInterpreter are mutually exclusive since 
    // they both use I2.
    assert(0, "FAST_DISPATCH and +ProfileInterpreter are mutually exclusive");
#endif // FAST_DISPATCH
    __ set_method_data_pointer();
  }
#endif // !CORE

}

// End of helpers

// Various method entries

// Abstract method entry
// Attempt to execute abstract method. Throw exception
//
address InterpreterGenerator::generate_abstract_entry(void) {
  address entry = __ pc();
  // abstract method entry
  // throw exception
  __ call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::throw_AbstractMethodError));
  // the call_VM checks for exception, so we should never return here.
  __ should_not_reach_here();
  return entry;

}

// Empty method, generate a very fast return.

address InterpreterGenerator::generate_empty_entry(void) {

  // A method that does nother but return...

  address entry = __ pc();


  const Address size_of_parameters(G5_method, 0, in_bytes(methodOopDesc::size_of_parameters_offset()));
  Label Lentry;
  __ bind(Lentry);

  __ verify_oop(G5_method);

  const Register Glocals_size = G3;
  assert_different_registers(Glocals_size, G4_scratch, Gframe_size);

  // do nothing for empty methods (do not even increment invocation counter)
  if ( UseFastEmptyMethods) {
    // Code: _return
    __ retl();
    __ delayed()->nop();

    return entry;
  }
  return NULL;
}

// Call an accessor method (assuming it is resolved, otherwise drop into vanilla (slow path) entry

// Generates code to elide accessor methods
// Uses G3_scratch and G1_scratch as scratch
address InterpreterGenerator::generate_accessor_entry(void) {

  // Code: _aload_0, _(i|a)getfield, _(i|a)return or any rewrites thereof;
  // parameter size = 1
  // Note: We can only use this code if the getfield has been resolved
  //       and if we don't have a null-pointer exception => check for
  //       these conditions first and use slow path if necessary.
  address entry = __ pc();
  Label slow_path;

  if ( UseFastAccessorMethods) {
    // Check if local 0 != NULL
    __ ld_ptr(Gargs, G0, Otos_i);  // get local 0
    __ tst(Otos_i);  // check if local 0 == NULL and go the slow path
    __ brx(Assembler::zero, false, Assembler::pn, slow_path);
    __ delayed()->nop();
    
    // read first instruction word and extract bytecode @ 1 and index @ 2
    // get first 4 bytes of the bytecodes (big endian!)
    __ ld_ptr(Address(G5_method, 0, in_bytes(methodOopDesc::const_offset())), G1_scratch);
    __ ld(Address(G1_scratch, 0, in_bytes(constMethodOopDesc::codes_offset())), G1_scratch);

    // move index @ 2 far left then to the right most two bytes.
    __ sll(G1_scratch, 2*BitsPerByte, G1_scratch); 
    __ srl(G1_scratch, 2*BitsPerByte - exact_log2(in_words(
                      ConstantPoolCacheEntry::size()) * BytesPerWord), G1_scratch);

    // get constant pool cache
    __ ld_ptr(G5_method, in_bytes(methodOopDesc::constants_offset()), G3_scratch);
    __ ld_ptr(G3_scratch, constantPoolOopDesc::cache_offset_in_bytes(), G3_scratch);

    // get specific constant pool cache entry
    __ add(G3_scratch, G1_scratch, G3_scratch);

    // Check the constant Pool cache entry to see if it has been resolved.
    // If not, need the slow path.
    ByteSize cp_base_offset = constantPoolCacheOopDesc::base_offset();
    __ ld_ptr(G3_scratch, in_bytes(cp_base_offset + ConstantPoolCacheEntry::indices_offset()), G1_scratch);
    __ srl(G1_scratch, 2*BitsPerByte, G1_scratch);
    __ and3(G1_scratch, 0xFF, G1_scratch);
    __ cmp(G1_scratch, Bytecodes::_getfield);
    __ br(Assembler::notEqual, false, Assembler::pn, slow_path);
    __ delayed()->nop();
      
    // Get the type and return field offset from the constant pool cache
    __ ld_ptr(G3_scratch, in_bytes(cp_base_offset + ConstantPoolCacheEntry::flags_offset()), G1_scratch);
    __ ld_ptr(G3_scratch, in_bytes(cp_base_offset + ConstantPoolCacheEntry::f2_offset()), G3_scratch);

    Label xreturn_path;
    // Need to differentiate between igetfield, agetfield, bgetfield etc.
    // because they are different sizes.
    // Get the type from the constant pool cache
    __ srl(G1_scratch, ConstantPoolCacheEntry::tosBits, G1_scratch);
    // Make sure we don't need to mask G1_scratch for tosBits after the above shift
    ConstantPoolCacheEntry::verify_tosBits();
    __ cmp(G1_scratch, atos );
    __ br(Assembler::equal, true, Assembler::pt, xreturn_path);
    __ delayed()->ld_ptr(Otos_i, G3_scratch, Otos_i);
    __ cmp(G1_scratch, itos);
    __ br(Assembler::equal, true, Assembler::pt, xreturn_path);
    __ delayed()->ld(Otos_i, G3_scratch, Otos_i);
    __ cmp(G1_scratch, stos);
    __ br(Assembler::equal, true, Assembler::pt, xreturn_path);
    __ delayed()->ldsh(Otos_i, G3_scratch, Otos_i);
    __ cmp(G1_scratch, ctos);
    __ br(Assembler::equal, true, Assembler::pt, xreturn_path);
    __ delayed()->lduh(Otos_i, G3_scratch, Otos_i);
#ifdef ASSERT
    __ cmp(G1_scratch, btos);
    __ br(Assembler::equal, true, Assembler::pt, xreturn_path);
    __ delayed()->ldsb(Otos_i, G3_scratch, Otos_i);
    __ should_not_reach_here();
#endif
    __ ldsb(Otos_i, G3_scratch, Otos_i);
    __ bind(xreturn_path);

    // _ireturn/_areturn
    __ retl();                      // return from leaf routine
    __ delayed()->nop();

    // Generate regular method entry
    __ bind(slow_path);
    (void) generate_asm_interpreter_entry(false);
    return entry;
  }
  return NULL;
}


//
// Interpreter stub for calling a native method. (asm interpreter)
// This sets up a somewhat different looking stack for calling the native method
// than the typical interpreter frame setup.
//

address InterpreterGenerator::generate_native_entry(bool synchronized) {
  address entry = __ pc();

  // the following temporary registers are used during frame creation
  const Register Gtmp1 = G3_scratch ;
  const Register Gtmp2 = G1_scratch;
  bool inc_counter  = UseCompiler || CountCompiledCalls;

  // make sure registers are different!
  assert_different_registers(G2_thread, G5_method, Gargs, Gtmp1, Gtmp2);

  const Address Laccess_flags     (Lmethod, 0, in_bytes(methodOopDesc::access_flags_offset()));

  Label Lentry;
  __ bind(Lentry);

  __ verify_oop(G5_method);

  const Register Glocals_size = G3;
  assert_different_registers(Glocals_size, G4_scratch, Gframe_size);

  // make sure method is native & not abstract
  // rethink these assertions - they can be simplified and shared (gri 2/25/2000)
#ifdef ASSERT
  __ ld(G5_method, in_bytes(methodOopDesc::access_flags_offset()), Gtmp1);
  {
    Label L;
    __ btst(JVM_ACC_NATIVE, Gtmp1);
    __ br(Assembler::notZero, false, Assembler::pt, L);
    __ delayed()->nop();
    __ stop("tried to execute non-native method as native");
    __ bind(L);
  }
  { Label L;
    __ btst(JVM_ACC_ABSTRACT, Gtmp1);
    __ br(Assembler::zero, false, Assembler::pt, L);
    __ delayed()->nop();
    __ stop("tried to execute abstract method as non-abstract");
    __ bind(L);
  }
#endif // ASSERT

#ifndef CORE
  // Generate check for compiled code
  Label run_compiled_code;
  if (!PreferInterpreterNativeStubs && !CompileTheWorld) {
    generate_check_compiled_code(run_compiled_code);
  }  
#endif
 
 // generate the code to allocate the interpreter stack frame

  generate_fixed_frame(true);

  //
  // No locals to initialize for native method
  //

  // this slot will be set later, we initialize it to null here just in
  // case we get a GC before the actual value is stored later
  __ st_ptr(G0, Address(FP, 0, (frame::interpreter_frame_mirror_offset*wordSize) + STACK_BIAS));

  const Address do_not_unlock_if_synchronized(G2_thread, 0,
      in_bytes(JavaThread::do_not_unlock_if_synchronized_offset()));
  // Since at this point in the method invocation the exception handler
  // would try to exit the monitor of synchronized methods which hasn't
  // been entered yet, we set the thread local variable
  // _do_not_unlock_if_synchronized to true. If any exception was thrown by
  // runtime, exception handling i.e. unlock_if_synchronized_method will
  // check this thread local flag.
  // This flag has two effects, one is to force an unwind in the topmost
  // interpreter frame and not perform an unlock while doing so.

  __ mov(1, G3_scratch);
  __ st(G3_scratch, do_not_unlock_if_synchronized);

#ifndef CORE
  // increment invocation counter and check for overflow
  //
  // Note: checking for negative value instead of overflow
  //       so we have a 'sticky' overflow test (may be of
  //       importance as soon as we have true MT/MP)
  Label invocation_counter_overflow; 
  if (inc_counter) {
    generate_counter_incr(&invocation_counter_overflow, NULL, NULL);
  }
#endif // CORE

  bang_stack_shadow_pages(true);

  // reset the _do_not_unlock_if_synchronized flag
  __ st(G0, do_not_unlock_if_synchronized);

  // check for synchronized methods
  // Must happen AFTER invocation_counter check and stack overflow check,
  // so method is not locked if overflows.

  if (synchronized) {
    lock_method();
  } else {
#ifdef ASSERT
    { Label ok;
      __ ld(Laccess_flags, O0);
      __ btst(JVM_ACC_SYNCHRONIZED, O0);
      __ br( Assembler::zero, false, Assembler::pt, ok);
      __ delayed()->nop();
      __ stop("method needs synchronization");
      __ bind(ok);
    }
#endif // ASSERT
  }


  // start execution

  __ verify_thread();

  // jvmdi/jvmpi support 
  __ notify_method_entry();


  // native call

  // (note that O0 is never an oop--at most it is a handle)
  // It is important not to smash any handles created by this call,
  // until any oop handle in O0 is dereferenced.

  // (note that the space for outgoing params is preallocated)

  // get signature handler
  { Label L;
    __ ld_ptr(Address(Lmethod, 0, in_bytes(methodOopDesc::signature_handler_offset())), G3_scratch);
    __ tst(G3_scratch);
    __ brx(Assembler::notZero, false, Assembler::pt, L);
    __ delayed()->nop();
    __ call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::prepare_native_call), Lmethod);
    __ ld_ptr(Address(Lmethod, 0, in_bytes(methodOopDesc::signature_handler_offset())), G3_scratch);
    __ bind(L);
  }

  // Push a new frame so that the args will really be stored in
  // Copy a few locals across so the new frame has the variables
  // we need but these values will be dead at the jni call and
  // therefore not gc volatile like the values in the current
  // frame (Lmethod in particular)

  // Flush the method pointer to the register save area
  __ st_ptr(Lmethod, SP, (Lmethod->sp_offset_in_saved_window() * wordSize) + STACK_BIAS);
  __ mov(Llocals, O1);
  // calculate where the mirror handle body is allocated in the interpreter frame:

  Address mirror(FP, 0, (frame::interpreter_frame_mirror_offset*wordSize) + STACK_BIAS);
  __ add(mirror, O2);

  // Calculate current frame size
  __ sub(SP, FP, O3);         // Calculate negative of current frame size 
  __ save(SP, O3, SP);        // Allocate an identical sized frame

  // Note I7 has leftover trash. Slow signature handler will fill it in
  // should we get there. Normal jni call will set reasonable last_Java_pc
  // below (and fix I7 so the stack trace doesn't have a meaningless frame
  // in it).

  // Load interpreter frame's Lmethod into same register here

  __ ld_ptr(FP, (Lmethod->sp_offset_in_saved_window() * wordSize) + STACK_BIAS, Lmethod);

  __ mov(I1, Llocals);
  __ mov(I2, Lscratch2);     // save the address of the mirror
  

  // ONLY Lmethod and Llocals are valid here!

  // call signature handler, It will move the arg properly since Llocals in current frame
  // matches that in outer frame

  __ callr(G3_scratch, 0);
  __ delayed()->nop();

  // Result handler is in Lscratch

  // Reload interpreter frame's Lmethod since slow signature handler may block
  __ ld_ptr(FP, (Lmethod->sp_offset_in_saved_window() * wordSize) + STACK_BIAS, Lmethod);

  { Label not_static;

    __ ld(Laccess_flags, O0);
    __ btst(JVM_ACC_STATIC, O0);
    __ br( Assembler::zero, false, Assembler::pt, not_static);
    __ delayed()->
      // get native function entry point(O0 is a good temp until the very end)
       ld_ptr(Address(Lmethod, 0, in_bytes(methodOopDesc::native_function_offset())), O0);
    // for static methods insert the mirror argument
    const int mirror_offset = klassOopDesc::klass_part_offset_in_bytes() + Klass::java_mirror_offset_in_bytes();

    __ ld_ptr(Address(Lmethod, 0, in_bytes(methodOopDesc:: constants_offset())), O1);
    __ ld_ptr(Address(O1, 0, constantPoolOopDesc::pool_holder_offset_in_bytes()), O1);
    __ ld_ptr(O1, mirror_offset, O1);
#ifdef ASSERT
    if (!PrintSignatureHandlers)  // do not dirty the output with this
    { Label L;
      __ tst(O1);
      __ brx(Assembler::notZero, false, Assembler::pt, L);
      __ delayed()->nop();
      __ stop("mirror is missing");
      __ bind(L);
    }
#endif // ASSERT
    __ st_ptr(O1, Lscratch2, 0);
    __ mov(Lscratch2, O1);
    __ bind(not_static);
  }

  // At this point, arguments have been copied off of stack into
  // their JNI positions, which are O1..O5 and SP[68..].
  // Oops are boxed in-place on the stack, with handles copied to arguments.
  // The result handler is in Lscratch.  O0 will shortly hold the JNIEnv*.

#ifdef ASSERT
  { Label L;
    __ tst(O0);
    __ brx(Assembler::notZero, false, Assembler::pt, L);
    __ delayed()->nop();
    __ stop("native entry point is missing");
    __ bind(L);
  }
#endif // ASSERT

  // reset handle block
  __ ld_ptr(G2_thread, in_bytes(JavaThread::active_handles_offset()), G3_scratch);
  __ st_ptr(G0, G3_scratch, JNIHandleBlock::top_offset_in_bytes());

  //
  // setup the frame anchor
  //
  // The scavenge function only needs to know that the PC of this frame is
  // in the interpreter method entry code, it doesn't need to know the exact
  // PC and hence we can use O7 which points to the return address from the
  // previous call in the code stream (signature handler function)
  //
  // The other trick is we set last_Java_sp to FP instead of the usual SP because
  // we have pushed the extra frame in order to protect the volatile register(s)
  // in that frame when we return from the jni call
  //

  __ set_last_Java_frame(FP, O7);
  __ mov(O7, I7);  // make dummy interpreter frame look like one above,
                   // not meaningless information that'll confuse me.

  // flush the windows now. We don't care about the current (protection) frame
  // only the outer frames

  __ flush_windows(); 

  // mark windows as flushed
  Address flags(G2_thread,
		0,
		in_bytes(JavaThread::frame_anchor_offset()) + in_bytes(JavaFrameAnchor::flags_offset()));
  __ set(JavaFrameAnchor::flushed, G3_scratch);
  __ st(G3_scratch, flags);

  // Transition from _thread_in_Java to _thread_in_native. We are already safepoint ready.

  Address thread_state(G2_thread, 0, in_bytes(JavaThread::thread_state_offset()));
#ifdef ASSERT
  { Label L;
    __ ld(thread_state, G3_scratch);
    __ cmp(G3_scratch, _thread_in_Java);
    __ br(Assembler::equal, false, Assembler::pt, L);
    __ delayed()->nop();
    __ stop("Wrong thread state in native stub");
    __ bind(L);
  }
#endif // ASSERT
  __ set(_thread_in_native, G3_scratch);
  __ st(G3_scratch, thread_state);
  
  // Call the jni method, using the delay slot to set the JNIEnv* argument.
  __ save_thread(L7_thread_cache); // save Gthread
  __ callr(O0, 0);
  __ delayed()->
     add(L7_thread_cache, in_bytes(JavaThread::jni_environment_offset()), O0);

  // Back from jni method Lmethod in this frame is DEAD, DEAD, DEAD

  __ restore_thread(L7_thread_cache); // restore G2_thread

  // must we block?
  
  // Block, if necessary, before resuming in _thread_in_Java state.
  // In order for GC to work, don't clear the last_Java_sp until after blocking.
  { Label no_block;
    Address sync_state(G3_scratch, SafepointSynchronize::address_of_state());

    // Switch thread to "native transition" state before reading the synchronization state.
    // This additional state is necessary because reading and testing the synchronization
    // state is not atomic w.r.t. GC, as this scenario demonstrates:
    //     Java thread A, in _thread_in_native state, loads _not_synchronized and is preempted.
    //     VM thread changes sync state to synchronizing and suspends threads for GC.
    //     Thread A is resumed to finish this native method, but doesn't block here since it
    //     didn't see any synchronization is progress, and escapes.
    __ set(_thread_in_native_trans, G3_scratch);
    __ st(G3_scratch, thread_state);
    if( os::is_MP() ) __ membar( Assembler::StoreLoad ); // Force this write out before the read below
    __ load_contents(sync_state, G3_scratch);
    __ cmp(G3_scratch, SafepointSynchronize::_not_synchronized);

    Label L;
    Address suspend_state(G2_thread, 0, in_bytes(JavaThread::suspend_flags_offset()));
    __ br(Assembler::notEqual, false, Assembler::pn, L);
    __ delayed()->
      ld(suspend_state, G3_scratch);
    __ cmp(G3_scratch, 0);
    __ br(Assembler::equal, false, Assembler::pt, no_block);
    __ delayed()->nop();
    __ bind(L);

    // Block.  Save any potential method result value before the operation and
    // use a leaf call to leave the last_Java_frame setup undisturbed.
    save_native_result();
    __ call_VM_leaf(L7_thread_cache,
		    CAST_FROM_FN_PTR(address, JavaThread::check_safepoint_and_suspend_for_native_trans),
		    G2_thread);

    // Restore any method result value
    restore_native_result();
    __ bind(no_block);
  }

  // Clear the frame anchor now

  __ reset_last_Java_frame();

  // Move the result handler address
  __ mov(Lscratch, G3_scratch);         
  // return possible result to the outer frame
#ifndef __LP64
  __ mov(O0, I0);
  __ restore(O1, G0, O1);
#else
  __ restore(O0, G0, O0);
#endif /* __LP64 */

  // Move result handler to expected register
  __ mov(G3_scratch, Lscratch);  

  // Back in normal (native) interpreter frame. State is thread_in_native_trans
  // switch to thread_in_Java. 

  __ set(_thread_in_Java, G3_scratch);
  __ st(G3_scratch, thread_state);

  // JVMDI jframeIDs are invalidated on exit from native method.
  // JVMTI does not use jframeIDs, this whole mechanism must be removed when JVMDI is removed.
  if (JvmtiExport::must_purge_jvmdi_frames_on_native_exit()) { 
    save_native_result();
    __ call_VM_leaf(noreg, CAST_FROM_FN_PTR(address, JvmtiExport::thread_leaving_native_code));
    restore_native_result();
  }

  // handle exceptions (exception handling will handle unlocking!)
  { Label L;
    Address exception_addr (G2_thread, 0, in_bytes(Thread::pending_exception_offset()));

    __ ld_ptr(exception_addr, Gtemp);
    __ tst(Gtemp);
    __ brx(Assembler::equal, false, Assembler::pt, L);
    __ delayed()->nop();
    // Note: This could be handled more efficiently since we know that the native
    //       method doesn't have an exception handler. We could directly return
    //       to the exception handler for the caller.
    __ call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::throw_pending_exception));
    __ should_not_reach_here();
    __ bind(L);
  }

  // jvmdi/jvmpi support (preserves thread register) 
  __ notify_method_exit(true, ilgl);  

  if (synchronized) {
    // save and restore any potential method result value around the unlocking operation
    save_native_result();

    __ add( __ top_most_monitor(), O1);
    __ unlock_object(O1);

    restore_native_result();
  }

  // dispose of return address and remove activation
  if (TraceJumps) {
    // Move target to register that is recordable
    __ mov(Lscratch, G3_scratch);  
    __ JMP(G3_scratch, 0);
  } else {
    __ jmp(Lscratch, 0);
  }
  __ delayed()->nop();


#ifndef CORE
  if (inc_counter) {
    // handle invocation counter overflow
    __ bind(invocation_counter_overflow);
    generate_counter_overflow(true, Lentry);
                              // Lentry is the beginning of this
                              // function and checks again for compiled code
  }

  // compiled code for method exists
  // G5_method: methodOop
  // G3_scratch: nmethod   (NOTE:G3_scratch should not clobbered until the call to nmethod_entry_point)
  __ bind(run_compiled_code);

  // reset the _do_not_unlock_if_synchronized flag - can get here without
  // unlocking above if the invocation counter overflows
  __ st(G0, do_not_unlock_if_synchronized);
  generate_run_compiled_code();

#endif // not CORE

  return entry;
}

// Generic method entry to (asm) interpreter
//------------------------------------------------------------------------------------------------------------------------
// Entry points
//
// Arguments:

//----------------------------------------------------------------------------------------------------
// Entry points & stack frame layout
address InterpreterGenerator::generate_asm_interpreter_entry(bool synchronized) {
  address entry = __ pc();

  bool inc_counter  = UseCompiler || CountCompiledCalls;

  // the following temporary registers are used during frame creation
  const Register Gtmp1 = G3_scratch ;
  const Register Gtmp2 = G1_scratch;

  // make sure registers are different!
  assert_different_registers(G2_thread, G5_method, Gargs, Gtmp1, Gtmp2);

  const Address size_of_parameters(G5_method, 0, in_bytes(methodOopDesc::size_of_parameters_offset()));
  const Address size_of_locals    (G5_method, 0, in_bytes(methodOopDesc::size_of_locals_offset()));
  // Seems like G5_method is live at the point this is used. So we could make this look consistent
  // and use in the asserts.
  const Address access_flags      (Lmethod, 0, in_bytes(methodOopDesc::access_flags_offset()));

  Label Lentry;
  __ bind(Lentry);

  __ verify_oop(G5_method);

  const Register Glocals_size = G3;
  assert_different_registers(Glocals_size, G4_scratch, Gframe_size);

  // make sure method is not native & not abstract
  // rethink these assertions - they can be simplified and shared (gri 2/25/2000)
#ifdef ASSERT
  __ ld(G5_method, in_bytes(methodOopDesc::access_flags_offset()), Gtmp1);
  {
    Label L;
    __ btst(JVM_ACC_NATIVE, Gtmp1);
    __ br(Assembler::zero, false, Assembler::pt, L);
    __ delayed()->nop();
    __ stop("tried to execute native method as non-native");
    __ bind(L);
  }
  { Label L;
    __ btst(JVM_ACC_ABSTRACT, Gtmp1);
    __ br(Assembler::zero, false, Assembler::pt, L);
    __ delayed()->nop();
    __ stop("tried to execute abstract method as non-abstract");
    __ bind(L);
  }
#endif // ASSERT

#ifndef CORE
  Label run_compiled_code;
  // Generate check for compiled code
  if (!CompileTheWorld) {
    generate_check_compiled_code(run_compiled_code);
  }
#endif

  // generate the code to allocate the interpreter stack frame

  generate_fixed_frame(false);

#ifdef FAST_DISPATCH
  __ set((intptr_t)Interpreter::dispatch_table(), IdispatchTables);
                                          // set bytecode dispatch table base
#endif

  //
  // Code to initialize the extra (i.e. non-parm) locals
  //
  Register init_value = noreg;    // will be G0 if we must clear locals
  // The way the code was setup before zerolocals was always true for vanilla java entries.
  // It could only be false for the specialized entries like accessor or empty which have
  // no extra locals so the testing was a waste of time and the extra locals were always
  // initialized. We removed this extra complication to already over complicated code.

  init_value = G0;
  Label clear_loop;

  // NOTE: If you change the frame layout, this code will need to
  // be updated!
  __ lduh( size_of_locals, O2 );
  __ lduh( size_of_parameters, O1 );
  __ sll( O2, LogBytesPerWord, O2);
  __ sll( O1, LogBytesPerWord, O1 );
  __ sub( Llocals, O2, O2 );
  __ sub( Llocals, O1, O1 );
  
  __ bind( clear_loop );
  __ inc( O2, wordSize );

  __ cmp( O2, O1 );
  __ brx( Assembler::lessEqualUnsigned, true, Assembler::pt, clear_loop );
  __ delayed()->st_ptr( init_value, O2, 0 );

  const Address do_not_unlock_if_synchronized(G2_thread, 0,
        in_bytes(JavaThread::do_not_unlock_if_synchronized_offset()));
  // Since at this point in the method invocation the exception handler
  // would try to exit the monitor of synchronized methods which hasn't
  // been entered yet, we set the thread local variable
  // _do_not_unlock_if_synchronized to true. If any exception was thrown by
  // runtime, exception handling i.e. unlock_if_synchronized_method will
  // check this thread local flag.
  __ mov(1, G3_scratch);
  __ st(G3_scratch, do_not_unlock_if_synchronized);

#ifndef CORE
  // increment invocation counter and check for overflow
  //
  // Note: checking for negative value instead of overflow
  //       so we have a 'sticky' overflow test (may be of
  //       importance as soon as we have true MT/MP)
  Label invocation_counter_overflow; 
  Label profile_method;
  Label profile_method_continue;
  if (inc_counter) {
    generate_counter_incr(&invocation_counter_overflow, &profile_method, &profile_method_continue);
    if (ProfileInterpreter) {
      __ bind(profile_method_continue);
    }
  }
#endif // CORE

  bang_stack_shadow_pages(false);

  // reset the _do_not_unlock_if_synchronized flag
  __ st(G0, do_not_unlock_if_synchronized);

  // check for synchronized methods
  // Must happen AFTER invocation_counter check and stack overflow check,
  // so method is not locked if overflows.

  if (synchronized) {
    lock_method();
  } else {
#ifdef ASSERT
    { Label ok;
      __ ld(access_flags, O0);
      __ btst(JVM_ACC_SYNCHRONIZED, O0);
      __ br( Assembler::zero, false, Assembler::pt, ok);
      __ delayed()->nop();
      __ stop("method needs synchronization");
      __ bind(ok);
    }
#endif // ASSERT
  }

  // start execution

  __ verify_thread();

  // jvmdi/jvmpi support 
  __ notify_method_entry();

  // start executing instructions
  __ dispatch_next(vtos);


#ifndef CORE
  if (inc_counter) {
    if (ProfileInterpreter) {
      // We have decided to profile this method in the interpreter
      __ bind(profile_method);

      __ call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::profile_method), Lbcp, true);

#ifdef ASSERT
      __ tst(O0);
      __ breakpoint_trap(Assembler::notEqual);
#endif

      __ set_method_data_pointer();
      
      __ ba(false, profile_method_continue);
      __ delayed()->nop();
    }

    // handle invocation counter overflow
    __ bind(invocation_counter_overflow);
    generate_counter_overflow(false, Lentry);
                              // Lentry is the beginning of this
                              // function and checks again for compiled code
  }

  // compiled code for method exists
  // G5_method: methodOop
  // G3_scratch: nmethod   (NOTE:G3_scratch should not clobbered until the call to nmethod_entry_point)
  __ bind(run_compiled_code);

  // reset the _do_not_unlock_if_synchronized flag - can get here without
  // unlocking above if the invocation counter overflows
  __ st(G0, do_not_unlock_if_synchronized);
  generate_run_compiled_code();

#endif // not CORE

  return entry;
}

//----------------------------------------------------------------------------------------------------
// Entry points & stack frame layout
//
// Here we generate the various kind of entries into the interpreter.
// The two main entry type are generic bytecode methods and native call method.
// These both come in synchronized and non-synchronized versions but the
// frame layout they create is very similar. The other method entry
// types are really just special purpose entries that are really entry
// and interpretation all in one. These are for trivial methods like
// accessor, empty, or special math methods.
//
// When control flow reaches any of the entry types for the interpreter
// the following holds ->
// 
// C2 Calling Conventions:
//
// The entry code below assumes that the following registers are set
// when coming in:
//    G5_method: holds the methodOop of the method to call
//    Lesp:    points to the TOS of the callers expression stack
//             after having pushed all the parameters
//
// The entry code does the following to setup an interpreter frame
//   pop parameters from the callers stack by adjusting Lesp
//   set O0 to Lesp
//   compute X = (max_locals - num_parameters)
//   bump SP up by X to accomadate the extra locals
//   compute X = max_expression_stack
//               + vm_local_words
//               + 16 words of register save area
//   save frame doing a save sp, -X, sp growing towards lower addresses
//   set Lbcp, Lmethod, LcpoolCache
//   set Llocals to i0
//   set Lmonitors to FP - rounded_vm_local_words
//   set Lesp to Lmonitors - 4
//
//  The frame has now been setup to do the rest of the entry code

// Try this optimization:  Most method entries could live in a
// "one size fits all" stack frame without all the dynamic size
// calculations.  It might be profitable to do all this calculation
// statically and approximately for "small enough" methods.

//-----------------------------------------------------------------------------------------------

// C1 Calling conventions
//
// Upon method entry, the following registers are setup:
//
// g2 G2_thread: current thread
// g5 G5_method: method to activate
// g4 Gargs  : pointer to last argument
//
//
// Stack:
//
// +---------------+ <--- sp
// |               |
// : reg save area :
// |               |
// +---------------+ <--- sp + 0x40
// |               |
// : extra 7 slots :      note: these slots are not really needed for the interpreter (fix later)
// |               |
// +---------------+ <--- sp + 0x5c
// |               |
// :     free      :
// |               |
// +---------------+ <--- Gargs
// |               |
// :   arguments   :
// |               |
// +---------------+
// |               |
//
//
//
// AFTER FRAME HAS BEEN SETUP for method interpretation the stack looks like:
//
// +---------------+ <--- sp
// |               |
// : reg save area :
// |               |
// +---------------+ <--- sp + 0x40
// |               |
// : extra 7 slots :      note: these slots are not really needed for the interpreter (fix later)
// |               |
// +---------------+ <--- sp + 0x5c
// |               |
// :               :
// |               | <--- Lesp
// +---------------+ <--- Lmonitors (fp - 0x18)
// |   VM locals   |
// +---------------+ <--- fp
// |               |
// : reg save area :
// |               |
// +---------------+ <--- fp + 0x40
// |               |
// : extra 7 slots :      note: these slots are not really needed for the interpreter (fix later)
// |               |
// +---------------+ <--- fp + 0x5c
// |               |
// :     free      :
// |               |
// +---------------+
// |               |
// : nonarg locals :
// |               |
// +---------------+
// |               |
// :   arguments   :
// |               | <--- Llocals
// +---------------+ <--- Gargs
// |               |

address AbstractInterpreterGenerator::generate_method_entry(AbstractInterpreter::MethodKind kind) {
  // determine code generation flags
  bool synchronized = false;
  address entry_point = NULL;

  switch (kind) {    
    case Interpreter::zerolocals             :                                                                             break;
    case Interpreter::zerolocals_synchronized: synchronized = true;                                                        break;
    case Interpreter::native                 : entry_point = ((InterpreterGenerator*)this)->generate_native_entry(false);  break;
    case Interpreter::native_synchronized    : entry_point = ((InterpreterGenerator*)this)->generate_native_entry(true);   break;
    case Interpreter::empty                  : entry_point = ((InterpreterGenerator*)this)->generate_empty_entry();        break;
    case Interpreter::accessor               : entry_point = ((InterpreterGenerator*)this)->generate_accessor_entry();     break;
    case Interpreter::abstract               : entry_point = ((InterpreterGenerator*)this)->generate_abstract_entry();     break;
    case Interpreter::java_lang_math_sin     : break;
    case Interpreter::java_lang_math_cos     : break;
    case Interpreter::java_lang_math_sqrt    : break;
    default                                  : ShouldNotReachHere();                                                       break;
  }

  if (entry_point) return entry_point;

  return ((InterpreterGenerator*)this)->generate_asm_interpreter_entry(synchronized);
}


static int size_activation_helper(int callee_locals, int max_stack, int monitor_size) {
  const int rounded_vm_local_words = round_to(frame::interpreter_frame_vm_local_words,WordsPerLong);
  const int locals_size = round_to(callee_locals, WordsPerLong);
  return (round_to((max_stack
                   + rounded_vm_local_words
                   + frame::memory_parameter_word_sp_offset), WordsPerLong)
                   + locals_size
                   + monitor_size);
}

// How much stack a method top interpreter activation needs in words.
int AbstractInterpreter::size_top_interpreter_activation(methodOop method) {
  // Save space for one monitor to get into the interpreted method in case
  // the method is synchronized
  int monitor_size    = method->is_synchronized() ?
                                1*frame::interpreter_frame_monitor_size() : 0;
  // See call_stub code
  int call_stub_size  = round_to(7 + frame::memory_parameter_word_sp_offset,
                                 WordsPerLong);    // 7 + register save area
  return size_activation_helper(method->max_locals(), method->max_stack(),
                                monitor_size) + call_stub_size;
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


int AbstractInterpreter::layout_activation_impl(methodOop method,
						int tempcount,
						int moncount,
						int callee_param_size,
						int callee_locals_size,
						frame* caller,
						frame* interpreter_frame,
						bool is_top_frame) {
  // Note: This calculation must exactly parallel the frame setup
  // in InterpreterGenerator::generate_fixed_frame.
  // If f!=NULL, set up the following variables:
  //   - Lmethod
  //   - Llocals
  //   - Lmonitors (to the indicated number of monitors)
  //   - Lesp (to the indicated number of temps)
  // The frame f (if not NULL) on entry is a description of the caller of the frame
  // we are about to layout. We are guaranteed that we will be able to fill in a
  // new interpreter frame as its callee (i.e. the stack space is allocated and
  // the amount was determined by an earlier call to this method with f == NULL).
  // On return f (if not NULL) while describe the interpreter frame we just layed out.


  int raw_frame_size;
  int monitor_size           = moncount * frame::interpreter_frame_monitor_size();
  int rounded_vm_local_words = round_to(frame::interpreter_frame_vm_local_words,WordsPerLong);

  assert(monitor_size == round_to(monitor_size, WordsPerLong), "must align");
  //
  // Note: if you look closely this appears to be doing something much different
  // than generate_fixed_frame. What is happening is this. On sparc we have to do
  // this dance with interpreter_sp_adjustment because the window save area would
  // appear just below the bottom (tos) of the caller's java expression stack. Because
  // the interpreter want to have the locals completely contiguous generate_fixed_frame
  // will adjust the caller's sp so that the "extra locals" (max_locals - parameter_size).
  // Now in generate_fixed_frame the extension of the caller's sp happens in the callee.
  // In this code the opposite occurs the caller adjusts it's own stack base on the callee.
  // This is mostly ok but it does cause a problem when we get to the initial frame (the oldest)
  // because the oldest frame would have adjust its callers frame and yet that frame
  // already exists and isn't part of this array of frames we are unpacking. So at first
  // glance this would seem to mess up that frame. However Deoptimization::fetch_unroll_info_helper()
  // will after it calculates all of the frame's on_stack_size()'s will then figure out the
  // amount to adjust the caller of the initial (oldest) frame and the calculation will all
  // add up. It does seem like it simpler to account for the adjustment here (and remove the 
  // callee... parameters here). However this would mean that this routine would have to take
  // the caller frame as input so we could adjust its sp (and set it's interpreter_sp_adjustment)
  // and run the calling loop in the reverse order. This would also would appear to mean making
  // this code aware of what the interactions are when that initial caller fram was an osr or
  // other adapter frame. deoptimization is complicated enough and  hard enough to debug that
  // there is no sense in messing working code.
  //
  callee_locals_size = round_to((callee_locals_size - callee_param_size), WordsPerLong);
  assert(callee_locals_size == round_to(callee_locals_size, WordsPerLong), "must align");
  raw_frame_size = size_activation_helper(callee_locals_size,
                                          method->max_stack(), monitor_size);
  if (interpreter_frame != NULL) {
    // The skeleton frame must already look like an interpreter frame even if not fully 
    // filled out
    assert(interpreter_frame->is_interpreted_frame(), "Must be interpreted frame");

    intptr_t* fp = interpreter_frame->fp();

    RegisterMap map(JavaThread::current(), false);
    // More verification that skeleton frame is properly walkable
    assert(fp == caller->sp(), "fp must match");

    intptr_t* montop     = fp - rounded_vm_local_words;

    // preallocate monitors (cf. __ add_monitor_to_stack)
    intptr_t* monitors = montop - monitor_size;

    // preallocate stack space
    intptr_t*  esp = monitors - 1 - tempcount;

    NEEDS_CLEANUP;
    intptr_t locals;
    if (caller->is_interpreted_frame()) {
      // Can force the locals area to end up properly overlapping the top of the expression stack.
      intptr_t* Lesp_ptr = caller->interpreter_frame_tos_address() - 1;
      // Note that this computation means we replace size_of_parameters() values from the caller
      // interpreter frame's expression stack with our argument locals
      locals = (intptr_t) (Lesp_ptr + method->size_of_parameters());
    } else if (caller->is_c2i_frame()) {
      // Don't have Lesp available; lay out locals block in the caller
      // adjacent to the register window save area.  C2I use a smaller
      // frame size than normal, so this code is different than the
      // code below.
      locals = (intptr_t)(fp + frame::callee_aggregate_return_pointer_sp_offset + method->max_locals() - 1);
    } else {
#ifdef ASSERT
      bool is_deopted;
      assert(caller->is_osr_adapter_frame() || 
             caller->is_compiled_frame(&is_deopted) || caller->is_entry_frame(), "only possible cases");
#endif /* ASSERT */
      // Don't have Lesp available; lay out locals block in the caller
      // adjacent to the register window save area.
      locals = (intptr_t)(fp + frame::memory_parameter_word_sp_offset + method->max_locals() - 1);
    }
    int delta = method->max_locals() - method->size_of_parameters();
    int computed_sp_adjustment = (delta > 0) ? round_to(delta, WordsPerLong) : 0;
    *interpreter_frame->register_addr(IsavedSP)    = (intptr_t) (fp + computed_sp_adjustment) - STACK_BIAS;
    if (caller->is_entry_frame()) {
      // make sure IsavedSP and the entry frames notion of saved SP
      // agree.  This assertion duplicate a check in entry frame code
      // but catches the failure earlier.
      assert(*caller->register_addr(Lscratch) == *interpreter_frame->register_addr(IsavedSP),
             "would change callers SP");
    }
#ifdef _LP64
    assert(*interpreter_frame->register_addr(IsavedSP) & 1, "must be odd");
#endif

    *interpreter_frame->register_addr(Lmethod)     = (intptr_t) method;
    *interpreter_frame->register_addr(Llocals)     =            locals;
    *interpreter_frame->register_addr(Lmonitors)   = (intptr_t) monitors;
    *interpreter_frame->register_addr(Lesp)        = (intptr_t) esp;
    *interpreter_frame->register_addr(LcpoolCache) = (intptr_t) method->constants()->cache();
#ifdef FAST_DISPATCH
    *interpreter_frame->register_addr(IdispatchTables) = (intptr_t) Interpreter::dispatch_table();
#endif


#ifdef ASSERT
    BasicObjectLock* mp = (BasicObjectLock*)monitors;

    assert(interpreter_frame->interpreter_frame_method() == method, "method matches");
    assert(&interpreter_frame->interpreter_frame_local_at(9) == (intptr_t *)(locals - (9 * wordSize)), "locals match");
    assert(interpreter_frame->interpreter_frame_monitor_end()   == mp, "monitor_end matches");
    assert(((intptr_t *)interpreter_frame->interpreter_frame_monitor_begin()) == ((intptr_t *)mp)+monitor_size, "monitor_begin matches");
    assert(interpreter_frame->interpreter_frame_tos_address()-1 == esp, "esp matches");

    // check bounds
    intptr_t* lo = interpreter_frame->sp() + (frame::memory_parameter_word_sp_offset - 1);
    intptr_t* hi = interpreter_frame->fp() - rounded_vm_local_words;
    assert(lo < monitors && montop <= hi, "monitors in bounds");
    assert(lo <= esp && esp < monitors, "esp in bounds");
#endif // ASSERT
  }

  return raw_frame_size;
}
#endif // not CORE


//----------------------------------------------------------------------------------------------------
// Exceptions
void AbstractInterpreterGenerator::generate_throw_exception() {

  // Entry point in previous activation (i.e., if the caller was interpreted)
  Interpreter::_rethrow_exception_entry = __ pc();
  // O0: exception
  
  // entry point for exceptions thrown within interpreter code
  Interpreter::_throw_exception_entry = __ pc();
  __ verify_thread();
  // expression stack is undefined here
  // O0: exception, i.e. Oexception
  // Lbcp: exception bcx
  __ verify_oop(Oexception);
  
  
  // expression stack must be empty before entering the VM in case of an exception 
  __ empty_expression_stack();
  // find exception handler address and preserve exception oop
  // call C routine to find handler and jump to it
  __ call_VM(O1, CAST_FROM_FN_PTR(address, InterpreterRuntime::exception_handler_for_exception), Oexception);
  __ push_ptr(O1); // push exception for exception handler bytecodes
  
  __ JMP(O0, 0); // jump to exception handler (may be remove activation entry!)
  __ delayed()->nop();


  // if the exception is not handled in the current frame
  // the frame is removed and the exception is rethrown
  // (i.e. exception continuation is _rethrow_exception)
  //
  // Note: At this point the bci is still the bxi for the instruction which caused
  //       the exception and the expression stack is empty. Thus, for any VM calls
  //       at this point, GC will find a legal oop map (with empty expression stack).

  // in current activation
  // tos: exception
  // Lbcp: exception bcp
  
  //
  // JVMTI PopFrame support
  //

  Interpreter::_remove_activation_preserving_args_entry = __ pc();
  Address popframe_condition_addr (G2_thread, 0, in_bytes(JavaThread::popframe_condition_offset()));
  // Set the popframe_processing bit in popframe_condition indicating that we are
  // currently handling popframe, so that call_VMs that may happen later do not trigger new
  // popframe handling cycles.

  __ ld(popframe_condition_addr, G3_scratch);
  __ or3(G3_scratch, JavaThread::popframe_processing_bit, G3_scratch);
  __ stw(G3_scratch, popframe_condition_addr);

  // Empty the expression stack, as in normal exception handling
  __ empty_expression_stack();
  __ unlock_if_synchronized_method(vtos, /* throw_monitor_exception */ false, /* install_monitor_exception */ false);

#ifndef CORE
  {
    // Check to see whether we are returning to a deoptimized frame.
    // (The PopFrame call ensures that the caller of the popped frame is
    // either interpreted or compiled and deoptimizes it if compiled.)
    // In this case, we can't call dispatch_next() after the frame is
    // popped, but instead must save the incoming arguments and restore
    // them after deoptimization has occurred.
    //
    // Note that we don't compare the return PC against the
    // deoptimization blob's unpack entry because of the presence of
    // adapter frames in C2.
    Label caller_not_deoptimized;
    __ call_VM_leaf(L7_thread_cache, CAST_FROM_FN_PTR(address, InterpreterRuntime::interpreter_contains), I7);
    __ tst(O0);
    __ brx(Assembler::notEqual, false, Assembler::pt, caller_not_deoptimized);
    __ delayed()->nop();

    const Register Gtmp1 = G3_scratch;
    const Register Gtmp2 = G1_scratch;

    // Compute size of arguments for saving when returning to deoptimized caller
    __ lduh(Lmethod, in_bytes(methodOopDesc::size_of_parameters_offset()), Gtmp1);
    __ sll(Gtmp1, LogBytesPerWord, Gtmp1);
    __ sub(Llocals, Gtmp1, Gtmp2);
    __ add(Gtmp2, wordSize, Gtmp2);
    // Save these arguments
    __ call_VM_leaf(L7_thread_cache, CAST_FROM_FN_PTR(address, Deoptimization::popframe_preserve_args), G2_thread, Gtmp1, Gtmp2);
    // Inform deoptimization that it is responsible for restoring these arguments
    __ set(JavaThread::popframe_force_deopt_reexecution_bit, Gtmp1);
    Address popframe_condition_addr(G2_thread, 0, in_bytes(JavaThread::popframe_condition_offset()));
    __ st(Gtmp1, popframe_condition_addr);

    // Return from the current method
    // The caller's SP was adjusted upon method entry to accomodate
    // the callee's non-argument locals. Undo that adjustment.
    __ ret();
    __ delayed()->restore(IsavedSP, G0, SP);

    __ bind(caller_not_deoptimized);
  }
#endif /* !CORE */

  // Clear the popframe condition flag
  __ stw(G0 /* popframe_inactive */, popframe_condition_addr);

  // Get out of the current method (how this is done depends on the particular compiler calling
  // convention that the interpreter currently follows)
  // The caller's SP was adjusted upon method entry to accomodate
  // the callee's non-argument locals. Undo that adjustment.
  __ restore(IsavedSP, G0, SP);
#ifndef CORE
  // The method data pointer was incremented already during
  // call profiling. We have to restore the mdp for the current bcp.
  if (ProfileInterpreter) {
    __ set_method_data_pointer_for_bcp();
  }
#endif // !CORE
  // Resume bytecode interpretation at the current bcp
  __ dispatch_next(vtos);
  // end of JVMTI PopFrame support

  Interpreter::_remove_activation_entry = __ pc();

  // preserve exception over this code sequence (remove activation calls the vm, but oopmaps are not correct here)
  __ pop_ptr(Oexception);                                  // get exception

  // Intel has the following comment:
  //// remove the activation (without doing throws on illegalMonitorExceptions)
  // They remove the activation without checking for bad monitor state.
  // %%% We should make sure this is the right semantics before implementing.

  // %%% changed set_vm_result_2 to set_vm_result and get_vm_result_2 to get_vm_result. Is there a bug here?
  __ set_vm_result(Oexception);
  __ unlock_if_synchronized_method(vtos, /* throw_monitor_exception */ false);

  // jvmpi support (preserves thread register)
  // (jvmdi does not generate MethodExit on exception / popFrame)
  __ notify_jvmpi_method_exit(false, vtos);

  __ get_vm_result(Oexception);
  __ verify_oop(Oexception);

    const int return_reg_adjustment = frame::pc_return_offset;
  Address issuing_pc_addr(I7, 0, return_reg_adjustment);

  // We are done with this activation frame; find out where to go next.
  // The continuation point will be an exception handler, which expects
  // the following registers set up:
  //
  // Oexception: exception
  // Oissuing_pc: the local call that threw exception
  // Other On: garbage
  // In/Ln:  the contents of the caller's register window
  //
  // We do the required restore at the last possible moment, because we
  // need to preserve some state across a runtime call.
  // (Remember that the caller activation is unknown--it might not be
  // interpreted, so things like Lscratch are useless in the caller.)

  // Although the Intel version uses call_C, we can use the more
  // compact call_VM.  (The only real difference on SPARC is a
  // harmlessly ignored [re]set_last_Java_frame, compared with
  // the Intel code which lacks this.)
  __ mov(Oexception,      Oexception ->after_save());  // get exception in I0 so it will be on O0 after restore
  __ add(issuing_pc_addr, Oissuing_pc->after_save());  // likewise set I1 to a value local to the caller
  __ super_call_VM_leaf(L7_thread_cache,
                        CAST_FROM_FN_PTR(address, SharedRuntime::exception_handler_for_return_address),
                        Oissuing_pc->after_save());

  // The caller's SP was adjusted upon method entry to accomodate
  // the callee's non-argument locals. Undo that adjustment.
  __ JMP(O0, 0);                         // return exception handler in caller
  __ delayed()->restore(IsavedSP, G0, SP);

  // (same old exception object is already in Oexception; see above)
  // Note that an "issuing PC" is actually the next PC after the call
}


//------------------------------------------------------------------------------------------------------------------------
// Helper for vtos entry point generation

void AbstractInterpreterGenerator::set_vtos_entry_points(Template* t, address& bep, address& cep, address& sep, address& aep, address& iep, address& lep, address& fep, address& dep, address& vep) {
  assert(t->is_valid() && t->tos_in() == vtos, "illegal template");
  Label L;
  aep = __ pc(); __ push_ptr(); __ ba(false, L); __ delayed()->nop(); 
  fep = __ pc(); __ push_f();   __ ba(false, L); __ delayed()->nop();
  dep = __ pc(); __ push_d();   __ ba(false, L); __ delayed()->nop();
  lep = __ pc(); __ push_l();   __ ba(false, L); __ delayed()->nop();
  iep = __ pc(); __ push_i();          
  bep = cep = sep = iep;                        // there aren't any
  vep = __ pc(); __ bind(L);                    // fall through
  generate_and_dispatch(t);
}

// --------------------------------------------------------------------------------


InterpreterGenerator::InterpreterGenerator(StubQueue* code) 
 : AbstractInterpreterGenerator(code) {
   generate_all(); // down here so it can be "virtual"
}

// --------------------------------------------------------------------------------

// when JVM/PI is retired this method can be made '#ifndef PRODUCT'
address AbstractInterpreterGenerator::generate_trace_code(TosState state) {
  address entry = __ pc();

  __ push(state);
  __ mov(O7, Lscratch); // protect return address within interpreter

  // Pass a 0 (not used in sparc) and the top of stack to the bytecode tracer
  __ mov( Otos_l2, G3_scratch );
  __ call_VM(noreg, CAST_FROM_FN_PTR(address, SharedRuntime::trace_bytecode), G0, Otos_l1, G3_scratch);
  __ mov(Lscratch, O7); // restore return address
  __ pop(state);
  __ retl();  
  __ delayed()->nop();

  return entry;
}


// Non-product code
#ifndef PRODUCT
// helpers for generate_and_dispatch

void AbstractInterpreterGenerator::count_bytecode() { 
  Address c(G3_scratch, (address)&BytecodeCounter::_counter_value);
  __ load_contents(c, G4_scratch);
  __ inc(G4_scratch);
  __ st(G4_scratch, c);
}


void AbstractInterpreterGenerator::histogram_bytecode(Template* t) { 
  Address bucket( G3_scratch, (address) &BytecodeHistogram::_counters[t->bytecode()] );
  __ load_contents(bucket, G4_scratch);
  __ inc(G4_scratch);
  __ st(G4_scratch, bucket);
}


void AbstractInterpreterGenerator::histogram_bytecode_pair(Template* t) { 
  address index_addr      = (address)&BytecodePairHistogram::_index;
  Address index(G3_scratch, index_addr);

  address counters_addr   = (address)&BytecodePairHistogram::_counters;
  Address counters(G3_scratch, counters_addr);

  // get index, shift out old bytecode, bring in new bytecode, and store it
  // _index = (_index >> log2_number_of_codes) | 
  //          (bytecode << log2_number_of_codes);


  __ load_contents( index,      G4_scratch );
  __ srl( G4_scratch, BytecodePairHistogram::log2_number_of_codes, G4_scratch );
  __ set( ((int)t->bytecode()) << BytecodePairHistogram::log2_number_of_codes,  G3_scratch );
  __ or3( G3_scratch,  G4_scratch, G4_scratch );
  __ store_contents( G4_scratch, index );

  // bump bucket contents
  // _counters[_index] ++;

  __ load_address( counters );  // loads into G3_scratch
  __ sll( G4_scratch, LogBytesPerWord, G4_scratch );  // Index is word address
  __ add (G3_scratch, G4_scratch, G3_scratch);        // Add in index
  __ ld (G3_scratch, 0, G4_scratch);
  __ inc (G4_scratch);
  __ st (G4_scratch, 0, G3_scratch);
}
#endif // not PRODUCT



// when JVM/PI is retired this method can be made '#ifndef PRODUCT'
void AbstractInterpreterGenerator::trace_bytecode(Template* t) {
  // Call a little run-time stub to avoid blow-up for each bytecode.
  // The run-time runtime saves the right registers, depending on
  // the tosca in-state for the given template.
  address entry = Interpreter::trace_code(t->tos_in());
  guarantee(entry != NULL, "entry must have been generated");
  __ call(entry, relocInfo::none);
  __ delayed()->nop();
}


// Non-product code
#ifndef PRODUCT
void AbstractInterpreterGenerator::stop_interpreter_at() {
  __ load_contents    (Address(G3_scratch , (address)&BytecodeCounter::_counter_value), G3_scratch );
  __ load_ptr_contents(Address(G4_scratch, (address)&StopInterpreterAt              ), G4_scratch);
  __ cmp(G3_scratch, G4_scratch);
  __ breakpoint_trap(Assembler::equal);
}
#endif // not PRODUCT

//Reconciliation History
// 1.67 97/11/12 09:26:11 interpreter_i486.cpp
// 1.75 97/12/04 14:39:58 interpreter_i486.cpp
// 1.78 97/12/19 08:49:42 interpreter_i486.cpp
// 1.80 98/01/12 16:14:11 interpreter_i486.cpp
// 1.87 98/02/03 19:06:48 interpreter_i486.cpp
// 1.92 98/02/20 17:07:36 interpreter_i486.cpp
// 1.94 98/02/28 11:28:31 interpreter_i486.cpp
// 1.99 98/03/10 13:46:51 interpreter_i486.cpp
// 1.100 98/03/17 11:02:27 interpreter_i486.cpp
// 1.102 98/03/26 16:49:49 interpreter_i486.cpp
// 1.108 98/04/08 11:09:44 interpreter_i486.cpp
// 1.111 98/04/16 19:07:07 interpreter_i486.cpp
// 1.114 98/04/22 12:43:43 interpreter_i486.cpp
// 1.110 98/04/30 16:37:59 interpreter_i486.cpp
// 1.134 98/05/06 14:32:09 interpreter_i486.cpp
// 1.136 98/05/07 14:49:24 interpreter_i486.cpp
// 1.141 98/05/11 17:34:50 interpreter_i486.cpp
// 1.147 98/05/21 11:56:36 interpreter_i486.cpp
// 1.151 98/05/28 10:48:58 interpreter_i486.cpp
// 1.156 98/06/08 07:59:36 interpreter_i486.cpp
// 1.159 98/06/15 15:36:17 interpreter_i486.cpp
// 1.163 98/06/17 20:35:29 interpreter_i486.cpp
// 1.167 98/06/23 18:19:20 interpreter_i486.cpp
// 1.171 98/07/06 15:08:14 interpreter_i486.cpp
// 1.173 98/07/24 13:32:36 interpreter_i486.cpp
// 1.175 98/08/06 12:57:14 interpreter_i486.cpp
// 1.177 98/08/26 18:28:03 interpreter_i486.cpp
// 1.181 98/09/25 16:05:19 interpreter_i486.cpp
// 1.184 98/09/30 14:24:49 interpreter_i486.cpp
// 1.185 98/10/05 22:30:59 interpreter_i486.cpp
// 1.187 98/10/16 17:13:06 interpreter_i486.cpp
// 1.188 98/10/21 16:49:20 interpreter_i486.cpp
// 1.189 98/11/23 15:35:13 interpreter_i486.cpp
// 1.193 98/11/27 13:06:30 interpreter_i486.cpp
// 1.194 99/01/12 14:22:48 interpreter_i486.cpp
// 1.195 99/01/20 15:59:42 interpreter_i486.cpp
// 1.198 99/02/02 17:54:19 interpreter_i486.cpp
// 1.199 99/02/03 19:49:52 interpreter_i486.cpp
// 1.200 99/02/17 10:14:51 interpreter_i486.cpp
// 1.201 99/03/04 15:52:33 interpreter_i486.cpp
// 1.203 99/03/22 18:07:10 interpreter_i486.cpp
// 1.204 99/04/01 13:51:50 interpreter_i486.cpp
// 1.206 99/04/19 16:49:29 interpreter_i486.cpp
// 1.207 99/06/16 13:37:11 interpreter_i486.cpp
// 1.211 99/06/24 18:35:37 interpreter_i486.cpp
// 1.214 99/06/29 18:05:21 interpreter_i486.cpp
// 1.217 99/07/23 18:43:55 interpreter_i486.cpp
// 1.218 99/08/17 02:39:51 interpreter_i486.cpp
// 1.219 99/08/18 14:09:58 interpreter_i486.cpp
// 1.221 99/08/27 12:55:40 interpreter_i486.cpp
//End
