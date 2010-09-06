#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)interpreter_i486.cpp	1.319 04/03/22 21:13:53 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_interpreter_i486.cpp.incl"

#define __ _masm->

const int method_offset = frame::interpreter_frame_method_offset * wordSize;
const int bci_offset    = frame::interpreter_frame_bcx_offset    * wordSize;
const int locals_offset = frame::interpreter_frame_locals_offset * wordSize;

//------------------------------------------------------------------------------------------------------------------------

address AbstractInterpreterGenerator::generate_StackOverflowError_handler() {
  address entry = __ pc();

  // Note: There should be a minimal interpreter frame set up when stack
  // overflow occurs since we check explicitly for it now.
  // 
#ifdef ASSERT
  { Label L;
    __ leal(eax, Address(ebp,
                frame::interpreter_frame_monitor_block_top_offset * wordSize));
    __ cmpl(eax, esp);  // eax = maximal esp for current ebp
                        //  (stack grows negative)
    __ jcc(Assembler::aboveEqual, L); // check if frame is complete
    __ stop ("interpreter frame not set up");
    __ bind(L);
  }
#endif // ASSERT
  // Restore bcp under the assumption that the current frame is still 
  // interpreted
  __ restore_bcp();

  // expression stack must be empty before entering the VM if an exception
  // happened
  __ empty_expression_stack();
  __ empty_FPU_stack();
  // throw exception
  __ call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::throw_StackOverflowError));
  return entry;
}

address AbstractInterpreterGenerator::generate_ArrayIndexOutOfBounds_handler(const char* name) {
  address entry = __ pc();
  // expression stack must be empty before entering the VM if an exception happened
  __ empty_expression_stack();
  __ empty_FPU_stack();
  // setup parameters
  // ??? convention: expect aberrant index in register ebx
  __ movl(eax, (int)name);
  __ call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::throw_ArrayIndexOutOfBoundsException), eax, ebx);
  return entry;
}

address AbstractInterpreterGenerator::generate_exception_handler_common(const char* name, const char* message, bool pass_oop) {
  assert(!pass_oop || message == NULL, "either oop or message but not both");
  address entry = __ pc();
  if (pass_oop) {
    // object is at TOS
    __ popl(ebx);
  }
  // expression stack must be empty before entering the VM if an exception happened
  __ empty_expression_stack();
  __ empty_FPU_stack();
  // setup parameters
  __ movl(eax, (int)name);
  if (pass_oop) {
    __ call_VM(eax, CAST_FROM_FN_PTR(address, InterpreterRuntime::create_klass_exception), eax, ebx);
  } else {
    __ movl(ebx, (int)message);
    __ call_VM(eax, CAST_FROM_FN_PTR(address, InterpreterRuntime::create_exception), eax, ebx);
  }
  // throw exception
  __ jmp(Interpreter::throw_exception_entry(), relocInfo::none);
  return entry;
}


address AbstractInterpreterGenerator::generate_continuation_for(TosState state) {
  address entry = __ pc();
  __ dispatch_next(state);
  return entry;
}


address AbstractInterpreterGenerator::generate_return_entry_for(TosState state, int step) {
  address entry = __ pc();
  __ restore_bcp();
  __ restore_locals();
  __ get_cache_and_index_at_bcp(ebx, ecx, 1);
  __ movl(ebx, Address(ebx, ecx, Address::times_4, in_bytes(constantPoolCacheOopDesc::base_offset()) + 3*wordSize));
  __ andl(ebx, 0xFF);
  __ leal(esp, Address(esp, ebx, Address::times_4));
  __ dispatch_next(state, step);
  return entry;
}


#ifndef CORE
address AbstractInterpreterGenerator::generate_deopt_entry_for(TosState state, int step) {
  address entry = __ pc();
  __ restore_bcp();
  __ restore_locals();
  // handle exceptions
  { Label L;
    const Register thread = ecx;
    __ get_thread(thread);
    __ cmpl(Address(thread, Thread::pending_exception_offset()), (int)NULL);
    __ jcc(Assembler::zero, L);    
    __ call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::throw_pending_exception));
    __ should_not_reach_here();
    __ bind(L);
  }
  __ dispatch_next(state, step);
  return entry;
}
#endif


int AbstractInterpreter::BasicType_as_index(BasicType type) {
  int i = 0;
  switch (type) {
    case T_BOOLEAN: i = 0; break;
    case T_CHAR   : i = 1; break;
    case T_BYTE   : i = 2; break;
    case T_SHORT  : i = 3; break;
    case T_INT    : // fall through
    case T_LONG   : // fall through
    case T_VOID   : i = 4; break;
    case T_FLOAT  : // fall through
    case T_DOUBLE : i = 5; break;
    case T_OBJECT : // fall through
    case T_ARRAY  : i = 6; break;
    default       : ShouldNotReachHere();
  }
  assert(0 <= i && i < AbstractInterpreter::number_of_result_handlers, "index out of bounds");
  return i;
}


address AbstractInterpreterGenerator::generate_result_handler_for(BasicType type) {
  address entry = __ pc();
  switch (type) {
    case T_BOOLEAN: __ c2bool(eax);            break;
    case T_CHAR   : __ andl(eax, 0xFFFF);      break;
    case T_BYTE   : __ sign_extend_byte (eax); break;
    case T_SHORT  : __ sign_extend_short(eax); break;
    case T_INT    : /* nothing to do */        break;
    case T_FLOAT  :
      { const Register t = InterpreterRuntime::SignatureHandlerGenerator::temp();
        __ popl(t);                            // remove return address first
        __ pop(dtos);                          // restore ST0        
        __ pushl(t);                           // restore return address
      }
      break;
    case T_OBJECT :
      { Label L;
        __ testl(eax, eax);                    // test if NULL handle
        __ jcc(Assembler::zero, L);            // if not then
        __ movl(eax, Address(eax));            // unbox result
        __ verify_oop(eax);                    // and verify it
        __ bind(L);
      }
      break;
    default       : ShouldNotReachHere();
  }
  __ ret(0);                                   // return from result handler
  return entry;
}


address AbstractInterpreterGenerator::generate_slow_signature_handler() {
  address entry = __ pc();
  // ebx: method
  // ecx: temporary
  // edi: pointer to locals
  // esp: end of copied parameters area
  __ movl(ecx, esp);
  __ call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::slow_signature_handler), ebx, edi, ecx);
  __ ret(0);
  return entry;
}


address AbstractInterpreterGenerator::generate_safept_entry_for(TosState state, address runtime_entry) {
  address entry = __ pc();
  __ push(state);
  __ call_VM(noreg, runtime_entry);
  __ dispatch_via(vtos, Interpreter::_normal_table.table_for(vtos));
  return entry;
}



// Helpers for commoning out cases in the various type of method entries.
//

#ifndef CORE

// increment invocation count & check for overflow
//
// Note: checking for negative value instead of overflow
//       so we have a 'sticky' overflow test
//
// ebx: method
// ecx: invocation counter
//
void InterpreterGenerator::generate_counter_incr(Label* overflow, Label* profile_method, Label* profile_method_continue) {

  const Address invocation_counter(ebx, methodOopDesc::invocation_counter_offset() + InvocationCounter::counter_offset());
  const Address backedge_counter  (ebx, methodOopDesc::backedge_counter_offset() + InvocationCounter::counter_offset());

#ifdef COMPILER2
  if (ProfileInterpreter) { // %%% Merge this into methodDataOop
    __ incl(Address(ebx,methodOopDesc::interpreter_invocation_counter_offset()));
  }
#endif // COMPILER2
  // Update standard invocation counters
  __ movl(eax, backedge_counter);              	// load backedge counter

  __ increment(ecx, InvocationCounter::count_increment);
  __ andl(eax, InvocationCounter::count_mask_value);  // mask out the status bits

  __ movl(invocation_counter, ecx);            	// save invocation count
  __ addl(ecx, eax);            		// add both counters

  // profile_method is non-null only for interpreted method so
  // profile_method != NULL == !native_call

  if (ProfileInterpreter && profile_method != NULL) {
    // Test to see if we should create a method data oop
    __ cmpl(ecx, Address(int(&InvocationCounter::InterpreterProfileLimit), relocInfo::none));
    __ jcc(Assembler::less, *profile_method_continue);

    // if no method data exists, go to profile_method
    __ test_method_data_pointer(eax, *profile_method); 
  }

  __ cmpl(ecx, Address(int(&InvocationCounter::InterpreterInvocationLimit), relocInfo::none));
  __ jcc(Assembler::aboveEqual, *overflow);

}

