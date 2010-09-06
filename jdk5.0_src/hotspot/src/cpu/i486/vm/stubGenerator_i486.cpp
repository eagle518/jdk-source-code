#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)stubGenerator_i486.cpp	1.50 04/04/04 23:33:29 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_stubGenerator_i486.cpp.incl"

// Declaration and definition of StubGenerator (no .hpp file).
// For a more detailed description of the stub routine structure
// see the comment in stubRoutines.hpp

#define __ _masm->

// -------------------------------------------------------------------------------------------------------------------------
// Stub Code definitions

static address handle_unsafe_access() {
  JavaThread* thread = JavaThread::current();
  address pc  = thread->saved_exception_pc();
  // pc is the instruction which we must emulate
  // doing a no-op is fine:  return garbage from the load
  // therefore, compute npc
  address npc = Assembler::locate_next_instruction(pc);

  // request an async exception
  thread->set_pending_unsafe_access_error();

  // return address of next instruction to execute
  return npc;
}

class StubGenerator: public StubCodeGenerator {
 private:

  //------------------------------------------------------------------------------------------------------------------------
  // Call stubs are used to call Java from C
  //
  //    [ return_from_Java     ] <--- esp
  //    [ argument word n      ]
  //      ...
  // -4 [ argument word 1      ]
  // -3 [ saved ebx            ] <--- esp_after_call
  // -2 [ saved esi            ]
  // -1 [ saved edi            ]
  //  0 [ saved ebp            ] <--- ebp
  //  1 [ return address       ]
  //  2 [ ptr. to call wrapper ]
  //  3 [ result               ]
  //  4 [ result_type          ]
  //  5 [ method               ]
  //  6 [ entry_point          ]
  //  7 [ parameters           ]
  //  8 [ parameter_size       ]
  //  9 [ thread               ]


  address generate_call_stub(address& return_address) {
    StubCodeMark mark(this, "StubRoutines", "call_stub");
    address start = __ pc();

    // stub code parameters / addresses
    assert(frame::entry_frame_call_wrapper_offset == 2, "adjust this code");
    bool  sse_save = false;
    const Address esp_after_call(ebp, -4 * wordSize); // same as in generate_catch_exception()!
    const Address mxcsr_save    (ebp, -4 * wordSize);
    const Address result        (ebp,  3 * wordSize);
    const Address result_type   (ebp,  4 * wordSize);
    const Address method        (ebp,  5 * wordSize);
    const Address entry_point   (ebp,  6 * wordSize);
    const Address parameters    (ebp,  7 * wordSize);
    const Address parameter_size(ebp,  8 * wordSize);
    const Address thread        (ebp,  9 * wordSize); // same as in generate_catch_exception()!
#ifdef COMPILER2
    sse_save =  VM_Version::supports_sse();
#endif

    // stub code
    __ enter();    

    // save edi, esi, & ebx, according to C calling conventions
    __ pushl(edi);
    __ pushl(esi);
    __ pushl(ebx);
    __ subl(esp, wordSize);  // space for %mxcsr save
    // save and initialize %mxcsr
    if (sse_save) {
      __ stmxcsr(mxcsr_save);
      __ ldmxcsr(Address((int) StubRoutines::addr_mxcsr_std(), relocInfo::none));
    }

#ifdef ASSERT
    // make sure we have no pending exceptions
    { Label L;
      __ movl(ecx, thread);
      __ cmpl(Address(ecx, Thread::pending_exception_offset()), (int)NULL);
      __ jcc(Assembler::equal, L);
      __ stop("StubRoutines::call_stub: entered with pending exception");
      __ bind(L);
    }
#endif

    // pass parameters if any
    Label parameters_done;
    __ movl(ecx, parameter_size);  // parameter counter
    __ testl(ecx, ecx);
    __ jcc(Assembler::zero, parameters_done);

    // parameter passing loop

    Label loop;
    __ movl(edx, parameters);	       // parameter pointer
    __ movl(esi, ecx);                 // parameter counter is in esi now
    __ movl(ecx,  Address(edx));       // get first parameter in case it is a receiver

    __ bind(loop);
    __ movl(eax, Address(edx));	       // get parameter
    __ addl(edx, wordSize);            // advance to next parameter
    __ decl(esi);                      // decrement counter
    __ pushl(eax);                     // pass parameter
    __ jcc(Assembler::notZero, loop);

    // call Java function
    __ bind(parameters_done);
    __ movl(ebx, method);              // get methodOop
    __ movl(esi, entry_point);         // get entry_point
    __ call(esi, relocInfo::none);
    return_address = __ pc();

    // store result depending on type
    // (everything that is not T_LONG, T_FLOAT or T_DOUBLE is treated as T_INT)
    __ movl(edi, result);
    Label is_long, is_float, is_double, exit;
    __ movl(esi, result_type);
    __ cmpl(esi, T_LONG);
    __ jcc(Assembler::equal, is_long);
    __ cmpl(esi, T_FLOAT);
    __ jcc(Assembler::equal, is_float);
    __ cmpl(esi, T_DOUBLE);
    __ jcc(Assembler::equal, is_double);

    // handle T_INT case
    __ movl(Address(edi), eax);
    __ bind(exit);

    // pop parameters
    __ movl(ecx, parameter_size);
    __ leal(esp, Address(esp, ecx, Address::times_4));

    // check if parameters have been popped correctly
#ifdef ASSERT
      Label esp_wrong;
      __ leal(edi, esp_after_call);
      __ cmpl(esp, edi);
      __ jcc(Assembler::notEqual, esp_wrong);
#endif

    // restore %mxcsr
    if (sse_save) {
      __ ldmxcsr(mxcsr_save);
    }
    // restore edi & esi
    __ addl(esp, wordSize);  // remove %mxcsr save area
    __ popl(ebx);
    __ popl(esi);
    __ popl(edi);    

    // return
    __ popl(ebp);
    __ ret(0);

    // handle return types different from T_INT
    __ bind(is_long);
    __ movl(Address(edi, 0 * wordSize), eax);
    __ movl(Address(edi, 1 * wordSize), edx);
    __ jmp(exit);

    __ bind(is_float);
    __ fstp_s(Address(edi));
    __ jmp(exit);

    __ bind(is_double);
    __ fstp_d(Address(edi));
    __ jmp(exit);

#ifdef ASSERT
      // stack pointer misadjusted
      __ bind(esp_wrong);
      __ stop("esp wrong after Java call");
#endif

    return start;
  }


