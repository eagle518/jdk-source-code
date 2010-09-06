#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)interp_masm_amd64.cpp	1.16 04/06/16 09:41:05 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_interp_masm_amd64.cpp.incl"


// Implementation of InterpreterMacroAssembler

void InterpreterMacroAssembler::call_VM_leaf_base(address entry_point,
                                                  int number_of_arguments)
{
  // interpreter specific
  //
  // Note: No need to save/restore bcp & locals (r13 & r14) pointer
  //       since these are callee saved registers and no blocking/
  //       GC can happen in leaf calls.
#ifdef ASSERT
  save_bcp();
#endif
  // super call
  MacroAssembler::call_VM_leaf_base(entry_point, number_of_arguments);
  // interpreter specific
#ifdef ASSERT
  { 
    Label L;
    cmpq(r13, Address(rbp, frame::interpreter_frame_bcx_offset * wordSize));
    jcc(Assembler::equal, L);
    stop("InterpreterMacroAssembler::call_VM_leaf_base:"
         " r13 not callee saved?");
    bind(L);
  }
  { 
    Label L;
    cmpq(r14, Address(rbp, frame::interpreter_frame_locals_offset * wordSize));
    jcc(Assembler::equal, L);
    stop("InterpreterMacroAssembler::call_VM_leaf_base:"
         " r14 not callee saved?");
    bind(L);
  }
#endif
}

void InterpreterMacroAssembler::call_VM_base(Register oop_result,
                                             Register java_thread,
                                             Register last_java_sp,
                                             address  entry_point,
                                             int      number_of_arguments,
                                             bool     check_exceptions)
{
  // interpreter specific
  //
  // Note: Could avoid restoring locals ptr (callee saved) - however doesn't
  //       really make a difference for these runtime calls, since they are
  //       slow anyway. Btw., bcp must be saved/restored since it may change
  //       due to GC.
  // assert(java_thread == noreg , "not expecting a precomputed java thread");
  save_bcp();
  // super call
  MacroAssembler::call_VM_base(oop_result, noreg, last_java_sp,
                               entry_point, number_of_arguments,
                               check_exceptions);
  // interpreter specific
  restore_bcp();
  restore_locals();
}


// call_VM_Ico handles calls to InterpreterRuntime::frequency_counter_overflow,
// which returns a struct IcoResult defined in interpreterRT_amd64.hpp.
// Returning a struct on windows requires a hidden first parameter, hence the
// special call_VM_Ico.
void InterpreterMacroAssembler::call_VM_Ico(Register oop_result,
                                            address entry_point,
                                            Register arg_1,
                                            bool check_exceptions)
{
#ifdef _WIN64
  Label C;
  // Push a return address so we can be identified if an exception occurs
  Assembler::call(C, relocInfo::none);
  bind(C);
  subq(rsp, 2*wordSize);
  movq(rarg0, rsp);
  if (rarg2 != arg_1) {
    movq(rarg2, arg_1);
  }
  movq(rarg1, r15_thread);
  leaq(rax, Address(rsp, 3*wordSize));
  call_VM_base(oop_result, noreg, rax, entry_point, 2, false);
  movq(rax, Address(rsp));
  movq(rdx, Address(rsp, wordSize));
  if (check_exceptions) {
    // Pop the hidden args leaving the identifying pc for exception handling
    addq(rsp, 2*wordSize);
    cmpq(Address(r15_thread, Thread::pending_exception_offset()), (int) NULL);
    jcc(Assembler::notEqual,
	StubRoutines::forward_exception_entry(),
	relocInfo::runtime_call_type);
    // pop the now useless identifying pc.
    addq(rsp, 1*wordSize);
  } else {
    addq(rsp, 3*wordSize);
  }
#else
  call_VM(oop_result, entry_point, arg_1, check_exceptions);
#endif
}


void InterpreterMacroAssembler::check_and_handle_popframe(Register java_thread)
{
  if (JvmtiExport::can_pop_frame()) {
    Label L;
    // Initiate popframe handling only if it is not already being
    // processed.  If the flag has the popframe_processing bit set, it
    // means that this code is called *during* popframe handling - we
    // don't want to reenter.
    // This method is only called just after the call into the vm in
    // call_VM_base, so the arg registers are available.
    movl(rarg0, Address(r15_thread, JavaThread::popframe_condition_offset()));
    testl(rarg0, JavaThread::popframe_pending_bit);
    jcc(Assembler::zero, L);
    testl(rarg0, JavaThread::popframe_processing_bit);
    jcc(Assembler::notZero, L);   
    call(CAST_FROM_FN_PTR(address, 
                          Interpreter::
                          remove_activation_preserving_args_entry),
         relocInfo::runtime_call_type);
    jmp(rax, relocInfo::runtime_call_type);
    bind(L);
  }
}


void InterpreterMacroAssembler::get_unsigned_2_byte_index_at_bcp(
  Register reg,
  int bcp_offset) 
{
  assert(bcp_offset >= 0, "bcp is still pointing to start of bytecode");
  movl(reg, Address(r13, bcp_offset));
  bswapl(reg);
  shrl(reg, 16);
}


void InterpreterMacroAssembler::get_cache_and_index_at_bcp(Register cache,
                                                           Register index, 
                                                           int bcp_offset)
{
  assert(bcp_offset > 0, "bcp is still pointing to start of bytecode");
  assert(cache != index, "must use different registers");
  load_unsigned_word(index, Address(r13, bcp_offset));
  movq(cache, Address(rbp, frame::interpreter_frame_cache_offset * wordSize));
  assert(sizeof(ConstantPoolCacheEntry) == 4 * wordSize, "adjust code below");
  // convert from field index to ConstantPoolCacheEntry index
  shll(index, 2);
}


