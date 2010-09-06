#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)interp_masm_i486.cpp	1.135 04/03/22 19:28:33 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_interp_masm_i486.cpp.incl"


// Implementation of InterpreterMacroAssembler

void InterpreterMacroAssembler::call_VM_leaf_base(
  address entry_point,
  int     number_of_arguments
) {
  // interpreter specific
  //
  // Note: No need to save/restore bcp & locals (esi & edi) pointer
  //       since these are callee saved registers and no blocking/
  //       GC can happen in leaf calls.
#ifdef ASSERT
  save_bcp();
#endif
  // super call
  MacroAssembler::call_VM_leaf_base(entry_point, number_of_arguments);
  // interpreter specific
#ifdef ASSERT
  { Label L;
    cmpl(esi, Address(ebp, frame::interpreter_frame_bcx_offset * wordSize));
    jcc(Assembler::equal, L);
    stop("InterpreterMacroAssembler::call_VM_leaf_base: esi not callee saved?");
    bind(L);
  }
  { Label L;
    cmpl(edi, Address(ebp, frame::interpreter_frame_locals_offset * wordSize));
    jcc(Assembler::equal, L);
    stop("InterpreterMacroAssembler::call_VM_leaf_base: edi not callee saved?");
    bind(L);
  }
#endif
}


void InterpreterMacroAssembler::call_VM_base(
  Register oop_result,
  Register java_thread,
  Register last_java_sp,
  address  entry_point,
  int      number_of_arguments,
  bool     check_exceptions
) {
  // interpreter specific
  //
  // Note: Could avoid restoring locals ptr (callee saved) - however doesn't
  //       really make a difference for these runtime calls, since they are
  //       slow anyway. Btw., bcp must be saved/restored since it may change
  //       due to GC.
  assert(java_thread == noreg , "not expecting a precomputed java thread");
  save_bcp();
  // super call
  MacroAssembler::call_VM_base(oop_result, java_thread, last_java_sp, entry_point, number_of_arguments, check_exceptions);
  // interpreter specific
  restore_bcp();
  restore_locals();
}


void InterpreterMacroAssembler::check_and_handle_popframe(Register java_thread) {
  if (JvmtiExport::can_pop_frame()) {
    Label L;
    // Initiate popframe handling only if it is not already being processed.  If the flag
    // has the popframe_processing bit set, it means that this code is called *during* popframe
    // handling - we don't want to reenter.
    Register pop_cond = java_thread;  // Not clear if any other register is available...
    movl(pop_cond, Address(java_thread, JavaThread::popframe_condition_offset()));
    testl(pop_cond, JavaThread::popframe_pending_bit);
    jcc(Assembler::zero, L);
    testl(pop_cond, JavaThread::popframe_processing_bit);
    jcc(Assembler::notZero, L);   
    call( CAST_FROM_FN_PTR(address, Interpreter::remove_activation_preserving_args_entry), relocInfo::runtime_call_type);
    jmp(eax, relocInfo::runtime_call_type);
    bind(L);
    get_thread(java_thread);
  }
}


void InterpreterMacroAssembler::get_unsigned_2_byte_index_at_bcp(Register reg, int bcp_offset) {
  assert(bcp_offset >= 0, "bcp is still pointing to start of bytecode");
  movl(reg, Address(esi, bcp_offset));
  bswap(reg);
  shrl(reg, 16);
}


void InterpreterMacroAssembler::get_cache_and_index_at_bcp(Register cache, Register index, int bcp_offset) {
  assert(bcp_offset > 0, "bcp is still pointing to start of bytecode");
  assert(cache != index, "must use different registers");
  load_unsigned_word(index, Address(esi, bcp_offset));
  movl(cache, Address(ebp, frame::interpreter_frame_cache_offset * wordSize));
  assert(sizeof(ConstantPoolCacheEntry) == 4*wordSize, "adjust code below");
  shll(index, 2); // convert from field index to ConstantPoolCacheEntry index
}


void InterpreterMacroAssembler::get_cache_entry_pointer_at_bcp(Register cache, Register tmp, int bcp_offset) {
  assert(bcp_offset > 0, "bcp is still pointing to start of bytecode");
  assert(cache != tmp, "must use different register");
  load_unsigned_word(tmp, Address(esi, bcp_offset));
  assert(sizeof(ConstantPoolCacheEntry) == 4*wordSize, "adjust code below");
                               // convert from field index to ConstantPoolCacheEntry index
                               // and from word offset to byte offset
  shll(tmp, 2 + LogBytesPerWord);
  movl(cache, Address(ebp, frame::interpreter_frame_cache_offset * wordSize));
                               // skip past the header
  addl(cache, in_bytes(constantPoolCacheOopDesc::base_offset()));
  addl(cache, tmp);            // construct pointer to cache entry
}


  // Generate a subtype check: branch to ok_is_subtype if sub_klass is
  // a subtype of super_klass.  EAX holds the super_klass.  Blows ECX.
  // Resets EDI to locals.  Register sub_klass cannot be any of the above.
