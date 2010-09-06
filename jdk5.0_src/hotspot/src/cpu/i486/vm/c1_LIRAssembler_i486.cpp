#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_LIRAssembler_i486.cpp	1.123 04/05/05 16:15:15 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_c1_LIRAssembler_i486.cpp.incl"


NEEDS_CLEANUP // remove this definitions ?
const Register RECEIVER    = ecx;   // where receiver is preloaded
const Register IC_Klass    = eax;   // where the IC klass is cached
const Register SYNC_header = eax;   // synchronization header
const Register SHIFT_count = ecx;   // where count for shift operations must be

#define __ _masm->


void LIR_Assembler::safepoint_nop() {
  __ fat_nop();
}


bool LIR_Assembler::is_small_constant(LIR_Opr opr) {
  if (opr->is_constant()) {
    LIR_Const* constant = opr->as_constant_ptr();
    switch (constant->type()) {
    case T_INT: {
      return true;
    }

    default:
      return false;
    }
  }
  return false;
}



//--------------fpu register translations-----------------------


void LIR_Assembler::fpu_pop(RInfo reg) {
  frame_map()->fpu_stack()->pop(reg.fpu());
}


void LIR_Assembler::fpu_push(RInfo reg) {
 frame_map()-> fpu_stack()->push(reg.fpu());
}


void LIR_Assembler::set_fpu_stack_empty() {
  frame_map()->fpu_stack()->clear();
}


// forces reg to be on FPU TOS
void LIR_Assembler::fpu_on_tos(RInfo reg) {
  if (frame_map()->fpu_stack()->has_stack_offset(reg.fpu(), 0)) return; // register already on TOS
  int delta = frame_map()->fpu_stack()->move_on_tos(reg.fpu());
  __ fxch(delta);
  assert(frame_map()->fpu_stack()->has_stack_offset(reg.fpu(), 0), "should be on TOS?");
}


// if two regs are not on TOS return false;
// if two regs are on TOS and in right order return false;
// if two regs are on TOS and in reverse order, return true
bool LIR_Assembler::must_swap_two_on_tos(RInfo tos0, RInfo tos1) {
  bool tos1_is0 = frame_map()->fpu_stack()->has_stack_offset(tos1.fpu(), 0); 
  bool tos0_is1 = frame_map()->fpu_stack()->has_stack_offset(tos0.fpu(), 1); 
  return tos0_is1 && tos1_is0;
}


// Forces tos0 and tos1 to be on FPU TOS; if "must_be_ordered" is true,
// then tos0 must be in ST(0) and tos1 must be in ST(1)
void LIR_Assembler::fpu_two_on_tos(RInfo tos0, RInfo tos1, bool must_be_ordered) {
  if (!must_be_ordered) {
    bool tos1_is1 = frame_map()->fpu_stack()->has_stack_offset(tos1.fpu(), 1); 
    bool tos1_is0 = frame_map()->fpu_stack()->has_stack_offset(tos1.fpu(), 0); 
    bool tos0_is1 = frame_map()->fpu_stack()->has_stack_offset(tos0.fpu(), 1); 
    bool tos0_is0 = frame_map()->fpu_stack()->has_stack_offset(tos0.fpu(), 0); 
    if (tos1_is1 && tos0_is0) {
      return; // Done
    } else if (tos1_is0 && tos0_is1) {
      frame_map()->fpu_stack()->swap(); // fake exchange; no fxch emitted
      return; // Done!
    }
  }
  if (!frame_map()->fpu_stack()->has_stack_offset(tos1.fpu(), 1)) {
    fpu_on_tos(tos1);
    frame_map()->fpu_stack()->swap();
    __ fxch(1);
  }
  assert(frame_map()->fpu_stack()->has_stack_offset(tos1.fpu(), 1), "did not work?");
  fpu_on_tos(tos0);
}

void LIR_Assembler::set_24bit_FPU() {
  __ fldcw(Address((int)StubRoutines::addr_fpu_cntrl_wrd_24(), relocInfo::none));
}

void LIR_Assembler::reset_FPU() {
  __ fldcw(Address((int)StubRoutines::addr_fpu_cntrl_wrd_std(), relocInfo::none));
}

void LIR_Assembler::remove_fpu_result(RInfo reg) {
  bool slot_is_empty = frame_map()->fpu_stack()->pop(reg.fpu());
  assert(slot_is_empty, "inconsistent fpu stack sim");
  __ fpop();
}

void LIR_Assembler::fpop() {
  __ fpop();
}

void LIR_Assembler::breakpoint() {
  __ int3();
}

void LIR_Assembler::push(LIR_Opr opr) {
  if (opr->is_register()) {
    RInfo reg = opr->rinfo();
    if (reg.is_float_kind()) {
      fpu_on_tos(reg);
      if (reg.is_float()) { 
        __ push_reg(eax);
        __ fstp_s(Address(esp));
      } else {
        __ push_reg(eax);
        __ push_reg(eax);
        __ fstp_d (Address(esp));
      } 
      fpu_pop(reg);
    } else if (reg.is_word()) {
      __ push_reg(reg.as_register());
    } else if (reg.is_long()) {
      __ push_reg(reg.as_register_hi());
      __ push_reg(reg.as_register_lo());
    } else {
      ShouldNotReachHere();
    }
  } else if (opr->is_stack()) {
    __ push_addr(frame_map()->address_for_name(opr->single_stack_ix(), false));
  } else if (opr->is_constant()) {
    LIR_Const* const_opr = opr->as_constant_ptr();
    if (const_opr->type() == T_OBJECT) {
      __ push_oop(const_opr->as_jobject());
    } else {
      __ push_jint(const_opr->as_jint());
    }
  } else {
    ShouldNotReachHere();
  }
}

void LIR_Assembler::pop(LIR_Opr opr) {
  if (opr->is_single_cpu()) {
    __ pop(opr->rinfo().as_register());
  } else if (opr->is_single_fpu() || opr->is_double_fpu()) {
    // Only used for store elimination bug fix
    fpu_on_tos(opr->rinfo());
    remove_fpu_result(opr->rinfo());
  } else {
    assert(false, "Must be single word register or floating-point register");
  }
}

//-------------------------------------------
Address LIR_Assembler::as_Address(LIR_Address* addr) {
  if (addr->base()->is_illegal()) {
    assert(addr->index()->is_illegal(), "must be illegal too");
    return Address(addr->disp(), relocInfo::none);
  }

  Register base = addr->base()->rinfo().as_register();

  if (addr->index()->is_illegal()) {
    return Address( base, addr->disp());
  } else if (addr->index()->is_single_cpu()) {
    Register index = addr->index()->rinfo().as_register();
    return Address(base, index, (Address::ScaleFactor) addr->scale(), addr->disp());
  } else if (addr->index()->is_constant()) {
    int addr_offset = (addr->index()->as_constant_ptr()->as_jint() << addr->scale()) + addr->disp();

    return Address(base, addr_offset);
  } else {
    Unimplemented();
    return Address();
  }
}


Address LIR_Assembler::as_Address_hi(LIR_Address* addr) {
  Register base = addr->base()->rinfo().as_register();
  if (addr->index()->is_illegal()) {
    return Address(base, addr->disp() + BytesPerWord);
  } else {
    return Address(base, addr->index()->rinfo().as_register(), Address::times_1, BytesPerWord + addr->disp());
  }
}


Address LIR_Assembler::as_Address_lo(LIR_Address* addr) {
  return as_Address(addr);
}


Address LIR_Assembler::as_ArrayAddress(LIR_Address* addr, BasicType type) {
  Address result = as_Address(addr);
  assert(LIR_Address::scale(type) == addr->scale() || addr->index()->is_illegal(), "should match");
  return result;
}


// This is a copy form CodeEmitter
void LIR_Assembler::emit_osr_entry(IRScope* scope, int number_of_locks, Label* continuation, int osr_bci) { 
  assert(scope->is_top_scope(), "inlined OSR not yet implemented");
  _offsets->_osr_offset = code_offset();
  // we jump here if osr happens with the interpreter
  // state set up to continue at the beginning of the
  // loop that triggered osr - in particular, we have
  // the following registers setup:
  //
  // edi: interpreter locals pointer
  // eax: interpreter locks pointer
  // edx: osr nmethod
  // esi: bcp
  //

  // build frame
  ciMethod* m = scope->method();
  __ build_frame(initial_frame_size_in_bytes());

  // note: we do osr only if the expression stack at the loop beginning is empty,
  //       in which case the spill area is empty too and we don't have to setup
  //       spilled locals
  //
  // copy monitors
  // eax: pointer to locks
  { assert(frame::interpreter_frame_monitor_size() == BasicObjectLock::size(), "adjust code below");
    assert(BasicLock::displaced_header_offset_in_bytes() == 0, "adjust code below");
    for (int i = 0; i < number_of_locks; i++) {
      int slot_offset = - i * BasicObjectLock::size();
#ifdef ASSERT
      { Label L;
        __ cmpl(Address(eax, slot_offset * BytesPerWord + BasicObjectLock::obj_offset_in_bytes()), (int)NULL);
        __ jcc(Assembler::notZero, L);
        __ stop("locked object is NULL");
        __ bind(L);
      }
#endif
      __ movl(edx, Address(eax, slot_offset * BytesPerWord + BasicObjectLock::lock_offset_in_bytes()));
      __ movl(frame_map()->address_for_monitor_lock_index(i), edx);
      __ movl(edx, Address(eax, slot_offset * BytesPerWord + BasicObjectLock::obj_offset_in_bytes()));
      __ movl(frame_map()->address_for_monitor_object_index(i), edx);
    }
  }



  // copy locals
  // Iterate through all types and copy
  LocalList locals = *scope->locals();
  BlockBegin* osr_entry = compilation()->hir()->osr_entry();
  const LocalMapping* cached_locals = osr_entry->local_mapping();
  int index_of_local_in_eax = -1;
  int index_of_local_in_edi = -1;
  for (int i = 0; i < locals.length(); i++) {
    Local* aLocal = locals[i];
    int index = aLocal->java_index();
    int name = aLocal->local_name();
    RInfo reg = cached_locals != NULL ? cached_locals->get_cache_reg(name, aLocal->type()->tag()) : norinfo;
    if (aLocal->type()->size() == 2) {
      assert(reg.is_illegal(), "we do not cache two-wrd types..yet");
      // double word
      __ movl(eax, Address(edi, -index*BytesPerWord));
      __ movl(frame_map()->address_for_local_name(name, true, true), eax); // hi word
      __ movl(eax, Address(edi, -(index + 1)*BytesPerWord));
      __ movl(frame_map()->address_for_local_name(name, true, false), eax); // lo word
    } else {
      if (reg.is_illegal()) { 
        __ movl(eax, Address(edi, -index*BytesPerWord));
        __ movl(frame_map()->address_for_local_name(name, false), eax);
      } else {
        Register r = reg.as_register();
        // delay initialization of registers being used to copy locals from the interpreted frame
        if (r == eax) {
          index_of_local_in_eax = index;
        } else if (r == edi) {
          index_of_local_in_edi = index;
        } else {
          __ movl(reg.as_register(), Address(edi, -index*BytesPerWord));
        }
      }
    }
  }
  if (index_of_local_in_eax >= 0) {
    __ movl(eax, Address(edi, -index_of_local_in_eax*BytesPerWord));
  }
  if (index_of_local_in_edi >= 0) {
    __ movl(edi, Address(edi, -index_of_local_in_edi*BytesPerWord));
  }

  // sometimes the receiver is expected in ecx, copy always if any
  if (m->max_locals() > 0) {
    __ movl(ecx, frame_map()->address_for_local_name(frame_map()->name_for_argument(0), false));
  }
  // continue at loop header/body corresponding to the osr'ed interpreter loop
  __ jmp(*continuation);
}


void LIR_Assembler::emit_constants() { 
  _const_table.emit_entries(masm(), true);
}


// In prolog esp points to return address, and frame has not been built yet
Address LIR_Assembler::receiver_address_in_prolog(ciMethod* method) {
  return Address(esp, method->arg_size() * wordSize);
}



// inline cache check; done before the frame is built.
int LIR_Assembler::check_icache() {
  return check_icache(RECEIVER, IC_Klass);
}


int LIR_Assembler::check_icache(Register receiver, Register ic_klass) {
  assert(receiver == RECEIVER && ic_klass == IC_Klass, "must be the standard registers");
  if (!VerifyOops) {
    // insert some nops so that the verified entry point is aligned on CodeEntryAlignment
    while ((__ offset() + 9) % CodeEntryAlignment != 0) {
      __ nop();
    }
  }
  int offset = __ offset();
  __ inline_cache_check(RECEIVER, IC_Klass);
  assert(__ offset() % CodeEntryAlignment == 0 || VerifyOops, "alignment must be correct");
  if (VerifyOops) {
    // force alignment after the cache check.
    // It's been verified to be aligned if !VerifyOops
    __ align(CodeEntryAlignment);
  }
  return offset;
}


void LIR_Assembler::jvmpi_method_exit(ciMethod* method, bool result_is_oop) {
  assert(compilation()->jvmpi_event_method_exit_enabled(), "wrong call");
  // save flags and potential floating-point result
  __ pushfd();
  __ push_FPU_state();
  // save potential integer result
  if (result_is_oop) {
    // pass oop result in ecx to runtime stub; which will preserve it in current thread
    __ movl(ecx, eax);
  } else {
    // no oop to be preserved by runtime stub
    __ movl(ecx, (jobject)NULL);
    // save potential integer result
    __ pushl(eax);
    __ pushl(edx);
  }
  __ movl (eax, method->encoding());
  __ call(Runtime1::entry_for(Runtime1::jvmpi_method_exit_id), relocInfo::runtime_call_type);
  // eax is now the method's oop result, possibly updated by GC, or null
  if (!result_is_oop) {
    // restore potential integer result
    __ popl(edx);
    __ popl(eax);
  }
  // restore potential floating-point result and flags
  __ pop_FPU_state(); 
  __ popfd();
}


void LIR_Assembler::jvmpi_method_enter(CodeEmitInfo* info) {
  assert(compilation()->jvmpi_event_method_entry_enabled() || compilation()->jvmpi_event_method_entry2_enabled(), "wrong call");
  ciMethod* method = compilation()->method(); 
  if (method->is_static()) { 
    __ movl(ecx, (jobject)NULL); 
  } else { 
    // receiver is in ecx
    // __ movl(ecx, RECEIVER); 
  } 
  __ movl (eax, method->encoding()); 
  // eax: method
  // ecx: receiver or NULL
  __ call(Runtime1::entry_for(Runtime1::jvmpi_method_entry_id), relocInfo::runtime_call_type); 
  add_call_info(code_offset(), info); 
  if (!method->is_static()) {
    __ movl(RECEIVER, frame_map()->address_for_local_name(frame_map()->name_for_argument(0), false));
  } else if (method->is_native()) {
    __ nop(); // for natives, there will be debug information conflicts without a different pc_offset
  }
}


