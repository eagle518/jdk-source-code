#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)interp_masm_sparc.cpp	1.170 04/03/22 19:28:21 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_interp_masm_sparc.cpp.incl"
#include <v9/sys/psr_compat.h>

#ifndef FAST_DISPATCH
#define FAST_DISPATCH 1
#endif
#undef FAST_DISPATCH

// Implementation of InterpreterMacroAssembler

// This file specializes the assember with interpreter-specific macros

const Address InterpreterMacroAssembler::l_tmp( FP, 0,  (frame::interpreter_frame_l_scratch_fp_offset    * wordSize ) + STACK_BIAS);
const Address InterpreterMacroAssembler::d_tmp( FP, 0,  (frame::interpreter_frame_d_scratch_fp_offset    * wordSize) + STACK_BIAS);


void InterpreterMacroAssembler::compute_extra_locals_size_in_bytes(Register args_size, Register locals_size, Register delta) {
  // Note: this algorithm is also used by C1's OSR entry sequence.
  // Any changes should also be applied to CodeEmitter::emit_osr_entry().
  assert_different_registers(args_size, locals_size);
  subcc(locals_size, args_size, delta);// extra space for non-arguments locals in words
  // Use br/mov combination because it works on both V8 and V9 and is
  // faster.
  Label skip_move; 
  br(Assembler::negative, true, Assembler::pt, skip_move); 
  delayed()->mov(G0, delta); 
  bind(skip_move);
  round_to(delta, WordsPerLong);       // make multiple of 2 (SP must be 2-word aligned)
  sll(delta, LogBytesPerWord, delta);  // extra space for locals in bytes
}


// Dispatch code executed in the prolog of a bytecode which does not do it's
// own dispatch. The dispatch address is computed and placed in IdispatchAddress
void InterpreterMacroAssembler::dispatch_prolog(TosState state, int bcp_incr) {
  assert_not_delayed();
#ifdef FAST_DISPATCH
  // FAST_DISPATCH and ProfileInterpreter are mutually exclusive since 
  // they both use I2.
  assert(!ProfileInterpreter, "FAST_DISPATCH and +ProfileInterpreter are mutually exclusive");
  ldub(Lbcp, bcp_incr, Lbyte_code);			// load next bytecode
  add(Lbyte_code, Interpreter::distance_from_dispatch_table(state), Lbyte_code);
							// add offset to correct dispatch table
  sll(Lbyte_code, LogBytesPerWord, Lbyte_code);		// multiply by wordSize
  ld_ptr(IdispatchTables, Lbyte_code, IdispatchAddress);// get entry addr
#else
  ldub( Lbcp, bcp_incr, Lbyte_code);               // load next bytecode
  // dispatch table to use
  Address tbl(G3_scratch, (address)Interpreter::dispatch_table(state));

  sethi(tbl);
  sll(Lbyte_code, LogBytesPerWord, Lbyte_code);    // multiply by wordSize
  add(tbl, tbl.base(), 0);
  ld_ptr( G3_scratch, Lbyte_code, IdispatchAddress);     // get entry addr
#endif
}


// Dispatch code executed in the epilog of a bytecode which does not do it's
// own dispatch. The dispatch address in IdispatchAddress is used for the
// dispatch.
void InterpreterMacroAssembler::dispatch_epilog(TosState state, int bcp_incr) {
  assert_not_delayed();
  verify_FPU(1, state);
  verify_oop(Otos_i, state);
  jmp( IdispatchAddress, 0 );
  if (bcp_incr != 0)  delayed()->inc(Lbcp, bcp_incr);
  else                delayed()->nop();
}


void InterpreterMacroAssembler::dispatch_next(TosState state, int bcp_incr) {
  // %%%% consider branching to a single shared dispatch stub (for each bcp_incr)
  assert_not_delayed();
  ldub( Lbcp, bcp_incr, Lbyte_code);               // load next bytecode
  dispatch_Lbyte_code(state, Interpreter::dispatch_table(state), bcp_incr);
}


void InterpreterMacroAssembler::dispatch_next_noverify_oop(TosState state, int bcp_incr) {
  // %%%% consider branching to a single shared dispatch stub (for each bcp_incr)
  assert_not_delayed();
  ldub( Lbcp, bcp_incr, Lbyte_code);               // load next bytecode
  dispatch_Lbyte_code(state, Interpreter::dispatch_table(state), bcp_incr, false);
}


void InterpreterMacroAssembler::dispatch_via(TosState state, address* table) {
  // load current bytecode       
  assert_not_delayed();
  ldub( Lbcp, 0, Lbyte_code);               // load next bytecode
  dispatch_base(state, table);   
}                                
              

void InterpreterMacroAssembler::call_VM_leaf_base(
  Register java_thread,
  address  entry_point,
  int      number_of_arguments
) {
  if (!java_thread->is_valid())
    java_thread = L7_thread_cache;
  // super call
  MacroAssembler::call_VM_leaf_base(java_thread, entry_point, number_of_arguments);
}
 
 
void InterpreterMacroAssembler::call_VM_base(
  Register        oop_result,
  Register        java_thread,
  Register        last_java_sp,
  address         entry_point,
  int             number_of_arguments,
  bool		  check_exception
) {
  if (!java_thread->is_valid())
    java_thread = L7_thread_cache;
  // See class ThreadInVMfromInterpreter, which assumes that the interpreter
  // takes responsibility for setting its own thread-state on call-out.
  // However, ThreadInVMfromInterpreter resets the state to "in_Java".

  //save_bcp();                                  // save bcp
  MacroAssembler::call_VM_base(oop_result, java_thread, last_java_sp, entry_point, number_of_arguments, check_exception);
  //restore_bcp();                               // restore bcp
  //restore_locals();                            // restore locals pointer
}


void InterpreterMacroAssembler::check_and_handle_popframe(Register scratch_reg) {
  if (JvmtiExport::can_pop_frame()) {
    Label L;

    // Check the "pending popframe condition" flag in the current thread
    Address popframe_condition_addr(G2_thread, 0, in_bytes(JavaThread::popframe_condition_offset()));
    ld(popframe_condition_addr, scratch_reg);
    // Initiate popframe handling only if it is not already being processed.  If the flag
    // has the popframe_processing bit set, it means that this code is called *during* popframe
    // handling - we don't want to reenter.
    btst(JavaThread::popframe_pending_bit, scratch_reg);
    br(zero, false, pt, L);
    delayed()->nop();
    btst(JavaThread::popframe_processing_bit, scratch_reg);
    br(notZero, false, pt, L);
    delayed()->nop();
    // Call the Interpreter::remove_activation_preserving_args_entry() 
    // func to get the address of the same-named entrypoint in the
    // generated interpreter code
    call( CAST_FROM_FN_PTR(address, Interpreter::remove_activation_preserving_args_entry), relocInfo::runtime_call_type);
    delayed()->nop();
    // Jump to Interpreter::_remove_activation_preserving_args_entry
    jmpl(O0, G0, G0);
    delayed()->nop();
    bind(L);
  }
}


void InterpreterMacroAssembler::super_call_VM_leaf(Register thread_cache, address entry_point, Register arg_1) {
  mov(arg_1, O0);
  MacroAssembler::call_VM_leaf_base(thread_cache, entry_point, 1);
}


void InterpreterMacroAssembler::super_call_VM(Register thread_cache, Register oop_result, Register last_java_sp, address entry_point, Register arg_1, Register arg_2, bool check_exception) {
  // O0 is reserved for the thread
  mov(arg_1, O1);
  mov(arg_2, O2);
  MacroAssembler::call_VM_base(oop_result, thread_cache, last_java_sp, entry_point, 2, check_exception);
}


void InterpreterMacroAssembler::dispatch_base(TosState state, address* table) {
  assert_not_delayed();
  dispatch_Lbyte_code(state, table);
}


void InterpreterMacroAssembler::dispatch_normal(TosState state) {
  dispatch_base(state, Interpreter::normal_table(state));
}                                


void InterpreterMacroAssembler::dispatch_only(TosState state) {
  dispatch_base(state, Interpreter::dispatch_table(state));
}

 
// common code to dispatch and dispatch_only
// dispatch value in Lbyte_code and increment Lbcp

void InterpreterMacroAssembler::dispatch_Lbyte_code(TosState state, address* table, int bcp_incr, bool verify) {
  verify_FPU(1, state);
  // %%%%% maybe implement +VerifyActivationFrameSize here
  //verify_thread(); //too slow; we will just verify on method entry & exit
  if (verify) verify_oop(Otos_i, state);
#ifdef FAST_DISPATCH
  if (table == Interpreter::dispatch_table(state)) {
    // use IdispatchTables
    add(Lbyte_code, Interpreter::distance_from_dispatch_table(state), Lbyte_code);
							// add offset to correct dispatch table
    sll(Lbyte_code, LogBytesPerWord, Lbyte_code);	// multiply by wordSize
    ld_ptr(IdispatchTables, Lbyte_code, G3_scratch);	// get entry addr
  } else {
#endif
    // dispatch table to use
    Address tbl(G3_scratch, (address)table);

    sll(Lbyte_code, LogBytesPerWord, Lbyte_code);	// multiply by wordSize
    load_address(tbl);					// compute addr of table
    ld_ptr(G3_scratch, Lbyte_code, G3_scratch);		// get entry addr
#ifdef FAST_DISPATCH
  }
#endif
  jmp( G3_scratch, 0 );
  if (bcp_incr != 0)  delayed()->inc(Lbcp, bcp_incr);
  else                delayed()->nop();
}


// helpers for expression stack

void InterpreterMacroAssembler::pop_i(Register r) { 
  assert_not_delayed();
  ld(Lesp, wordSize, r);  
  inc(Lesp, wordSize); 
  debug_only(verify_esp(Lesp));
}

void InterpreterMacroAssembler::pop_ptr(Register r) { 
  assert_not_delayed();
  ld_ptr(Lesp, wordSize, r);  
  inc(Lesp, wordSize); 
  debug_only(verify_esp(Lesp));
}

void InterpreterMacroAssembler::pop_l(Register r) {
  assert_not_delayed();
  load_unaligned_long(Lesp, 1 * wordSize, r);
  inc(Lesp, 2 * wordSize); 
  debug_only(verify_esp(Lesp));
}