void InterpreterMacroAssembler::gen_subtype_check( Register Rsub_klass, Label &ok_is_subtype ) {
  assert( Rsub_klass != eax, "eax holds superklass" );
  assert( Rsub_klass != ecx, "ecx holds 2ndary super array length" );
  assert( Rsub_klass != edi, "edi holds 2ndary super array scan ptr" );
  Label not_subtype, loop;

  // Load the super-klass's check offset into ECX
  movl( ecx, Address(eax, sizeof(oopDesc) + Klass::super_check_offset_offset_in_bytes() ) );
  // Load from the sub-klass's super-class display list, or a 1-word cache of
  // the secondary superclass list, or a failing value with a sentinel offset
  // if the super-klass is an interface or exceptionally deep in the Java
  // hierarchy and we have to scan the secondary superclass list the hard way.
  // See if we get an immediate positive hit
  cmpl( eax, Address(Rsub_klass,ecx,Address::times_1) );
  jcc( Assembler::equal,ok_is_subtype );

  // Check for immediate negative hit
  cmpl( ecx, sizeof(oopDesc) + Klass::secondary_super_cache_offset_in_bytes() );
  jcc( Assembler::notEqual, not_subtype );
  // Check for self
  cmpl( Rsub_klass, eax );
  jcc( Assembler::equal, ok_is_subtype );

  // Now do a linear scan of the secondary super-klass chain.
  movl( edi, Address(Rsub_klass, sizeof(oopDesc) + Klass::secondary_supers_offset_in_bytes()) );
  // EDI holds the objArrayOop of secondary supers.
  movl( ecx, Address(edi, arrayOopDesc::length_offset_in_bytes()));// Load the array length
  // Skip to start of data; also clear Z flag incase ECX is zero
  addl( edi, arrayOopDesc::base_offset_in_bytes(T_OBJECT) );
  // Scan ECX words at [EDI] for occurance of EAX
  // Set NZ/Z based on last compare
  repne_scan();
  restore_locals();           // Restore EDI; Must not blow flags
  // Not equal?
  jcc( Assembler::notEqual, not_subtype );
  // Must be equal but missed in cache.  Update cache.
  movl( Address(Rsub_klass, sizeof(oopDesc) + Klass::secondary_super_cache_offset_in_bytes()), eax );
  jmp( ok_is_subtype );

  bind(not_subtype);
}

void InterpreterMacroAssembler::f2ieee() {
  if (IEEEPrecision) {
    fstp_s(Address(esp));
    fld_s(Address(esp));
  }
}


void InterpreterMacroAssembler::d2ieee() {
  if (IEEEPrecision) {
    fstp_d(Address(esp));
    fld_d(Address(esp));
  }
}


void InterpreterMacroAssembler::pop(TosState state) {
  switch (state) {
    case atos: /* treat the same as itos */                  // fall through
    case btos:						     // fall through
    case ctos:						     // fall through
    case stos:						     // fall through
    case itos: popl(eax);                                    break;
    case ltos: popl(eax);           popl(edx);               break;
    case ftos: fld_s(Address(esp)); addl(esp, 1 * wordSize); break;
    case dtos: fld_d(Address(esp)); addl(esp, 2 * wordSize); break;
    case vtos: /* nothing to do */                           break;
    default  : ShouldNotReachHere();
  }
  verify_oop(eax, state);
}


void InterpreterMacroAssembler::push(TosState state) {
  verify_oop(eax, state);
  switch (state) {
    case atos: /* treat the same as itos */                   // fall through
    case btos:						     // fall through
    case ctos:						     // fall through
    case stos:						     // fall through
    case itos: pushl(eax);                                    break;
    case ltos: pushl(edx);              pushl(eax);           break;
    case ftos: subl(esp, 1 * wordSize); fstp_s(Address(esp)); break; // Do not schedule for no AGI! Never write beyond esp!
    case dtos: subl(esp, 2 * wordSize); fstp_d(Address(esp)); break; // Do not schedule for no AGI! Never write beyond esp!
    case vtos: /* nothing to do */                            break;
    default  : ShouldNotReachHere();
  }
}


void InterpreterMacroAssembler::super_call_VM_leaf(address entry_point) {
  MacroAssembler::call_VM_leaf_base(entry_point, 0);
}