void InterpreterGenerator::generate_counter_overflow(address entry_point) {

  // Asm interpreter on entry
  // edi - locals
  // esi - bcp
  // ebx - method
  // edx - cpool
  // ebp - interpreter frame

  // On return (i.e. jump to entry_point)
  // ebx - method
  // ecx - rcvr (assuming there is one)
  // top of stack return address of interpreter caller
  // esp - sender_sp

#ifndef CORE
  const Address size_of_parameters(ebx, methodOopDesc::size_of_parameters_offset());

  // InterpreterRuntime::frequency_counter_overflow takes two arguments,
  // the first indicates if the counter overflow occurs at a backwards branch (NULL bcp)
  // and the second is only used when the first is true.  We pass zero for both.
  // The call returns the address of the verified entry point for the method or NULL
  // if the compilation did not complete (either went background or bailed out).
  __ movl(eax, (int)false);
  __ call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::frequency_counter_overflow), eax, eax, true);

  __ movl(ebx, Address(ebp, method_offset));   // restore methodOop

  // method has been compiled - remove activation frame
  // (leave return address on stack) and continue at
  // verified entry point (eax). (eax in some past life maybe, seems to use methodoop these days)
  //
  // Note: continuation at verified entry point works if the method that has been
  //       compiled is the right one (in case of virtual calls); i.e., the inline
  //       cache check must have happened before the invocation counter overflow
  //       check.
  __ load_unsigned_word(ecx, size_of_parameters);     // get size of parameters in words
  __ leave();                                         // remove stack frame
  __ popl(edx);                                       // get return address
  __ negl(ecx);                                       // so we can subtract in next step
  __ leal(esp, Address(edi, ecx, Address::times_4, wordSize));  // adjust esp so that it points to last argument
  __ pushl(edx);                                      // push return address after arguments

  __ movl(ecx, Address(edi));                         // restore potential receiver

  // Preserve invariant that esi/edi contain bcp/locals of sender frame  
  __ jmp(entry_point, relocInfo::none);
#endif CORE

}

void InterpreterGenerator::generate_run_compiled_code(void) {

  // compiled code for method exists
  // ebx: methodOop
  // edx: nmethod
  // ecx: (potential) receiver

  // make sure interpreter entry point exists
  { Label L;
    __ movl(eax, Address(edx, nmethod::interpreter_entry_point_offset()));
    __ testl(eax, eax);

#ifdef COMPILER2
    // The call to nmethod_entry in the vm is especially stressful to stack walking
    // if we are doing SafepointALot then call into the vm even if we already know
    // where we are going.
    if (!SafepointALot) {
      __ jcc(Assembler::notZero, L);
    }
#else /* !COMPILER2 -> COMPILER1 */
    __ jcc(Assembler::notZero, L);
    __ should_not_reach_here();
#endif /* COMPILER1 */
    __ leal(eax, Address(esp, wordSize));             // set last_Java_sp to last argument (return address on tos)

    // Do not use __ call_VM here, since it will call to the InterpreterMacroAssembler, which
    // will save and restore edi/esi relative to the current ebp (which could be compiled)
    __ super_call_VM(ebx, eax, CAST_FROM_FN_PTR(address, InterpreterRuntime::nmethod_entry_point), ebx, edx);    

    // Note: At this point, the nmethod may have been recompiled once more; i.e., if one would
    //       look at the nmethod's interpreter entry point, it could be NULL! However, since
    //       the interpreter entry point is simply the corresponding i2c adapter, which is
    //       dependent of the method's signature only, we don't care. If the nmethod has been
    //       deoptimized, we will end up in a zombie nmethod, which is ok, too.
    __ bind(L);
    __ movl(edx, eax);
    // ebx: methodOop
    // edx: interpreter nmethod entry

#ifdef ASSERT
    { Label L;
      __ testl(edx, edx);
      __ jcc(Assembler::notZero, L);
      __ stop("interpreter entry point == NULL");
      __ bind(L);
    }
#endif
  }
  // ebx: methodOop
  // edx: interpreter nmethod entry
  __ movl(eax, ebx);                                  // compiled code expects methodOop in eax
  __ jmp(edx);                                        // enter compiled code via i2c adapter
}

void InterpreterGenerator::check_for_compiled_code(Label & run_compiled_code) {

  // Generate code to see in compiled version of method exists

  Label skip_compiled_code;
  if (JvmtiExport::can_post_interpreter_events()) {
    // JVMTI events, such as single-stepping, are implemented partly by avoiding running
    // compiled code in threads for which the event is enabled.  Check here for
    // interp_only_mode if these events CAN be enabled.
    __ get_thread(edx);
    __ movl(edx, Address(edx, JavaThread::interp_only_mode_offset()));
    __ testl(edx, edx);
    __ jcc(Assembler::notZero, skip_compiled_code);
  }

  __ movl(edx, Address(ebx, methodOopDesc::compiled_code_offset()));
  __ testl(edx, edx);
  __ jcc(Assembler::notZero, run_compiled_code);

  if (JvmtiExport::can_post_interpreter_events()) {
    __ bind(skip_compiled_code);
  }
}
#endif /* !CORE */

void InterpreterGenerator::generate_stack_overflow_check(void) {
  // see if we've got enough room on the stack for locals plus overhead.
  // the expression stack grows down incrementally, so the normal guard
  // page mechanism will work for that.
  //
  // Registers live on entry:
  //
  // edx: number of additional locals this frame needs (what we must check)
  // ebx: methodOop

  // destroyed on exit
  // eax

  // NOTE:  since the additional locals are also always pushed (wasn't obvious in
  // generate_method_entry) so the guard should work for them too. 
  //

  // monitor entry size: see picture of stack set (generate_method_entry) and frame_i486.hpp
  const int entry_size    = frame::interpreter_frame_monitor_size() * wordSize;

  // total overhead size: entry_size + (saved ebp thru expr stack bottom).
  // be sure to change this if you add/subtract anything to/from the overhead area
  const int overhead_size = -(frame::interpreter_frame_initial_sp_offset*wordSize) + entry_size;

  const int page_size = os::vm_page_size();

  Label after_frame_check;

  // see if the frame is greater than one page in size. If so,
  // then we need to verify there is enough stack space remaining
  // for the additional locals.
  __ cmpl(edx, (page_size - overhead_size) / wordSize);
  __ jcc(Assembler::belowEqual, after_frame_check);

  // compute esp as if this were going to be the last frame on
  // the stack before the red zone

  Label after_frame_check_pop;

  // save esi == caller's bytecode ptr
  __ pushl(esi);

  const Register thread = esi;

  __ get_thread(thread);

  const Address stack_base(thread, Thread::stack_base_offset());
  const Address stack_size(thread, Thread::stack_size_offset());

  // locals + overhead, in bytes
  __ leal(eax, Address(noreg, edx, Address::times_4, overhead_size));

#ifdef ASSERT
  Label stack_base_okay, stack_size_okay;
  // verify that thread stack base is non-zero
  __ cmpl(stack_base, 0);
  __ jcc(Assembler::notEqual, stack_base_okay);
  __ stop("stack base is zero");
  __ bind(stack_base_okay);
  // verify that thread stack size is non-zero
  __ cmpl(stack_size, 0);
  __ jcc(Assembler::notEqual, stack_size_okay);
  __ stop("stack size is zero");
  __ bind(stack_size_okay);
#endif

  // Add stack base to locals and subtract stack size
  __ addl(eax, stack_base);
  __ subl(eax, stack_size);

  // add in the redzone and yellow size
  __ addl(eax, (StackRedPages+StackYellowPages) * page_size);

  // check against the current stack bottom
  __ cmpl(esp, eax);
  __ jcc(Assembler::above, after_frame_check_pop);

  __ popl(esi);  // get saved bcp
  __ popl(eax);  // get return address
  __ jmp(Interpreter::throw_StackOverflowError_entry(), relocInfo::runtime_call_type);

  // all done with frame size check
  __ bind(after_frame_check_pop);
  __ popl(esi);

  __ bind(after_frame_check);
}

