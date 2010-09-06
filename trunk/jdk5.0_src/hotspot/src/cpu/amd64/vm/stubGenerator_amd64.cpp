#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)stubGenerator_amd64.cpp	1.15 04/04/04 23:45:43 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_stubGenerator_amd64.cpp.incl"

// Declaration and definition of StubGenerator (no .hpp file).
// For a more detailed description of the stub routine structure
// see the comment in stubRoutines.hpp

#define __ _masm->

// Stub Code definitions

static address handle_unsafe_access() 
{
  JavaThread* thread = JavaThread::current();
  address pc = thread->saved_exception_pc();
  // pc is the instruction which we must emulate
  // doing a no-op is fine:  return garbage from the load
  // therefore, compute npc
  address npc = Assembler::locate_next_instruction(pc);

  // request an async exception
  thread->set_pending_unsafe_access_error();

  // return address of next instruction to execute
  return npc;
}

class StubGenerator: public StubCodeGenerator 
{
 private:
  // Call stubs are used to call Java from C
  //
  // Linux Arguments:
  //    rarg0:   call wrapper address                   address
  //    rarg1:   result                                 address
  //    rarg2:   result type                            BasicType
  //    rarg3:   method                                 methodOop
  //    rarg4:   (interpreter) entry point              address
  //    rarg5:   parameters                             intptr_t*
  //    16(rbp): parameter size (in words)              int
  //    24(rbp): thread                                 Thread*
  //
  //     [ return_from_Java     ] <--- rsp
  //     [ argument word n      ]
  //      ...
  // -12 [ argument word 1      ]
  // -11 [ saved r15            ] <--- rsp_after_call
  // -10 [ saved r14            ]
  //  -9 [ saved r13            ]
  //  -8 [ saved r12            ]
  //  -7 [ saved rbx            ]
  //  -6 [ call wrapper         ]
  //  -5 [ result               ]
  //  -4 [ result type          ]
  //  -3 [ method               ]
  //  -2 [ entry point          ]
  //  -1 [ parameters           ]
  //   0 [ saved rbp            ] <--- rbp
  //   1 [ return address       ]
  //   2 [ parameter size       ]
  //   3 [ thread               ]
  //
  // Windows Arguments:
  //    rarg0:   call wrapper address                   address
  //    rarg1:   result                                 address
  //    rarg2:   result type                            BasicType
  //    rarg3:   method                                 methodOop
  //    48(rbp): (interpreter) entry point              address
  //    56(rbp): parameters                             intptr_t*
  //    64(rbp): parameter size (in words)              int
  //    72(rbp): thread                                 Thread*
  //
  //     [ return_from_Java     ] <--- rsp
  //     [ argument word n      ]
  //      ...
  //  -8 [ argument word 1      ]
  //  -7 [ saved r15            ] <--- rsp_after_call
  //  -6 [ saved r14            ]
  //  -5 [ saved r13            ]
  //  -4 [ saved r12            ]
  //  -3 [ saved rdi            ]
  //  -2 [ saved rsi            ]
  //  -1 [ saved rbx            ]
  //   0 [ saved rbp            ] <--- rbp
  //   1 [ return address       ]
  //   2 [ call wrapper         ]
  //   3 [ result               ]
  //   4 [ result type          ]
  //   5 [ method               ]
  //   6 [ entry point          ]
  //   7 [ parameters           ]
  //   8 [ parameter size       ]
  //   9 [ thread               ]
  //
  //    Windows reserves the callers stack space for arguments 1-4.
  //    We spill rarg0-rarg3 to this space.

  // Call stub stack layout word offsets from rbp
  enum call_stub_layout {
#ifdef _WIN64
    rsp_after_call_off = -7,
    r15_off            = rsp_after_call_off,
    r14_off            = -6,
    r13_off            = -5,
    r12_off            = -4,
    rdi_off            = -3,
    rsi_off            = -2,
    rbx_off            = -1,
    rbp_off            =  0,
    retaddr_off        =  1,
    call_wrapper_off   =  2,
    result_off         =  3,
    result_type_off    =  4,
    method_off         =  5,
    entry_point_off    =  6,
    parameters_off     =  7,
    parameter_size_off =  8,
    thread_off         =  9
#else
    rsp_after_call_off = -12,
    mxcsr_off          = rsp_after_call_off,
    r15_off            = -11,
    r14_off            = -10,
    r13_off            = -9,
    r12_off            = -8,
    rbx_off            = -7,
    call_wrapper_off   = -6,
    result_off         = -5,
    result_type_off    = -4,
    method_off         = -3,
    entry_point_off    = -2,
    parameters_off     = -1,
    rbp_off            =  0,
    retaddr_off        =  1,
    parameter_size_off =  2,
    thread_off         =  3
#endif
  };