void InterpreterMacroAssembler::super_call_VM_leaf(address entry_point, Register arg_1) {
  pushl(arg_1);  
  MacroAssembler::call_VM_leaf_base(entry_point, 1);
}


void InterpreterMacroAssembler::super_call_VM_leaf(address entry_point, Register arg_1, Register arg_2) {
  pushl(arg_2);
  pushl(arg_1);
  MacroAssembler::call_VM_leaf_base(entry_point, 2);
}


void InterpreterMacroAssembler::super_call_VM_leaf(address entry_point, Register arg_1, Register arg_2, Register arg_3) {
  pushl(arg_3);
  pushl(arg_2);
  pushl(arg_1);
  MacroAssembler::call_VM_leaf_base(entry_point, 3);
}


void InterpreterMacroAssembler::super_call_VM(Register oop_result, Register last_java_sp, address entry_point, Register arg_1, Register arg_2) {
  pushl(arg_2);  
  pushl(arg_1);
  MacroAssembler::call_VM_base(oop_result, noreg, last_java_sp, entry_point, 2, true);
}


// The following two routines provide a hook so that an implementation
// can schedule the dispatch in two parts.  Intel does not do this.
void InterpreterMacroAssembler::dispatch_prolog(TosState state, int step) {
  // Nothing Intel-specific to be done here.
}

void InterpreterMacroAssembler::dispatch_epilog(TosState state, int step) {
  dispatch_next(state, step);
}

void InterpreterMacroAssembler::dispatch_base(TosState state, address* table,
                                              bool verifyoop) {
  verify_FPU(1, state);
  if (VerifyActivationFrameSize) {
    Label L;
    movl(ecx, ebp);
    subl(ecx, esp);
    int min_frame_size = (frame::link_offset - frame::interpreter_frame_initial_sp_offset) * wordSize;
    cmpl(ecx, min_frame_size);
    jcc(Assembler::greaterEqual, L);
    stop("broken stack frame");
    bind(L);
  }
  if (verifyoop) verify_oop(eax, state);
  jmp(Address(noreg, ebx, Address::times_4, (int)table));
}


void InterpreterMacroAssembler::dispatch_only(TosState state) {
  dispatch_base(state, Interpreter::dispatch_table(state));
}


void InterpreterMacroAssembler::dispatch_only_normal(TosState state) {
  dispatch_base(state, Interpreter::normal_table(state));
}

void InterpreterMacroAssembler::dispatch_only_noverify(TosState state) {
  dispatch_base(state, Interpreter::normal_table(state), false);
}


void InterpreterMacroAssembler::dispatch_next(TosState state, int step) {
  // load next bytecode (load before advancing esi to prevent AGI)
  load_unsigned_byte(ebx, Address(esi, step));
  // advance esi
  increment(esi, step);
  dispatch_base(state, Interpreter::dispatch_table(state));
}