void InterpreterMacroAssembler::get_cache_entry_pointer_at_bcp(Register cache,
                                                               Register tmp,
                                                               int bcp_offset)
{
  assert(bcp_offset > 0, "bcp is still pointing to start of bytecode");
  assert(cache != tmp, "must use different register");
  load_unsigned_word(tmp, Address(r13, bcp_offset));
  assert(sizeof(ConstantPoolCacheEntry) == 4 * wordSize, "adjust code below");
  // convert from field index to ConstantPoolCacheEntry index
  // and from word offset to byte offset
  shll(tmp, 2 + LogBytesPerWord);
  movq(cache, Address(rbp, frame::interpreter_frame_cache_offset * wordSize));
  // skip past the header
  addq(cache, in_bytes(constantPoolCacheOopDesc::base_offset()));
  addq(cache, tmp);  // construct pointer to cache entry
}


// Generate a subtype check: branch to ok_is_subtype if sub_klass is a
// subtype of super_klass.
//
// Args:
//      rax: superklass
//      rarg3: 2ndary super array length
//      rarg0: 2ndary super array scan ptr
//      Rsub_klass: subklass
//
// Kills:
//      rarg3, rarg2
void InterpreterMacroAssembler::gen_subtype_check(Register Rsub_klass,
                                                  Label& ok_is_subtype)
{
  assert(Rsub_klass != rax, "rax holds superklass");
  assert(Rsub_klass != rarg3, "rarg3 holds 2ndary super array length");
  assert(Rsub_klass != rarg0, "rarg0 holds 2ndary super array scan ptr");
  assert(Rsub_klass != r14, "r14 holds locals");
  assert(Rsub_klass != r13, "r13 holds bcp");
  Label not_subtype, loop;

  // Load the super-klass's check offset into rcx
  movl(rcx, Address(rax, sizeof(oopDesc) + 
                    Klass::super_check_offset_offset_in_bytes()));
  // Load from the sub-klass's super-class display list, or a 1-word
  // cache of the secondary superclass list, or a failing value with a
  // sentinel offset if the super-klass is an interface or
  // exceptionally deep in the Java hierarchy and we have to scan the
  // secondary superclass list the hard way.  See if we get an
  // immediate positive hit
  cmpq(rax, Address(Rsub_klass, rcx, Address::times_1));
  jcc(Assembler::equal,ok_is_subtype);

  // Check for immediate negative hit
  cmpl(rcx, sizeof(oopDesc) + Klass::secondary_super_cache_offset_in_bytes());
  jcc( Assembler::notEqual, not_subtype );
  // Check for self
  cmpq(Rsub_klass, rax);
  jcc(Assembler::equal, ok_is_subtype);

  // Now do a linear scan of the secondary super-klass chain.
  movq(rdi, Address(Rsub_klass, sizeof(oopDesc) + 
                    Klass::secondary_supers_offset_in_bytes()));
  // rdi holds the objArrayOop of secondary supers.
  // Load the array length
  movl(rcx, Address(rdi, arrayOopDesc::length_offset_in_bytes())); 
  // Skip to start of data; also clear Z flag incase rcx is zero
  addq(rdi, arrayOopDesc::base_offset_in_bytes(T_OBJECT));
  // Scan rcx words at [rdi] for occurance of rax
  // Set NZ/Z based on last compare
  repne_scan();
  // Not equal?
  jcc(Assembler::notEqual, not_subtype);
  // Must be equal but missed in cache.  Update cache.
  movq(Address(Rsub_klass, sizeof(oopDesc) + 
               Klass::secondary_super_cache_offset_in_bytes()), rax);
  jmp(ok_is_subtype);

  bind(not_subtype);
}

void InterpreterMacroAssembler::pop_ptr(Register r)
{
  popq(r);
}

void InterpreterMacroAssembler::pop_i(Register r)
{
  // XXX can't use popq currently, upper half non clean
  movl(r, Address(rsp));
  addq(rsp, wordSize);
}

void InterpreterMacroAssembler::pop_l(Register r)
{
  movq(r, Address(rsp));
  addq(rsp, 2 * wordSize);
}

void InterpreterMacroAssembler::pop_f(FloatRegister r)
{
  movss(r, Address(rsp));
  addq(rsp, wordSize);
}

void InterpreterMacroAssembler::pop_d(FloatRegister r)
{
  movlpd(r, Address(rsp));
  addq(rsp, 2 * wordSize);
}

void InterpreterMacroAssembler::push_ptr(Register r)
{
  pushq(r);
}

void InterpreterMacroAssembler::push_i(Register r)
{
  pushq(r);
}

void InterpreterMacroAssembler::push_l(Register r)
{
  subq(rsp, 2 * wordSize);
  movq(Address(rsp), r);
}

void InterpreterMacroAssembler::push_f(FloatRegister r)
{
  subq(rsp, wordSize);
  movss(Address(rsp), r);
}

void InterpreterMacroAssembler::push_d(FloatRegister r)
{
  subq(rsp, 2 * wordSize);
  movsd(Address(rsp), r);
}

void InterpreterMacroAssembler::pop(TosState state) 
{
  switch (state) {
  case atos: pop_ptr();                 break;
  case btos:                            
  case ctos: 
  case stos: 
  case itos: pop_i();                   break;
  case ltos: pop_l();                   break;
  case ftos: pop_f();                   break;
  case dtos: pop_d();                   break;
  case vtos: /* nothing to do */        break;
  default:   ShouldNotReachHere();
  }
  verify_oop(rax, state);
}