  address generate_call_stub(address& return_address) 
  {
    assert(frame::entry_frame_after_call_words == -rsp_after_call_off + 1 &&
           frame::entry_frame_call_wrapper_offset == call_wrapper_off,
           "adjust this code");
    StubCodeMark mark(this, "StubRoutines", "call_stub");
    address start = __ pc();
    
    // same as in generate_catch_exception()!
    const Address rsp_after_call(rbp, rsp_after_call_off * wordSize);

    const Address call_wrapper  (rbp, call_wrapper_off   * wordSize);
    const Address result        (rbp, result_off         * wordSize);
    const Address result_type   (rbp, result_type_off    * wordSize);
    const Address method        (rbp, method_off         * wordSize);
    const Address entry_point   (rbp, entry_point_off    * wordSize);
    const Address parameters    (rbp, parameters_off     * wordSize);
    const Address parameter_size(rbp, parameter_size_off * wordSize);

    // same as in generate_catch_exception()!
    const Address thread        (rbp, thread_off         * wordSize);

    const Address r15_save(rbp, r15_off * wordSize);
    const Address r14_save(rbp, r14_off * wordSize);
    const Address r13_save(rbp, r13_off * wordSize);
    const Address r12_save(rbp, r12_off * wordSize);
    const Address rbx_save(rbp, rbx_off * wordSize);

    // stub code
    __ enter();
    __ subq(rsp, -rsp_after_call_off * wordSize);

    // save register parameters
#ifndef _WIN64
    __ movq(parameters,   rarg5); // parameters
    __ movq(entry_point,  rarg4); // entry_point
#endif

    __ movq(method,       rarg3); // method
    __ movl(result_type,  rarg2); // result type
    __ movq(result,       rarg1); // result
    __ movq(call_wrapper, rarg0); // call wrapper

    // save regs belonging to calling function
    __ movq(rbx_save, rbx);
    __ movq(r12_save, r12);
    __ movq(r13_save, r13);
    __ movq(r14_save, r14);
    __ movq(r15_save, r15);

#ifdef _WIN64
    const Address rdi_save(rbp, rdi_off * wordSize);
    const Address rsi_save(rbp, rsi_off * wordSize);

    __ movq(rsi_save, rsi);
    __ movq(rdi_save, rdi);
#else
    const Address mxcsr_save(rbp, mxcsr_off * wordSize);

    __ stmxcsr(mxcsr_save);
    __ ldmxcsr(Address(StubRoutines::amd64::mxcsr_std(), relocInfo::none));
#endif

    // Load up thread register
    __ movq(r15_thread, thread);

#ifdef ASSERT
    // make sure we have no pending exceptions
    { 
      Label L;
      __ movq(rarg3, thread);
      __ cmpq(Address(rarg3, Thread::pending_exception_offset()), (int) NULL);
      __ jcc(Assembler::equal, L);
      __ stop("StubRoutines::call_stub: entered with pending exception");
      __ bind(L);
    }
#endif

    // pass parameters if any
    Label parameters_done;
    __ movl(rarg3, parameter_size);
    __ testl(rarg3, rarg3);
    __ jcc(Assembler::zero, parameters_done);

    Label loop;
    __ movq(rarg2, parameters);     // parameter pointer
    __ movl(rarg1, rarg3);          // parameter counter is in rarg1
    __ movl(rarg3, Address(rarg2)); // get first parameter in case it is
                                    // a receiver
    __ bind(loop);
    __ movq(rax, Address(rarg2));   // get parameter
    __ addq(rarg2, wordSize);       // advance to next parameter
    __ decl(rarg1);                 // decrement counter
    __ pushq(rax);                  // pass parameter
    __ jcc(Assembler::notZero, loop);

    // call Java function
    __ bind(parameters_done);
    __ movq(rbx, method);           // get methodOop
    __ movq(rarg1, entry_point);    // get entry_point
    __ call(rarg1, relocInfo::none);
    return_address = __ pc();

    // store result depending on type (everything that is not
    // T_OBJECT, T_LONG, T_FLOAT or T_DOUBLE is treated as T_INT)
    __ movq(rarg0, result);
    Label is_long, is_float, is_double, exit;
    __ movl(rarg1, result_type);
    __ cmpl(rarg1, T_OBJECT);
    __ jcc(Assembler::equal, is_long);
    __ cmpl(rarg1, T_LONG);
    __ jcc(Assembler::equal, is_long);
    __ cmpl(rarg1, T_FLOAT);
    __ jcc(Assembler::equal, is_float);
    __ cmpl(rarg1, T_DOUBLE);
    __ jcc(Assembler::equal, is_double);

    // handle T_INT case
    __ movl(Address(rarg0), rax);

    __ bind(exit);

    // pop parameters
    __ movl(rarg3, parameter_size);
    __ leaq(rsp, Address(rsp, rarg3, Address::times_8));

#ifdef ASSERT
    // check if parameters have been popped correctly
    Label rsp_wrong;
    __ leaq(rarg3, rsp_after_call);
    __ cmpq(rsp, rarg3);
    __ jcc(Assembler::notEqual, rsp_wrong);
#endif

#ifdef ASSERT
    // verify that threads correspond
    { 
      Label L, S;
      __ cmpq(r15_thread, thread);
      __ jcc(Assembler::notEqual, S);
      __ get_thread(rbx);
      __ cmpq(r15_thread, rbx);
      __ jcc(Assembler::equal, L);
      __ bind(S);
      __ jcc(Assembler::equal, L);
      __ stop("StubRoutines::call_stub: threads must correspond");
      __ bind(L);
    }
#endif

    // restore regs belonging to calling function
    __ movq(r15, r15_save);
    __ movq(r14, r14_save);
    __ movq(r13, r13_save);
    __ movq(r12, r12_save);
    __ movq(rbx, rbx_save);

#ifdef _WIN64
    __ movq(rdi, rdi_save);
    __ movq(rsi, rsi_save);
#else
    __ ldmxcsr(mxcsr_save);
#endif

    // restore rsp
    __ addq(rsp, -rsp_after_call_off * wordSize);

    // return
    __ popq(rbp);
    __ ret(0);

    // handle return types different from T_INT
    __ bind(is_long);
    __ movq(Address(rarg0), rax);
    __ jmp(exit);

    __ bind(is_float);
    __ movss(Address(rarg0), farg0);
    __ jmp(exit);

    __ bind(is_double);
    __ movsd(Address(rarg0), farg0);
    __ jmp(exit);

#ifdef ASSERT
      // stack pointer misadjusted
      __ bind(rsp_wrong);
      __ stop("rsp wrong after Java call");
#endif

    return start;
  }