  //------------------------------------------------------------------------------------------------------------------------
  // Return point for a Java call if there's an exception thrown in Java code.
  // The exception is caught and transformed into a pending exception stored in
  // JavaThread that can be tested from within the VM.
  //
  // Note: Usually the parameters are removed by the callee. In case of an exception
  //       crossing an activation frame boundary, that is not the case if the callee
  //       is compiled code => need to setup the esp.
  //
  // eax: exception oop

  address generate_catch_exception() {
    StubCodeMark mark(this, "StubRoutines", "catch_exception");
    const Address esp_after_call(ebp, -4 * wordSize); // same as in generate_call_stub()!
    const Address thread        (ebp,  9 * wordSize); // same as in generate_call_stub()!
    address start = __ pc();

    // get thread directly
    __ movl(ecx, thread);
#ifdef ASSERT
    // verify that threads correspond
    { Label L;
      __ get_thread(ebx);
      __ cmpl(ebx, ecx);
      __ jcc(Assembler::equal, L);
      __ stop("StubRoutines::catch_exception: threads must correspond");
      __ bind(L);
    }
#endif
    // set pending exception
    __ verify_oop(eax);
    __ movl(Address(ecx, Thread::pending_exception_offset()), eax          );
    __ movl(Address(ecx, Thread::exception_file_offset   ()), (int)__FILE__);
    __ movl(Address(ecx, Thread::exception_line_offset   ()),      __LINE__);
    // complete return to VM
    assert(StubRoutines::_call_stub_return_address != NULL, "_call_stub_return_address must have been generated before");
    __ jmp(StubRoutines::_call_stub_return_address, relocInfo::none);

    return start;
  }

  
  //------------------------------------------------------------------------------------------------------------------------
  // Continuation point for runtime calls returning with a pending exception.
  // The pending exception check happened in the runtime or native call stub.
  // The pending exception in Thread is converted into a Java-level exception.
  //
  // Contract with Java-level exception handlers:
  // eax: exception
  // edx: throwing pc
  //
  // NOTE: At entry of this stub, exception-pc must be on stack !!

