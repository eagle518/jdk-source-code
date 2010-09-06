#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)stubGenerator_ia64.cpp	1.57 04/03/08 11:15:08 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_stubGenerator_ia64.cpp.incl"

// Declaration and definition of StubGenerator (no .hpp file).
// For a more detailed description of the stub routine structure
// see the comment in stubRoutines.hpp

#define __ _masm->

// -------------------------------------------------------------------------------------------------------------------------
// Stub Code definitions

extern "C" {
  void ia_64_verify_oop(const char* message, oop o) { 
    uintptr_t x = (uintptr_t) o;
    if ( x & 7) fatal(message);
    if (o != NULL) {
      x = (uintptr_t) o->klass();
      if ( x & 7) fatal(message);
    }
  }
}


class StubGenerator: public StubCodeGenerator {
private:

  //------------------------------------------------------------------------------------------------------------------------
  // Call stubs are used to call Java from C
  //
  // GR_I0 - call wrapper address     : address
  // GR_I1 - result                   : intptr_t*
  // GR_I2 - result type              : BasicType
  // GR_I3 - method                   : methodOop
  // GR_I4 - interpreter entry point  : address
  // GR_I5 - parameter block          : intptr_t*
  // GR_I6 - parameter count in words : int
  // GR_I7 - thread                   : Thread*
  //
  address generate_call_stub(address& return_address) {
    StubCodeMark mark(this, "StubRoutines", "call_stub");

    const Register result     = GR_I1;
    const Register type       = GR_I2;
    const Register method     = GR_I3;
    const Register entry_fd   = GR_I4;
    const Register parms      = GR_I5;
    const Register parm_count = GR_I6;
    const Register thread     = GR_I7;

    const Register parm_size = GR31_SCRATCH;
    const Register entry     = GR30_SCRATCH;
    const Register arg       = GR29_SCRATCH;

    const Register out_tos   = GR49; // Equivalent of GR_Otos
    const Register out_parms = GR50; // Equivalent of GR_Olocals (unused)

    const BranchRegister    entry_br = BR6_SCRATCH;
    const PredicateRegister no_args  = PR6_SCRATCH;

    address start = __ emit_fd();

    __ alloc(GR_Lsave_PFS, 8, 9, 2, 0);                     // save AR_PFS
    __ sxt4(parm_count, parm_count);                        // # of parms
    __ mov(GR_Lsave_SP, SP);                                // save caller's SP
    __ mov(GR_entry_frame_GR5, GR5_poll_page_addr);
    __ mov(GR_entry_frame_GR6, GR6_caller_BSP);
    __ mov(GR_entry_frame_GR7, GR7_reg_stack_limit);

    // We can not tolerate an eager RSE cpu. Itanium-1 & 2 do not support
    // this feature but we turn it off anyway 
    const Register RSC   = GR2_SCRATCH;
    __ mov(RSC, AR_RSC);
    __ and3(RSC, -4, RSC);	// Turn off two low bits
    __ mov(AR_RSC, RSC);        //  enforced lazy mode

    __ shladd(parm_size, parm_count, LogBytesPerWord, GR0); // size of stack space for the parms
    __ ld8(entry, entry_fd, sizeof(address));               // entry point
    __ mov(GR_Lsave_RP, RP);                                // save return address

    __ add(parm_size, parm_size, 15);                       // round up to multiple of 16 bytes.  we use
                                                            // caller's 16-byte scratch area for params,
                                                            // so no need to add 16 to the current frame size.
    __ mov(GR_Lsave_LC, AR_LC);                             // save AR_LC
    __ add(out_parms, SP, BytesPerWord);                    // caller's SP+8 is 1st parm addr == target method locals addr

    __ and3(parm_size, parm_size, -16);
    __ cmp4(PR0, no_args, 0, parm_count, Assembler::less);  // any parms?

    __ mov(GR_entry_frame_GR4, GR4_thread);                 // save GR4_thread: it's a preserved register
    __ sub(SP, SP, parm_size);                              // allocate the space for args + scratch
    __ mov(entry_br, entry);

    __ mov(GR27_method, method);                            // load method
    __ mov(GR4_thread, thread);                             // load thread
    __ sub(parm_count, parm_count, 1);                      // cloop counts down to zero

    // Initialize the register and memory stack limits for stack checking in compiled code
    __ add(GR7_reg_stack_limit, thread_(register_stack_limit));
    __ mov(GR6_caller_BSP, AR_BSP);                         // load register SP
    __ movl(GR5_poll_page_addr, (intptr_t) os::get_polling_page() );
    __ ld8(GR7_reg_stack_limit, GR7_reg_stack_limit);       // load register stack limit

    Label exit;

    __ mov(AR_LC, parm_count);
    __ mov(out_tos, out_parms);                             // out_tos = &out_parms[0]
    __ br(no_args, exit, Assembler::dpnt);

    // Reverse argument list and set up sender tos

    Label copy_word;
    __ bind(copy_word);

    __ ld8(arg, parms, BytesPerWord);                       // load *parms++
    __ st8(out_tos, arg, -BytesPerWord);                    // store *out_tos--
    __ cloop(copy_word, Assembler::sptk, Assembler::few);

    __ bind(exit);

    __ ld8(GP, entry_fd);
    __ mov(GR_entry_frame_TOS, out_tos);                    // so entry_frame_argument_at can find TOS

    // call interpreter frame manager

    __ call(entry_br);

    return_address = __ pc();

    // Store result depending on type.  Everything that is not
    // T_OBJECT, T_LONG, T_FLOAT, or T_DOUBLE is treated as T_INT.

    const PredicateRegister is_obj = PR6_SCRATCH;
    const PredicateRegister is_flt = PR7_SCRATCH;
    const PredicateRegister is_dbl = PR8_SCRATCH;
    const PredicateRegister is_lng = PR9_SCRATCH;

    __ cmp4(is_obj, PR0,    T_OBJECT, type, Assembler::equal);
    __ cmp4(is_flt, PR0,    T_FLOAT,  type, Assembler::equal);
    __ st4( result, GR_RET);

    __ st8( is_obj, result, GR_RET);
    __ stfs(is_flt, result, FR_RET);
    __ cmp4(is_dbl, PR0,    T_DOUBLE, type, Assembler::equal);

    __ stfd(is_dbl, result, FR_RET);
    __ cmp4(is_lng, PR0,    T_LONG,   type, Assembler::equal);
    __ mov(RP, GR_Lsave_RP);

    __ st8( is_lng, result, GR_RET);
    __ mov(GR4_thread, GR_entry_frame_GR4);

    __ mov(GR6_caller_BSP, GR_entry_frame_GR6);
    __ mov(GR7_reg_stack_limit, GR_entry_frame_GR7);
    __ mov(GR5_poll_page_addr, GR_entry_frame_GR5);
    __ mov(AR_PFS, GR_Lsave_PFS);

    __ mov(AR_LC, GR_Lsave_LC);
    __ mov(SP, GR_Lsave_SP);
    __ ret();

    return start;
  }