void LIR_Assembler::jobject2reg_with_patching(Register reg, CodeEmitInfo* info) {
  jobject o = NULL;
  PatchingStub* patch = new PatchingStub(_masm, PatchingStub::load_klass_id);
  __ movl(reg, o);
  patching_epilog(patch, LIR_Op1::patch_normal, noreg, info);
}


NEEDS_CLEANUP
// This monitor-exit is used only by exception handler and native_method_exit.
// we have to replicate monitor-exit code here
// Also, monitor-enter is replicated here as it is used be the method_enter.
// (it exists in LIR form already) until emission of exception handler is 
// done during LIR
void LIR_Assembler::monitorenter(RInfo obj_, RInfo lock_, Register hdr, int monitor_no, CodeEmitInfo* info) {
  if (!GenerateSynchronizationCode) return;
  
  Register obj_reg  = obj_.as_register();
  Register lock_reg = lock_.as_register();
  // setup registers (hdr must be eax for lock_object)
  assert(obj_reg != SYNC_header && lock_reg != SYNC_header, "eax must be available here");
  assert(hdr == SYNC_header, "wrong header register");
  // compute pointer to BasicLock
  __ leal(lock_reg, frame_map()->address_for_monitor_lock_index(monitor_no));
  __ verify_oop(obj_reg);
  // lock object
  assert(__ esp_offset() == 0, "frame size should be fixed");
  MonitorEnterStub* slow_case = new MonitorEnterStub(obj_, lock_, info);
  _slow_case_stubs->append(slow_case);
  if (UseFastLocking) {
    // try inlined fast locking first, revert to slow locking if it fails
    // note: lock_reg points to the displaced header since the displaced header offset is 0!
    assert(BasicLock::displaced_header_offset_in_bytes() == 0, "lock_reg must point to the displaced header");
    add_debug_info_for_null_check_here(info);
    __ lock_object(hdr, obj_reg, lock_reg, *slow_case->entry());
  } else {
    // always do slow locking
    // note: the slow locking code could be inlined here, however if we use
    //       slow locking, speed doesn't matter anyway and this solution is
    //       simpler and requires less duplicated code - additionally, the
    //       slow locking code is the same in either case which simplifies
    //       debugging
    __ jmp(*slow_case->entry());
  }
  // done
  __ bind(*slow_case->continuation());
}

void LIR_Assembler::monitorexit(RInfo obj_, RInfo lock_, Register new_hdr, int monitor_no, Register exception) {
  if (!GenerateSynchronizationCode) return;

  if (exception->is_valid()) {
    // preserve exception
    // note: the monitor_exit runtime call is a leaf routine
    //       and cannot block => no GC can happen
    // The slow case (MonitorAccessStub) uses the first two stack slots
    // ([esp+0] and [esp+4]), therefore we store the exception at [esp+8]
    __ movl (Address(esp, 2*wordSize), exception);
  }

  Register obj_reg  = obj_.as_register();
  Register lock_reg = lock_.as_register();

  // setup registers (lock_reg must be eax for lock_object)
  assert(obj_reg != SYNC_header && lock_reg != SYNC_header, "eax must be available here");
  Register hdr = lock_reg;
  assert(new_hdr == SYNC_header, "wrong register");
  lock_reg = new_hdr;
  // compute pointer to BasicLock
  Address lock_addr = frame_map()->address_for_monitor_lock_index(monitor_no);
  __ leal(lock_reg, lock_addr);
  // unlock object
  MonitorAccessStub* slow_case = new MonitorExitStub(lock_, true, monitor_no);
  // _slow_case_stubs->append(slow_case);
  // temporary fix: must be created after exceptionhandler, therefore as call stub
  _call_stubs->append(slow_case);
  if (UseFastLocking) {
    // try inlined fast unlocking first, revert to slow locking if it fails
    // note: lock_reg points to the displaced header since the displaced header offset is 0!
    assert(BasicLock::displaced_header_offset_in_bytes() == 0, "lock_reg must point to the displaced header");
    __ unlock_object(hdr, obj_reg, lock_reg, *slow_case->entry());
  } else {
    // always do slow unlocking
    // note: the slow unlocking code could be inlined here, however if we use
    //       slow unlocking, speed doesn't matter anyway and this solution is
    //       simpler and requires less duplicated code - additionally, the
    //       slow unlocking code is the same in either case which simplifies
    //       debugging
    __ jmp(*slow_case->entry());
  }
  // done
  __ bind(*slow_case->continuation());

  if (exception->is_valid()) {
    // restore exception
    __ movl (exception, Address(esp, 2 * wordSize));
  }
}

// This specifies the esp decrement needed to build the frame
int LIR_Assembler::initial_frame_size_in_bytes() {
  // if rounding, must let FrameMap know!
  return (frame_map()->framesize() - 2)  * BytesPerWord; // subtract two words to account for return address and link
}


void LIR_Assembler::load_receiver_reg(Register reg) {
  __ movl(reg, frame_map()->address_for_local_name(frame_map()->name_for_argument(0), false));
}


int LIR_Assembler::emit_exception_handler() { 
  // if the last instruction is a call (typically to do a throw which
  // is coming at the end after block reordering) the return address
  // must still point into the code area in order to avoid assertion
  // failures when searching for the corresponding bci => add a nop
  // (was bug 5/14/1999 - gri)
  // Lazy deopt bug 4932387. If last instruction is a call then we
  // need an area to patch where we won't overwrite the exception
  // handler. This means we need 5 bytes. Could use a fat_nop 
  // but since this never gets executed it doesn't really make
  // much difference.
  // 
  for (int i = 0; i < NativeCall::instruction_size ; i++ ) {
    __ nop();
  }

  // generate code for exception handler
  ciMethod* method = compilation()->method();
  __ bind (_throw_entry_label);
  int offset = code_offset();
  // Temporary solution: always generate exception handlers
  // check if exception handler exist
  // tos: last argument (if the exception happened at a call and there are arguments)
  // eax: exception
  // edx: throwing pc
  __ exception_handler(compilation()->has_exception_handlers(), initial_frame_size_in_bytes());

  __ bind (_unwind_entry_label);
  // unlock the receiver/klass if necessary
  // eax: exception
  if (method->is_synchronized()) {
    monitorexit(FrameMap::_ebxRInfo, FrameMap::_ecxRInfo, SYNC_header, 0, eax);
  }

  if (compilation()->jvmpi_event_method_exit_enabled()) {
    jvmpi_method_exit(method, true);
  }
  // unwind activation and forward exception to caller
  // eax: exception
  __ jmp(Runtime1::entry_for(Runtime1::unwind_exception_id), relocInfo::runtime_call_type);
  return offset;
}

void LIR_Assembler::emit_method_entry(LIR_Emitter* emit, IRScope* scope) {
  ciMethod* m = compilation()->method();

  // to check: make alignment 16 bytes for Pentium Pro.
  // verify this and measure it: we hope that thanks to CHA, we can almost
  // always jump to the verified entry point anyway.
  const int AlignEntryCode = 4;

  // We misalign the entry point so that the verified entry point will be
  // aligned and no nops between check_icache and the verified entry. The code
  // in the unverified entry is known and fixed and so we can pad with these
  // nops.
  // At this point only native method stubs need this alignment since only
  // they can be zombied with threads active. When c1 deoptimizes this
  // will have to happen for all deoptimization candidates.
  //

  // entry point
  _offsets->_ep_offset = __ offset();
  if (needs_icache(m)) {
    _offsets->_ep_offset = check_icache();
  }

  // verified entry point
  assert(__ offset() % CodeEntryAlignment == 0, "want verified entry point aligned on CodeEntryAlignment");
  _offsets->_vep_offset = __ offset();
  _offsets->_iep_offset = __ offset();

  // Native method stubs can be zombied at non-safepoints so they need an
  // instruction large enough for the patching in patch_verified_entry
  // (5+ bytes)
  if (m->is_native()) {
    __ fat_nop();
  }
  if (C1Breakpoint) __ int3();

  // build frame
  _offsets->_code_offset = __ offset();
  __ verify_FPU(0, "method_entry");

  // For the native method Object.hashCode(), if the object's header can be read
  // while the object is unlocked and it holds a valid (non-zero) hash code,
  // return the hash value immediately without creating a frame.
  if (InlineObjectHash && m->intrinsic_id() == methodOopDesc::_hash && !compilation()->jvmpi_event_method_enabled()) {
    __ fast_ObjectHashCode(RECEIVER, eax);
  }

  __ build_frame(initial_frame_size_in_bytes());

  if (m->is_synchronized() && GenerateSynchronizationCode) {
    const RInfo obj_info = FrameMap::_ecxRInfo; // object to lock
    if (m->is_static()) {
      __ movl(obj_info.as_register(), m->holder()->java_mirror()->encoding());
    } else {
      // receiver is already loaded
    }
    RInfo lock_info = FrameMap::_esiRInfo;
    CodeEmitInfo* info = m->is_native() ? new CodeEmitInfo(scope, SynchronizationEntryBCI, NULL)
                                        : new CodeEmitInfo(emit,  SynchronizationEntryBCI, NULL, scope->start()->state(), NULL);
#ifdef ASSERT
    if (!m->is_native()) {
      // check that we have the lock stack
      assert(info->stack()->locks().length() == 1, "missing locking information");
    }
#endif
    monitorenter(obj_info, lock_info, SYNC_header, 0, info); // sync. slot is 0, however should really use a function here
  }

  if (compilation()->jvmpi_event_method_entry_enabled() || compilation()->jvmpi_event_method_entry2_enabled()) {
    // we need the map for arguments only, therefore we use SynchronizationEntryBCI,
    // although there may be no synchronization in this method
    CodeEmitInfo info(scope, SynchronizationEntryBCI, NULL);

    jvmpi_method_enter(&info);
  }
}

void LIR_Assembler::emit_native_call(address native_entry, CodeEmitInfo* info) {
  ciMethod* m = compilation()->method();
  assert (m->is_native(), "Must be a native method");
 
  // get current pc information
  int pc_in_native        = (int)__ pc();
  int pc_in_native_offset = __ offset();

  // pass parameters (other than the receiver)

  ciSignature* sig = m->signature();
  int esp_offset = 2;
  int local_index = (m->is_static() ? 0 : 1);

  for (int i = 0; i < sig->count(); i++) { // note: sig->count() excludes the receiver!
    if (! sig->type_at(i)->is_primitive_type()) {
      // use NULL for NULL oops, use handle otherwise
      Register arg = eax;
      Label L;
      __ movl(arg, frame_map()->address_for_local_name(frame_map()->name_for_argument(local_index), false));
      __ testl(arg, arg);
      __ jcc(Assembler::zero, L);
      __ leal(arg, frame_map()->address_for_local_name(frame_map()->name_for_argument(local_index), false));
      __ bind(L);
      push_parameter(arg, esp_offset);
    } else if (sig->type_at(i)->is_two_word()) {
      // double-word argument
      Register arg_lo = eax;
      Register arg_hi = edx;
      __ movl(arg_lo, frame_map()->address_for_local_name(frame_map()->name_for_argument(local_index), true, false));
      __ movl(arg_hi, frame_map()->address_for_local_name(frame_map()->name_for_argument(local_index), true, true ));
      push_parameter(arg_lo, esp_offset);
      push_parameter(arg_hi, esp_offset+1);
    } else {
      // single-word non-oop argument
      Register arg = eax;
      __ movl(arg, frame_map()->address_for_local_name(frame_map()->name_for_argument(local_index), false));
      push_parameter(arg, esp_offset);
    }
    // Using these conditional expressions hits a solaris-X86 compiler bug
    // local_index += (sig->type_at(i)->is_two_word() ? 2 : 1);
    // esp_offset  += (sig->type_at(i)->is_two_word() ? 2 : 1);
    local_index += sig->type_at(i)->size();
    esp_offset  += sig->type_at(i)->size();
    assert( sig->type_at(i)->size() == 1 || sig->type_at(i)->size() == 2, "Invalid type input to size(), must return 1 or 2");
  }

  Register handle = eax;
  if (m->is_static()) {
    // the klass mirror follows the JNIEnv
    Register mirror = edx;
    __ movl(mirror, m->holder()->java_mirror()->encoding());
    __ movl(frame_map()->address_for_spill_index(0, false), mirror);
    __ leal(handle, frame_map()->address_for_spill_index(0, false)); // get a handle to the mirror
  } else {
    __ leal(handle, frame_map()->address_for_local_name(frame_map()->name_for_argument(0), false)); // get a handle to the receiver
  }
  push_parameter(handle, 1);                           // pass the handle

  // push JNIEnv (must be on TOS before the call)
  Register thread = ebx;
  Register t      = edx;
  __ get_thread(thread);
  __ leal(t, Address(thread, JavaThread::jni_environment_offset()));
  push_parameter(t, 0);

  // reset handle block
  __ movl(t, Address(thread, JavaThread::active_handles_offset()));
  __ movl(Address(t, JNIHandleBlock::top_offset_in_bytes()), 0);

  // set_last_Java_frame_before_call
  __ movl(Address(thread, JavaThread::last_Java_fp_offset()), ebp);
  __ movl(Address(thread, JavaThread::last_Java_sp_offset()), esp);
#ifdef ASSERT
  { Label L;
    __ cmpl(Address(thread,
		    JavaThread::frame_anchor_offset() + JavaFrameAnchor::flags_offset()),
		    0);
    __ jcc(Assembler::equal, L);
    __ stop("LIR_Assembler::emit_native_call: flags not cleared");
    __ bind(L);
  }
#endif /* ASSERT */

  // change thread state

  // Push return address on stack. We need to do this before we change thread-state, to
  // make sure that the safepoint code will always see a valid return address on the stack.
  
  // Change state to native (we save the return address in the thread, since it might not
  // be pushed on the stack when we do a a stack traversal). It is enough that the pc()
  // points into the right code segment. It does not have to be the correct return pc.

  __ leal(t, Address(pc_in_native, relocInfo::internal_word_type));
  __ movl(Address(thread,
		  JavaThread::frame_anchor_offset() + JavaFrameAnchor::last_Java_pc_offset()),
		  t);
  __ movl(Address(thread, JavaThread::thread_state_offset()), _thread_in_native); 
  
  // This outcommented code may be used for debugging purposes
  // __ empty_FPU_stack();

  __ call(native_entry, relocInfo::runtime_call_type);
  // result potentially in edx:eax or ST0

  // last_native_pc is used to build this frame, therefore set debug info at last_native_pc
  add_call_info(pc_in_native_offset, info); 
  add_call_info(__ offset(), info);
}