  address generate_forward_exception() {
    StubCodeMark mark(this, "StubRoutines", "forward exception");
    address start = __ pc();

    // Upon entry, the sp points to the return address returning into Java
    // (interpreted or compiled) code; i.e., the return address becomes the
    // throwing pc.
    //
    // Arguments pushed before the runtime call are still on the stack but
    // the exception handler will reset the stack pointer -> ignore them.
    // A potential result in registers can be ignored as well.

#ifdef ASSERT
    // make sure this code is only executed if there is a pending exception
    { Label L;
      __ get_thread(ecx);
      __ cmpl(Address(ecx, Thread::pending_exception_offset()), (int)NULL);
      __ jcc(Assembler::notEqual, L);
      __ stop("StubRoutines::forward exception: no pending exception (1)");
      __ bind(L);
    }
#endif

    // compute exception handler into ebx
    __ movl(eax, Address(esp));
    __ call_VM_leaf(CAST_FROM_FN_PTR(address, SharedRuntime::exception_handler_for_return_address), eax);
    __ movl(ebx, eax);

    // setup eax & edx, remove return address & clear pending exception
    __ get_thread(ecx);
    __ popl(edx);
    __ movl(eax, Address(ecx, Thread::pending_exception_offset()));
    __ movl(Address(ecx, Thread::pending_exception_offset()), (int)NULL);

#ifdef ASSERT
    // make sure exception is set
    { Label L;
      __ testl(eax, eax);
      __ jcc(Assembler::notEqual, L);
      __ stop("StubRoutines::forward exception: no pending exception (2)");
      __ bind(L);
    }
#endif

    // continue at exception handler (return address removed)
    // eax: exception
    // ebx: exception handler
    // edx: throwing pc
    __ verify_oop(eax);
    __ jmp(ebx);

    return start;
  }
  

  //----------------------------------------------------------------------------------------------------
  // Support for jint Atomic::xchg(jint exchange_value, volatile jint* dest)
  // 
  // xchg exists as far back as 8086, lock needed for MP only
  // Stack layout immediately after call:
  //
  // 0 [ret addr ] <--- esp
  // 1 [  ex     ]
  // 2 [  dest   ] 
  //
  // Result:   *dest <- ex, return (old *dest)
  //
  // Note: win32 does not currently use this code

  address generate_atomic_xchg() {
    StubCodeMark mark(this, "StubRoutines", "atomic_xchg");
    address start = __ pc();

    __ pushl(edx);
    Address exchange(esp, 2 * wordSize);
    Address dest_addr(esp, 3 * wordSize);
    __ movl(eax, exchange);
    __ movl(edx, dest_addr);    
    __ xchg(eax, Address(edx, 0));
    __ popl(edx);
    __ ret(0);

    return start;
  }


  //----------------------------------------------------------------------------------------------------
  // Support for jint get_previous_fp()
  // 
  // This routine is used to find the previous frame pointer for
  // the caller (current_frame_guess). This is used as part of debugging
  // ps() is seemingly lost trying to find frames.
  // This code assumes that caller current_frame_guess) has a frame.

  address generate_get_previous_fp() {
    StubCodeMark mark(this, "StubRoutines", "get_previous_fp");
    const Address old_fp       (ebp,  0);
    const Address older_fp       (eax,  0);
    address start = __ pc();

    __ enter();    
    __ movl(eax, old_fp); // callers fp
    __ movl(eax, older_fp); // the frame for ps()
    __ popl(ebp);
    __ ret(0);

    return start;
  }
  

  //---------------------------------------------------------------------------
  // Wrapper for slow-case handling of double-to-integer conversion 
  // d2i or f2i fast case failed either because it is nan or because
  // of under/overflow.
  // Input:  FPU TOS: float value
  // Output: eax (edx): integer (long) result