// Allocate monitor and lock method (asm interpreter)
// ebx - methodOop
// 
void InterpreterGenerator::lock_method(void) {
  // synchronize method
  const Address access_flags      (ebx, methodOopDesc::access_flags_offset());
  const Address monitor_block_top (ebp, frame::interpreter_frame_monitor_block_top_offset * wordSize);
  const int entry_size            = frame::interpreter_frame_monitor_size() * wordSize;

  #ifdef ASSERT
    { Label L;
      __ movl(eax, access_flags);
      __ testl(eax, JVM_ACC_SYNCHRONIZED);
      __ jcc(Assembler::notZero, L);
      __ stop("method doesn't need synchronization");
      __ bind(L);
    }
  #endif // ASSERT
  // get synchronization object
  { Label done;
    const int mirror_offset = klassOopDesc::klass_part_offset_in_bytes() + Klass::java_mirror_offset_in_bytes();
    __ movl(eax, access_flags);
    __ testl(eax, JVM_ACC_STATIC);
    __ movl(eax, Address(edi));                                         // get receiver (assume this is frequent case)
    __ jcc(Assembler::zero, done);
    __ movl(eax, Address(ebx, methodOopDesc::constants_offset()));
    __ movl(eax, Address(eax, constantPoolOopDesc::pool_holder_offset_in_bytes()));
    __ movl(eax, Address(eax, mirror_offset));
    __ bind(done);
  }
  // add space for monitor & lock
  __ subl(esp, entry_size);                                             // add space for a monitor entry
  __ movl(monitor_block_top, esp);                                      // set new monitor block top
  __ movl(Address(esp, BasicObjectLock::obj_offset_in_bytes()), eax);   // store object
  __ movl(edx, esp);                                                    // object address
  __ lock_object(edx);          
}

//
// Generate a fixed interpreter frame. This is identical setup for interpreted methods
// and for native methods hence the shared code.

void InterpreterGenerator::generate_fixed_frame(bool native_call) {
  // initialize fixed part of activation frame
  __ pushl(eax);                                      // save return address  
  __ enter();                                         // save old & set new ebp
  __ pushl(esi);                                      // set sender sp
  __ movl(esi, Address(ebx,methodOopDesc::const_offset())); // get constMethodOop
  __ leal(esi, Address(esi,constMethodOopDesc::codes_offset())); // get codebase
  __ pushl(ebx);                                      // save methodOop
#ifndef CORE
  if (ProfileInterpreter) {
    Label method_data_continue;
    __ movl(edx, Address(ebx, in_bytes(methodOopDesc::method_data_offset())));
    __ testl(edx, edx);
    __ jcc(Assembler::zero, method_data_continue);
    __ addl(edx, in_bytes(methodDataOopDesc::data_offset()));
    __ bind(method_data_continue);
    __ pushl(edx);                                      // set the mdp (method data pointer)
  } else {
    __ pushl(0);
  }
#endif // !CORE

  __ movl(edx, Address(ebx, methodOopDesc::constants_offset()));
  __ movl(edx, Address(edx, constantPoolOopDesc::cache_offset_in_bytes()));
  __ pushl(edx);                                      // set constant pool cache
  __ pushl(edi);                                      // set locals pointer
  if (native_call) {
    __ pushl(0);                                      // no bcp
  } else {
    __ pushl(esi);                                    // set bcp
    }
  __ pushl(0);                                        // reserve word for pointer to expression stack bottom
  __ movl(Address(esp), esp);                         // set expression stack bottom
}


// End of helpers

//
// Various method entries
//------------------------------------------------------------------------------------------------------------------------
//
//
address InterpreterGenerator::generate_math_entry(AbstractInterpreter::MethodKind kind) {

  // ebx: methodOop
  // ecx: receiver (unused)
  // esi: previous interpreter state (C++ interpreter) must preserve

  if (!InlineIntrinsics) return NULL; // Generate a vanilla entry

  address entry_point = __ pc();

  // mathematical functions inlined by compiler
  // (interpreter must provide identical implementation
  // in order to avoid monotonicity bugs when switching
  // from interpreter to compiler in the middle of some
  // computation)
  //
  // stack: [ ret adr ] <-- esp
  //        [ lo(arg) ]
  //        [ hi(arg) ]
  if (Universe::is_jdk12x_version()) {
    // Note: For JDK 1.2 StrictMath doesn't exist and Math.sin/cos/sqrt are
    //       native methods. Interpreter::method_kind(...) does a check for
    //       native methods first before checking for intrinsic methods and
    //       thus will never select this entry point. Make sure it is not
    //       called accidentally since the SharedRuntime entry points will
    //       not work for JDK 1.2.
    __ should_not_reach_here();
  } else {
    // Note: For JDK 1.3 StrictMath exists and Math.sin/cos/sqrt are
    //       java methods.  Interpreter::method_kind(...) will select
    //       this entry point for the corresponding methods in JDK 1.3.
    __ fld_d(Address(esp, 1*wordSize)); // get argument
    switch (kind) {
      case Interpreter::java_lang_math_sin :
	__ sincos(true, true);
	break;
      case Interpreter::java_lang_math_cos :
	__ sincos(false, true);
	break;
      case Interpreter::java_lang_math_sqrt: 
	__ fsqrt();
	break;
      default                              : 
	ShouldNotReachHere();
    }

    // done, result in FPU ST(0)
    __ ret(0);		// return w/o popping argument
  }
  return entry_point;    
}


// Abstract method entry
// Attempt to execute abstract method. Throw exception
address InterpreterGenerator::generate_abstract_entry(void) {

  // ebx: methodOop
  // ecx: receiver (unused)
  // esi: previous interpreter state (C++ interpreter) must preserve

  address entry_point = __ pc();

  // abstract method entry
  // remove return address. Not really needed, since exception handling throws away expression stack
  __ popl(ebx);             
  // throw exception
  __ call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::throw_AbstractMethodError));
  // the call_VM checks for exception, so we should never return here.
  __ should_not_reach_here();

  return entry_point;
}


// Empty method, generate a very fast return.

address InterpreterGenerator::generate_empty_entry(void) {

  // ebx: methodOop
  // ecx: receiver (unused)
  // esi: previous interpreter state (C++ interpreter) must preserve

  if (!UseFastEmptyMethods) return NULL;

  address entry_point = __ pc();

  // do nothing for empty methods (do not even increment invocation counter)
  // Code: _return
  // _return
  // return w/o popping parameters
  __ ret(0);
  return entry_point;

}

// Call an accessor method (assuming it is resolved, otherwise drop into vanilla (slow path) entry