void LIR_Assembler::emit_native_method_exit(CodeEmitInfo* info) {
  ciMethod* m = compilation()->method();
  assert (m->is_native(), "Must be a native method");

  // change thread state
  Register thread = ebx;
  __ get_thread(thread);
  __ movl(Address(thread, JavaThread::thread_state_offset()), _thread_in_native_trans);
  // make sure the store is seen before reading the SafepointSynchronize state
  if (os::is_MP()) membar();

  // handle result
  const BasicType return_type = m->return_type()->basic_type();
  switch (return_type) {
    case T_ARRAY  : // fall through (result is handle)
    case T_OBJECT :
      { Label L;
        __ testl(eax, eax);
        __ jcc(Assembler::zero, L);
        __ movl(eax, Address(eax)); 
        __ bind(L);
        __ verify_oop(eax);
      }
      break;
    case T_BOOLEAN: __ c2bool(eax);            break;
    case T_CHAR   : __ andl(eax, 0xFFFF);      break;
    case T_BYTE   : __ sign_extend_byte (eax); break;
    case T_SHORT  : __ sign_extend_short(eax); break;
    case T_FLOAT  : // fall through (no rounding for floats)
    case T_DOUBLE : // fall through (no rounding for double)
    case T_VOID   : // fall through
    case T_LONG   : // fall through
    case T_INT    : /* do nothing */           break;
    default       : ShouldNotReachHere();
  }
  
  if (return_type == T_DOUBLE) {
    Address spill = frame_map()->address_for_spill_index(1, true);
    __ fstp_d(spill);
  } else if (return_type == T_FLOAT) {
    Address spill = frame_map()->address_for_spill_index(1, false);
    __ fstp_s(spill);
  }

  // check for safepoint operation in progress and/or pending suspend requests
  { Label Continue;

    __ cmpl(Address((int)SafepointSynchronize::address_of_state(), relocInfo::none), SafepointSynchronize::_not_synchronized);

    Label L;
    __ jcc(Assembler::notEqual, L);
    __ cmpl(Address(thread, JavaThread::suspend_flags_offset()), 0);
    __ jcc(Assembler::equal, Continue);
    __ bind(L);

    // Note: result object is upacked here
    bool result_is_oop = return_type == T_OBJECT || return_type == T_ARRAY;
    if (result_is_oop) {
      // preserve potential oop result
      __ movl(Address(thread, JavaThread::vm_result_offset()), eax);
    }

    // Do not change esp, as the oop map for last_native_pc would be wrong
    // eax/edx are preserved in the runtime stub

    // Nobody changes the last_Java_xx since the call to native
    __ call (Runtime1::entry_for(Runtime1::check_safepoint_and_suspend_for_native_trans_id), relocInfo::runtime_call_type);
    add_call_info(__ offset(), info);
    __ get_thread(thread);
    if (result_is_oop) {
      // restore preserved oop
      __ movl(eax, Address(thread, JavaThread::vm_result_offset()));
      __ movl(Address(thread, JavaThread::vm_result_offset()), (int)NULL);
    }
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
		  (int)NULL);
  // must clear fp, so that compiled frames are not confused; it is possible
  // that we need it only for debugging
  __ movl(Address(thread, JavaThread::last_Java_fp_offset()), (int)NULL);


  {
    Label no_reguard;
    __ cmpl(Address(thread, JavaThread::stack_guard_state_offset()), JavaThread::stack_guard_yellow_disabled);
    __ jcc(Assembler::notEqual, no_reguard);

    __ pushad();
    __ call_VM_leaf(CAST_FROM_FN_PTR(address, SharedRuntime::reguard_yellow_pages));
    __ popad();

    __ bind(no_reguard);
  }

  // JVMDI jframeIDs are invalidated on exit from native method.
  // JVMTI does not use jframeIDs, this whole mechanism must be removed when JVMDI is removed.
  if (JvmtiExport::must_purge_jvmdi_frames_on_native_exit()) { 
    __ pushad();
    __ call_VM_leaf(CAST_FROM_FN_PTR(address, JvmtiExport::thread_leaving_native_code));
    __ popad();
  }

  // handle exceptions (exception handling will handle unlocking!)
  { Label L;
    __ cmpl(Address(thread, Thread::pending_exception_offset()), (int)NULL);
    __ jcc(Assembler::zero, L);
    // make sure to unlock the object before exiting to exception handler
    __ movl(eax, Address(thread, Thread::pending_exception_offset()));
    // Note: pending exception is cleared in StubRoutines::forward_exception_entry
    // push address of native stub instruction so that the exception handler of
    // native is invoked in case it needs unlocking
    // note that pc  in native does not have correct oop map, but we cannot have GC at exit
    assert(StubRoutines::forward_exception_entry() != NULL, "must be created");
    __ call(StubRoutines::forward_exception_entry(), relocInfo::runtime_call_type);

    __ bind(L);
  }

  if (m->is_synchronized()) { 
    if (return_type != T_DOUBLE && return_type != T_FLOAT) {
      // must preserve results across monitor_exit
      __ movl(frame_map()->address_for_spill_index(1, false), eax);
      __ movl(frame_map()->address_for_spill_index(2, false), edx);
    }
    
    // do not use eax/edx
    RInfo obj_info  = FrameMap::_ecxRInfo;
    RInfo lock_info = FrameMap::_esiRInfo;
    monitorexit(obj_info, lock_info, SYNC_header, 0, noreg);

    if (return_type != T_DOUBLE && return_type != T_FLOAT) {
      // restore potential results (synchronization exit cannot block, but destroys register)
      __ movl(eax, frame_map()->address_for_spill_index(1, false));
      __ movl(edx, frame_map()->address_for_spill_index(2, false));
    }
  }

  if (return_type == T_DOUBLE) {
    Address spill = frame_map()->address_for_spill_index(1, true);
    __ fld_d(spill);
  } else if (return_type == T_FLOAT) {
    Address spill = frame_map()->address_for_spill_index(1, false);
    __ fld_s(spill);
  }

  if (return_type == T_OBJECT || return_type == T_ARRAY)
    return_op(FrameMap::_eaxRInfo, true);
  else
    return_op(norinfo, false);

  assert(__ esp_offset() == 0, "must remove stack");
}

// Optimized Library calls
// This is the fast version of java.lang.String.compare; it has not
// OSR-entry and therefore, we generate a slow version for OSR's
void LIR_Assembler::emit_string_compare(IRScope* scope) {
  assert(OptimizeLibraryCalls, "should not call this");
  // Load input parameters
  Address in0 = frame_map()->address_for_local_name(frame_map()->name_for_argument(0), false);
  Address in1 = frame_map()->address_for_local_name(frame_map()->name_for_argument(1), false);
  __ movl (ebx, ecx); // receiver is in ecx
  __ movl (eax, in1);
  
  // Get addresses of first characters from both Strings
  {
    CodeEmitInfo* info = new CodeEmitInfo(scope, 0, NULL);
    add_debug_info_for_null_check_here(info);
  }
  __ movl (esi, Address(eax, java_lang_String::value_offset_in_bytes()));
  __ movl (ecx, Address(eax, java_lang_String::offset_offset_in_bytes()));
  __ leal (esi, Address(esi, ecx, Address::times_2, arrayOopDesc::base_offset_in_bytes(T_CHAR)));


  // ebx may be NULL
  {
    CodeEmitInfo* info = new CodeEmitInfo(scope, 0, NULL);
    add_debug_info_for_null_check_here(info);
  }
  __ movl (edi, Address(ebx, java_lang_String::value_offset_in_bytes()));
  __ movl (ecx, Address(ebx, java_lang_String::offset_offset_in_bytes()));
  __ leal (edi, Address(edi, ecx, Address::times_2, arrayOopDesc::base_offset_in_bytes(T_CHAR)));  

  // compute minimum length (in eax) and difference of lengths (on top of stack)
  if (VM_Version::supports_cmov()) {
    __ movl (ebx, Address(ebx, java_lang_String::count_offset_in_bytes()));
    __ movl (eax, Address(eax, java_lang_String::count_offset_in_bytes()));
    __ movl (ecx, ebx);
    __ subl (ebx, eax); // subtract lengths
    __ pushl(ebx);      // result
    __ cmovl(Assembler::lessEqual, eax, ecx);
  } else {
    Label L;
    __ movl (ebx, Address(ebx, java_lang_String::count_offset_in_bytes()));
    __ movl (ecx, Address(eax, java_lang_String::count_offset_in_bytes()));
    __ movl (eax, ebx);
    __ subl (ebx, ecx);
    __ pushl(ebx);
    __ jcc  (Assembler::lessEqual, L);
    __ movl (eax, ecx);
    __ bind (L);
  }
  // is minimum length 0?
  Label noLoop, haveResult;
  __ testl (eax, eax);
  __ jcc (Assembler::zero, noLoop);

  // compare first characters
  __ load_unsigned_word(ecx, Address(edi));
  __ load_unsigned_word(ebx, Address(esi));
  __ subl(ecx, ebx);
  __ jcc(Assembler::notZero, haveResult);
  // starting loop
  __ decl(eax); // we already tested index: skip one  
  __ jcc(Assembler::zero, noLoop);

  // set esi.edi to the end of the arrays (arrays have same length)
  // negate the index

  __ leal(esi, Address(esi, eax, Address::times_2, type2aelembytes[T_CHAR]));
  __ leal(edi, Address(edi, eax, Address::times_2, type2aelembytes[T_CHAR]));
  __ negl(eax);

  // compare the strings in a loop

  Label loop;
  __ align(wordSize);
  __ bind(loop);
  __ load_unsigned_word(ecx, Address(edi, eax, Address::times_2, 0));
  __ load_unsigned_word(ebx, Address(esi, eax, Address::times_2, 0));
  __ subl(ecx, ebx);
  __ jcc(Assembler::notZero, haveResult);
  __ incl(eax);
  __ jcc(Assembler::notZero, loop);

  // strings are equal up to min length

  __ bind(noLoop);
  __ popl(eax);
  return_op(norinfo, false);

  __ bind(haveResult);
  // leave instruction is going to discard the TOS value
  __ movl (eax, ecx); // result of call is in eax
  return_op(norinfo, false);
}


void LIR_Assembler::emit_triglib(ciMethod::IntrinsicId trig_id) {
  assert(OptimizeLibraryCalls, "should not call this");
  // get argument
  __ fld_d(frame_map()->address_for_local_name(frame_map()->name_for_argument(0), true));

  bool is_sin = (trig_id == methodOopDesc::_dsin);

  switch (trig_id) {
  case methodOopDesc::_dsin:
  case methodOopDesc::_dcos: 
    // Should consider not saving ebx if not necessary
    __ sincos(is_sin, true, fpu_stack_size());
    break;

  default                  : Unimplemented();
  }
  // result in FPU ST(0)
  return_op(norinfo, false);
}


void LIR_Assembler::return_op(RInfo result, bool result_is_oop) {
  // Note: we do not need to round double result; float result has the right precision  
  if (compilation()->jvmpi_event_method_exit_enabled()) {
    jvmpi_method_exit(compilation()->method(), result_is_oop);
  }

  // Pop the stack before the safepoint code
  __ leave();

  // the poll sets the condition code, but no data registers
  if (SafepointPolling) {
    Address polling_page((int)os::get_polling_page() + (SafepointPollOffset % os::vm_page_size()),
                         relocInfo::none);

    int testl_offset = __ offset();
    __ relocate(relocInfo::poll_return_type);
    __ testl(eax, polling_page);

    int frame_size = frame_map()->framesize();
    int arg_count  = frame_map()->oop_map_arg_count();
    OopMap* map    = new OopMap(frame_size, arg_count);
    if (result_is_oop)
      map->set_oop(FrameMap::cpu_regname(result), frame_size, arg_count);
    compilation()->debug_info_recorder()->add_oopmap(testl_offset, false, map);
  }

  __ method_exit(false);  // pop frame at exit
  assert(result.is_illegal() || !result.is_word() || result.is_same(FrameMap::_eaxRInfo), "word returns are in eax");
  if (!result.is_illegal() && result.is_float_kind()) {
    fpu_pop(result);
  }
}


void LIR_Assembler::safepoint_poll(RInfo tmp, CodeEmitInfo* info) {
  Address polling_page((intptr_t)os::get_polling_page() + (SafepointPollOffset % os::vm_page_size()),
                       relocInfo::none);

  if (info != NULL) {
    add_debug_info_for_branch(info);
  } else {
    __ relocate(relocInfo::poll_type);
  }

  __ testl(eax, polling_page);
}


void LIR_Assembler::const2reg(LIR_Const* c, RInfo to_reg, LIR_Op1::LIR_PatchCode patch_code, CodeEmitInfo* info) {
  if (c->type() == T_INT) {
    assert(patch_code == LIR_Op1::patch_none, "no patching handled here");
    __ movl(to_reg.as_register(), c->as_jint());
  } else if (c->type() == T_OBJECT) {
    PatchingStub* patch = NULL;
    if (patch_code != LIR_Op1::patch_none) {
      patch = new PatchingStub(_masm, PatchingStub::load_klass_id);
    }
    __ movl(to_reg.as_register(), c->as_jobject());
    if (patch_code != LIR_Op1::patch_none) {
      patching_epilog(patch, patch_code, noreg, info);
    }
  } else if (c->type() == T_FLOAT || c->type() == T_DOUBLE) {
    bool is32 = c->type() == T_FLOAT;
    if ((is32 && c->is_zero_float()) || (!is32 && c->is_zero_double())) {
      __ fldz();
    } else if ((is32 && c->is_one_float()) || (!is32 && c->is_one_double())) {
      __ fld1();
    } else {
      address const_addr = is32 ? _const_table.address_of_float_constant(c->as_jfloat()) 
                                : _const_table.address_of_double_constant(c->as_jdouble());
      assert(const_addr != NULL, "incorrect float/double constant maintainance");
      if (is32) {
        __ fld_s (Address((int)const_addr, relocInfo::internal_word_type));
      } else {
        __ fld_d (Address((int)const_addr, relocInfo::internal_word_type));
      }
    }
    fpu_push(to_reg);
  } else {
    Unimplemented();
  }
}

void LIR_Assembler::const2stack(LIR_Const* c, int stack_index) {
  if (c->type() == T_INT) {
    __ movl(frame_map()->address_for_name(stack_index, false), c->as_jint());
  } else if (c->type() == T_OBJECT) {
    __ movl(frame_map()->address_for_name(stack_index, false), c->as_jobject());
  } else {
    __ movl(frame_map()->address_for_name(stack_index, true, false), c->as_jint_lo());
    __ movl(frame_map()->address_for_name(stack_index, true, true ), c->as_jint_hi());
  }
}

void LIR_Assembler::const2mem(LIR_Const* c, LIR_Address* addr, BasicType type, CodeEmitInfo* info ) {
  if (info != NULL) add_debug_info_for_null_check_here(info);
  switch (type) {
  case T_DOUBLE:  // fall through
  case T_LONG:    
    __ movl(as_Address_hi(addr), c->as_jint_hi()); 
    __ movl(as_Address_lo(addr), c->as_jint_lo()); 
    break;
  case T_OBJECT:  // fall through
  case T_ARRAY:   __ movl(as_Address(addr), c->as_jobject()); break;
  case T_FLOAT:   // fall through
  case T_INT:     __ movl(as_Address(addr), c->as_jint()); break;
  case T_BOOLEAN: // fall through
  case T_BYTE:    __ movb(as_Address(addr), c->as_jint() & 0xFF); break;
  case T_CHAR:    // fall through
  case T_SHORT:   __ movw(as_Address(addr), c->as_jint() & 0xFFFF); break;
  default: ShouldNotReachHere();
  };
}