  // Return point for a Java call if there's an exception thrown in
  // Java code.  The exception is caught and transformed into a
  // pending exception stored in JavaThread that can be tested from
  // within the VM.
  //
  // Note: Usually the parameters are removed by the callee. In case
  // of an exception crossing an activation frame boundary, that is
  // not the case if the callee is compiled code => need to setup the
  // rsp.
  //
  // rax: exception oop

  address generate_catch_exception() 
  {
    StubCodeMark mark(this, "StubRoutines", "catch_exception");
    address start = __ pc();

    // same as in generate_call_stub():
    const Address rsp_after_call(rbp, rsp_after_call_off * wordSize);
    const Address thread        (rbp, thread_off         * wordSize);

#ifdef ASSERT
    // verify that threads correspond
    { 
      Label L, S;
      __ cmpq(r15_thread, thread);
      __ jcc(Assembler::notEqual, S);
      __ get_thread(rbx);
      __ cmpq(r15_thread, rbx);
      __ jcc(Assembler::equal, L);
      __ bind(S);
      __ stop("StubRoutines::catch_exception: threads must correspond");
      __ bind(L);
    }
#endif

    // set pending exception
    __ verify_oop(rax);

    __ movq(Address(r15_thread, Thread::pending_exception_offset()), rax);
    __ movq(rscratch1, (int64_t) __FILE__);
    __ movq(Address(r15_thread, Thread::exception_file_offset()), rscratch1);
    __ movl(Address(r15_thread, Thread::exception_line_offset()), (int)  __LINE__);

    // complete return to VM
    assert(StubRoutines::_call_stub_return_address != NULL,
           "_call_stub_return_address must have been generated before");
    __ jmp(StubRoutines::_call_stub_return_address, relocInfo::none);

    return start;
  }

  
  // Continuation point for runtime calls returning with a pending
  // exception.  The pending exception check happened in the runtime
  // or native call stub.  The pending exception in Thread is
  // converted into a Java-level exception.
  //
  // Contract with Java-level exception handlers:
  // rax: exception
  // rdx: throwing pc
  //
  // NOTE: At entry of this stub, exception-pc must be on stack !!

  address generate_forward_exception() 
  {
    StubCodeMark mark(this, "StubRoutines", "forward exception");
    address start = __ pc();

    // Upon entry, the sp points to the return address returning into
    // Java (interpreted or compiled) code; i.e., the return address
    // becomes the throwing pc.
    //
    // Arguments pushed before the runtime call are still on the stack
    // but the exception handler will reset the stack pointer ->
    // ignore them.  A potential result in registers can be ignored as
    // well.

#ifdef ASSERT
    // make sure this code is only executed if there is a pending exception
    { 
      Label L;
      __ cmpq(Address(r15_thread, Thread::pending_exception_offset()), (int) NULL);
      __ jcc(Assembler::notEqual, L);
      __ stop("StubRoutines::forward exception: no pending exception (1)");
      __ bind(L);
    }
#endif

    // compute exception handler into rbx
    __ movq(rarg0, Address(rsp)); 
    __ call_VM_leaf(CAST_FROM_FN_PTR(address, 
                         SharedRuntime::exception_handler_for_return_address),
                    rarg0);
    __ movq(rbx, rax);

    // setup rax & rdx, remove return address & clear pending exception
    __ popq(rdx);
    __ movq(rax, Address(r15_thread, Thread::pending_exception_offset()));
    __ movq(Address(r15_thread, Thread::pending_exception_offset()), (int) NULL);

#ifdef ASSERT
    // make sure exception is set
    { 
      Label L;
      __ testq(rax, rax);
      __ jcc(Assembler::notEqual, L);
      __ stop("StubRoutines::forward exception: no pending exception (2)");
      __ bind(L);
    }
#endif

    // continue at exception handler (return address removed)
    // rax: exception
    // rbx: exception handler
    // rdx: throwing pc
    __ verify_oop(rax);
    __ jmp(rbx);

    return start;
  }