address InterpreterGenerator::generate_accessor_entry(void) {

  // ebx: methodOop
  // ecx: receiver (preserve for slow entry into asm interpreter)

  address entry_point = __ pc();
  Label xreturn_path;

  // do fastpath for resolved accessor methods
  if (UseFastAccessorMethods) {
    // Code: _aload_0, _(i|a)getfield, _(i|a)return or any rewrites thereof; parameter size = 1
    // Note: We can only use this code if the getfield has been resolved
    //       and if we don't have a null-pointer exception => check for
    //       these conditions first and use slow path if necessary.
    Label slow_path;
    // ebx: method
    // ecx: receiver
    __ movl(eax, Address(esp, wordSize));

    // check if local 0 != NULL and read field
    __ testl(eax, eax);
    __ jcc(Assembler::zero, slow_path);

    __ movl(edi, Address(ebx, methodOopDesc::constants_offset()));
    // read first instruction word and extract bytecode @ 1 and index @ 2
    __ movl(edx, Address(ebx, methodOopDesc::const_offset()));
    __ movl(edx, Address(edx, constMethodOopDesc::codes_offset()));
    // Shift codes right to get the index on the right.
    // The bytecode fetched looks like <index><0xb4><0x2a>
    __ shrl(edx, 2*BitsPerByte);
    __ shll(edx, exact_log2(in_words(ConstantPoolCacheEntry::size())));
    __ movl(edi, Address(edi, constantPoolOopDesc::cache_offset_in_bytes()));

    // eax: local 0
    // ebx: method
    // ecx: receiver - do not destroy since it is needed for slow path!
    // edx: constant pool cache index
    // edi: constant pool cache

    // check if getfield has been resolved and read constant pool cache entry
    // check the validity of the cache entry by testing whether _indices field
    // contains Bytecode::_getfield in b1 byte.
    assert(in_words(ConstantPoolCacheEntry::size()) == 4, "adjust shift below");
    __ movl(esi, 
	    Address(edi, 
		    edx, 
		    Address::times_4, constantPoolCacheOopDesc::base_offset() + ConstantPoolCacheEntry::indices_offset()));
    __ shrl(esi, 2*BitsPerByte);
    __ andl(esi, 0xFF);
    __ cmpl(esi, Bytecodes::_getfield);
    __ jcc(Assembler::notEqual, slow_path);

    // Note: constant pool entry is not valid before bytecode is resolved
    __ movl(esi, 
	    Address(edi, 
		    edx, 
		    Address::times_4, constantPoolCacheOopDesc::base_offset() + ConstantPoolCacheEntry::f2_offset()));
    __ movl(edx, 
	    Address(edi, 
		    edx, 
		    Address::times_4, constantPoolCacheOopDesc::base_offset() + ConstantPoolCacheEntry::flags_offset()));

    Label notByte, notShort, notChar;
    const Address field_address (eax, esi, Address::times_1);

    // Need to differentiate between igetfield, agetfield, bgetfield etc.
    // because they are different sizes.
    // Use the type from the constant pool cache
    __ shrl(edx, ConstantPoolCacheEntry::tosBits);
    // Make sure we don't need to mask edx for tosBits after the above shift
    ConstantPoolCacheEntry::verify_tosBits();
    __ cmpl(edx, btos);
    __ jcc(Assembler::notEqual, notByte);
    __ load_signed_byte(eax, field_address);
    __ jmp(xreturn_path);

    __ bind(notByte);
    __ cmpl(edx, stos);
    __ jcc(Assembler::notEqual, notShort);
    __ load_signed_word(eax, field_address);
    __ jmp(xreturn_path);

    __ bind(notShort);
    __ cmpl(edx, ctos);
    __ jcc(Assembler::notEqual, notChar);
    __ load_unsigned_word(eax, field_address);
    __ jmp(xreturn_path);

    __ bind(notChar);
#ifdef ASSERT
    Label okay;
    __ cmpl(edx, atos);
    __ jcc(Assembler::equal, okay);
    __ cmpl(edx, itos);
    __ jcc(Assembler::equal, okay);
    __ stop("what type is this?");
    __ bind(okay);
#endif // ASSERT
    // All the rest are a 32 bit wordsize
    __ movl(eax, field_address);

    __ bind(xreturn_path);

    // _ireturn/_areturn
    __ ret(0);

    // generate a vanilla interpreter entry as the slow path
    __ bind(slow_path);
    (void) generate_asm_interpreter_entry(false);
  } else {
    (void) generate_asm_interpreter_entry(false);
  }

  return entry_point;
}


//
// Interpreter stub for calling a native method. (asm interpreter)
// This sets up a somewhat different looking stack for calling the native method
// than the typical interpreter frame setup.
//

address InterpreterGenerator::generate_native_entry(bool synchronized) {
  // determine code generation flags
  bool inc_counter  = UseCompiler || CountCompiledCalls;

  // ebx: methodOop
  // ecx: receiver (unused)
  // esi: previous interpreter state (C++ interpreter) must preserve
  address entry_point = __ pc();


#ifndef CORE
  // check if compiled code exists
  Label run_compiled_code;
  if (!PreferInterpreterNativeStubs && !CompileTheWorld) {
    check_for_compiled_code(run_compiled_code);
  }
#endif

  const Address size_of_parameters(ebx, methodOopDesc::size_of_parameters_offset());
#ifndef CORE
  const Address invocation_counter(ebx, methodOopDesc::invocation_counter_offset() + InvocationCounter::counter_offset());
#endif
  const Address access_flags      (ebx, methodOopDesc::access_flags_offset());

  // get parameter size (always needed)
  __ load_unsigned_word(ecx, size_of_parameters);

  // native calls don't need the stack size check since they have no expression stack
  // and the arguments are already on the stack and we only add a handful of words
  // to the stack 

  // ebx: methodOop
  // ecx: size of parameters
  __ popl(eax);                                       // get return address
  // for natives the size of locals is zero

  // compute beginning of parameters (edi)
  __ leal(edi, Address(esp, ecx, Address::times_4, -wordSize));
  __ movl(esi, esp);                                  // remember sender sp


  // add 2 zero-initialized slots for native calls
  __ pushl((int)NULL);                                // slot for native result type info (setup via runtime)
  __ pushl((int)NULL);                                // slot for static native method holder mirror (setup via runtime)

#ifndef CORE
  if (inc_counter) __ movl(ecx, invocation_counter);  // (pre-)fetch invocation count
#endif
  // initialize fixed part of activation frame

  generate_fixed_frame(true);

  // make sure method is native & not abstract
#ifdef ASSERT
  __ movl(eax, access_flags);
  {
    Label L;
    __ testl(eax, JVM_ACC_NATIVE);
    __ jcc(Assembler::notZero, L);
    __ stop("tried to execute non-native method as native");
    __ bind(L);
  }
  { Label L;
    __ testl(eax, JVM_ACC_ABSTRACT);
    __ jcc(Assembler::zero, L);
    __ stop("tried to execute abstract method in interpreter");
    __ bind(L);
  }
#endif

  // Since at this point in the method invocation the exception handler
  // would try to exit the monitor of synchronized methods which hasn't
  // been entered yet, we set the thread local variable
  // _do_not_unlock_if_synchronized to true. The remove_activation will
  // check this flag.

  __ get_thread(eax);
  const Address do_not_unlock_if_synchronized(eax,
        in_bytes(JavaThread::do_not_unlock_if_synchronized_offset()));
  __ movl(do_not_unlock_if_synchronized, (int)true);

#ifndef CORE
  // increment invocation count & check for overflow
  Label invocation_counter_overflow;
  if (inc_counter) {
    generate_counter_incr(&invocation_counter_overflow, NULL, NULL);
  }

#endif // CORE

  bang_stack_shadow_pages(true);

  // reset the _do_not_unlock_if_synchronized flag
  __ get_thread(eax);
  __ movl(do_not_unlock_if_synchronized, (int)false);

  // check for synchronized methods
  // Must happen AFTER invocation_counter check and stack overflow check,
  // so method is not locked if overflows.
  //
  if (synchronized) {
    lock_method();
  } else {
    // no synchronization necessary
#ifdef ASSERT
      { Label L;
        __ movl(eax, access_flags);
        __ testl(eax, JVM_ACC_SYNCHRONIZED);
        __ jcc(Assembler::zero, L);
        __ stop("method needs synchronization");
        __ bind(L);
      }
#endif
  }

  // start execution
#ifdef ASSERT
  { Label L;
    const Address monitor_block_top (ebp,
                 frame::interpreter_frame_monitor_block_top_offset * wordSize);
    __ movl(eax, monitor_block_top);
    __ cmpl(eax, esp);
    __ jcc(Assembler::equal, L);
    __ stop("broken stack frame setup in interpreter");
    __ bind(L);
  }
#endif

  // jvmti/jvmpi support
  __ notify_method_entry();

  // work registers
  const Register method = ebx;
  const Register thread = edi;
  const Register t      = ecx;    

  // allocate space for parameters
  __ get_method(method);
  __ verify_oop(method);
  __ load_unsigned_word(t, Address(method, methodOopDesc::size_of_parameters_offset()));
  __ shll(t, 2);
  __ subl(esp, t);

  // get signature handler
  { Label L;
    __ movl(t, Address(method, methodOopDesc::signature_handler_offset()));
    __ testl(t, t);
    __ jcc(Assembler::notZero, L);
    __ call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::prepare_native_call), method);
    __ get_method(method);
    __ movl(t, Address(method, methodOopDesc::signature_handler_offset()));
    __ bind(L);
  }

  // call signature handler
  assert(InterpreterRuntime::SignatureHandlerGenerator::from() == edi, "adjust this code");
  assert(InterpreterRuntime::SignatureHandlerGenerator::to  () == esp, "adjust this code");
  assert(InterpreterRuntime::SignatureHandlerGenerator::temp() == t  , "adjust this code");
  // The generated handlers do not touch EBX (the method oop).
  // However, large signatures cannot be cached and are generated 
  // each time here.  The slow-path generator will blow EBX
  // sometime, so we must reload it after the call.
  __ call(t, relocInfo::none);
  __ get_method(method);	// slow path call blows EBX on DevStudio 5.0

  // result handler is in eax
  // set result handler
  __ movl(Address(ebp, (frame::interpreter_frame_mirror_offset + 1)*wordSize), eax);

  // pass mirror handle if static call
  { Label L;
    const int mirror_offset = klassOopDesc::klass_part_offset_in_bytes() + Klass::java_mirror_offset_in_bytes();
    __ movl(t, Address(method, methodOopDesc::access_flags_offset()));
    __ testl(t, JVM_ACC_STATIC);
    __ jcc(Assembler::zero, L);
    // get mirror
    __ movl(t, Address(method, methodOopDesc:: constants_offset()));
    __ movl(t, Address(t, constantPoolOopDesc::pool_holder_offset_in_bytes()));
    __ movl(t, Address(t, mirror_offset));
    // copy mirror into activation frame
    __ movl(Address(ebp, frame::interpreter_frame_mirror_offset * wordSize), t);
    // pass handle to mirror
    __ leal(t, Address(ebp, frame::interpreter_frame_mirror_offset * wordSize));
    __ pushl(t);
    __ bind(L);
  }

  // get native function entry point
  { Label L;
    __ movl(eax, Address(method, methodOopDesc::native_function_offset()));
    __ cmpl(eax, (uintptr_t) SharedRuntime::native_method_throw_unsatisfied_link_error_entry());
    __ jcc(Assembler::notEqual, L);
    __ call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::prepare_native_call), method);
    __ get_method(method);
    __ verify_oop(method);
    __ movl(eax, Address(method, methodOopDesc::native_function_offset()));
    __ bind(L);
  }

  // pass JNIEnv
  __ get_thread(thread);
  __ leal(t, Address(thread, JavaThread::jni_environment_offset()));
  __ pushl(t);

  // reset handle block
  __ movl(t, Address(thread, JavaThread::active_handles_offset()));
  __ movl(Address(t, JNIHandleBlock::top_offset_in_bytes()), 0);

  // set_last_Java_frame_before_call
  __ movl(Address(thread, JavaThread::last_Java_fp_offset()), ebp);