void InterpreterMacroAssembler::dispatch_via(TosState state, address* table) {
  // load current bytecode
  load_unsigned_byte(ebx, Address(esi));
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
void InterpreterMacroAssembler::remove_activation(TosState state, Register ret_addr,
                                                  bool throw_monitor_exception,
                                                  bool install_monitor_exception,
                                                  bool notify_jvmdi) {
  // Note: Registers eax, edx and FPU ST(0) may be in use for the result
  // check if synchronized method  
  Label unlocked, unlock, no_unlock;

  // get the value of _do_not_unlock_if_synchronized into edi
  get_thread(ecx);
  const Address do_not_unlock_if_synchronized(ecx,
    in_bytes(JavaThread::do_not_unlock_if_synchronized_offset()));
  movl(edi, do_not_unlock_if_synchronized);
  movl(do_not_unlock_if_synchronized, (int)false); // reset the flag

  movl(ebx, Address(ebp, frame::interpreter_frame_method_offset * wordSize)); // get method access flags
  movl(ecx, Address(ebx, methodOopDesc::access_flags_offset()));
  testl(ecx, JVM_ACC_SYNCHRONIZED);
  jcc(Assembler::zero, unlocked);
 
  // Don't unlock anything if the _do_not_unlock_if_synchronized flag
  // is set.
  testl(edi, edi);
  jcc(Assembler::notZero, no_unlock);
 
  // unlock monitor
  push(state);                                   // save result
    
  // BasicObjectLock will be first in list, since this is a synchronized method. However, need
  // to check that the object has not been unlocked by an explicit monitorexit bytecode.  
  const Address monitor(ebp, frame::interpreter_frame_initial_sp_offset * wordSize - (int)sizeof(BasicObjectLock));
  leal  (edx, monitor);                          // address of first monitor
  
  movl  (eax, Address(edx, BasicObjectLock::obj_offset_in_bytes()));
  testl (eax, eax);
  jcc   (Assembler::notZero, unlock);
                                  
  pop(state);
  if (throw_monitor_exception) {
    // Entry already unlocked, need to throw exception
    call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::throw_illegal_monitor_state_exception));
    should_not_reach_here();
  } else {
    // Monitor already unlocked during a stack unroll. 
    // If requested, install an illegal_monitor_state_exception.
    // Continue with stack unrolling.
    if (install_monitor_exception) {
      call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::new_illegal_monitor_state_exception));
    }
    jmp(unlocked);
  }

  bind(unlock);  
  unlock_object(edx);              
  pop(state);

  // Check that for block-structured locking (i.e., that all locked objects has been unlocked)  
  bind(unlocked);  

  // eax, edx: Might contain return value

  // Check that all monitors are unlocked
  {
    Label loop, exception, entry, restart;
    const int entry_size               = frame::interpreter_frame_monitor_size()           * wordSize;
    const Address monitor_block_top(ebp, frame::interpreter_frame_monitor_block_top_offset * wordSize);
    const Address monitor_block_bot(ebp, frame::interpreter_frame_initial_sp_offset        * wordSize);
    
    bind(restart);
    movl(ecx, monitor_block_top);             // points to current entry, starting with top-most entry
    leal(ebx, monitor_block_bot);             // points to word before bottom of monitor block
    jmp(entry);
          
    // Entry already locked, need to throw exception
    bind(exception); 

    if (throw_monitor_exception) {
      // Throw exception      
      MacroAssembler::call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::throw_illegal_monitor_state_exception));
      should_not_reach_here();
    } else {
      // Stack unrolling. Unlock object and install illegal_monitor_exception
      // Unlock does not block, so don't have to worry about the frame
      // We don't have to preserve eax, edx since we are going to throw an exception

      movl(edx, ecx);
      unlock_object(edx);
      
      if (install_monitor_exception) {
        call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::new_illegal_monitor_state_exception));
      }

      jmp(restart);
    }
  
    bind(loop);
    cmpl(Address(ecx, BasicObjectLock::obj_offset_in_bytes()), (int)NULL);  // check if current entry is used
    jcc(Assembler::notEqual, exception);
          
    addl(ecx, entry_size);                       // otherwise advance to next entry
    bind(entry);
    cmpl(ecx, ebx);                              // check if bottom reached
    jcc(Assembler::notEqual, loop);              // if not at bottom then check this entry      
  }        

  bind(no_unlock);

  // jvmpi support (jvmdi does not generate MethodExit on exception / popFrame)
  if (notify_jvmdi) {
    notify_method_exit(state);                     // preserve TOSCA
  } else {
    notify_jvmpi_method_exit(state);               // preserve TOSCA
  }

  // remove activation
  movl(ebx, Address(ebp, frame::interpreter_frame_sender_sp_offset * wordSize)); // get sender sp
  leave();                                     // remove frame anchor
  popl(ret_addr);                              // get return address
  movl(esp, ebx);                              // set sp to sender sp
}


// Lock object
//
// Argument: edx : Points to BasicObjectLock to be used for locking. Must
// be initialized with object to lock
void InterpreterMacroAssembler::lock_object(Register lock_reg) {
  assert(lock_reg == edx, "The argument is only for looks. It must be edx");

  if (UseHeavyMonitors) {
    call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::monitorenter), lock_reg);
  } else {

    Label done;

    const Register swap_reg = eax;  // Must use eax for cmpxchg instruction
    const Register obj_reg  = ecx;  // Will contain the oop

    const int obj_offset = BasicObjectLock::obj_offset_in_bytes();
    const int lock_offset = BasicObjectLock::lock_offset_in_bytes ();
    const int mark_offset = lock_offset + BasicLock::displaced_header_offset_in_bytes(); 

    // Load object pointer into obj_reg %ecx
    movl(obj_reg, Address(lock_reg, obj_offset));

   // Load immediate 1 into swap_reg %eax
    movl(swap_reg, 1);

    // Load (object->mark() | 1) into swap_reg %eax
    orl(swap_reg, Address(obj_reg));

    // Save (object->mark() | 1) into BasicLock's displaced header
    movl(Address(lock_reg, mark_offset), swap_reg);

    assert(lock_offset == 0, "displached header must be first word in BasicObjectLock");
    if (os::is_MP()) {
      lock();
    }    
    cmpxchg(lock_reg, Address(obj_reg));    
    jcc(Assembler::zero, done);

    // Test if the oopMark is an obvious stack pointer, i.e.,
    //  1) (mark & 3) == 0, and
    //  2) esp <= mark < mark + os::pagesize()
    //
    // These 3 tests can be done by evaluating the following 
    // expression: ((mark - esp) & (3 - os::vm_page_size())),
    // assuming both stack pointer and pagesize have their
    // least significant 2 bits clear.
    // NOTE: the oopMark is in swap_reg %eax as the result of cmpxchg
    subl(swap_reg, esp);
    andl(swap_reg, 3 - os::vm_page_size());

    // Save the test result, for recursive case, the result is zero
    movl(Address(lock_reg, mark_offset), swap_reg);

    jcc(Assembler::zero, done);

    // Call the runtime routine for slow case
    call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::monitorenter), lock_reg);

    bind(done);
  }   
}