  // Support for jint atomic::xchg(jint exchange_value, volatile jint* dest)
  // 
  // Arguments :
  //    rarg0: exchange_value
  //    rarg0: dest
  // 
  // Result:
  //    *dest <- ex, return (orig *dest)
  address generate_atomic_xchg() 
  {
    StubCodeMark mark(this, "StubRoutines", "atomic_xchg");
    address start = __ pc();

    __ movl(rax, rarg0); // Copy to eax we need a return value anyhow
    __ xchgl(rax, Address(rarg1)); // automatic LOCK
    __ ret(0);

    return start;
  }

  // Support for intptr_t atomic::xchg_ptr(intptr_t exchange_value, volatile intptr_t* dest)
  // 
  // Arguments :
  //    rarg0: exchange_value
  //    rarg1: dest
  // 
  // Result:
  //    *dest <- ex, return (orig *dest)
  address generate_atomic_xchg_ptr() 
  {
    StubCodeMark mark(this, "StubRoutines", "atomic_xchg_ptr");
    address start = __ pc();

    __ movq(rax, rarg0); // Copy to eax we need a return value anyhow
    __ xchgq(rax, Address(rarg1)); // automatic LOCK
    __ ret(0);

    return start;
  }

  // Support for jint atomic::atomic_cmpxchg(jint exchange_value, volatile jint* dest,
  //                                         jint compare_value)
  // 
  // Arguments :
  //    rarg0: exchange_value
  //    rarg1: dest
  //    rarg2: compare_value
  // 
  // Result:
  //    if ( compare_value == *dest ) { 
  //       *dest = exchange_value
  //       return compare_value;
  //    else 
  //       return *dest;
  address generate_atomic_cmpxchg() 
  {
    StubCodeMark mark(this, "StubRoutines", "atomic_cmpxchg");
    address start = __ pc();

    __ movl(rax, rarg2);
   if ( os::is_MP() ) __ lock();
    __ cmpxchgl(rarg0, Address(rarg1)); 
    __ ret(0);

    return start;
  }

  // Support for jint atomic::atomic_cmpxchg_long(jlong exchange_value, 
  //                                             volatile jlong* dest,
  //                                             jlong compare_value)
  // Arguments :
  //    rarg0: exchange_value
  //    rarg1: dest
  //    rarg2: compare_value
  // 
  // Result:
  //    if ( compare_value == *dest ) { 
  //       *dest = exchange_value
  //       return compare_value;
  //    else 
  //       return *dest;
  address generate_atomic_cmpxchg_long() 
  {
    StubCodeMark mark(this, "StubRoutines", "atomic_cmpxchg_long");
    address start = __ pc();

    __ movq(rax, rarg2);
   if ( os::is_MP() ) __ lock();
    __ cmpxchgq(rarg0, Address(rarg1)); 
    __ ret(0);

    return start;
  }

  // Support for jint atomic::add(jint add_value, volatile jint* dest)
  // 
  // Arguments :
  //    rarg0: add_value
  //    rarg1: dest
  // 
  // Result:
  //    *dest += add_value
  //    return *dest;
  address generate_atomic_add() 
  {
    StubCodeMark mark(this, "StubRoutines", "atomic_add");
    address start = __ pc();

    __ movl(rax, rarg0); 
   if ( os::is_MP() ) __ lock();
    __ xaddl(Address(rarg1), rarg0); 
    __ addl(rax, rarg0);
    __ ret(0);

    return start;
  }

  // Support for intptr_t atomic::add_ptr(intptr_t add_value, volatile intptr_t* dest)
  // 
  // Arguments :
  //    rarg0: add_value
  //    rarg1: dest
  // 
  // Result:
  //    *dest += add_value
  //    return *dest;
  address generate_atomic_add_ptr() 
  {
    StubCodeMark mark(this, "StubRoutines", "atomic_add_ptr");
    address start = __ pc();

    __ movq(rax, rarg0); // Copy to eax we need a return value anyhow
   if ( os::is_MP() ) __ lock();
    __ xaddl(Address(rarg1), rarg0); 
    __ addl(rax, rarg0);
    __ ret(0);

    return start;
  }

  // Support for intptr_t OrderAccess::fence()
  // 
  // Arguments :
  // 
  // Result:
  address generate_orderaccess_fence() 
  {
    StubCodeMark mark(this, "StubRoutines", "orderaccess_fence");
    address start = __ pc();
    __ mfence();
    __ ret(0);

    return start;
  }

  // Support for intptr_t get_previous_fp()
  //
  // This routine is used to find the previous frame pointer for the
  // caller (current_frame_guess). This is used as part of debugging
  // ps() is seemingly lost trying to find frames.
  // This code assumes that caller current_frame_guess) has a frame.
  address generate_get_previous_fp() 
  {
    StubCodeMark mark(this, "StubRoutines", "get_previous_fp");
    const Address old_fp(rbp, 0);
    const Address older_fp(rax, 0);
    address start = __ pc();

    __ enter();    
    __ movq(rax, old_fp); // callers fp
    __ movq(rax, older_fp); // the frame for ps()
    __ popq(rbp);
    __ ret(0);

    return start;
  }
  