  address generate_d2i_wrapper( address fcn ) {
    StubCodeMark mark(this, "StubRoutines", "d2i_wrapper");
    address start = __ pc();

  // Capture info about frame layout
  enum layout { FPUState_off         = 0,
                ebp_off              = FPUStateSizeInWords,
                edi_off,         
                esi_off,
                ecx_off,
                ebx_off,
                saved_argument_off,
                saved_argument_off2, // 2nd half of double
	        framesize 
  };

  assert(FPUStateSizeInWords == 27, "update stack layout");

    // Save outgoing argument to stack across push_FPU_state()
    __ subl(esp, wordSize * 2);
    __ fstp_d(Address(esp));

    // Save CPU & FPU state
    __ pushl(ebx);
    __ pushl(ecx);
    __ pushl(esi);
    __ pushl(edi);
    __ pushl(ebp);
    __ push_FPU_state();

    // push_FPU_state() resets the FP top of stack 
    // Load original double into FP top of stack
    __ fld_d(Address(esp, saved_argument_off * wordSize));
    // Store double into stack as outgoing argument
    __ subl(esp, wordSize*2);
    __ fst_d(Address(esp));

    // Prepare FPU for doing math in C-land
    __ empty_FPU_stack();
    // Call the C code to massage the double.  Result in EAX
    __ call_VM_leaf( fcn, 2 );

    // Restore CPU & FPU state
    __ pop_FPU_state();
    __ popl(ebp);
    __ popl(edi);
    __ popl(esi);
    __ popl(ecx);
    __ popl(ebx);
    __ addl(esp, wordSize * 2);

    __ ret(0);

    return start;
  }


  //---------------------------------------------------------------------------
  // The following routine generates a subroutine to throw an asynchronous
  // UnknownError when an unsafe access gets a fault that could not be
  // reasonably prevented by the programmer.  (Example: SIGBUS/OBJERR.)
  address generate_handler_for_unsafe_access() {
    StubCodeMark mark(this, "StubRoutines", "handler_for_unsafe_access");
    address start = __ pc();

    __ pushl(0);                      // hole for return address-to-be
    __ pushad();                      // push registers
    Address next_pc(esp, RegisterImpl::number_of_registers * BytesPerWord);
    __ call(CAST_FROM_FN_PTR(address, handle_unsafe_access), relocInfo::runtime_call_type);
    __ movl(next_pc, eax);            // stuff next address 
    __ popad();
    __ ret(0);                        // jump to next address

    return start;
  }


  //----------------------------------------------------------------------------------------------------
  // Non-destructive plausibility checks for oops
  
  address generate_verify_oop() {
    StubCodeMark mark(this, "StubRoutines", "verify_oop");
    address start = __ pc();
    
    // Incoming arguments on stack after saving eax:
    //
    // [tos    ]: saved edx
    // [tos + 1]: saved EFLAGS
    // [tos + 2]: return address
    // [tos + 3]: char* error message
    // [tos + 4]: oop   object to verify
    // [tos + 5]: saved eax - saved by caller and bashed
    
    Label exit, error;
    __ pushfd();
    __ incl(Address((int)StubRoutines::verify_oop_count_addr(), relocInfo::none));
    __ pushl(edx);                               // save edx
    // make sure object is 'reasonable'
    __ movl(eax, Address(esp, 4 * wordSize));    // get object
    __ testl(eax, eax);
    __ jcc(Assembler::zero, exit);               // if obj is NULL it is ok
    
    // Check if the oop is in the right area of memory
    const int oop_mask = Universe::verify_oop_mask();
    const int oop_bits = Universe::verify_oop_bits();
    __ movl(edx, eax);
    __ andl(edx, oop_mask);
    __ cmpl(edx, oop_bits);
    __ jcc(Assembler::notZero, error);

    // make sure klass is 'reasonable'
    __ movl(eax, Address(eax, oopDesc::klass_offset_in_bytes())); // get klass
    __ testl(eax, eax);
    __ jcc(Assembler::zero, error);              // if klass is NULL it is broken

    // Check if the klass is in the right area of memory
    const int klass_mask = Universe::verify_klass_mask();
    const int klass_bits = Universe::verify_klass_bits();
    __ movl(edx, eax);
    __ andl(edx, klass_mask);
    __ cmpl(edx, klass_bits);
    __ jcc(Assembler::notZero, error);

    // make sure klass' klass is 'reasonable'
    __ movl(eax, Address(eax, oopDesc::klass_offset_in_bytes())); // get klass' klass
    __ testl(eax, eax);
    __ jcc(Assembler::zero, error);              // if klass' klass is NULL it is broken

    __ movl(edx, eax);
    __ andl(edx, klass_mask);
    __ cmpl(edx, klass_bits);
    __ jcc(Assembler::notZero, error);           // if klass not in right area
                                                 // of memory it is broken too.

    // return if everything seems ok
    __ bind(exit);
    __ movl(eax, Address(esp, 5 * wordSize));    // get saved eax back
    __ popl(edx);                                // restore edx
    __ popfd();                                  // restore EFLAGS
    __ ret(3 * wordSize);                        // pop arguments

    // handle errors
    __ bind(error);
    __ movl(eax, Address(esp, 5 * wordSize));    // get saved eax back
    __ popl(edx);                                // get saved edx back
    __ popfd();                                  // get saved EFLAGS off stack -- will be ignored
    __ pushad();                                 // push registers (eip = return address & msg are already pushed)
    __ call(CAST_FROM_FN_PTR(address, MacroAssembler::debug), relocInfo::runtime_call_type);
    __ popad();
    __ ret(3 * wordSize);                        // pop arguments
    return start;
  }


