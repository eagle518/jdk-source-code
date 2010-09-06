#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)interpreter_amd64.cpp	1.17 04/03/22 21:14:06 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_interpreter_amd64.cpp.incl"

#define __ _masm->

const int method_offset = frame::interpreter_frame_method_offset * wordSize;
const int bci_offset    = frame::interpreter_frame_bcx_offset    * wordSize;
const int locals_offset = frame::interpreter_frame_locals_offset * wordSize;

//-----------------------------------------------------------------------------

address AbstractInterpreterGenerator::generate_StackOverflowError_handler()
{
  address entry = __ pc();

#ifdef ASSERT
  {
    Label L;
    __ leaq(rax, Address(rbp,
                         frame::interpreter_frame_monitor_block_top_offset *
                         wordSize));
    __ cmpq(rax, rsp); // rax = maximal rsp for current rbp (stack
                       // grows negative)
    __ jcc(Assembler::aboveEqual, L); // check if frame is complete
    __ stop ("interpreter frame not set up");
    __ bind(L);
  }
#endif // ASSERT
  // Restore bcp under the assumption that the current frame is still
  // interpreted
  __ restore_bcp();

  // expression stack must be empty before entering the VM if an
  // exception happened
  __ empty_expression_stack();
  // throw exception
  __ call_VM(noreg,
             CAST_FROM_FN_PTR(address,
                              InterpreterRuntime::throw_StackOverflowError));
  return entry;
}

address AbstractInterpreterGenerator::generate_ArrayIndexOutOfBounds_handler(
        const char* name)
 {
  address entry = __ pc();
  // expression stack must be empty before entering the VM if an
  // exception happened
  __ empty_expression_stack();
  // setup parameters
  // ??? convention: expect aberrant index in register ebx
  __ movq(rarg1, (int64_t) name);
  __ call_VM(noreg,
             CAST_FROM_FN_PTR(address,
                              InterpreterRuntime::
                              throw_ArrayIndexOutOfBoundsException),
             rarg1, rbx);
  return entry;
}

address AbstractInterpreterGenerator::generate_exception_handler_common(
        const char* name, const char* message, bool pass_oop)
{
  assert(!pass_oop || message == NULL, "either oop or message but not both");
  address entry = __ pc();
  if (pass_oop) {
    // object is at TOS
    __ popq(rarg2);
  }
  // expression stack must be empty before entering the VM if an
  // exception happened
  __ empty_expression_stack();
  // setup parameters
  __ movq(rarg1, (int64_t) name);
  if (pass_oop) {
    __ call_VM(rax, CAST_FROM_FN_PTR(address,
                                     InterpreterRuntime::
                                     create_klass_exception),
               rarg1, rarg2);
  } else {
    __ movq(rarg2, (int64_t) message);
    __ call_VM(rax,
               CAST_FROM_FN_PTR(address, InterpreterRuntime::create_exception),
               rarg1, rarg2);
  }
  // throw exception
  __ jmp(Interpreter::throw_exception_entry(), relocInfo::none);
  return entry;
}


address AbstractInterpreterGenerator::generate_continuation_for(TosState state)
{
  address entry = __ pc();
  __ dispatch_next(state);
  return entry;
}


address AbstractInterpreterGenerator::generate_return_entry_for(TosState state,
                                                                int step)
{
  address entry = __ pc();
  __ restore_bcp();
  __ restore_locals();
  __ get_cache_and_index_at_bcp(rbx, rarg3, 1);
  __ movl(rbx, Address(rbx, rarg3,
                       Address::times_8,
                       in_bytes(constantPoolCacheOopDesc::base_offset()) +
                       3 * wordSize));
  __ andl(rbx, 0xFF);
  __ leaq(rsp, Address(rsp, rbx, Address::times_8));
  __ dispatch_next(state, step);
  return entry;
}


#ifndef CORE
address AbstractInterpreterGenerator::generate_deopt_entry_for(TosState state,
                                                               int step)
{
  address entry = __ pc();
  __ restore_bcp();
  __ restore_locals();
  // handle exceptions
  {
    Label L;
    __ cmpq(Address(r15_thread, Thread::pending_exception_offset()), (int) NULL);
    __ jcc(Assembler::zero, L);
    __ call_VM(noreg,
               CAST_FROM_FN_PTR(address,
                                InterpreterRuntime::throw_pending_exception));
    __ should_not_reach_here();
    __ bind(L);
  }
  __ dispatch_next(state, step);
  return entry;
}
#endif


int AbstractInterpreter::BasicType_as_index(BasicType type)
{
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
  assert(0 <= i && i < AbstractInterpreter::number_of_result_handlers,
         "index out of bounds");
  return i;
}


address AbstractInterpreterGenerator::generate_result_handler_for(
        BasicType type)
{
  address entry = __ pc();
  switch (type) {
  case T_BOOLEAN: __ c2bool(rax);            break;
  case T_CHAR   : __ movzwl(rax, rax);       break;
  case T_BYTE   : __ sign_extend_byte(rax);  break;
  case T_SHORT  : __ sign_extend_short(rax); break;
  case T_INT    : /* nothing to do */        break;
  case T_LONG   : /* nothing to do */        break;
  case T_VOID   : /* nothing to do */        break;
  case T_FLOAT  : /* nothing to do */        break;
  case T_DOUBLE : /* nothing to do */        break;
  case T_OBJECT : 
    {
      Label L;
      __ testq(rax, rax);                    // test if NULL handle
      __ jcc(Assembler::zero, L);            // if not then
      __ movq(rax, Address(rax));            // unbox result
      __ verify_oop(rax);                    // and verify it
      __ bind(L);
    }
    break;
  default       : ShouldNotReachHere();
  }
  __ ret(0);                                   // return from result handler
  return entry;
}

#ifdef _WIN64
address AbstractInterpreterGenerator::generate_slow_signature_handler()
{
  address entry = __ pc();

  // rbx: method
  // r14: pointer to locals
  // rarg3: first stack arg - wordSize
  __ movq(rarg3, rsp);
  // adjust rsp
  __ subq(rsp, 4 * wordSize);
  __ call_VM(noreg,
             CAST_FROM_FN_PTR(address,
                              InterpreterRuntime::slow_signature_handler),
             rbx, r14, rarg3);

  // rax: result handler

  // Stack layout:
  // rsp: 3 integer or float args (if static first is unused)
  //      1 float/double identifiers
  //        return address
  //        stack args
  //        garbage
  //        expression stack bottom
  //        bcp (NULL)
  //        ...

  // Do FP first so we can use rarg3 as temp
  __ movl(rarg3, Address(rsp, 3 * wordSize)); // float/double identifiers

  for ( int i= 0; i < Argument::n_int_register_parameters-1; i++ ) {
    FloatRegister floatreg = as_FloatRegister(i+1);
    Label isfloatordouble, isdouble, next;

    __ testl(rarg3, 1 << (i*2));      // Float or Double?
    __ jcc(Assembler::notZero, isfloatordouble);

    // Do Int register here
    switch ( i ) {
      case 0:
        __ movl(rscratch1, Address(rbx, methodOopDesc::access_flags_offset()));
        __ testl(rscratch1, JVM_ACC_STATIC);
        __ cmovq(Assembler::zero, rarg1, Address(rsp));
        break;
      case 1:
        __ movq(rarg2, Address(rsp, wordSize));
        break;
      case 2:
        __ movq(rarg3, Address(rsp, 2 * wordSize));
        break;
      default:
        break;
    }

    __ jmp (next);

    __ bind(isfloatordouble);
    __ testl(rarg3, 1 << ((i*2)+1));     // Double?
    __ jcc(Assembler::notZero, isdouble);

// Do Float Here
    __ movss(floatreg, Address(rsp, i * wordSize));
    __ jmp(next);

// Do Double here
    __ bind(isdouble);
    __ movlpd(floatreg, Address(rsp, i * wordSize));

    __ bind(next);
  }


  // restore rsp
  __ addq(rsp, 4 * wordSize);

  __ ret(0);

  return entry;
}
#else
address AbstractInterpreterGenerator::generate_slow_signature_handler()
{
  address entry = __ pc();

  // rbx: method
  // r14: pointer to locals
  // rarg3: first stack arg - wordSize
  __ movq(rarg3, rsp);
  // adjust rsp
  __ subq(rsp, 14 * wordSize);
  __ call_VM(noreg,
             CAST_FROM_FN_PTR(address,
                              InterpreterRuntime::slow_signature_handler),
             rbx, r14, rarg3);

  // rax: result handler

  // Stack layout:
  // rsp: 5 integer args (if static first is unused)
  //      1 float/double identifiers
  //      8 double args
  //        return address
  //        stack args
  //        garbage
  //        expression stack bottom
  //        bcp (NULL)
  //        ...

  // Do FP first so we can use rarg3 as temp
  __ movl(rarg3, Address(rsp, 5 * wordSize)); // float/double identifiers

  for (int i = 0; i < Argument::n_float_register_parameters; i++) {
    const FloatRegister r = as_FloatRegister(i);

    Label d, done;
    
    __ testl(rarg3, 1 << i);
    __ jcc(Assembler::notZero, d);
    __ movss(r, Address(rsp, (6 + i) * wordSize));
    __ jmp(done);
    __ bind(d);
    __ movlpd(r, Address(rsp, (6 + i) * wordSize));
    __ bind(done);
  }

  // Now handle integrals.  Only do rarg1 if not static.
  __ movl(rarg3, Address(rbx, methodOopDesc::access_flags_offset()));
  __ testl(rarg3, JVM_ACC_STATIC);
  __ cmovq(Assembler::zero, rarg1, Address(rsp));

  __ movq(rarg2, Address(rsp, wordSize));
  __ movq(rarg3, Address(rsp, 2 * wordSize));
  __ movq(rarg4, Address(rsp, 3 * wordSize));
  __ movq(rarg5, Address(rsp, 4 * wordSize));

  // restore rsp
  __ addq(rsp, 14 * wordSize);

  __ ret(0);

  return entry;
}
#endif