// Unlocks an object. Used in monitorexit bytecode and remove_activation.
//
// Argument: edx : Points to BasicObjectLock structure for lock
// Throw an IllegalMonitorException if object is not locked by current thread
// 
// Uses: eax, ebx, ecx, edx
void InterpreterMacroAssembler::unlock_object(Register lock_reg) {
  assert(lock_reg == edx, "The argument is only for looks. It must be edx");

  if (UseHeavyMonitors) {
    call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::monitorexit), lock_reg);
  } else {
    Label done;

    const Register swap_reg   = eax;  // Must use eax for cmpxchg instruction
    const Register header_reg = ebx;  // Will contain the old oopMark
    const Register obj_reg    = ecx;  // Will contain the oop

    save_bcp(); // Save in case of exception

    // Convert from BasicObjectLock structure to object and BasicLock structure
    // Store the BasicLock address into %eax
    leal(swap_reg, Address(lock_reg, BasicObjectLock::lock_offset_in_bytes()));

    // Load oop into obj_reg(%ecx)
    movl(obj_reg, Address(lock_reg, BasicObjectLock::obj_offset_in_bytes ()));

    // Load the old header from BasicLock structure
    movl(header_reg, Address(swap_reg, BasicLock::displaced_header_offset_in_bytes()));

    // Free entry
    movl(Address(lock_reg, BasicObjectLock::obj_offset_in_bytes()), (int)NULL);

    // Test for recursion
    testl(header_reg, header_reg);

    // zero for recursive case
    jcc(Assembler::zero, done);
    
    // Atomic swap back the old header
    if (os::is_MP()) lock();
    cmpxchg(header_reg, Address(obj_reg));

    // zero for recursive case
    jcc(Assembler::equal, done);

    // Call the runtime routine for slow case.
    movl(Address(lock_reg, BasicObjectLock::obj_offset_in_bytes()), obj_reg); // restore obj
    call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::monitorexit), lock_reg);

    bind(done);

    restore_bcp();
  }
}


#ifndef CORE
// Test ImethodDataPtr.  If it is null, continue at the specified label
void InterpreterMacroAssembler::test_method_data_pointer(Register mdp, Label& zero_continue) {
  assert(ProfileInterpreter, "must be profiling interpreter");
  movl(mdp, Address(ebp, frame::interpreter_frame_mdx_offset * wordSize));
  testl(mdp, mdp);
  jcc(Assembler::zero, zero_continue);
}


// Set the method data pointer for the current bcp.
void InterpreterMacroAssembler::set_method_data_pointer_for_bcp() {
  assert(ProfileInterpreter, "must be profiling interpreter");
  Label zero_continue;
  pushl(eax);
  pushl(ebx);

  get_method(ebx);
  // Test MDO to avoid the call if it is NULL.
  movl(eax, Address(ebx, in_bytes(methodOopDesc::method_data_offset())));
  testl(eax, eax);
  jcc(Assembler::zero, zero_continue);

  // ebx: method
  // esi: bcp
  call_VM_leaf(CAST_FROM_FN_PTR(address, InterpreterRuntime::bcp_to_di), ebx, esi);
  // eax: mdi

  movl(ebx, Address(ebx, in_bytes(methodOopDesc::method_data_offset())));
  testl(ebx, ebx);
  jcc(Assembler::zero, zero_continue);
  addl(ebx, in_bytes(methodDataOopDesc::data_offset()));
  addl(ebx, eax);
  movl(Address(ebp, frame::interpreter_frame_mdx_offset * wordSize), ebx);

  bind(zero_continue);
  popl(ebx);
  popl(eax);
}