 public:
  // Information about frame layout at time of blocking runtime call.
  // Note that we only have to preserve callee-saved registers since
  // the compilers are responsible for supplying a continuation point
  // if they expect all registers to be preserved.
  enum layout {
    thread_off,    // last_java_sp                
    xmm6_off,      // callee saved register      sp + 1
    xmm6_off_2,    // callee saved register      sp + 2
    xmm7_off,      // callee saved register      sp + 3
    xmm7_off_2,    // callee saved register      sp + 4
    edi_off,       // callee saved register      sp + 5
    esi_off,       // callee saved register      sp + 6
    ebp_off,       // callee saved register      sp + 7
    framesize
  };

 private:

#undef  __
#define __ masm->

  //------------------------------------------------------------------------------------------------------------------------
  // Continuation point for throwing of implicit exceptions that are not handled in
  // the current activation. Fabricates an exception oop and initiates normal
  // exception dispatching in this frame. Since we need to preserve callee-saved values
  // (currently only for C2, but done for C1 as well) we need a callee-saved oop map and
  // therefore have to make these stubs into RuntimeStubs rather than BufferBlobs.
  // If the compiler needs all registers to be preserved between the fault
  // point and the exception handler then it must assume responsibility for that in
  // AbstractCompiler::continuation_for_implicit_null_exception or
  // continuation_for_implicit_division_by_zero_exception. All other implicit
  // exceptions (e.g., NullPointerException or AbstractMethodError on entry) are
  // either at call sites or otherwise assume that stack unwinding will be initiated,
  // so caller saved registers were assumed volatile in the compiler.
  //
  // Note: the routine set_pc_not_at_call_for_caller in SharedRuntime.cpp requires
  // that this code be generated into a RuntimeStub.
  address StubGenerator::generate_throw_exception(const char* name, address runtime_entry, bool restore_saved_exception_pc) {

    int insts_size = 256;
    int locs_size  = 32;

    CodeBuffer* code     = new CodeBuffer(insts_size, locs_size, 0, 0, 0, false, NULL, NULL, NULL, false, NULL, name, false);
    OopMapSet* oop_maps  = new OopMapSet();
    MacroAssembler* masm = new MacroAssembler(code);

    address start = __ pc();

    // This is an inlined and slightly modified version of call_VM
    // which has the ability to fetch the return PC out of
    // thread-local storage and also sets up last_Java_sp slightly
    // differently than the real call_VM
    Register java_thread = ebx;
    __ get_thread(java_thread);
    if (restore_saved_exception_pc) {
      __ movl(eax, Address(java_thread, in_bytes(JavaThread::saved_exception_pc_offset())));
      __ pushl(eax);
    }
      
#ifndef COMPILER2
    __ enter(); // required for proper stackwalking of RuntimeStub frame
#endif COMPILER2

    __ subl(esp, framesize * wordSize); // prolog

#ifdef COMPILER2
    if( OptoRuntimeCalleeSavedFloats ) {
      if( UseSSE == 1 ) {
        __ movss(Address(esp,xmm6_off*wordSize),xmm6);
        __ movss(Address(esp,xmm7_off*wordSize),xmm7);
      } else if( UseSSE == 2 ) {
        __ movsd(Address(esp,xmm6_off*wordSize),xmm6);
        __ movsd(Address(esp,xmm7_off*wordSize),xmm7);
      }
    }
#endif /* COMPILER2 */
    __ movl(Address(esp, ebp_off * wordSize), ebp);
    __ movl(Address(esp, edi_off * wordSize), edi);
    __ movl(Address(esp, esi_off * wordSize), esi);

    // push java thread (becomes first argument of C function)
    __ movl(Address(esp, thread_off * wordSize), java_thread);

    // Set up last_Java_sp and last_Java_fp
    __ set_last_Java_frame(java_thread, esp, ebp, NULL);

    // Call runtime
    __ call(runtime_entry, relocInfo::runtime_call_type);
    // Generate oop map
    OopMap* map =  new OopMap(framesize, 0);        
#ifdef COMPILER2
    // SharedInfo is apparently not initialized if -Xint is specified
    if (UseCompiler) {
      map->set_callee_saved(SharedInfo::stack2reg(ebp_off), framesize, 0, OptoReg::Name(EBP_num));
      map->set_callee_saved(SharedInfo::stack2reg(edi_off), framesize, 0, OptoReg::Name(EDI_num));
      map->set_callee_saved(SharedInfo::stack2reg(esi_off), framesize, 0, OptoReg::Name(ESI_num));
      if( OptoRuntimeCalleeSavedFloats ) {
        map->set_callee_saved(SharedInfo::stack2reg(xmm6_off  ), framesize, 0, OptoReg::Name(XMM6a_num));
        map->set_callee_saved(SharedInfo::stack2reg(xmm6_off+1), framesize, 0, OptoReg::Name(XMM6b_num));
        map->set_callee_saved(SharedInfo::stack2reg(xmm7_off  ), framesize, 0, OptoReg::Name(XMM7a_num));
        map->set_callee_saved(SharedInfo::stack2reg(xmm7_off+1), framesize, 0, OptoReg::Name(XMM7b_num));
      }
    }
#endif
#ifdef COMPILER1
    map->set_callee_saved(OptoReg::Name(SharedInfo::stack0+ebp_off), framesize, 0, OptoReg::Name(ebp->encoding()));
    map->set_callee_saved(OptoReg::Name(SharedInfo::stack0+esi_off), framesize, 0, OptoReg::Name(esi->encoding()));
    map->set_callee_saved(OptoReg::Name(SharedInfo::stack0+edi_off), framesize, 0, OptoReg::Name(edi->encoding()));
#endif
    oop_maps->add_gc_map(__ pc() - start, true, map);
      
    // restore the thread (cannot use the pushed argument since arguments
    // may be overwritten by C code generated by an optimizing compiler);
    // however can use the register value directly if it is callee saved.
    __ get_thread(java_thread);

    __ reset_last_Java_frame(java_thread, false);

    // Restore callee save registers.  This must be done after resetting the Java frame
#ifdef COMPILER2
    if( OptoRuntimeCalleeSavedFloats ) {
      if( UseSSE == 1 ) {
        __ movss(xmm6,Address(esp,xmm6_off*wordSize));
        __ movss(xmm7,Address(esp,xmm7_off*wordSize));
      } else if( UseSSE == 2 ) {
        __ movsd(xmm6,Address(esp,xmm6_off*wordSize));
        __ movsd(xmm7,Address(esp,xmm7_off*wordSize));
      }
    }
#endif /* COMPILER2 */
    __ movl(ebp,Address(esp, ebp_off * wordSize));
    __ movl(edi,Address(esp, edi_off * wordSize));
    __ movl(esi,Address(esp, esi_off * wordSize));

    // discard arguments
    __ addl(esp, framesize * wordSize); // epilog

#ifndef COMPILER2
    __ leave(); // required for proper stackwalking of RuntimeStub frame
#endif COMPILER2

    // check for pending exceptions
#ifdef ASSERT
    Label L;
    __ cmpl(Address(java_thread, Thread::pending_exception_offset()), (int)NULL);
    __ jcc(Assembler::notEqual, L);
    __ should_not_reach_here();
    __ bind(L);
#endif ASSERT
    __ jmp(StubRoutines::forward_exception_entry(), relocInfo::runtime_call_type);

    // Note: it seems the frame size reported to the RuntimeStub has
    // to be incremented by 1 to account for the return PC. It
    // definitely must be one more than the amount by which SP was
    // decremented.
    int extra_words = 1;
#ifdef COMPILER1
    ++extra_words; // Not strictly necessary since C1 ignores frame size and uses link
#endif COMPILER1

    RuntimeStub* stub = RuntimeStub::new_runtime_stub(name, code, framesize + extra_words, oop_maps, false);
    return stub->entry_point();
  }