#ifdef ASSERT
  { Label L;
    __ cmpl(Address(thread,
		    JavaThread::frame_anchor_offset() + JavaFrameAnchor::flags_offset()),
		    0);
    __ jcc(Assembler::equal, L);
    __ stop("InterpreterGenerator::generate_native_entry: flags not cleared");
    __ bind(L);
  }
#endif /* ASSERT */

  // change thread state
#ifdef ASSERT
  { Label L;
    __ movl(t, Address(thread, JavaThread::thread_state_offset()));
    __ cmpl(t, _thread_in_Java);
    __ jcc(Assembler::equal, L);
    __ stop("Wrong thread state in native stub");
    __ bind(L);
  }
#endif

  // Change state to native (we save the return address in the thread, since it might not
  // be pushed on the stack when we do a a stack traversal). It is enough that the pc()
  // points into the right code segment. It does not have to be the correct return pc.
  __ movl(Address(thread,
		  JavaThread::frame_anchor_offset() + JavaFrameAnchor::last_Java_pc_offset()),
		  (int) __ pc());  
  __ movl(Address(thread, JavaThread::last_Java_sp_offset()), esp);
  __ movl(Address(thread, JavaThread::thread_state_offset()), _thread_in_native);    

  __ call(eax, relocInfo::none);

  // result potentially in edx:eax or ST0
  __ get_method(method);    
  __ get_thread(thread);

#ifdef COMPILER2
    // we are not guaranteed that native code preserves the %mxcsr register
    // so restore it to the value the VM requires
    if (VM_Version::supports_sse() || VM_Version::supports_sse2()) {
      __ ldmxcsr(Address((int) StubRoutines::addr_mxcsr_std(), relocInfo::none));
    }