void InterpreterMacroAssembler::verify_method_data_pointer() {
  assert(ProfileInterpreter, "must be profiling interpreter");
#ifdef ASSERT
  Label verify_continue;
  pushl(eax);
  pushl(ebx);
  pushl(ecx);
  pushl(edx);
  test_method_data_pointer(ecx, verify_continue); // If mdp is zero, continue
  get_method(ebx);

  // If the mdp is valid, it will point to a DataLayout header which is
  // consistent with the bcp.  The converse is highly probable also.
  load_unsigned_word(edx, Address(ecx, in_bytes(DataLayout::bci_offset())));
  addl(edx, Address(ebx, methodOopDesc::const_offset()));
  leal(edx, Address(edx, constMethodOopDesc::codes_offset()));
  cmpl(edx, esi);
  jcc(Assembler::equal, verify_continue);
  // ebx: method
  // esi: bcp
  // ecx: mdp
  call_VM_leaf(CAST_FROM_FN_PTR(address, InterpreterRuntime::verify_mdp), ebx, esi, ecx);
  bind(verify_continue);
  popl(edx);
  popl(ecx);
  popl(ebx);
  popl(eax);
#endif // ASSERT
}


void InterpreterMacroAssembler::set_mdp_data_at(Register mdp_in, int constant, Register value) {
  assert(ProfileInterpreter, "must be profiling interpreter");
  Address data(mdp_in, constant);
  movl(data, value);
}


void InterpreterMacroAssembler::increment_mdp_data_at(Register mdp_in, int constant) {
  assert(ProfileInterpreter, "must be profiling interpreter");

  // Counter address
  Address data(mdp_in, constant);

  assert( DataLayout::counter_increment==1, "flow-free idiom only works with 1" );
  // Increment the register.  Set carry flag.
  addl(data, DataLayout::counter_increment);
  // If the increment causes the counter to overflow, pull back by 1.
  sbbl(data, 0);
}


void InterpreterMacroAssembler::increment_mdp_data_at(Register mdp_in,
                                                      Register reg,
                                                      int constant) {
  assert(ProfileInterpreter, "must be profiling interpreter");

  Address data(mdp_in, reg, Address::times_1, constant);

  assert( DataLayout::counter_increment==1, "flow-free idiom only works with 1" );
  // Increment the register.  Set carry flag.
  addl(data, DataLayout::counter_increment);
  // If the increment causes the counter to overflow, pull back by 1.
  sbbl(data, 0);
}


void InterpreterMacroAssembler::set_mdp_flag_at(Register mdp_in, int flag_constant) {
  assert(ProfileInterpreter, "must be profiling interpreter");
  // Set the flag
  orl(Address(mdp_in, in_bytes(ProfileData::header_offset())), flag_constant);
}



void InterpreterMacroAssembler::test_mdp_data_at(Register mdp_in,
                                                 int offset,
                                                 Register value,
                                                 Label& not_equal_continue) {
  assert(ProfileInterpreter, "must be profiling interpreter");
  cmpl(value, Address(mdp_in, offset));
  jcc(Assembler::notEqual, not_equal_continue);
}


void InterpreterMacroAssembler::update_mdp_by_offset(Register mdp_in, int offset_of_disp) {
  assert(ProfileInterpreter, "must be profiling interpreter");
  Address disp_address(mdp_in, offset_of_disp);
  addl(mdp_in,disp_address);
  movl(Address(ebp, frame::interpreter_frame_mdx_offset * wordSize), mdp_in);
}


void InterpreterMacroAssembler::update_mdp_by_offset(Register mdp_in, Register reg, int offset_of_disp) {
  assert(ProfileInterpreter, "must be profiling interpreter");
  Address disp_address(mdp_in, reg, Address::times_1, offset_of_disp);
  addl(mdp_in, disp_address);
  movl(Address(ebp, frame::interpreter_frame_mdx_offset * wordSize), mdp_in);
}


void InterpreterMacroAssembler::update_mdp_by_constant(Register mdp_in, int constant) {
  assert(ProfileInterpreter, "must be profiling interpreter");
  addl(mdp_in, constant);
  movl(Address(ebp, frame::interpreter_frame_mdx_offset * wordSize), mdp_in);
}


void InterpreterMacroAssembler::update_mdp_for_ret(Register return_bci) {
  assert(ProfileInterpreter, "must be profiling interpreter");
  pushl(return_bci);             // save/restore across call_VM
  call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::update_mdp_for_ret), return_bci);
  popl(return_bci);
}
#endif // !CORE


void InterpreterMacroAssembler::profile_taken_branch(Register mdp, Register bumped_count) {
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
    movl(bumped_count,data);
    assert( DataLayout::counter_increment==1, "flow-free idiom only works with 1" );
    addl(bumped_count, DataLayout::counter_increment);
    sbbl(bumped_count, 0);
    movl(data,bumped_count);    // Store back out

    // The method data pointer needs to be updated to reflect the new target.
    update_mdp_by_offset(mdp, in_bytes(JumpData::displacement_offset()));
    bind (profile_continue);
  }