  void create_control_words() {
    // Round to nearest, 53-bit mode, exceptions masked
    StubRoutines::_fpu_cntrl_wrd_std   = 0x027F;
    // Round to zero, 53-bit mode, exception mased
    StubRoutines::_fpu_cntrl_wrd_trunc = 0x0D7F;
    // Round to nearest, 24-bit mode, exceptions masked
    StubRoutines::_fpu_cntrl_wrd_24    = 0x007F;
    // Round to nearest, 64-bit mode, exceptions masked
    StubRoutines::_fpu_cntrl_wrd_64    = 0x037F;
    // Round to nearest, 64-bit mode, exceptions masked
    StubRoutines::_mxcsr_std           = 0x1F80;
    // Note: the following two constants are 80-bit values
    //       layout is critical for correct loading by FPU.
    // Bias for strict fp multiply/divide
    StubRoutines::_fpu_subnormal_bias1[0]= 0x00000000; // 2^(-15360) == 0x03ff 8000 0000 0000 0000
    StubRoutines::_fpu_subnormal_bias1[1]= 0x80000000;
    StubRoutines::_fpu_subnormal_bias1[2]= 0x03ff;
    // Un-Bias for strict fp multiply/divide
    StubRoutines::_fpu_subnormal_bias2[0]= 0x00000000; // 2^(+15360) == 0x7bff 8000 0000 0000 0000
    StubRoutines::_fpu_subnormal_bias2[1]= 0x80000000;
    StubRoutines::_fpu_subnormal_bias2[2]= 0x7bff;
  }