void InterpreterMacroAssembler::push(TosState state) 
{
  verify_oop(rax, state);
  switch (state) {
  case atos: push_ptr();                break;
  case btos: 
  case ctos: 
  case stos: 
  case itos: push_i();                  break;
  case ltos: push_l();                  break;
  case ftos: push_f();                  break;
  case dtos: push_d();                  break;
  case vtos: /* nothing to do */        break;
  default  : ShouldNotReachHere();
  }
}

void InterpreterMacroAssembler::super_call_VM_leaf(address entry_point) 
{
  MacroAssembler::call_VM_leaf_base(entry_point, 0);
}


void InterpreterMacroAssembler::super_call_VM_leaf(address entry_point,
                                                   Register arg_1)
{
  if (rarg0 != arg_1) {
    movq(rarg0, arg_1);
  }
  MacroAssembler::call_VM_leaf_base(entry_point, 1);
}


void InterpreterMacroAssembler::super_call_VM_leaf(address entry_point,
                                                   Register arg_1,
                                                   Register arg_2)
{
  assert(rarg0 != arg_2, "smashed argument");
  assert(rarg1 != arg_1, "smashed argument");
  if (rarg0 != arg_1) {
    movq(rarg0, arg_1);
  }
  if (rarg1 != arg_2) {
    movq(rarg1, arg_2);
  }
  MacroAssembler::call_VM_leaf_base(entry_point, 2);
}

void InterpreterMacroAssembler::super_call_VM_leaf(address entry_point,
                                                   Register arg_1,
                                                   Register arg_2,
                                                   Register arg_3)
{
  assert(rarg0 != arg_2, "smashed argument");
  assert(rarg0 != arg_3, "smashed argument");
  assert(rarg1 != arg_1, "smashed argument");
  assert(rarg1 != arg_3, "smashed argument");
  assert(rarg2 != arg_1, "smashed argument");
  assert(rarg2 != arg_2, "smashed argument");
  if (rarg0 != arg_1) {
    movq(rarg0, arg_1);
  }
  if (rarg1 != arg_2) {
    movq(rarg1, arg_2);
  }
  if (rarg2 != arg_3) {
    movq(rarg2, arg_3);
  }
  MacroAssembler::call_VM_leaf_base(entry_point, 3);
}


void InterpreterMacroAssembler::super_call_VM(Register oop_result,
                                              Register last_java_sp,
                                              address entry_point,
                                              Register arg_1,
                                              Register arg_2)
{
  assert(rarg0 != arg_1, "smashed argument");
  assert(rarg0 != arg_2, "smashed argument");
  assert(rarg1 != arg_2, "smashed argument");
  assert(rarg1 != last_java_sp, "smashed argument");
  assert(rarg2 != arg_1, "smashed argument");
  assert(rarg2 != last_java_sp, "smashed argument");
  // rarg0 is reserved for thread
  movq(rarg0, r15_thread);
  if (rarg1 != arg_1) {
    movq(rarg1, arg_1);
  }
  if (rarg2 != arg_2) {
    movq(rarg2, arg_2);
  }
  MacroAssembler::call_VM_base(oop_result, noreg, last_java_sp,
                               entry_point, 2, true);
}

// The following two routines provide a hook so that an implementation
// can schedule the dispatch in two parts.  amd64 does not do this.
void InterpreterMacroAssembler::dispatch_prolog(TosState state, int step) 
{
  // Nothing amd64 specific to be done here
}

void InterpreterMacroAssembler::dispatch_epilog(TosState state, int step)
{
  dispatch_next(state, step);
}

void InterpreterMacroAssembler::dispatch_base(TosState state, 
                                              address* table,
                                              bool verifyoop) 
{
  verify_FPU(1, state);
  if (VerifyActivationFrameSize) {
    Label L;
    movq(rarg3, rbp);
    subq(rarg3, rsp);
    int min_frame_size = 
      (frame::link_offset - frame::interpreter_frame_initial_sp_offset) *
      wordSize;
    cmpq(rarg3, min_frame_size);
    jcc(Assembler::greaterEqual, L);
    stop("broken stack frame");
    bind(L);
  }
  if (verifyoop) {
    verify_oop(rax, state);
  }
  movq(rscratch1,(int64_t) table);
  jmp(Address(rscratch1, rbx, Address::times_8));
}

void InterpreterMacroAssembler::dispatch_only(TosState state) 
{
  dispatch_base(state, Interpreter::dispatch_table(state));
}

void InterpreterMacroAssembler::dispatch_only_normal(TosState state) 
{
  dispatch_base(state, Interpreter::normal_table(state));
}

void InterpreterMacroAssembler::dispatch_only_noverify(TosState state) 
{
  dispatch_base(state, Interpreter::normal_table(state), false);
}


void InterpreterMacroAssembler::dispatch_next(TosState state, int step) 
{
  // load next bytecode (load before advancing esi to prevent AGI)
  load_unsigned_byte(rbx, Address(r13, step));
  // advance r13
  incrementq(r13, step);
  dispatch_base(state, Interpreter::dispatch_table(state));
}

void InterpreterMacroAssembler::dispatch_via(TosState state, address* table) 
{
  // load current bytecode
  load_unsigned_byte(rbx, Address(r13));
  dispatch_base(state, table);
}