#endif // !CORE
}


void InterpreterMacroAssembler::profile_not_taken_branch(Register mdp) {
#ifndef CORE
  if (ProfileInterpreter) {
    Label profile_continue;

    // If no method data exists, go to profile_continue.
    test_method_data_pointer(mdp, profile_continue);

    // We are taking a branch.  Increment the not taken count.
    increment_mdp_data_at(mdp, in_bytes(BranchData::not_taken_offset()));

    // The method data pointer needs to be updated to correspond to the next bytecode
    update_mdp_by_constant(mdp, in_bytes(BranchData::branch_data_size()));
    bind (profile_continue);
  }
#endif // !CORE
}


void InterpreterMacroAssembler::profile_call(Register mdp) {
#ifndef CORE
  if (ProfileInterpreter) {
    Label profile_continue;

    // If no method data exists, go to profile_continue.
    test_method_data_pointer(mdp, profile_continue);

    // We are making a call.  Increment the count.
    increment_mdp_data_at(mdp, in_bytes(CounterData::count_offset()));

    // The method data pointer needs to be updated to reflect the new target.
    update_mdp_by_constant(mdp, in_bytes(CounterData::counter_data_size()));
    bind (profile_continue);
  }
#endif // !CORE
}


void InterpreterMacroAssembler::profile_final_call(Register mdp) {
#ifndef CORE
  if (ProfileInterpreter) {
    Label profile_continue;

    // If no method data exists, go to profile_continue.
    test_method_data_pointer(mdp, profile_continue);

    // We are making a call.  Increment the count.
    increment_mdp_data_at(mdp, in_bytes(CounterData::count_offset()));

    // The method data pointer needs to be updated to reflect the new target.
    update_mdp_by_constant(mdp, in_bytes(VirtualCallData::virtual_call_data_size()));
    bind (profile_continue);
  }
#endif // !CORE
}


void InterpreterMacroAssembler::profile_virtual_call(Register receiver, Register mdp, Register reg2) {
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
      test_mdp_data_at(mdp, in_bytes(VirtualCallData::receiver_offset(row)),
		       receiver, next_test);

      // The receiver is receiver[n].  Increment count[n].
      increment_mdp_data_at(mdp, in_bytes(VirtualCallData::receiver_count_offset(row)));
      jmp(update_mdp);
      bind(next_test);
    }

    // Zero out reg2.
    xorl(reg2, reg2);

    for (row = 0; row < VirtualCallData::row_limit(); row++) {
      Label next_zero_test;

      // See if receiver[n] is null:
      test_mdp_data_at(mdp, in_bytes(VirtualCallData::receiver_offset(row)),
		       reg2, next_zero_test);

      // receiver[n] is NULL.  Fill in the receiver field and increment the count.
      set_mdp_data_at(mdp, in_bytes(VirtualCallData::receiver_offset(row)), receiver);
      movl(reg2, DataLayout::counter_increment);
      set_mdp_data_at(mdp, in_bytes(VirtualCallData::receiver_count_offset(row)), reg2);
      jmp(update_mdp);
      bind(next_zero_test);
    }

    bind (update_mdp);

    // The method data pointer needs to be updated to reflect the new target.
    update_mdp_by_constant(mdp, in_bytes(VirtualCallData::virtual_call_data_size()));
    bind (profile_continue);
  }
#endif // !CORE
}


void InterpreterMacroAssembler::profile_ret(Register return_bci, Register mdp) {
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
      test_mdp_data_at(mdp, in_bytes(RetData::bci_offset(row)), return_bci, next_test);

      // return_bci is equal to bci[n].  Increment the count.
      increment_mdp_data_at(mdp, in_bytes(RetData::bci_count_offset(row)));

      // The method data pointer needs to be updated to reflect the new target.
      update_mdp_by_offset(mdp, in_bytes(RetData::bci_displacement_offset(row)));
      jmp(profile_continue);
      bind(next_test);
    }

    update_mdp_for_ret(return_bci);

    bind (profile_continue);
  }
#endif // !CORE
}


void InterpreterMacroAssembler::profile_checkcast(bool is_null, Register mdp) {
#ifndef CORE
  if (ProfileInterpreter) {
    Label profile_continue;

    // If no method data exists, go to profile_continue.
    test_method_data_pointer(mdp, profile_continue);

    if (is_null)                // Set the flag to true.
      set_mdp_flag_at(mdp, BitData::null_flag_constant());

    // The method data pointer needs to be updated.
    update_mdp_by_constant(mdp, in_bytes(BitData::bit_data_size()));

    bind (profile_continue);
  }
#endif // !CORE
}