  //------------------------------------------------------------------------------------------------------------------------
  // Return point for a Java call if there's an exception thrown in Java code.
  // The exception is caught and transformed into a pending exception stored in
  // JavaThread that can be tested from within the VM.
  //
  address generate_catch_exception() {
    StubCodeMark mark(this, "StubRoutines", "catch_exception");

    address start = __ pc();

    // verify that thread corresponds
//  __ verify_thread();

    // set pending exception
//  __ verify_oop(GR8_exception, "generate_catch_exception");

    const Register pending_exception_addr   = GR2_SCRATCH;
    const Register exception_file_addr      = GR3_SCRATCH;
    const Register exception_line_addr      = GR31_SCRATCH;
    const Register exception_file           = GR30_SCRATCH;
    const Register exception_line           = GR29_SCRATCH;
    const Register call_stub_return_address = GR28_SCRATCH;

    const BranchRegister call_stub_return_address_br = BR6_SCRATCH;

    __ add(pending_exception_addr, thread_(pending_exception));
    __ mova(exception_file, (address)__FILE__);
    __ add(exception_file_addr, thread_(exception_file));
    __ mova(exception_line, (address)__LINE__);

    __ st8(pending_exception_addr, GR8_exception);
    __ st8(exception_file_addr, exception_file);
    __ add(exception_line_addr, thread_(exception_line));

    __ st8(exception_line_addr, exception_line);

    // complete return to VM
    assert(StubRoutines::_call_stub_return_address != NULL, "must have been generated before");

    __ mova(call_stub_return_address, StubRoutines::_call_stub_return_address);
    __ mov(call_stub_return_address_br, call_stub_return_address);
    __ br(call_stub_return_address_br);

    __ flush_bundle();

    return start;
  }