// remove activation
//
// Unlock the receiver if this is a synchronized method.
// Unlock any Java monitors from syncronized blocks.
// Remove the activation from the stack.
//
// If there are locked Java monitors
//    If throw_monitor_exception
//       throws IllegalMonitorStateException
//    Else if install_monitor_exception
//       installs IllegalMonitorStateException
//    Else
//       no error processing
void InterpreterMacroAssembler::remove_activation(
        TosState state,
        Register ret_addr,
        bool throw_monitor_exception,
        bool install_monitor_exception,
        bool notify_jvmdi) 
{
  // Note: Registers eax, edx and FPU ST(0) may be in use for the
  // result check if synchronized method
  Label unlocked, unlock, no_unlock;

  // get the value of _do_not_unlock_if_synchronized into rarg0
  const Address do_not_unlock_if_synchronized(r15_thread,
    in_bytes(JavaThread::do_not_unlock_if_synchronized_offset()));
  movl(rarg0, do_not_unlock_if_synchronized);
  movl(do_not_unlock_if_synchronized, (int) false); // reset the flag

 // get method access flags
  movq(rbx, Address(rbp, frame::interpreter_frame_method_offset * wordSize));
  movl(rarg3, Address(rbx, methodOopDesc::access_flags_offset()));
  testl(rarg3, JVM_ACC_SYNCHRONIZED);
  jcc(Assembler::zero, unlocked);
 
  // Don't unlock anything if the _do_not_unlock_if_synchronized flag
  // is set.
  testl(rarg0, rarg0);
  jcc(Assembler::notZero, no_unlock);
 
  // unlock monitor
  push(state); // save result
    
  // BasicObjectLock will be first in list, since this is a
  // synchronized method. However, need to check that the object has
  // not been unlocked by an explicit monitorexit bytecode.
  const Address monitor(rbp, frame::interpreter_frame_initial_sp_offset * 
                        wordSize - (int) sizeof(BasicObjectLock));
  leaq(rarg1, monitor); // address of first monitor
  
  movq(rax, Address(rarg1, BasicObjectLock::obj_offset_in_bytes()));
  testq(rax, rax);
  jcc(Assembler::notZero, unlock);
                                  
  pop(state);
  if (throw_monitor_exception) {
    // Entry already unlocked, need to throw exception
    call_VM(noreg, CAST_FROM_FN_PTR(address, 
                   InterpreterRuntime::throw_illegal_monitor_state_exception));
    should_not_reach_here();
  } else {
    // Monitor already unlocked during a stack unroll. If requested,
    // install an illegal_monitor_state_exception.  Continue with
    // stack unrolling.
    if (install_monitor_exception) {
      call_VM(noreg, CAST_FROM_FN_PTR(address, 
                     InterpreterRuntime::new_illegal_monitor_state_exception));
    }
    jmp(unlocked);
  }

  bind(unlock);  
  unlock_object(rarg1);              
  pop(state);

  // Check that for block-structured locking (i.e., that all locked
  // objects has been unlocked)
  bind(unlocked);  

  // rax: Might contain return value

  // Check that all monitors are unlocked
  {
    Label loop, exception, entry, restart;
    const int entry_size = frame::interpreter_frame_monitor_size() * wordSize;
    const Address monitor_block_top(
        rbp, frame::interpreter_frame_monitor_block_top_offset * wordSize);
    const Address monitor_block_bot(
        rbp, frame::interpreter_frame_initial_sp_offset * wordSize);
    
    bind(restart);
    movq(rarg3, monitor_block_top); // points to current entry, starting
                                  // with top-most entry
    leaq(rbx, monitor_block_bot); // points to word before bottom of
                                  // monitor block
    jmp(entry);
          
    // Entry already locked, need to throw exception
    bind(exception); 

    if (throw_monitor_exception) {
      // Throw exception      
      MacroAssembler::call_VM(noreg, 
                              CAST_FROM_FN_PTR(address, InterpreterRuntime::
                                   throw_illegal_monitor_state_exception));
      should_not_reach_here();
    } else {
      // Stack unrolling. Unlock object and install
      // illegal_monitor_exception Unlock does not block, so don't
      // have to worry about the frame We don't have to preserve rax,
      // rarg1 since we are going to throw an exception

      movq(rarg1, rarg3);
      unlock_object(rarg1);
      
      if (install_monitor_exception) {
        call_VM(noreg, CAST_FROM_FN_PTR(address, 
                                        InterpreterRuntime::
                                        new_illegal_monitor_state_exception));
      }

      jmp(restart);
    }
  
    bind(loop);
    // check if current entry is used
    cmpq(Address(rarg3, BasicObjectLock::obj_offset_in_bytes()), (int) NULL);
    jcc(Assembler::notEqual, exception);
          
    addq(rarg3, entry_size); // otherwise advance to next entry
    bind(entry);
    cmpq(rarg3, rbx); // check if bottom reached
    jcc(Assembler::notEqual, loop); // if not at bottom then check this entry
  }        

  bind(no_unlock);

  // jvmpi support (jvmdi does not generate MethodExit on exception / popFrame)
  if (notify_jvmdi) {
    notify_method_exit(state);       // preserve TOSCA
  } else {
    notify_jvmpi_method_exit(state); // preserve TOSCA
  }

  // remove activation
  // get sender sp
  movq(rbx, 
       Address(rbp, frame::interpreter_frame_sender_sp_offset * wordSize));
  leave();                           // remove frame anchor
  popq(ret_addr);                    // get return address
  movq(rsp, rbx);                    // set sp to sender sp
}