void InterpreterMacroAssembler::pop_f(FloatRegister f) { 
  assert_not_delayed();
  ldf(FloatRegisterImpl::S, Lesp, 1 * wordSize, f); 
  inc(Lesp, wordSize); 
  debug_only(verify_esp(Lesp));
}


void InterpreterMacroAssembler::pop_d(FloatRegister f) { 
  assert_not_delayed();
  load_unaligned_double(Lesp, 1*wordSize, f);
  inc(Lesp, 2 * wordSize); 
  debug_only(verify_esp(Lesp));
}


// (Note use register first, then decrement so dec can be done during store stall)

void InterpreterMacroAssembler::push_i(Register r) {
  assert_not_delayed();
  debug_only(verify_esp(Lesp));
  st(  r,    Lesp, 0);
  dec( Lesp, wordSize); 
}

void InterpreterMacroAssembler::push_ptr(Register r) {
  assert_not_delayed();
  st_ptr(  r,    Lesp, 0);
  dec( Lesp, wordSize); 
}

// remember: our convention for longs in SPARC is:
// O0 (Otos_l1) has high-order part in first word,
// O1 (Otos_l2) has low-order part in second word

void InterpreterMacroAssembler::push_l(Register r) {
  assert_not_delayed();
  debug_only(verify_esp(Lesp));
  // Longs are in stored in memory-correct order, even if unaligned.
  store_unaligned_long(r, Lesp, -1 *wordSize);
  dec( Lesp, 2 * wordSize); 
}


void InterpreterMacroAssembler::push_f(FloatRegister f) { 
  assert_not_delayed();
  debug_only(verify_esp(Lesp));
  stf(FloatRegisterImpl::S, f, Lesp, 0); 
  dec(                     Lesp, wordSize); 
}


void InterpreterMacroAssembler::push_d(FloatRegister f)   { 
  assert_not_delayed();
  debug_only(verify_esp(Lesp));
  // Doubles are in stored in memory-correct order, even if unaligned.
  store_unaligned_double(f, Lesp, -1*wordSize);
  dec(Lesp, 2 * wordSize);
}


void InterpreterMacroAssembler::push(TosState state) {
  verify_oop(Otos_i, state);
  switch (state) {
    case atos: push_ptr();	      break;
    case btos: push_i();              break;  
    case ctos:
    case stos: push_i();              break;  
    case itos: push_i();              break;  
    case ltos: push_l();              break;
    case ftos: push_f();              break;
    case dtos: push_d();              break;
    case vtos: /* nothing to do */    break;
    default  : ShouldNotReachHere();
  }
}


void InterpreterMacroAssembler::pop(TosState state) {
  switch (state) {
    case atos: pop_ptr();	     break;
    case btos: pop_i();              break;
    case ctos:
    case stos: pop_i();              break;
    case itos: pop_i();              break;
    case ltos: pop_l();              break;
    case ftos: pop_f();              break;
    case dtos: pop_d();              break;
    case vtos: /* nothing to do */   break;
    default  : ShouldNotReachHere();
  }
  verify_oop(Otos_i, state);
}


void InterpreterMacroAssembler::empty_expression_stack() {
  // Reset Lesp.
  sub( Lmonitors, wordSize, Lesp );

  // Reset SP by subtracting more space from Lesp.
  Label done;

  const Address max_stack   (Lmethod, 0, in_bytes(methodOopDesc::max_stack_offset()));
  const Address access_flags(Lmethod, 0, in_bytes(methodOopDesc::access_flags_offset()));

  verify_oop(Lmethod);

  assert( G4_scratch    != Gframe_size,
          "Only you can prevent register aliasing!");

  // A native does not need to do this, since its callee does not change SP.
  ld(access_flags, Gframe_size);
  btst(JVM_ACC_NATIVE, Gframe_size);
  br(Assembler::notZero, false, Assembler::pt, done);
  delayed()->nop();

  //
  // Compute max expression stack+register save area
  //
  lduh( max_stack, Gframe_size );
  add( Gframe_size, frame::memory_parameter_word_sp_offset, Gframe_size );

  //
  // now set up a stack frame with the size computed above
  //
  //round_to( Gframe_size, WordsPerLong ); // -- moved down to the "and" below
  sll( Gframe_size, LogBytesPerWord, Gframe_size );
  sub( Lesp, Gframe_size, Gframe_size );
  and3( Gframe_size, -(2 * wordSize), Gframe_size );          // align SP (downwards) to an 8/16-byte boundary
  debug_only(verify_sp(Gframe_size, G4_scratch));
#ifdef _LP64
  sub(Gframe_size, STACK_BIAS, Gframe_size );
#endif
  mov(Gframe_size, SP);

  bind(done);
}


#ifdef ASSERT
void InterpreterMacroAssembler::verify_sp(Register Rsp, Register Rtemp) {
  Label Bad, OK;

  // Saved SP must be aligned.
#ifdef _LP64
  btst(2*BytesPerWord-1, Rsp);
#else
  btst(LongAlignmentMask, Rsp);
#endif
  br(Assembler::notZero, false, Assembler::pn, Bad);
  delayed()->nop();

  // Saved SP, plus register window size, must not be above FP.
  add(Rsp, frame::register_save_words * wordSize, Rtemp);
#ifdef _LP64
  sub(Rtemp, STACK_BIAS, Rtemp);  // Bias Rtemp before cmp to FP
#endif
  cmp(Rtemp, FP);
  brx(Assembler::greaterUnsigned, false, Assembler::pn, Bad);
  delayed()->nop();

  // Saved SP must not be ridiculously below current SP.
  size_t maxstack = MAX2(JavaThread::stack_size_at_create(), (size_t) 4*K*K);
  set(maxstack, Rtemp);
  sub(SP, Rtemp, Rtemp);
#ifdef _LP64
  add(Rtemp, STACK_BIAS, Rtemp);  // Unbias Rtemp before cmp to Rsp
#endif
  cmp(Rsp, Rtemp);
  brx(Assembler::lessUnsigned, false, Assembler::pn, Bad);
  delayed()->nop();

  br(Assembler::always, false, Assembler::pn, OK);
  delayed()->nop();

  bind(Bad);
  stop("on return to interpreted call, restored SP is corrupted");

  bind(OK);
}


void InterpreterMacroAssembler::verify_esp(Register Resp) {
  // about to read or write Resp[0]
  // make sure it is not in the monitors or the register save area
  Label OK1, OK2;

  cmp(Resp, Lmonitors);
  brx(Assembler::lessUnsigned, true, Assembler::pt, OK1);
  delayed()->sub(Resp, frame::memory_parameter_word_sp_offset * wordSize, Resp);
  stop("too many pops:  Lesp points into monitor area");
  bind(OK1);
#ifdef _LP64
  sub(Resp, STACK_BIAS, Resp);
#endif
  cmp(Resp, SP);
  brx(Assembler::greaterEqualUnsigned, false, Assembler::pt, OK2);
  delayed()->add(Resp, STACK_BIAS + frame::memory_parameter_word_sp_offset * wordSize, Resp);
  stop("too many pushes:  Lesp points into register window");
  bind(OK2);
}
#endif // ASSERT


void InterpreterMacroAssembler::if_cmp(Condition cc, bool ptr_compare) {
  assert_not_delayed();

  Label not_taken;
  if (ptr_compare) brx(cc, false, Assembler::pn, not_taken);
  else             br (cc, false, Assembler::pn, not_taken);
  delayed()->nop();

  TemplateTable::branch(false,false);

  bind(not_taken);

  profile_not_taken_branch(G3_scratch);
}


void InterpreterMacroAssembler::get_2_byte_integer_at_bcp(
                                  int         bcp_offset, 
                                  Register    Rtmp, 
				  Register    Rdst,
                                  signedOrNot is_signed,
                        	  setCCOrNot  should_set_CC ) {
  assert(Rtmp != Rdst, "need separate temp register");
  assert_not_delayed();
  switch (is_signed) {
   default: ShouldNotReachHere();

   case   Signed:  ldsb( Lbcp, bcp_offset, Rdst  );  break; // high byte
   case Unsigned:  ldub( Lbcp, bcp_offset, Rdst  );  break; // high byte
  }
  ldub( Lbcp, bcp_offset + 1, Rtmp ); // low byte
  sll( Rdst, BitsPerByte, Rdst);
  switch (should_set_CC ) {
   default: ShouldNotReachHere();

   case      set_CC:  orcc( Rdst, Rtmp, Rdst ); break;
   case dont_set_CC:  or3(  Rdst, Rtmp, Rdst ); break;
  }
} 


void InterpreterMacroAssembler::get_4_byte_integer_at_bcp(
                                  int        bcp_offset,
                                  Register   Rtmp, 
				  Register   Rdst,
				  setCCOrNot should_set_CC ) {
  assert(Rtmp != Rdst, "need separate temp register");
  assert_not_delayed();
  add( Lbcp, bcp_offset, Rtmp);
  andcc( Rtmp, 3, G0);
  Label aligned;
  switch (should_set_CC ) {
   default: ShouldNotReachHere();

   case      set_CC: break;
   case dont_set_CC: break;
  }

  br(Assembler::zero, true, Assembler::pn, aligned);
#ifdef _LP64
  delayed()->ldsw(Rtmp, 0, Rdst);
#else
  delayed()->ld(Rtmp, 0, Rdst);
#endif

  ldub(Lbcp, bcp_offset + 3, Rdst);
  ldub(Lbcp, bcp_offset + 2, Rtmp);  sll(Rtmp,  8, Rtmp);  or3(Rtmp, Rdst, Rdst);
  ldub(Lbcp, bcp_offset + 1, Rtmp);  sll(Rtmp, 16, Rtmp);  or3(Rtmp, Rdst, Rdst);
#ifdef _LP64
  ldsb(Lbcp, bcp_offset + 0, Rtmp);  sll(Rtmp, 24, Rtmp);
#else
  // Unsigned load is faster than signed on some implementations
  ldub(Lbcp, bcp_offset + 0, Rtmp);  sll(Rtmp, 24, Rtmp);
#endif
  or3(Rtmp, Rdst, Rdst );

  bind(aligned);
  if (should_set_CC == set_CC) tst(Rdst);
} 