  //------------------------------------------------------------------------------------------------------------------------
  // Continuation point for runtime calls returning with a pending exception.
  // The pending exception check happened in the runtime or native call stub.
  // The pending exception in Thread is converted into a Java-level exception.
  //
  // Contract with Java-level exception handlers:
  //
  address generate_forward_exception() {
    StubCodeMark mark(this, "StubRoutines", "forward exception");

    address start = __ pc();

    // Upon entry, GR_Lsave_RP has the return address returning into Java
    // compiled code; i.e. the return address becomes the throwing pc.

    const Register pending_exception_addr = GR31_SCRATCH;
    const Register handler                = GR30_SCRATCH;

    const PredicateRegister is_not_null   = PR15_SCRATCH;
    const BranchRegister    handler_br    = BR6_SCRATCH;

    // Allocate abi scratch, since the compiler didn't allocate a memory frame.
    // pop_dummy_thin_frame will restore the caller's SP.
    __ sub(SP, SP, 16);

#ifdef ASSERT
    // Get pending exception oop.
    __ add(pending_exception_addr, thread_(pending_exception));
    __ ld8(GR8_exception, pending_exception_addr);

    // Make sure that this code is only executed if there is a pending exception.
    {
      Label not_null;
      __ cmp(is_not_null, PR0, 0, GR8_exception, Assembler::notEqual);
      __ br(is_not_null, not_null);
      __ stop("StubRoutines::forward exception: no pending exception (1)");
      __ bind(not_null);
    }

//  __ verify_oop(GR8_exception, "generate_forward_exception");
#endif

    // Find exception handler
    __ call_VM_leaf(CAST_FROM_FN_PTR(address, SharedRuntime::exception_handler_for_return_address), GR_Lsave_RP);

    __ mov(handler, GR_RET);

    // Load pending exception oop.
    __ add(pending_exception_addr, thread_(pending_exception));
    __ ld8(GR8_exception, pending_exception_addr);

    // The exception pc is the return address in the caller.
    __ mov(GR9_issuing_pc, GR_Lsave_RP);

    // Uses GR2, BR6
    __ pop_dummy_thin_frame();
    // Now in caller of native/stub register frame

#ifdef ASSERT
    // make sure exception is set
    {
      Label not_null;
      __ cmp(is_not_null, PR0, 0, GR8_exception, Assembler::notEqual);
      __ br(is_not_null, not_null);
      __ stop("StubRoutines::forward exception: no pending exception (2)");
      __ bind(not_null);
    }
#endif
    // clear pending exception
    __ st8(pending_exception_addr, GR0);

    // jump to exception handler
    __ mov(handler_br, handler);
    __ br(handler_br);

    __ flush_bundle();

    return start;
  }