void LIR_Assembler::stack2reg(LIR_Opr src, LIR_Opr dest, BasicType type) {
  if (dest->is_double_fpu() || dest->is_single_fpu()) {
    int stack_index = (dest->is_single_fpu() ? src->single_stack_ix() : src->double_stack_ix());
    Address src_addr = frame_map()->address_for_name(stack_index, src->is_double_stack() ); 
    if (dest->is_single_fpu()) __ fld_s(src_addr);
    else                       __ fld_d(src_addr);
    fpu_push(dest->rinfo());
  } else if (dest->is_double_cpu()) {
    Address src_addr_LO = frame_map()->address_for_name(src->double_stack_ix(), true, false);
    Address src_addr_HI = frame_map()->address_for_name(src->double_stack_ix(), true, true);
    __ movl(dest->rinfo().as_register_hi(), src_addr_HI);
    __ movl(dest->rinfo().as_register_lo(), src_addr_LO);
  } else if (dest->is_single_cpu()) {
    __ movl(dest->rinfo().as_register(), frame_map()->address_for_name(src->single_stack_ix(), false));
    if (type == T_ARRAY || type == T_OBJECT) __ verify_oop(dest->as_register());
  } else {
    ShouldNotReachHere();
  }
}


void LIR_Assembler::mem2reg(LIR_Address* addr, RInfo to_rinfo, BasicType type, LIR_Op1::LIR_PatchCode patch_code, CodeEmitInfo* info) {
  Address from_addr = as_Address(addr);
  
  switch (type) {
  case T_BOOLEAN:
  case T_BYTE:
  case T_CHAR:
  case T_SHORT:
    if (!VM_Version::is_P6() && !from_addr.uses(to_rinfo.as_register())) {
      // on pre P6 processors we may get partial register stalls
      // so blow away the value of to_rinfo before loading a
      // partial word into it.  Do it here so that it precedes
      // the potential patch point below.
      __ xorl(to_rinfo.as_register(), to_rinfo.as_register());
      break;
    }
  }

  PatchingStub* patch = NULL;
  if (patch_code != LIR_Op1::patch_none) {
    patch = new PatchingStub(_masm, PatchingStub::access_field_id);
  }
  if (info != NULL) {
    add_debug_info_for_null_check_here(info);
  }

  switch (type) {
  case T_FLOAT:  __ fld_s(from_addr); fpu_push(to_rinfo); break;
  case T_DOUBLE: __ fld_d(from_addr); fpu_push(to_rinfo); break;
  case T_OBJECT: // fall through
  case T_ARRAY:  // fall through
  case T_INT:    __ movl(to_rinfo.as_register(), from_addr); break;
  case T_LONG:   
    __ movl(to_rinfo.as_register_hi(), as_Address_hi(addr));
    __ movl(to_rinfo.as_register_lo(), as_Address_lo(addr));
    break;
  case T_BOOLEAN: // fall through
  case T_BYTE: {
    Register dest = to_rinfo.as_register();
    assert(VM_Version::is_P6() || dest->has_byte_register(), "must use byte registers if not P6");
    if (VM_Version::is_P6() || from_addr.uses(dest)) {
      __ movsxb(dest, from_addr);
    } else {
      __ movb(dest, from_addr);
      __ shll(dest, 24);
      __ sarl(dest, 24);
    }
    break;
  }
  case T_CHAR: {
    Register dest = to_rinfo.as_register();
    assert(VM_Version::is_P6() || dest->has_byte_register(), "must use byte registers if not P6");
    if (VM_Version::is_P6() || from_addr.uses(dest)) {
      __ movzxw(dest, from_addr);
    } else {
      __ movw(dest, from_addr);
    }
    break;
  }
  case T_SHORT: {
    Register dest = to_rinfo.as_register();
    if (VM_Version::is_P6() || from_addr.uses(dest)) {
      __ movsxw(dest, from_addr);
    } else {
      __ movw(dest, from_addr);
      __ shll(dest, 16);
      __ sarl(dest, 16);
    }
    break;
  }
  
  default: ShouldNotReachHere();
  }

  if (patch != NULL) {
    patching_epilog(patch, patch_code, addr->base()->as_register(), info);
  }

  if (type == T_ARRAY || type == T_OBJECT) __ verify_oop(to_rinfo.as_register());
}


void LIR_Assembler::const2array(LIR_Const* c, LIR_Address* addr, BasicType type, CodeEmitInfo* info) {
  Address to_addr = as_ArrayAddress(addr, type);
  switch (type) {
  case T_DOUBLE:
  case T_LONG:  
   // Fall through LONGS have been split into 2 INTS
  case T_FLOAT:
  case T_INT:
    __ movl (to_addr, c->as_jint());
    break;

  case T_ARRAY:
  case T_OBJECT:
    __ movl (to_addr, c->as_jobject());
    break;

  case T_BYTE:
    __ movb (to_addr, c->as_jint() & 0xFF);
    break;
  default: 
    ShouldNotReachHere();
  }
}


void LIR_Assembler::reg2array(RInfo from_rinfo, LIR_Address* addr, BasicType type, CodeEmitInfo* info) {
  if (type == T_ARRAY || type == T_OBJECT) __ verify_oop(from_rinfo.as_register());

  Address to_addr = as_ArrayAddress(addr, type);
  switch (type) {
  case T_LONG:
   // Fall through LONGS have been split into 2 INTS
  case T_INT:
  case T_ARRAY:
  case T_OBJECT:
    __ movl(to_addr, from_rinfo.as_register());
    break;

  case T_BYTE:
    __ movb(to_addr, from_rinfo.as_register());
    break;

  case T_CHAR:
    __ movw(to_addr, from_rinfo.as_register());
    break;

  case T_SHORT:
    NEEDS_CLEANUP // do we need the sign extension here???
    __ sign_extend_short(from_rinfo.as_register());
    __ movw(to_addr, from_rinfo.as_register());
    break;

  case T_FLOAT:
  case T_DOUBLE:
    fpu_on_tos(from_rinfo);
    if (type == T_FLOAT) __ fstp_s(to_addr);
    else                 __ fstp_d(to_addr);
    fpu_pop(from_rinfo);
    break;

  default: ShouldNotReachHere();
  }
}


void LIR_Assembler::array2reg(LIR_Address* addr, RInfo to_rinfo, BasicType type, CodeEmitInfo* info) {
  int offset = code_offset();
  Address from_addr = as_ArrayAddress(addr, type);
  switch (type) {
  case T_LONG:
   // Fall through LONGS have been split into 2 INTS
  case T_INT:
  case T_ARRAY:
  case T_OBJECT:
    __ movl(to_rinfo.as_register(), from_addr);
    break;

  case T_BYTE:
    {
      Register dest = to_rinfo.as_register();
      assert(VM_Version::is_P6() || dest->has_byte_register(), "must use byte registers if not P6");
      offset = __ load_signed_byte(dest, from_addr);
    }
    break;

  case T_CHAR:
    offset = __ load_unsigned_word(to_rinfo.as_register(), from_addr);
    break;

  case T_SHORT:
    offset = __ load_signed_word(to_rinfo.as_register(), from_addr);
    break;

  case T_FLOAT:
    __ fld_s(from_addr);
    fpu_push(to_rinfo);
    break;

  case T_DOUBLE:
    __ fld_d(from_addr);
    fpu_push(to_rinfo);
    break;

  default: ShouldNotReachHere();
  }

  if (info != NULL) add_debug_info_for_null_check(offset, info);

  if (type == T_ARRAY || type == T_OBJECT) __ verify_oop(to_rinfo.as_register());
}


void LIR_Assembler::reg2stack(RInfo from_reg, int to_stack_index, BasicType type) {
  if (from_reg.is_float_kind()) {
    fpu_on_tos(from_reg);

    Address dst_addr = frame_map()->address_for_name(to_stack_index, from_reg.is_double());
    if (from_reg.is_float()) __ fstp_s (dst_addr);
    else                     __ fstp_d (dst_addr);
    fpu_pop(from_reg);
  } else {
    if (from_reg.is_long()) {
      Address dstLO = frame_map()->address_for_name(to_stack_index, true, false);
      Address dstHI = frame_map()->address_for_name(to_stack_index, true, true);
      __ movl (dstLO, from_reg.as_register_lo());
      __ movl (dstHI, from_reg.as_register_hi());
    } else {
      Address dst = frame_map()->address_for_name(to_stack_index, false);
      if (type == T_ARRAY || type == T_OBJECT) __ verify_oop(from_reg.as_register());
      __ movl (dst, from_reg.as_register());
    }
  }
}


void LIR_Assembler::move_regs(Register from_reg, Register to_reg) {
  if (from_reg != to_reg) __ movl(to_reg, from_reg);
}


void LIR_Assembler::reg2reg(RInfo from_reg, RInfo to_reg) {
  if (to_reg.is_float_kind() && from_reg.is_float_kind()) {
    fpu_on_tos(from_reg);
    fpu_pop(from_reg);
    fpu_push(to_reg);
  } else if (!to_reg.is_float_kind() && !from_reg.is_float_kind()) {
    if (to_reg.is_word()) {
      assert(from_reg.is_word(), "must match");
      move_regs(from_reg.as_register(), to_reg.as_register());
    } else if (to_reg.is_long()) {
      assert(from_reg.is_long() && !from_reg.overlaps(to_reg), "must match and not overlap");
      move_regs(from_reg.as_register_lo(), to_reg.as_register_lo());
      move_regs(from_reg.as_register_hi(), to_reg.as_register_hi());
    }
  } else {
    // float to int or int to float moves
    if (to_reg.is_long()) {
      Address temp_slot1 = frame_map()->address_for_scratch_index(0, true, false);
      Address temp_slot2 = frame_map()->address_for_scratch_index(0, true, true);
      fpu_on_tos(from_reg);
      fpu_pop(from_reg);
      __ fstp_d(temp_slot1);
      __ movl(to_reg.as_register_lo(), temp_slot1);
      __ movl(to_reg.as_register_hi(), temp_slot2);
    } else if (to_reg.is_word()) {
      Address temp_slot1 = frame_map()->address_for_scratch_index(0, false, false);
      fpu_on_tos(from_reg);
      fpu_pop(from_reg);
      __ fstp_s(temp_slot1);
      __ movl(to_reg.as_register(), temp_slot1);
    } else if (to_reg.is_double()) {
      Address temp_slot1 = frame_map()->address_for_scratch_index(0, true, false);
      Address temp_slot2 = frame_map()->address_for_scratch_index(0, true, true);
      __ movl(temp_slot2, from_reg.as_register_hi());
      __ movl(temp_slot1, from_reg.as_register_lo());
      __ fld_d(temp_slot1);
      fpu_push(to_reg);
    } else if (to_reg.is_float()) {
      Address temp_slot1 = frame_map()->address_for_scratch_index(0, false, false);
      __ movl(temp_slot1,  from_reg.as_register());
      __ fld_s(temp_slot1);
      fpu_push(to_reg);
    }
  }
}

void LIR_Assembler::swap_reg(Register a, Register b) {
  __ xchgl(a, b);
}

void LIR_Assembler::reg2mem(RInfo from_reg, LIR_Address* to_addr, BasicType type, LIR_Op1::LIR_PatchCode patch_code, CodeEmitInfo* info) {
  PatchingStub* patch = NULL;

  if (type == T_ARRAY || type == T_OBJECT) __ verify_oop(from_reg.as_register());

  if (patch_code != LIR_Op1::patch_none) {
    patch = new PatchingStub(_masm, PatchingStub::access_field_id);
  }

  if (info != NULL) {
    add_debug_info_for_null_check_here(info);
  }

  switch (type) {
  case T_FLOAT:   // fall through
  case T_DOUBLE:
    { fpu_on_tos(from_reg);
      if (from_reg.is_float())  __ fstp_s(as_Address(to_addr));
      else                      __ fstp_d(as_Address(to_addr));
      fpu_pop(from_reg);
    }
    break;
   
  case T_ADDRESS: // fall through
  case T_ARRAY:   // fall through
  case T_OBJECT:  // fall through
  case T_INT:     __ movl(as_Address(to_addr), from_reg.as_register()); break;
  case T_LONG:
    __ movl(as_Address_hi(to_addr), from_reg.as_register_hi()); 
    __ movl(as_Address_lo(to_addr), from_reg.as_register_lo()); 
    break;
  
  case T_BYTE:    // fall through
  case T_BOOLEAN: 
    { Register src_reg = from_reg.as_register();
      Address dst_addr = as_Address(to_addr);
      assert(VM_Version::is_P6() || src_reg->has_byte_register(), "must use byte registers if not P6");
      __ movb (dst_addr, src_reg);
    }
    break;

  case T_CHAR:    // fall through
  case T_SHORT:   __ movw(as_Address(to_addr), from_reg.as_register()); break;
   
  default: ShouldNotReachHere(); 
  }

  if (patch_code != LIR_Op1::patch_none) {
    patching_epilog(patch, patch_code, to_addr->base()->as_register(), info);
  }
}


NEEDS_CLEANUP; // This could be static? 
Address::ScaleFactor LIR_Assembler::array_element_size(BasicType type) const {
  int elem_size = type2aelembytes[type];
  switch (elem_size) {
    case 1: return Address::times_1;
    case 2: return Address::times_2;
    case 4: return Address::times_4;
    case 8: return Address::times_8;
  }
  ShouldNotReachHere();
  return Address::no_scale;
}


void LIR_Assembler::emit_op3(LIR_Op3* op) {
  switch (op->code()) {
    case lir_idiv:
    case lir_irem:
      arithmetic_idiv(
        op->code(),
        op->in_opr1(),
        op->in_opr2(),
        op->in_opr3(),
        op->result_opr(),
        op->info());
      break;
    default:      ShouldNotReachHere(); break;
  }
}

void LIR_Assembler::emit_opBranch(LIR_OpBranch* op) {
  if (op->cond_reg()->is_illegal()) {
    if (op->cond() == LIR_OpBranch::always) {
      if (op->info() != NULL) add_debug_info_for_branch(op->info());
      __ jmp (*(op->label()));
    } else if (op->cond() == LIR_OpBranch::intrinsicFailed) {
      __ jC2(noreg, *(op->label()));
    } else {
      Assembler::Condition acond = Assembler::zero;
      if (op->code() == lir_cond_float_branch) {
        assert(op->ulabel() != NULL, "must have unordered successor");
        __ jcc(Assembler::parity, *(op->ulabel()));
        switch(op->cond()) {
          case LIR_OpBranch::equal:        acond = Assembler::equal;      break;
          case LIR_OpBranch::notEqual:     acond = Assembler::notEqual;   break;
          case LIR_OpBranch::less:         acond = Assembler::below;      break;
          case LIR_OpBranch::lessEqual:    acond = Assembler::belowEqual; break;
          case LIR_OpBranch::greaterEqual: acond = Assembler::aboveEqual; break;
          case LIR_OpBranch::greater:      acond = Assembler::above;      break;
          default:                         ShouldNotReachHere();
        }
      } else {
        switch (op->cond()) {
          case LIR_OpBranch::equal:        acond = Assembler::equal;       break;
          case LIR_OpBranch::notEqual:     acond = Assembler::notEqual;    break;
          case LIR_OpBranch::less:         acond = Assembler::less;        break;
          case LIR_OpBranch::lessEqual:    acond = Assembler::lessEqual;   break;
          case LIR_OpBranch::greaterEqual: acond = Assembler::greaterEqual;break;
          case LIR_OpBranch::greater:      acond = Assembler::greater;     break;
          case LIR_OpBranch::belowEqual:   acond = Assembler::belowEqual;  break;
          case LIR_OpBranch::aboveEqual:   acond = Assembler::aboveEqual;  break;
          default:                         ShouldNotReachHere();
        }
      }
      
      if (op->info() != NULL) ShouldNotReachHere();
      __ jcc(acond,*(op->label()));
    }
  } else {
    Unimplemented();
  }
}