void InterpreterMacroAssembler::get_cache_and_index_at_bcp(Register cache, Register tmp, int bcp_offset) {
  assert(bcp_offset > 0, "bcp is still pointing to start of bytecode");
  assert_different_registers(cache, tmp);
  assert_not_delayed();
  get_2_byte_integer_at_bcp(bcp_offset, cache, tmp, Unsigned);
	      // convert from field index to ConstantPoolCacheEntry index
	      // and from word index to byte offset
  sll(tmp, exact_log2(in_words(ConstantPoolCacheEntry::size()) * BytesPerWord), tmp);
  add(LcpoolCache, tmp, cache);
}


void InterpreterMacroAssembler::get_cache_entry_pointer_at_bcp(Register cache, Register tmp, int bcp_offset) {
  assert(bcp_offset > 0, "bcp is still pointing to start of bytecode");
  assert_different_registers(cache, tmp);
  assert_not_delayed();
  get_2_byte_integer_at_bcp(bcp_offset, cache, tmp, Unsigned);
              // convert from field index to ConstantPoolCacheEntry index
              // and from word index to byte offset
  sll(tmp, exact_log2(in_words(ConstantPoolCacheEntry::size()) * BytesPerWord), tmp);
              // skip past the header
  add(tmp, in_bytes(constantPoolCacheOopDesc::base_offset()), tmp);
              // construct pointer to cache entry
  add(LcpoolCache, tmp, cache);
}


// Generate a subtype check: branch to ok_is_subtype if sub_klass is
// a subtype of super_klass.  Blows registers Rsub_klass, tmp1, tmp2.
void InterpreterMacroAssembler::gen_subtype_check( Register Rsub_klass, Register Rsuper_klass, Register Rtmp1, Register Rtmp2, Register Rtmp3, Label &ok_is_subtype ) {
  Label not_subtype, loop;

  // Load the super-klass's check offset into Rtmp1
  ld( Rsuper_klass, sizeof(oopDesc) + Klass::super_check_offset_offset_in_bytes(), Rtmp1 );
  // Load from the sub-klass's super-class display list, or a 1-word cache of
  // the secondary superclass list, or a failing value with a sentinel offset
  // if the super-klass is an interface or exceptionally deep in the Java
  // hierarchy and we have to scan the secondary superclass list the hard way.
  ld_ptr( Rsub_klass, Rtmp1, Rtmp2 );
  // See if we get an immediate positive hit
  cmp( Rtmp2, Rsuper_klass );
  brx( Assembler::equal, false, Assembler::pt, ok_is_subtype );
  // In the delay slot, check for immediate negative hit
  delayed()->cmp( Rtmp1, sizeof(oopDesc) + Klass::secondary_super_cache_offset_in_bytes() );
  br( Assembler::notEqual, false, Assembler::pt, not_subtype );
  // In the delay slot, check for self
  delayed()->cmp( Rsub_klass, Rsuper_klass );
  brx( Assembler::equal, false, Assembler::pt, ok_is_subtype );

  // Now do a linear scan of the secondary super-klass chain.
  delayed()->ld_ptr( Rsub_klass, sizeof(oopDesc) + Klass::secondary_supers_offset_in_bytes(), Rtmp2 );

  // Rtmp2 holds the objArrayOop of secondary supers.
  ld( Rtmp2, arrayOopDesc::length_offset_in_bytes(), Rtmp1 );// Load the array length
  // Check for empty secondary super list
  tst(Rtmp1);

  // Top of search loop
  bind( loop );
  br( Assembler::equal, false, Assembler::pn, not_subtype );
  // In delay slot, load next super to check
  delayed()->ld_ptr( Rtmp2, arrayOopDesc::base_offset_in_bytes(T_OBJECT), Rtmp3 );

  // Bump array pointer forward one oop
  add( Rtmp2, wordSize, Rtmp2 );
  // Look for Rsuper_klass on Rsub_klass's secondary super-class-overflow list
  cmp( Rtmp3, Rsuper_klass );
  // A miss means we are NOT a subtype and need to keep looping
  brx( Assembler::notEqual, false, Assembler::pt, loop );
  delayed()->deccc( Rtmp1 );    // dec trip counter in delay slot
  // Falling out the bottom means we found a hit; we ARE a subtype
  br( Assembler::always, false, Assembler::pt, ok_is_subtype );
  // Update the cache
  delayed()->st_ptr( Rsuper_klass, Rsub_klass, sizeof(oopDesc) + Klass::secondary_super_cache_offset_in_bytes() );

  bind(not_subtype);
}

// Separate these two to allow for delay slot in middle
// These are used to do a test and full jump to exception-throwing code.

// %%%%% Could possibly reoptimize this by testing to see if could use
// a single conditional branch (i.e. if span is small enough.
// If you go that route, than get rid of the split and give up
// on the delay-slot hack. 

void InterpreterMacroAssembler::throw_if_not_1_icc( Condition ok_condition, 
			         	            Label&    ok ) {
  assert_not_delayed();
  br(ok_condition, true, pt, ok);
  // DELAY SLOT
}

void InterpreterMacroAssembler::throw_if_not_1_xcc( Condition ok_condition, 
			         	            Label&    ok ) {
  assert_not_delayed();
  bp( ok_condition, true, Assembler::xcc, pt, ok);
  // DELAY SLOT
}

void InterpreterMacroAssembler::throw_if_not_1_x( Condition ok_condition, 
			         	          Label&    ok ) {
  assert_not_delayed();
  brx(ok_condition, true, pt, ok);
  // DELAY SLOT
}

void InterpreterMacroAssembler::throw_if_not_2( address  throw_entry_point,
					        Register Rscratch,
					        Label&   ok ) {
  assert(throw_entry_point != NULL, "entry point must be generated by now");
  jump_to(Address( Rscratch, throw_entry_point ));
  delayed()->nop();
  bind(ok);
}


// And if you cannot use the delay slot, here is a shorthand:

void InterpreterMacroAssembler::throw_if_not_icc( Condition ok_condition,
                                                  address   throw_entry_point,
					          Register  Rscratch ) {
  Label ok;
  if (ok_condition != never) {
    throw_if_not_1_icc( ok_condition, ok);
    delayed()->nop();
  }
  throw_if_not_2( throw_entry_point, Rscratch, ok);
}
void InterpreterMacroAssembler::throw_if_not_xcc( Condition ok_condition,
                                                  address   throw_entry_point,
					          Register  Rscratch ) {
  Label ok;
  if (ok_condition != never) {
    throw_if_not_1_xcc( ok_condition, ok);
    delayed()->nop();
  }
  throw_if_not_2( throw_entry_point, Rscratch, ok);
}
void InterpreterMacroAssembler::throw_if_not_x( Condition ok_condition,
                                                address   throw_entry_point,
					        Register  Rscratch ) {
  Label ok;
  if (ok_condition != never) {
    throw_if_not_1_x( ok_condition, ok);
    delayed()->nop();
  }
  throw_if_not_2( throw_entry_point, Rscratch, ok);
}

// Check that index is in range for array, then shift index by index_shift, and put arrayOop + shifted_index into res
// Note: res is still shy of address by array offset into object.

void InterpreterMacroAssembler::index_check_without_pop(Register array, Register index, int index_shift, Register tmp, Register res) {
  assert_not_delayed();

  verify_oop(array);

  // check array
  Label ptr_ok;
  tst(array);
  throw_if_not_1_x( notZero, ptr_ok );
  delayed()->ld( array, arrayOopDesc::length_offset_in_bytes(), tmp ); // check index
  throw_if_not_2( Interpreter::_throw_NullPointerException_entry, G3_scratch, ptr_ok);

  Label index_ok;
  cmp(index, tmp);
  throw_if_not_1_icc( lessUnsigned, index_ok );
  if (index_shift > 0)  delayed()->sll(index, index_shift, index);
  else                  delayed()->add(array, index, res); // addr - const offset in index
  // convention: move aberrant index into G3_scratch for exception message
  mov(index, G3_scratch);
  throw_if_not_2( Interpreter::_throw_ArrayIndexOutOfBoundsException_entry, G4_scratch, index_ok);

  // add offset if didn't do it in delay slot
  if (index_shift > 0)   add(array, index, res); // addr - const offset in index
}


void InterpreterMacroAssembler::index_check(Register array, Register index, int index_shift, Register tmp, Register res) {
  assert_not_delayed();

  // pop array
  pop_ptr(array);

  // check array
  index_check_without_pop(array, index, index_shift, tmp, res);
}


void InterpreterMacroAssembler::get_constant_pool(Register Rdst) {
  ld_ptr(Lmethod, in_bytes(methodOopDesc::constants_offset()), Rdst);
}


void InterpreterMacroAssembler::get_constant_pool_cache(Register Rdst) {
  get_constant_pool(Rdst);
  ld_ptr(Rdst, constantPoolOopDesc::cache_offset_in_bytes(), Rdst);
}


void InterpreterMacroAssembler::get_cpool_and_tags(Register Rcpool, Register Rtags) {
  get_constant_pool(Rcpool);
  ld_ptr(Rcpool, constantPoolOopDesc::tags_offset_in_bytes(), Rtags);
}

#if 0
void InterpreterMacroAssembler::is_a(Label& ok) {
  assert_not_delayed();
  verify_oop(Otos_i);
  tst(Otos_i);
  brx(Assembler::zero, false, Assembler::pn, ok);

  delayed()->mov(Otos_i, O3);
  get_constant_pool(O1);
  get_2_byte_integer_at_bcp(1, G4_scratch, O2, Unsigned );

  TemplateTable::call_VM(noreg, (address)InterpreterRuntime::is_a, O1, O2, O3);
  // (use TemplateTable version, because it has extra asserts)
}
#endif