  //------------------------------------------------------------------------------------------------------------------------
  // Continuation point for throwing of implicit exceptions that are not handled in
  // the current activation. Fabricates an exception oop and initiates normal
  // exception dispatching in this frame. Only callee-saved registers are preserved
  // (through the normal register window / RegisterMap handling).
  // If the compiler needs all registers to be preserved between the fault
  // point and the exception handler then it must assume responsibility for that in
  // AbstractCompiler::continuation_for_implicit_null_exception or
  // continuation_for_implicit_division_by_zero_exception. All other implicit
  // exceptions (e.g., NullPointerException or AbstractMethodError on entry) are
  // either at call sites or otherwise assume that stack unwinding will be initiated,
  // so caller saved registers were assumed volatile in the compiler.
  //
  address generate_throw_exception(const char* name, address runtime_entry, bool restore_saved_exception_pc) {
    int insts_size = VerifyThread ? 1 * K : 512;
    int locs_size  = 32;

    CodeBuffer*     code  = new CodeBuffer(insts_size, locs_size, 0, 0, 0, false, NULL, NULL, NULL, false, NULL, name, false);
    MacroAssembler* _masm = new MacroAssembler(code);

    const Register saved_exception_pc_addr = GR31_SCRATCH;
    const Register continuation            = GR30_SCRATCH;

    const Register saved_exception_pc      = GR31_SCRATCH;

    const BranchRegister continuation_br   = BR6_SCRATCH;

    // 4826555: nsk test stack016 fails.  See os_linux_ia64.cpp.
    // Reload register stack limit because the Linux kernel
    // doesn't reload GR4-7 from the ucontext.
    __ add(GR7_reg_stack_limit, thread_(register_stack_limit));
    __ ld8(GR7_reg_stack_limit, GR7_reg_stack_limit);

//  __ verify_thread();

    // If the exception occured at a call site target, RP contains the return address
    // (which is also the exception PC), and the call instruction has pushed a degenerate
    // register frame.  If the exception happened elsewhere, the signal handler has saved
    // the exception address in the thread state and we must execute a call instruction
    // in order to obtain a degenerate frame.  In either case, we must then push a frame
    // so we can execute a call_VM.

    if (restore_saved_exception_pc) {
      __ add(saved_exception_pc_addr, thread_(saved_exception_pc));
      __ ld8(saved_exception_pc, saved_exception_pc_addr);
      __ push_dummy_full_frame(saved_exception_pc);
    } else {
      __ push_full_frame();
    }

    // Install the exception oop in the thread state, etc.
    __ call_VM(noreg, runtime_entry);

    // Scale back to a thin frame for forward_exception.
    __ pop_full_to_thin_frame();

    // Branch to forward_exception.
    __ mova(continuation, StubRoutines::forward_exception_entry());
    __ mov(continuation_br, continuation);
    __ br(continuation_br);

    __ flush_bundle();

    RuntimeStub* stub = RuntimeStub::new_runtime_stub(name, code, 0, NULL, false);
    return stub->entry_point();
  }


  //------------------------------------------------------------------------------------------------------------------------
  // Flush the register stack.
  //
  address generate_get_backing_store_pointer() {
    StubCodeMark mark(this, "StubRoutines", "get_backing_store_pointer");

    address start = __ emit_fd();

    __ mov(GR_RET, AR_BSP);
    __ ret();

    return start;
  }


  //------------------------------------------------------------------------------------------------------------------------
  // Flush the register stack.
  //
  address generate_flush_register_stack() {
    StubCodeMark mark(this, "StubRoutines", "flush_register_stack");

    address start = __ emit_fd();

    const Register orig_RSC   = GR2_SCRATCH;
    const Register mod_RSC    = GR3_SCRATCH;
    __ mov(orig_RSC, AR_RSC);
    __ movl(mod_RSC, CONST64(0xFFFFFFFFC000FFFC));    // mask tear point to zero, rse to lazy

    __ flushrs();

    __ and3(mod_RSC, mod_RSC, orig_RSC);
    __ mov(AR_RSC, mod_RSC); 
    __ loadrs();		// Invalidate lower frames
    __ mov(AR_RSC, orig_RSC);   // restore tear point to original

    __ ret();

    return start;
  }

  //----------------------------------------------------------------------------------------------------
  // Support for jint Atomic::xchg(jint exchange_value, volatile jint* dest)
  // 
  // Arguments:
  //
  //     exchange_value - GR_I0
  //     dest           - GR_I1
  //
  // Results:
  //
  //     GR_RET - the value previously stored in dest
  //
  address generate_atomic_xchg() {
    StubCodeMark mark(this, "StubRoutines", "atomic_xchg");

    const Register exchange_value = GR_I0;
    const Register dest           = GR_I1;

    address start = __ emit_fd();

    __ mf();

    __ xchg4(GR_RET, dest, exchange_value);
    __ sxt4(GR_RET, GR_RET);

    __ ret();

    return start;
  }