#endif

  // restore esi to have legal interpreter frame, i.e., bci == 0 <=> esi == code_base()
  __ movl(esi, Address(method,methodOopDesc::const_offset())); // get constMethodOop
  __ leal(esi, Address(esi,constMethodOopDesc::codes_offset()));    // get codebase

  // save potential result in ST(0) & edx:eax
  // (if result handler is the T_FLOAT handler (which is the same as the T_DOUBLE handler), result must be in ST0 -
  // the check is necessary to avoid potential Intel FPU overflow problems by saving/restoring 'empty' FPU registers)
  // It is safe to do this push because state is _thread_in_native and return address will be found
  // via _last_native_pc and not via _last_jave_sp


  { Label L;
    __ cmpl(Address(ebp, (frame::interpreter_frame_mirror_offset + 1)*wordSize), 
           (int)AbstractInterpreter::result_handler(T_FLOAT));
    __ jcc(Assembler::notEqual, L);
    __ push(dtos);      
    __ bind(L);
  }
  __ push(ltos);

  // change thread state
  __ movl(Address(thread, JavaThread::thread_state_offset()), _thread_in_native_trans);
  if( os::is_MP() ) __ membar(); // Force this write out before the read below

  // check for safepoint operation in progress and/or pending suspend requests
  { Label Continue;

    __ cmpl(Address((int)SafepointSynchronize::address_of_state(), relocInfo::none), SafepointSynchronize::_not_synchronized);

    Label L;
    __ jcc(Assembler::notEqual, L);
    __ cmpl(Address(thread, JavaThread::suspend_flags_offset()), 0);
    __ jcc(Assembler::equal, Continue);
    __ bind(L);

    // Don't use call_VM as it will see a possible pending exception and forward it
    // and never return here preventing us from clearing _last_native_pc down below.
    // Also can't use call_VM_leaf either as it will check to see if esi & edi are
    // preserved and correspond to the bcp/locals pointers. So we do a runtime call
    // by hand.
    //
    __ pushl(thread);
    __ call(CAST_FROM_FN_PTR(address, JavaThread::check_safepoint_and_suspend_for_native_trans), relocInfo::runtime_call_type);
    __ increment(esp, wordSize);

    __ get_method(method);
    __ get_thread(thread);

    __ bind(Continue);
  }

  // change thread state
  __ movl(Address(thread, JavaThread::thread_state_offset()), _thread_in_Java);


  // reset_last_Java_frame
  __ movl(Address(thread, JavaThread::last_Java_sp_offset()), (int)NULL);
  __ movl(Address(thread,
		  JavaThread::frame_anchor_offset() + JavaFrameAnchor::last_Java_pc_offset()),
		  (int)NULL);  
  __ movl(Address(thread,
		  JavaThread::frame_anchor_offset() + JavaFrameAnchor::flags_offset()),
		  0);  
  // must clear fp, so that compiled frames are not confused; it is possible
  // that we need it only for debugging
  __ movl(Address(thread, JavaThread::last_Java_fp_offset()), (int)NULL);

  {
     Label no_reguard;
     __ cmpl(Address(thread, JavaThread::stack_guard_state_offset()), JavaThread::stack_guard_yellow_disabled);
     __ jcc(Assembler::notEqual, no_reguard);

     __ pushad();
     __ call(CAST_FROM_FN_PTR(address, SharedRuntime::reguard_yellow_pages), relocInfo::runtime_call_type);
     __ popad();

     __ bind(no_reguard);
   }

  // JVMDI jframeIDs are invalidated on exit from native method.
  // JVMTI does not use jframeIDs, this whole mechanism must be removed when JVMDI is removed.
  if (JvmtiExport::must_purge_jvmdi_frames_on_native_exit()) { 
    // Can not use call_VM_leaf because of esi/edi checking
    __ call(CAST_FROM_FN_PTR(address, JvmtiExport::thread_leaving_native_code), relocInfo::runtime_call_type);
  }

  // handle exceptions (exception handling will handle unlocking!)
  { Label L;
    __ cmpl(Address(thread, Thread::pending_exception_offset()), (int)NULL);
    __ jcc(Assembler::zero, L);
    // Note: At some point we may want to unify this with the code used in call_VM_base();
    //       i.e., we should use the StubRoutines::forward_exception code. For now this
    //       doesn't work here because the esp is not correctly set at this point.
    __ MacroAssembler::call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::throw_pending_exception));
    __ should_not_reach_here();
    __ bind(L);
  }

  // do unlocking if necessary
  { Label L;
    __ movl(t, Address(method, methodOopDesc::access_flags_offset()));
    __ testl(t, JVM_ACC_SYNCHRONIZED);
    __ jcc(Assembler::zero, L);
    // the code below should be shared with interpreter macro assembler implementation
    { Label unlock;
      // BasicObjectLock will be first in list, since this is a synchronized method. However, need
      // to check that the object has not been unlocked by an explicit monitorexit bytecode.        
      const Address monitor(ebp, frame::interpreter_frame_initial_sp_offset * wordSize - (int)sizeof(BasicObjectLock));

      __ leal(edx, monitor);                   // address of first monitor

      __ movl(t, Address(edx, BasicObjectLock::obj_offset_in_bytes()));
      __ testl(t, t);
      __ jcc(Assembler::notZero, unlock);
				
      // Entry already unlocked, need to throw exception
      __ MacroAssembler::call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::throw_illegal_monitor_state_exception));
      __ should_not_reach_here();
  
      __ bind(unlock);        
      __ unlock_object(edx);             
    }
    __ bind(L);
  }    

  // jvmti/jvmpi support
  // Note: This must happen _after_ handling/throwing any exceptions since
  //       the exception handler code notifies the runtime of method exits
  //       too. If this happens before, method entry/exit notifications are
  //       not properly paired (was bug - gri 11/22/99).
  __ notify_method_exit(vtos);

  // restore potential result in edx:eax, call result handler to restore potential result in ST0 & handle result
  __ pop(ltos);
  __ movl(t, Address(ebp, (frame::interpreter_frame_mirror_offset + 1)*wordSize));
  __ call(t, relocInfo::none);

  // remove activation
  __ movl(t, Address(ebp, frame::interpreter_frame_sender_sp_offset * wordSize)); // get sender sp
  __ leave();                                // remove frame anchor
  __ popl(edi);                              // get return address
  __ movl(esp, t);                           // set sp to sender sp
  __ jmp(edi);

#ifndef CORE
  if (inc_counter) {
    // Handle overflow of counter and compile method
    __ bind(invocation_counter_overflow);
    generate_counter_overflow(entry_point);
                              // entry_point is the beginning of this
                              // function and checks again for compiled code
  }

  // Compiled code exists run it
  __ bind(run_compiled_code);

  // reset the _do_not_unlock_if_synchronized flag - can get here without
  // unlocking above if the invocation counter overflows
  __ get_thread(eax);
  __ movl(do_not_unlock_if_synchronized, (int)false);

  generate_run_compiled_code();

#endif

  return entry_point;
}


//
// Generic interpreted method entry to (asm) interpreter
//
address InterpreterGenerator::generate_asm_interpreter_entry(bool synchronized) {
  // determine code generation flags
  bool inc_counter  = UseCompiler || CountCompiledCalls;

  // ebx: methodOop
  // ecx: receiver
  address entry_point = __ pc();

#ifndef CORE
  // check if compiled code exists
  Label run_compiled_code;
  if (!CompileTheWorld) {
    check_for_compiled_code(run_compiled_code);
  }
#endif

  const Address size_of_parameters(ebx, methodOopDesc::size_of_parameters_offset());
  const Address size_of_locals    (ebx, methodOopDesc::size_of_locals_offset());
#ifndef CORE
  const Address invocation_counter(ebx, methodOopDesc::invocation_counter_offset() + InvocationCounter::counter_offset());
#endif
  const Address access_flags      (ebx, methodOopDesc::access_flags_offset());

  // get parameter size (always needed)
  __ load_unsigned_word(ecx, size_of_parameters);

  // ebx: methodOop
  // ecx: size of parameters
  __ load_unsigned_word(edx, size_of_locals);       // get size of locals in words
  __ subl(edx, ecx);                                // edx = no. of additional locals

  // see if we've got enough room on the stack for locals plus overhead.
  generate_stack_overflow_check();

  // get return address
  __ popl(eax);                                       

  // compute beginning of parameters (edi)
  __ leal(edi, Address(esp, ecx, Address::times_4, -wordSize));
  // remember sender sp
  __ movl(esi, esp);                                  

  // edx - # of additional locals
  // allocate space for locals
  // explicitly initialize locals
  {
    Label exit, loop;
    __ testl(edx, edx);
    __ jcc(Assembler::lessEqual, exit);               // do nothing if edx <= 0
    __ bind(loop);
    __ pushl((int)NULL);                              // initialize local variables
    __ decl(edx);                                     // until everything initialized
    __ jcc(Assembler::greater, loop);
    __ bind(exit);
  }

#ifndef CORE
  if (inc_counter) __ movl(ecx, invocation_counter);  // (pre-)fetch invocation count
#endif
  // initialize fixed part of activation frame
  generate_fixed_frame(false);

  // make sure method is not native & not abstract
#ifdef ASSERT
  __ movl(eax, access_flags);
  {
    Label L;
    __ testl(eax, JVM_ACC_NATIVE);
    __ jcc(Assembler::zero, L);
    __ stop("tried to execute native method as non-native");
    __ bind(L);
  }
  { Label L;
    __ testl(eax, JVM_ACC_ABSTRACT);
    __ jcc(Assembler::zero, L);
    __ stop("tried to execute abstract method in interpreter");
    __ bind(L);
  }
#endif

  // Since at this point in the method invocation the exception handler
  // would try to exit the monitor of synchronized methods which hasn't
  // been entered yet, we set the thread local variable
  // _do_not_unlock_if_synchronized to true. The remove_activation will
  // check this flag.

  __ get_thread(eax);
  const Address do_not_unlock_if_synchronized(eax,
        in_bytes(JavaThread::do_not_unlock_if_synchronized_offset()));
  __ movl(do_not_unlock_if_synchronized, (int)true);

#ifndef CORE
  // increment invocation count & check for overflow
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
  __ get_thread(eax);
  __ movl(do_not_unlock_if_synchronized, (int)false);

  // check for synchronized methods
  // Must happen AFTER invocation_counter check and stack overflow check,
  // so method is not locked if overflows.
  //
  if (synchronized) {
    // Allocate monitor and lock method
    lock_method();
  } else {
    // no synchronization necessary
#ifdef ASSERT
      { Label L;
        __ movl(eax, access_flags);
        __ testl(eax, JVM_ACC_SYNCHRONIZED);
        __ jcc(Assembler::zero, L);
        __ stop("method needs synchronization");
        __ bind(L);
      }
#endif
  }

  // start execution
#ifdef ASSERT
  { Label L;
     const Address monitor_block_top (ebp,
                 frame::interpreter_frame_monitor_block_top_offset * wordSize);
    __ movl(eax, monitor_block_top);
    __ cmpl(eax, esp);
    __ jcc(Assembler::equal, L);
    __ stop("broken stack frame setup in interpreter");
    __ bind(L);
  }
#endif

  // jvmti/jvmpi support
  __ notify_method_entry();
 
  __ dispatch_next(vtos);

#ifndef CORE
  // invocation counter overflow
  if (inc_counter) {
    if (ProfileInterpreter) {
      // We have decided to profile this method in the interpreter
      __ bind(profile_method);

      __ call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::profile_method), esi, true);

      __ movl(ebx, Address(ebp, method_offset));   // restore methodOop
      __ movl(eax, Address(ebx, in_bytes(methodOopDesc::method_data_offset())));
      __ movl(Address(ebp, frame::interpreter_frame_mdx_offset * wordSize), eax);
      __ test_method_data_pointer(eax, profile_method_continue);
      __ addl(eax, in_bytes(methodDataOopDesc::data_offset()));
      __ movl(Address(ebp, frame::interpreter_frame_mdx_offset * wordSize), eax);
      __ jmp(profile_method_continue);
    }
    // Handle overflow of counter and compile method
    __ bind(invocation_counter_overflow);
    generate_counter_overflow(entry_point); 
                              // entry_point is the beginning of this
                              // function and checks again for compiled code
  }

  // Compiled code exists run it
  __ bind(run_compiled_code);

  // reset the _do_not_unlock_if_synchronized flag - can get here without
  // unlocking above if the invocation counter overflows
  __ get_thread(eax);
  __ movl(do_not_unlock_if_synchronized, (int)false);
  generate_run_compiled_code();