// unlock if synchronized method
//
// Unlock the receiver if this is a synchronized method.
// Unlock any Java monitors from syncronized blocks.
//
// If there are locked Java monitors
//    If throw_monitor_exception
//       throws IllegalMonitorStateException
//    Else if install_monitor_exception
//       installs IllegalMonitorStateException
//    Else
//       no error processing
void InterpreterMacroAssembler::unlock_if_synchronized_method(TosState state,
                                                              bool throw_monitor_exception, 
                                                              bool install_monitor_exception) {
  Label unlocked, unlock, no_unlock;

  // get the value of _do_not_unlock_if_synchronized into G1_scratch
  const Address do_not_unlock_if_synchronized(G2_thread, 0,
    in_bytes(JavaThread::do_not_unlock_if_synchronized_offset()));
  ld(do_not_unlock_if_synchronized, G1_scratch);
  st(G0, do_not_unlock_if_synchronized); // reset the flag

  // check if synchronized method
  const Address access_flags(Lmethod, 0, in_bytes(methodOopDesc::access_flags_offset()));
  verify_oop(Otos_i, state);
  push(state); // save tos
  ld(access_flags, G3_scratch);
  btst(JVM_ACC_SYNCHRONIZED, G3_scratch);
  br( zero, false, pt, unlocked);
  delayed()->nop();

  // Don't unlock anything if the _do_not_unlock_if_synchronized flag
  // is set.
  tst(G1_scratch);
  br(Assembler::notZero, false, pn, no_unlock);
  delayed()->nop();

  // BasicObjectLock will be first in list, since this is a synchronized method. However, need
  // to check that the object has not been unlocked by an explicit monitorexit bytecode.  

  //Intel: if (throw_monitor_exception) ... else ...
  // Entry already unlocked, need to throw exception
  //...

  // pass top-most monitor elem
  add( top_most_monitor(), O1 );

  ld_ptr(O1, BasicObjectLock::obj_offset_in_bytes(), G3_scratch);
  br_notnull(G3_scratch, false, pt, unlock);
  delayed()->nop();

  if (throw_monitor_exception) {
    // Entry already unlocked need to throw an exception
    MacroAssembler::call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::throw_illegal_monitor_state_exception));
    should_not_reach_here();
  } else {
    // Monitor already unlocked during a stack unroll. 
    // If requested, install an illegal_monitor_state_exception.
    // Continue with stack unrolling.
    if (install_monitor_exception) {
      MacroAssembler::call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::new_illegal_monitor_state_exception));
    }
    ba(false, unlocked);
    delayed()->nop();
  }

  bind(unlock);

  unlock_object(O1);

  bind(unlocked);

  // I0, I1: Might contain return value

  // Check that all monitors are unlocked
  { Label loop, exception, entry, restart;

    Register Rmptr   = O0;
    Register Rtemp   = O1;
    Register Rlimit  = Lmonitors;
    const jint delta = frame::interpreter_frame_monitor_size() * wordSize;
    assert( (delta & LongAlignmentMask) == 0, 
	    "sizeof BasicObjectLock must be even number of doublewords");

    #ifdef ASSERT
    add(top_most_monitor(), Rmptr, delta);
    { Label L;
      // ensure that Rmptr starts out above (or at) Rlimit
      cmp(Rmptr, Rlimit);
      brx(Assembler::greaterEqualUnsigned, false, pn, L);
      delayed()->nop();
      stop("monitor stack has negative size");
      bind(L);
    }
    #endif
    bind(restart);
    ba(false, entry);
    delayed()->
    add(top_most_monitor(), Rmptr, delta);      // points to current entry, starting with bottom-most entry

    // Entry is still locked, need to throw exception
    bind(exception);
    if (throw_monitor_exception) {
      MacroAssembler::call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::throw_illegal_monitor_state_exception));
      should_not_reach_here();
    } else {
      // Stack unrolling. Unlock object and if requested, install illegal_monitor_exception.
      // Unlock does not block, so don't have to worry about the frame
      unlock_object(Rmptr);
      if (install_monitor_exception) {
        MacroAssembler::call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::new_illegal_monitor_state_exception));
      }
      ba(false, restart);
      delayed()->nop();
    }

    bind(loop);
    cmp(Rtemp, (int)NULL);                      // check if current entry is used
    brx(Assembler::notEqual, false, pn, exception);
    delayed()->
    dec(Rmptr, delta);                          // otherwise advance to next entry
    #ifdef ASSERT
    { Label L;
      // ensure that Rmptr has not somehow stepped below Rlimit
      cmp(Rmptr, Rlimit);
      brx(Assembler::greaterEqualUnsigned, false, pn, L);
      delayed()->nop();
      stop("ran off the end of the monitor stack");
      bind(L);
    }
    #endif
    bind(entry);
    cmp(Rmptr, Rlimit);                         // check if bottom reached
    brx(Assembler::notEqual, true, pn, loop);    // if not at bottom then check this entry
    delayed()->
    ld_ptr(Rmptr, BasicObjectLock::obj_offset_in_bytes() - delta, Rtemp);
  }

  bind(no_unlock);
  pop(state);
  verify_oop(Otos_i, state);
}


// Lock object
//
// Argument - lock_reg points to the BasicObjectLock to be used for locking,
//            it must be initialized with the object to lock
void InterpreterMacroAssembler::lock_object(Register lock_reg, Register Object) {
  if (UseHeavyMonitors) {
    call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::monitorenter), lock_reg);
  }
  else {
    Register obj_reg = Object;
    Register mark_reg = G4_scratch;
    Register temp_reg = G1_scratch;
    Address  lock_addr = Address(lock_reg, 0, BasicObjectLock::lock_offset_in_bytes());
    Address  mark_addr = Address(obj_reg, 0, oopDesc::mark_offset_in_bytes());
    Label    done;

    // load markOop from object into mark_reg
    ld_ptr(mark_addr, mark_reg);

    // get the address of basicLock on stack that will be stored in the object
    // we need a temporary register here as we do not want to clobber lock_reg
    // (cas clobbers the destination register)
    mov(lock_reg, temp_reg);
    // set mark reg to be (markOop of object | UNLOCK_VALUE)
    or3(mark_reg, markOopDesc::unlocked_value, mark_reg);
    // initialize the box  (Must happen before we update the object mark!)
    st_ptr(mark_reg, lock_addr, BasicLock::displaced_header_offset_in_bytes());
    // compare and exchange object_addr, markOop | 1, stack address of basicLock
    assert(mark_addr.disp() == 0, "cas must take a zero displacement");
    casx_under_lock(mark_addr.base(), mark_reg, temp_reg, 
      (address)StubRoutines::sparc::atomic_memory_operation_lock_addr());

    // if the compare and exchange succeeded we are done (we saw an unlocked object)
    // store the original markOop of the object into the displaced_header field of
    // the basicLock on the stack
    cmp(mark_reg, temp_reg);
    brx(Assembler::equal, true, Assembler::pt, done);
    delayed()->nop();

    // We did not see an unlocked object so try the fast recursive case

    // Check if owner is self by comparing the value in the markOop of object
    // with the stack pointer
    sub(temp_reg, SP, temp_reg);
#ifdef _LP64
    sub(temp_reg, STACK_BIAS, temp_reg);
#endif
    assert(os::vm_page_size() > 0xfff, "page size too small - change the constant");

    // Composite "andcc" test:
    // (a) %sp -vs- markword proximity check, and,
    // (b) verify mark word LSBs == 0 (Stack-locked).  
    //
    // FFFFF003/FFFFFFFFFFFF003 is (markOopDesc::lock_mask_in_place | -os::vm_page_size())
    // Note that the page size used for %sp proximity testing is arbitrary and is
    // unrelated to the actual MMU page size.  We use a 'logical' page size of 
    // 4096 bytes.   F..FFF003 is designed to fit conveniently in the SIMM13 immediate
    // field of the andcc instruction. 
    andcc (temp_reg, 0xFFFFF003, G0) ;

    // if condition is true we are done and hence we can store 0 in the displaced
    // header indicating it is a recursive lock and be done
    brx(Assembler::zero, true, Assembler::pt, done);
    delayed()->st_ptr(G0, lock_addr, BasicLock::displaced_header_offset_in_bytes());

    // none of the above fast optimizations worked so we have to get into the
    // slow case of monitor enter
    call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::monitorenter), lock_reg);

    bind(done);   
  }
}

// Unlocks an object. Used in monitorexit bytecode and remove_activation.
//
// Argument - lock_reg points to the BasicObjectLock for lock
// Throw IllegalMonitorException if object is not locked by current thread
void InterpreterMacroAssembler::unlock_object(Register lock_reg) {
  if (UseHeavyMonitors) {
    call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::monitorexit), lock_reg);
  } else {
    Register obj_reg = G3_scratch;
    Register mark_reg = G4_scratch;
    Register displaced_header_reg = G1_scratch;
    Address  lock_addr = Address(lock_reg, 0, BasicObjectLock::lock_offset_in_bytes());
    Address  lockobj_addr = Address(lock_reg, 0, BasicObjectLock::obj_offset_in_bytes());
    Address  mark_addr = Address(obj_reg, 0, oopDesc::mark_offset_in_bytes());
    Label    done;

    // Test first if we are in the fast recursive case
    ld_ptr(lock_addr, displaced_header_reg, BasicLock::displaced_header_offset_in_bytes());
    br_null(displaced_header_reg, true, Assembler::pn, done);
    delayed()->st_ptr(G0, lockobj_addr);  // free entry

    // See if it is still a light weight lock, if so we just unlock
    // the object and we are done

    // load the markOop from the object
    ld_ptr(lockobj_addr, obj_reg);

    // we have the displaced header in displaced_header_reg
    // we expect to see the stack address of the basicLock in case the
    // lock is still a light weight lock (lock_reg)
    assert(mark_addr.disp() == 0, "cas must take a zero displacement");
    casx_under_lock(mark_addr.base(), lock_reg, displaced_header_reg, 
      (address)StubRoutines::sparc::atomic_memory_operation_lock_addr());
    cmp(lock_reg, displaced_header_reg);
    brx(Assembler::equal, true, Assembler::pn, done);
    delayed()->st_ptr(G0, lockobj_addr);  // free entry

    // The lock has been converted into a heavy lock and hence
    // we need to get into the slow case

    call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::monitorexit), lock_reg);

    bind(done);
  }
}

// Get the method data pointer from the methodOop and set the
// specified register to its value.