  //----------------------------------------------------------------------------------------------------
  // Support for intptr_t Atomic::xchg_ptr(intptr_t exchange_value, volatile intptr_t* dest).
  //
  // Arguments:
  //
  //     exchange_value - GR_I0
  //     dest           - GR_I1
  //
  // Results:
  //
  //     GR_RET - the value previously stored in dest
  //
  address generate_atomic_xchg_ptr() {
    StubCodeMark mark(this, "StubRoutines", "atomic_xchg_ptr");

    const Register exchange_value = GR_I0;
    const Register dest           = GR_I1;

    address start = __ emit_fd();

    __ mf();

    __ xchg8(GR_RET, dest, exchange_value);

    __ ret();

    return start;
  }


  //----------------------------------------------------------------------------------------------------
  // Support for jint Atomic::cmpxchg(jint exchange_value, volatile jint* dest, jint compare_value)
  // 
  // Arguments:
  //
  //     exchange_value - GR_I0
  //     dest           - GR_I1
  //     compare_value  - GR_I2
  //
  // Results:
  //
  //     GR_RET - the value previously stored in dest
  //
  address generate_atomic_cmpxchg() {
    StubCodeMark mark(this, "StubRoutines", "atomic_cmpxchg");

    const Register exchange_value = GR_I0;
    const Register dest           = GR_I1;
    const Register compare_value  = GR_I2;

    address start = __ emit_fd();

    __ mf();

    __ zxt4(compare_value, compare_value);
    __ mov(AR_CCV, compare_value);
    __ cmpxchg4(GR_RET, dest, exchange_value, Assembler::acquire);
    __ sxt4(GR_RET, GR_RET);

    __ ret();

    return start;
  }


  // Support for intptr_t Atomic::cmpxchg_ptr(intptr_t exchange_value, volatile intptr_t* dest, intptr_t compare_value)
  //
  // Arguments:
  //
  //     exchange_value - GR_I0
  //     dest           - GR_I1
  //     compare_value  - GR_I2
  //
  // Results:
  //
  //     GR_RET - the value previously stored in dest
  //
  address generate_atomic_cmpxchg_ptr() { 
    StubCodeMark mark(this, "StubRoutines", "atomic_cmpxchg_ptr");

    const Register exchange_value = GR_I0;
    const Register dest           = GR_I1;
    const Register compare_value  = GR_I2;

    address start = __ emit_fd();

    __ mf();

    __ mov(AR_CCV, compare_value);
    __ cmpxchg8(GR_RET, dest, exchange_value, Assembler::acquire);

    __ ret();

    return start;
  }


  //----------------------------------------------------------------------------------------------------
  // Support for jint Atomic::add(jint inc, volatile jint* dest).
  //
  // Arguments:
  //
  //     inc  - GR_I0 (e.g., +1 or -1)
  //     dest - GR_I1
  //
  // Results:
  //
  //     GR_RET - the new value stored in dest
  //
  //
  address generate_atomic_add() {
    StubCodeMark mark(this, "StubRoutines", "atomic_add");

    const Register inc  = GR_I0;
    const Register dest = GR_I1;

    address start = __ emit_fd();

    __ mf();

    // increment or decrement
    __ cmp4(PR6, PR7, 1, inc, Assembler::equal);

    __ fetchadd4(PR6, GR_RET, dest,  1, Assembler::acquire);
    __ fetchadd4(PR7, GR_RET, dest, -1, Assembler::acquire);

    // GR_RET contains result of the fetch, not the add
    __ sxt4(GR_RET, GR_RET);
    __ adds(PR6, GR_RET,  1, GR_RET);
    __ adds(PR7, GR_RET, -1, GR_RET);

    __ ret();

    return start;
  }