#endif

  return entry_point;
}

//------------------------------------------------------------------------------------------------------------------------
// Entry points
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
// Arguments:
//
// ebx: methodOop
// ecx: receiver
//
//
// Stack layout immediately at entry
//
// [ return address     ] <--- esp
// [ parameter n        ]
//   ...
// [ parameter 1        ]
// [ expression stack   ] (caller's java expression stack)

// Assuming that we don't go to one of the trivial specialized
// entries the stack will look like below when we are ready to execute
// the first bytecode (or call the native routine). The register usage
// will be as the template based interpreter expects (see interpreter_i486.hpp).
//
// local variables follow incoming parameters immediately; i.e.
// the return address is moved to the end of the locals).
//
// [ monitor entry      ] <--- esp
//   ...
// [ monitor entry      ]
// [ expr. stack bottom ]
// [ saved esi          ]
// [ current edi        ]
// [ methodOop          ]
// [ saved ebp          ] <--- ebp
// [ return address     ]
// [ local variable m   ]
//   ...
// [ local variable 1   ]
// [ parameter n        ]
//   ...
// [ parameter 1        ] <--- edi

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

    case Interpreter::java_lang_math_sin     : // fall thru
    case Interpreter::java_lang_math_cos     : // fall thru
    case Interpreter::java_lang_math_sqrt    : entry_point = ((InterpreterGenerator*)this)->generate_math_entry(kind);     break;

    default                                  : ShouldNotReachHere();                                                       break;
  }

  if (entry_point) return entry_point;

  return ((InterpreterGenerator*)this)->generate_asm_interpreter_entry(synchronized);

}

// How much stack a method activation needs in words.
int AbstractInterpreter::size_top_interpreter_activation(methodOop method) {

  const int entry_size    = frame::interpreter_frame_monitor_size();

  // total overhead size: entry_size + (saved ebp thru expr stack bottom).
  // be sure to change this if you add/subtract anything to/from the overhead area
  const int overhead_size = -(frame::interpreter_frame_initial_sp_offset) + entry_size;

  const int stub_code = 4;  // see generate_call_stub
  return overhead_size + method->max_locals() + method->max_stack() + stub_code;
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
				(frame*) NULL,
				(frame*) NULL,
				is_top_frame);
}


int AbstractInterpreter::layout_activation_impl(methodOop method,
						int tempcount,
						int moncount,
						int callee_param_size,
						int callee_locals,
						frame* caller,
						frame* interpreter_frame,
						bool is_top_frame) {
  // Note: This calculation must exactly parallel the frame setup
  // in AbstractInterpreterGenerator::generate_method_entry.
  // If interpreter_frame!=NULL, set up the method, locals, and monitors.
  // The frame interpreter_frame, if not NULL, is guaranteed to be the right size,
  // as determined by a previous call to this method.
  // It is also guaranteed to be walkable even though it is in a skeletal state

  // fixed size of an interpreter frame:
  int max_locals = method->max_locals();
  int overhead = frame::sender_sp_offset - frame::interpreter_frame_initial_sp_offset;
  // Our locals were accounted for by the caller (or last_frame_adjust on the transistion)
  // Since the callee parameters already account for the callee's params we only need to account for
  // the extra locals.

  int size = overhead + callee_locals - callee_param_size + moncount*frame::interpreter_frame_monitor_size() + tempcount;

  if (interpreter_frame != NULL) {
#ifdef ASSERT
    assert(caller->sp() == interpreter_frame->interpreter_frame_sender_sp(), "Frame not properly walkable");
#endif

    interpreter_frame->interpreter_frame_set_method(method);
    // NOTE the difference in using sender_sp and interpreter_frame_sender_sp
    // interpreter_frame_sender_sp is the original sp of the caller (the unextended_sp)
    // and sender_sp is fp+8
    jint* locals = interpreter_frame->sender_sp() + max_locals - 1;

    interpreter_frame->interpreter_frame_set_locals(locals);
    BasicObjectLock* montop = interpreter_frame->interpreter_frame_monitor_begin();
    interpreter_frame->interpreter_frame_set_monitor_end(montop - moncount);

    // All frames but the initial interpreter frame we fill in have a
    // value for sender_sp that allows walking the stack but isn't
    // truly correct. Correct the value here.
    // 
    int extra_locals = method->max_locals() - method->size_of_parameters();
    if (extra_locals != 0 && 
	interpreter_frame->sender_sp() == interpreter_frame->interpreter_frame_sender_sp() ) {
      interpreter_frame->set_interpreter_frame_sender_sp(caller->sp() + extra_locals);
    }
    *interpreter_frame->interpreter_frame_cache_addr() = 
      method->constants()->cache();
  }
  return size;
}
#endif /* CORE */


//------------------------------------------------------------------------------------------------------------------------
// Exceptions