  address generate_f2i_fixup()
  {
    StubCodeMark mark(this, "StubRoutines", "f2i_fixup");
    Address inout(rsp, 5 * wordSize); // return address + 4 saves

    address start = __ pc();

    Label L;

    __ pushq(rax);
    __ pushq(rarg3);
    __ pushq(rarg2);
    __ pushq(rarg1);

    __ movl(rax, 0x7f800000);
    __ xorl(rarg3, rarg3);
    __ movl(rarg2, inout);
    __ movl(rarg1, rarg2);
    __ andl(rarg1, 0x7fffffff);
    __ cmpl(rax, rarg1); // NaN? -> 0
    __ jcc(Assembler::negative, L);
    __ testl(rarg2, rarg2); // signed ? min_jint : max_jint
    __ movl(rarg3, 0x80000000);
    __ movl(rax, 0x7fffffff);
    __ cmovl(Assembler::positive, rarg3, rax);

    __ bind(L);
    __ movq(inout, rarg3);

    __ popq(rarg1);
    __ popq(rarg2);
    __ popq(rarg3);
    __ popq(rax);

    __ ret(0);

    return start;
  }

  address generate_f2l_fixup()
  {
    StubCodeMark mark(this, "StubRoutines", "f2l_fixup");
    Address inout(rsp, 5 * wordSize); // return address + 4 saves
    address start = __ pc();

    Label L;

    __ pushq(rax);
    __ pushq(rarg3);
    __ pushq(rarg2);
    __ pushq(rarg1);

    __ movl(rax, 0x7f800000);
    __ xorl(rarg3, rarg3);
    __ movl(rarg2, inout);
    __ movl(rarg1, rarg2);
    __ andl(rarg1, 0x7fffffff);
    __ cmpl(rax, rarg1); // NaN? -> 0
    __ jcc(Assembler::negative, L);
    __ testl(rarg2, rarg2); // signed ? min_jlong : max_jlong
    __ movq(rarg3, 0x8000000000000000);
    __ movq(rax, 0x7fffffffffffffff);
    __ cmovq(Assembler::positive, rarg3, rax);

    __ bind(L);
    __ movq(inout, rarg3);

    __ popq(rarg1);
    __ popq(rarg2);
    __ popq(rarg3);
    __ popq(rax);

    __ ret(0);

    return start;
  }

  address generate_d2i_fixup()
  {
    StubCodeMark mark(this, "StubRoutines", "d2i_fixup");
    Address inout(rsp, 6 * wordSize); // return address + 5 saves

    address start = __ pc();

    Label L;

    __ pushq(rax);
    __ pushq(rarg3);
    __ pushq(rarg2);
    __ pushq(rarg1);
    __ pushq(rarg0);

    __ movl(rax, 0x7ff00000);
    __ movq(rarg2, inout);
    __ movl(rarg3, rarg2);
    __ movq(rarg1, rarg2);
    __ movq(rarg0, rarg2);
    __ negl(rarg3);
    __ shrq(rarg1, 0x20);
    __ orl(rarg3, rarg2);
    __ andl(rarg1, 0x7fffffff);
    __ xorl(rarg2, rarg2);
    __ shrl(rarg3, 0x1f);
    __ orl(rarg1, rarg3);
    __ cmpl(rax, rarg1);
    __ jcc(Assembler::negative, L); // NaN -> 0
    __ testq(rarg0, rarg0); // signed ? min_jint : max_jint
    __ movl(rarg2, 0x80000000);
    __ movl(rax, 0x7fffffff);
    __ cmovl(Assembler::positive, rarg2, rax);
    
    __ bind(L);
    __ movq(inout, rarg2);

    __ popq(rarg0);
    __ popq(rarg1);
    __ popq(rarg2);
    __ popq(rarg3);
    __ popq(rax);

    __ ret(0);

    return start;
  }

  address generate_d2l_fixup()
  {
    StubCodeMark mark(this, "StubRoutines", "d2l_fixup");
    Address inout(rsp, 6 * wordSize); // return address + 5 saves

    address start = __ pc();

    Label L;

    __ pushq(rax);
    __ pushq(rarg3);
    __ pushq(rarg2);
    __ pushq(rarg1);
    __ pushq(rarg0);

    __ movl(rax, 0x7ff00000);
    __ movq(rarg2, inout);
    __ movl(rarg3, rarg2);
    __ movq(rarg1, rarg2);
    __ movq(rarg0, rarg2);
    __ negl(rarg3);
    __ shrq(rarg1, 0x20);
    __ orl(rarg3, rarg2);
    __ andl(rarg1, 0x7fffffff);
    __ xorl(rarg2, rarg2);
    __ shrl(rarg3, 0x1f);
    __ orl(rarg1, rarg3);
    __ cmpl(rax, rarg1);
    __ jcc(Assembler::negative, L); // NaN -> 0
    __ testq(rarg0, rarg0); // signed ? min_jlong : max_jlong
    __ movq(rarg2, 0x8000000000000000);
    __ movq(rax, 0x7fffffffffffffff);
    __ cmovq(Assembler::positive, rarg2, rax);
    
    __ bind(L);
    __ movq(inout, rarg2);

    __ popq(rarg0);
    __ popq(rarg1);
    __ popq(rarg2);
    __ popq(rarg3);
    __ popq(rax);

    __ ret(0);

    return start;
  }

  address generate_fp_mask(const char *stub_name, int64 mask)
  {
    StubCodeMark mark(this, "StubRoutines", stub_name);

    __ align(16);
    address start = __ pc();

    __ emit_data64( mask, relocInfo::none );
    __ emit_data64( mask, relocInfo::none );

    return start;
  }