#ifndef CORE
void InterpreterMacroAssembler::set_method_data_pointer_offset(Register Roff) {
  assert(ProfileInterpreter, "must be profiling interpreter");
  Label get_continue;

  ld_ptr(Lmethod, in_bytes(methodOopDesc::method_data_offset()), ImethodDataPtr);
  test_method_data_pointer(get_continue);
  add(ImethodDataPtr, in_bytes(methodDataOopDesc::data_offset()), ImethodDataPtr);
  if (Roff != noreg)
    // Roff contains a method data index ("mdi").  It defaults to zero.
    add(ImethodDataPtr, Roff, ImethodDataPtr);
  bind(get_continue);
}

// Set the method data pointer for the current bcp.

void InterpreterMacroAssembler::set_method_data_pointer_for_bcp() {
  assert(ProfileInterpreter, "must be profiling interpreter");
  Label zero_continue;

  // Test MDO to avoid the call if it is NULL.
  ld_ptr(Lmethod, in_bytes(methodOopDesc::method_data_offset()), ImethodDataPtr);
  test_method_data_pointer(zero_continue);
  call_VM_leaf(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::bcp_to_di), Lmethod, Lbcp);
  set_method_data_pointer_offset(O0);
  bind(zero_continue);
}

// Test ImethodDataPtr.  If it is null, continue at the specified label

void InterpreterMacroAssembler::test_method_data_pointer(Label& zero_continue) {
  assert(ProfileInterpreter, "must be profiling interpreter");
#ifdef _LP64
  bpr(Assembler::rc_z, false, Assembler::pn, ImethodDataPtr, zero_continue);
#else
  tst(ImethodDataPtr);
  br(Assembler::zero, false, Assembler::pn, zero_continue);
#endif
  delayed()->nop();
}

void InterpreterMacroAssembler::verify_method_data_pointer() {
  assert(ProfileInterpreter, "must be profiling interpreter");
#ifdef ASSERT
  Label verify_continue;
  test_method_data_pointer(verify_continue);

  // If the mdp is valid, it will point to a DataLayout header which is
  // consistent with the bcp.  The converse is highly probable also.
  lduh(ImethodDataPtr, in_bytes(DataLayout::bci_offset()), G3_scratch);
  ld_ptr(Address(Lmethod, 0, in_bytes(methodOopDesc::const_offset())), O5);
  add(G3_scratch, in_bytes(constMethodOopDesc::codes_offset()), G3_scratch);
  add(G3_scratch, O5, G3_scratch);
  cmp(Lbcp, G3_scratch);
  brx(Assembler::equal, false, Assembler::pt, verify_continue);

  Register temp_reg = O5;
  delayed()->mov(ImethodDataPtr, temp_reg);
  // %%% should use call_VM_leaf here?
  //call_VM_leaf(noreg, ..., Lmethod, Lbcp, ImethodDataPtr);
  save_frame_and_mov(sizeof(jdouble) / wordSize, Lmethod, O0, Lbcp, O1);
  Address d_save(FP, 0, -sizeof(jdouble) + STACK_BIAS);
  stf(FloatRegisterImpl::D, Ftos_d, d_save);
  mov(temp_reg->after_save(), O2);
  save_thread(L7_thread_cache);
  call(CAST_FROM_FN_PTR(address, InterpreterRuntime::verify_mdp), relocInfo::none);
  delayed()->nop();
  restore_thread(L7_thread_cache);
  ldf(FloatRegisterImpl::D, d_save, Ftos_d);
  restore();
  bind(verify_continue);
#endif // ASSERT
}

void InterpreterMacroAssembler::test_invocation_counter_for_mdp(Register invocation_count, 
								Register cur_bcp, 
								Register Rtmp,
								Label &profile_continue) {
  assert(ProfileInterpreter, "must be profiling interpreter");
  // Control will flow to "profile_continue" if the counter is less than the 
  // limit or if we call profile_method()

  Label done;

  // if no method data exists, and the counter is high enough, make one
#ifdef _LP64
  bpr(Assembler::rc_nz, false, Assembler::pn, ImethodDataPtr, done);
#else
  tst(ImethodDataPtr);
  br(Assembler::notZero, false, Assembler::pn, done);
#endif

  // Test to see if we should create a method data oop
  Address profile_limit(Rtmp, (address)&InvocationCounter::InterpreterProfileLimit);
#ifdef _LP64
  delayed()->nop();
  sethi(profile_limit);
#else
  delayed()->sethi(profile_limit);
#endif
  ld(profile_limit, Rtmp);
  cmp(invocation_count, Rtmp);
  br(Assembler::lessUnsigned, false, Assembler::pn, profile_continue);
  delayed()->nop();

  // Build it now.
  call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::profile_method), cur_bcp);
  set_method_data_pointer_offset(O0);
  ba(false, profile_continue);
  delayed()->nop();
  bind(done);
}

// Store a value at some constant offset from the method data pointer.

void InterpreterMacroAssembler::set_mdp_data_at(int constant, Register value) {
  assert(ProfileInterpreter, "must be profiling interpreter");
  st_ptr(value, ImethodDataPtr, constant);
}

// Increment the value at some constant offset from the method data pointer.

void InterpreterMacroAssembler::increment_mdp_data_at(int constant,
						      Register bumped_count) {
  assert(ProfileInterpreter, "must be profiling interpreter");

  // Load the counter.
  ld_ptr(ImethodDataPtr, constant, bumped_count);

  // Increment the register.  Set carry flag.
  addcc(bumped_count, DataLayout::counter_increment, bumped_count);

  // If the increment causes the counter to overflow, pull back by 1.
  subc(bumped_count, G0, bumped_count);

  // Store the incremented counter.
  st_ptr(bumped_count, ImethodDataPtr, constant);
}

// Increment the value at some non-fixed (reg + constant) offset from
// the method data pointer.

void InterpreterMacroAssembler::increment_mdp_data_at(Register reg,
                                                      int constant,
                                                      Register bumped_count,
						      Register scratch2) {
  assert(ProfileInterpreter, "must be profiling interpreter");

  // Add the constant to reg to get the offset.
  add(reg, constant, scratch2);

  // Load the counter.
  ld_ptr(ImethodDataPtr, scratch2, bumped_count);

  // Increment the register.  Set carry flag.
  addcc(bumped_count, DataLayout::counter_increment, bumped_count);

  // If the increment causes the counter to overflow, pull back by 1.
  subc(bumped_count, G0, bumped_count);

  // Store the incremented counter.
  st_ptr(bumped_count, ImethodDataPtr, scratch2);
}

// Set a flag value at the current method data pointer position.

void InterpreterMacroAssembler::set_mdp_flag_at(int flag_constant,
						Register scratch,
						Register scratch2) {
  assert(ProfileInterpreter, "must be profiling interpreter");
  // Load the data header
  ld_ptr(ImethodDataPtr, in_bytes(ProfileData::header_offset()), scratch);

  // Set the flag
  set(flag_constant, scratch2);
  or3(scratch, scratch2, scratch);

  // Store the modified header.
  st_ptr(scratch, ImethodDataPtr, in_bytes(ProfileData::header_offset()));
}

// Test the location at some offset from the method data pointer.
// If it is not equal to value, branch to the not_equal_continue Label.

void InterpreterMacroAssembler::test_mdp_data_at(int offset,
                                                 Register value,
                                                 Label& not_equal_continue,
						 Register scratch) {
  assert(ProfileInterpreter, "must be profiling interpreter");
  ld_ptr(ImethodDataPtr, offset, scratch);
  cmp(value, scratch);
  brx(Assembler::notEqual, false, Assembler::pn, not_equal_continue);
  delayed()->nop();
}

// Update the method data pointer by the displacement located at some fixed
// offset from the method data pointer.

void InterpreterMacroAssembler::update_mdp_by_offset(int offset_of_disp,
						     Register scratch) {
  assert(ProfileInterpreter, "must be profiling interpreter");
  ld_ptr(ImethodDataPtr, offset_of_disp, scratch);
  add(ImethodDataPtr, scratch, ImethodDataPtr);
}

// Update the method data pointer by the displacement located at the
// offset (reg + offset_of_disp).

void InterpreterMacroAssembler::update_mdp_by_offset(Register reg,
						     int offset_of_disp,
						     Register scratch) {
  assert(ProfileInterpreter, "must be profiling interpreter");
  add(reg, offset_of_disp, scratch);
  ld_ptr(ImethodDataPtr, scratch, scratch);
  add(ImethodDataPtr, scratch, ImethodDataPtr);
}

// Update the method data pointer by a simple constant displacement.

void InterpreterMacroAssembler::update_mdp_by_constant(int constant) {
  assert(ProfileInterpreter, "must be profiling interpreter");
  add(ImethodDataPtr, constant, ImethodDataPtr);
}

// Update the method data pointer for a _ret bytecode whose target
// was not among our cached targets.

void InterpreterMacroAssembler::update_mdp_for_ret(TosState state,
						   Register return_bci) {
  assert(ProfileInterpreter, "must be profiling interpreter");
  push(state);
  st_ptr(return_bci, l_tmp);  // protect return_bci, in case it is volatile
  call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::update_mdp_for_ret), return_bci);
  ld_ptr(l_tmp, return_bci);
  pop(state);
}
#endif // !CORE

// Count a taken branch in the bytecodes.

void InterpreterMacroAssembler::profile_taken_branch(Register scratch, Register bumped_count) {
#ifndef CORE
  if (ProfileInterpreter) {
    Label profile_continue;

    // If no method data exists, go to profile_continue.
    test_method_data_pointer(profile_continue);

    // We are taking a branch.  Increment the taken count.
    increment_mdp_data_at(in_bytes(JumpData::taken_offset()), bumped_count);

    // The method data pointer needs to be updated to reflect the new target.
    update_mdp_by_offset(in_bytes(JumpData::displacement_offset()), scratch);
    bind (profile_continue);
  }
#endif // !CORE
}


// Count a not-taken branch in the bytecodes.

void InterpreterMacroAssembler::profile_not_taken_branch(Register scratch) {
#ifndef CORE
  if (ProfileInterpreter) {
    Label profile_continue;

    // If no method data exists, go to profile_continue.
    test_method_data_pointer(profile_continue);

    // We are taking a branch.  Increment the not taken count.
    increment_mdp_data_at(in_bytes(BranchData::not_taken_offset()), scratch);

    // The method data pointer needs to be updated to correspond to the
    // next bytecode.
    update_mdp_by_constant(in_bytes(BranchData::branch_data_size()));
    bind (profile_continue);
  }
#endif // !CORE
}