// Lock object
//
// Args:
//      rarg1: BasicObjectLock to be used for locking
// 
// Kills:
//      rax
//      rarg0, rarg1, rarg2, rarg3, rarg4, rarg5 (param regs)
//      rscratch1, rscratch2 (scratch regs)
void InterpreterMacroAssembler::lock_object(Register lock_reg) 
{
  assert(lock_reg == rarg1, "The argument is only for looks. It must be rarg1");

  if (UseHeavyMonitors) {
    call_VM(noreg, 
            CAST_FROM_FN_PTR(address, InterpreterRuntime::monitorenter),
            lock_reg);
  } else {
    Label done;

    const Register swap_reg = rax; // Must use rax for cmpxchg instruction
    const Register obj_reg = rarg3; // Will contain the oop

    const int obj_offset = BasicObjectLock::obj_offset_in_bytes();
    const int lock_offset = BasicObjectLock::lock_offset_in_bytes ();
    const int mark_offset = lock_offset + 
                            BasicLock::displaced_header_offset_in_bytes(); 

    // Load object pointer into obj_reg %rarg3
    movq(obj_reg, Address(lock_reg, obj_offset));

    // Load immediate 1 into swap_reg %rax
    movl(swap_reg, 1);

    // Load (object->mark() | 1) into swap_reg %rax
    orq(swap_reg, Address(obj_reg));

    // Save (object->mark() | 1) into BasicLock's displaced header
    movq(Address(lock_reg, mark_offset), swap_reg);

    assert(lock_offset == 0, 
           "displached header must be first word in BasicObjectLock");

    if (os::is_MP()) lock();
    cmpxchgq(lock_reg, Address(obj_reg));  

    jcc(Assembler::zero, done);

    // Test if the oopMark is an obvious stack pointer, i.e.,
    //  1) (mark & 7) == 0, and
    //  2) rsp <= mark < mark + os::pagesize()
    //
    // These 3 tests can be done by evaluating the following 
    // expression: ((mark - rsp) & (7 - os::vm_page_size())),
    // assuming both stack pointer and pagesize have their
    // least significant 3 bits clear.
    // NOTE: the oopMark is in swap_reg %rax as the result of cmpxchg
    subq(swap_reg, rsp);
    andq(swap_reg, 7 - os::vm_page_size());

    // Save the test result, for recursive case, the result is zero
    movq(Address(lock_reg, mark_offset), swap_reg);

    jcc(Assembler::zero, done);

    // Call the runtime routine for slow case
    call_VM(noreg, 
            CAST_FROM_FN_PTR(address, InterpreterRuntime::monitorenter),
            lock_reg);

    bind(done);
  }   
}


// Unlocks an object. Used in monitorexit bytecode and
// remove_activation.  Throws an IllegalMonitorException if object is
// not locked by current thread.
//
// Args:
//      rarg1: BasicObjectLock for lock
// 
// Kills:
//      rax
//      rarg0, rarg1, rarg2, rarg3, rarg4, rarg5 (param regs)
//      rscratch1, rscratch2 (scratch regs)
void InterpreterMacroAssembler::unlock_object(Register lock_reg) 
{
  assert(lock_reg == rarg1, "The argument is only for looks. It must be rarg1");

  if (UseHeavyMonitors) {
    call_VM(noreg, 
            CAST_FROM_FN_PTR(address, InterpreterRuntime::monitorexit),
            lock_reg);
  } else {
    Label done;

    const Register swap_reg   = rax;  // Must use rax for cmpxchg instruction
    const Register header_reg = rarg2;  // Will contain the old oopMark
    const Register obj_reg    = rarg3;  // Will contain the oop

    save_bcp(); // Save in case of exception

    // Convert from BasicObjectLock structure to object and BasicLock
    // structure Store the BasicLock address into %rax
    leaq(swap_reg, Address(lock_reg, BasicObjectLock::lock_offset_in_bytes()));

    // Load oop into obj_reg(%rarg3)
    movq(obj_reg, Address(lock_reg, BasicObjectLock::obj_offset_in_bytes()));

    // Load the old header from BasicLock structure
    movq(header_reg, Address(swap_reg, 
                             BasicLock::displaced_header_offset_in_bytes()));

    // Free entry
    movq(Address(lock_reg, BasicObjectLock::obj_offset_in_bytes()), 
         (int) NULL);

    // Test for recursion
    testq(header_reg, header_reg);

    // zero for recursive case
    jcc(Assembler::zero, done);
    
    // Atomic swap back the old header
    if (os::is_MP()) lock();
    cmpxchgq(header_reg, Address(obj_reg));

    // zero for recursive case
    jcc(Assembler::equal, done);

    // Call the runtime routine for slow case.
    movq(Address(lock_reg, BasicObjectLock::obj_offset_in_bytes()), 
         obj_reg); // restore obj
    call_VM(noreg, 
            CAST_FROM_FN_PTR(address, InterpreterRuntime::monitorexit),
            lock_reg);

    bind(done);

    restore_bcp();
  }
}


#ifndef CORE
void InterpreterMacroAssembler::test_method_data_pointer(Register mdp, 
                                                         Label& zero_continue)
{
  assert(ProfileInterpreter, "must be profiling interpreter");
  movq(mdp, Address(rbp, frame::interpreter_frame_mdx_offset * wordSize));
  testq(mdp, mdp);
  jcc(Assembler::zero, zero_continue);
}


// Set the method data pointer for the current bcp.
void InterpreterMacroAssembler::set_method_data_pointer_for_bcp() {
  assert(ProfileInterpreter, "must be profiling interpreter");
  Label zero_continue;
  pushq(rax);
  pushq(rbx);

  get_method(rbx);
  // Test MDO to avoid the call if it is NULL.
  movq(rax, Address(rbx, in_bytes(methodOopDesc::method_data_offset())));
  testq(rax, rax);
  jcc(Assembler::zero, zero_continue);

  // rbx: method
  // r13: bcp
  call_VM_leaf(CAST_FROM_FN_PTR(address, InterpreterRuntime::bcp_to_di), rbx, r13);
  // rax: mdi

  movq(rbx, Address(rbx, in_bytes(methodOopDesc::method_data_offset())));
  testq(rbx, rbx);
  jcc(Assembler::zero, zero_continue);
  addq(rbx, in_bytes(methodDataOopDesc::data_offset()));
  addq(rbx, rax);
  movq(Address(rbp, frame::interpreter_frame_mdx_offset * wordSize), rbx);

  bind(zero_continue);
  popq(rbx);
  popq(rax);
}