  // The following routine generates a subroutine to throw an
  // asynchronous UnknownError when an unsafe access gets a fault that
  // could not be reasonably prevented by the programmer.  (Example:
  // SIGBUS/OBJERR.)
  address generate_handler_for_unsafe_access() 
  {
    StubCodeMark mark(this, "StubRoutines", "handler_for_unsafe_access");
    address start = __ pc();

    __ pushq(0);                      // hole for return address-to-be
    __ pushaq();                      // push registers
    Address next_pc(rsp, RegisterImpl::number_of_registers * BytesPerWord);

    __ subq(rsp, frame::arg_reg_save_area_bytes);
    __ call(CAST_FROM_FN_PTR(address, handle_unsafe_access),
            relocInfo::runtime_call_type);
    __ addq(rsp, frame::arg_reg_save_area_bytes);

    __ movq(next_pc, rax);            // stuff next address 
    __ popaq();
    __ ret(0);                        // jump to next address

    return start;
  }

  // Non-destructive plausibility checks for oops
  //
  // Arguments:
  //    all args on stack!
  // 
  // Stack after saving rarg3:
  //    [tos + 0]: saved rarg3
  //    [tos + 1]: saved rarg2
  //    [tos + 2]: saved flags
  //    [tos + 3]: return address
  //  * [tos + 4]: error message (char*)
  //  * [tos + 5]: object to verify (oop)
  //  * [tos + 6]: saved rax - saved by caller and bashed
  //  * = popped on exit
  address generate_verify_oop() 
  {
    StubCodeMark mark(this, "StubRoutines", "verify_oop");
    address start = __ pc();
    
    Label exit, error;

    __ pushfq();
    __ incl(Address((address) StubRoutines::verify_oop_count_addr(),
                    relocInfo::none));

    // save rarg2 and rarg3
    __ pushq(rarg2);
    __ pushq(rarg3);

    // get object
    __ movq(rax, Address(rsp, 5 * wordSize));

    // make sure object is 'reasonable'
    __ testq(rax, rax);
    __ jcc(Assembler::zero, exit); // if obj is NULL it is OK
    // Check if the oop is in the right area of memory
    __ movq(rarg2, rax);
    __ movq(rarg3, (int64_t) Universe::verify_oop_mask());
    __ andq(rarg2, rarg3);
    __ movq(rarg3, (int64_t) Universe::verify_oop_bits());
    __ cmpq(rarg2, rarg3);
    __ jcc(Assembler::notZero, error);

    // make sure klass is 'reasonable'
    __ movq(rax, Address(rax, oopDesc::klass_offset_in_bytes())); // get klass
    __ testq(rax, rax);
    __ jcc(Assembler::zero, error); // if klass is NULL it is broken
    // Check if the klass is in the right area of memory
    __ movq(rarg2, rax);
    __ movq(rarg3, (int64_t) Universe::verify_klass_mask());
    __ andq(rarg2, rarg3);
    __ movq(rarg3, (int64_t) Universe::verify_klass_bits());
    __ cmpq(rarg2, rarg3);
    __ jcc(Assembler::notZero, error);

    // make sure klass' klass is 'reasonable'
    __ movq(rax, Address(rax, oopDesc::klass_offset_in_bytes()));
    __ testq(rax, rax);
    __ jcc(Assembler::zero, error); // if klass' klass is NULL it is broken
    // Check if the klass' klass is in the right area of memory
    __ movq(rarg3, (int64_t) Universe::verify_klass_mask());
    __ andq(rax, rarg3);
    __ movq(rarg3, (int64_t) Universe::verify_klass_bits());
    __ cmpq(rax, rarg3);
    __ jcc(Assembler::notZero, error);

    // return if everything seems ok
    __ bind(exit);
    __ movq(rax, Address(rsp, 6 * wordSize));    // get saved rax back
    __ popq(rarg3);                              // restore rarg3
    __ popq(rarg2);                              // restore rarg2
    __ popfq();                                  // restore flags
    __ ret(3 * wordSize);                        // pop caller saved stuff

    // handle errors
    __ bind(error);
    __ movq(rax, Address(rsp, 6 * wordSize));    // get saved rax back
    __ popq(rarg3);                              // get saved rarg3 back
    __ popq(rarg2);                              // get saved rarg2 back
    __ popfq();                                  // get saved flags off stack --
                                                 // will be ignored

    __ pushaq();                                 // push registers
                                                 // (rip is already
                                                 // already pushed)
    // debug(char* msg, int64_t regs[])
    // We've popped the registers we'd saved (rarg3, rarg2 and flags), and
    // pushed all the registers, so now the stack looks like:
    //     [tos +  0] 16 saved registers
    //     [tos + 16] return address
    //     [tos + 17] error message (char*)

    __ movq(rarg0, Address(rsp, 17 * wordSize)); // pass address of error message
    __ movq(rarg1, rsp);                         // pass address of regs on stack
    __ movq(r12, rsp);                           // remember rsp
    __ subq(rsp, frame::arg_reg_save_area_bytes);// windows
    __ andq(rsp, -16);                           // align stack as required by ABI
    __ call(CAST_FROM_FN_PTR(address, MacroAssembler::debug), 
            relocInfo::runtime_call_type);
    __ movq(rsp, r12);                           // restore rsp
    __ popaq();                                  // pop registers
    __ ret(3 * wordSize);                        // pop caller saved stuff

    return start;
  }

#undef __
#define __ masm->