void InterpreterMacroAssembler::profile_switch_default(Register mdp) {
#ifndef CORE
  if (ProfileInterpreter) {
    Label profile_continue;

    // If no method data exists, go to profile_continue.
    test_method_data_pointer(mdp, profile_continue);

    // Update the default case count
    increment_mdp_data_at(mdp, in_bytes(MultiBranchData::default_count_offset()));

    // The method data pointer needs to be updated.
    update_mdp_by_offset(mdp, in_bytes(MultiBranchData::default_displacement_offset()));

    bind (profile_continue);
  }
#endif // !CORE
}


void InterpreterMacroAssembler::profile_switch_case(Register index, Register mdp, Register reg2) {
#ifndef CORE
  if (ProfileInterpreter) {
    Label profile_continue;

    // If no method data exists, go to profile_continue.
    test_method_data_pointer(mdp, profile_continue);

    // Build the base (index * per_case_size_in_bytes()) + case_array_offset_in_bytes()  
    movl(reg2, in_bytes(MultiBranchData::per_case_size()));
    imull(index, reg2);
    addl(index, in_bytes(MultiBranchData::case_array_offset()));

    // Update the case count
    increment_mdp_data_at(mdp, index, in_bytes(MultiBranchData::relative_count_offset()));  

    // The method data pointer needs to be updated.
    update_mdp_by_offset(mdp, index, in_bytes(MultiBranchData::relative_displacement_offset()));

    bind (profile_continue);
  }
#endif // !CORE
}



void InterpreterMacroAssembler::verify_oop(Register reg, TosState state) {
  if (state == atos) MacroAssembler::verify_oop(reg);
}


void InterpreterMacroAssembler::verify_FPU(int stack_depth, TosState state) {
  if (state == ftos || state == dtos) MacroAssembler::verify_FPU(stack_depth);
}

 
void InterpreterMacroAssembler::notify_method_entry() {
  // Whenever JVMTI is interp_only_mode, method entry/exit events are sent to
  // track stack depth.  If it is possible to enter interp_only_mode we add
  // the code to check if the event should be sent.
  if (JvmtiExport::can_post_interpreter_events()) {
    Label L;
    get_thread(ecx);
    movl(ecx, Address(ecx, JavaThread::interp_only_mode_offset()));
    testl(ecx,ecx);
    jcc(Assembler::zero, L);
    call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::post_method_entry));
    bind(L);
  }
  { Label E;
    Label S;
    cmpl(Address((int)jvmpi::event_flags_array_at_addr(JVMPI_EVENT_METHOD_ENTRY ), relocInfo::none), (int)JVMPI_EVENT_ENABLED);
    jcc(Assembler::equal, S);
    cmpl(Address((int)jvmpi::event_flags_array_at_addr(JVMPI_EVENT_METHOD_ENTRY2), relocInfo::none), (int)JVMPI_EVENT_ENABLED);
    jcc(Assembler::notEqual, E);
    bind(S);
    // notify method entry
    { get_method(ebx);
      // get receiver
      { Label L;
        xorl(ecx, ecx);             // receiver = NULL for a static method
        movl(eax, Address(ebx, methodOopDesc::access_flags_offset()));
        testl(eax, JVM_ACC_STATIC); // check if method is static
        jcc(Assembler::notZero, L); // if static we're done
        movl(ecx, Address(edi));    // otherwise get receiver
        bind(L);
      }
      call_VM(noreg, CAST_FROM_FN_PTR(address, SharedRuntime::jvmpi_method_entry), ebx, ecx);
    }
    bind(E);
  }
}

 
void InterpreterMacroAssembler::notify_method_exit(TosState state) {
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
    get_thread(ecx);
    movl(ecx, Address(ecx, JavaThread::interp_only_mode_offset()));
    testl(ecx,ecx);
    jcc(Assembler::zero, L);
    call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::post_method_exit));
    bind(L);
    pop(state);     
  }
  notify_jvmpi_method_exit(state);
}
 
void InterpreterMacroAssembler::notify_jvmpi_method_exit(TosState state) {
  { Label E;
    cmpl(Address((int)jvmpi::event_flags_array_at_addr(JVMPI_EVENT_METHOD_EXIT), relocInfo::none), (int)JVMPI_EVENT_ENABLED);
    jcc(Assembler::notEqual, E);
    // notify method exit
    { push(state);
      get_method(ebx);
      call_VM(noreg, CAST_FROM_FN_PTR(address, SharedRuntime::jvmpi_method_exit), ebx);
      pop(state);
    }
    bind(E);
  }
}