address AbstractInterpreterGenerator::generate_safept_entry_for(
        TosState state,
        address runtime_entry)
{
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
// rbx: method
// ecx: invocation counter
//
void InterpreterGenerator::generate_counter_incr(
        Label* overflow,
        Label* profile_method,
        Label* profile_method_continue)
{

  const Address invocation_counter(rbx,
                                   methodOopDesc::invocation_counter_offset() +
                                   InvocationCounter::counter_offset());
  const Address backedge_counter(rbx,
                                 methodOopDesc::backedge_counter_offset() +
                                 InvocationCounter::counter_offset());

#ifdef COMPILER2
  if (ProfileInterpreter) { // %%% Merge this into methodDataOop
    __ incl(Address(rbx,
                    methodOopDesc::interpreter_invocation_counter_offset()));
  }
#endif // COMPILER2
  // Update standard invocation counters
  __ movl(rax, backedge_counter); // load backedge counter

  __ incrementl(rarg3, InvocationCounter::count_increment);
  __ andl(rax, InvocationCounter::count_mask_value); // mask out the
                                                     // status bits

  __ movl(invocation_counter, rarg3); // save invocation count
  __ addl(rarg3, rax); // add both counters

  // profile_method is non-null only for interpreted method so
  // profile_method != NULL == !native_call

  if (ProfileInterpreter && profile_method != NULL) {
    // Test to see if we should create a method data oop
    __ cmpl(rarg3, Address((address) &InvocationCounter::InterpreterProfileLimit,
                         relocInfo::none));
    __ jcc(Assembler::less, *profile_method_continue);

    // if no method data exists, go to profile_method
    __ test_method_data_pointer(rax, *profile_method); 
  }

  __ cmpl(rarg3, Address((address)
                       &InvocationCounter::InterpreterInvocationLimit,
                       relocInfo::none));
  __ jcc(Assembler::aboveEqual, *overflow);
}

void InterpreterGenerator::generate_counter_overflow(address entry_point)
{

  // Asm interpreter on entry
  // r14 - locals
  // r13 - bcp
  // rbx - method
  // edx - cpool
  // rbp - interpreter frame

  // On return (i.e. jump to entry_point)
  // rbx - method
  // rarg3 - rcvr (assuming there is one)
  // top of stack return address of interpreter caller
  // rsp - sender_sp

#ifndef CORE
  const Address size_of_parameters(rbx,
                                   methodOopDesc::size_of_parameters_offset());

  // InterpreterRuntime::frequency_counter_overflow takes two
  // arguments, the first (thread) is passed by call_VM, the second
  // indicates if the counter overflow occurs at a backwards branch
  // (NULL bcp).  We pass zero for it.  The call returns the address
  // of the verified entry point for the method or NULL if the
  // compilation did not complete (either went background or bailed
  // out).
  __ movl(rarg1, 0);
  // IcoResult frequency_counter_overflow([JavaThread*], address branch_bcp)
  __ call_VM_Ico(noreg,
                 CAST_FROM_FN_PTR(address,
                                  InterpreterRuntime::frequency_counter_overflow),
                 rarg1, true);

  __ movq(rbx, Address(rbp, method_offset));   // restore methodOop

  // method has been compiled - remove activation frame (leave return
  // address on stack) and continue at verified entry point
  // (rax). (rax in some past life maybe, seems to use methodoop these
  // days)
  //
  // Note: continuation at verified entry point works if the method
  // that has been compiled is the right one (in case of virtual
  // calls); i.e., the inline cache check must have happened before
  // the invocation counter overflow check.

  // get size of parameters in words
  __ load_unsigned_word(rcx, size_of_parameters);
  // remove stack frame
  __ leave();
  // get return address
  __ popq(rdx);
  // so we can subtract in next step
  __ negq(rcx);
  // adjust rsp so that it points to last argument
  __ leaq(rsp, Address(r14, rcx, Address::times_8, wordSize));
  // push return address after arguments
  __ pushq(rdx);
  // restore potential receiver
  __ movq(rcx, Address(r14));

  // Preserve invariant that r13/r14 contain bcp/locals of sender frame
  __ jmp(entry_point, relocInfo::none);
#endif CORE
}

void InterpreterGenerator::generate_run_compiled_code(void)
{

  // compiled code for method exists
  // rbx: methodOop
  // rarg2: nmethod
  // rarg3: (potential) receiver

  // make sure interpreter entry point exists
  {
    Label L;
    __ movq(rax, Address(rarg2, nmethod::interpreter_entry_point_offset()));
    __ testq(rax, rax);
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

    __ leaq(rax, Address(rsp, wordSize)); // set last_Java_sp to last
                                          // argument (return address
                                          // on tos)

    // Do not use __ call_VM here, since it will call to the
    // InterpreterMacroAssembler, which will save and restore r13/r14
    // relative to the current ebp (which could be compiled)
    __ super_call_VM(rbx, rax,
                     CAST_FROM_FN_PTR(address,
                                      InterpreterRuntime::nmethod_entry_point),
                     rbx, rarg2);

    // Note: At this point, the nmethod may have been recompiled once
    // more; i.e., if one would look at the nmethod's interpreter
    // entry point, it could be NULL! However, since the interpreter
    // entry point is simply the corresponding i2c adapter, which is
    // dependent of the method's signature only, we don't care. If the
    // nmethod has been deoptimized, we will end up in a zombie
    // nmethod, which is ok, too.
    __ bind(L);
    __ movq(rarg2, rax);
    // rbx: methodOop
    // rarg2: interpreter nmethod entry

#ifdef ASSERT
    {
      Label L;
      __ testq(rarg2, rarg2);
      __ jcc(Assembler::notZero, L);
      __ stop("interpreter entry point == NULL");
      __ bind(L);
    }
#endif
  }
  // rbx: methodOop
  // rarg2: interpreter nmethod entry
  __ movq(rax, rbx); // compiled code expects methodOop in rax
  __ jmp(rarg2); // enter compiled code via i2c adapter
}

void InterpreterGenerator::check_for_compiled_code(Label & run_compiled_code)
{
  // Generate code to see in compiled version of method exists

  Label skip_compiled_code;
  if (JvmtiExport::can_post_interpreter_events()) {
    // JVMTI events, such as single-stepping, are implemented partly
    // by avoiding running compiled code in threads for which the
    // event is enabled.  Check here for interp_only_mode if these
    // events CAN be enabled.
    __ movl(rarg2, Address(r15_thread, JavaThread::interp_only_mode_offset()));
    __ testl(rarg2, rarg2);
    __ jcc(Assembler::notZero, skip_compiled_code);
  }

  __ movq(rarg2, Address(rbx, methodOopDesc::compiled_code_offset()));
  __ testq(rarg2, rarg2);
  __ jcc(Assembler::notZero, run_compiled_code);

  if (JvmtiExport::can_post_interpreter_events()) {
    __ bind(skip_compiled_code);
  }
}
#endif /* !CORE */

// See if we've got enough room on the stack for locals plus overhead.
// The expression stack grows down incrementally, so the normal guard
// page mechanism will work for that.
//
// NOTE: Since the additional locals are also always pushed (wasn't
// obvious in generate_method_entry) so the guard should work for them
// too.
//
// Args:
//      rarg2: number of additional locals this frame needs (what we must check)
//      rbx: methodOop
//
// Kills:
//      rax
void InterpreterGenerator::generate_stack_overflow_check(void)
{

  // monitor entry size: see picture of stack set
  // (generate_method_entry) and frame_amd64.hpp
  const int entry_size = frame::interpreter_frame_monitor_size() * wordSize;

  // total overhead size: entry_size + (saved rbp through expr stack
  // bottom).  be sure to change this if you add/subtract anything
  // to/from the overhead area
  const int overhead_size =
    -(frame::interpreter_frame_initial_sp_offset * wordSize) + entry_size;

  const int page_size = os::vm_page_size();

  Label after_frame_check;

  // see if the frame is greater than one page in size. If so,
  // then we need to verify there is enough stack space remaining
  // for the additional locals.
  __ cmpl(rarg2, (page_size - overhead_size) / wordSize);
  __ jcc(Assembler::belowEqual, after_frame_check);

  // compute rsp as if this were going to be the last frame on
  // the stack before the red zone

  const Address stack_base(r15_thread, Thread::stack_base_offset());
  const Address stack_size(r15_thread, Thread::stack_size_offset());

  // locals + overhead, in bytes
  __ leaq(rax, Address(noreg, rarg2, Address::times_8, overhead_size));

#ifdef ASSERT
  Label stack_base_okay, stack_size_okay;
  // verify that thread stack base is non-zero
  __ cmpq(stack_base, 0);
  __ jcc(Assembler::notEqual, stack_base_okay);
  __ stop("stack base is zero");
  __ bind(stack_base_okay);
  // verify that thread stack size is non-zero
  __ cmpq(stack_size, 0);
  __ jcc(Assembler::notEqual, stack_size_okay);
  __ stop("stack size is zero");
  __ bind(stack_size_okay);
#endif

  // Add stack base to locals and subtract stack size
  __ addq(rax, stack_base);
  __ subq(rax, stack_size);

  // add in the red and yellow zone sizes
  __ addq(rax, (StackRedPages + StackYellowPages) * page_size);

  // check against the current stack bottom
  __ cmpq(rsp, rax);
  __ jcc(Assembler::above, after_frame_check);

  __ popq(rax); // get return address
  __ jmp(Interpreter::throw_StackOverflowError_entry(),
         relocInfo::runtime_call_type);

  // all done with frame size check
  __ bind(after_frame_check);
}

// Allocate monitor and lock method (asm interpreter)
//
// Args:
//      rbx: methodOop
//      r14: locals
// 
// Kills:
//      rax
//      rarg0, rarg1, rarg2, rarg3, rarg4, rarg5 (param regs)
//      rscratch1, rscratch2 (scratch regs)
void InterpreterGenerator::lock_method(void)
{
  // synchronize method
  const Address access_flags(rbx, methodOopDesc::access_flags_offset());
  const Address monitor_block_top(
        rbp,
        frame::interpreter_frame_monitor_block_top_offset * wordSize);
  const int entry_size = frame::interpreter_frame_monitor_size() * wordSize;

#ifdef ASSERT
  {
    Label L;
    __ movl(rax, access_flags);
    __ testl(rax, JVM_ACC_SYNCHRONIZED);
    __ jcc(Assembler::notZero, L);
    __ stop("method doesn't need synchronization");
    __ bind(L);
  }
#endif // ASSERT

  // get synchronization object
  {
    const int mirror_offset = klassOopDesc::klass_part_offset_in_bytes() +
                              Klass::java_mirror_offset_in_bytes();
    Label done;
    __ movl(rax, access_flags);
    __ testl(rax, JVM_ACC_STATIC);
    __ movq(rax, Address(r14)); // get receiver (assume this is frequent case)
    __ jcc(Assembler::zero, done);
    __ movq(rax, Address(rbx, methodOopDesc::constants_offset()));
    __ movq(rax, Address(rax,
                         constantPoolOopDesc::pool_holder_offset_in_bytes()));
    __ movq(rax, Address(rax, mirror_offset));

#ifdef ASSERT
    {
      Label L;
      __ testq(rax, rax);
      __ jcc(Assembler::notZero, L);
      __ stop("synchronization object is NULL");
      __ bind(L);
    }
#endif // ASSERT

    __ bind(done);
  }

  // add space for monitor & lock
  __ subq(rsp, entry_size); // add space for a monitor entry
  __ movq(monitor_block_top, rsp);  // set new monitor block top
  // store object
  __ movq(Address(rsp, BasicObjectLock::obj_offset_in_bytes()), rax);
  __ movq(rarg1, rsp); // object address
  __ lock_object(rarg1);
}

// Generate a fixed interpreter frame. This is identical setup for
// interpreted methods and for native methods hence the shared code.
//
// Args:
//      rax: return address
//      rbx: methodOop
//      r14: pointer to locals
//      r13: sender sp
//      rarg2: cp cache
void InterpreterGenerator::generate_fixed_frame(bool native_call)
{
  // initialize fixed part of activation frame
  __ pushq(rax);       // save return address
  __ enter();          // save old & set new rbp
  __ pushq(r13);       // set sender sp
  __ movq(r13, Address(rbx, methodOopDesc::const_offset()));      // get constMethodOop
  __ leaq(r13, Address(r13, constMethodOopDesc::codes_offset())); // get codebase
  __ pushq(rbx);       // save methodOop
#ifndef CORE
  if (ProfileInterpreter) {
    Label method_data_continue;
    __ movq(rarg2, Address(rbx, in_bytes(methodOopDesc::method_data_offset())));
    __ testq(rarg2, rarg2);
    __ jcc(Assembler::zero, method_data_continue);
    __ addq(rarg2, in_bytes(methodDataOopDesc::data_offset()));
    __ bind(method_data_continue);
    __ pushq(rarg2);     // set the mdp (method data pointer)
  } else {
    __ pushq(0);
  }
#endif // !CORE

  __ movq(rarg2, Address(rbx, methodOopDesc::constants_offset()));
  __ movq(rarg2, Address(rarg2, constantPoolOopDesc::cache_offset_in_bytes()));
  __ pushq(rarg2); // set constant pool cache
  __ pushq(r14); // set locals pointer
  if (native_call) {
    __ pushq(0); // no bcp
  } else {
    __ pushq(r13); // set bcp
  }
  __ pushq(0); // reserve word for pointer to expression stack bottom
  __ movq(Address(rsp), rsp); // set expression stack bottom
}

// End of helpers

//
// Various method entries
//

address InterpreterGenerator::generate_math_entry(
  AbstractInterpreter::MethodKind kind)
{
  // rbx: methodOop
  // rarg3: receiver (unused)
  // r13: previous interpreter state (C++ interpreter) must preserve

  if (!InlineIntrinsics) return NULL; // Generate a vanilla entry

  assert(kind == Interpreter::java_lang_math_sqrt,
         "Other intrinsics are not special");

  address entry_point = __ pc();

  // mathematical functions inlined by compiler
  // (interpreter must provide identical implementation
  // in order to avoid monotonicity bugs when switching
  // from interpreter to compiler in the middle of some
  // computation)
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
    __ sqrtsd(xmm0, Address(rsp, wordSize));

    __ ret(0);		// return w/o popping argument
  }

  return entry_point;    
}