void InterpreterMacroAssembler::verify_method_data_pointer() 
{
  assert(ProfileInterpreter, "must be profiling interpreter");
#ifdef ASSERT
  Label verify_continue;
  pushq(rax);
  pushq(rbx);
  pushq(rarg3);
  pushq(rarg2);
  test_method_data_pointer(rarg3, verify_continue); // If mdp is zero, continue
  get_method(rbx);

  // If the mdp is valid, it will point to a DataLayout header which is
  // consistent with the bcp.  The converse is highly probable also.
  load_unsigned_word(rarg2, 
                     Address(rarg3, in_bytes(DataLayout::bci_offset())));
  addq(rarg2, Address(rbx, methodOopDesc::const_offset()));
  leaq(rarg2, Address(rarg2, constMethodOopDesc::codes_offset()));
  cmpq(rarg2, r13);
  jcc(Assembler::equal, verify_continue);
  // rbx: method
  // r13: bcp
  // rarg3: mdp
  call_VM_leaf(CAST_FROM_FN_PTR(address, InterpreterRuntime::verify_mdp),
               rbx, r13, rarg3);
  bind(verify_continue);
  popq(rarg2);
  popq(rarg3);
  popq(rbx);
  popq(rax);
#endif // ASSERT
}


void InterpreterMacroAssembler::set_mdp_data_at(Register mdp_in, 
                                                int constant, 
                                                Register value)
{
  assert(ProfileInterpreter, "must be profiling interpreter");
  Address data(mdp_in, constant);
  movq(data, value);
}


void InterpreterMacroAssembler::increment_mdp_data_at(Register mdp_in,
                                                      int constant)
{
  assert(ProfileInterpreter, "must be profiling interpreter");

  // Counter address
  Address data(mdp_in, constant);

  assert(DataLayout::counter_increment == 1,
         "flow-free idiom only works with 1");
  // Increment the register.  Set carry flag.
  addq(data, DataLayout::counter_increment);
  // If the increment causes the counter to overflow, pull back by 1.
  sbbq(data, 0);
}


void InterpreterMacroAssembler::increment_mdp_data_at(Register mdp_in,
                                                      Register reg,
                                                      int constant) 
{
  assert(ProfileInterpreter, "must be profiling interpreter");

  Address data(mdp_in, reg, Address::times_1, constant);

  assert(DataLayout::counter_increment == 1, 
         "flow-free idiom only works with 1");
  // Increment the register.  Set carry flag.
  addq(data, DataLayout::counter_increment);
  // If the increment causes the counter to overflow, pull back by 1.
  sbbq(data, 0);
}

void InterpreterMacroAssembler::set_mdp_flag_at(Register mdp_in, 
                                                int flag_constant)
{
  assert(ProfileInterpreter, "must be profiling interpreter");
  orq(Address(mdp_in, in_bytes(ProfileData::header_offset())), flag_constant);
}



void InterpreterMacroAssembler::test_mdp_data_at(Register mdp_in,
                                                 int offset,
                                                 Register value,
                                                 Label& not_equal_continue) 
{
  assert(ProfileInterpreter, "must be profiling interpreter");
  cmpq(value, Address(mdp_in, offset));
  jcc(Assembler::notEqual, not_equal_continue);
}


void InterpreterMacroAssembler::update_mdp_by_offset(Register mdp_in,
                                                     int offset_of_disp) 
{
  assert(ProfileInterpreter, "must be profiling interpreter");
  Address disp_address(mdp_in, offset_of_disp);
  addq(mdp_in, disp_address);
  movq(Address(rbp, frame::interpreter_frame_mdx_offset * wordSize), mdp_in);
}


void InterpreterMacroAssembler::update_mdp_by_offset(Register mdp_in, 
                                                     Register reg,
                                                     int offset_of_disp) 
{
  assert(ProfileInterpreter, "must be profiling interpreter");
  Address disp_address(mdp_in, reg, Address::times_1, offset_of_disp);
  addq(mdp_in, disp_address);
  movq(Address(rbp, frame::interpreter_frame_mdx_offset * wordSize), mdp_in);
}


void InterpreterMacroAssembler::update_mdp_by_constant(Register mdp_in,
                                                       int constant)
{
  assert(ProfileInterpreter, "must be profiling interpreter");
  addq(mdp_in, constant);
  movq(Address(rbp, frame::interpreter_frame_mdx_offset * wordSize), mdp_in);
}


void InterpreterMacroAssembler::update_mdp_for_ret(Register return_bci)
{
  assert(ProfileInterpreter, "must be profiling interpreter");
  pushq(return_bci); // save/restore across call_VM
  call_VM(noreg, 
          CAST_FROM_FN_PTR(address, InterpreterRuntime::update_mdp_for_ret),
          return_bci);
  popq(return_bci);
}
#endif // !CORE


void InterpreterMacroAssembler::profile_taken_branch(Register mdp, 
                                                     Register bumped_count) 
{
#ifndef CORE
  if (ProfileInterpreter) {
    Label profile_continue;

    // If no method data exists, go to profile_continue.
    // Otherwise, assign to mdp
    test_method_data_pointer(mdp, profile_continue);

    // We are taking a branch.  Increment the taken count.
    // We inline increment_mdp_data_at to return bumped_count in a register
    //increment_mdp_data_at(mdp, in_bytes(JumpData::taken_offset()));
    Address data(mdp, in_bytes(JumpData::taken_offset()));
    movq(bumped_count, data);
    assert(DataLayout::counter_increment == 1,
            "flow-free idiom only works with 1");
    addq(bumped_count, DataLayout::counter_increment);
    sbbq(bumped_count, 0);
    movq(data, bumped_count); // Store back out

    // The method data pointer needs to be updated to reflect the new target.
    update_mdp_by_offset(mdp, in_bytes(JumpData::displacement_offset()));
    bind(profile_continue);
  }
#endif // !CORE
}