  //----------------------------------------------------------------------------------------------------
  // Support for intptr_t Atomic::add_ptr(intptr_t inc, volatile intptr_t* dest).
  //
  // Arguments:
  //
  //     inc  - GR_I0 (e.g., +1 or -1)
  //     dest - GR_I1
  //
  // Results:
  //
  //     GR_RET - the new value stored in dest
  //
  address generate_atomic_add_ptr() {
    StubCodeMark mark(this, "StubRoutines", "atomic_add_ptr");

    const Register inc  = GR_I0;
    const Register dest = GR_I1;

    address start = __ emit_fd();

    __ mf();

    // increment or decrement
    __ cmp(PR6, PR7, 1, inc, Assembler::equal);

    __ fetchadd8(PR6, GR_RET, dest,  1, Assembler::acquire);
    __ fetchadd8(PR7, GR_RET, dest, -1, Assembler::acquire);

    // GR_RET contains result of the fetch, not the add
    __ adds(PR6, GR_RET,  1, GR_RET);
    __ adds(PR7, GR_RET, -1, GR_RET);

    __ ret();

    return start;
  }


  //----------------------------------------------------------------------------------------------------
  // Support for void OrderAccess::fence().
  //
  address generate_fence() {
    StubCodeMark mark(this, "StubRoutines", "fence");

    address start = __ emit_fd();

    // severe overkill
    __ mf();
    __ ret();

    return start;
  }


  //----------------------------------------------------------------------------------------------------
  // Support for void OrderAccess::acquire().  Windows only until compiler supports inline asm.
  //
  address generate_acquire() {
    StubCodeMark mark(this, "StubRoutines", "acquire");

    address start = __ emit_fd();

    // Issue a dummy ld8.acq
    __ ld8(GR31, SP, Assembler::acquired);
    __ ret();

    return start;
  }


  //----------------------------------------------------------------------------------------------------
  // Non-destructive plausibility checks for oops
  //  
  // Arguments:
  //     GR_I0 - oop to verify
  //
  address generate_verify_oop() {
    StubCodeMark mark(this, "StubRoutines", "verify_oop");

    address start = CAST_FROM_FN_PTR(address, ia_64_verify_oop);

    return start;
  }


  // Support for uint StubRoutine::ia64::partial_subtype_check( Klass sub, Klass super );
  // Arguments :
  //      ret  : GR_RET, returned
  //      sub  : I0, argument
  //      super: I1, argument
  //
  address generate_partial_subtype_check() {
    StubCodeMark mark(this, "StubRoutines", "partial_subtype_check");
    address start = __ pc();

    Label loop, missed;

    const Register subklass   = GR_I0; // subklass
    const Register superklass = GR_I1; // superklass

    const Register length     = GR_L0; // cache array length
    const Register index      = GR_L1; // index into cache array
    const Register value      = GR_L2; // current value from cache array
    const Register save_PFS   = GR_L3;

    const PredicateRegister miss = PR6_SCRATCH;

    // Allocate a small frame for a leaf routine
    __ alloc(save_PFS, 8, 4, 0, 0);

    // Set up the input and local registers

    int source_offset = Klass::secondary_supers_offset_in_bytes();
    int target_offset = Klass::secondary_super_cache_offset_in_bytes();

    int length_offset = arrayOopDesc::length_offset_in_bytes();
    int base_offset   = arrayOopDesc::base_offset_in_bytes(T_OBJECT);

    __ add(subklass, sizeof(oopDesc) + source_offset, subklass);

    __ ld8(value, subklass, target_offset - source_offset);

    // Point to the length
    __ add(value, length_offset, value);

    // Load the length, set the pointer to the base, and clear the index
    __ ld2(length, value, base_offset - length_offset);
    __ clr(index);

    // Load the next pointer (which can run 1 past the end)
    // Exit the loop if the count is reached
    __ bind(loop);

    __ ld8(GR_RET, value, BytesPerWord);
    __ cmp(miss, PR0, index, length, Assembler::equal); 
    __ br(miss, missed, Assembler::spnt);

    // Increment the loop counter
    // Exit if this is a match
    __ cmp(miss, PR0, GR_RET, superklass, Assembler::notEqual);
                                  // Check for match
    __ add(index, 1, index);	  // Bump index
    __ br(miss, loop, Assembler::sptk);

    // Got a hit; return success (zero result); set cache.
    // Cache load doesn't happen here; for speed it is directly emitted by the compiler.
    __ st8(subklass, superklass); // Save result to cache
    __ mov(AR_PFS, save_PFS);
    __ clr(GR_RET);               // Set zero result
    __ ret();			  // Result in GR_RET is ok

    // Got a miss, return non-zero result
    __ bind(missed);

    __ mov(AR_PFS, save_PFS);
    __ mov(GR_RET, 1);		  // Set non-zero result
    __ ret();			  // Result in GR_RET is ok

    return start;
  }