// Count a non-virtual call in the bytecodes.

void InterpreterMacroAssembler::profile_call(Register scratch) {
#ifndef CORE
  if (ProfileInterpreter) {
    Label profile_continue;

    // If no method data exists, go to profile_continue.
    test_method_data_pointer(profile_continue);

    // We are making a call.  Increment the count.
    increment_mdp_data_at(in_bytes(CounterData::count_offset()), scratch);

    // The method data pointer needs to be updated to reflect the new target.
    update_mdp_by_constant(in_bytes(CounterData::counter_data_size()));
    bind (profile_continue);
  }
#endif // !CORE
}


// Count a final call in the bytecodes.

void InterpreterMacroAssembler::profile_final_call(Register scratch) {
#ifndef CORE
  if (ProfileInterpreter) {
    Label profile_continue;

    // If no method data exists, go to profile_continue.
    test_method_data_pointer(profile_continue);

    // We are making a call.  Increment the count.
    increment_mdp_data_at(in_bytes(CounterData::count_offset()), scratch);

    // The method data pointer needs to be updated to reflect the new target.
    update_mdp_by_constant(in_bytes(VirtualCallData::virtual_call_data_size()));
    bind (profile_continue);
  }
#endif // !CORE
}


// Count a virtual call in the bytecodes.

void InterpreterMacroAssembler::profile_virtual_call(Register receiver,
						     Register scratch) {
#ifndef CORE
  if (ProfileInterpreter) {
    Label profile_continue;
    Label update_mdp;
    uint row;
    
    // If no method data exists, go to profile_continue.
    test_method_data_pointer(profile_continue);
    
    // We are making a call.  Increment the count.
    increment_mdp_data_at(in_bytes(CounterData::count_offset()), scratch);
    
    for (row = 0; row < VirtualCallData::row_limit(); row++) {
      Label next_test;
      
      // See if the receiver is receiver[n].
      test_mdp_data_at(in_bytes(VirtualCallData::receiver_offset(row)),
		       receiver, next_test, scratch);
      
      // The receiver is receiver[n].  Increment count[n].
      increment_mdp_data_at(in_bytes(VirtualCallData::receiver_count_offset(row)), scratch);
      ba(false, update_mdp);
      delayed()->nop();
      bind(next_test);
    }
    
    for (row = 0; row < VirtualCallData::row_limit(); row++) {
      Label next_zero_test;
      
      // See if receiver[n] is null:
      test_mdp_data_at(in_bytes(VirtualCallData::receiver_offset(row)),
		       G0, next_zero_test, scratch);
      
      // receiver[n] is NULL.  Fill in the receiver field and increment the count.
      set_mdp_data_at(in_bytes(VirtualCallData::receiver_offset(row)), receiver);
      mov(DataLayout::counter_increment, scratch);
      set_mdp_data_at(in_bytes(VirtualCallData::receiver_count_offset(row)), scratch);
      ba(false, update_mdp);
      delayed()->nop();
      bind(next_zero_test);
    }
    
    bind (update_mdp);
    
    // The method data pointer needs to be updated to reflect the new target.
    update_mdp_by_constant(in_bytes(VirtualCallData::virtual_call_data_size()));
    bind (profile_continue);
  }
#endif // !CORE
}


// Count a ret in the bytecodes.

void InterpreterMacroAssembler::profile_ret(TosState state,
					    Register return_bci,
					    Register scratch) {
#ifndef CORE
  if (ProfileInterpreter) {
    Label profile_continue;
    uint row;
    
    // If no method data exists, go to profile_continue.
    test_method_data_pointer(profile_continue);
    
    // Update the total ret count.
    increment_mdp_data_at(in_bytes(CounterData::count_offset()), scratch);
    
    for (row = 0; row < RetData::row_limit(); row++) {
      Label next_test;
      
      // See if return_bci is equal to bci[n]:
      test_mdp_data_at(in_bytes(RetData::bci_offset(row)),
		       return_bci, next_test, scratch);
      
      // return_bci is equal to bci[n].  Increment the count.
      increment_mdp_data_at(in_bytes(RetData::bci_count_offset(row)), scratch);
      
      // The method data pointer needs to be updated to reflect the new target.
      update_mdp_by_offset(in_bytes(RetData::bci_displacement_offset(row)), scratch);
      ba(false, profile_continue);
      delayed()->nop();
      bind(next_test);
    }
    
    update_mdp_for_ret(state, return_bci);
    
    bind (profile_continue);
  }
#endif // !CORE
}

// Profile a checkcast in the bytecodes.

void InterpreterMacroAssembler::profile_checkcast(bool is_null,
						  Register scratch,
						  Register scratch2) {
#ifndef CORE
  if (ProfileInterpreter) {
    Label profile_continue;

    // If no method data exists, go to profile_continue.
    test_method_data_pointer(profile_continue);

    if (is_null) {
      // Set the flag to true.
      set_mdp_flag_at(BitData::null_flag_constant(), scratch, scratch2);
    }

    // The method data pointer needs to be updated.
    update_mdp_by_constant(in_bytes(BitData::bit_data_size()));

    bind (profile_continue);
  }
#endif // !CORE
}

// Count the default case of a switch construct.

void InterpreterMacroAssembler::profile_switch_default(Register scratch) {
#ifndef CORE
  if (ProfileInterpreter) {
    Label profile_continue;

    // If no method data exists, go to profile_continue.
    test_method_data_pointer(profile_continue);

    // Update the default case count
    increment_mdp_data_at(in_bytes(MultiBranchData::default_count_offset()),
			  scratch);  

    // The method data pointer needs to be updated.
    update_mdp_by_offset(
		    in_bytes(MultiBranchData::default_displacement_offset()),
		    scratch);

    bind (profile_continue);
  }
#endif // !CORE
}

// Count the index'th case of a switch construct.

void InterpreterMacroAssembler::profile_switch_case(Register index,
						    Register scratch,
						    Register scratch2,
						    Register scratch3) {
#ifndef CORE
  if (ProfileInterpreter) {
    Label profile_continue;

    // If no method data exists, go to profile_continue.
    test_method_data_pointer(profile_continue);

    // Build the base (index * per_case_size_in_bytes()) + case_array_offset_in_bytes()  
    set(in_bytes(MultiBranchData::per_case_size()), scratch);
    smul(index, scratch, scratch);
    add(scratch, in_bytes(MultiBranchData::case_array_offset()), scratch);

    // Update the case count
    increment_mdp_data_at(scratch,
			  in_bytes(MultiBranchData::relative_count_offset()),
			  scratch2,
			  scratch3);  

    // The method data pointer needs to be updated.
    update_mdp_by_offset(scratch,
		     in_bytes(MultiBranchData::relative_displacement_offset()),
		     scratch2);

    bind (profile_continue);
  }
#endif // !CORE
}

// add a InterpMonitorElem to stack (see frame_sparc.hpp)

void InterpreterMacroAssembler::add_monitor_to_stack( bool stack_is_empty,
						      Register Rtemp,
						      Register Rtemp2 ) {

  Register Rlimit = Lmonitors;
  const jint delta = frame::interpreter_frame_monitor_size() * wordSize;
  assert( (delta & LongAlignmentMask) == 0, 
	  "sizeof BasicObjectLock must be even number of doublewords");

  sub( SP,        delta, SP);
  sub( Lesp,      delta, Lesp);
  sub( Lmonitors, delta, Lmonitors);

  if (!stack_is_empty) {

    // must copy stack contents down

    Label start_copying, next;

    // untested("monitor stack expansion");
    compute_stack_base(Rtemp);
    ba( false, start_copying );
    delayed()->cmp( Rtemp, Rlimit); // done? duplicated below

    // note: must copy from low memory upwards
    // On entry to loop,
    // Rtemp points to new base of stack, Lesp points to new end of stack (1 past TOS)
    // Loop mutates Rtemp

    bind( next);

    st_ptr(Rtemp2, Rtemp, 0);
    inc(Rtemp, wordSize);
    cmp(Rtemp, Rlimit); // are we done? (duplicated above)

    bind( start_copying );

    brx( notEqual, true, pn, next );
    delayed()->ld_ptr( Rtemp, delta, Rtemp2 );

    // done copying stack
  }
}


void InterpreterMacroAssembler::access_local_ptr( Register index, Register dst ) {
  assert_not_delayed();
  sll(index, LogBytesPerWord, index);
  sub(Llocals, index, index);
  ld_ptr(index, 0, dst);
  // Note:  index must hold the effective address--the iinc template uses it
}

void InterpreterMacroAssembler::access_local_int( Register index, Register dst ) {
  assert_not_delayed();
  sll(index, LogBytesPerWord, index);
  sub(Llocals, index, index);
  ld(index, 0, dst);
  // Note:  index must hold the effective address--the iinc template uses it
}


void InterpreterMacroAssembler::access_local_long( Register index, Register dst ) {
  assert_not_delayed();
  sll(index, LogBytesPerWord, index);
  sub(Llocals, index, index);
  load_unaligned_long(index, -1*wordSize, dst);
}


void InterpreterMacroAssembler::access_local_float( Register index, FloatRegister dst ) {
  assert_not_delayed();
  sll(index, LogBytesPerWord, index);
  sub(Llocals, index, index);
  ldf(FloatRegisterImpl::S, index, 0, dst);
}


void InterpreterMacroAssembler::access_local_double( Register index, FloatRegister dst ) {
  assert_not_delayed();
  sll(index, LogBytesPerWord, index);
  sub(Llocals, index, index);
  load_unaligned_double(index, -1*wordSize, dst);
}