void LIR_Assembler::emit_opConvert(LIR_OpConvert* op) {
  LIR_Opr value        = op->in_opr();
  RInfo dst            = op->result_opr()->rinfo();
  Bytecodes::Code code = op->bytecode();


  if (op->in_opr()->is_single_fpu() || op->in_opr()->is_double_fpu()) {
    fpu_on_tos(value->rinfo());
    fpu_pop(value->rinfo());
  }

  if (op->result_opr()->is_single_fpu() || op->result_opr()->is_double_fpu() ) {
    fpu_push(dst);
  }

  switch (code) {
    case Bytecodes::_i2l: 
      move_regs(value->rinfo().as_register(), dst.as_register_lo());
      move_regs(value->rinfo().as_register(), dst.as_register_hi());
      __ sarl (dst.as_register_hi(), 31);
      break;
    case Bytecodes::_i2d:
    case Bytecodes::_i2f:
      if (value->is_single_cpu()) {
        __ push_reg  (value->rinfo().as_register());
        __ fild_s (Address(esp));
        __ dec_stack(1);
      } else if (value->is_stack()) {
        Address src = value->is_single_stack()
                         ? frame_map()->address_for_name(value->single_stack_ix(), false)
                         : frame_map()->address_for_name(value->double_stack_ix(), true); 
        __ fild_s (src);
      } else {
        Unimplemented();
      }
      break;
    case Bytecodes::_l2f:
      if (value->is_double_cpu()) {
        __ movl(Address(esp, BytesPerWord), value->rinfo().as_register_hi());
        __ movl(Address(esp),               value->rinfo().as_register_lo());
        __ fild_d (Address(esp));
        __ fstp_s(Address(esp));
        __ fld_s(Address(esp));
      } else if (value->is_double_stack()) {
        Address src = frame_map()->address_for_name(value->double_stack_ix(), true);
        __ fild_d (src);
        __ fstp_s(Address(esp));
        __ fld_s(Address(esp));
      } else {
        Unimplemented();
      }
      break;
    case Bytecodes::_l2d:
      if (value->is_double_cpu()) {
        __ movl(Address(esp, BytesPerWord), value->rinfo().as_register_hi());
        __ movl(Address(esp),               value->rinfo().as_register_lo());
        __ fild_d (Address(esp));
        __ fstp_d(Address(esp));
        __ fld_d(Address(esp));
      } else if (value->is_double_stack()) {
        Address src = frame_map()->address_for_name(value->double_stack_ix(), true);
        __ fild_d (src);
        __ fstp_d(Address(esp));
        __ fld_d(Address(esp));
      } else {
        Unimplemented();
      }
      break;
    case Bytecodes::_l2i:
      move_regs (value->rinfo().as_register_lo(), dst.as_register());
      break;
    case Bytecodes::_d2f:
    case Bytecodes::_f2d:
      // do nothing (must round later)
      // d2f is rounded through spilling
      assert (value->is_single_fpu() || value->is_double_fpu(), "wrong value state");
      break;
    case Bytecodes::_i2b:
      move_regs (value->rinfo().as_register(), dst.as_register());
      __ sign_extend_byte(dst.as_register());
      break;
    case Bytecodes::_i2c:
      move_regs (value->rinfo().as_register(), dst.as_register());
      __ andl (dst.as_register(), 0xFFFF);
      break;
    case Bytecodes::_i2s:
      move_regs (value->rinfo().as_register(), dst.as_register());
      __ sign_extend_short(dst.as_register());
      break;
    case Bytecodes::_f2l: 
    case Bytecodes::_d2l: 
      __ call(Runtime1::entry_for(Runtime1::d2l_stub_id), relocInfo::runtime_call_type);
      if (eax == dst.as_register_hi() && edx == dst.as_register_lo()) {
        __ xchgl(eax, edx);
      } else if (edx != dst.as_register_lo()) {
        move_regs(eax, dst.as_register_lo());
        move_regs(edx, dst.as_register_hi());
      } else {
        move_regs(edx, dst.as_register_hi());
        move_regs(eax, dst.as_register_lo());
      }
      break;

    case Bytecodes::_f2i:
    case Bytecodes::_d2i: {
      address entry = NULL;
      if (op->is_32bit()) { 
        entry = CAST_FROM_FN_PTR(address, Runtime1::entry_for(Runtime1::f2i_is32bit_stub_id));
      } else {
        entry = CAST_FROM_FN_PTR(address, Runtime1::entry_for(Runtime1::f2i_not32bit_stub_id));
      }
      
      Label done, pop;
      __ pushl(eax); // make room on stack
      __ fldcw(Address((int)StubRoutines::addr_fpu_cntrl_wrd_trunc(), relocInfo::none));
      __ fist_s(Address(esp));
      if (op->is_32bit()) {
        set_24bit_FPU();
      } else {
        reset_FPU();
      }
      __ popl(eax);
      __ cmpl(eax, 0x80000000);
      __ jcc(Assembler::notEqual, pop);
      __ call(entry, relocInfo::runtime_call_type);
      __ jmp(done);
      __ bind(pop);
      __ fpop();
      __ bind(done);
      break;
    }
    
    default: ShouldNotReachHere();
  }
}

void LIR_Assembler::emit_alloc_obj(LIR_OpAllocObj* op) {
  __ allocate_object(
    op->obj()->rinfo().as_register(), 
    op->tmp1()->rinfo().as_register(),
    op->tmp2()->rinfo().as_register(),
    op->header_size(),
    op->object_size(),
    op->klass()->rinfo().as_register(),
    *op->stub()->entry());
  __ bind(*op->stub()->continuation());
}

void LIR_Assembler::emit_alloc_array(LIR_OpAllocArray* op) {
  __ allocate_array(
      op->obj()->rinfo().as_register(), 
      op->len()->rinfo().as_register(), 
      op->tmp1()->rinfo().as_register(), 
      op->tmp2()->rinfo().as_register(), 
      arrayOopDesc::header_size(op->type()),
      array_element_size(op->type()),
      op->klass()->rinfo().as_register(),
      *op->stub()->entry());
  __ bind(*op->stub()->continuation());
}



void LIR_Assembler::emit_opTypeCheck(LIR_OpTypeCheck* op) {
  LIR_Code code = op->code();
  if (code == lir_store_check) {
    Register value = op->object()->as_register();
    Register array = op->array()->as_register();
    Register k_RInfo = op->tmp1()->as_register();
    Register klass_RInfo = op->tmp2()->as_register();
    Register Rtmp1 = op->tmp3()->as_register();
    
    CodeStub* stub = op->stub();
    Label done;
    __ cmpl(value, 0);
    __ jcc(Assembler::equal, done);
    add_debug_info_for_null_check_here(op->info_for_exception());
    __ movl(k_RInfo, Address(array, oopDesc::klass_offset_in_bytes()));
    __ movl(klass_RInfo, Address(value, oopDesc::klass_offset_in_bytes()));
    
    // get instance klass
    __ movl(k_RInfo, Address(k_RInfo, objArrayKlass::element_klass_offset_in_bytes() + sizeof(oopDesc)));
    // get super_check_offset
    __ movl(Rtmp1, Address(k_RInfo, sizeof(oopDesc) + Klass::super_check_offset_offset_in_bytes()));
    // See if we get an immediate positive hit
    __ cmpl(k_RInfo, Address(klass_RInfo, Rtmp1, Address::times_1));
    __ jcc(Assembler::equal, done);
    // check for immediate negative hit
    __ cmpl(Rtmp1, sizeof(oopDesc) + Klass::secondary_super_cache_offset_in_bytes());
    __ jcc(Assembler::notEqual, *stub->entry());
    // check for self
    __ cmpl(klass_RInfo, k_RInfo);
    __ jcc(Assembler::equal, done);

    __ pushl(klass_RInfo);
    __ pushl(k_RInfo);
    __ call(Runtime1::entry_for(Runtime1::slow_subtype_check_id), relocInfo::runtime_call_type);
    __ popl(klass_RInfo);
    __ popl(k_RInfo);
    __ cmpl(k_RInfo, 0);
    __ jcc(Assembler::equal, *stub->entry());
    __ bind(done);
  } else if (op->code() == lir_checkcast) {
    // we always need a stub for the failure case.
    CodeStub* stub = op->stub();
    Register obj = op->object()->as_register();
    Register k_RInfo = op->tmp1()->as_register();
    Register klass_RInfo = op->tmp2()->as_register();
    Register dst = op->result_opr()->as_register();
    ciKlass* k = op->klass();
    
    Label done;
    // patching may screw with our temporaries on sparc,
    // so let's do it before loading the class
    if (!k->is_loaded()) {
      jobject2reg_with_patching(k_RInfo, op->info_for_patch());
    } else {
      k_RInfo = noreg;
    }
    assert(obj != k_RInfo, "must be different");
    __ cmpl(obj, 0);
    __ jcc(Assembler::equal, done);

    if (op->fast_check()) {
      // get object class
      // not a safepoint as obj null check happens earlier
      if (k->is_loaded()) {
        __ cmpl(Address(obj, oopDesc::klass_offset_in_bytes()), k->encoding());
      } else {
        __ cmpl(k_RInfo, Address(obj, oopDesc::klass_offset_in_bytes()));
        
      }
      __ jcc(Assembler::notEqual, *stub->entry());
      __ bind(done);
    } else {
      // get object class
      // not a safepoint as obj null check happens earlier
      __ movl(klass_RInfo, Address(obj, oopDesc::klass_offset_in_bytes()));
      if (k->is_loaded()) {
        // See if we get an immediate positive hit
        __ cmpl(Address(klass_RInfo, k->super_check_offset()), k->encoding());
        if (sizeof(oopDesc) + Klass::secondary_super_cache_offset_in_bytes() != k->super_check_offset()) {
          __ jcc(Assembler::notEqual, *stub->entry());
        } else {
          // See if we get an immediate positive hit
          __ jcc(Assembler::equal, done);
          // check for self
          __ cmpl(klass_RInfo, k->encoding());
          __ jcc(Assembler::equal, done);
          
          __ pushl(klass_RInfo);
          __ pushl(k->encoding());
          __ call(Runtime1::entry_for(Runtime1::slow_subtype_check_id), relocInfo::runtime_call_type);
          __ popl(klass_RInfo);
          __ popl(klass_RInfo);
          __ cmpl(klass_RInfo, 0);
          __ jcc(Assembler::equal, *stub->entry());
        }
        __ bind(done);
      } else {
        assert(dst != obj, "need different registers so we have a temporary");
        assert(dst != klass_RInfo && dst != k_RInfo, "need 3 registers");
        
        __ movl(dst, Address(k_RInfo, sizeof(oopDesc) + Klass::super_check_offset_offset_in_bytes()));
        // See if we get an immediate positive hit
        __ cmpl(k_RInfo, Address(klass_RInfo, dst, Address::times_1));
        __ jcc(Assembler::equal, done);
        // check for immediate negative hit
        __ cmpl(dst, sizeof(oopDesc) + Klass::secondary_super_cache_offset_in_bytes());
        __ jcc(Assembler::notEqual, *stub->entry());
        // check for self
        __ cmpl(klass_RInfo, k_RInfo);
        __ jcc(Assembler::equal, done);
        
        __ pushl(klass_RInfo);
        __ pushl(k_RInfo);
        __ call(Runtime1::entry_for(Runtime1::slow_subtype_check_id), relocInfo::runtime_call_type);
        __ popl(klass_RInfo);
        __ popl(k_RInfo);
        __ cmpl(k_RInfo, 0);
        __ jcc(Assembler::equal, *stub->entry());
        __ bind(done);
      }
      
    }
    __ movl(dst, obj);
  } else if (code == lir_instanceof) {
    Register obj = op->object()->as_register();
    Register k_RInfo = op->tmp1()->as_register();
    Register klass_RInfo = op->tmp2()->as_register();
    Register dst = op->result_opr()->as_register();
    ciKlass* k = op->klass();
    
    Label done;
    Label zero;
    Label one;
    // patching may screw with our temporaries on sparc,
    // so let's do it before loading the class
    if (!k->is_loaded()) {
      jobject2reg_with_patching(k_RInfo, op->info_for_patch());
    }
    assert(obj != k_RInfo, "must be different");

    if (op->fast_check()) {
      __ cmpl(obj, 0);
      __ jcc(Assembler::equal, zero);
      // get object class
      // not a safepoint as obj null check happens earlier
      if (k->is_loaded()) {
        __ cmpl(Address(obj, oopDesc::klass_offset_in_bytes()), k->encoding());
        k_RInfo = noreg;
      } else {
        __ cmpl(k_RInfo, Address(obj, oopDesc::klass_offset_in_bytes()));
        
      }
      __ jcc(Assembler::equal, one);
    } else {
      // get object class
      // not a safepoint as obj null check happens earlier
      __ cmpl(obj, 0);
      __ jcc(Assembler::equal, zero);
      __ movl(klass_RInfo, Address(obj, oopDesc::klass_offset_in_bytes()));
      if (k->is_loaded()) {
        assert(dst != obj, "need different registers so we have a temporary");
        
        // See if we get an immediate positive hit
        __ cmpl(Address(klass_RInfo, k->super_check_offset()), k->encoding());
        __ jcc(Assembler::equal, one);
        if (sizeof(oopDesc) + Klass::secondary_super_cache_offset_in_bytes() == k->super_check_offset()) {
          // check for self
          __ cmpl(klass_RInfo, k->encoding());
          __ jcc(Assembler::equal, one);
          __ pushl(klass_RInfo);
          __ pushl(k->encoding());
          __ call(Runtime1::entry_for(Runtime1::slow_subtype_check_id), relocInfo::runtime_call_type);
          __ popl(klass_RInfo);
          __ popl(dst);
          __ jmp(done);
        }
      } else {
        assert(dst != klass_RInfo && dst != k_RInfo, "need 3 registers");
        
        __ movl(dst, Address(k_RInfo, sizeof(oopDesc) + Klass::super_check_offset_offset_in_bytes()));
        // See if we get an immediate positive hit
        __ cmpl(k_RInfo, Address(klass_RInfo, dst, Address::times_1));
        __ jcc(Assembler::equal, one);
        // check for immediate negative hit
        __ cmpl(dst, sizeof(oopDesc) + Klass::secondary_super_cache_offset_in_bytes());
        __ jcc(Assembler::notEqual, zero);
        // check for self
        __ cmpl(klass_RInfo, k_RInfo);
        __ jcc(Assembler::equal, one);
        
        __ pushl(klass_RInfo);
        __ pushl(k_RInfo);
        __ call(Runtime1::entry_for(Runtime1::slow_subtype_check_id), relocInfo::runtime_call_type);
        __ popl(klass_RInfo);
        __ popl(dst);
        __ jmp(done);
      }
    }
    __ bind(zero);
    __ xorl(dst, dst);
    __ jmp(done);
    __ bind(one);
    __ movl(dst, 1);
    __ bind(done);
  } else {
    ShouldNotReachHere();
  }
  
}