void InterpreterMacroAssembler::profile_not_taken_branch(Register mdp) 
{
#ifndef CORE
  if (ProfileInterpreter) {
    Label profile_continue;

    // If no method data exists, go to profile_continue.
    test_method_data_pointer(mdp, profile_continue);

    // We are taking a branch.  Increment the not taken count.
    increment_mdp_data_at(mdp, in_bytes(BranchData::not_taken_offset()));

    // The method data pointer needs to be updated to correspond to
    // the next bytecode
    update_mdp_by_constant(mdp, in_bytes(BranchData::branch_data_size()));
    bind(profile_continue);
  }
#endif // !CORE
}


void InterpreterMacroAssembler::profile_call(Register mdp)
{
#ifndef CORE
  if (ProfileInterpreter) {
    Label profile_continue;

    // If no method data exists, go to profile_continue.
    test_method_data_pointer(mdp, profile_continue);

    // We are making a call.  Increment the count.
    increment_mdp_data_at(mdp, in_bytes(CounterData::count_offset()));

    // The method data pointer needs to be updated to reflect the new target.
    update_mdp_by_constant(mdp, in_bytes(CounterData::counter_data_size()));
    bind(profile_continue);
  }
#endif // !CORE
}


void InterpreterMacroAssembler::profile_final_call(Register mdp)
{
#ifndef CORE
  if (ProfileInterpreter) {
    Label profile_continue;

    // If no method data exists, go to profile_continue.
    test_method_data_pointer(mdp, profile_continue);

    // We are making a call.  Increment the count.
    increment_mdp_data_at(mdp, in_bytes(CounterData::count_offset()));

    // The method data pointer needs to be updated to reflect the new target.
    update_mdp_by_constant(mdp, 
                           in_bytes(VirtualCallData::
                                    virtual_call_data_size()));
    bind(profile_continue);
  }
#endif // !CORE
}


void InterpreterMacroAssembler::profile_virtual_call(Register receiver,
                                                     Register mdp,
                                                     Register reg2)
{
#ifndef CORE
  if (ProfileInterpreter) {
    Label profile_continue;
    Label update_mdp;
    uint row;

    // If no method data exists, go to profile_continue.
    test_method_data_pointer(mdp, profile_continue);

    // We are making a call.  Increment the count.
    increment_mdp_data_at(mdp, in_bytes(CounterData::count_offset()));

    for (row = 0; row < VirtualCallData::row_limit(); row++) {
      Label next_test;

      // See if the receiver is receiver[n].
      test_mdp_data_at(mdp, 
                       in_bytes(VirtualCallData::receiver_offset(row)),
		       receiver, 
                       next_test);

      // The receiver is receiver[n].  Increment count[n].
      increment_mdp_data_at(mdp, 
                            in_bytes(VirtualCallData::
                                     receiver_count_offset(row)));
      jmp(update_mdp);
      bind(next_test);
    }

    // Zero out reg2.
    xorl(reg2, reg2);

    for (row = 0; row < VirtualCallData::row_limit(); row++) {
      Label next_zero_test;

      // See if receiver[n] is null:
      test_mdp_data_at(mdp, 
                       in_bytes(VirtualCallData::receiver_offset(row)),
		       reg2, 
                       next_zero_test);

      // receiver[n] is NULL.  Fill in the receiver field and
      // increment the count.
      set_mdp_data_at(mdp, 
                      in_bytes(VirtualCallData::receiver_offset(row)),
                      receiver);
      movl(reg2, DataLayout::counter_increment);
      set_mdp_data_at(mdp, 
                      in_bytes(VirtualCallData::receiver_count_offset(row)),
                      reg2);
      jmp(update_mdp);
      bind(next_zero_test);
    }

    bind (update_mdp);

    // The method data pointer needs to be updated to reflect the new target.
    update_mdp_by_constant(mdp, 
                           in_bytes(VirtualCallData::
                                    virtual_call_data_size()));
    bind(profile_continue);
  }
#endif // !CORE
}


void InterpreterMacroAssembler::profile_ret(Register return_bci, 
                                            Register mdp)
{
#ifndef CORE
  if (ProfileInterpreter) {
    Label profile_continue;
    uint row;

    // If no method data exists, go to profile_continue.
    test_method_data_pointer(mdp, profile_continue);

    // Update the total ret count.
    increment_mdp_data_at(mdp, in_bytes(CounterData::count_offset()));

    for (row = 0; row < RetData::row_limit(); row++) {
      Label next_test;

      // See if return_bci is equal to bci[n]:
      test_mdp_data_at(mdp, 
                       in_bytes(RetData::bci_offset(row)),
                       return_bci, 
                       next_test);

      // return_bci is equal to bci[n].  Increment the count.
      increment_mdp_data_at(mdp, in_bytes(RetData::bci_count_offset(row)));

      // The method data pointer needs to be updated to reflect the new target.
      update_mdp_by_offset(mdp, 
                           in_bytes(RetData::bci_displacement_offset(row)));
      jmp(profile_continue);
      bind(next_test);
    }

    update_mdp_for_ret(return_bci);

    bind(profile_continue);
  }
#endif // !CORE
}