#ifdef ASSERT
void InterpreterMacroAssembler::check_for_regarea_stomp(Register Rindex, int offset, Register Rlimit, Register Rscratch, Register Rscratch1) {
  Label L;

  assert(Rindex != Rscratch, "Registers cannot be same");
  assert(Rindex != Rscratch1, "Registers cannot be same");
  assert(Rlimit != Rscratch, "Registers cannot be same");
  assert(Rlimit != Rscratch1, "Registers cannot be same");
  assert(Rscratch1 != Rscratch, "Registers cannot be same");

  // untested("reg area corruption");
  add(Rindex, offset, Rscratch);
  add(Rlimit, 64 + STACK_BIAS, Rscratch1);
  cmp(Rscratch, Rscratch1);
  brx(Assembler::greaterEqualUnsigned, false, pn, L);
  delayed()->nop();
  stop("regsave area is being clobbered");
  bind(L);
}
#endif // ASSERT

void InterpreterMacroAssembler::store_local_int( Register index, Register src ) {
  assert_not_delayed();
  sll(index, LogBytesPerWord, index);
  sub(Llocals, index, index);
  debug_only(check_for_regarea_stomp(index, 0, FP, G1_scratch, G4_scratch);)
  st(src, index, 0);
}

void InterpreterMacroAssembler::store_local_ptr( Register index, Register src ) {
  assert_not_delayed();
  sll(index, LogBytesPerWord, index);
  sub(Llocals, index, index);
  #ifdef ASSERT
  check_for_regarea_stomp(index, 0, FP, G1_scratch, G4_scratch);
  #endif
  st_ptr(src, index, 0);
}

void InterpreterMacroAssembler::store_local_long( Register index, Register src ) {
  assert_not_delayed();
  sll(index, LogBytesPerWord, index);
  sub(Llocals, index, index);
  #ifdef ASSERT
  check_for_regarea_stomp(index, -1 * wordSize, FP, G1_scratch, G4_scratch);
  #endif
  store_unaligned_long(src, index, -1*wordSize);
}


void InterpreterMacroAssembler::store_local_float( Register index, FloatRegister src ) {
  assert_not_delayed();
  sll(index, LogBytesPerWord, index);
  sub(Llocals, index, index);
  #ifdef ASSERT
  check_for_regarea_stomp(index, 0, FP, G1_scratch, G4_scratch);
  #endif
  stf(FloatRegisterImpl::S, src, index, 0);
}


void InterpreterMacroAssembler::store_local_double( Register index, FloatRegister src ) {
  assert_not_delayed();
  sll(index, LogBytesPerWord, index);
  sub(Llocals, index, index);
  #ifdef ASSERT
  check_for_regarea_stomp(index, -1 * wordSize, FP, G1_scratch, G4_scratch);
  #endif
  store_unaligned_double(src, index, -1*wordSize);
}


int InterpreterMacroAssembler::top_most_monitor_byte_offset() {
  const jint delta = frame::interpreter_frame_monitor_size() * wordSize;
  int rounded_vm_local_words = ::round_to(frame::interpreter_frame_vm_local_words, WordsPerLong);
  return ((-rounded_vm_local_words * wordSize) - delta ) + STACK_BIAS;
}


Address InterpreterMacroAssembler::top_most_monitor() {
  return Address(FP, 0, top_most_monitor_byte_offset());
}


void InterpreterMacroAssembler::compute_stack_base( Register Rdest ) {
  add( Lesp,      wordSize,                                    Rdest );
}


// iinc to statically known place

void InterpreterMacroAssembler::static_iinc( int which_local, jint increment, Register Rtmp, Register Rtmp2 ) {
  if (increment == 0)   return;

  Address loc( Llocals, 0, -which_local * wordSize );
  Register r;

  untested("static iinc memory");
  r = Rtmp;
  ld(loc, r);

  if (is_simm13(increment) )          add( r, increment, r );
  else {  set( increment, Rtmp2 );    add( r, Rtmp2,     r ); }

  st(r, loc);
}


#ifndef CORE
void InterpreterMacroAssembler::increment_invocation_counter( Register Rtmp, Register Rtmp2 ) {
  assert(UseCompiler, "incrementing must be useful");
  Address inv_counter(Lmethod, 0, in_bytes(methodOopDesc::invocation_counter_offset()
			    + InvocationCounter::counter_offset()));
  Address be_counter(Lmethod, 0, in_bytes(methodOopDesc::backedge_counter_offset()
                            + InvocationCounter::counter_offset()));
  int delta = InvocationCounter::count_increment;

  // Load each counter in a register
  ld( inv_counter, Rtmp );
  ld( be_counter, Rtmp2 );

  assert( is_simm13( delta ), " delta too large.");

  // Add the delta to the invocation counter and store the result
  add( Rtmp, delta, Rtmp );

  // Mask the backedge counter
  and3( Rtmp2, InvocationCounter::count_mask_value, Rtmp2 );

  // Store value
  st( Rtmp, inv_counter);

  // Add invocation counter + backedge counter
  add( Rtmp, Rtmp2, Rtmp);

  // Note that this macro must leave the backedge_count + invocation_count in Rtmp!
}

void InterpreterMacroAssembler::increment_backedge_counter( Register Rtmp, Register Rtmp2 ) {
  assert(UseCompiler, "incrementing must be useful");
  Address be_counter(Lmethod, 0, in_bytes(methodOopDesc::backedge_counter_offset()
			    + InvocationCounter::counter_offset()));
  Address inv_counter(Lmethod, 0, in_bytes(methodOopDesc::invocation_counter_offset()
                            + InvocationCounter::counter_offset()));
  int delta = InvocationCounter::count_increment;
  // Load each counter in a register
  ld( be_counter, Rtmp );
  ld( inv_counter, Rtmp2 );

  // Add the delta to the backedge counter
  add( Rtmp, delta, Rtmp );

  // Mask the invocation counter, add to backedge counter
  and3( Rtmp2, InvocationCounter::count_mask_value, Rtmp2 );

  // and store the result to memory
  st( Rtmp, be_counter );

  // Add backedge + invocation counter
  add( Rtmp, Rtmp2, Rtmp );

  // Note that this macro must leave backedge_count + invocation_count in Rtmp!
}

void InterpreterMacroAssembler::test_backedge_count_for_osr( Register backedge_count,
                                                             Register branch_bcp,
                                                             Register Rtmp ) {
  Label did_not_overflow;
  Label overflow_with_error;
  assert_different_registers(backedge_count, Rtmp, branch_bcp);
  assert(UseOnStackReplacement,"Must UseOnStackReplacement to test_backedge_count_for_osr");

  Address limit(Rtmp, address(&InvocationCounter::InterpreterBackwardBranchLimit));
  load_contents(limit, Rtmp);
  cmp(backedge_count, Rtmp);
  br(Assembler::lessUnsigned, false, Assembler::pt, did_not_overflow);
  delayed()->nop();

  // When ProfileInterpreter is on, the backedge_count comes from the 
  // methodDataOop, which value does not get reset on the call to 
  // frequency_counter_overflow().  To avoid excessive calls to the overflow
  // routine while the method is being compiled, add a second test to make sure 
  // the overflow function is called only once every overflow_frequency.
  if (ProfileInterpreter) {
    const int overflow_frequency = 1024;
    andcc(backedge_count, overflow_frequency-1, Rtmp);
    brx(Assembler::notZero, false, Assembler::pt, did_not_overflow);
    delayed()->nop();
  }

  // overflow in loop, pass branch bytecode
  set(6,Rtmp);
  call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::frequency_counter_overflow), branch_bcp, Rtmp);

  // Was an OSR adapter generated?
  // O0 = osr_adapter_frame_address
  // O1 = osr_code
  tst(O0);
  brx(Assembler::zero, false, Assembler::pn, overflow_with_error);
  delayed()->nop();

  // Has the nmethod been invalidated already?	
  ld(O1,nmethod::entry_bci_offset(),O2);
  cmp(O2, InvalidOSREntryBci);
  br(Assembler::equal, false, Assembler::pn, overflow_with_error);
  delayed()->nop();

  // Reserve space for incoming arguments.  This is needed since a C2I adapter
  // could be introduced as part of deoptimization.  The C2I adapter will think
  // it owns the incoming arguments area.  If we do not reserve space for this
  // explicitly, then the C2I adapter might potentially override stuff in the
  // frame above the OSR adapter frame and frame traversal will fail.
  ld_ptr(O1, nmethod::method_offset_in_bytes(), O2); // get methodOop
  lduh(Address(O2, 0, in_bytes(methodOopDesc::size_of_parameters_offset())), O2);
  // get #parameters
  round_to(O2, WordsPerLong);                    // align
  sll(O2, LogBytesPerWord, O2);
  sub(SP, O2, SP);              // Adjust SP

  // The argument is passed in Lesp, which the OSR prolog will copy           
  // into interpreter_arg_ptr_reg.  Lesp needs to point to java local 0
  mov( Llocals, Lesp );
  // Pass the monitors address in the inline-cache register
  add( top_most_monitor(), G5 );

  // We want to return to the OSRAdaptor, the addr of which is in o0.
  // Due to the way returns are handled in sparc land, we need to set
  // o7 to (address - 8 bytes)
  sub(O0, 2 * BytesPerInstWord, O7);

  // We have to force all register windows to the stack, including our own
  save_frame(0);
  flush_windows();
  restore();

  // The osr return logic must restore the SP of the activation that
  // invoked this interpreted activation, since the interpreter
  // extends a caller's frame to allocate a method's non-parameter
  // locals next to its parameters.  Compute the correct SP into IsavedSP
  lduh(Lmethod, in_bytes(methodOopDesc::size_of_parameters_offset()), G3_scratch);
  lduh(Lmethod, in_bytes(methodOopDesc::size_of_locals_offset()), G1_scratch);
  compute_extra_locals_size_in_bytes(G3_scratch, G1_scratch, G1_scratch);
  add(FP, G1_scratch, IsavedSP);

  // Jump to the osr code.
  ld_ptr(O1, nmethod::osr_entry_point_offset(), O2);
  jmp(O2,G0);
  delayed()->nop();

  bind(overflow_with_error);

  bind(did_not_overflow);
}
#endif // not CORE



void InterpreterMacroAssembler::verify_oop(Register reg, TosState state) {
  if (state == atos) { MacroAssembler::verify_oop(reg); }
}