void LIR_Assembler::emit_compare_and_swap(LIR_OpCompareAndSwap* op) {
  if (op->code() == lir_cas_long) {
    assert(VM_Version::supports_cx8(), "wrong machine");
    assert(op->cmp_value()->as_register_lo() == eax, "wrong register");
    assert(op->cmp_value()->as_register_hi() == edx, "wrong register");
    assert(op->new_value()->as_register_lo() == ebx, "wrong register");
    assert(op->new_value()->as_register_hi() == ecx, "wrong register");
    Register addr = op->addr()->as_register();
    if (os::is_MP()) {
      __ lock();
    }    
    __ cmpxchg8(addr);

  } else if (op->code() == lir_cas_int || op->code() == lir_cas_obj) {
    Register addr = op->addr()->as_register();
    Register newval = op->new_value()->as_register();
    Register cmpval = op->cmp_value()->as_register();
    assert(cmpval == eax, "wrong register");
    assert(newval != NULL, "new val must be register");
    assert(cmpval != newval, "cmp and new values must be in different registers");
    assert(cmpval != addr, "cmp and addr must be in different registers");
    assert(newval != addr, "new value and addr must be in different registers");
    if (os::is_MP()) {
      __ lock();
    }
    __ cmpxchg(newval, addr);
  } else {
    Unimplemented();
  }
}



void LIR_Assembler::arith_op(LIR_Code code, LIR_Opr left, LIR_Opr right, LIR_Opr dest, CodeEmitInfo* info) {
  assert(info == NULL || ((code == lir_rem || code == lir_div || code == lir_sub) && right->is_double_cpu()), "info is only for ldiv/lrem");
  if (right->is_double_cpu()) {
    assert(left->is_double_cpu(), "left must be double");
    assert(right->is_double_cpu(),"right must be double");
    assert(dest->is_double_cpu(), "dest must be double");
    
    Register op1_lo = left->rinfo().as_register_lo();
    Register op1_hi = left->rinfo().as_register_hi();
    Register op2_lo = right->rinfo().as_register_lo();
    Register op2_hi = right->rinfo().as_register_hi();
    Register dst_lo = dest->rinfo().as_register_lo();
    Register dst_hi = dest->rinfo().as_register_hi();

    if (code == lir_add) {
      assert(op2_hi != op1_lo, "overwriting registers");
      __ addl(op1_lo, op2_lo);
      __ adcl(op1_hi, op2_hi);
    } else if (code == lir_sub) {
      assert(op2_hi != op1_lo, "overwriting registers");
      __ subl(op1_lo, op2_lo);
      __ sbbl(op1_hi, op2_hi);
    } else if (code == lir_mul) {
      assert(dest->rinfo().is_same(FrameMap::_eax_edxRInfo), "must be");
      assert(left->is_destroyed(), "bad register allocation");
      Register tmp = noreg;
      if (op1_hi != eax && op1_hi != edx) {
        tmp = op1_hi;
      } else if (op1_lo != eax && op1_lo != edx) {
        swap_reg(op1_lo, op1_hi);
        tmp = op1_lo;
        op1_lo = op1_hi;
        op1_hi = tmp;
      } else if (op2_hi != eax && op2_hi != edx) {
        assert(right->is_destroyed(), "bad register allocation");
        tmp = op2_hi;
      } else if (op2_lo != eax && op2_lo != edx) {
        assert(right->is_destroyed(), "bad register allocation");
        swap_reg(op2_lo, op2_hi);
        tmp = op2_lo;
        op2_lo = op2_hi;
        op2_hi = tmp;
      } else {
        ShouldNotReachHere();
      }
      assert(tmp != noreg, "what?");
      __ imull(op1_hi, op2_lo);
      __ imull(op2_hi, op1_lo);
      if (op1_hi == tmp) {
        __ addl(tmp, op2_hi);
      } else {
        assert(op2_hi == tmp, "must be");
        __ addl(tmp, op1_hi);
      }
      if ((op1_lo == eax && op2_lo == edx) || (op1_lo == edx && op2_lo == eax)) {
        __ mull(edx);
      } else {
        if (op1_lo == eax) {
          __ mull(op2_lo);
        } else if (op2_lo == eax) {
          __ mull(op1_lo);
        } else {
          __ movl(eax, op1_lo);
          __ mull(op2_lo);
        }
      }
      __ addl(edx,tmp);
      op1_lo = eax;
      op1_hi = edx;
    } else if (code == lir_div) {
      __ call(Runtime1::entry_for(Runtime1::ldiv_stub_id), relocInfo::runtime_call_type);
      add_call_info(code_offset(), info);
      op1_lo = eax;
      op1_hi = edx;
    } else if (code == lir_rem) {
      __ call(Runtime1::entry_for(Runtime1::lrem_stub_id), relocInfo::runtime_call_type);
      add_call_info(code_offset(), info);
      op1_lo = eax;
      op1_hi = edx;
    } else {
      ShouldNotReachHere();
    }

    if (dest->rinfo().as_register_lo() == op1_hi) {
      assert(dest->rinfo().as_register_hi() != op1_lo, "overwriting registers");
      move_regs(op1_hi, dest->rinfo().as_register_hi());
      move_regs(op1_lo, dest->rinfo().as_register_lo());
    } else {
      assert(dest->rinfo().as_register_lo() != op1_hi, "overwriting registers");
      move_regs(op1_lo, dest->rinfo().as_register_lo());
      move_regs(op1_hi, dest->rinfo().as_register_hi());
    }
  } else if (right->is_single_fpu() || right->is_double_fpu()) {
    bool must_be_ordered = !(code == lir_add || code == lir_mul || code == lir_mul_strictfp);
    fpu_two_on_tos(right->rinfo(), left->rinfo(), must_be_ordered);
    
    int index = 1;
    switch (code) {
      case lir_add: __ faddp(index);            break;
      case lir_sub: __ fsubp(index);            break;
      case lir_mul: __ fmulp(index);            break;
      case lir_mul_strictfp:
        {
          if (right->is_single_fpu()) {
            __ fmulp(index);
          } else {
            // Double values require special handling for strictfp mul/div on x86
            __ fld_x(Address((int)StubRoutines::addr_fpu_subnormal_bias1(), relocInfo::none));
            __ fmulp(index);
            __ fmulp(index);
            __ fld_x(Address((int)StubRoutines::addr_fpu_subnormal_bias2(), relocInfo::none));
            __ fmulp();
          }
        }
        break;
      case lir_div: __ fdivp(index);            break;
      case lir_div_strictfp:
        if (right->is_single_fpu()) {
          __ fdivp(index);
        } else {
          // Double values require special handling for strictfp mul/div on x86
          __ fld_x(Address((int)StubRoutines::addr_fpu_subnormal_bias1(), relocInfo::none));
          __ fmulp(index+1); // scale dividend
          __ fdivp(index);
          __ fld_x(Address((int)StubRoutines::addr_fpu_subnormal_bias2(), relocInfo::none));
          __ fmulp(index);
        }
        break;
      case lir_rem: __ fxch(); __ fremr(noreg); break;
      default     : ShouldNotReachHere();
    }
    fpu_pop(right->rinfo());
    fpu_pop(left->rinfo());
    fpu_push(dest->rinfo());
  } else if (right->is_stack()) {
    if (left->is_single_cpu()) {
      Address raddr = frame_map()->address_for_name(right->single_stack_ix(), false);
      switch (code) {
        case lir_add: __ addl(left->rinfo().as_register(), raddr); break;
        case lir_mul: Unimplemented();     break;
        case lir_sub: __ subl(left->rinfo().as_register(), raddr); break;
        default              : ShouldNotReachHere();
      }   
      // move must be at the end as res may be the rreg parameter
      move_regs(left->rinfo().as_register(), dest->rinfo().as_register());
    } else {
      fpu_on_tos(left->rinfo());
      if (right->is_double_stack()) {
        assert(left->is_double_fpu(), "unmatched type");
        Address addr = frame_map()->address_for_name(right->double_stack_ix(), true);
        switch (code) {
          case lir_add: __ fadd_d(addr); break;
          case lir_sub: __ fsub_d(addr); break;
          case lir_mul: __ fmul_d(addr); break;
          case lir_mul_strictfp:
            {
              // Double values require special handling for strictfp mul/div on x86
              __ fld_x(Address((int)StubRoutines::addr_fpu_subnormal_bias1(), relocInfo::none));
              __ fmulp();
              __ fmul_d(addr);
              __ fld_x(Address((int)StubRoutines::addr_fpu_subnormal_bias2(), relocInfo::none));
              __ fmulp();
            }
            break;
          case lir_div: __ fdiv_d(addr); break;
          case lir_div_strictfp:
            {
              // Double values require special handling for strictfp mul/div on x86
              __ fld_x(Address((int)StubRoutines::addr_fpu_subnormal_bias1(), relocInfo::none));
              __ fmulp(); // scale dividend
              __ fdiv_d(addr);
              __ fld_x(Address((int)StubRoutines::addr_fpu_subnormal_bias2(), relocInfo::none));
              __ fmulp();
            }
            break;
          default     : ShouldNotReachHere();
        }
      } else {
        assert(left->is_single_fpu(), "unmatched type");
        Address addr = frame_map()->address_for_name(right->single_stack_ix(), false);
        switch (code) {
          case lir_add: __ fadd_s(addr); break;
          case lir_sub: __ fsub_s(addr); break;
          case lir_mul: // fall through
          case lir_mul_strictfp: __ fmul_s(addr); break;
          case lir_div: // fall through
          case lir_div_strictfp: __ fdiv_s(addr); break;
          default     : ShouldNotReachHere();
        }
      }
      fpu_pop(left->rinfo());
      fpu_push(dest->rinfo());
    }
  } else if (left->is_stack() && dest->is_stack()) {
    assert(left->single_stack_ix() == dest->single_stack_ix(), "must be the same");
    Address addr = frame_map()->address_for_name(left->single_stack_ix(), false);
    if (right->is_single_cpu()) {
      switch (code) {
        case lir_add: __ addl (addr, right->as_register()); break;
        case lir_mul: Unimplemented();     break;
        case lir_sub: __ subl (addr, right->as_register()); break;
        default              : ShouldNotReachHere();
      }   
    } else if (right->is_constant()) {
      jint c = right->as_constant_ptr()->as_jint();
      switch (code) { 
        case lir_add:  
          { 
            switch (c) {
              case  1: __ incl(addr);    break;
              case -1: __ decl(addr);    break;
              default: __ addl(addr, c); break;
            }
          }
          break;
        case lir_sub:  
          {
            switch (c) {
              case  1: __ decl(addr);    break;
              case -1: __ incl(addr);    break;
              default: __ subl(addr, c); break;
            }
          }
          break;
        default: ShouldNotReachHere();
      }
    } else {
      ShouldNotReachHere();
    }
  } else if (left->is_single_cpu() && dest->is_single_cpu()) {
    Register lreg = left->rinfo().as_register();
    if (right->is_single_cpu()) {
      Register rreg = right->rinfo().as_register();
      switch (code) {
        case lir_add: __ addl(lreg, rreg);  break;
        case lir_mul: __ imull(lreg, rreg); break;
        case lir_sub: __ subl(lreg, rreg);  break;
        default:      ShouldNotReachHere();
      }
    } else if (right->is_constant()) {
      jint c = right->as_constant_ptr()->as_jint();
      switch (code) { 
        case lir_add:  
          { 
            switch (c) {
              case  1: __ incl(lreg);    break;
              case -1: __ decl(lreg);    break;
              default: __ addl(lreg, c); break;
            }
          }
          break;
        case lir_sub:  
          {
            switch (c) {
              case  1: __ decl(lreg);    break;
              case -1: __ incl(lreg);    break;
              default: __ subl(lreg, c); break;
            }
          }
          break;
        default: ShouldNotReachHere();
      }
    } else {
      Unimplemented();
    }
    Register res = dest->rinfo().as_register();
    
    // move must be at the end as res may be the rreg parameter
    move_regs(lreg, res);
  } else {
    ShouldNotReachHere();
  }
}


void LIR_Assembler::intrinsic_op(LIR_Code code, LIR_Opr value, LIR_Opr unused, LIR_Opr dest) {
  if (value->is_double_fpu()) {
    fpu_on_tos(value->rinfo());
    fpu_pop(value->rinfo());
    switch(code) {
      case lir_sqrt: __ fsqrt(); break;
      case lir_sin :
      case lir_cos :
        // Should consider not saving ebx if not necessary
        __ sincos((code == lir_sin), true, fpu_stack_size());
        break;
      default      : ShouldNotReachHere();
    }
    fpu_push(dest->rinfo());
  } else {
    Unimplemented();
  }
}

void LIR_Assembler::logic_op(LIR_Code code, LIR_Opr left, LIR_Opr right, LIR_Opr dst) {
  // assert(left->destroys_register(), "check");
  if (left->is_single_cpu()) {
    Register reg = left->rinfo().as_register();
    if (right->is_constant()) {
      int val = right->as_constant_ptr()->as_jint();
      switch (code) {
        case lir_logic_and: __ andl (reg, val); break;
        case lir_logic_orcc: // fall through
        case lir_logic_or:  __ orl  (reg, val); break;
        case lir_logic_xor: __ xorl (reg, val); break;
        default: ShouldNotReachHere();
      }
    } else {
      Register rright = right->rinfo().as_register();
      switch (code) {
        case lir_logic_and: __ andl (reg, rright); break;
        case lir_logic_orcc: // fall through
        case lir_logic_or : __ orl  (reg, rright); break;
        case lir_logic_xor: __ xorl (reg, rright); break;
        default: ShouldNotReachHere();
      }
    }
    move_regs(reg, dst->rinfo().as_register());
  } else {
    Register l_lo = left->rinfo().as_register_lo();
    Register l_hi = left->rinfo().as_register_hi();
    if (right->is_constant()) {
      int r_lo = right->as_constant_ptr()->as_jint_lo();
      int r_hi = right->as_constant_ptr()->as_jint_hi();
      switch (code) {
        case lir_logic_and: 
          __ andl(l_lo, r_lo);
          __ andl(l_hi, r_hi);
          break;
        case lir_logic_or:
          __ orl(l_lo, r_lo);
          __ orl(l_hi, r_hi);
          break;
        case lir_logic_xor:
          __ xorl(l_lo, r_lo);
          __ xorl(l_hi, r_hi);
          break;
        default: ShouldNotReachHere();
      }
    } else {
      Register r_lo = right->rinfo().as_register_lo();
      Register r_hi = right->rinfo().as_register_hi();
      assert(l_lo != r_hi, "overwriting registers");
      switch (code) {
        case lir_logic_and: 
          __ andl(l_lo, r_lo);
          __ andl(l_hi, r_hi);
          break;
        case lir_logic_or:
          __ orl(l_lo, r_lo);
          __ orl(l_hi, r_hi);
          break;
        case lir_logic_xor:
          __ xorl(l_lo, r_lo);
          __ xorl(l_hi, r_hi);
          break;
        default: ShouldNotReachHere();
      }
    }

    Register dst_lo = dst->rinfo().as_register_lo();
    Register dst_hi = dst->rinfo().as_register_hi();

    if (dst_lo == l_hi) {
      assert(dst_hi != l_lo, "overwriting registers");
      move_regs(l_hi, dst_hi);
      move_regs(l_lo, dst_lo);
    } else {
      assert(dst_lo != l_hi, "overwriting registers");
      move_regs(l_lo, dst_lo);
      move_regs(l_hi, dst_hi);
    }
  }
}