  // Support for jdouble StubRoutine::ia64::ldffill()( address fresult )
  // Arguments :
  //      ret	   : FR_RET, returned
  //      fresult  : I0, argument
  //
  address generate_ldffill() {
    StubCodeMark mark(this, "StubRoutines", "ldffill");
    address start = __ emit_fd();
    __ ldffill(FR_RET, GR_I0);
    __ ret();
    return start;
  }

  //---------------------------------------------------------------------------
  // Initialization
  
  void generate_initial() {
    // Generates all stubs and initializes the entry points

    //------------------------------------------------------------------------------------------------------------------------
    // entry points that exist in all platforms
    // Note: This is code that could be shared among different platforms - however the benefit seems to be smaller than
    //       the disadvantage of having a much more complicated generator structure. See also comment in stubRoutines.hpp.

    StubRoutines::_forward_exception_entry                = generate_forward_exception();    
    StubRoutines::_call_stub_entry                        = generate_call_stub(StubRoutines::_call_stub_return_address);
    StubRoutines::_catch_exception_entry                  = generate_catch_exception();    

    StubRoutines::_atomic_xchg_ptr_entry                  = generate_atomic_xchg_ptr();
    StubRoutines::_atomic_cmpxchg_ptr_entry               = generate_atomic_cmpxchg_ptr();
    StubRoutines::_atomic_cmpxchg_long_entry              = StubRoutines::_atomic_cmpxchg_ptr_entry;
    StubRoutines::_atomic_add_ptr_entry                   = generate_atomic_add_ptr();

    StubRoutines::_atomic_xchg_entry                      = generate_atomic_xchg();
    StubRoutines::_atomic_cmpxchg_entry                   = generate_atomic_cmpxchg();
    StubRoutines::_atomic_add_entry                       = generate_atomic_add();
    StubRoutines::_fence_entry                            = generate_fence();

    StubRoutines::ia64::_acquire_entry                    = generate_acquire();

    StubRoutines::ia64::_partial_subtype_check            = generate_partial_subtype_check();
    StubRoutines::ia64::_flush_register_stack_entry       = generate_flush_register_stack();
    StubRoutines::ia64::_get_backing_store_pointer        = generate_get_backing_store_pointer();

    StubRoutines::ia64::_ldffill_entry			  = generate_ldffill(); 
  }

  void generate_all() {
    // Generates all stubs and initializes the entry points
    
    // These entry points require SharedInfo::stack0 to be set up in non-core builds
    StubRoutines::_throw_AbstractMethodError_entry         = generate_throw_exception("AbstractMethodError throw_exception",          CAST_FROM_FN_PTR(address, SharedRuntime::throw_AbstractMethodError),  false);
    StubRoutines::_throw_ArithmeticException_entry         = generate_throw_exception("ArithmeticException throw_exception",          CAST_FROM_FN_PTR(address, SharedRuntime::throw_ArithmeticException),  true);
    StubRoutines::_throw_NullPointerException_entry        = generate_throw_exception("NullPointerException throw_exception",         CAST_FROM_FN_PTR(address, SharedRuntime::throw_NullPointerException), true);
    StubRoutines::_throw_NullPointerException_at_call_entry= generate_throw_exception("NullPointerException at call throw_exception", CAST_FROM_FN_PTR(address, SharedRuntime::throw_NullPointerException_at_call), false);
    StubRoutines::_throw_StackOverflowError_entry          = generate_throw_exception("StackOverflowError throw_exception",           CAST_FROM_FN_PTR(address, SharedRuntime::throw_StackOverflowError),   false);
  }


public:
  StubGenerator(CodeBuffer* code, bool all) : StubCodeGenerator(code) { 
    if (all) {
      generate_all();
    } else {
      generate_initial();
    }
    __ flush_bundle(); // Insurance
    __ flush();
  }

}; // StubGenerator


void StubGenerator_generate(CodeBuffer* code, bool all) {
  StubGenerator g(code, all);
}