void AbstractInterpreterGenerator::generate_throw_exception() {
  // Entry point in previous activation (i.e., if the caller was interpreted)
  Interpreter::_rethrow_exception_entry = __ pc();
  // eax: exception
  // edx: return address/pc that threw exception
  __ restore_bcp();                              // esi points to call/send
  __ restore_locals();

  // Entry point for exceptions thrown within interpreter code
  Interpreter::_throw_exception_entry = __ pc();  
  // expression stack is undefined here
  // eax: exception
  // esi: exception bcp
  __ verify_oop(eax);

  // expression stack must be empty before entering the VM in case of an exception
  __ empty_expression_stack();
  __ empty_FPU_stack();
  // find exception handler address and preserve exception oop
  __ call_VM(edx, CAST_FROM_FN_PTR(address, InterpreterRuntime::exception_handler_for_exception), eax);
  // eax: exception handler entry point
  // edx: preserved exception oop
  // esi: bcp for exception handler
  __ pushl(edx);                                 // push exception which is now the only value on the stack
  __ jmp(eax);                                   // jump to exception handler (may be _remove_activation_entry!)

  // If the exception is not handled in the current frame the frame is removed and
  // the exception is rethrown (i.e. exception continuation is _rethrow_exception).
  //
  // Note: At this point the bci is still the bxi for the instruction which caused
  //       the exception and the expression stack is empty. Thus, for any VM calls
  //       at this point, GC will find a legal oop map (with empty expression stack).

  // In current activation
  // tos: exception
  // esi: exception bcp

  //
  // JVMTI PopFrame support
  //

   Interpreter::_remove_activation_preserving_args_entry = __ pc();
  __ empty_expression_stack();
  __ empty_FPU_stack();
  // Set the popframe_processing bit in pending_popframe_condition indicating that we are
  // currently handling popframe, so that call_VMs that may happen later do not trigger new
  // popframe handling cycles.
  __ get_thread(ecx);
  __ movl(edx, Address(ecx, JavaThread::popframe_condition_offset()));
  __ orl(edx, JavaThread::popframe_processing_bit);
  __ movl(Address(ecx, JavaThread::popframe_condition_offset()), edx);

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
    __ movl(edx, Address(ebp, frame::return_addr_offset * wordSize));
    __ super_call_VM_leaf(CAST_FROM_FN_PTR(address, InterpreterRuntime::interpreter_contains), edx);
    __ testl(eax, eax);
    __ jcc(Assembler::notZero, caller_not_deoptimized);

    // Compute size of arguments for saving when returning to deoptimized caller
    __ get_method(eax);
    __ load_unsigned_word(eax, Address(eax, in_bytes(methodOopDesc::size_of_parameters_offset())));
    __ shll(eax, LogBytesPerWord);
    __ restore_locals();
    __ subl(edi, eax);
    __ addl(edi, wordSize);
    // Save these arguments
    __ get_thread(ecx);
    __ super_call_VM_leaf(CAST_FROM_FN_PTR(address, Deoptimization::popframe_preserve_args), ecx, eax, edi);

    __ remove_activation(vtos, edx, 
			 /* throw_monitor_exception */ false, 
			 /* install_monitor_exception */ false,
			 /* notify_jvmdi */ false);

    // Inform deoptimization that it is responsible for restoring these arguments
    __ get_thread(ecx);
    __ movl(Address(ecx, JavaThread::popframe_condition_offset()), JavaThread::popframe_force_deopt_reexecution_bit);

    // Continue in deoptimization handler
    __ jmp(edx);

    __ bind(caller_not_deoptimized);
  }
#endif /* !CORE */

  __ remove_activation(vtos, edx, 
                       /* throw_monitor_exception */ false, 
                       /* install_monitor_exception */ false,
                       /* notify_jvmdi */ false);

  // Clear the popframe condition flag
  __ get_thread(ecx);
  __ movl(Address(ecx, JavaThread::popframe_condition_offset()), JavaThread::popframe_inactive);

  // Finish with popframe handling
  __ restore_bcp();
  __ restore_locals();
#ifndef CORE
  // The method data pointer was incremented already during
  // call profiling. We have to restore the mdp for the current bcp.
  if (ProfileInterpreter) {
    __ set_method_data_pointer_for_bcp();
  }
#endif // !CORE
  __ dispatch_next(vtos);
  // end of PopFrame support

  Interpreter::_remove_activation_entry = __ pc();
  
  // preserve exception over this code sequence
  __ popl(eax);
  __ get_thread(ecx);
  __ movl(Address(ecx, JavaThread::vm_result_offset()), eax);
  // remove the activation (without doing throws on illegalMonitorExceptions)
  __ remove_activation(vtos, edx, false, true, false);
  // restore exception
  __ get_thread(ecx);
  __ movl(eax, Address(ecx, JavaThread::vm_result_offset()));
  __ movl(Address(ecx, JavaThread::vm_result_offset()), (int)NULL);
  __ verify_oop(eax);

  // Inbetween activations - previous activation type unknown yet
  // compute continuation point - the continuation point expects
  // the following registers set up:
  //
  // eax: exception
  // edx: return address/pc that threw exception
  // esp: expression stack of caller
  // ebp: ebp of caller
  __ pushl(eax);                                 // save exception
  __ pushl(edx);                                 // save return address
  __ super_call_VM_leaf(CAST_FROM_FN_PTR(address, SharedRuntime::exception_handler_for_return_address), edx);
  __ movl(ebx, eax);                             // save exception handler
  __ popl(edx);                                  // restore return address
  __ popl(eax);                                  // restore exception
  // Note that an "issuing PC" is actually the next PC after the call
  __ jmp(ebx);                                   // jump to exception handler of caller
}

//------------------------------------------------------------------------------------------------------------------------
// Helper for vtos entry point generation

void AbstractInterpreterGenerator::set_vtos_entry_points (Template* t, address& bep, address& cep, address& sep, address& aep, address& iep, address& lep, address& fep, address& dep, address& vep) {
  assert(t->is_valid() && t->tos_in() == vtos, "illegal template");
  Label L;
  fep = __ pc(); __ push(ftos); __ jmp(L);
  dep = __ pc(); __ push(dtos); __ jmp(L);
  lep = __ pc(); __ pushl(edx); // fall through
  bep = cep = sep = aep =       // fall through 
  iep = __ pc(); __ pushl(eax); // fall through
  vep = __ pc(); __ bind(L);    // fall through
  generate_and_dispatch(t);
}


//------------------------------------------------------------------------------------------------------------------------
// Generation of individual instructions

// helpers for generate_and_dispatch


InterpreterGenerator::InterpreterGenerator(StubQueue* code) 
 : AbstractInterpreterGenerator(code) {
   generate_all(); // down here so it can be "virtual"
}

//------------------------------------------------------------------------------------------------------------------------

// when JVM/PI is retired this method can be made '#ifndef PRODUCT'
address AbstractInterpreterGenerator::generate_trace_code(TosState state) {
  address entry = __ pc();

  // prepare expression stack
  __ popl(ecx);         // pop return address so expression stack is 'pure'
  __ push(state);       // save tosca

  // pass arguments & call tracer
  __ call_VM(noreg, CAST_FROM_FN_PTR(address, SharedRuntime::trace_bytecode), ecx);
  __ movl(ecx, eax);    // make sure return address is not destroyed by pop(state)

  // restore expression stack
  __ pop(state);        // restore tosca

  // return
  __ jmp(ecx);

  return entry;
}


// Non-product code
#ifndef PRODUCT
void AbstractInterpreterGenerator::count_bytecode() { 
  __ incl(Address((int)&BytecodeCounter::_counter_value, relocInfo::none)); 
}


void AbstractInterpreterGenerator::histogram_bytecode(Template* t) { 
  __ incl(Address((int)&BytecodeHistogram::_counters[t->bytecode()], relocInfo::none));
}


void AbstractInterpreterGenerator::histogram_bytecode_pair(Template* t) { 
  __ movl(ebx, Address((int)&BytecodePairHistogram::_index, relocInfo::none));
  __ shrl(ebx, BytecodePairHistogram::log2_number_of_codes);
  __ orl(ebx, ((int)t->bytecode()) << BytecodePairHistogram::log2_number_of_codes);
  __ movl(Address((int)&BytecodePairHistogram::_index, relocInfo::none), ebx);  
  __ incl(Address(noreg, ebx, Address::times_4, (int)BytecodePairHistogram::_counters));
}
#endif // !PRODUCT


// when JVM/PI is retired this method can be made '#ifndef PRODUCT'
void AbstractInterpreterGenerator::trace_bytecode(Template* t) {
  // Call a little run-time stub to avoid blow-up for each bytecode.
  // The run-time runtime saves the right registers, depending on
  // the tosca in-state for the given template.
  address entry = Interpreter::trace_code(t->tos_in());
  assert(entry != NULL, "entry must have been generated");
  __ call(entry, relocInfo::none);
}


// Non-product code
#ifndef PRODUCT
void AbstractInterpreterGenerator::stop_interpreter_at() {
  Label L;
  __ cmpl(Address(int(&BytecodeCounter::_counter_value), relocInfo::none), StopInterpreterAt);
  __ jcc(Assembler::notEqual, L);
  __ int3();
  __ bind(L);
}
#endif // !PRODUCT