static int real_index(FrameMap* fm, int java_index) {
  return fm->size_arguments() - java_index + (java_index < fm->size_arguments() ? 1 : -1);
}


void LIR_Assembler::maybe_adjust_stack_alignment(ciMethod* method) {
  if (ForceStackAlignment && !method->is_native() && !compilation()->is_optimized_library_method()) {
    offsets()->_iep_offset = code_offset();
    
    __ movl(esi, esp);
    __ andl(esi, 4);
    __ cmpl(esi, 4);
    __ jcc(Assembler::equal, _vep_label);

    const CallingConvention* cc = FrameMap::calling_convention(method);
    const int n = method->arg_size();

    __ enter();
    __ subl(esp, (round_to(n, 2) + 1) * BytesPerWord);

    for (int i = 0; i < n; i++) {
      ArgumentLocation loc = cc->arg_at(n - 1 - i);
      int offset = real_index(frame_map(), i);
      Address src(ebp, (offset + 0) * BytesPerWord);
      Address dst(esp, (offset - 2) * BytesPerWord);

      __ movl(esi, src);
      __ movl(dst, esi);
    }
    // make it look like we are called from our cleanup code
    __ pushl((int)Runtime1::entry_for(Runtime1::alignment_frame_return_id));
    __ jmp(_vep_label);
  }
}


// we assume that eax and edx can be overwritten
void LIR_Assembler::arithmetic_idiv(LIR_Code code, LIR_Opr left, LIR_Opr right, LIR_Opr temp, LIR_Opr result, CodeEmitInfo* info) {

  assert(left->is_single_cpu(),   "left must be register");
  assert(right->is_single_cpu() || right->is_constant(),  "right must be register or constant");
  assert(result->is_single_cpu(), "result must be register");

//  assert(left->destroys_register(), "check");
//  assert(right->destroys_register(), "check");

  Register lreg = left->rinfo().as_register();
  Register dreg = result->rinfo().as_register();

  if (right->is_constant()) {
    int divisor = right->as_constant_ptr()->as_jint();
    assert(divisor > 0 && is_power_of_2(divisor), "must be");
    if (code == lir_idiv) {
      assert(lreg == eax, "must be eax");
      assert(temp->rinfo().as_register() == edx, "tmp register must be edx");
      __ cdql(); // sign extend into edx:eax
      if (divisor == 2) {
        __ subl(lreg, edx);
      } else {
        __ andl(edx, divisor - 1);
        __ addl(lreg, edx);
      }
      __ sarl(lreg, log2_intptr(divisor));
      move_regs(lreg, dreg);
    } else if (code == lir_irem) {
      Label done;
      __ movl(dreg, lreg);
      __ andl(dreg, 0x80000000 | (divisor - 1));
      __ jcc(Assembler::positive, done);
      __ decl(dreg);
      __ orl(dreg, ~(divisor - 1));
      __ incl(dreg);
      __ bind(done);
    } else {
      ShouldNotReachHere();
    }
  } else {
    Register rreg = right->rinfo().as_register();
    assert(lreg == eax, "left register must be eax");
    assert(rreg != edx, "right register must not be edx");
    assert(temp->rinfo().as_register() == edx, "tmp register must be edx");
    
    move_regs(lreg, eax);
    
    int idivl_offset = __ corrected_idivl(rreg);
    add_debug_info_for_div0(idivl_offset, info);
    if (code == lir_irem) {
      move_regs(edx, dreg); // result is in edx
    } else {
      move_regs(eax, dreg);
    }
  }
}


void LIR_Assembler::comp_op(LIR_OpBranch::LIR_Condition condition, LIR_Opr opr1, LIR_Opr opr2, BasicType type) {
  if (opr1->is_single_cpu()) {
    if (opr2->is_pointer()) {
      if (opr2->is_constant()) {
        if (opr2->pointer()->as_constant()->type() == T_INT) {
          __ cmpl(opr1->rinfo().as_register(), opr2->pointer()->as_constant()->as_jint());
        } else if (opr2->pointer()->as_constant()->type() == T_OBJECT) {
          __ cmpl(opr1->rinfo().as_register(), opr2->pointer()->as_constant()->as_jobject());
        } else {
          ShouldNotReachHere();
        }
      } else if (opr2->is_address()) {
        __ cmpl(opr1->rinfo().as_register(), as_Address(opr2->pointer()->as_address()));
      } else {
        ShouldNotReachHere();
      }
    } else if (opr2->is_single_cpu()) {
      __ cmpl(opr1->rinfo().as_register(), opr2->rinfo().as_register());
    } else if (opr2->is_stack()) {
      __ cmpl(opr1->rinfo().as_register(), frame_map()->address_for_name(opr2->single_stack_ix(), false));
    } else {
      Unimplemented();
    }
  } else if(opr1->is_address()) {
    if (opr2->is_constant()) {
      if (opr2->as_constant_ptr()->type() == T_INT) {
        __ cmpl (as_Address(opr1->pointer()->as_address()), opr2->as_constant_ptr()->as_jint());
      } else if (opr2->as_constant_ptr()->type() == T_OBJECT) {
        __ cmpl (as_Address(opr1->pointer()->as_address()), opr2->as_constant_ptr()->as_jobject());
      } else {
        Unimplemented();
      }
    } else {
      Unimplemented();
    }
  } else if(opr1->is_double_cpu()) {
    assert(opr1->is_destroyed(), "check");
    Register xlo = opr1->as_register_lo();
    Register xhi = opr1->as_register_hi();
    if (opr2->is_constant()) {
      assert(opr2->as_jlong() == (jlong)0, "only handles zero");
      assert(condition == LIR_OpBranch::equal || condition == LIR_OpBranch::notEqual, "only handles equals case");
      __ orl(xhi, xlo);
    } else {
      Register ylo = opr2->as_register_lo();
      Register yhi = opr2->as_register_hi();
      __ subl(xlo, ylo);
      __ sbbl(xhi, yhi);
      if (condition == LIR_OpBranch::equal || condition == LIR_OpBranch::notEqual) {
        __ orl(xhi, xlo);
      }
    }
  } else if(opr1->is_single_fpu() || opr1->is_double_fpu()) {
    // assert (opr1->type_field() == opr2->type_field(), "Must compare same types");
    // we cannot mirror the condition any more :-(
    fpu_two_on_tos(opr1->rinfo(), opr2->rinfo(), true);
    fpu_pop(opr1->rinfo());
    fpu_pop(opr2->rinfo());
    __ fcmp(noreg);
  } else {
    Unimplemented();
  }
}

void LIR_Assembler::comp_fl2i(LIR_Code code, LIR_Opr left, LIR_Opr right, LIR_Opr dst) {
  if (code == lir_cmp_fd2i || code == lir_ucmp_fd2i) {
    fpu_two_on_tos(left->rinfo(), right->rinfo(), true);
    fpu_pop(left->rinfo());
    fpu_pop(right->rinfo());
    __ fcmp2int(dst->rinfo().as_register(), code == lir_ucmp_fd2i);
  } else {
    assert(code == lir_cmp_l2i, "check");
    __ lcmp2int(left->rinfo().as_register_hi(),
                left->rinfo().as_register_lo(),
                right->rinfo().as_register_hi(),
                right->rinfo().as_register_lo());
    move_regs(left->rinfo().as_register_hi(), dst->rinfo().as_register());
  }
}


void LIR_Assembler::align_call(LIR_Code code) {
  if (os::is_MP()) {
    // make sure that the displacement word of the call ends up word aligned
    int offset = __ offset();
    switch (code) {
    case lir_static_call:  
    case lir_optvirtual_call: 
      offset += NativeCall::displacement_offset;
      break;
    case lir_icvirtual_call:
      offset += NativeCall::displacement_offset + NativeMovConstReg::instruction_size;
      break;
    case lir_virtual_call:  // currently, sparc-specific for niagara
    default: ShouldNotReachHere();
    }
    while (offset++ % BytesPerWord != 0) {
      __ nop();
    }
  }
}


void LIR_Assembler::call(address entry, relocInfo::relocType rtype, CodeEmitInfo* info) {
  assert(!os::is_MP() || (__ offset() + NativeCall::displacement_offset) % BytesPerWord == 0,
         "must be aligned");
  __ call(entry, rtype);
  add_call_info(code_offset(), info);
}


void LIR_Assembler::ic_call(address entry, CodeEmitInfo* info) {
  RelocationHolder rh = virtual_call_Relocation::spec(pc());
  __ movl(IC_Klass, (jobject)Universe::non_oop_word());
  assert(!os::is_MP() ||
         (__ offset() + NativeCall::displacement_offset) % BytesPerWord == 0,
         "must be aligned");
  __ call(entry, rh);
  add_call_info(code_offset(), info);
}


/* Currently, vtable-dispatch is only enabled for sparc platforms */
void LIR_Assembler::vtable_call(int vtable_offset, CodeEmitInfo* info) {
    ShouldNotReachHere();
}

void LIR_Assembler::emit_call_stubs() { 
  _masm->code()->set_stubs_begin(_masm->pc());
  emit_stubs(_call_stubs);
  // Interpreter entry point (argument marshalling)
  maybe_adjust_stack_alignment(method());
  _masm->code()->set_stubs_end( _masm->pc());
}


void LIR_Assembler::setup_locals_at_entry() {
  BasicTypeArray* sig_types = FrameMap::signature_type_array_for(compilation()->method());

  const LocalMapping* cached_locals = current_block() != NULL ? current_block()->local_mapping() : NULL;
  CallingConvention* args = FrameMap::calling_convention(compilation()->method());
  for (int n = 0; n < args->length(); n++) {
    ArgumentLocation loc = args->arg_at(n);
    RInfo cache_reg = cached_locals != NULL ? cached_locals->get_cache_reg(n, as_ValueType(sig_types->at(n))->tag()) : norinfo;
    if (!cache_reg.is_illegal()) {
      // local n is cached in cache_reg
      assert(cache_reg.is_word(), "cannot handle others yet");
      assert(!loc.is_register_arg(), "we don't have register args on intel");
      __ movl(cache_reg.as_register(), frame_map()->address_for_local_name(frame_map()->name_for_argument(n), false));
      if (C1InvalidateCachedOopLocation) {
        // store -1 in local location making that location invalid instead of
        // having a stale copy there
        __ movl(frame_map()->address_for_local_name(frame_map()->name_for_argument(n), false), -1); 
      }
    }
  }
}


void LIR_Assembler::throw_op(RInfo exceptionPC, RInfo exceptionOop, CodeEmitInfo* info, bool unwind) {
  assert(exceptionOop.is_same(FrameMap::_eaxRInfo), "must match");

  if (SafepointPolling) {
    info->add_register_oop(exceptionOop);
    safepoint_poll(exceptionPC, info);
  }

  // get current pc information
  int pc_for_athrow  = (int)__ pc();
  int pc_for_athrow_offset = __ offset();
  __ nop(); // pc_for_athrow can not point to itself (relocInfo restriction)
  __ leal(exceptionPC.as_register(), Address(pc_for_athrow, relocInfo::internal_word_type));
  add_call_info(pc_for_athrow_offset, info); // for exception handler

  if (JvmtiExport::can_post_exceptions()) {
    // Tell the runtime that we have thrown an exception
    __ call(Runtime1::entry_for(Runtime1::jvmdi_exception_throw_id), relocInfo::runtime_call_type);
    info->add_register_oop(exceptionOop);
    add_call_info(__ offset(), info);
  }

  if (UseCompilerSafepoints && !SafepointPolling) {
    info->add_register_oop(exceptionOop);
    add_debug_info_for_branch(info);
  }

  if (unwind) {
    __ jmp(_unwind_entry_label);
  } else {
    __ jmp(_throw_entry_label);
  }
  // enough room for two byte trap
  __ nop();
}


void LIR_Assembler::shift_op(LIR_Code code, RInfo leftR, RInfo countR, RInfo destR, RInfo tmpR) {
  if (destR.is_word()) {
    Register value = leftR.as_register();
    Register count = countR.as_register();
    Register tmp   = tmpR.as_register();
    assert_different_registers(count, tmp);
    assert_different_registers(value, tmp);
    // normalize registers so that count is in ecx; the register allocator
    // may have not be successful in doing that.
    // Situations:
    //    - count is in ecx, everything is fine
    //    - value(left) is in ecx:
    //        - move value into tmp (frees ecx for count)
    //        - move count into ecx
    //        - shift tmp
    //        - move tmp into dest
    //        ( no need to restore ecx, as value is marked as being destroyed, see CodeGenerator)
    //    - value is not ecx, count is not ecx
    //        - move ecx into tmp (preserve ecx)
    //        - move count into ecx
    //        - shift value
    //        - move tmo into ecx (restore ecx)
    bool must_restore = false; // must restore SHIFT_count
    if (count != SHIFT_count) {
      if (value == SHIFT_count) {
        __ movl(tmp, value);
        __ movl(SHIFT_count, count);
        value = tmp;
      } else {
        assert_different_registers(SHIFT_count, value);
        assert_different_registers(SHIFT_count, count);
        __ movl(tmp, SHIFT_count);
        __ movl(SHIFT_count, count);
        must_restore = true;
      }
    }
    switch (code) {
      case lir_shl:  __ shll(value); break;
      case lir_shr:  __ sarl(value); break;
      case lir_ushr: __ shrl(value); break;
      default: ShouldNotReachHere();
    }
    if (must_restore) {
      move_regs(tmp, SHIFT_count); // tmp -> SHIFT_count
    }
    move_regs(value, destR.as_register());
  } else if (destR.is_long()) {
    Register creg = countR.as_register();
    Register lo = leftR.as_register_lo();
    Register hi = leftR.as_register_hi();
    Register lo_val = lo;
    Register hi_val = hi;
    bool lo_swapped = false;
    bool hi_swapped = false;
    if (hi_val == SHIFT_count) {
      hi_swapped = true;
      swap_reg(hi, creg);
      hi_val = creg;
    } else if (lo_val == SHIFT_count) {
      lo_swapped = true;
      swap_reg(lo, creg);
      lo_val = creg;
    } else if (creg != SHIFT_count) {
      __ push_reg(SHIFT_count);
      __ movl (SHIFT_count, creg);
    }
    assert(lo_val != SHIFT_count && hi_val != SHIFT_count, "ecx is count register");
    switch (code) {
      case lir_shl:  __ lshl(hi_val, lo_val);        break;
      case lir_shr:  __ lshr(hi_val, lo_val, true);  break;
      case lir_ushr: __ lshr(hi_val, lo_val, false); break;
      default: ShouldNotReachHere();
    }
    if (hi_swapped) {
      swap_reg(hi, creg);
    } else if (lo_swapped) {
      swap_reg(lo, creg);
    } else if (creg != SHIFT_count) {
      __ pop(SHIFT_count);
    }
  } else {
    ShouldNotReachHere();
  }
}