  //---------------------------------------------------------------------------
  // Initialization
  
  void generate_initial() {
    // Generates all stubs and initializes the entry points

    //------------------------------------------------------------------------------------------------------------------------
    // entry points that exist in all platforms
    // Note: This is code that could be shared among different platforms - however the benefit seems to be smaller than
    //       the disadvantage of having a much more complicated generator structure. See also comment in stubRoutines.hpp.
    StubRoutines::_forward_exception_entry                 = generate_forward_exception();    

    StubRoutines::_call_stub_entry                         = generate_call_stub(StubRoutines::_call_stub_return_address);
    // is referenced by megamorphic call    
    StubRoutines::_catch_exception_entry                   = generate_catch_exception();    

    // These are currently used by Solaris/Intel
    StubRoutines::_atomic_xchg_entry                       = generate_atomic_xchg();

    // platform dependent
    StubRoutines::i486::_handler_for_unsafe_access_entry    = generate_handler_for_unsafe_access();
    StubRoutines::i486::_get_previous_fp_entry              = generate_get_previous_fp();

    StubRoutines::_d2i_wrapper                              = generate_d2i_wrapper( CAST_FROM_FN_PTR(address, SharedRuntime::d2i) );
    StubRoutines::_d2l_wrapper                              = generate_d2i_wrapper( CAST_FROM_FN_PTR(address, SharedRuntime::d2l) );
    create_control_words();
  }


  void generate_all() {
    // Generates all stubs and initializes the entry points
    
    // These entry points require SharedInfo::stack0 to be set up in non-core builds
    // and need to be relocatable, so they each fabricate a RuntimeStub internally.
    StubRoutines::_throw_AbstractMethodError_entry         = generate_throw_exception("AbstractMethodError throw_exception",          CAST_FROM_FN_PTR(address, SharedRuntime::throw_AbstractMethodError),  false);
    StubRoutines::_throw_ArithmeticException_entry         = generate_throw_exception("ArithmeticException throw_exception",          CAST_FROM_FN_PTR(address, SharedRuntime::throw_ArithmeticException),  true);
    StubRoutines::_throw_NullPointerException_entry        = generate_throw_exception("NullPointerException throw_exception",         CAST_FROM_FN_PTR(address, SharedRuntime::throw_NullPointerException), true);
    StubRoutines::_throw_NullPointerException_at_call_entry= generate_throw_exception("NullPointerException at call throw_exception", CAST_FROM_FN_PTR(address, SharedRuntime::throw_NullPointerException_at_call), false);
    StubRoutines::_throw_StackOverflowError_entry          = generate_throw_exception("StackOverflowError throw_exception",           CAST_FROM_FN_PTR(address, SharedRuntime::throw_StackOverflowError),   false);

    //------------------------------------------------------------------------------------------------------------------------
    // entry points that are platform specific  

    // support for verify_oop (must happen after universe_init)
    StubRoutines::_verify_oop_subroutine_entry	   = generate_verify_oop();
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


void StubGenerator_generate(CodeBuffer* code, bool all) {
  StubGenerator g(code, all);
}