  // Continuation point for throwing of implicit exceptions that are
  // not handled in the current activation. Fabricates an exception
  // oop and initiates normal exception dispatching in this
  // frame. Since we need to preserve callee-saved values (currently
  // only for C2, but done for C1 as well) we need a callee-saved oop
  // map and therefore have to make these stubs into RuntimeStubs
  // rather than BufferBlobs.  If the compiler needs all registers to
  // be preserved between the fault point and the exception handler
  // then it must assume responsibility for that in
  // AbstractCompiler::continuation_for_implicit_null_exception or
  // continuation_for_implicit_division_by_zero_exception. All other
  // implicit exceptions (e.g., NullPointerException or
  // AbstractMethodError on entry) are either at call sites or
  // otherwise assume that stack unwinding will be initiated, so
  // caller saved registers were assumed volatile in the compiler.
  //
  // Note: the routine set_pc_not_at_call_for_caller in
  // SharedRuntime.cpp requires that this code be generated into a
  // RuntimeStub.
  address 
  generate_throw_exception(const char* name,
                           address runtime_entry,
                           bool restore_saved_exception_pc)
  {
    // Information about frame layout at time of blocking runtime call.
    // Note that we only have to preserve callee-saved registers since
    // the compilers are responsible for supplying a continuation point
    // if they expect all registers to be preserved.
    enum layout {
      r13_off = frame::arg_reg_save_area_bytes/BytesPerInt,
                  r13_off2,
      r14_off,    r14_off2,
      rbp_off,    rbp_off2,
      framesize, // exclusive of return address
      return_off = framesize,
                  return_off2
    };

    int insts_size = 512;
    int locs_size  = 64;

    CodeBuffer* code  = new CodeBuffer(insts_size, locs_size, 0, 0, 0, false,
                                       NULL, NULL, NULL, false, NULL, name,
                                       false);
    OopMapSet* oop_maps  = new OopMapSet();
    MacroAssembler* masm = new MacroAssembler(code);

    address start = __ pc();

    // This is an inlined and slightly modified version of call_VM
    // which has the ability to fetch the return PC out of
    // thread-local storage and also sets up last_Java_sp slightly
    // differently than the real call_VM
    if (restore_saved_exception_pc) {
      __ movq(rax,
              Address(r15_thread,
                      in_bytes(JavaThread::saved_exception_pc_offset())));
      __ pushq(rax);
    }
      
#ifndef COMPILER2
    __ enter(); // required for proper stackwalking of RuntimeStub frame
#endif COMPILER2

    assert(is_odd(framesize/2), "sp not 16-byte aligned");

    __ subq(rsp, framesize << LogBytesPerInt); // prolog

    __ movq(Address(rsp, rbp_off << LogBytesPerInt), rbp);
    __ movq(Address(rsp, r13_off << LogBytesPerInt), r13);
    __ movq(Address(rsp, r14_off << LogBytesPerInt), r14);

    // Set up last_Java_sp and last_Java_fp
    __ set_last_Java_frame(noreg, rsp, rbp, NULL);

    // Call runtime
    __ movq(rarg0, r15_thread);
    __ call(runtime_entry, relocInfo::runtime_call_type);

    // Generate oop map
    OopMap* map = new OopMap(framesize, 0);

#ifdef COMPILER2
    // SharedInfo is apparently not initialized if -Xint is specified
    if (UseCompiler) {
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
    }
#endif
#ifdef COMPILER1
#error check this
    map->set_callee_saved(OptoReg::Name(SharedInfo::stack0+ebp_off),
                          framesize, 0, OptoReg::Name(ebp->encoding()));
    map->set_callee_saved(OptoReg::Name(SharedInfo::stack0+esi_off),
                          framesize, 0, OptoReg::Name(esi->encoding()));
    map->set_callee_saved(OptoReg::Name(SharedInfo::stack0+edi_off),
                          framesize, 0, OptoReg::Name(edi->encoding()));
#endif

    oop_maps->add_gc_map(__ pc() - start, true, map);

    __ reset_last_Java_frame(noreg, false);

   // restore callee-save registers.  This must be done after
   // resetting the frame
    __ movq(rbp, Address(rsp, rbp_off << LogBytesPerInt));
    __ movq(r13, Address(rsp, r13_off << LogBytesPerInt));
    __ movq(r14, Address(rsp, r14_off << LogBytesPerInt));

    // discard arguments
    __ addq(rsp, framesize << LogBytesPerInt); // epilog

#ifndef COMPILER2
    __ leave(); // required for proper stackwalking of RuntimeStub frame
#endif COMPILER2

    // check for pending exceptions
#ifdef ASSERT
    Label L;
    __ cmpq(Address(r15_thread, Thread::pending_exception_offset()),
            (int) NULL);
    __ jcc(Assembler::notEqual, L);
    __ should_not_reach_here();
    __ bind(L);
#endif ASSERT
    __ jmp(StubRoutines::forward_exception_entry(),
           relocInfo::runtime_call_type);

    // Note: it seems the frame size reported to the RuntimeStub has
    // to be incremented by 1 to account for the return PC. It
    // definitely must be one more than the amount by which SP was
    // decremented.
    int extra_words = 1;
#ifdef COMPILER1
    ++extra_words; // Not strictly necessary since C1 ignores frame
                   // size and uses link
#endif COMPILER1

    RuntimeStub* stub =
      RuntimeStub::new_runtime_stub(name, code,
                                    (framesize >>
                                     (LogBytesPerWord - LogBytesPerInt)) +
                                    extra_words,
                                    oop_maps, false);
    return stub->entry_point();
  }