void LIR_Assembler::shift_op(LIR_Code code, RInfo leftR, jint  count, RInfo destR) {
  Register value = leftR.as_register();
  count = count & 0x1F; // Java spec
  if (destR.is_word()) {
    switch (code) {
      case lir_shl:  __ shll(value, count); break;
      case lir_shr:  __ sarl(value, count); break;
      case lir_ushr: __ shrl(value, count); break;
      default: ShouldNotReachHere();
    }
    move_regs(value, destR.as_register());
  } else if (destR.is_long()) {
    Unimplemented();
  } else {
    ShouldNotReachHere();
  }
}

NEEDS_CLEANUP // check same routine in LIR_Emitter
void LIR_Assembler::push_parameter(Register r, int offset_from_esp_in_words) {
  assert(offset_from_esp_in_words >= 0, "invalid offset from esp");
  assert(offset_from_esp_in_words < frame_map()->reserved_argument_area_size(), "invalid offset");
  int offset_from_esp_in_bytes = offset_from_esp_in_words * BytesPerWord;
  __ movl (Address(esp, offset_from_esp_in_bytes), r);
}


void LIR_Assembler::push_parameter(jint c,     int offset_from_esp_in_words) {
  assert(offset_from_esp_in_words >= 0, "invalid offset from esp");
  assert(offset_from_esp_in_words < frame_map()->reserved_argument_area_size(), "invalid offset");
  int offset_from_esp_in_bytes = offset_from_esp_in_words * BytesPerWord;
  __ movl (Address(esp, offset_from_esp_in_bytes), c);
}


// This code replaces a call to arraycopy; no exception may 
// be thrown in this code, they must be thrown in the System.arraycopy
// activation frame; we could save some checks if this would not be the case
void LIR_Assembler::emit_arraycopy(LIR_OpArrayCopy* op) {
  ciArrayKlass* default_type = op->expected_type();
  Register src = op->src()->as_register();
  Register dst = op->dst()->as_register();
  Register src_pos = op->src_pos()->as_register();
  Register dst_pos = op->dst_pos()->as_register();
  Register length  = op->length()->as_register();
  Register tmp = op->tmp()->as_register();
  int flags = op->flags();
  BasicType basic_type = default_type != NULL ? default_type->element_type()->basic_type() : T_ILLEGAL;
  if (basic_type == T_ARRAY) basic_type = T_OBJECT;

  // if we don't know anything or it's an object array, just go through the generic arraycopy
  if (default_type == NULL) {
    Label done;
    // save outgoing arguments on stack in case call to System.arraycopy is needed
    push_parameter(length, 0);
    push_parameter(dst_pos, 1);
    push_parameter(dst, 2);
    push_parameter(src_pos, 3);
    push_parameter(src, 4);
    
    // pass arguments: may push as this is not a safepoint; SP must be fix at each safepoint
    __ pushl(length);
    __ pushl(dst_pos);
    __ pushl(dst);
    __ pushl(src_pos);
    __ pushl(src);
    address entry = CAST_FROM_FN_PTR(address, Runtime1::arraycopy);
    __ call_VM_leaf(entry, 5); // removes pushed parameter from the stack
    
    __ cmpl(eax, 0);
    __ jcc(Assembler::equal, done);

    align_call(lir_static_call);
    
    StaticCallStub* stub = new StaticCallStub(__ pc());
    _call_stubs->append(stub);
    __ call(Runtime1::entry_for(Runtime1::resolve_invokestatic_id), relocInfo::static_call_type);
    add_call_info(code_offset(), op->info());
    __ bind(done);
    return;
  }

  assert(default_type != NULL && default_type->is_array_klass() && default_type->is_loaded(), "must be true at this point");

  int elem_size = type2aelembytes[basic_type];
  int shift_amount;
  Address::ScaleFactor scale;

  switch (elem_size) {
  case 1 :
    shift_amount = 0;
    scale = Address::times_1;
    break;
  case 2 :
    shift_amount = 1;
    scale = Address::times_2;
    break;
  case 4 :
    shift_amount = 2; 
    scale = Address::times_4;
    break;
  case 8 :
    shift_amount = 3; 
    scale = Address::times_8;
    break;
  default:
    ShouldNotReachHere();
  }

  Address src_length_addr = Address(src, arrayOopDesc::length_offset_in_bytes());
  Address dst_length_addr = Address(dst, arrayOopDesc::length_offset_in_bytes());
  Address src_klass_addr = Address(src, oopDesc::klass_offset_in_bytes());
  Address dst_klass_addr = Address(dst, oopDesc::klass_offset_in_bytes());

  StaticCallStub* call_stub = new StaticCallStub(); // pc will get filled in by ArrayCopyStub
  ArrayCopyStub* stub = new ArrayCopyStub(op->info(), call_stub);
  stub->set_tmp(op->tmp()->rinfo());
  stub->set_src(as_RInfo(src));
  stub->set_src_pos(as_RInfo(src_pos));
  stub->set_dst(as_RInfo(dst));
  stub->set_dst_pos(as_RInfo(dst_pos));
  stub->set_length(as_RInfo(length));
  emit_code_stub(stub);
  emit_code_stub(call_stub);

  // test for NULL
  if (flags & LIR_OpArrayCopy::src_null_check) {
    __ testl(src, src);
    __ jcc(Assembler::zero, *stub->entry());
  }
  if (flags & LIR_OpArrayCopy::dst_null_check) {
    __ testl(dst, dst);
    __ jcc(Assembler::zero, *stub->entry());
  }

  // check if negative
  if (flags & LIR_OpArrayCopy::src_pos_positive_check) {
    __ testl(src_pos, src_pos);
    __ jcc(Assembler::less, *stub->entry());
  }
  if (flags & LIR_OpArrayCopy::dst_pos_positive_check) {
    __ testl(dst_pos, dst_pos);
    __ jcc(Assembler::less, *stub->entry());
  }
  if (flags & LIR_OpArrayCopy::length_positive_check) {
    __ testl(length, length);
    __ jcc(Assembler::less, *stub->entry());
  }
    
  if (flags & LIR_OpArrayCopy::src_range_check) {
    __ leal(tmp, Address(src_pos, length, Address::times_1, 0));
    __ cmpl(tmp, src_length_addr);
    __ jcc(Assembler::above, *stub->entry());
  }
  if (flags & LIR_OpArrayCopy::dst_range_check) {
    __ leal(tmp, Address(dst_pos, length, Address::times_1, 0));
    __ cmpl(tmp, dst_length_addr);
    __ jcc(Assembler::above, *stub->entry());
  }

  if (flags & LIR_OpArrayCopy::type_check) {
    __ movl(tmp, src_klass_addr);
    __ cmpl(tmp, dst_klass_addr);
    __ jcc(Assembler::notEqual, *stub->entry());
  }

#ifdef ASSERT
  if (basic_type != T_OBJECT || !(flags & LIR_OpArrayCopy::type_check)) {
    // Sanity check the known type with the incoming class.  For the
    // primitive case the types must match exactly.  For the object array
    // case, if no type check is needed then the dst type must match the
    // expected type and the src type is so subtype which we can't check.  If
    // a type check is needed then at this point the classes are known to be
    // the same but again which don't know which type so we can't check them.
    Label known_ok, halt;
    __ movl(tmp, default_type->encoding());
    __ cmpl(tmp, dst_klass_addr);
    if (basic_type != T_OBJECT) {
      __ jcc(Assembler::notEqual, halt);
      __ cmpl(tmp, src_klass_addr);
    }
    __ jcc(Assembler::equal, known_ok);
    __ bind(halt);
    __ stop("incorrect type information in arraycopy");
    __ bind(known_ok);
  }
#endif

  __ leal(tmp, Address(src, src_pos, scale, arrayOopDesc::base_offset_in_bytes(basic_type)));
  push_parameter(tmp, 0);
  __ leal(tmp, Address(dst, dst_pos, scale, arrayOopDesc::base_offset_in_bytes(basic_type)));
  push_parameter(tmp, 1);
  if (shift_amount > 0 && basic_type != T_OBJECT) {
    __ shll(length, shift_amount);
  }
  push_parameter(length, 2);
  if (basic_type == T_OBJECT) {
    __ call_VM_leaf(CAST_FROM_FN_PTR(address, Runtime1::oop_arraycopy), 0);
  } else {
    __ call_VM_leaf(CAST_FROM_FN_PTR(address, Runtime1::primitive_arraycopy), 0);
  }
  
  __ bind(*stub->continuation());
}


void LIR_Assembler::emit_lock(LIR_OpLock* op) {
  Register obj = op->obj_opr()->rinfo().as_register();  // may not be an oop
  Register hdr = op->hdr_opr()->rinfo().as_register();
  Register lock = op->lock_opr()->rinfo().as_register();
  if (!UseFastLocking) {
    __ jmp(*op->stub()->entry());
  } else if (op->code() == lir_lock) {
    assert(BasicLock::displaced_header_offset_in_bytes() == 0, "lock_reg must point to the displaced header");
    // add debug info for NullPointerException only if one is possible
    if (op->info() != NULL) {
      add_debug_info_for_null_check_here(op->info());
    }
    __ lock_object(hdr, obj, lock, *op->stub()->entry());
    // done
  } else if (op->code() == lir_unlock) {
    assert(BasicLock::displaced_header_offset_in_bytes() == 0, "lock_reg must point to the displaced header");
    __ unlock_object(hdr, obj, lock, *op->stub()->entry());
  } else {
    Unimplemented();
  }
  __ bind(*op->stub()->continuation());
}

void LIR_Assembler::monitor_address(int monitor_no, RInfo dst) {
  __ leal(dst.as_register(), frame_map()->address_for_monitor_lock_index(monitor_no));
}


void LIR_Assembler::align_backward_branch_target() {
  __ align(BytesPerWord);
}


void LIR_Assembler::negate(LIR_Opr left, LIR_Opr dest) {
  assert(left->is_single_cpu() || 
         left->is_double_cpu() ||
         left->is_single_fpu() ||
         left->is_double_fpu(), "can only handle registers");

  RInfo leftR = left->rinfo();
  RInfo destR = dest->rinfo();
  if (leftR.is_word()) {
    __ negl(leftR.as_register());
    move_regs(leftR.as_register(), destR.as_register());
  } else if (leftR.is_float_kind()) {
    fpu_on_tos(leftR);
    fpu_pop(leftR);
    __ fchs();
    fpu_push(destR);
  } else if (leftR.is_long()) {
    Register lo = leftR.as_register_lo();
    Register hi = leftR.as_register_hi();
    __ lneg(hi, lo);
    if (destR.as_register_lo() == hi) {
      assert(destR.as_register_hi() != lo, "destroying register");
      move_regs (hi, destR.as_register_hi());
      move_regs (lo, destR.as_register_lo());
    } else {
      move_regs (lo, destR.as_register_lo());
      move_regs (hi, destR.as_register_hi());
    }
  } else {
    ShouldNotReachHere();
  }
}


void LIR_Assembler::leal(LIR_Opr addr, LIR_Opr dest) {
  assert(addr->is_address() && dest->is_register(), "check");
  Register reg = dest->rinfo().as_register();
  __ leal(dest->as_register(), as_Address(addr->as_address_ptr()));
}



void LIR_Assembler::dup_fpu(RInfo from_reg, RInfo to_reg) {
  assert(from_reg.is_float_kind(), "wrong item register");
  int offset = frame_map()->fpu_stack()->offset_from_tos(from_reg.fpu());
  __ fld_s(offset);
  frame_map()->fpu_stack()->push(to_reg.fpu());
}


void LIR_Assembler::rt_call(address dest, RInfo tmp, int size_in_words, int args) {
  __ call_VM_leaf(dest, size_in_words);
  __ dec_stack_after_call(size_in_words);
}


void LIR_Assembler::new_multi_array(int rank, RInfo dest, CodeEmitInfo* info) {
  __ movl(ebx, rank);
  __ movl(ecx, esp);
  __ call(Runtime1::entry_for(Runtime1::new_multi_array_id), relocInfo::runtime_call_type);
  add_call_info(code_offset(), info);
  move_regs(eax, dest.as_register());
}


void LIR_Assembler::volatile_move_op(LIR_Opr src, LIR_Opr dest, BasicType type, LIR_Op1::LIR_PatchCode patch_code, CodeEmitInfo* info) {
  PatchingStub* patch = NULL;
  Register base = noreg;

  if (src->is_register() && dest->is_address()) {
    assert(type == T_LONG, "used for longs only (they are stored in two registers)");
    base = dest->as_address_ptr()->base()->as_register();

    RInfo srcR = src->rinfo();
    // there must be space on stack
    __ movl(Address(esp, BytesPerWord), srcR.as_register_hi());
    __ movl(Address(esp),               srcR.as_register_lo());
    __ fild_d(Address(esp));

    if (patch_code != LIR_Op1::patch_none) {
      patch = new PatchingStub(_masm, PatchingStub::access_field_id);
    }
    if (info != NULL) {
      add_debug_info_for_null_check_here(info);
    }
    __ fistp_d(as_Address(dest->as_address_ptr()));
  } else if (src->is_address() && dest->is_register()) {
    assert(type == T_LONG, "used for longs only (they are stored in two registers)");
    base = src->as_address_ptr()->base()->as_register();
    
    if (patch_code != LIR_Op1::patch_none) {
      patch = new PatchingStub(_masm, PatchingStub::access_field_id);
    }
    if (info != NULL) {
      add_debug_info_for_null_check_here(info);
    }
    __ fild_d(as_Address(src->as_address_ptr()));
    __ fistp_d(Address(esp));
    RInfo destR = dest->rinfo();
    __ movl(destR.as_register_lo(), Address(esp));
    __ movl(destR.as_register_hi(), Address(esp, BytesPerWord));  
  } else {
    Unimplemented();
  }
  if (patch != NULL) {
    patching_epilog(patch, patch_code, base, info);
  }
}


void LIR_Assembler::membar() {
  __ membar();
}

void LIR_Assembler::membar_acquire() {
  // No x86 machines currently require load fences
  // __ load_fence();
}

void LIR_Assembler::membar_release() {
  // No x86 machines currently require store fences
  // __ store_fence();
}

void LIR_Assembler::get_thread(LIR_Opr result_reg) {
  assert(result_reg->is_register(), "check");
  __ get_thread(result_reg->as_register());
}

#undef __ 