// local helper function for the verify_oop_or_return_address macro
static bool verify_return_address(methodOop m, int bci) {
#ifndef PRODUCT
  address pc = (address)(m->constMethod())
             + in_bytes(constMethodOopDesc::codes_offset()) + bci;
  // assume it is a valid return address if it is inside m and is preceded by a jsr
  if (!m->contains(pc))                                          return false;
  address jsr_pc;
  jsr_pc = pc - Bytecodes::length_for(Bytecodes::_jsr);
  if (*jsr_pc == Bytecodes::_jsr   && jsr_pc >= m->code_base())    return true;
  jsr_pc = pc - Bytecodes::length_for(Bytecodes::_jsr_w);
  if (*jsr_pc == Bytecodes::_jsr_w && jsr_pc >= m->code_base())    return true;
#endif // PRODUCT
  return false;
}


void InterpreterMacroAssembler::verify_oop_or_return_address(Register reg, Register Rtmp) {
  if (!VerifyOops)  return;
  // the VM documentation for the astore[_wide] bytecode allows
  // the TOS to be not only an oop but also a return address
  Label test;
  Label skip;
  // See if it is an address (in the current method):

  mov(reg, Rtmp);
  const int log2_bytecode_size_limit = 16;
  srl(Rtmp, log2_bytecode_size_limit, Rtmp);
  br_notnull( Rtmp, false, pt, test );
  delayed()->nop();

  // %%% should use call_VM_leaf here?
  save_frame_and_mov(0, Lmethod, O0, reg, O1);
  save_thread(L7_thread_cache);
  call(CAST_FROM_FN_PTR(address,verify_return_address), relocInfo::none);
  delayed()->nop();
  restore_thread(L7_thread_cache);
  br_notnull( O0, false, pt, skip );
  delayed()->restore();

  // Perform a more elaborate out-of-line call
  // Not an address; verify it:
  bind(test);
  verify_oop(reg);
  bind(skip);
}


void InterpreterMacroAssembler::verify_FPU(int stack_depth, TosState state) {
  if (state == ftos || state == dtos) MacroAssembler::verify_FPU(stack_depth);
}


// Inline assembly for:
//
// if (thread is in interp_only_mode) {
//   InterpreterRuntime::post_method_entry();
// }
// if (*jvmpi::event_flags_array_at_addr(JVMPI_EVENT_METHOD_ENTRY ) ||
//     *jvmpi::event_flags_array_at_addr(JVMPI_EVENT_METHOD_ENTRY2)   ) {
//   SharedRuntime::jvmpi_method_entry(method, receiver);
// }

void InterpreterMacroAssembler::notify_method_entry() {
  // Whenever JVMTI puts a thread in interp_only_mode, method
  // entry/exit events are sent for that thread to track stack
  // depth.  If it is possible to enter interp_only_mode we add
  // the code to check if the event should be sent.
  if (JvmtiExport::can_post_interpreter_events()) {
    Label L;
    Register temp_reg = O5;

    const Address interp_only       (G2_thread, 0, in_bytes(JavaThread::interp_only_mode_offset()));

    ld(interp_only, temp_reg);
    tst(temp_reg);
    br(zero, false, pt, L);
    delayed()->nop();
    call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::post_method_entry));
    bind(L);
  }
  { Label E;
    Label S;
    Register temp_reg = O5;
    Address event0(temp_reg, (address)jvmpi::event_flags_array_at_addr(JVMPI_EVENT_METHOD_ENTRY ), relocInfo::none);
    load_contents(event0, temp_reg);
    cmp(temp_reg, (int)JVMPI_EVENT_ENABLED);
    br(equal, false, pn, S);
    delayed()->nop();
    Address event1(temp_reg, (address)jvmpi::event_flags_array_at_addr(JVMPI_EVENT_METHOD_ENTRY2), relocInfo::none);
    load_contents(event1, temp_reg);
    cmp(temp_reg, (int)JVMPI_EVENT_ENABLED);
    br(notEqual, false, pt, E);
    delayed()->nop();
    bind(S);
    // notify method entry
    { Label L;
      const Address access_flags(Lmethod, 0, in_bytes(methodOopDesc::access_flags_offset()));
      ld(access_flags, temp_reg);
      and3(temp_reg, JVM_ACC_STATIC, temp_reg);
      cmp(temp_reg, JVM_ACC_STATIC); // check if method is static
      br(equal, true, pn, L);        // if static we're done
      delayed()->mov(G0, temp_reg);  // receiver = NULL for a static method
      ld_ptr(Llocals, 0, temp_reg);  // otherwise get receiver
      bind(L);
      call_VM(noreg, CAST_FROM_FN_PTR(address, SharedRuntime::jvmpi_method_entry), Lmethod, temp_reg);
    }
    bind(E);
  }
}


// Inline assembly for:
//
// if (thread is in interp_only_mode) {
//   // save result
//   InterpreterRuntime::post_method_exit();
//   // restore result
// }
// if (*jvmpi::event_flags_array_at_addr(JVMPI_EVENT_METHOD_EXIT)) {
//   // save result
//   SharedRuntime::jvmpi_method_exit();
//   // restore result
// }
// 
// Native methods have their result stored in d_tmp and l_tmp
// Java methods have their result stored in the expression stack

void InterpreterMacroAssembler::notify_method_exit(bool is_native_method, TosState state) {
  // Whenever JVMTI puts a thread in interp_only_mode, method
  // entry/exit events are sent for that thread to track stack
  // depth.  If it is possible to enter interp_only_mode we add
  // the code to check if the event should be sent.
  if (JvmtiExport::can_post_interpreter_events()) {
    Label L;
    Register temp_reg = O5;

    const Address interp_only       (G2_thread, 0, in_bytes(JavaThread::interp_only_mode_offset()));

    ld(interp_only, temp_reg);
    tst(temp_reg);
    br(zero, false, pt, L);
    delayed()->nop();

    // Note: frame::interpreter_frame_result has a dependency on how the 
    // method result is saved across the call to post_method_exit. For
    // native methods it assumes the result registers are saved to
    // l_scratch and d_scratch. If this changes then the interpreter_frame_result
    // implementation will need to be updated too.

    if (is_native_method) {
      stf(FloatRegisterImpl::D, F0, d_tmp);
#ifdef _LP64
      stx(O0, l_tmp);
#else
      std(O0, l_tmp);
#endif
    } else {
      push(state);
    }

    call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::post_method_exit));

    if (is_native_method) {
      ldf(FloatRegisterImpl::D, d_tmp, F0);
#ifdef _LP64
      ldx(l_tmp, O0);
#else
      ldd(l_tmp, O0);
#endif
    } else {
      pop(state);
    }

    bind(L);

  }
  notify_jvmpi_method_exit(is_native_method, state);
}

void InterpreterMacroAssembler::notify_jvmpi_method_exit(bool is_native_method, TosState state) {
  { Label E;
    Register temp_reg = O5;
    Address event0(temp_reg, (address)jvmpi::event_flags_array_at_addr(JVMPI_EVENT_METHOD_EXIT ), relocInfo::none);
    load_contents(event0, temp_reg);
    cmp(temp_reg, (int)JVMPI_EVENT_ENABLED);
    br(notEqual, false, pn, E);
    delayed()->nop();
    if (is_native_method) {
      stf(FloatRegisterImpl::D, F0, d_tmp);
#ifdef _LP64
      stx(O0, l_tmp);
#else
      std(O0, l_tmp);
#endif
    } else {
      push(state);
    }
    call_VM(noreg, CAST_FROM_FN_PTR(address, SharedRuntime::jvmpi_method_exit), Lmethod);
    if (is_native_method) {
      ldf(FloatRegisterImpl::D, d_tmp, F0);
#ifdef _LP64
      ldx(l_tmp, O0);
#else
      ldd(l_tmp, O0);
#endif
    } else {
      pop(state);
    }
    bind(E);
  }
}


//Reconciliation History
// 1.19 97/11/16 18:25:24 interp_masm_i486.cpp
// 1.20 97/11/24 14:07:35 interp_masm_i486.cpp
// 1.24 97/12/11 19:23:37 interp_masm_i486.cpp
// 1.26 98/01/30 10:04:18 interp_masm_i486.cpp
// 1.28 98/02/10 13:33:52 interp_masm_i486.cpp
// 1.29 98/02/23 15:09:33 interp_masm_i486.cpp
// 1.31 98/03/04 13:43:08 interp_masm_i486.cpp
// 1.34 98/03/30 17:20:31 interp_masm_i486.cpp
// 1.39 98/04/22 18:20:45 interp_masm_i486.cpp
// 1.40 98/04/24 12:52:40 interp_masm_i486.cpp
// 1.41 98/04/30 12:10:40 interp_masm_i486.cpp
// 1.42 98/05/11 13:42:33 interp_masm_i486.cpp
// 1.44 98/06/22 17:40:10 interp_masm_i486.cpp
// 1.47 98/07/01 15:42:15 interp_masm_i486.cpp
// 1.49 98/07/24 13:32:32 interp_masm_i486.cpp
// 1.52 98/10/02 15:21:57 interp_masm_i486.cpp
// 1.55 98/10/05 02:31:58 interp_masm_i486.cpp
// 1.57 98/10/21 17:27:40 interp_masm_i486.cpp
// 1.59 98/10/26 10:29:27 interp_masm_i486.cpp
// 1.61 98/11/04 13:20:33 interp_masm_i486.cpp
// 1.63 98/11/25 16:10:35 interp_masm_i486.cpp
// 1.64 98/12/01 16:12:22 interp_masm_i486.cpp
// 1.65 99/01/05 09:25:37 interp_masm_i486.cpp
// 1.66 99/01/13 16:18:05 interp_masm_i486.cpp
// 1.68 99/01/25 14:07:46 interp_masm_i486.cpp
// 1.72 99/03/10 18:06:14 interp_masm_i486.cpp
// 1.75 99/04/19 13:50:32 interp_masm_i486.cpp
// 1.78 99/06/28 09:58:09 interp_masm_i486.cpp
// 1.74 99/04/01 16:52:54 interp_masm_i486.cpp
// 1.75 99/04/05 16:57:56 interp_masm_i486.cpp
// 1.75 99/04/12 12:58:21 interp_masm_i486.cpp
// 1.79 99/07/06 16:02:50 interp_masm_i486.cpp
//End