  // Initialization
  void generate_initial() 
  {
    // Generates all stubs and initializes the entry points

    // This platform-specific stub is needed by generate_call_stub()
    StubRoutines::amd64::_mxcsr_std        = generate_fp_mask("mxcsr_std",        0x0000000000001F80);

    // entry points that exist in all platforms Note: This is code
    // that could be shared among different platforms - however the
    // benefit seems to be smaller than the disadvantage of having a
    // much more complicated generator structure. See also comment in
    // stubRoutines.hpp.

    StubRoutines::_forward_exception_entry = generate_forward_exception();

    StubRoutines::_call_stub_entry = 
      generate_call_stub(StubRoutines::_call_stub_return_address);

    // is referenced by megamorphic call    
    StubRoutines::_catch_exception_entry = generate_catch_exception();    

    // atomic calls
    StubRoutines::_atomic_xchg_entry         = generate_atomic_xchg();
    StubRoutines::_atomic_xchg_ptr_entry     = generate_atomic_xchg_ptr();
    StubRoutines::_atomic_cmpxchg_entry      = generate_atomic_cmpxchg();
    StubRoutines::_atomic_cmpxchg_long_entry = generate_atomic_cmpxchg_long();
    StubRoutines::_atomic_add_entry          = generate_atomic_add();
    StubRoutines::_atomic_add_ptr_entry      = generate_atomic_add_ptr();
    StubRoutines::_fence_entry               = generate_orderaccess_fence();


    // platform dependent
    StubRoutines::amd64::_handler_for_unsafe_access_entry =
      generate_handler_for_unsafe_access();

    StubRoutines::amd64::_get_previous_fp_entry = generate_get_previous_fp();
  }

  void generate_all()
  {
    // Generates all stubs and initializes the entry points
    
    // These entry points require SharedInfo::stack0 to be set up in
    // non-core builds and need to be relocatable, so they each
    // fabricate a RuntimeStub internally.
    StubRoutines::_throw_AbstractMethodError_entry =
      generate_throw_exception("AbstractMethodError throw_exception",
                               CAST_FROM_FN_PTR(address, 
                                                SharedRuntime::
                                                throw_AbstractMethodError),
                               false);

    StubRoutines::_throw_ArithmeticException_entry =
      generate_throw_exception("ArithmeticException throw_exception",
                               CAST_FROM_FN_PTR(address,
                                                SharedRuntime::
                                                throw_ArithmeticException),
                               true);

    StubRoutines::_throw_NullPointerException_entry =
      generate_throw_exception("NullPointerException throw_exception", 
                               CAST_FROM_FN_PTR(address, 
                                                SharedRuntime::
                                                throw_NullPointerException),
                               true);

    StubRoutines::_throw_NullPointerException_at_call_entry =
      generate_throw_exception("NullPointerException at call throw_exception",
                               CAST_FROM_FN_PTR(address,
                                                SharedRuntime::
                                                throw_NullPointerException_at_call),
                               false);

    StubRoutines::_throw_StackOverflowError_entry =
      generate_throw_exception("StackOverflowError throw_exception",
                               CAST_FROM_FN_PTR(address, 
                                                SharedRuntime::
                                                throw_StackOverflowError),
                               false);

    // entry points that are platform specific  
    StubRoutines::amd64::_f2i_fixup = generate_f2i_fixup();
    StubRoutines::amd64::_f2l_fixup = generate_f2l_fixup();
    StubRoutines::amd64::_d2i_fixup = generate_d2i_fixup();
    StubRoutines::amd64::_d2l_fixup = generate_d2l_fixup();

    StubRoutines::amd64::_float_sign_mask  = generate_fp_mask("float_sign_mask",  0x7FFFFFFF7FFFFFFF);
    StubRoutines::amd64::_float_sign_flip  = generate_fp_mask("float_sign_flip",  0x8000000080000000);
    StubRoutines::amd64::_double_sign_mask = generate_fp_mask("double_sign_mask", 0x7FFFFFFFFFFFFFFF);
    StubRoutines::amd64::_double_sign_flip = generate_fp_mask("double_sign_flip", 0x8000000000000000);

    // support for verify_oop (must happen after universe_init)
    StubRoutines::_verify_oop_subroutine_entry = generate_verify_oop();
  }


 public:
  StubGenerator(CodeBuffer* code, bool all) : StubCodeGenerator(code) { 
    if (all) {
      generate_all();
    } else {
      generate_initial();
    }
  }
}; // end class declaration


void StubGenerator_generate(CodeBuffer* code, bool all) 
{
  StubGenerator g(code, all);
}