void InterpreterMacroAssembler::profile_checkcast(bool is_null, Register mdp)
{
#ifndef CORE
  if (ProfileInterpreter) {
    Label profile_continue;

    // If no method data exists, go to profile_continue.
    test_method_data_pointer(mdp, profile_continue);

    if (is_null) {
      // Set the flag to true.
      set_mdp_flag_at(mdp, BitData::null_flag_constant());
    }

    // The method data pointer needs to be updated.
    update_mdp_by_constant(mdp, in_bytes(BitData::bit_data_size()));

    bind(profile_continue);
  }
#endif // !CORE
}


void InterpreterMacroAssembler::profile_switch_default(Register mdp) 
{
#ifndef CORE
  if (ProfileInterpreter) {
    Label profile_continue;

    // If no method data exists, go to profile_continue.
    test_method_data_pointer(mdp, profile_continue);

    // Update the default case count
    increment_mdp_data_at(mdp, 
                          in_bytes(MultiBranchData::default_count_offset()));

    // The method data pointer needs to be updated.
    update_mdp_by_offset(mdp, 
                         in_bytes(MultiBranchData::
                                  default_displacement_offset()));

    bind(profile_continue);
  }
#endif // !CORE
}


void InterpreterMacroAssembler::profile_switch_case(Register index,
                                                    Register mdp,
                                                    Register reg2)
{
#ifndef CORE
  if (ProfileInterpreter) {
    Label profile_continue;

    // If no method data exists, go to profile_continue.
    test_method_data_pointer(mdp, profile_continue);

    // Build the base (index * per_case_size_in_bytes()) +
    // case_array_offset_in_bytes()
    movl(reg2, in_bytes(MultiBranchData::per_case_size()));
    imulq(index, reg2); // XXX l ?
    addq(index, in_bytes(MultiBranchData::case_array_offset())); // XXX l ?

    // Update the case count
    increment_mdp_data_at(mdp, 
                          index, 
                          in_bytes(MultiBranchData::relative_count_offset())); 

    // The method data pointer needs to be updated.
    update_mdp_by_offset(mdp, 
                         index, 
                         in_bytes(MultiBranchData::
                                  relative_displacement_offset()));

    bind(profile_continue);
  }
#endif // !CORE
}


void InterpreterMacroAssembler::verify_oop(Register reg, TosState state) 
{
  if (state == atos) {
    MacroAssembler::verify_oop(reg);
  }
}

void InterpreterMacroAssembler::verify_FPU(int stack_depth, TosState state) 
{
}

 
void InterpreterMacroAssembler::notify_method_entry() 
{
  // Whenever JVMTI is interp_only_mode, method entry/exit events are sent to
  // track stack depth.  If it is possible to enter interp_only_mode we add
  // the code to check if the event should be sent.
  if (JvmtiExport::can_post_interpreter_events()) {
    Label L;
    movl(rarg2, Address(r15_thread, JavaThread::interp_only_mode_offset()));
    testl(rarg2, rarg2);
    jcc(Assembler::zero, L);
    call_VM(noreg, CAST_FROM_FN_PTR(address, 
                                    InterpreterRuntime::post_method_entry));
    bind(L);
  }
  Label E;
  Label S;
  cmpl(Address((address) 
               jvmpi::event_flags_array_at_addr(JVMPI_EVENT_METHOD_ENTRY),
               relocInfo::none),
       (int) JVMPI_EVENT_ENABLED);
  jcc(Assembler::equal, S);
  cmpl(Address((address) 
               jvmpi::event_flags_array_at_addr(JVMPI_EVENT_METHOD_ENTRY2),
               relocInfo::none),
       (int) JVMPI_EVENT_ENABLED);
  jcc(Assembler::notEqual, E);
  bind(S);
  // notify method entry
  get_method(rarg1);
  // get receiver
  xorl(rarg2, rarg2);             // receiver = NULL for a static method
  movl(rax, Address(rarg1, methodOopDesc::access_flags_offset()));
  testl(rax, JVM_ACC_STATIC); // check if method is static
  cmovq(Assembler::zero, rarg2, Address(r14));    // otherwise get receiver
  call_VM(noreg, 
          CAST_FROM_FN_PTR(address, 
                           SharedRuntime::jvmpi_method_entry), rarg1, rarg2);
  bind(E);
}

 
void InterpreterMacroAssembler::notify_method_exit(TosState state) 
{
  // Whenever JVMTI is interp_only_mode, method entry/exit events are sent to
  // track stack depth.  If it is possible to enter interp_only_mode we add
  // the code to check if the event should be sent.
  if (JvmtiExport::can_post_interpreter_events()) {
    Label L;
    // Note: frame::interpreter_frame_result has a dependency on how the 
    // method result is saved across the call to post_method_exit. If this
    // is changed then the interpreter_frame_result implementation will
    // need to be updated too.
    push(state);
    movl(rarg2, Address(r15_thread, JavaThread::interp_only_mode_offset()));
    testl(rarg2, rarg2);
    jcc(Assembler::zero, L);
    call_VM(noreg, 
            CAST_FROM_FN_PTR(address, InterpreterRuntime::post_method_exit));
    bind(L);
    pop(state);     
  }
  notify_jvmpi_method_exit(state);
}
 
void InterpreterMacroAssembler::notify_jvmpi_method_exit(TosState state) 
{
  Label E;
  cmpl(Address((address) 
               jvmpi::event_flags_array_at_addr(JVMPI_EVENT_METHOD_EXIT),
               relocInfo::none),
       (int) JVMPI_EVENT_ENABLED);
  jcc(Assembler::notEqual, E);

  // notify method exit
  push(state);
  get_method(rarg1);
  call_VM(noreg, 
          CAST_FROM_FN_PTR(address, SharedRuntime::jvmpi_method_exit), rarg1);
  pop(state);
  bind(E);
}