// Abstract method entry
// Attempt to execute abstract method. Throw exception
address InterpreterGenerator::generate_abstract_entry(void)
{
  // rbx: methodOop
  // rarg3: receiver (unused)
  // r13: previous interpreter state (C++ interpreter) must preserve

  address entry_point = __ pc();

  // abstract method entry
  // remove return address. Not really needed, since exception
  // handling throws away expression stack
  __ popq(rbx);
  // throw exception
  __ call_VM(noreg, CAST_FROM_FN_PTR(address,
                             InterpreterRuntime::throw_AbstractMethodError));
  // the call_VM checks for exception, so we should never return here.
  __ should_not_reach_here();

  return entry_point;
}


// Empty method, generate a very fast return.

address InterpreterGenerator::generate_empty_entry(void)
{
  // rbx: methodOop
  // rarg3: receiver (unused)
  // r13: previous interpreter state (C++ interpreter) must preserve

  if (!UseFastEmptyMethods) {
    return NULL;
  }

  address entry_point = __ pc();

  // do nothing for empty methods (do not even increment invocation counter)
  // Code: _return
  // _return
  // return w/o popping parameters
  __ ret(0);
  return entry_point;

}

// Call an accessor method (assuming it is resolved, otherwise drop
// into vanilla (slow path) entry
address InterpreterGenerator::generate_accessor_entry(void)
{
  // rbx: methodOop
  // rarg3: receiver (preserve for slow entry into asm interpreter)

  address entry_point = __ pc();
  Label xreturn_path;

  // do fastpath for resolved accessor methods
  if (UseFastAccessorMethods) {
    // Code: _aload_0, _(i|a)getfield, _(i|a)return or any rewrites
    //       thereof; parameter size = 1
    // Note: We can only use this code if the getfield has been resolved
    //       and if we don't have a null-pointer exception => check for
    //       these conditions first and use slow path if necessary.
    Label slow_path;
    // rbx: method
    // rarg3: receiver
    __ movq(rax, Address(rsp, wordSize));

    // check if local 0 != NULL and read field
    __ testq(rax, rax);
    __ jcc(Assembler::zero, slow_path);

    __ movq(rarg0, Address(rbx, methodOopDesc::constants_offset()));
    // read first instruction word and extract bytecode @ 1 and index @ 2
    __ movq(rarg2, Address(rbx, methodOopDesc::const_offset()));
    __ movl(rarg2, Address(rarg2, constMethodOopDesc::codes_offset()));
    // Shift codes right to get the index on the right.
    // The bytecode fetched looks like <index><0xb4><0x2a>
    __ shrl(rarg2, 2 * BitsPerByte);
    __ shll(rarg2, exact_log2(in_words(ConstantPoolCacheEntry::size())));
    __ movq(rarg0, Address(rarg0, constantPoolOopDesc::cache_offset_in_bytes()));

    // rax: local 0
    // rbx: method
    // rarg3: receiver - do not destroy since it is needed for slow path!
    // edx: constant pool cache index
    // rarg0: constant pool cache

    // check if getfield has been resolved and read constant pool cache entry
    // check the validity of the cache entry by testing whether _indices field
    // contains Bytecode::_getfield in b1 byte.
    assert(in_words(ConstantPoolCacheEntry::size()) == 4,
           "adjust shift below");
    __ movl(rarg1,
	    Address(rarg0,
		    rarg2,
		    Address::times_8,
                    constantPoolCacheOopDesc::base_offset() +
                    ConstantPoolCacheEntry::indices_offset()));
    __ shrl(rarg1, 2 * BitsPerByte);
    __ andl(rarg1, 0xFF);
    __ cmpl(rarg1, Bytecodes::_getfield);
    __ jcc(Assembler::notEqual, slow_path);

    // Note: constant pool entry is not valid before bytecode is resolved
    __ movq(rarg1,
	    Address(rarg0,
		    rarg2,
		    Address::times_8,
                    constantPoolCacheOopDesc::base_offset() +
                    ConstantPoolCacheEntry::f2_offset()));
    // edx: flags
    __ movl(rarg2,
	    Address(rarg0,
		    rarg2,
		    Address::times_8,
                    constantPoolCacheOopDesc::base_offset() +
                    ConstantPoolCacheEntry::flags_offset()));

    Label notObj, notInt, notByte, notShort;
    const Address field_address(rax, rarg1, Address::times_1);

    // Need to differentiate between igetfield, agetfield, bgetfield etc.
    // because they are different sizes.
    // Use the type from the constant pool cache
    __ shrl(rarg2, ConstantPoolCacheEntry::tosBits);
    // Make sure we don't need to mask edx for tosBits after the above shift
    ConstantPoolCacheEntry::verify_tosBits();

    __ cmpl(rarg2, atos);
    __ jcc(Assembler::notEqual, notObj);
    // atos
    __ movq(rax, field_address);
    __ jmp(xreturn_path);

    __ bind(notObj);
    __ cmpl(rarg2, itos);
    __ jcc(Assembler::notEqual, notInt);
    // itos
    __ movl(rax, field_address);
    __ jmp(xreturn_path);

    __ bind(notInt);
    __ cmpl(rarg2, btos);
    __ jcc(Assembler::notEqual, notByte);
    // btos
    __ load_signed_byte(rax, field_address);
    __ jmp(xreturn_path);

    __ bind(notByte);
    __ cmpl(rarg2, stos);
    __ jcc(Assembler::notEqual, notShort);
    // stos
    __ load_signed_word(rax, field_address);
    __ jmp(xreturn_path);

    __ bind(notShort);
#ifdef ASSERT
    Label okay;
    __ cmpl(rarg2, ctos);
    __ jcc(Assembler::equal, okay);
    __ stop("what type is this?");
    __ bind(okay);
#endif
    // ctos
    __ load_unsigned_word(rax, field_address);

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

// Interpreter stub for calling a native method. (asm interpreter)
// This sets up a somewhat different looking stack for calling the
// native method than the typical interpreter frame setup.
address InterpreterGenerator::generate_native_entry(bool synchronized) 
{
  // determine code generation flags
  bool inc_counter  = UseCompiler || CountCompiledCalls;

  // rbx: methodOop
  // rarg3: receiver (unused)
  // r13: previous interpreter state (C++ interpreter) must preserve
  address entry_point = __ pc();

#ifndef CORE
  // check if compiled code exists
  Label run_compiled_code;
  if (!PreferInterpreterNativeStubs && !CompileTheWorld) {
    check_for_compiled_code(run_compiled_code);
  }
#endif

  const Address size_of_parameters(rbx, methodOopDesc::
                                        size_of_parameters_offset());
#ifndef CORE
  const Address invocation_counter(rbx, methodOopDesc::
                                        invocation_counter_offset() +
                                        InvocationCounter::counter_offset());
#endif
  const Address access_flags      (rbx, methodOopDesc::access_flags_offset());

  // get parameter size (always needed)
  __ load_unsigned_word(rarg3, size_of_parameters);

  // native calls don't need the stack size check since they have no
  // expression stack and the arguments are already on the stack and
  // we only add a handful of words to the stack

  // rbx: methodOop
  // rarg3: size of parameters
  __ popq(rax);                                       // get return address

  // for natives the size of locals is zero

  // compute beginning of parameters (r14)
  __ leaq(r14, Address(rsp, rarg3, Address::times_8, -wordSize));
  __ movq(r13, rsp); // remember sender sp

  // add 2 zero-initialized slots for native calls
  __ pushq((int) NULL); // slot for native result type info (setup
                        // via runtime)
  __ pushq((int) NULL); // slot for static native method holder mirror
                        // (setup via runtime)

#ifndef CORE
  if (inc_counter) {
    __ movl(rarg3, invocation_counter);  // (pre-)fetch invocation count
  }
#endif

  // initialize fixed part of activation frame
  generate_fixed_frame(true);

  // make sure method is native & not abstract
#ifdef ASSERT
  __ movl(rax, access_flags);
  {
    Label L;
    __ testl(rax, JVM_ACC_NATIVE);
    __ jcc(Assembler::notZero, L);
    __ stop("tried to execute non-native method as native");
    __ bind(L);
  }
  {
    Label L;
    __ testl(rax, JVM_ACC_ABSTRACT);
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

  const Address do_not_unlock_if_synchronized(r15_thread,
        in_bytes(JavaThread::do_not_unlock_if_synchronized_offset()));
  __ movl(do_not_unlock_if_synchronized, (int) true);

#ifndef CORE
  // increment invocation count & check for overflow
  Label invocation_counter_overflow;
  if (inc_counter) {
    generate_counter_incr(&invocation_counter_overflow, NULL, NULL);
  }

#endif // CORE

  bang_stack_shadow_pages(true);

  // reset the _do_not_unlock_if_synchronized flag
  __ movl(do_not_unlock_if_synchronized, (int) false);

  // check for synchronized methods
  // Must happen AFTER invocation_counter check and stack overflow check,
  // so method is not locked if overflows.
  if (synchronized) {
    lock_method();
  } else {
    // no synchronization necessary
#ifdef ASSERT
    {
      Label L;
      __ movl(rax, access_flags);
      __ testl(rax, JVM_ACC_SYNCHRONIZED);
      __ jcc(Assembler::zero, L);
      __ stop("method needs synchronization");
      __ bind(L);
    }
#endif
  }

  // start execution
#ifdef ASSERT
  {
    Label L;
    const Address monitor_block_top(rbp,
                 frame::interpreter_frame_monitor_block_top_offset * wordSize);
    __ movq(rax, monitor_block_top);
    __ cmpq(rax, rsp);
    __ jcc(Assembler::equal, L);
    __ stop("broken stack frame setup in interpreter");
    __ bind(L);
  }
#endif

  // jvmti/jvmpi support
  __ notify_method_entry();

  // work registers
  const Register method = rbx;
  const Register t      = r12;

  // allocate space for parameters
  __ get_method(method);
  __ verify_oop(method);
  __ load_unsigned_word(t,
                        Address(method,
                                methodOopDesc::size_of_parameters_offset()));
  __ shll(t, LogBytesPerWord);

  __ subq(rsp, t);
  __ subq(rsp, frame::arg_reg_save_area_bytes); // windows
  __ andq(rsp, -16); // must be 16 byte boundry (see amd64 ABI)

  // get signature handler
  {
    Label L;
    __ movq(t, Address(method, methodOopDesc::signature_handler_offset()));
    __ testq(t, t);
    __ jcc(Assembler::notZero, L);
    __ call_VM(noreg,
               CAST_FROM_FN_PTR(address,
                                InterpreterRuntime::prepare_native_call),
               method);
    __ get_method(method);
    __ movq(t, Address(method, methodOopDesc::signature_handler_offset()));
    __ bind(L);
  }

  // call signature handler
  assert(InterpreterRuntime::SignatureHandlerGenerator::from() == r14,
         "adjust this code");
  assert(InterpreterRuntime::SignatureHandlerGenerator::to() == rsp,
         "adjust this code");
  assert(InterpreterRuntime::SignatureHandlerGenerator::temp() == rscratch1,
          "adjust this code");

  // The generated handlers do not touch RBX (the method oop).
  // However, large signatures cannot be cached and are generated
  // each time here.  The slow-path generator can do a GC on return,
  // so we must reload it after the call.
  __ call(t, relocInfo::none);
  __ get_method(method);        // slow path can do a GC, reload RBX


  // result handler is in rax
  // set result handler
  __ movq(Address(rbp,
                  (frame::interpreter_frame_mirror_offset + 1) * wordSize),
          rax);

  // pass mirror handle if static call
  {
    Label L;
    const int mirror_offset = klassOopDesc::klass_part_offset_in_bytes() +
                              Klass::java_mirror_offset_in_bytes();
    __ movl(t, Address(method, methodOopDesc::access_flags_offset()));
    __ testl(t, JVM_ACC_STATIC);
    __ jcc(Assembler::zero, L);
    // get mirror
    __ movq(t, Address(method, methodOopDesc::constants_offset()));
    __ movq(t, Address(t, constantPoolOopDesc::pool_holder_offset_in_bytes()));
    __ movq(t, Address(t, mirror_offset));
    // copy mirror into activation frame
    __ movq(Address(rbp, frame::interpreter_frame_mirror_offset * wordSize),
            t);
    // pass handle to mirror
    __ leaq(rarg1,
            Address(rbp, frame::interpreter_frame_mirror_offset * wordSize));
    __ bind(L);
  }

  // get native function entry point
  {
    Label L;
    __ movq(rax, Address(method, methodOopDesc::native_function_offset()));
    __ movq(rscratch2, (int64_t)
            SharedRuntime::native_method_throw_unsatisfied_link_error_entry());
    __ cmpq(rax, rscratch2);
    __ jcc(Assembler::notEqual, L);
    __ call_VM(noreg,
               CAST_FROM_FN_PTR(address,
                                InterpreterRuntime::prepare_native_call),
               method);
    __ get_method(method);
    __ verify_oop(method);
    __ movq(rax, Address(method, methodOopDesc::native_function_offset()));
    __ bind(L);
  }

  // pass JNIEnv
  __ leaq(rarg0, Address(r15_thread, JavaThread::jni_environment_offset()));

  // reset handle block
  __ movq(t, Address(r15_thread, JavaThread::active_handles_offset()));
  __ movq(Address(t, JNIHandleBlock::top_offset_in_bytes()), 0);

  // set_last_Java_frame_before_call
  __ movq(Address(r15_thread, JavaThread::last_Java_fp_offset()), rbp);
#ifdef ASSERT
  {
    Label L;
    __ cmpl(Address(r15_thread,
                    JavaThread::frame_anchor_offset() +
                    JavaFrameAnchor::flags_offset()),
            0);
    __ jcc(Assembler::equal, L);
    __ stop("InterpreterGenerator::generate_native_entry: flags not cleared");
    __ bind(L);
  }
#endif /* ASSERT */

  // change thread state
#ifdef ASSERT
  {
    Label L;
    __ movl(t, Address(r15_thread, JavaThread::thread_state_offset()));
    __ cmpl(t, _thread_in_Java);
    __ jcc(Assembler::equal, L);
    __ stop("Wrong thread state in native stub");
    __ bind(L);
  }
#endif

  // Change state to native (we save the return address in the thread,
  // since it might not be pushed on the stack when we do a a stack
  // traversal). It is enough that the pc() points into the right code
  // segment. It does not have to be the correct return pc.
  __ leaq(t, Address((address) __ pc(), relocInfo::none));
  __ movq(Address(r15_thread,
		  JavaThread::frame_anchor_offset() +
                  JavaFrameAnchor::last_Java_pc_offset()),
          t);
  __ movq(Address(r15_thread, JavaThread::last_Java_sp_offset()), rsp);
  __ movl(Address(r15_thread, JavaThread::thread_state_offset()),
          _thread_in_native);

  // Call the native method.
  __ call(rax, relocInfo::none);

  // result potentially in rax or xmm0
  __ get_method(method);

#ifndef WIN64
  // On non-Windows systems, we are not guaranteed that native code preserves the
  // %mxcsr register so restore it to the value the VM requires
  __ ldmxcsr(Address(StubRoutines::amd64::mxcsr_std(), relocInfo::none));
#endif
  // restore r13 to have legal interpreter frame, i.e., bci == 0 <=>
  // r13 == code_base()
  __ movq(r13, Address(method, methodOopDesc::const_offset()));   // get constMethodOop
  __ leaq(r13, Address(r13, constMethodOopDesc::codes_offset())); // get codebase

  __ push(ltos);
  __ push(dtos);

  // change thread state
  __ movl(Address(r15_thread, JavaThread::thread_state_offset()),
          _thread_in_native_trans);

  if (os::is_MP()) // Force this write out before the read below
    __ membar(Assembler::Membar_mask_bits(Assembler::LoadLoad  |
                                          Assembler::LoadStore |
                                          Assembler::StoreLoad |
                                          Assembler::StoreStore));

  // check for safepoint operation in progress and/or pending suspend requests
  {
    Label Continue;
    __ cmpl(Address((address) SafepointSynchronize::address_of_state(),
                    relocInfo::none),
            SafepointSynchronize::_not_synchronized);

    Label L;
    __ jcc(Assembler::notEqual, L);
    __ cmpl(Address(r15_thread, JavaThread::suspend_flags_offset()), 0);
    __ jcc(Assembler::equal, Continue);
    __ bind(L);

    // Don't use call_VM as it will see a possible pending exception
    // and forward it and never return here preventing us from
    // clearing _last_native_pc down below.  Also can't use
    // call_VM_leaf either as it will check to see if r13 & r14 are
    // preserved and correspond to the bcp/locals pointers. So we do a
    // runtime call by hand.
    //
    __ movq(rarg0, r15_thread);
    __ movq(r12, rsp); // remember sp
    __ subq(rsp, wordSize); // windows
    __ andq(rsp, -16); // align stack as required by ABI
    __ call(CAST_FROM_FN_PTR(address,
                     JavaThread::check_safepoint_and_suspend_for_native_trans),
            relocInfo::runtime_call_type);
    __ movq(rsp, r12); // restore sp
    __ get_method(method); // XXX do we need this ???
    __ bind(Continue);
  }

  // change thread state
  __ movl(Address(r15_thread, JavaThread::thread_state_offset()), _thread_in_Java);

  // reset_last_Java_frame
  __ movq(Address(r15_thread, JavaThread::last_Java_sp_offset()), (int) NULL);
  __ movq(Address(r15_thread,
                  JavaThread::frame_anchor_offset() +
                  JavaFrameAnchor::last_Java_pc_offset()),
          (int) NULL);
  __ movl(Address(r15_thread,
                  JavaThread::frame_anchor_offset() +
                  JavaFrameAnchor::flags_offset()),
          0);
  // must clear fp, so that compiled frames are not confused; it is
  // possible that we need it only for debugging
  __ movq(Address(r15_thread, JavaThread::last_Java_fp_offset()), (int) NULL);

  {
    Label no_reguard;
    __ cmpl(Address(r15_thread, JavaThread::stack_guard_state_offset()),
            JavaThread::stack_guard_yellow_disabled);
    __ jcc(Assembler::notEqual, no_reguard);

    __ pushaq(); // XXX only save smashed registers
    __ movq(r12, rsp); // remember sp
    __ andq(rsp, -16); // align stack as required by ABI
    __ call(CAST_FROM_FN_PTR(address, SharedRuntime::reguard_yellow_pages),
            relocInfo::runtime_call_type);
    __ movq(rsp, r12); // restore sp
    __ popaq(); // XXX only restore smashed registers

    __ bind(no_reguard);
  }

  // JVMDI jframeIDs are invalidated on exit from native method.
  // JVMTI does not use jframeIDs, this whole mechanism must be
  // removed when JVMDI is removed.
  if (JvmtiExport::must_purge_jvmdi_frames_on_native_exit()) {
    // Can not use call_VM_leaf because of r13/r14 checking
    __ movq(r12, rsp); // remember sp
    __ andq(rsp, -16); // align stack as required by ABI
    __ call(CAST_FROM_FN_PTR(address, JvmtiExport::thread_leaving_native_code),
            relocInfo::runtime_call_type);
    __ movq(rsp, r12); // restore sp
  }

  // handle exceptions (exception handling will handle unlocking!)
  {
    Label L;
    __ cmpq(Address(r15_thread, Thread::pending_exception_offset()), (int) NULL);
    __ jcc(Assembler::zero, L);
    // Note: At some point we may want to unify this with the code
    // used in call_VM_base(); i.e., we should use the
    // StubRoutines::forward_exception code. For now this doesn't work
    // here because the rsp is not correctly set at this point.
    __ MacroAssembler::call_VM(noreg,
                               CAST_FROM_FN_PTR(address,
                               InterpreterRuntime::throw_pending_exception));
    __ should_not_reach_here();
    __ bind(L);
  }

  // do unlocking if necessary
  {
    Label L;
    __ movl(t, Address(method, methodOopDesc::access_flags_offset()));
    __ testl(t, JVM_ACC_SYNCHRONIZED);
    __ jcc(Assembler::zero, L);
    // the code below should be shared with interpreter macro
    // assembler implementation
    {
      Label unlock;
      // BasicObjectLock will be first in list, since this is a
      // synchronized method. However, need to check that the object
      // has not been unlocked by an explicit monitorexit bytecode.
      const Address monitor(rbp,
                            (intptr_t)(frame::interpreter_frame_initial_sp_offset *
                                       wordSize - sizeof(BasicObjectLock)));

      __ leaq(rarg1, monitor); // address of first monitor

      __ movq(t, Address(rarg1, BasicObjectLock::obj_offset_in_bytes()));
      __ testq(t, t);
      __ jcc(Assembler::notZero, unlock);
				
      // Entry already unlocked, need to throw exception
      __ MacroAssembler::call_VM(noreg,
                                 CAST_FROM_FN_PTR(address,
                   InterpreterRuntime::throw_illegal_monitor_state_exception));
      __ should_not_reach_here();
  
      __ bind(unlock);
      __ unlock_object(rarg1);
    }
    __ bind(L);
  }

  // jvmti/jvmpi support
  // Note: This must happen _after_ handling/throwing any exceptions since
  //       the exception handler code notifies the runtime of method exits
  //       too. If this happens before, method entry/exit notifications are
  //       not properly paired (was bug - gri 11/22/99).
  __ notify_method_exit(vtos);

  // restore potential result in edx:eax, call result handler to
  // restore potential result in ST0 & handle result

  __ pop(dtos);
  __ pop(ltos);

  __ movq(t, Address(rbp,
                     (frame::interpreter_frame_mirror_offset + 1) * wordSize));
  __ call(t, relocInfo::none);

  // remove activation
  __ movq(t, Address(rbp,
                     frame::interpreter_frame_sender_sp_offset * 
                     wordSize)); // get sender sp
  __ leave();                                // remove frame anchor
  __ popq(rarg0);                              // get return address
  __ movq(rsp, t);                           // set sp to sender sp
  __ jmp(rarg0);

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
  __ movl(do_not_unlock_if_synchronized, (int) false);

  generate_run_compiled_code();
#endif

  return entry_point;
}

//
// Generic interpreted method entry to (asm) interpreter
//
address InterpreterGenerator::generate_asm_interpreter_entry(bool synchronized)
{
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

  const Address size_of_parameters(rbx,
                                   methodOopDesc::size_of_parameters_offset());
  const Address size_of_locals(rbx, methodOopDesc::size_of_locals_offset());
#ifndef CORE
  const Address invocation_counter(rbx,
                                   methodOopDesc::invocation_counter_offset() +
                                   InvocationCounter::counter_offset());
#endif
  const Address access_flags(rbx, methodOopDesc::access_flags_offset());

  // get parameter size (always needed)
  __ load_unsigned_word(rarg3, size_of_parameters);

  // rbx: methodOop
  // rarg3: size of parameters
  __ load_unsigned_word(rarg2, size_of_locals); // get size of locals in words
  __ subl(rarg2, rarg3); // rarg2 = no. of additional locals

  // YYY
//   __ incl(rarg2);
//   __ andl(rarg2, -2);

  // see if we've got enough room on the stack for locals plus overhead.
  generate_stack_overflow_check();

  // get return address
  __ popq(rax);

  // compute beginning of parameters (r14)
  __ leaq(r14, Address(rsp, rarg3, Address::times_8, -wordSize));
  // remember sender sp
  __ movq(r13, rsp);

  // rarg2 - # of additional locals
  // allocate space for locals
  // explicitly initialize locals
  {
    Label exit, loop;
    __ testl(rarg2, rarg2);
    __ jcc(Assembler::lessEqual, exit); // do nothing if rarg2 <= 0
    __ bind(loop);
    __ pushq((int) NULL); // initialize local variables
    __ decl(rarg2); // until everything initialized
    __ jcc(Assembler::greater, loop);
    __ bind(exit);
  }

#ifndef CORE
  // (pre-)fetch invocation count
  if (inc_counter) {
    __ movl(rarg3, invocation_counter);
  }
#endif
  // initialize fixed part of activation frame
  generate_fixed_frame(false);

  // make sure method is not native & not abstract
#ifdef ASSERT
  __ movl(rax, access_flags);
  {
    Label L;
    __ testl(rax, JVM_ACC_NATIVE);
    __ jcc(Assembler::zero, L);
    __ stop("tried to execute native method as non-native");
    __ bind(L);
  }
  {
    Label L;
    __ testl(rax, JVM_ACC_ABSTRACT);
    __ jcc(Assembler::zero, L);
    __ stop("tried to execute abstract method in interpreter");
    __ bind(L);
  }
#endif

  // Since at this point in the method invocation the exception
  // handler would try to exit the monitor of synchronized methods
  // which hasn't been entered yet, we set the thread local variable
  // _do_not_unlock_if_synchronized to true. The remove_activation
  // will check this flag.

  const Address do_not_unlock_if_synchronized(r15_thread,
        in_bytes(JavaThread::do_not_unlock_if_synchronized_offset()));
  __ movl(do_not_unlock_if_synchronized, (int) true);

#ifndef CORE
  // increment invocation count & check for overflow
  Label invocation_counter_overflow;
  Label profile_method;
  Label profile_method_continue;
  if (inc_counter) {
    generate_counter_incr(&invocation_counter_overflow,
                          &profile_method,
                          &profile_method_continue);
    if (ProfileInterpreter) {
      __ bind(profile_method_continue);
    }
  }
#endif // CORE

  // check for synchronized interpreted methods
  bang_stack_shadow_pages(false);

  // reset the _do_not_unlock_if_synchronized flag
  __ movl(do_not_unlock_if_synchronized, (int) false);

  // check for synchronized methods
  // Must happen AFTER invocation_counter check and stack overflow check,
  // so method is not locked if overflows.
  if (synchronized) {
    // Allocate monitor and lock method
    lock_method();
  } else {
    // no synchronization necessary
#ifdef ASSERT
    {
      Label L;
      __ movl(rax, access_flags);
      __ testl(rax, JVM_ACC_SYNCHRONIZED);
      __ jcc(Assembler::zero, L);
      __ stop("method needs synchronization");
      __ bind(L);
    }
#endif
  }

  // start execution
#ifdef ASSERT
  {
    Label L;
     const Address monitor_block_top (rbp,
                 frame::interpreter_frame_monitor_block_top_offset * wordSize);
    __ movq(rax, monitor_block_top);
    __ cmpq(rax, rsp);
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

      __ call_VM(noreg,
                 CAST_FROM_FN_PTR(address, InterpreterRuntime::profile_method),
                 r13, true);

      __ movq(rbx, Address(rbp, method_offset)); // restore methodOop
      __ movq(rax, Address(rbx,
                           in_bytes(methodOopDesc::method_data_offset())));
      __ movq(Address(rbp, frame::interpreter_frame_mdx_offset * wordSize),
              rax);
      __ test_method_data_pointer(rax, profile_method_continue);
      __ addq(rax, in_bytes(methodDataOopDesc::data_offset()));
      __ movq(Address(rbp, frame::interpreter_frame_mdx_offset * wordSize),
              rax);
      __ jmp(profile_method_continue);
    }
    // Handle overflow of counter and compile method
    __ bind(invocation_counter_overflow);
    generate_counter_overflow(entry_point);
    // entry_point is the beginning of this function and checks again
    // for compiled code
  }
  // Compiled code exists run it
  __ bind(run_compiled_code);

  // reset the _do_not_unlock_if_synchronized flag - can get here
  // without unlocking above if the invocation counter overflows
  __ movl(do_not_unlock_if_synchronized, (int) false);

  generate_run_compiled_code();
#endif

  return entry_point;
}

// Entry points
// 
// Here we generate the various kind of entries into the interpreter.
// The two main entry type are generic bytecode methods and native
// call method.  These both come in synchronized and non-synchronized
// versions but the frame layout they create is very similar. The
// other method entry types are really just special purpose entries
// that are really entry and interpretation all in one. These are for
// trivial methods like accessor, empty, or special math methods.
//
// When control flow reaches any of the entry types for the interpreter
// the following holds ->
//
// Arguments:
//
// rbx: methodOop
// rarg3: receiver
//
// Stack layout immediately at entry
//
// [ return address     ] <--- rsp
// [ parameter n        ]
//   ...
// [ parameter 1        ]
// [ expression stack   ] (caller's java expression stack)

// Assuming that we don't go to one of the trivial specialized entries
// the stack will look like below when we are ready to execute the
// first bytecode (or call the native routine). The register usage
// will be as the template based interpreter expects (see
// interpreter_amd64.hpp).
//
// local variables follow incoming parameters immediately; i.e.
// the return address is moved to the end of the locals).
//
// [ monitor entry      ] <--- rsp
//   ...
// [ monitor entry      ]
// [ expr. stack bottom ]
// [ saved r13          ]
// [ current r14        ]
// [ methodOop          ]
// [ saved ebp          ] <--- rbp
// [ return address     ]
// [ local variable m   ]
//   ...
// [ local variable 1   ]
// [ parameter n        ]
//   ...
// [ parameter 1        ] <--- r14

address AbstractInterpreterGenerator::generate_method_entry(
                                        AbstractInterpreter::MethodKind kind)
{
  // determine code generation flags
  bool synchronized = false;
  address entry_point = NULL;

  switch (kind) {
  case Interpreter::zerolocals:
    break;
  case Interpreter::zerolocals_synchronized:
    synchronized = true;
    break;
  case Interpreter::native:
    entry_point = ((InterpreterGenerator*) this)->generate_native_entry(false);
    break;
  case Interpreter::native_synchronized:
    entry_point = ((InterpreterGenerator*) this)->generate_native_entry(true);
    break;
  case Interpreter::empty:
    entry_point = ((InterpreterGenerator*) this)->generate_empty_entry();
    break;
  case Interpreter::accessor:
    entry_point = ((InterpreterGenerator*) this)->generate_accessor_entry();
    break;
  case Interpreter::abstract:
    entry_point = ((InterpreterGenerator*) this)->generate_abstract_entry();
    break;
  case Interpreter::java_lang_math_sin:
    break;
  case Interpreter::java_lang_math_cos:
    break;
  case Interpreter::java_lang_math_sqrt:
    entry_point = ((InterpreterGenerator*) this)->generate_math_entry(kind);
    break;
  default:
    ShouldNotReachHere();
    break;
  }

  if (entry_point) {
    return entry_point;
  }

  return ((InterpreterGenerator*) this)->
                                generate_asm_interpreter_entry(synchronized);
}

// How much stack a method activation needs in words.
int AbstractInterpreter::size_top_interpreter_activation(methodOop method)
{
  const int entry_size = frame::interpreter_frame_monitor_size();

  // total overhead size: entry_size + (saved rbp thru expr stack
  // bottom).  be sure to change this if you add/subtract anything
  // to/from the overhead area
  const int overhead_size =
    -(frame::interpreter_frame_initial_sp_offset) + entry_size;

  const int stub_code = frame::entry_frame_after_call_words;
  return
    overhead_size + method->max_locals() + method->max_stack() + stub_code;
}

#ifndef CORE
// This method tells the deoptimizer how big an interpreted frame must be:
int AbstractInterpreter::size_activation(methodOop method,
                                         int tempcount,
                                         int moncount,
                                         int callee_param_size,
                                         int callee_locals,
                                         bool is_top_frame)
{
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
                                                bool is_top_frame)
{
  // Note: This calculation must exactly parallel the frame setup
  // in AbstractInterpreterGenerator::generate_method_entry.
  // If interpreter_frame!=NULL, set up the method, locals, and monitors.
  // The frame interpreter_frame, if not NULL, is guaranteed to be the
  // right size, as determined by a previous call to this method.
  // It is also guaranteed to be walkable even though it is in a skeletal state

  // fixed size of an interpreter frame:
  int max_locals = method->max_locals();
  int overhead = frame::sender_sp_offset -
                 frame::interpreter_frame_initial_sp_offset;
  // Our locals were accounted for by the caller (or last_frame_adjust
  // on the transistion) Since the callee parameters already account
  // for the callee's params we only need to account for the extra
  // locals.
  int size = overhead + callee_locals - callee_param_size +
             moncount * frame::interpreter_frame_monitor_size() +
             tempcount;
  if (interpreter_frame != NULL) {
#ifdef ASSERT
    assert(caller->sp() == interpreter_frame->interpreter_frame_sender_sp(),
           "Frame not properly walkable");
#endif

    interpreter_frame->interpreter_frame_set_method(method);
    // NOTE the difference in using sender_sp and
    // interpreter_frame_sender_sp interpreter_frame_sender_sp is
    // the original sp of the caller (the unextended_sp) and
    // sender_sp is fp+16 XXX
    intptr_t* locals = interpreter_frame->sender_sp() + max_locals - 1;

    interpreter_frame->interpreter_frame_set_locals(locals);
    BasicObjectLock* montop =
      interpreter_frame->interpreter_frame_monitor_begin();
    interpreter_frame->interpreter_frame_set_monitor_end(montop - moncount);

    // All frames but the initial interpreter frame we fill in have
    // a value for sender_sp that allows walking the stack but isn't
    // truly correct. Correct the value here.
    int extra_locals = method->max_locals() - method->size_of_parameters();
    if (extra_locals != 0 &&
        interpreter_frame->sender_sp() ==
        interpreter_frame->interpreter_frame_sender_sp()) {
      interpreter_frame->set_interpreter_frame_sender_sp(caller->sp() +
                                                         extra_locals);
    }
    *interpreter_frame->interpreter_frame_cache_addr() =
      method->constants()->cache();
  }
  return size;
}
#endif /* CORE */


//-----------------------------------------------------------------------------
// Exceptions

void AbstractInterpreterGenerator::generate_throw_exception()
{
  // Entry point in previous activation (i.e., if the caller was
  // interpreted)
  Interpreter::_rethrow_exception_entry = __ pc();
  // rax: exception
  // rdx: return address/pc that threw exception
  __ restore_bcp();    // r13 points to call/send
  __ restore_locals();
  // Entry point for exceptions thrown within interpreter code
  Interpreter::_throw_exception_entry = __ pc();
  // expression stack is undefined here
  // rax: exception
  // r13: exception bcp
  __ verify_oop(rax);
  __ movq(rarg1, rax);

  // expression stack must be empty before entering the VM in case of
  // an exception
  __ empty_expression_stack();
  // find exception handler address and preserve exception oop
  __ call_VM(rarg2,
             CAST_FROM_FN_PTR(address,
                          InterpreterRuntime::exception_handler_for_exception),
             rarg1);
  // rax: exception handler entry point
  // rarg2: preserved exception oop
  // r13: bcp for exception handler
  __ pushq(rarg2); // push exception which is now the only value on the stack
  __ jmp(rax); // jump to exception handler (may be _remove_activation_entry!)

  // If the exception is not handled in the current frame the frame is
  // removed and the exception is rethrown (i.e. exception
  // continuation is _rethrow_exception).
  //
  // Note: At this point the bci is still the bxi for the instruction
  // which caused the exception and the expression stack is
  // empty. Thus, for any VM calls at this point, GC will find a legal
  // oop map (with empty expression stack).

  // In current activation
  // tos: exception
  // esi: exception bcp

  //
  // JVMTI PopFrame support
  //

  Interpreter::_remove_activation_preserving_args_entry = __ pc();
  __ empty_expression_stack();
  // Set the popframe_processing bit in pending_popframe_condition
  // indicating that we are currently handling popframe, so that
  // call_VMs that may happen later do not trigger new popframe
  // handling cycles.
  __ movl(rarg2, Address(r15_thread, JavaThread::popframe_condition_offset()));
  __ orl(rarg2, JavaThread::popframe_processing_bit);
  __ movl(Address(r15_thread, JavaThread::popframe_condition_offset()), rarg2);

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
    __ movq(rarg2, Address(rbp, frame::return_addr_offset * wordSize));
    __ super_call_VM_leaf(CAST_FROM_FN_PTR(address,
                               InterpreterRuntime::interpreter_contains), rarg2);
    __ testl(rax, rax);
    __ jcc(Assembler::notZero, caller_not_deoptimized);

    // Compute size of arguments for saving when returning to
    // deoptimized caller
    __ get_method(rax);
    __ load_unsigned_word(rax, Address(rax, in_bytes(methodOopDesc::
                                                size_of_parameters_offset())));
    __ shll(rax, LogBytesPerWord);
    __ restore_locals(); // XXX do we need this?
    __ subq(r14, rax);
    __ addq(r14, wordSize);
    // Save these arguments
    __ super_call_VM_leaf(CAST_FROM_FN_PTR(address,
                                           Deoptimization::
                                           popframe_preserve_args),
                          r15_thread, rax, r14);

    __ remove_activation(vtos, rarg2,
			 /* throw_monitor_exception */ false,
			 /* install_monitor_exception */ false,
			 /* notify_jvmdi */ false);

    // Inform deoptimization that it is responsible for restoring
    // these arguments
    __ movl(Address(r15_thread, JavaThread::popframe_condition_offset()), 
            JavaThread::popframe_force_deopt_reexecution_bit);

    // Continue in deoptimization handler
    __ jmp(rarg2);

    __ bind(caller_not_deoptimized);
  }
#endif /* !CORE */

  __ remove_activation(vtos, rarg2,
                       /* throw_monitor_exception */ false,
                       /* install_monitor_exception */ false,
                       /* notify_jvmdi */ false);

  // Clear the popframe condition flag
  __ movl(Address(r15_thread, JavaThread::popframe_condition_offset()),
          JavaThread::popframe_inactive);

  // Finish with popframe handling
  __ restore_bcp();  // XXX do we need this?
  __ restore_locals(); // XXX do we need this?
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
  __ popq(rax);
  __ movq(Address(r15_thread, JavaThread::vm_result_offset()), rax);
  // remove the activation (without doing throws on illegalMonitorExceptions)
  __ remove_activation(vtos, rarg2, false, true, false);
  // restore exception
  __ movq(rax, Address(r15_thread, JavaThread::vm_result_offset()));
  __ movq(Address(r15_thread, JavaThread::vm_result_offset()), (int) NULL);
  __ verify_oop(rax);

  // In between activations - previous activation type unknown yet
  // compute continuation point - the continuation point expects the
  // following registers set up:
  //
  // rax: exception
  // rdx: return address/pc that threw exception
  // rsp: expression stack of caller
  // rbp: ebp of caller
  __ pushq(rax);                                 // save exception
  __ pushq(rarg2);                                 // save return address
  __ super_call_VM_leaf(CAST_FROM_FN_PTR(address,
                          SharedRuntime::exception_handler_for_return_address),
                        rarg2);
  __ movq(rbx, rax);                             // save exception handler
  __ popq(rdx);                                  // restore return address
  __ popq(rax);                                  // restore exception
  // Note that an "issuing PC" is actually the next PC after the call
  __ jmp(rbx);                                   // jump to exception
                                                 // handler of caller
}

//-----------------------------------------------------------------------------
// Helper for vtos entry point generation

void AbstractInterpreterGenerator::set_vtos_entry_points(Template* t,
                                                         address& bep,
                                                         address& cep,
                                                         address& sep,
                                                         address& aep,
                                                         address& iep,
                                                         address& lep,
                                                         address& fep,
                                                         address& dep,
                                                         address& vep)
{
  assert(t->is_valid() && t->tos_in() == vtos, "illegal template");
  Label L;
  bep = __ pc();  __ push_i();    __ jmp(L);
  cep = __ pc();  __ push_i();    __ jmp(L);
  sep = __ pc();  __ push_i();    __ jmp(L);
  aep = __ pc();  __ push_ptr();  __ jmp(L);
  fep = __ pc();  __ push_f();    __ jmp(L);
  dep = __ pc();  __ push_d();    __ jmp(L);
  lep = __ pc();  __ push_l();    __ jmp(L);
  iep = __ pc();  __ push_i();
  vep = __ pc();
  __ bind(L);
  generate_and_dispatch(t);
}


//-----------------------------------------------------------------------------
// Generation of individual instructions

// helpers for generate_and_dispatch


InterpreterGenerator::InterpreterGenerator(StubQueue* code)
  : AbstractInterpreterGenerator(code)
{
   generate_all(); // down here so it can be "virtual"
}

//-----------------------------------------------------------------------------

// when JVM/PI is retired this method can be made '#ifndef PRODUCT'
address AbstractInterpreterGenerator::generate_trace_code(TosState state)
{
  address entry = __ pc();

  __ push(state);
  __ pushq(rarg0);
  __ pushq(rarg1);
  __ pushq(rarg2);
  __ pushq(rarg3);
  __ movq(rarg2, rax);  // Pass itos
#ifdef _WIN64
  __ movss(xmm3, xmm0); // Pass ftos 
#endif
  __ call_VM(noreg,
             CAST_FROM_FN_PTR(address, SharedRuntime::trace_bytecode),
             rarg1, rarg2, rarg3);
  __ popq(rarg3);
  __ popq(rarg2);
  __ popq(rarg1);
  __ popq(rarg0);
  __ pop(state);
  __ ret(0);                                   // return from result handler

  return entry;
}

// Non-product code
#ifndef PRODUCT
void AbstractInterpreterGenerator::count_bytecode()
{
  __ incl(Address((address) &BytecodeCounter::_counter_value,
                  relocInfo::none));
}

void AbstractInterpreterGenerator::histogram_bytecode(Template* t)
{
  __ incl(Address((address) &BytecodeHistogram::_counters[t->bytecode()],
                  relocInfo::none));
}

void AbstractInterpreterGenerator::histogram_bytecode_pair(Template* t)
{
  __ movl(rbx, Address((address) &BytecodePairHistogram::_index,
                       relocInfo::none));
  __ shrl(rbx, BytecodePairHistogram::log2_number_of_codes);
  __ orl(rbx,
         ((int) t->bytecode()) <<
         BytecodePairHistogram::log2_number_of_codes);
  __ movl(Address((address) &BytecodePairHistogram::_index,
                  relocInfo::none),
          rbx);
  __ movq(rscratch1, (int64_t) BytecodePairHistogram::_counters);
  __ incl(Address(rscratch1, rbx, Address::times_4));
}
#endif // !PRODUCT


// when JVM/PI is retired this method can be made '#ifndef PRODUCT'
void AbstractInterpreterGenerator::trace_bytecode(Template* t)
{
  // Call a little run-time stub to avoid blow-up for each bytecode.
  // The run-time runtime saves the right registers, depending on
  // the tosca in-state for the given template.
  address entry = Interpreter::trace_code(t->tos_in());
  assert(entry != NULL, "entry must have been generated");
  __ movq(r12, rsp); // remember sp
  __ andq(rsp, -16); // align stack as required by ABI
  __ call(entry, relocInfo::none);
  __ movq(rsp, r12); // restore sp
}


// Non-product code
#ifndef PRODUCT
void AbstractInterpreterGenerator::stop_interpreter_at()
{
  Label L;
  __ cmpl(Address((address) &BytecodeCounter::_counter_value,
                  relocInfo::none),
          StopInterpreterAt);
  __ jcc(Assembler::notEqual, L);
  __ int3();
  __ bind(L);
}
#endif // !PRODUCT
