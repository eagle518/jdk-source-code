#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_LIRAssembler_sparc.cpp	1.165 04/04/20 15:56:19 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_c1_LIRAssembler_sparc.cpp.incl"

#define __ _masm->


//------------------------------------------------------------


void LIR_Assembler::safepoint_nop() {
  _masm->nop();
}


bool LIR_Assembler::is_small_constant(LIR_Opr opr) {
  if (opr->is_constant()) {
    LIR_Const* constant = opr->as_constant_ptr();
    switch (constant->type()) {
    case T_INT: {
      jint value = constant->as_jint();
      return Assembler::is_simm13(value);
    }

    default:
      return false;
    }
  }
  return false;
}


bool LIR_Assembler::is_single_instruction(LIR_Op* op, FrameMap* frame_map) {
  switch (op->code()) {
  case lir_null_check:
    return true;


  case lir_add:
  case lir_ushr:
  case lir_shr:
  case lir_shl:
    // integer shifts and adds are always one instruction
    return op->result_opr()->is_single_cpu();


  case lir_move: {
    LIR_Op1* op1 = op->as_Op1();
    LIR_Opr src = op1->in_opr();
    LIR_Opr dst = op1->result_opr();

    if (src == dst) {
      NEEDS_CLEANUP;
      // this works around a problem where moves with the same src and dst
      // end up in the delay slot and then the assembler swallows the mov
      // since it has no effect and then it complains because the delay slot
      // is empty.  returning false stops the optimizer from putting this in
      // the delay slot
      return false;
    }

    // don't put moves involving oops into the delay slot since the VerifyOops code
    // will make it much larger than a single instruction.
    if (VerifyOops && (op1->type() == T_OBJECT || op1->type() == T_ARRAY)) {
      return false;
    }

    if (src->is_double_word() || dst->is_double_word() || op1->patch_code() != LIR_Op1::patch_none) {
      return false;
    }

    if (dst->is_register()) {
      if (src->is_address() && Assembler::is_simm13(src->as_address_ptr()->disp())) {
        return true;
      } else if (src->is_stack()) {
        Address addr = frame_map->address_for_name(src->single_stack_ix(), false);
        return Assembler::is_simm13(addr.disp());
      }
    }

    if (src->is_register()) {
      if (dst->is_address() && Assembler::is_simm13(dst->as_address_ptr()->disp())) {
        return true;
      } else if (dst->is_stack()) {
        Address addr = frame_map->address_for_name(dst->single_stack_ix(), false);
        return Assembler::is_simm13(addr.disp());
      }
    }

    if ((src->is_register() ||
         (src->is_constant() &&
          LIR_Assembler::is_small_constant(op->as_Op1()->in_opr()))) &&
        dst->is_register()) {
      return true;
    }

    return false;
  }

  default:
    return false;
  }
  ShouldNotReachHere();
}


int LIR_Assembler::initial_frame_size_in_bytes() {
  return round_to(frame_map()->framesize(), 2)  * BytesPerWord;
}


void LIR_Assembler::trace_method_entry(jint con) {
  Unimplemented();
}


// inline cache check: the inline cached class is in G5_inline_cache_reg(G5);
// we fetch the class of the receiver (O0) and compare it with the cached class.
// If they do not match we jump to slow case.
int LIR_Assembler::check_icache() {
  return check_icache(O0, G5_inline_cache_reg);
}


int LIR_Assembler::check_icache(Register receiver, Register ic_klass) {
  assert(receiver == O0 && ic_klass == G5_inline_cache_reg, "must match");
  int offset = __ offset();
  __ inline_cache_check(O0, G5_inline_cache_reg);
  return offset;
}


void LIR_Assembler::emit_osr_entry(IRScope* scope, int number_of_locks, Label* continuation, int osr_bci) { 

  // On-stack-replacement entry sequence (interpreter frame layout described in interpreter_sparc.cpp):
  //
  //   1. Create a new compiled activation.
  //   2. Initialize local variables in the compiled activation.  The expression stack must be empty
  //      at the osr_bci; it is not initialized.
  //   3. Jump to the continuation address in compiled code to resume execution.

  // OSR entry point
  _offsets->_osr_offset = code_offset();

  // Save pointers into the interpreted activation for initializing the compiled activation.
  __ mov(Llocals, G1);

  // Create a frame for the compiled activation.
  __ build_frame(initial_frame_size_in_bytes());

  // Initialize local variables in the compiled activation.
  LocalList locals = *scope->locals();
  BlockBegin* osr_entry = compilation()->hir()->osr_entry();
  const LocalMapping* cached_locals = osr_entry->local_mapping();
  for (int i = 0; i < locals.length(); i++) {
    Local* aLocal = locals[i];
    int index = aLocal->java_index();
    int name = aLocal->local_name();
    RInfo reg = cached_locals != NULL ? cached_locals->get_cache_reg(name, aLocal->type()->tag()) : norinfo;
    if (aLocal->type()->size() == 2) {
      assert(reg.is_illegal(), "we do not cache two-word types..yet");
      // high word of big endian double word
      __ ld(G1, -((index+1) * BytesPerWord), O7);
      __ st(O7, frame_map()->address_for_local_name(name, true, true));
      // low word of big endian double word
      __ ld(G1, -((index+1) * BytesPerWord) + (longSize/2), O7);
      __ st(O7, frame_map()->address_for_local_name(name, true, false));
    } else {
      if ( aLocal->type()->is_object() ) {
        if (reg.is_illegal()) { 
          __ ld_ptr(G1, -index * BytesPerWord, O7);
          __ st_ptr(O7, frame_map()->address_for_local_name(name, false));
        } else {
          __ ld_ptr(G1, -index * BytesPerWord, reg.as_register());
        }
      }
      else {
        if (reg.is_illegal()) { 
          __ ld(G1, -index * BytesPerWord, O7);
          __ st(O7, frame_map()->address_for_local_name(name, false));
        } else if (reg.is_word()) {
          __ ld(G1, -index * BytesPerWord, reg.as_register());
        } else if (reg.is_float()) {
          __ ldf(FloatRegisterImpl::S, G1, -index * BytesPerWord, reg.as_float_reg());
        } else {
          ShouldNotReachHere();
        }
      }
    }
  }
  // Initialize monitors in the compiled activation.
  // Note: interpreter uses BasicObjectLock for monitors while compiler1 uses BasicLock
  {
    assert(frame::interpreter_frame_monitor_size() == BasicObjectLock::size(), "adjust code below");
    int rounded_vm_local_words = ::round_to(frame::interpreter_frame_vm_local_words, WordsPerLong);
    for (int i = 0; i < number_of_locks; i++) {
      Address compiled_monitor_lock_address      = frame_map()->address_for_monitor_lock_index(number_of_locks - i - 1);
      Address compiled_monitor_object_address    = frame_map()->address_for_monitor_object_index(number_of_locks - i - 1);
      Address interpreter_monitor_address = Address(G5, 0, - i * BasicObjectLock::size() * BytesPerWord);
#ifdef ASSERT
      // verify the interpreter's monitor has a non-null object
      {
        Label L;
        load(interpreter_monitor_address, O7, T_OBJECT, NULL, BasicObjectLock::obj_offset_in_bytes());
        __ cmp(G0, O7);
        __ br(Assembler::notEqual, false, Assembler::pt, L);
        __ delayed()->nop();
        __ stop("locked object is NULL");
        __ bind(L);
      }
#endif // ASSERT
      // Copy the lock field into the compiled activation.
      load(interpreter_monitor_address, O7, T_ADDRESS, NULL, BasicObjectLock::lock_offset_in_bytes());
      store(O7, compiled_monitor_lock_address,   T_ADDRESS, NULL);
      load(interpreter_monitor_address, O7, T_OBJECT, NULL, BasicObjectLock::obj_offset_in_bytes());
      store(O7, compiled_monitor_object_address,   T_OBJECT, NULL);
    }
  }
  
  // Continue execution in compiled code.
  __ br(Assembler::always, false, Assembler::pt, *continuation);
  __ delayed()->nop();
}


// Uses G3 and G4
void LIR_Assembler::emit_method_entry(LIR_Emitter* emit, IRScope* scope) { 
  ciMethod* m = compilation()->method();
  
  
  // entry point
  __ align(CodeEntryAlignment);
  _offsets->_ep_offset = __ offset();
  if (needs_icache(m)) {
    // Note: even if this is a final method, we may need IC check
    // as the final method may be one of many methods in the hierarchu
    if (C1Breakpoint)  __ breakpoint_trap();
    check_icache();
  }

  // verified entry point
  _offsets->_vep_offset = __ offset();
  __ bind (_vep_label);
  if (C1Breakpoint) __ breakpoint_trap();

  _offsets->_code_offset = __ offset();
  __ verify_FPU(0, "method_entry");

  // For the native method Object.hashCode(), if the object's header can be read
  // while the object is unlocked and it holds a valid (non-zero) hash code,
  // return the hash value immediately without creating a frame.
  if (InlineObjectHash && m->intrinsic_id() == methodOopDesc::_hash && !compilation()->jvmpi_event_method_enabled()) {
	  CallingConvention* args       = FrameMap::calling_convention(m);
	  ArgumentLocation receiver_loc = args->arg_at(0);
	  assert(args->length() == 1, "Object.hashCode has only one parameter");
	  Register receiver = receiver_loc.is_register_arg() ?
	                            receiver_loc.outgoing_reg_location().as_register() :
	                            noreg;
	  __ fast_ObjectHashCode(receiver, O0);
  }

  // Create an activation frame for this method.  All components of the frame must be recorded in a FrameMap,
  // and only FrameMap::framesize() is used to compute the frame's size.
  __ build_frame(initial_frame_size_in_bytes());

  if (C1TraceMethod) trace_method_entry(0);

  BasicTypeArray* sig_types = FrameMap::signature_type_array_for(m);
  CallingConvention* args = FrameMap::calling_convention(m);

  if (!m->is_native()) {
    setup_locals(args, sig_types);
  }

  if (m->is_synchronized() && GenerateSynchronizationCode) {
    Register obj; // object to lock
    if (m->is_static()) {
      obj = O0;
      ciObject2reg(m->holder()->java_mirror(), O0);
    } else {
      ArgumentLocation receiver = args->arg_at(0);
      if (receiver.is_register_arg()) {
        obj = receiver.incoming_reg_location().as_register();
      } else {
        obj = O0;
        load(receiver.incoming_stack_location(), obj, T_OBJECT);
      }
    }
    Register lock_reg = G1;
    // The setup_locals() call above ensures that parameter locations will be found correctly for debug info
    // generation at the SynchronizationEntryBCI (if it had not been called, then arguments passed in registers
    // would not be found.)  Since caller-save registers are not used for caching locals, it is okay to use
    // O0 and G1 for the object and lock in the monitorenter stub.  This NEEDS_CLEANUP (to ensure the safety
    // of using these registers) when deoptimization becomes LIR-based.
    CodeEmitInfo* info = m->is_native() ? new CodeEmitInfo(scope, SynchronizationEntryBCI, NULL)
                                        : new CodeEmitInfo(emit , SynchronizationEntryBCI, NULL, scope->start()->state(), NULL);
    monitorenter(obj, lock_reg, G4, G3, 0, info); // sync. slot is 0, however should really use a function here
  }

  if (compilation()->jvmpi_event_method_entry_enabled() || compilation()->jvmpi_event_method_entry2_enabled()) {
    // we need the map for arguments only, therefore we use SynchronizationEntryBCI,
    // although there may be no synchronization in this method
    CodeEmitInfo info(scope, SynchronizationEntryBCI, NULL);
    jvmpi_method_enter(&info);
  }
}

void LIR_Assembler::monitorenter(Register obj_reg, Register lock_reg, Register hdr, Register scratch, int monitor_no, CodeEmitInfo* info) {
  if (!GenerateSynchronizationCode) return;
  Address mon_addr = frame_map()->address_for_monitor_lock_index(monitor_no);
  Register reg = mon_addr.base();
  int offset = mon_addr.disp();
  // compute pointer to BasicLock
  if (mon_addr.is_simm13()) {
    __ add(reg, offset, lock_reg);
  } else {
    __ set(offset, lock_reg);
    __ add(reg, lock_reg, lock_reg);
  }
  __ verify_oop(obj_reg);

  // lock object
  MonitorAccessStub* slow_case = new MonitorEnterStub(as_RInfo(obj_reg), as_RInfo(lock_reg), info);
  _slow_case_stubs->append(slow_case);
  if (UseFastLocking) {
    // try inlined fast locking first, revert to slow locking if it fails
    // note: lock_reg points to the displaced header since the displaced header offset is 0!
    assert(BasicLock::displaced_header_offset_in_bytes() == 0, "lock_reg must point to the displaced header");
    add_debug_info_for_null_check_here(info);
    __ lock_object(hdr, obj_reg, lock_reg, scratch, *slow_case->entry());
  } else {         
    // always do slow locking
    // note: the slow locking code could be inlined here, however if we use
    //       slow locking, speed doesn't matter anyway and this solution is
    //       simpler and requires less duplicated code - additionally, the
    //       slow locking code is the same in either case which simplifies
    //       debugging
    __ br(Assembler::always, false, Assembler::pt, *slow_case->entry());
    __ delayed()->nop();
  }                
  // done          
  __ bind(*slow_case->continuation());
}


void LIR_Assembler::save_native_fp_result(BasicType return_type, Address float_spill_addr) {
  switch (return_type) {
    case T_DOUBLE: __ stf(FloatRegisterImpl::S, F0, float_spill_addr, BytesPerWord);
                   __ stf(FloatRegisterImpl::S, F1, float_spill_addr); break;
    case T_FLOAT : __ stf(FloatRegisterImpl::S, F0, float_spill_addr); break;
    default      : /* do nothing */                                    break;
  }
}


void LIR_Assembler::restore_native_fp_result(BasicType return_type, Address float_spill_addr) {
  switch (return_type) {
    case T_DOUBLE: __ ldf(FloatRegisterImpl::S, float_spill_addr, F0, BytesPerWord);
                   __ ldf(FloatRegisterImpl::S, float_spill_addr, F1); break;
    case T_FLOAT : __ ldf(FloatRegisterImpl::S, float_spill_addr, F0); break;
    default      : /* do nothing */                                    break;
  }
}


void LIR_Assembler::emit_native_call (address native_entry, CodeEmitInfo* info) {
  ciMethod* method = compilation()->method();
  assert (method->is_native(), "Method must be native");
  
  // Pass JNIEnv
  __ verify_thread();
  __ add(G2_thread, in_bytes(JavaThread::jni_environment_offset()), O0);
  int jni_offset = 1;

  // Store a static method holder's mirror in this frame and pass its address as a handle.
  // It is known, and will be reflected in oop maps, that spill slot 0 is used for the mirror.
  if (method->is_static()) {
    Address mirror_addr = frame_map()->address_for_spill_index(0, false);
    ciObject2reg(method->holder()->java_mirror(), L0);
    Register reg = mirror_addr.base();
    int offset = mirror_addr.disp();
    if (mirror_addr.is_simm13()) {
      __ add(reg, offset, O1);
    } else {
      __ set(offset, O1);
      __ add(reg, O1, O1);
    }
    __ st_ptr(L0, mirror_addr);
    jni_offset++;
  }

  // Pass the method's parameters to its native implementation.
  BasicTypeArray* sig_types = FrameMap::signature_type_array_for(method);
  CallingConvention* args = FrameMap::calling_convention(method->is_static(), *sig_types);

  int java_arg_index = 0;
  while (java_arg_index < args->length()) {
    BasicType arg_type = sig_types->at(java_arg_index);
    switch (arg_type) {
    case T_OBJECT: // fall through
    case T_ARRAY:
      {
        Argument jni_arg(jni_offset, false);     // Assembler abstraction; obeys SPARC ABI
        ArgumentLocation java_arg = args->arg_at(java_arg_index); // C1 abstraction for method argument
        if (java_arg.is_register_arg()) {
          // Flush this argument to its stack location, which is used for a handle.
          Register java_arg_reg = java_arg.incoming_reg_location().as_register();
          Address java_arg_addr = frame_map()->address_for_local_name(FrameMap::name_for_argument(java_arg_index), false);
          store(java_arg_reg, java_arg_addr, T_OBJECT);
          pass_oop_to_native(java_arg_index, java_arg_addr, jni_arg);
        } else {
          assert(java_arg.is_stack_arg(), "invalid mapping of args");
          Address java_arg_addr = java_arg.incoming_stack_location();
          pass_oop_to_native(java_arg_index, java_arg_addr, jni_arg);
        }
      }
      break;
    case T_INT:       // fall through
    case T_BOOLEAN:   // fall through
    case T_CHAR:      // fall through
    case T_BYTE:      // fall through
    case T_SHORT:     // fall through
#ifndef _LP64
    case T_FLOAT:
#endif
      {
        Argument jni_arg(jni_offset, false);           // Assembler abstraction; obeys SPARC ABI
        ArgumentLocation java_arg = args->arg_at(java_arg_index); // C1 abstraction for method argument
        if (java_arg.is_register_arg()) {
          Register java_arg_reg = java_arg.incoming_reg_location().as_register();
          __ store_argument(java_arg_reg, jni_arg);
        } else {
          assert(java_arg.is_stack_arg(), "invalid mapping of args");
          Address java_arg_addr = java_arg.incoming_stack_location();
          if (jni_arg.is_register()) {
            load(java_arg_addr, jni_arg.as_register(), T_INT);
          } else {
            load(java_arg_addr, G3_scratch, T_INT);
            __ store_argument(G3_scratch, jni_arg);
          }
        }
      }
      break;
//
// 64 bit Sparc ABI passes 
//   Float arguments in F1, F3, F5 ...
//   Double arguments in D0, D2, D4 ...
//   LONG arguments in first available 64 bit 
//        integer registers O0 etc.
//   All other types are signed extended into 64 bit O registers
//   or sign extended on the 64 bit stack.
//
#ifdef _LP64
    case T_FLOAT:
      {
        FloatRegister  Rtmp = F0; 
        Argument jni_arg(jni_offset, false);           // Assembler abstraction; obeys SPARC ABI
        ArgumentLocation java_arg = args->arg_at(java_arg_index); // C1 abstraction for method argument
        if (java_arg.is_register_arg()) {
    assert(0, "Float as java arg not supported");
        } else {
          assert(java_arg.is_stack_arg(), "invalid mapping of args");
          Address java_arg_addr = java_arg.incoming_stack_location();
          load(java_arg_addr, Rtmp, T_FLOAT);
          __ store_float_argument(Rtmp, jni_arg);
        }
      }
      break;
    case T_DOUBLE:
      {
        FloatRegister  Rtmp = F32;
        Argument jni_arg(jni_offset, false);           // Assembler abstraction; obeys SPARC ABI
        ArgumentLocation java_arg = args->arg_at(java_arg_index); // C1 abstraction for method argument
        if (java_arg.is_register_arg()) {
    assert(0, "Double as java arg not supported");
        } else {
          assert(java_arg.is_stack_arg(), "invalid mapping of args");
          Address java_arg_addr = java_arg.incoming_stack_location();
          load(java_arg_addr, Rtmp, T_DOUBLE);
          __ store_double_argument(Rtmp, jni_arg);
        }
      }
      break;
    case T_LONG:    // fall through
      {
        ArgumentLocation java_arg = args->arg_at(java_arg_index); 
        Argument jni_arg(jni_offset, false);           // Assembler abstraction; obeys SPARC ABI
        Address java_arg_addr = java_arg.incoming_stack_location();

        // Load an object so we generate a 64 bit load.  Longs are
        // packed into one stack slot.
        if (jni_arg.is_register()) {
          load(java_arg_addr, jni_arg.as_register(), T_OBJECT);
        } else {
          load(java_arg_addr, G3_scratch, T_OBJECT);
          __ store_argument(G3_scratch, jni_arg);
        }
      }
      break;
#else
    case T_LONG:    // fall through
    case T_DOUBLE:
      {
        ArgumentLocation java_arg_hi = args->arg_at(java_arg_index+1);
        ArgumentLocation java_arg_lo = args->arg_at(java_arg_index+0);
        Argument jni_arg_hi(jni_offset+1, false);           // Assembler abstraction; obeys SPARC ABI
        Argument jni_arg_lo(jni_offset, false);           // Assembler abstraction; obeys SPARC ABI
        Address java_arg_addr_hi = java_arg_hi.incoming_stack_location();
        Address java_arg_addr_lo = java_arg_lo.incoming_stack_location();

        if (jni_arg_hi.is_register()) {
          load(java_arg_addr_hi, jni_arg_hi.as_register(), T_INT);
        } else {
          load(java_arg_addr_hi, G3_scratch, T_INT);
          __ store_argument(G3_scratch, jni_arg_hi);
        }
        if (jni_arg_lo.is_register()) {
          load(java_arg_addr_lo, jni_arg_lo.as_register(), T_INT);
        } else {
          load(java_arg_addr_lo, G3_scratch, T_INT);
          __ store_argument(G3_scratch, jni_arg_lo);
        }
      }
      break;
#endif
    default:
      ShouldNotReachHere();
      break;
    }
    java_arg_index +=type2size[arg_type];  // either one word or two words
#ifdef _LP64
    jni_offset++;                        // Each jni argument takes only one register or stack slot
#else
    jni_offset +=type2size[arg_type];      // either one word or two words
#endif
  }

  // Reset local handle block.
  __ ld_ptr(G2_thread, in_bytes(JavaThread::active_handles_offset()), G3_scratch);

  // Save the last Java frame - SP and a PC located somewhere within the method - before
  // entering native.  To get a PC, "call" the next instruction. If this frame is scanned by
  // GC an oop map is needed at this pc and not the real call since the pc to do the oop
  // map lookup will be from last_Java_pc and not the usual O7 in the callee. This oop map
  // will reflect this method's formal parameters and, if it is static, its holder's mirror.
  //
  { Label L;
    add_call_info(__ offset(),info);
    __ call(L, relocInfo::none);  // No relocation for call to pc+0x8
    // Reset local handle block.
    __ delayed()->st_ptr(G0, G3_scratch, JNIHandleBlock::top_offset_in_bytes());

    __ bind(L);
  }

  __ set_last_Java_frame(SP, O7);

  // Transition from _thread_in_Java to _thread_in_native.
  __ set(_thread_in_native, G3_scratch);
  __ st(G3_scratch, G2_thread, in_bytes(JavaThread::thread_state_offset()));

  // We flush the current window just so that there is a valid stack copy
  // the fact that the current window becomes active again instantly is 
  // not a problem there is nothing live in it.

  __ save_frame(0);
  __ flush_windows();
  __ restore();

  // mark windows as flushed
  __ set(JavaFrameAnchor::flushed, G3_scratch);

  Address flags(G2_thread,
		0,
		in_bytes(JavaThread::frame_anchor_offset()) + in_bytes(JavaFrameAnchor::flags_offset()));
  __ st(G3_scratch, flags);

  // Save the current thread and call the native function.  An oop map is needed here
  // in case this thread is stopped at the return address.
#ifdef _LP64
  Address dest(O7, native_entry);
  __ relocate(relocInfo::runtime_call_type);
  __ jumpl_to(dest, O7);
#else
  __ call(native_entry, relocInfo::runtime_call_type);
#endif
  __ delayed()->mov(G2_thread, L0);

}

void LIR_Assembler::emit_native_method_exit (CodeEmitInfo* info) {
  ciMethod* method = compilation()->method();
  assert (method->is_native(), "method must be native");
  
  // Process the native function result: An integer is converted to its Java type and moved to
  // register I0 (and I1 for long) in preparation for restore(), and for a floating-point
  // result, offsets from the frame pointer are computed for saving the result around VM calls.
  Address float_spill_addr;
  int float_spill_index = method->is_static() ? 1 : 0; // for method holder's mirror
  BasicType return_type = method->return_type()->basic_type();
  switch (return_type) {
    case T_BOOLEAN: __ subcc(G0, O0, G0); __ addc(G0, 0, I0); break; // !0 => true; 0 => false
    case T_CHAR   : __ sll(O0, 16, O0); __ srl(O0, 16, I0);   break; // clear unused bits
    case T_BYTE   : __ sll(O0, 24, O0); __ sra(O0, 24, I0);   break; // sign-extend
    case T_SHORT  : __ sll(O0, 16, O0); __ sra(O0, 16, I0);   break; // sign-extend
    case T_ARRAY  :                                           // fall through (result is a handle)
    case T_OBJECT :                                           // fall through (result is a handle)
    case T_INT    : __ mov(O0, I0);                           break;
    case T_LONG   :
#ifdef _LP64   
        // Unpack O0 -> O0, O1
        unpack64(O0);
#endif
        __ mov(O0, I0); __ mov(O1, I1);           break;
    case T_VOID   :                                           break;
    case T_DOUBLE :
      float_spill_addr = frame_map()->address_for_spill_index(float_spill_index, true,  true ); break;
    case T_FLOAT  :
      float_spill_addr = frame_map()->address_for_spill_index(float_spill_index, false, false); break;
    default       : ShouldNotReachHere();
  }

  // Switch thread to "native transition" state before reading the synchronization state.
  // This additional state is necessary because reading and testing the synchronization
  // state is not atomic w.r.t. GC, as this scenario demonstrates:
  //     Java thread A, in _thread_in_native state, loads _not_synchronized and is preempted.
  //     VM thread changes sync state to synchronizing and suspends threads for GC.
  //     Thread A is resumed to finish this native method, but doesn't block here since it
  //     didn't see any synchronization in progress, and escapes.
  // (thread was saved in L0 before the call to native code.)

  Register Lthread = L0;
  { 
    Label no_block;

    __ set(_thread_in_native_trans, G3_scratch);
    __ st(G3_scratch, Lthread, in_bytes(JavaThread::thread_state_offset()));

    if (os::is_MP) {
      membar(); // Force this write out before the read below
    }

    Address sync_state(G3_scratch, SafepointSynchronize::address_of_state());
    __ load_contents(sync_state, G3_scratch);
    __ cmp(G3_scratch, SafepointSynchronize::_not_synchronized);

    Label L;
    Address suspend_state(Lthread, 0, in_bytes(JavaThread::suspend_flags_offset()));
    __ br(Assembler::notEqual, false, Assembler::pn, L);
    __ delayed()->
       ld(suspend_state, G3_scratch);
    __ cmp(G3_scratch, 0);
    __ br(Assembler::equal, false, Assembler::pt, no_block);
    __ delayed()->nop();
    __ bind(L);

    // Before blocking, preserve a floating-point result; an integer result is safe in I0/I1
    // use a leaf call to leave the last_Java_frame setup undisturbed.
    save_native_fp_result(return_type, float_spill_addr);
    __ call_VM_leaf(L7_thread_cache,
		    CAST_FROM_FN_PTR(address, JavaThread::check_safepoint_and_suspend_for_native_trans),
		    Lthread);

    // Restore floating-point result
    restore_native_fp_result(return_type, float_spill_addr);
    __ bind(no_block);
  }
  // Restore global thread pointer
  __ mov(Lthread, G2_thread);

  // Change state to _thread_in_Java.
  __ set(_thread_in_Java, G3_scratch);
  __ st(G3_scratch, G2_thread, in_bytes(JavaThread::thread_state_offset()));

  // Clear "last Java frame" SP and PC.
  __ verify_thread(); // G2_thread must be correct
  __ reset_last_Java_frame();

  Label no_reguard;
  __ ld_ptr(G2_thread, in_bytes(JavaThread::stack_guard_state_offset()), G3_scratch);
  __ cmp(G3_scratch, JavaThread::stack_guard_yellow_disabled);
  __ br(Assembler::notEqual, false, Assembler::pt, no_reguard);
  __ delayed()->nop();
  
  __ call(CAST_FROM_FN_PTR(address, SharedRuntime::reguard_yellow_pages));
  __ delayed()->nop();

  // Restore global thread pointer
  __ mov(Lthread, G2_thread);
  __ bind(no_reguard);

  // JVMDI jframeIDs are invalidated on exit from native method.
  // JVMTI does not use jframeIDs, this whole mechanism must be removed when JVMDI is removed.
  if (JvmtiExport::must_purge_jvmdi_frames_on_native_exit()) { 
    save_native_fp_result(return_type, float_spill_addr);
    __ call_VM_leaf(noreg, CAST_FROM_FN_PTR(address, JvmtiExport::thread_leaving_native_code));
    restore_native_fp_result(return_type, float_spill_addr);
  }

  // Handle exception (will unlock if necessary)
  { Label L;
    __ ld(G2_thread, in_bytes(Thread::pending_exception_offset()), G3_scratch);
    __ tst(G3_scratch);
    __ br(Assembler::equal, false, Assembler::pt, L);
    __ delayed()->nop();
    assert(StubRoutines::forward_exception_entry() != NULL, "must be created");
    __ call(StubRoutines::forward_exception_entry(), relocInfo::runtime_call_type);
    __ delayed()->nop();
    __ bind(L);
  }

  // Unlock
  if (method->is_synchronized()) {
    // Preserve any floating-point result through the monitor exit code.
    // An integer result is safe in I0/I1 (even an oop, since monitorexit will not block).
    save_native_fp_result(return_type, float_spill_addr);

    Register obj_reg  = O0;
    Register lock_reg = O1;
    monitorexit(obj_reg, lock_reg, G4, 0);

    restore_native_fp_result(return_type, float_spill_addr);
  }

  // Unpack oop result
  if (return_type == T_OBJECT || return_type == T_ARRAY) {
      Label L;
      __ addcc(G0, I0, G0);
      __ brx(Assembler::notZero, true, Assembler::pt, L);
      __ delayed()->ld_ptr(I0, 0, I0);
      __ mov(G0, I0);
      __ bind(L);
      __ verify_oop(I0);
  }

  // Tell jvmpi about this method exit
  if (compilation()->jvmpi_event_method_exit_enabled()) {
    jvmpi_method_exit(compilation()->method(), return_type == T_OBJECT || return_type == T_ARRAY);
  }

  // Return
  __ ret();
  __ delayed()->restore();
}


// Optimized Library calls
// This is the fast version of java.lang.String.compare; it has not
// OSR-entry and therefore, we generate a slow version for OSR's
void LIR_Assembler::emit_string_compare(IRScope* scope) {
  assert(OptimizeLibraryCalls, "should not call this");
  Label Ldone;

  Register result = I0;
  {
    // Get a pointer to the first character of string0 in tmp0 and get string0.count in str0
    // Get a pointer to the first character of string1 in tmp1 and get string1.count in str1
    // Also, get string0.count-string1.count in o7 and get the condition code set
    // Note: some instructions have been hoisted for better instruction scheduling

    Register str0 = I0;
    Register str1 = I1;
    Register tmp0 = L0;
    Register tmp1 = L1;
    Register tmp2 = L2;

    CallingConvention* args = FrameMap::calling_convention(compilation()->method());
    assert(args->length() == 2, "String::compareTo takes only two arguments");

    ArgumentLocation string0 = args->arg_at(0);
    ArgumentLocation string1 = args->arg_at(1);
    if (string0.is_register_arg()) {
      assert(string0.incoming_reg_location().as_register() == str0, "should be in I0");
    } else {
      load(string0.incoming_stack_location(), str0, T_OBJECT);
    }
    if (string1.is_register_arg()) {
      assert(string1.incoming_reg_location().as_register() == str1, "should be in I0");
    } else {
      load(string1.incoming_stack_location(), str1, T_OBJECT);
    }

    int  value_offset = java_lang_String:: value_offset_in_bytes(); // char array
    int offset_offset = java_lang_String::offset_offset_in_bytes(); // first character position
    int  count_offset = java_lang_String:: count_offset_in_bytes();

    __ ld_ptr(Address(str0, 0,  value_offset), tmp0);
    __ ld(Address(str0, 0, offset_offset), tmp2);
    __ add(tmp0, arrayOopDesc::base_offset_in_bytes(T_CHAR), tmp0);
    __ ld(Address(str0, 0, count_offset), str0);
    __ sll(tmp2, exact_log2(sizeof(jchar)), tmp2);

    // str1 may be null
    CodeEmitInfo* info = new CodeEmitInfo(scope, 0, NULL);
    add_debug_info_for_null_check_here(info);

    __ ld_ptr(Address(str1, 0,  value_offset), tmp1);
    __ add(tmp0, tmp2, tmp0);

    __ ld(Address(str1, 0, offset_offset), tmp2);
    __ add(tmp1, arrayOopDesc::base_offset_in_bytes(T_CHAR), tmp1);
    __ ld(Address(str1, 0, count_offset), str1);
    __ sll(tmp2, exact_log2(sizeof(jchar)), tmp2);
    __ subcc(str0, str1, O7);
    __ add(tmp1, tmp2, tmp1);
  }

  {
    // Compute the minimum of the string lengths, scale it and store it in limit
    Register count0 = I0;
    Register count1 = I1;
    Register limit  = L3;

    Label Lskip;
    __ sll(count0, exact_log2(sizeof(jchar)), limit);             // string0 is shorter
    __ br(Assembler::greater, true, Assembler::pt, Lskip);
    __ delayed()->sll(count1, exact_log2(sizeof(jchar)), limit);  // string1 is shorter
    __ bind(Lskip);

    // If either string is empty (or both of them) the result is the difference in lengths
    __ cmp(limit, 0);
    __ br(Assembler::equal, true, Assembler::pn, Ldone);
    __ delayed()->mov(O7, result);  // result is difference in lengths
  }

  {
    // Neither string is empty
    Label Lloop;

    Register base0 = L0;
    Register base1 = L1;
    Register chr0  = I0;
    Register chr1  = I1;
    Register limit = L3;

    // Shift base0 and base1 to the end of the arrays, negate limit
    __ add(base0, limit, base0);
    __ add(base1, limit, base1);
    __ neg(limit);  // limit = -min{string0.count, strin1.count}

    __ lduh(base0, limit, chr0);
    __ bind(Lloop);
    __ lduh(base1, limit, chr1);
    __ subcc(chr0, chr1, chr0);
    __ br(Assembler::notZero, false, Assembler::pn, Ldone);
    assert(chr0 == result, "result must be pre-placed");
    __ delayed()->inccc(limit, sizeof(jchar));
    __ br(Assembler::notZero, true, Assembler::pt, Lloop);
    __ delayed()->lduh(base0, limit, chr0);
  }

  // If strings are equal up to min length, return the length difference.
  __ mov(O7, result);

  // Otherwise, return the difference between the first mismatched chars.
  __ bind(Ldone);
  return_op(norinfo, false);
}


void LIR_Assembler::emit_triglib(ciMethod::IntrinsicId trig_id) {
  assert(OptimizeLibraryCalls, "should not call this");

  CallingConvention* args = FrameMap::calling_convention(compilation()->method());
  assert(args->length() == 2, "Triglib functions (sine/cosine) take one double argument.");

  // SharedRuntime::dsin/dcos expect their argument in O0/O1, load it from stack
  ArgumentLocation x = args->arg_at(0);
  assert(x.is_stack_arg(), "double values are passed on the stack");
#ifdef _LP64
  // Load the complete Double from the lower addressed stack location
  load(frame_map()->address_for_local_name(FrameMap::name_for_argument(0), true, true), FrameMap::_F0_double_RInfo.as_double_reg(), T_DOUBLE);
#else
  load(frame_map()->address_for_local_name(FrameMap::name_for_argument(0), true, false), FrameMap::_O0_O1_RInfo.as_register_lo(), T_INT);
  load(frame_map()->address_for_local_name(FrameMap::name_for_argument(0), true, true ), FrameMap::_O0_O1_RInfo.as_register_hi(), T_INT);    
#endif

  address runtime_entry = NULL;
  switch (trig_id) {
  case methodOopDesc::_dsin:  runtime_entry = CAST_FROM_FN_PTR(address, SharedRuntime::dsin); break;
  case methodOopDesc::_dcos:  runtime_entry = CAST_FROM_FN_PTR(address, SharedRuntime::dcos); break;
  default: ShouldNotReachHere(); break;
  }

  Register thread_cache = L0;  // preserve G2_thread around the C call
  __ call_VM_leaf(thread_cache, runtime_entry);

  return_op(norinfo, false);
}

void LIR_Assembler::pass_oop_to_native(int java_arg_index, Address java_arg_addr, Argument jni_arg) {
  // If the outgoing oop is null, pass null, otherwise pass its handle.  For a nonstatic method, the
  // receiver is known to be nonnull, since the caller must check for a NullPointerException.)
  bool check_for_null = compilation()->method()->is_static() || java_arg_index != 0;
  Register jni_reg = jni_arg.is_register() ? jni_arg.as_register() : G4_scratch;
  if (check_for_null) {
    Label L;
    __ ld_ptr(java_arg_addr, G3_scratch);
    __ tst(G3_scratch);
    __ brx(Assembler::notZero, true, Assembler::pt, L);
    __ delayed()->add(java_arg_addr, jni_reg);
    __ mov(G0, jni_reg);
    __ bind(L);
  } else {
    __ add(java_arg_addr, jni_reg);
  }
  if (!jni_arg.is_register()) {
    __ st_ptr(jni_reg, jni_arg.as_address());
  }
}

// --------------------------------------------------------------------------------------------

void LIR_Assembler::setup_locals_at_entry() {
  BasicTypeArray* sig_types = FrameMap::signature_type_array_for(compilation()->method());
  CallingConvention* args = FrameMap::calling_convention(compilation()->method());
  setup_locals(args, sig_types);
}


class DependenceEdge;
define_array(DependenceEdgeArray, DependenceEdge*)
define_stack(DependenceEdgeList, DependenceEdgeArray)

//
// This class represent a register/register move and is used
// DependenceBreak below to detect cycles in the stores and generate
// correct orderings for emitting the stores.
//

class DependenceEdge: public ResourceObj {
  friend class DependenceBreaker;
 private:
  RInfo            _src;
  RInfo            _dst;
  DependenceEdge*  _next;
  DependenceEdge*  _prev;
  int              _processed;

 public:
  DependenceEdge(RInfo src, RInfo dst):
      _src(src)
    , _dst(dst)
    , _next(NULL)
    , _prev(NULL)
    , _processed(false) {
  }

  RInfo src() const                { return _src; }
  int src_id() const               { return _src.as_register()->encoding(); }
  RInfo dst() const                { return _dst; }
  void set_dst(RInfo dst)          { _dst = dst; }
  int dst_id() const               { return _dst.as_register()->encoding(); }
  DependenceEdge* next() const     { return _next; }
  DependenceEdge* prev() const     { return _prev; }
  void set_processed()             { _processed = true; }
  bool is_processed() const        { return _processed; }

  // insert node after this one, breaking any reverse links
  void break_cycle(DependenceEdge* node) {
    // break the reverse link.  
    if (_next != NULL) {
      _next->_prev = NULL;
    }
    _next = node;
    node->_prev = this;
  }

  void link(DependenceEdgeList& killer) {
    // link this store in front the store that it depends on
    DependenceEdge* n = killer.at_grow(src_id(), NULL);
    if (n != NULL) {
      assert(_next == NULL && n->_prev == NULL, "shouldn't have been set yet");
      _next = n;
      n->_prev = this;
    }
  }

#ifndef PRODUCT
  void print() {
    tty->print("edge ");
    src().print();
    tty->print(" ");
    dst().print();
  }
#endif

};


class DependenceBreaker: public StackObj {
 private:
  DependenceEdgeList killer;
  DependenceEdgeList edges;

 public:
  DependenceBreaker() { }

  // create a dependence edge and record this one as the killer of dst register
  void add_edge(RInfo src, RInfo dst) {
    DependenceEdge* s = new DependenceEdge(src, dst);
    killer.at_put_grow(s->dst_id(), s, NULL);
    edges.append(s);
  }

  // walk the edges breaking cycles.  Insert the chains
  // into a list and return it.  The result list can be
  // walked in order to produce the proper set of loads
  DependenceEdgeList* get_store_order(RInfo temp_register) {
    // create dependence chains between loads and stores
    for (int i = 0; i < edges.length(); i++) {
      edges.at(i)->link(killer);
    }

    // at this point, all the dependencies are chained together
    // in a doubly linked list.  Processing it backwards finds
    // the beginning of the chain, forwards finds the end.  If there's
    // a cycle it can be broken at any point,  so pick an edge and walk
    // backward until
    DependenceEdgeList* stores = new DependenceEdgeList();
    for (int e = 0; e < edges.length(); e++) {
      DependenceEdge* s = edges.at(e);
      if (!s->is_processed()) {
        DependenceEdge* start = s;
        // search for the beginning of the chain or cycle
        while (start->prev() != NULL && start->prev() != s) {
          start = start->prev();
        }
        if (start->prev() == s) {
          // create a new store following the last store
          // to move from the temp_register to the original
          DependenceEdge* new_store = new DependenceEdge(temp_register, start->dst());
          assert(killer.at_grow(new_store->src_id(), NULL) == NULL,
                 "make sure temp isn't in the registers that are killed");

          // break the cycle of links and insert new_store at the end
          start->prev()->break_cycle(new_store);

          // change the original store to save it's value in the temp.
          start->set_dst(temp_register);
        }
        // walk the chain forward inserting to store list
        while (start != NULL) {
          stores->append(start);
          start->set_processed();
          start = start->next();
        }
      }
    }
    return stores;
  }

#ifndef PRODUCT
  static void test() {
    static int tested = 0;
    if (tested == 0) {
      tested = 1;
      
      DependenceBreaker breaker;
      breaker.add_edge(as_RInfo(I0), as_RInfo(I1));
      breaker.add_edge(as_RInfo(I5), as_RInfo(I4));
      breaker.add_edge(as_RInfo(I1), as_RInfo(I2));
      breaker.add_edge(as_RInfo(I3), as_RInfo(I0));
      breaker.add_edge(as_RInfo(I4), as_RInfo(I5));
      breaker.add_edge(as_RInfo(I2), as_RInfo(I3));
      DependenceEdgeList* stores = breaker.get_store_order(as_RInfo(G1));
      for (int i = 0; i < stores->length(); i++) {
        stores->at(i)->print();
        if (i + 1 < stores->length()) {
          tty->print(", ");
        }
      }
      tty->cr();
    }
  }
#endif

};

// Note: all argument locals are at the same location : verify it
// Note: this is used when passing arguments in registers and when caching locals
// Situations:
//    - parameter on stack, local is cached -> move from stack location to register
void LIR_Assembler::setup_locals(CallingConvention* args, BasicTypeArray* sig_types) {
  const LocalMapping* cached_locals = current_block() != NULL ? current_block()->local_mapping() : NULL;
  DependenceBreaker dependences;

  // first move all incoming register arguments that aren't
  // cached to the stack to free up their registers since
  // other things might be cached in them.
  int n;
  for (n = 0; n < args->length(); n++) {
    ArgumentLocation loc = args->arg_at(n); // this is were parameter n goes according to the calling convention
    RInfo cache_reg = cached_locals != NULL ? cached_locals->get_cache_reg(n, as_ValueType(sig_types->at(n))->tag()) : norinfo; // where parameter n is cached
    if (loc.is_register_arg()) {
      if (cache_reg.is_illegal()) {
        // incoming argument in registers, but should be stored in stack local
        reg2local(n, loc.incoming_reg_location(), sig_types->at(n));
      } else {
        if (!loc.incoming_reg_location().is_same(cache_reg)) {
          // register argument that isn't already in the right register.
          // delay this store to make sure that the register moves are
          // emitted in the right order.
          dependences.add_edge(loc.incoming_reg_location(), cache_reg);
        }
      }
    }
  }

  // emit all the register/registers moves so that no
  // register is killed before being used
  DependenceEdgeList* stores = dependences.get_store_order(as_RInfo(G1));
  if (stores != NULL) {
    for (int i = 0; i < stores->length(); i++) {
      DependenceEdge* s = stores->at(i);
      reg2reg(s->src(), s->dst());
    }
  }
  
  // finally move arguments passed in memory into the proper registers
  for (n = 0; n < args->length(); n++) {
    ArgumentLocation loc = args->arg_at(n); // this is were parameter n goes according to the calling convention
    RInfo cache_reg = cached_locals != NULL ? cached_locals->get_cache_reg(n, as_ValueType(sig_types->at(n))->tag()) : norinfo; // where parameter n is cached
    if (loc.is_stack_arg() && cache_reg.is_valid()) {
      // it is being passed on the stack, but we want it in a register.
      local2reg(cache_reg, n, sig_types->at(n));
      if (C1InvalidateCachedOopLocation) {
        __ sub (G0, 1, G1);
        // store -1 in local location making that location invalid instead of
        // having a stale copy there
        __ st (G1, frame_map()->address_for_local_name(FrameMap::name_for_argument(n), false)); 
      }
    }
  }
}


void LIR_Assembler::load_receiver_reg(Register reg) {
  load(frame_map()->address_for_local_name(FrameMap::name_for_argument(0), false), reg, T_OBJECT);
}


void LIR_Assembler::monitorexit(Register obj_reg, Register lock_reg, Register hdr, int monitor_no) {
  if (!GenerateSynchronizationCode) return;

  Address mon_addr = frame_map()->address_for_monitor_lock_index(monitor_no);
  Register reg = mon_addr.base();
  int offset = mon_addr.disp();
  // compute pointer to BasicLock
  if (mon_addr.is_simm13()) {
    __ add(reg, offset, lock_reg);
  }
  else {
    __ set(offset, lock_reg);
    __ add(reg, lock_reg, lock_reg);
  }
  // unlock object
  MonitorAccessStub* slow_case = new MonitorExitStub(as_RInfo(lock_reg), UseFastLocking, monitor_no);
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
    __ br(Assembler::always, false, Assembler::pt, *slow_case->entry());
    __ delayed()->nop();
  }                
  // done          
  __ bind(*slow_case->continuation());
}


int LIR_Assembler::emit_exception_handler() { 
  // if the last instruction is a call (typically to do a throw which
  // is coming at the end after block reordering) the return address
  // must still point into the code area in order to avoid assertion
  // failures when searching for the corresponding bci => add a nop
  // (was bug 5/14/1999 - gri)
  ciMethod* method = compilation()->method();
  __ nop();
  int offset = code_offset();
  __ bind (_throw_entry_label);
  // verify_frame_size();
  __ exception_handler(compilation()->has_exception_handlers(), initial_frame_size_in_bytes());
  __ bind (_unwind_entry_label);
  if (method->is_synchronized()) {
    Register obj_reg = I0;

    // preserve the exception object but make sure the cached receiver is not destroyed
    Register saved_exception = L0;
    __ mov(Oexception, saved_exception); // save exception object
    monitorexit(obj_reg, L1, G4, 0);
    __ mov(saved_exception, Oexception); // restore exception object
  }

  // Move Oexception to its I register counterpart so it will be preserved by the jvmpi_method_exit_id
  // stub and end up in Oexception after the restore that is done by the unwind_exception_id stub.
  __ mov(Oexception, Oexception->after_save());

  if (compilation()->jvmpi_event_method_exit_enabled()) {
    jvmpi_method_exit(method, /*result_is_oop =*/ true);
  }

  __ call(Runtime1::entry_for(Runtime1::unwind_exception_id), relocInfo::runtime_call_type);
  __ delayed()->nop();
  debug_only(__ stop("should have gone to the caller");)

  // deoptimization will patch code into the exception handler to make up for
  // the fact that a branch may not be able to reach the deopt blob. Make sure
  // we have enough instructions in the exception area.

  for (int i = code_offset(); 
	   i < offset + nmethod::size_of_exception_handler(); 
	   i += BytesPerInstWord) {
    __ nop();
  }

  return offset;
}


void LIR_Assembler::emit_constants() { 
  _const_table.emit_entries(masm(), false);
}


void LIR_Assembler::jvmpi_method_exit(ciMethod* method, bool result_is_oop) {
  assert(compilation()->jvmpi_event_method_exit_enabled(), "wrong call");
  // The stub doesn't create a frame so that the nmethod's frame will appear to belong to the
  // stub at any safepoint caused by the runtime call.  This eliminates the need for debug info
  // at the call to the stub, but the stub must preserve any oop returned by the method.
  ciObject2reg (method, O0);
  if (result_is_oop) {
    __ mov(I0, O1);
  } else {
    __ clr(O1);
  }
  __ call(Runtime1::entry_for(Runtime1::jvmpi_method_exit_id), relocInfo::runtime_call_type);
  __ delayed()->nop();
  if (result_is_oop) {
    // The preserved oop result, which may have been changed by a GC, is in I2.
    __ mov(I2, I0);
  }
}


void LIR_Assembler::jvmpi_method_enter(CodeEmitInfo* info) {
  assert(compilation()->jvmpi_event_method_entry_enabled() || compilation()->jvmpi_event_method_entry2_enabled(), "wrong call");
  ciObject2reg (compilation()->method(), O0);
  if (!compilation()->method()->is_static()) { 
    __ mov(I0, O1);
  } else {
    __ clr(O1);
  }
  __ call(Runtime1::entry_for(Runtime1::jvmpi_method_entry_id), relocInfo::runtime_call_type);
  __ delayed()->nop();
  add_call_info(code_offset(), info);
}


void LIR_Assembler::jobject2reg(jobject o, Register reg) {
  if (o == NULL) {
    // This seems wrong as we do not emit relocInfo 
    // for classes that are not loaded yet, i.e., they will be
    // never GC'd
    NEEDS_CLEANUP
    __ set((jint)NULL, reg);
  } else {
    int oop_index = __ oop_recorder()->find_index(o);
    RelocationHolder rspec = oop_Relocation::spec(oop_index);
    __ set(NULL, reg, rspec); // Will be set when the nmethod is created
  }
}


void LIR_Assembler::jobject2reg_with_patching(Register reg, CodeEmitInfo *info) {
  PatchingStub* patch = new PatchingStub(_masm, PatchingStub::load_klass_id);
  
  int oop_index = __ oop_recorder()->allocate_index(NULL);
  Address addr = Address(reg, address(NULL), oop_Relocation::spec(oop_index));
  assert(addr.rspec().type() == relocInfo::oop_type, "must be an oop reloc");
  // It may not seem necessary to use a sethi/add pair to load a NULL into dest, but the
  // NULL will be dynamically patched later and the patched value may be large.  We must
  // therefore generate the sethi/add as a placeholders
  __ sethi(addr, true);
  __ add(addr, reg, 0);
  
  patching_epilog(patch, LIR_Op1::patch_normal, noreg, info);
}


void LIR_Assembler::ciObject2reg(ciObject* con, Register reg) {
  assert(con->is_loaded(), "must be loaded");
  jobject2reg(con->encoding(), reg);
}


void LIR_Assembler::emit_op3(LIR_Op3* op) {
  Register Rdividend = op->in_opr1()->as_register();
  Register Rdivisor  = noreg;
  Register Rscratch  = op->in_opr3()->as_register();
  Register Rresult   = op->result_opr()->as_register();
  int divisor = -1;

  if (op->in_opr2()->is_register()) {
    Rdivisor = op->in_opr2()->as_register();
  } else {
    divisor = op->in_opr2()->as_constant_ptr()->as_jint();
    assert(Assembler::is_simm13(divisor), "can only handle simm13");
  }

  assert(Rdividend != Rscratch, "");
  assert(Rdivisor  != Rscratch, "");
  assert(op->code() == lir_idiv || op->code() == lir_irem, "Must be irem or idiv");

  if (Rdivisor == noreg && is_power_of_2(divisor)) {
    // convert division by a power of two into some shifts and logical operations
    if (op->code() == lir_idiv) {
      if (divisor == 2) {
        __ srl(Rdividend, 31, Rscratch);
      } else {
        __ sra(Rdividend, 31, Rscratch);
        __ and3(Rscratch, divisor - 1, Rscratch);
      }
      __ add(Rdividend, Rscratch, Rscratch);
      __ sra(Rscratch, log2_intptr(divisor), Rresult);
      return;
    } else {
      if (divisor == 2) {
        __ srl(Rdividend, 31, Rscratch);
      } else {
        __ sra(Rdividend, 31, Rscratch);
        __ and3(Rscratch, divisor - 1,Rscratch);
      }
      __ add(Rdividend, Rscratch, Rscratch);
      __ andn(Rscratch, divisor - 1,Rscratch);
      __ sub(Rdividend, Rscratch, Rresult);
      return;
    }
  }

  __ sra(Rdividend, 31, Rscratch);
  __ wry(Rscratch);
  if (!VM_Version::v9_instructions_work()) {
    // v9 doesn't require these nops
    __ nop();
    __ nop();
    __ nop();
    __ nop();
  }

  add_debug_info_for_div0_here(op->info());

  if (Rdivisor != noreg) {
    __ sdivcc(Rdividend, Rdivisor, (op->code() == lir_idiv ? Rresult : Rscratch));
  } else {
    assert(Assembler::is_simm13(divisor), "can only handle simm13");
    __ sdivcc(Rdividend, divisor, (op->code() == lir_idiv ? Rresult : Rscratch));
  }

  Label skip;
  __ br(Assembler::overflowSet, true, Assembler::pn, skip);
  __ delayed()->Assembler::sethi(0x80000000, (op->code() == lir_idiv ? Rresult : Rscratch));
  __ bind(skip);

  if (op->code() == lir_irem) {
    if (Rdivisor != noreg) {
      __ smul(Rscratch, Rdivisor, Rscratch);
    } else {
      __ smul(Rscratch, divisor, Rscratch);
    }
    __ sub(Rdividend, Rscratch, Rresult);
  }
}


void LIR_Assembler::emit_opBranch(LIR_OpBranch* op) {
  assert(op->cond() != LIR_OpBranch::intrinsicFailed, "meaningless on sparc");

  // if info is not NULL then it is assumed that the prediction should be taken.
  CodeEmitInfo* info = op->info();

  if (op->cond() == LIR_OpBranch::always) {
    if (info != NULL) add_debug_info_for_branch(info);
    __ br(Assembler::always, false, Assembler::pt, *(op->label()));
  } else if (op->code() == lir_cond_float_branch) {
    assert(info == NULL, "shouldn't have CodeEmitInfo");
    assert(op->ulabel() != NULL, "must have unordered successor");
    bool is_unordered = (op->ulabel() == op->label());
    Assembler::Condition acond;
    switch (op->cond()) {
      case LIR_OpBranch::equal:         acond = Assembler::f_equal;    break;
      case LIR_OpBranch::notEqual:      acond = Assembler::f_notEqual; break;
      case LIR_OpBranch::less:          acond = (is_unordered ? Assembler::f_unorderedOrLess          : Assembler::f_less);           break;
      case LIR_OpBranch::greater:       acond = (is_unordered ? Assembler::f_unorderedOrGreater       : Assembler::f_greater);        break;
      case LIR_OpBranch::lessEqual:     acond = (is_unordered ? Assembler::f_unorderedOrLessOrEqual   : Assembler::f_lessOrEqual);    break;
      case LIR_OpBranch::greaterEqual:  acond = (is_unordered ? Assembler::f_unorderedOrGreaterOrEqual: Assembler::f_greaterOrEqual); break;
      default :                         ShouldNotReachHere();
    };
    
    if (!VM_Version::v9_instructions_work()) {
      __ nop();
    }
    __ fb( acond, false, info != NULL ? Assembler::pt : Assembler::pn, *(op->label()));  
  } else {
    assert(info == NULL, "shouldn't have CodeEmitInfo");
    assert (op->code() == lir_branch, "just checking");

    Assembler::Condition acond;
    switch (op->cond()) {
      case LIR_OpBranch::equal:        acond = Assembler::equal;        break;
      case LIR_OpBranch::notEqual:     acond = Assembler::notEqual;     break;
      case LIR_OpBranch::less:         acond = Assembler::less;         break;
      case LIR_OpBranch::lessEqual:    acond = Assembler::lessEqual;    break;
      case LIR_OpBranch::greaterEqual: acond = Assembler::greaterEqual; break;
      case LIR_OpBranch::greater:      acond = Assembler::greater;      break;
      case LIR_OpBranch::aboveEqual:   acond = Assembler::greaterEqualUnsigned;      break;
      case LIR_OpBranch::belowEqual:   acond = Assembler::lessEqualUnsigned;      break;
      default:                         ShouldNotReachHere();
    };
    __ br (acond, false, info != NULL ? Assembler::pt : Assembler::pn, *(op->label()));
  }
  // if the lir optimizer is running then the
  // delay slot will be filled for us.
  if (!LIRFillDelaySlots) {
    __ delayed()->nop();
  }
}


void LIR_Assembler::emit_opConvert(LIR_OpConvert* op) {
  Bytecodes::Code code = op->bytecode();
  RInfo dst = op->result_opr()->rinfo();

  switch(code) {
    case Bytecodes::_i2l: {
      assert(dst.is_long(), "check");
      Register rlo  = dst.as_register_lo();
      Register rhi  = dst.as_register_hi();
      Register rval = op->in_opr()->as_register();    
      __ mov(rval, rlo);
      __ sra(rval, BitsPerInt-1, rhi);
      break;
    }
    case Bytecodes::_i2d: 
    case Bytecodes::_i2f: {
      bool is_double = (code == Bytecodes::_i2d);
      FloatRegister rdst = is_double ? dst.as_double_reg() : dst.as_float_reg();
      FloatRegisterImpl::Width w = is_double ? FloatRegisterImpl::D : FloatRegisterImpl::S;
      __ fitof(w, rdst, rdst);
      break;
    }
    case Bytecodes::_f2i:{
      FloatRegister rsrc = op->in_opr()->rinfo().as_float_reg();
      Register      rdst = dst.as_register();
      Label L;
      // result must be 0 if value is NaN; test by comparing value to itself
      __ fcmp(FloatRegisterImpl::S, Assembler::fcc0, rsrc, rsrc);
      if (!VM_Version::v9_instructions_work()) {
        __ nop();
      }
      __ fb(Assembler::f_unordered, true, Assembler::pn, L);  
      __ delayed()->mov(G0, rdst); // annuled if contents of rsrc is not NaN
      __ ftoi(FloatRegisterImpl::S, rsrc, rsrc);
      // move integer result from float register to int register
      Address temp_slot = Address(SP, 0, (frame::register_save_words * wordSize) + STACK_BIAS);
      __ stf(FloatRegisterImpl::S, rsrc, temp_slot.base(), temp_slot.disp());
      __ lduw(temp_slot.base(), temp_slot.disp(), rdst);
      __ bind (L);
      break;
    }
    case Bytecodes::_l2i: {
      assert(dst.is_word(), "check");
      Register rlo  = op->in_opr()->rinfo().as_register_lo();
      Register rhi  = op->in_opr()->rinfo().as_register_hi();
      Register rdst = dst.as_register();
      __ mov(rlo, rdst);
      break;
    } 
    case Bytecodes::_d2f: 
    case Bytecodes::_f2d: {
      bool is_double = (code == Bytecodes::_f2d);
      assert((!is_double && dst.is_float()) || (is_double && dst.is_double()), "check");
      RInfo val = op->in_opr()->rinfo();
      FloatRegister rval = (code == Bytecodes::_d2f) ? val.as_double_reg() : val.as_float_reg();
      FloatRegister rdst = is_double ? dst.as_double_reg() : dst.as_float_reg();
      FloatRegisterImpl::Width vw = is_double ? FloatRegisterImpl::S : FloatRegisterImpl::D;
      FloatRegisterImpl::Width dw = is_double ? FloatRegisterImpl::D : FloatRegisterImpl::S;
      __ ftof(vw, dw, rval, rdst);
      break;
    }
    case Bytecodes::_i2s:
    case Bytecodes::_i2b: {
      assert(dst.is_word(), "check");
      Register rval = op->in_opr()->as_register();
      Register rdst = dst.as_register();
      int shift = (code == Bytecodes::_i2b) ? (BitsPerInt - T_BYTE_aelem_bytes * BitsPerByte) : (BitsPerInt - BitsPerShort);
      __ sll (rval, shift, rdst);
      __ sra (rdst, shift, rdst);
      break;
    }
    case Bytecodes::_i2c: {
      assert(dst.is_word(), "check");
      Register rval = op->in_opr()->as_register();
      Register rdst = dst.as_register();
      int shift = BitsPerInt - T_CHAR_aelem_bytes * BitsPerByte;
      __ sll (rval, shift, rdst);
      __ srl (rdst, shift, rdst);
      break;
    }
 
    default: ShouldNotReachHere();
  }
}


void LIR_Assembler::align_call(LIR_Code) {
  // do nothing since all instructions are word aligned on sparc
}


void LIR_Assembler::call(address entry, relocInfo::relocType rtype, CodeEmitInfo* info) {
  __ call(entry, rtype);
  // if the lir optimizer is running then the
  // delay slot will be filled for us.  The LIR_OpDelay also has the
  // CodeEmitInfo for this call and will emit it the add_call_info()
  // call necessary for this call.  This is kind of convoluted but
  // since add_debug_info is picky about the order in which  debug_info
  // is added, this is the only way I can see to do it.
  if (!LIRFillDelaySlots) {
    __ delayed()->nop();
    add_call_info(code_offset(), info);
  }
}


void LIR_Assembler::ic_call(address entry, CodeEmitInfo* info) {
  RelocationHolder rspec = virtual_call_Relocation::spec(pc());
  __ set_oop((jobject)Universe::non_oop_word(), G5_inline_cache_reg);
  __ relocate(rspec);
  __ call(entry, relocInfo::none);

  // if the lir optimizer is running then the
  // delay slot will be filled for us.
  if (!LIRFillDelaySlots) {
    __ delayed()->nop();
    add_call_info(code_offset(), info);
//   } else {
//     add_call_info(code_offset() + NativeInstruction::nop_instruction_size, info);
  }
}


void LIR_Assembler::vtable_call(int vtable_offset, CodeEmitInfo* info) {
  add_debug_info_for_null_check_here(info);
  __ ld_ptr(Address(O0, 0,  oopDesc::klass_offset_in_bytes()), G3_scratch);
  if (__ is_simm13(vtable_offset) ) {
    __ ld_ptr(G3_scratch, vtable_offset, G5_method);
  } else {
    // This will generate 2 instructions
    __ set(vtable_offset, G5_method);
    // ld_ptr, set_hi, set
    __ ld_ptr(G3_scratch, G5_method, G5_method);
  }
  __ ld_ptr(G5_method, in_bytes(methodOopDesc::from_compiled_code_entry_point_offset()), G3_scratch);
  __ callr(G3_scratch, G0);

  // if the lir optimizer is running then the
  // delay slot will be filled for us.
  if (!LIRFillDelaySlots) {
    __ delayed()->nop();
    add_call_info(code_offset(), info);
  }
}


// load with 32-bit displacement
int LIR_Assembler::load(Register s, int disp, Register d, BasicType ld_type, CodeEmitInfo *info) {
  int load_offset = code_offset();
  if (Assembler::is_simm13(disp)) { 
    if (info != NULL) add_debug_info_for_null_check_here(info);
    switch(ld_type) {
      case T_BOOLEAN: // fall through
      case T_BYTE  : __ ldsb(s, disp, d); break;
      case T_CHAR  : __ lduh(s, disp, d); break;
      case T_SHORT : __ ldsh(s, disp, d); break;
      case T_INT   : __ ld(s, disp, d); break;
      case T_ADDRESS:// fall through
      case T_ARRAY : // fall through
      case T_OBJECT: __ ld_ptr(s, disp, d); break;
      default      : ShouldNotReachHere();
    }
  } else { 
    __ sethi(disp & ~0x3ff, O7, true);
    __ add(O7, disp & 0x3ff, O7);
    if (info != NULL) add_debug_info_for_null_check_here(info);
    load_offset = code_offset();
    switch(ld_type) {
      case T_BOOLEAN: // fall through
      case T_BYTE  : __ ldsb(s, O7, d); break;
      case T_CHAR  : __ lduh(s, O7, d); break;
      case T_SHORT : __ ldsh(s, O7, d); break;
      case T_INT   : __ ld(s, O7, d); break;
      case T_ADDRESS:// fall through
      case T_ARRAY : // fall through
      case T_OBJECT: __ ld_ptr(s, O7, d); break;
      default      : ShouldNotReachHere();
    }
  }
  if (ld_type == T_ARRAY || ld_type == T_OBJECT) __ verify_oop(d);
  return load_offset;
}


// store with 32-bit displacement
void LIR_Assembler::store(Register value, Register base, int offset, BasicType type, CodeEmitInfo *info) { 
  if (Assembler::is_simm13(offset)) { 
    if (info != NULL)  add_debug_info_for_null_check_here(info);
    switch (type) {
      case T_BOOLEAN: // fall through
      case T_BYTE  : __ stb(value, base, offset); break;
      case T_CHAR  : __ sth(value, base, offset); break;
      case T_SHORT : __ sth(value, base, offset); break;
      case T_INT   : __ stw(value, base, offset); break;
      case T_ADDRESS:// fall through
      case T_ARRAY : // fall through
      case T_OBJECT: __ st_ptr(value, base, offset); break;
      default      : ShouldNotReachHere();
    }
  } else {
    __ sethi(offset & ~0x3ff, O7, true);
    __ add(O7, offset & 0x3ff, O7);
    if (info != NULL) add_debug_info_for_null_check_here(info);
    switch (type) {
      case T_BOOLEAN: // fall through
      case T_BYTE  : __ stb(value, base, O7); break;
      case T_CHAR  : __ sth(value, base, O7); break;
      case T_SHORT : __ sth(value, base, O7); break;
      case T_INT   : __ stw(value, base, O7); break;
      case T_ADDRESS:// fall through
      case T_ARRAY : //fall through
      case T_OBJECT: __ st_ptr(value, base, O7); break;
      default      : ShouldNotReachHere();
    }
  }
  // Note: Do the store before verification as the code might be patched!
  if (type == T_ARRAY || type == T_OBJECT) __ verify_oop(value);
} 


// load float with 32-bit displacement
void LIR_Assembler::load(Register s, int disp, FloatRegister d, BasicType ld_type, CodeEmitInfo *info) { 
  FloatRegisterImpl::Width w;
  switch(ld_type) {
    case T_FLOAT : w = FloatRegisterImpl::S; break;
    case T_DOUBLE: w = FloatRegisterImpl::D; break;
    default      : ShouldNotReachHere();
  }

  if (Assembler::is_simm13(disp)) { 
    if (info != NULL) add_debug_info_for_null_check_here(info);
    __ ldf(w, s, disp, d); 
  } else { 
    __ sethi(disp & ~0x3ff, O7, true);
    __ add(O7, disp & 0x3ff, O7);
    if (info != NULL) add_debug_info_for_null_check_here(info);
    __ ldf(w, s, O7, d); 
  } 
} 


// store float with 32-bit displacement
void LIR_Assembler::store(FloatRegister value, Register base, int offset, BasicType type, CodeEmitInfo *info) { 
  FloatRegisterImpl::Width w;
  switch(type) {
    case T_FLOAT : w = FloatRegisterImpl::S; break;
    case T_DOUBLE: w = FloatRegisterImpl::D; break;
    default      : ShouldNotReachHere();
  }

  if (Assembler::is_simm13(offset)) { 
    if (info != NULL) add_debug_info_for_null_check_here(info);
    __ stf(w, value, base, offset);
  } else { 
    __ sethi(offset & ~0x3ff, O7, true);
    __ add(O7, offset & 0x3ff, O7);
    if (info != NULL) add_debug_info_for_null_check_here(info);
    __ stf(w, value, O7, base); 
  } 
}

  
int LIR_Assembler::store(RInfo from_reg, Register base, int offset, BasicType type) { 
  int store_offset;
  if (!Assembler::is_simm13(offset + (type == T_LONG) ? wordSize : 0)) {
    // for offsets larger than a simm13 we setup the offset in O7
    __ sethi(offset & ~0x3ff, O7, true);
    __ add(O7, offset & 0x3ff, O7);
    store_offset = store(from_reg, base, O7, type);
  } else {
    if (type == T_ARRAY || type == T_OBJECT) __ verify_oop(from_reg.as_register());
    store_offset = code_offset();
    switch (type) {
      case T_BOOLEAN: // fall through
      case T_BYTE  : __ stb(from_reg.as_register(), base, offset); break;
      case T_CHAR  : __ sth(from_reg.as_register(), base, offset); break;
      case T_SHORT : __ sth(from_reg.as_register(), base, offset); break;
      case T_INT   : __ stw(from_reg.as_register(), base, offset); break;
      case T_LONG  :
        __ stw(from_reg.as_register_lo(), base, offset + lo_word_offset_in_bytes); 
        __ stw(from_reg.as_register_hi(), base, offset + hi_word_offset_in_bytes);
        break;
      case T_ADDRESS:// fall through
      case T_ARRAY : // fall through
      case T_OBJECT: __ st_ptr(from_reg.as_register(), base, offset); break;
      case T_FLOAT : __ stf(FloatRegisterImpl::S, from_reg.as_float_reg(), base, offset); break;
      case T_DOUBLE:
        if (offset % 8 != 0 && (base == FP || base == SP)) {
          // split unaligned double stores to the stack
          assert(Assembler::is_simm13(offset + BytesPerWord), "must be");
          FloatRegister value = from_reg.as_double_reg();
          __ stf(FloatRegisterImpl::S, value->successor(), base, offset + BytesPerWord);
          __ stf(FloatRegisterImpl::S, value,              base, offset);
        } else {
          __ stf(FloatRegisterImpl::D, from_reg.as_double_reg(), base, offset);
        }
        break;
      default      : ShouldNotReachHere();
    }
  }
  return store_offset;
}


int LIR_Assembler::store(RInfo from_reg, Register base, Register disp, BasicType type) { 
  if (type == T_ARRAY || type == T_OBJECT) __ verify_oop(from_reg.as_register());
  int store_offset = code_offset();
  switch (type) {
    case T_BOOLEAN: // fall through
    case T_BYTE  : __ stb(from_reg.as_register(), base, disp); break;
    case T_CHAR  : __ sth(from_reg.as_register(), base, disp); break;
    case T_SHORT : __ sth(from_reg.as_register(), base, disp); break;
    case T_INT   : __ stw(from_reg.as_register(), base, disp); break;
    case T_LONG  : Unimplemented(); break;
    case T_ADDRESS:// fall through
    case T_ARRAY : // fall through
    case T_OBJECT: __ st_ptr(from_reg.as_register(), base, disp); break;
    case T_FLOAT : __ stf(FloatRegisterImpl::S, from_reg.as_float_reg(), base, disp); break;
    case T_DOUBLE: __ stf(FloatRegisterImpl::D, from_reg.as_double_reg(), base, disp); break;
    default      : ShouldNotReachHere();
  }
  return store_offset;
}


int LIR_Assembler::load(Register base, int offset, RInfo to_reg, BasicType type) { 
  int load_offset;
  if (!Assembler::is_simm13(offset + (type == T_LONG) ? wordSize : 0)) {
    assert(base != O7, "destroying register");
    // for offsets larger than a simm13 we setup the offset in O7
    __ sethi(offset & ~0x3ff, O7, true);
    __ add(O7, offset & 0x3ff, O7);
    load_offset = load(base, O7, to_reg, type);
  } else {
    load_offset = code_offset();
    switch(type) {
      case T_BOOLEAN: // fall through
      case T_BYTE  : __ ldsb(base, offset, to_reg.as_register()); break;
      case T_CHAR  : __ lduh(base, offset, to_reg.as_register()); break;
      case T_SHORT : __ ldsh(base, offset, to_reg.as_register()); break;
      case T_INT   : __ ld(base, offset, to_reg.as_register()); break;
      case T_LONG  :
        __ ld(base, offset + lo_word_offset_in_bytes, to_reg.as_register_lo()); 
        __ ld(base, offset + hi_word_offset_in_bytes, to_reg.as_register_hi());
        break;
      case T_ADDRESS:// fall through
      case T_ARRAY : // fall through
      case T_OBJECT: __ ld_ptr(base, offset, to_reg.as_register()); break;
      case T_FLOAT:  __ ldf(FloatRegisterImpl::S, base, offset, to_reg.as_float_reg()); break;
      case T_DOUBLE:
        // split unaligned loads
        if (offset % 8 != 0 && (base == FP || base == SP)) {
          assert(Assembler::is_simm13(offset + BytesPerWord), "out of range");
          FloatRegister reg = to_reg.as_double_reg();
          __ ldf(FloatRegisterImpl::S, base, offset + BytesPerWord, reg->successor());
          __ ldf(FloatRegisterImpl::S, base, offset,                reg);
        } else {
          __ ldf(FloatRegisterImpl::D, base, offset, to_reg.as_double_reg());
        }
        break;
      default      : ShouldNotReachHere();
    }
    if (type == T_ARRAY || type == T_OBJECT) __ verify_oop(to_reg.as_register());
  }
  return load_offset;
}


int LIR_Assembler::load(Register base, Register disp, RInfo to_reg, BasicType type) { 
  int load_offset = code_offset();
  switch(type) {
    case T_BOOLEAN: // fall through
    case T_BYTE  : __ ldsb(base, disp, to_reg.as_register()); break;
    case T_CHAR  : __ lduh(base, disp, to_reg.as_register()); break;
    case T_SHORT : __ ldsh(base, disp, to_reg.as_register()); break;
    case T_INT   : __ ld(base, disp, to_reg.as_register()); break;
    case T_ADDRESS:// fall through
    case T_ARRAY : // fall through
    case T_OBJECT: __ ld_ptr(base, disp, to_reg.as_register()); break;
    case T_FLOAT:  __ ldf(FloatRegisterImpl::S, base, disp, to_reg.as_float_reg()); break;
    case T_DOUBLE: __ ldf(FloatRegisterImpl::D, base, disp, to_reg.as_double_reg()); break;
    default      : ShouldNotReachHere();
  }
  if (type == T_ARRAY || type == T_OBJECT) __ verify_oop(to_reg.as_register());
  return load_offset;
}


// load/store with an Address
void LIR_Assembler::load(const Address& a, Register d,  BasicType ld_type, CodeEmitInfo *info, int offset) { 
  load(a.base(), a.disp() + offset, d, ld_type, info);  
}


void LIR_Assembler::store(Register value, const Address& dest, BasicType type, CodeEmitInfo *info, int offset) { 
  store(value, dest.base(), dest.disp() + offset, type, info); 
}


// loadf/storef with an Address
void LIR_Assembler::load(const Address& a, FloatRegister d, BasicType ld_type, CodeEmitInfo *info, int offset) { 
  load(a.base(), a.disp() + offset, d, ld_type, info);   
}


void LIR_Assembler::store(FloatRegister value, const Address& dest, BasicType type, CodeEmitInfo *info, int offset) { 
  store(value, dest.base(), dest.disp() + offset, type, info); 
}


void LIR_Assembler::local2reg(RInfo reg, int local_index, BasicType t) {
  BasicType local_type = t;
  switch (t) {
    case T_BOOLEAN: // fall through
    case T_BYTE   : // fall through
    case T_CHAR   : // fall through
    case T_SHORT  : // fall through
    case T_INT    : // fall through
      local_type = T_INT; // fall through

    case T_FLOAT  : // fall through
    case T_OBJECT : // fall through
    case T_ADDRESS: // fall through
    case T_ARRAY  : // fall through
      {
        Address addr = frame_map()->address_for_local_name(FrameMap::name_for_argument(local_index), false);
        load(addr.base(), addr.disp(), reg, local_type);
      }
      break;

    case T_LONG   :
      {
        Address addrLo = frame_map()->address_for_local_name(FrameMap::name_for_argument(local_index), true, false);
        Address addrHi = frame_map()->address_for_local_name(FrameMap::name_for_argument(local_index), true, true);
        load(addrLo.base(), addrLo.disp(), reg.as_register_lo(), T_INT);
        load(addrHi.base(), addrHi.disp(), reg.as_register_hi(), T_INT);
      }
      break;

    case T_DOUBLE :
      {
        Address addrLo = frame_map()->address_for_local_name(FrameMap::name_for_argument(local_index), true, false);
        Address addrHi = frame_map()->address_for_local_name(FrameMap::name_for_argument(local_index), true, true);
        load(addrLo.base(), addrLo.disp(), reg.as_double_reg()->successor(), T_FLOAT);
        load(addrHi.base(), addrHi.disp(), reg.as_double_reg(),              T_FLOAT);
      }
      break;


    default       : ShouldNotReachHere();
  }
}


void LIR_Assembler::reg2local(int local_index, RInfo reg, BasicType t) {
  BasicType local_type = t;
  Address addr = frame_map()->address_for_local_name(FrameMap::name_for_argument(local_index), t == T_LONG || t == T_DOUBLE);
  switch (t) {
    case T_BOOLEAN: // fall through
    case T_BYTE   : // fall through
    case T_CHAR   : // fall through
    case T_SHORT  : // fall through
    case T_INT    : // fall through
      local_type = T_INT; // fall through

    case T_FLOAT  : // fall through
    case T_DOUBLE : // fall through
    case T_LONG   : // fall through
    case T_OBJECT : // fall through
    case T_ADDRESS: // fall through
    case T_ARRAY  : // fall through
      store(reg, addr.base(), addr.disp(), local_type);
      break;

    default       : ShouldNotReachHere();
  }
}


void LIR_Assembler::const2stack(LIR_Const* c, int stack_index) {
  if (c->type() == T_INT) {
    Register src_reg = O7;
    int value = c->as_jint();
    Address addr = frame_map()->address_for_name(stack_index, false);
    if (value == 0) {
      src_reg = G0;
    } else if (Assembler::is_simm13(value)) {
      __ or3(G0, value & 0x3ff, O7);
    } else {
      __ sethi(value & ~0x3ff, O7, false);
      __ or3(O7, value & 0x3ff, O7);

    }
    __ stw(src_reg, addr.base(), addr.disp());
  } else if (c->type() == T_OBJECT) {
    Register src_reg = O7;
    jobject2reg(c->as_jobject(), src_reg);
    Address addr = frame_map()->address_for_name(stack_index, false);
    __ st_ptr(src_reg, addr.base(), addr.disp());
  } else {
    Unimplemented();
  }
}


void LIR_Assembler::const2mem(LIR_Const* c, LIR_Address* addr, BasicType type, CodeEmitInfo* info ) {
  if (info != NULL) {
    add_debug_info_for_null_check_here(info);
  }
  if (c->type() == T_INT) {
    Register src_reg = O7;
    int value = c->as_jint();
    if (value == 0) {
      src_reg = G0;
    } else if (Assembler::is_simm13(value)) {
      __ or3(G0, value & 0x3ff, O7);
    } else {
      __ sethi(value & ~0x3ff, O7, false);
      __ or3(O7, value & 0x3ff, O7);

    }
    if (addr->index()->is_valid()) {
      assert(offset_of(addr) == 0, "must be zero");
      store(as_RInfo(src_reg), base_of(addr).as_register(), addr->index()->as_register(), type);
    } else {
      store(src_reg, base_of(addr).as_register(), offset_of(addr), type);
    }
  } else if (c->type() == T_OBJECT) {
    Register src_reg = O7;
    jobject2reg(c->as_jobject(), src_reg);
    __ st_ptr(src_reg, base_of(addr).as_register(), offset_of(addr));
  } else {
    Unimplemented();
  }
}


void LIR_Assembler::const2reg(LIR_Const* c, RInfo to_reg, LIR_Op1::LIR_PatchCode patch_code, CodeEmitInfo* info) {
  switch (c->type()) {
    case T_INT:
      {
        jint con = c->as_jint();
        if (to_reg.is_word()) {
          assert(patch_code == LIR_Op1::patch_none, "no patching handled here");
          __ set(con, to_reg.as_register());
        } else {
          assert(to_reg.is_float(), "wrong register kind");

          __ set(con, O7);
          Address temp_slot(SP, 0, (frame::register_save_words * wordSize) + STACK_BIAS);
          __ st(O7, temp_slot);
          __ ldf(FloatRegisterImpl::S, temp_slot, to_reg.as_float_reg());
        }
      }
      break;
  
    case T_LONG:
      {
        jlong con = c->as_jlong();

        if (to_reg.is_long()) {
          __ set(low(con),  to_reg.as_register_lo());
          __ set(high(con), to_reg.as_register_hi());
        } else {
          assert(to_reg.is_double(), "wrong register kind");
          Address temp_slot_lo(SP, 0, ((frame::register_save_words  ) * wordSize) + STACK_BIAS);
          Address temp_slot_hi(SP, 0, ((frame::register_save_words) * wordSize) + (longSize/2) + STACK_BIAS);
          __ set(low(con),  O7);
          __ st(O7, temp_slot_lo);
          __ set(high(con), O7);
          __ st(O7, temp_slot_hi);
          __ ldf(FloatRegisterImpl::D, temp_slot_lo, to_reg.as_double_reg());
        }
      }
      break;

    case T_OBJECT:
      {
        if (patch_code == LIR_Op1::patch_none) {
          jobject2reg(c->as_jobject(), to_reg.as_register());
        } else {
          jobject2reg_with_patching(to_reg.as_register(), info);
        }
      }
      break;

    case T_FLOAT:
      {
        address const_addr = _const_table.address_of_float_constant(c->as_jfloat());
        assert (const_addr != NULL, "must create float constant in the constant table");

        RelocationHolder rspec = internal_word_Relocation::spec(const_addr);
        if (to_reg.is_float()) {
          __ sethi(  (intx)const_addr & ~0x3ff, O7, true, rspec);
          __ relocate(rspec);

          int offset = (intx)const_addr & 0x3ff;
          __ ldf (FloatRegisterImpl::S, O7, offset, to_reg.as_float_reg());

        } else {
          assert(to_reg.is_word(), "Must be a cpu register.");

          __ set((intx)const_addr, O7, rspec);
          load(O7, 0, to_reg.as_register(), T_INT);
        }
      }
      break;
 
    case T_DOUBLE:
      {
        address const_addr = _const_table.address_of_double_constant(c->as_jdouble());
        assert (const_addr != NULL, "must create double constant in the constant table");
        RelocationHolder rspec = internal_word_Relocation::spec(const_addr);

        if (to_reg.is_double()) {
          __ sethi(  (intx)const_addr & ~0x3ff, O7, true, rspec);
          int offset = (intx)const_addr & 0x3ff;
          __ relocate(rspec);
          __ ldf (FloatRegisterImpl::D, O7, offset, to_reg.as_double_reg());
        } else {
          assert(to_reg.is_long(), "Must be a long register.");

          __ set((intx)const_addr, O7, rspec);
          load(O7, lo_word_offset_in_bytes, to_reg.as_register_lo(), T_INT);
          load(O7, hi_word_offset_in_bytes, to_reg.as_register_hi(), T_INT);
        }

      }
      break;

    default:
      ShouldNotReachHere();
  }
}

Address LIR_Assembler::as_Address(LIR_Address* addr) {
  Register reg = addr->base()->as_register();
  assert(reg != SP && reg != FP, "address must be in heap, not stack");
  return Address(reg, 0, addr->disp());

}


Address LIR_Assembler::as_Address_hi(LIR_Address* addr) {
  return as_Address(addr);
}


Address LIR_Assembler::as_Address_lo(LIR_Address* addr) { 
  Register reg = addr->base()->as_register();
  assert(reg != SP && reg != FP, "address must be in heap, not stack");
  return Address(reg, 0, addr->disp()+longSize/2);
}


void LIR_Assembler::mem2reg(LIR_Address* addr, RInfo to_reg, BasicType type, LIR_Op1::LIR_PatchCode patch_code, CodeEmitInfo* info) {
  Register src = addr->base()->as_register();
  Register disp_reg = noreg;
  int disp_value = addr->disp();
  bool needs_patching = (patch_code != LIR_Op1::patch_none);

  PatchingStub* patch = NULL;
  if (needs_patching) {
    patch = new PatchingStub(_masm, PatchingStub::access_field_id);
    assert(!to_reg.is_long() || patch_code == LIR_Op1::patch_low || patch_code == LIR_Op1::patch_high, "patching doesn't match register");
  }

  if (addr->index()->is_illegal()) {
    if (!Assembler::is_simm13(disp_value)) { 
      if (needs_patching) {
        __ sethi(0, O7, true);
        __ add(O7, 0, O7);
      } else {
        __ set(disp_value, O7);
      }
      disp_reg = O7;
    }
  } else {
    disp_reg = addr->index()->as_register();
    assert(disp_value == 0, "can't handle 3 operand address");
  }

  // remember the offset of the load.  The patching_epilog must be done
  // before the call to add_debug_info, otherwise the PcDescs don't get
  // entered in increasing order.
  int offset = code_offset();
  
  if (to_reg.is_double()) {
    FloatRegisterImpl::Width w = FloatRegisterImpl::D;
    if (disp_reg == noreg) {
      __ ldf(w, src, disp_value, to_reg.as_double_reg()); 
    } else {
      __ ldf(w, src, disp_reg, to_reg.as_double_reg()); 
    }
  } else if (to_reg.is_float()) {
    FloatRegisterImpl::Width w = FloatRegisterImpl::S;
    if (disp_reg == noreg) {
      __ ldf(w, src, disp_value, to_reg.as_float_reg()); 
    } else {
      __ ldf(w, src, disp_reg, to_reg.as_float_reg());
    } 
  } else if (to_reg.is_long()) {
    assert(disp_reg == noreg, "this is for array loads, which must use constant offsets");
    assert(Assembler::is_simm13(disp_value + lo_word_offset_in_bytes) && Assembler::is_simm13(disp_value + hi_word_offset_in_bytes), "must fit in constant");
    if (src == to_reg.as_register_lo()) {
      offset = load(src, disp_value + hi_word_offset_in_bytes, to_reg.as_register_hi(), T_INT);
      load(src, disp_value + lo_word_offset_in_bytes, to_reg.as_register_lo(), T_INT);
    } else {
      offset = load(src, disp_value + lo_word_offset_in_bytes, to_reg.as_register_lo(), T_INT);
      load(src, disp_value + hi_word_offset_in_bytes, to_reg.as_register_hi(), T_INT);
    }
  } else {
    if (disp_reg == noreg) {
      offset = load(src, disp_value, to_reg.as_register(), type);
    } else {
      offset = load(src, disp_reg, to_reg, type);
    }
  }
  
  if (needs_patching) {
    patching_epilog(patch, patch_code, src, info);
  }

  if (info != NULL) add_debug_info_for_null_check(offset, info);
}


void LIR_Assembler::stack2reg(LIR_Opr src, LIR_Opr dest, BasicType type) {
  NEEDS_CLEANUP; // do we endup handling untyped moves in here? If so, then what?
  if (dest->is_single_fpu()) {
    Address addr = frame_map()->address_for_name(src->single_stack_ix(), false);
    load( addr, dest->rinfo().as_float_reg(), T_FLOAT);
  } else if (dest->is_double_fpu())  {
    Address src_addr_LO = frame_map()->address_for_name(src->double_stack_ix(), true, false);
    Address src_addr_HI = frame_map()->address_for_name(src->double_stack_ix(), true, true);
    load( src_addr_LO, dest->rinfo().as_double_reg()->successor(), T_FLOAT);
    load( src_addr_HI, dest->rinfo().as_double_reg(), T_FLOAT);
  } else if (dest->is_double_cpu()) {
    Address src_addr_LO = frame_map()->address_for_name(src->double_stack_ix(), true, false);
    Address src_addr_HI = frame_map()->address_for_name(src->double_stack_ix(), true, true);
    load( src_addr_LO, dest->rinfo().as_register_lo(), T_INT);
    load( src_addr_HI, dest->rinfo().as_register_hi(), T_INT);
  } else {
    assert(dest->is_single_cpu(), "cannot be anything else but a single cpu");
    assert(type!= T_ILLEGAL, "Bad type in stack2reg")
    Address addr = frame_map()->address_for_name(src->single_stack_ix(), false);
    load( addr, dest->as_register(), type);
  }
}


void LIR_Assembler::reg2stack(RInfo from_reg, int to_stack_index, BasicType type) {
  if (from_reg.is_float()) {
    store(from_reg.as_float_reg(), frame_map()->address_for_name(to_stack_index, false), T_FLOAT);
  } else if (from_reg.is_double()) {
    Address dst_addr_LO = frame_map()->address_for_name(to_stack_index, true, false);
    Address dst_addr_HI = frame_map()->address_for_name(to_stack_index, true, true);
    store(from_reg.as_double_reg(),              dst_addr_HI, T_FLOAT);
    store(from_reg.as_double_reg()->successor(), dst_addr_LO, T_FLOAT);
  } else if (from_reg.is_long() ) {
    Register rlo  = from_reg.as_register_lo();
    Register rhi  = from_reg.as_register_hi();
    Address dst_addr_LO = frame_map()->address_for_name(to_stack_index, true, false);
    Address dst_addr_HI = frame_map()->address_for_name(to_stack_index, true, true);
    store(from_reg.as_register_hi(), dst_addr_HI, T_INT);
    store(from_reg.as_register_lo(), dst_addr_LO, T_INT);
  } else {
    assert(type != T_ILLEGAL, "Bad BasicType in reg2stack");
    store(from_reg.as_register(), frame_map()->address_for_name(to_stack_index, false), type);
  }
}


void LIR_Assembler::reg2reg(RInfo from_reg, RInfo to_reg) {
  if (from_reg.is_float_kind() && to_reg.is_float_kind()) {
    if (from_reg.is_double()) {
      // double to double moves
      assert(to_reg.is_double(), "should match");
      __ fmov(FloatRegisterImpl::D, from_reg.as_double_reg(), to_reg.as_double_reg());
    } else {
      // float to float moves
      assert(to_reg.is_float(), "should match");
      __ fmov(FloatRegisterImpl::S, from_reg.as_float_reg(), to_reg.as_float_reg());
    }
  } else if (!from_reg.is_float_kind() && !to_reg.is_float_kind()) {
    if (from_reg.is_long()) {
      // long to long moves
      __ mov(from_reg.as_register_hi(), to_reg.as_register_hi());
      __ mov(from_reg.as_register_lo(), to_reg.as_register_lo());
    } else {
      // int to int moves
      __ mov(from_reg.as_register(), to_reg.as_register());
    }
  } else {
    // float to int or int to float moves
    // on Sparc we have to move them to memory first
    bool is_two_word = to_reg.is_long() || to_reg.is_double();
    Address temp_slot = frame_map()->address_for_scratch_index(0, is_two_word, is_two_word);
    store(from_reg, temp_slot, BasicType_from_RInfo(from_reg));
    load(temp_slot, to_reg, BasicType_from_RInfo(to_reg));
  }
}


BasicType LIR_Assembler::BasicType_from_RInfo(RInfo r) {
  if (r.is_word())
    return T_INT;
  else if (r.is_long())
    return T_LONG;
  else if (r.is_float())
    return T_FLOAT;
  else   if (r.is_double())
    return T_DOUBLE;
  else
    ShouldNotReachHere();
}


RInfo LIR_Assembler::base_of(LIR_Address* addr) {
  return addr->base()->rinfo();
}


int LIR_Assembler::offset_of(LIR_Address* addr) {
  return addr->disp();
}


void LIR_Assembler::reg2mem(RInfo from_reg, LIR_Address* to_addr, BasicType type, LIR_Op1::LIR_PatchCode patch_code, CodeEmitInfo* info) {
  Register src = to_addr->base()->as_register();
  Register disp_reg = noreg;
  int disp_value = to_addr->disp();
  bool needs_patching = (patch_code != LIR_Op1::patch_none);

  PatchingStub* patch = NULL;
  if (needs_patching) {
    patch = new PatchingStub(_masm, PatchingStub::access_field_id);
    assert(!from_reg.is_long() || patch_code == LIR_Op1::patch_low || patch_code == LIR_Op1::patch_high, "patching doesn't match register");
  }

  if (to_addr->index()->is_illegal()) {
    if (!Assembler::is_simm13(disp_value)) { 
      if (needs_patching) {
        __ sethi(0, O7, true);
        __ add(O7, 0, O7);
      } else {
        __ set(disp_value, O7);
      }
      disp_reg = O7;
    }
  } else {
    disp_reg = to_addr->index()->as_register();
    if (disp_value != 0) {
      if (!Assembler::is_simm13(disp_value)) { 
        assert(!needs_patching, "can't patch this part of address");
        __ set(disp_value, O7);
        __ add(O7, disp_reg, O7);
      } else {
        __ add(disp_reg, disp_value, O7);
      }
      disp_reg = O7;
    }
  }

  // remember the offset of the store.  The patching_epilog must be done
  // before the call to add_debug_info_for_null_check, otherwise the PcDescs don't get
  // entered in increasing order.
  int offset;
  
  assert(disp_reg != noreg || Assembler::is_simm13(disp_value), "should have set this up");
  if (disp_reg == noreg) {
    offset = store(from_reg, base_of(to_addr).as_register(), disp_value, type);
  } else {
    offset = store(from_reg, base_of(to_addr).as_register(), disp_reg, type);
  }

  if (patch != NULL) {
    patching_epilog(patch, patch_code, base_of(to_addr).as_register(), info);
  }

  if (info != NULL) add_debug_info_for_null_check(offset, info);
}


void LIR_Assembler::return_op(RInfo result, bool result_is_oop) {
  if (compilation()->jvmpi_event_method_exit_enabled()) {
    jvmpi_method_exit(compilation()->method(), result_is_oop);
  }
    // the poll may need a register so just pick one that isn't the return register
  if (SafepointPolling) {
    __ sethi((intptr_t)os::get_polling_page(), L0, false);
    int lduw_offset = __ offset();
    __ relocate(relocInfo::poll_return_type);
    __ lduw(L0, 0, G0);

    int frame_size = frame_map()->framesize();
    int arg_count  = frame_map()->oop_map_arg_count();
    OopMap* map    = new OopMap(frame_size, arg_count);
    if (result_is_oop)
      map->set_oop(FrameMap::cpu_regname(result), frame_size, arg_count);
    compilation()->debug_info_recorder()->add_safepoint(lduw_offset, false, map);
    compilation()->debug_info_recorder()->describe_scope(compilation()->method(), -1);
  }
  __ method_exit(true);
}


void LIR_Assembler::safepoint_poll(RInfo tmp, CodeEmitInfo* info) {
  __ sethi((intptr_t)os::get_polling_page(), tmp.as_register(), false);

  if (info != NULL) {
    add_debug_info_for_branch(info);
  } else {
    __ relocate(relocInfo::poll_type);
  }

  __ lduw(tmp.as_register(), 0, G0);
}


void LIR_Assembler::emit_call_stubs() { 
  __ code()->set_stubs_begin(__ pc());
  // Interpreter entry point (argument marshalling)
  offsets()->_iep_offset = code_offset();
  interpreter_to_compiler_calling_convention(method());
  emit_stubs(_call_stubs);
  __ code()->set_stubs_end( __ pc());
}


// Uses G1
void LIR_Assembler::interpreter_to_compiler_calling_convention(ciMethod* method) {
  // G5_method: method to call
  // Gargs  : last argument
  //
  // Copy arguments if any: Copy last argument first
  // since parameter area may overlap. Use O0 as tmp
  // register, this way the receiver ends up there
  // automatically (receiver caching) since it is copied
  // last.
  //
  // Note: If there are no arguments we should not even
  //       go here but jump to the verified entry point
  //       right away. Fix this eventually.
#ifdef _LP64
  const Register value = O0;
  assert_different_registers(G5_method, Gargs, value);
  const CallingConvention* cc = FrameMap::calling_convention(method);
  BasicTypeArray* sig_types = FrameMap::signature_type_array_for(method);
  const int j = method->arg_size();
  const int n = sig_types->length();
  assert ( j == n, "Oops");
  int i = 0;
  while (i < n) {
    int arg_index = n - 1 - i;
    BasicType arg_type = sig_types->at(arg_index);
    ArgumentLocation loc = cc->arg_at(arg_index);
    switch (arg_type) {
      case T_ADDRESS:	// fall through
      case T_OBJECT: 	// fall through
      case T_ARRAY:
        if (loc.is_register_arg()) {
          __ ld_ptr(Gargs, i*BytesPerWord, 
	       loc.outgoing_reg_location().as_register());
        } else {
          __ ld_ptr(Gargs, i*BytesPerWord, value);
          __ st_ptr(value, SP, ((frame::memory_parameter_word_sp_offset + 
		i) * BytesPerWord) + STACK_BIAS);
        }
	break;
      case T_INT:       // fall through
      case T_BOOLEAN:   // fall through
      case T_CHAR:      // fall through
      case T_BYTE:      // fall through
      case T_SHORT:     
        if (loc.is_register_arg()) {
          __ ld(Gargs, i*BytesPerWord, 
	       loc.outgoing_reg_location().as_register());
        } else {
          __ ld(Gargs, i*BytesPerWord, value);
          __ st(value, SP, ((frame::memory_parameter_word_sp_offset + 
		i) * BytesPerWord) + STACK_BIAS);
        }
	break;
      case T_FLOAT:
        if (loc.is_register_arg()) {
          __ ld(Gargs, i*BytesPerWord, 
	       loc.outgoing_reg_location().as_register());
        } else {
          __ ld(Gargs, i*BytesPerWord, value);
          __ st(value, SP, ((frame::memory_parameter_word_sp_offset + 
		i) * BytesPerWord) + STACK_BIAS);
        }
	break;
      case T_LONG:    	// fall through
	// Longs are packed in the low addressed argument slot
        assert( !loc.is_register_arg(), "longs not supported in registers");
        __ ld_ptr(Gargs, i*BytesPerWord, value);
        __ st_ptr(value, SP, ((frame::memory_parameter_word_sp_offset + i) * BytesPerWord) + STACK_BIAS);
        break;
      case T_DOUBLE:
        if (loc.is_register_arg()) {
          __ ld_ptr(Gargs, i*BytesPerWord, 
	       loc.outgoing_reg_location().as_register());
        } else {
          __ ld_ptr(Gargs, i*BytesPerWord, value);
          __ st_ptr(value, SP, ((frame::memory_parameter_word_sp_offset + 
		i) * BytesPerWord) + STACK_BIAS);
        }
        break;
      default:
        ShouldNotReachHere();
        break;
    }
    i +=type2size[arg_type];  // either one word or two words
  }
#else
  const Register value = O0;
  assert_different_registers(G5_method, Gargs, value);
  const CallingConvention* cc = FrameMap::calling_convention(method);
  const int n = method->arg_size();
  for (int i = 0; i < n; i++) {
    ArgumentLocation loc = cc->arg_at(n - 1 - i);
    if (loc.is_register_arg()) {
      __ ld_ptr(Gargs, i*BytesPerWord, loc.outgoing_reg_location().as_register());
    } else {
      __ ld_ptr(Gargs, i*BytesPerWord, value);
      __ st_ptr(value, SP, ((frame::memory_parameter_word_sp_offset + i) * BytesPerWord) + STACK_BIAS);
    }
  }
#endif
  // branch to verified entry point
  __ br(Assembler::always, false, Assembler::pt, _vep_label);
  __ delayed()->nop();
}


void LIR_Assembler::comp_op(LIR_OpBranch::LIR_Condition condition, LIR_Opr opr1, LIR_Opr opr2, BasicType type) {
  if (opr1->is_single_fpu()) {
    __ fcmp(FloatRegisterImpl::S, Assembler::fcc0, opr1->rinfo().as_float_reg(), opr2->rinfo().as_float_reg());
  } else if (opr1->is_double_fpu()) {
    __ fcmp(FloatRegisterImpl::D, Assembler::fcc0, opr1->rinfo().as_double_reg(), opr2->rinfo().as_double_reg());
  } else if (opr1->is_single_cpu()) {
    if (opr2->is_constant()) {
      switch (opr2->as_constant_ptr()->type()) {
        case T_INT:
          { jint con = opr2->as_constant_ptr()->as_jint();
            if (Assembler::is_simm13(con)) {
              __ cmp(opr1->as_register(), con);
            } else {
              __ set(con, O7);
              __ cmp(opr1->as_register(), O7);
            }
          }
          break;
 
        case T_OBJECT: 
            // there are only equal/notequal comparisions on objects
          { jobject con = opr2->as_constant_ptr()->as_jobject();
            if (con == NULL) {
              __ cmp(opr1->as_register(), 0);
            } else {
              jobject2reg(con, O7);
              __ cmp(opr1->as_register(), O7);
            }
          }
          break;

        default:
          ShouldNotReachHere();
          break;
      }
    } else {
      if (opr2->is_address()) {
	assert( type != T_ILLEGAL, "Bad type in comp_op_helper");
	if ( type == T_OBJECT ) __ ld_ptr(as_Address(opr2->as_address_ptr()), O7);
   	else 			__ ld(as_Address(opr2->as_address_ptr()), O7); 
        __ cmp(opr1->as_register(), O7);
      } else {
        __ cmp(opr1->as_register(), opr2->as_register()); 
      }
    }
  } else if (opr1->is_double_cpu()) {
      Register xlo = opr1->rinfo().as_register_lo();
      Register xhi = opr1->rinfo().as_register_hi();
      if (opr2->is_constant() && opr2->as_jlong() == 0) {
        assert(condition == LIR_OpBranch::equal || condition == LIR_OpBranch::notEqual, "only handles these cases");
        __ orcc(xhi, xlo, G0);
      } else if (opr2->is_register()) {
        Register ylo = opr2->rinfo().as_register_lo();
        Register yhi = opr2->rinfo().as_register_hi();
        __ subcc(xlo, ylo, xlo);
        __ subccc(xhi, yhi, xhi);
        if (condition == LIR_OpBranch::equal || condition == LIR_OpBranch::notEqual) {
          __ orcc(xhi, xlo, G0);
        }
      } else {
        ShouldNotReachHere();
      }
  } else if (opr1->is_address()) {
    assert( type != T_ILLEGAL, "Bad type in comp_op_helper");
    assert (opr2->is_constant(), "Checking");
    if ( type == T_OBJECT ) __ ld_ptr(as_Address(opr1->as_address_ptr()), O7);
    else 		    __ ld(as_Address(opr1->as_address_ptr()), O7);
    __ cmp(O7, opr2->as_constant_ptr()->as_jint());
  } else {
    ShouldNotReachHere();
  }
}


void LIR_Assembler::comp_fl2i(LIR_Code code, LIR_Opr left, LIR_Opr right, LIR_Opr dst){
  if (code == lir_cmp_fd2i || code == lir_ucmp_fd2i) {
    bool is_unordered_less = (code == lir_ucmp_fd2i);
    if (left->is_single_fpu()) {
      __ float_cmp(true, is_unordered_less ? -1 : 1, left->rinfo().as_float_reg(), right->rinfo().as_float_reg(), dst->as_register());
    } else if (left->is_double_fpu()) {
      __ float_cmp(false, is_unordered_less ? -1 : 1, left->rinfo().as_double_reg(), right->rinfo().as_double_reg(), dst->as_register());
    } else {
      ShouldNotReachHere();
    }
  } else if (code == lir_cmp_l2i) {
    __ lcmp(left->as_register_hi(),  left->as_register_lo(),
            right->as_register_hi(), right->as_register_lo(),
            dst->as_register());
  } else {
    ShouldNotReachHere();
  }
}


void LIR_Assembler::arith_op(LIR_Code code, LIR_Opr left, LIR_Opr right, LIR_Opr dest, CodeEmitInfo* info) {
  assert(info == NULL, "unused on this code path");
  assert(left->is_register(), "wrong items state");
  assert(dest->is_register(), "wrong items state");

  if (right->is_register()) {
    if (right->is_float_kind()) {

      FloatRegister lreg, rreg, res;
      FloatRegisterImpl::Width w;
      if (right->is_single_fpu()) {
        w = FloatRegisterImpl::S;
        lreg = left->rinfo().as_float_reg();
        rreg = right->rinfo().as_float_reg();
        res  = dest->rinfo().as_float_reg();
      } else {
        w = FloatRegisterImpl::D;
        lreg = left->rinfo().as_double_reg();
        rreg = right->rinfo().as_double_reg();
        res  = dest->rinfo().as_double_reg();
      }

      //assert(lreg == res ^ !left->rinfo().destroys_register(), "check");
      //assert(rreg == res ^ !right->rinfo().destroys_register(), "check");

      switch (code) {
        case lir_add: __ fadd(w, lreg, rreg, res); break;
        case lir_sub: __ fsub(w, lreg, rreg, res); break;
        case lir_mul: // fall through
        case lir_mul_strictfp: __ fmul(w, lreg, rreg, res); break;
        case lir_div: // fall through
        case lir_div_strictfp: __ fdiv(w, lreg, rreg, res); break;
        default: ShouldNotReachHere();
      }

    } else if (right->is_double_cpu()) {

      Register op1_lo = left->rinfo().as_register_lo();
      Register op1_hi = left->rinfo().as_register_hi();
      Register op2_lo = right->rinfo().as_register_lo();
      Register op2_hi = right->rinfo().as_register_hi();
      Register dst_lo = dest->rinfo().as_register_lo();
      Register dst_hi = dest->rinfo().as_register_hi();

      switch (code) {
        case lir_add:
          __ addcc(op1_lo, op2_lo, dst_lo); 
          __ addc (op1_hi, op2_hi, dst_hi); 
        break;
        
        case lir_sub:
          __ subcc(op1_lo, op2_lo, dst_lo); 
          __ subc (op1_hi, op2_hi, dst_hi); 
        break;

        default: ShouldNotReachHere();
      }
    } else {
      assert (right->is_single_cpu(), "Just Checking");

      Register lreg = left->as_register();
      Register res  = dest->as_register();
      Register rreg = right->as_register();
      switch (code) {
        case lir_add:  __ add  (lreg, rreg, res); break;
        case lir_sub:  __ sub  (lreg, rreg, res); break;
        case lir_mul:  __ mult (lreg, rreg, res); break;
        default: ShouldNotReachHere();
      }
    }
  }
  else {
    assert (right->is_constant(), "must be constant");

    Register lreg = left->as_register();
    Register res  = dest->as_register();
    int    simm13 = right->as_constant_ptr()->as_jint();

    switch (code) {
      case lir_add:  __ add  (lreg, simm13, res); break;
      case lir_sub:  __ sub  (lreg, simm13, res); break;
      case lir_mul:  __ mult (lreg, simm13, res); break;
      default: ShouldNotReachHere();
    }
  }
}


void LIR_Assembler::fpop() {
  // do nothing
}


void LIR_Assembler::intrinsic_op(LIR_Code code, LIR_Opr value, LIR_Opr thread, LIR_Opr dest) {
  switch (code) {
    case lir_sin:
    case lir_cos: {
      assert(thread->rinfo().is_valid(), "preserve the thread object for performance reasons");
      assert(dest->rinfo().is_same(FrameMap::_F0_double_RInfo), "the result will be in f0/f1");
      break;
    }
    case lir_sqrt: {
      assert(!thread->rinfo().is_valid(), "there is no need for a thread_reg for dsqrt");
      FloatRegister src_reg = value->rinfo().as_double_reg();
      FloatRegister dst_reg = dest->rinfo().as_double_reg();
      __ fsqrt(FloatRegisterImpl::D, src_reg, dst_reg);
      break;
    }
    default: {
      ShouldNotReachHere();
      break;
    }
  }
}


void LIR_Assembler::logic_op(LIR_Code code, LIR_Opr left, LIR_Opr right, LIR_Opr dest) {
  if (right->is_constant()) {
    int simm13 = right->as_constant_ptr()->as_jint();
    switch (code) {
      case lir_logic_and:   __ and3 (left->as_register(), simm13, dest->as_register()); break;
      case lir_logic_or:    __ or3  (left->as_register(), simm13, dest->as_register()); break;
      case lir_logic_orcc:  __ orcc (left->as_register(), simm13, G0); break;
      case lir_logic_xor:   __ xor3 (left->as_register(), simm13, dest->as_register()); break;
      default: ShouldNotReachHere();
    }
  } else {
    assert(right->is_register(), "right should be in register");

    if (dest->is_single_cpu()) {
      switch (code) {
        case lir_logic_and:   __ and3 (left->as_register(), right->as_register(), dest->as_register()); break;
        case lir_logic_or:    __ or3  (left->as_register(), right->as_register(), dest->as_register()); break;
        case lir_logic_orcc:  __ orcc (left->as_register(), right->as_register(), G0); break;
        case lir_logic_xor:   __ xor3 (left->as_register(), right->as_register(), dest->as_register()); break;
        default: ShouldNotReachHere();
      }
    } else {
      switch (code) {
        case lir_logic_and: 
          __ and3 (left->rinfo().as_register_hi(), right->rinfo().as_register_hi(), dest->rinfo().as_register_hi()); 
          __ and3 (left->rinfo().as_register_lo(), right->rinfo().as_register_lo(), dest->rinfo().as_register_lo()); 
        break;

        case lir_logic_or:
          __ or3 (left->rinfo().as_register_hi(), right->rinfo().as_register_hi(), dest->rinfo().as_register_hi());
          __ or3 (left->rinfo().as_register_lo(), right->rinfo().as_register_lo(), dest->rinfo().as_register_lo()); 
        break;

        case lir_logic_orcc:
          __ orcc (left->rinfo().as_register_hi(), right->rinfo().as_register_hi(), G0);
          __ orcc (left->rinfo().as_register_lo(), right->rinfo().as_register_lo(), dest->rinfo().as_register_lo()); 
        break;

        case lir_logic_xor:
          __ xor3 (left->rinfo().as_register_hi(), right->rinfo().as_register_hi(), dest->rinfo().as_register_hi());
          __ xor3 (left->rinfo().as_register_lo(), right->rinfo().as_register_lo(), dest->rinfo().as_register_lo());
        break;

        default: ShouldNotReachHere();
      }
    }
  }
}


int LIR_Assembler::shift_amount(BasicType t) {
  int elem_size = type2aelembytes[t];
  switch (elem_size) {
    case 1 : return 0;
    case 2 : return 1;
    case 4 : return 2; 
    case 8 : return 3; 
  }
  ShouldNotReachHere();
  return -1;
}


Address LIR_Assembler::heap_Address(BasicType type, Register r, int disp_bytes, bool for_hi_word) { 
  assert(r != FP || r != SP, "need heap address");
  switch (type) {
    case T_LONG  : disp_bytes += for_hi_word ? 0 : longSize/2; break; // BigEndian
    case T_DOUBLE: disp_bytes += for_hi_word ? 0 : longSize/2; break; // BigEndian
    default      : assert(!for_hi_word, "Does not have hi word");  
  }
  return Address(r, 0, disp_bytes);
}


Address LIR_Assembler::stack_Address(BasicType type, Register r, int disp_bytes, bool for_hi_word) { 
  assert(r == FP || r == SP, "need stack address");
  switch (type) {
    case T_LONG  : disp_bytes += for_hi_word ? 0        : longSize/2;  break; // BigEndian
    case T_DOUBLE: disp_bytes += for_hi_word ? 0        : longSize/2 ; break; // BigEndian
    default      : assert(!for_hi_word, "Does not have hi word");  
  }
  return Address(r, 0, disp_bytes);
}


void LIR_Assembler::array2reg(LIR_Address* addr, RInfo to_rinfo, BasicType type, CodeEmitInfo* info) {
  assert (addr->base()->is_register(), "Must be a register");

  int shift = shift_amount(type);
  Register rarr = addr->base()->as_register();

  if (addr->index()->is_constant()) {
    int index = addr->index()->as_constant_ptr()->as_jint();
    int elem_size = type2aelembytes[type];
    int array_offset = index * elem_size;
    if (Assembler::is_simm13(array_offset)) {
      __ add(rarr, array_offset, O7);
    } else {
      __ mov(index, O7);
      __ sll(O7, shift, O7);
      __ add(O7, rarr, O7);
    }
  } else {
    assert (addr->index()->is_register(), "Must be register");
    Register rind = addr->index()->as_register();
    if (shift > 0) {
      __ sll(rind, shift, O7);
      __ add(O7, rarr, O7);  
    } else {
      __ add(rind, rarr, O7);
    }  
  }

  int base_offset =  arrayOopDesc::base_offset_in_bytes(type);
  switch(type) {
    case T_BYTE: 
    case T_CHAR: 
    case T_SHORT: 
    case T_ADDRESS:
    case T_ARRAY:
    case T_OBJECT:
    case T_INT:
    {
      assert(to_rinfo.is_word(), "check");
      load(O7, base_offset, to_rinfo.as_register(), type, info);  
    }  
    break;

    case T_FLOAT:
    {
      assert(to_rinfo.is_float(), "check");
      load(O7, base_offset, to_rinfo.as_float_reg(), type, info);  
    }
    break;

    case T_DOUBLE:
    {
      assert(to_rinfo.is_double(), "check");
      load(O7, base_offset, to_rinfo.as_double_reg(), type, info);  
    }
    break;

    case T_LONG:
    { 
      assert(to_rinfo.is_long(), "check");
      Register rlo = to_rinfo.as_register_lo();
      Register rhi = to_rinfo.as_register_hi();
      assert(rlo != rhi, "error in regalloc");
      assert(O7 != rlo && O7 != rhi, "O7 is for temps");
      load(heap_AddressLO(type, O7, base_offset), rlo, T_INT, info);
      load(heap_AddressHI(type, O7, base_offset), rhi, T_INT);
    }
    break;
  
    default: ShouldNotReachHere();
  }
}


void LIR_Assembler::throw_op(RInfo exceptionPC, RInfo exceptionOop, CodeEmitInfo* info, bool unwind) {
  assert(exceptionOop.is_same(FrameMap::_Oexception_RInfo), "should match");

  if (SafepointPolling) {
    info->add_register_oop(exceptionOop);
    safepoint_poll(exceptionPC, info);
  }

    address pc_for_athrow  = __ pc();
  int pc_for_athrow_offset = __ offset();
  __ nop(); // pc_for_athrow can not point to itself (relocInfo restriction)

  RelocationHolder rspec = internal_word_Relocation::spec(pc_for_athrow);
  __ set((intptr_t)pc_for_athrow, Oissuing_pc, rspec);
  add_call_info(pc_for_athrow_offset, info); // for exception handler

  if (JvmtiExport::can_post_exceptions()) {
    // Tell the runtime that we have thrown an exception
    info->add_register_oop(exceptionOop);
    __ call(Runtime1::entry_for(Runtime1::jvmdi_exception_throw_id), relocInfo::runtime_call_type);
    __ delayed()->nop();
    add_call_info(__ offset(), info);
  }

  // if we jump, we would escape the method, use a compiler safepoint
  // and mark exception oop as object
  if (UseCompilerSafepoints && !SafepointPolling) {
    info->add_register_oop(exceptionOop);
    add_debug_info_for_branch(info);
  }

  if (unwind) {
    __ br(Assembler::always, false, Assembler::pt, _unwind_entry_label);
  } else {
    __ br(Assembler::always, false, Assembler::pt, _throw_entry_label);
  }
  __ delayed()->nop();
}


void LIR_Assembler::emit_arraycopy(LIR_OpArrayCopy* op) {
  Register src = op->src()->as_register();
  Register dst = op->dst()->as_register();
  Register src_pos = op->src_pos()->as_register();
  Register dst_pos = op->dst_pos()->as_register();
  Register length  = op->length()->as_register();
  Register tmp = op->tmp()->as_register();
  Register tmp2 = O7;

  int flags = op->flags();
  ciArrayKlass* default_type = op->expected_type();
  BasicType basic_type = default_type != NULL ? default_type->element_type()->basic_type() : T_ILLEGAL;
  if (basic_type == T_ARRAY) basic_type = T_OBJECT;

  // set up the arraycopy stub information
  StaticCallStub* call_stub = new StaticCallStub(); // pc will get filled in by ArrayCopyStub
  ArrayCopyStub* stub = new ArrayCopyStub(op->info(), call_stub);
  stub->set_tmp(as_RInfo(tmp));
  stub->set_src(as_RInfo(src));
  stub->set_src_pos(as_RInfo(src_pos));
  stub->set_dst(as_RInfo(dst));
  stub->set_dst_pos(as_RInfo(dst_pos));
  stub->set_length(as_RInfo(length));
  emit_code_stub(stub);
  emit_code_stub(call_stub);

  // always do stub if no type information is available.  it's ok if
  // the known type isn't loaded since the code sanity checks
  // in debug mode and the type isn't required when we know the exact type
  // also check that the type is an array type.
  if (op->expected_type() == NULL) {
    __ mov(src,     O0);
    __ mov(src_pos, O1);
    __ mov(dst,     O2);
    __ mov(dst_pos, O3);
    __ mov(length,  O4);
    __ call_VM_leaf(tmp, CAST_FROM_FN_PTR(address, Runtime1::arraycopy));

    __ br_zero(Assembler::less, false, Assembler::pn, O0, *stub->entry());
    __ delayed()->nop();
    __ bind(*stub->continuation());
    return;
  }

  assert(default_type != NULL && default_type->is_array_klass(), "must be true at this point");

  // make sure src and dst are non-null and load array length
  if (flags & LIR_OpArrayCopy::src_null_check) {
    __ tst(src);
    __ br(Assembler::equal, false, Assembler::pn, *stub->entry());
    __ delayed()->nop();
  }

  if (flags & LIR_OpArrayCopy::dst_null_check) {
    __ tst(dst);
    __ br(Assembler::equal, false, Assembler::pn, *stub->entry());
    __ delayed()->nop();
  }

  if (flags & LIR_OpArrayCopy::src_pos_positive_check) {
    // test src_pos register
    __ tst(src_pos);
    __ br(Assembler::less, false, Assembler::pn, *stub->entry());
    __ delayed()->nop();
  }

  if (flags & LIR_OpArrayCopy::dst_pos_positive_check) {
    // test dst_pos register
    __ tst(dst_pos);
    __ br(Assembler::less, false, Assembler::pn, *stub->entry());
    __ delayed()->nop();
  }

  if (flags & LIR_OpArrayCopy::length_positive_check) {
    // make sure length isn't negative
    __ tst(length);
    __ br(Assembler::less, false, Assembler::pn, *stub->entry());
    __ delayed()->nop();
  }

  if (flags & LIR_OpArrayCopy::src_range_check) {
    __ ld(src, arrayOopDesc::length_offset_in_bytes(), tmp2);
    __ add(length, src_pos, tmp);
    __ cmp(tmp2, tmp);
    __ br(Assembler::carrySet, false, Assembler::pn, *stub->entry());
    __ delayed()->nop();
  }
  
  if (flags & LIR_OpArrayCopy::dst_range_check) {
    __ ld(dst, arrayOopDesc::length_offset_in_bytes(), tmp2);
    __ add(length, dst_pos, tmp);
    __ cmp(tmp2, tmp);
    __ br(Assembler::carrySet, false, Assembler::pn, *stub->entry());
    __ delayed()->nop();
  }

  if (flags & LIR_OpArrayCopy::type_check) {
    __ ld_ptr(src, oopDesc::klass_offset_in_bytes(), tmp);
    __ ld_ptr(dst, oopDesc::klass_offset_in_bytes(), tmp2);
    __ cmp(tmp, tmp2);
    __ br(Assembler::notEqual, false, Assembler::pt, *stub->entry());
    __ delayed()->nop();
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
    jobject2reg(op->expected_type()->encoding(), tmp);
    __ ld_ptr(dst, oopDesc::klass_offset_in_bytes(), tmp2);
    __ cmp(tmp, tmp2);
    if (basic_type != T_OBJECT) {
      __ br(Assembler::notEqual, false, Assembler::pn, halt);
      __ delayed()->nop();
      __ ld_ptr(src, oopDesc::klass_offset_in_bytes(), tmp2);
      __ cmp(tmp, tmp2);
    }
    __ br(Assembler::equal, false, Assembler::pn, known_ok);
    __ delayed()->nop();
    __ bind(halt);
    __ stop("incorrect type information in arraycopy");
    __ bind(known_ok);
  }
#endif

  int shift = shift_amount(basic_type);

  Register src_ptr = O0;
  Register dst_ptr = O1;
  Register len     = O2;

  __ add(src, arrayOopDesc::base_offset_in_bytes(basic_type), src_ptr);
  if (shift == 0) {
    __ add(src_ptr, src_pos, src_ptr);
  } else {
    __ sll(src_pos, shift, tmp);
    __ add(src_ptr, tmp, src_ptr);
  }

  __ add(dst, arrayOopDesc::base_offset_in_bytes(basic_type), dst_ptr);
  if (shift == 0) {
    __ add(dst_ptr, dst_pos, dst_ptr);
  } else {
    __ sll(dst_pos, shift, tmp);
    __ add(dst_ptr, tmp, dst_ptr);
  }

  if (basic_type != T_OBJECT) {
    if (shift == 0) {
      __ mov(length, len);
    } else {
      __ sll(length, shift, len);
    }
    __ call_VM_leaf(tmp, CAST_FROM_FN_PTR(address, Runtime1::primitive_arraycopy));
  } else {
    // oop_arraycopy takes a length in number of elements, so don't scale it.
    __ mov(length, len);
    __ call_VM_leaf(tmp, CAST_FROM_FN_PTR(address, Runtime1::oop_arraycopy));
  }
  
  __ bind(*stub->continuation());
}


void LIR_Assembler::shift_op(LIR_Code code, RInfo leftR, RInfo countR, RInfo destR, RInfo tmpR) {

  if (destR.is_word()) {
    switch (code) {
      case lir_shl:  __ sll   (leftR.as_register(), countR.as_register(), destR.as_register()); break;
      case lir_shr:  __ sra   (leftR.as_register(), countR.as_register(), destR.as_register()); break;
      case lir_ushr: __ srl   (leftR.as_register(), countR.as_register(), destR.as_register()); break;
      default: ShouldNotReachHere();
    }
  } else {
    switch (code) {
      case lir_shl:  __ lshl  (leftR.as_register_hi(), leftR.as_register_lo(), countR.as_register(), destR.as_register_hi(), destR.as_register_lo(), G3_scratch); break;
      case lir_shr:  __ lshr  (leftR.as_register_hi(), leftR.as_register_lo(), countR.as_register(), destR.as_register_hi(), destR.as_register_lo(), G3_scratch); break;
      case lir_ushr: __ lushr (leftR.as_register_hi(), leftR.as_register_lo(), countR.as_register(), destR.as_register_hi(), destR.as_register_lo(), G3_scratch); break;
      default: ShouldNotReachHere();
    }
  }
}


void LIR_Assembler::shift_op(LIR_Code code, RInfo leftR, jint  count, RInfo destR) {
  Register value = leftR.as_register();
  if ( code != lir_shlx )
    count = count & 0x1F; // Java spec

  if (destR.is_word()) {
    switch (code) {
      case lir_shl:  __ sll   (leftR.as_register(), count, destR.as_register()); break;
      case lir_shlx: __ sllx  (leftR.as_register(), count, destR.as_register()); break;
      case lir_shr:  __ sra   (leftR.as_register(), count, destR.as_register()); break;
      case lir_ushr: __ srl   (leftR.as_register(), count, destR.as_register()); break;
      default: ShouldNotReachHere();
    }
  } else if (destR.is_long()) {
    Unimplemented();
  } else {
    ShouldNotReachHere();
  }
}


void LIR_Assembler::emit_alloc_obj(LIR_OpAllocObj* op) {
  assert(op->tmp1()->as_register()  == G1 &&
         op->tmp2()->as_register()  == G3 &&
         op->tmp3()->as_register()  == G4 &&
         op->obj()->as_register()   == O0 &&
         op->klass()->as_register() == G5, "must be");
  __ allocate_object(op->obj()->as_register(), 
                     op->tmp1()->as_register(),
                     op->tmp2()->as_register(),
                     op->tmp3()->as_register(),
                     op->header_size(),
                     op->object_size(),
                     op->klass()->as_register(),
                     *op->stub()->entry());
  __ bind(*op->stub()->continuation());
  __ verify_oop(op->obj()->as_register());
}


void LIR_Assembler::emit_alloc_array(LIR_OpAllocArray* op) {
  assert(op->tmp1()->as_register()  == G1 &&
         op->tmp2()->as_register()  == G3 &&
         op->tmp3()->as_register()  == G4 &&
         op->tmp4()->is_illegal()         &&
         op->klass()->as_register() == G5, "must be");
  __ allocate_array(op->obj()->as_register(),
                    op->len()->as_register(), 
                    op->tmp1()->as_register(), 
                    op->tmp2()->as_register(),
                    op->tmp3()->as_register(),
                    arrayOopDesc::header_size(op->type()),
                    type2aelembytes[op->type()],
                    op->klass()->as_register(),
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
    __ cmp(value, 0);
    __ br(Assembler::equal, false, Assembler::pn, done);
    __ delayed()->nop();
    load(array, oopDesc::klass_offset_in_bytes(), k_RInfo, T_OBJECT, op->info_for_exception());
    load(value, oopDesc::klass_offset_in_bytes(), klass_RInfo, T_OBJECT, NULL);
    
    // get instance klass
    load(k_RInfo, objArrayKlass::element_klass_offset_in_bytes() + sizeof(oopDesc), k_RInfo, T_OBJECT, NULL);
    // get super_check_offset
    load(k_RInfo, sizeof(oopDesc) + Klass::super_check_offset_offset_in_bytes(), Rtmp1, T_INT, NULL);
    // See if we get an immediate positive hit
    load(klass_RInfo, Rtmp1, as_RInfo(O7), T_OBJECT);
    __ cmp(k_RInfo, O7);
    __ br(Assembler::equal, false, Assembler::pn, done);
    __ delayed()->nop();
    // check for immediate negative hit
    __ cmp(Rtmp1, sizeof(oopDesc) + Klass::secondary_super_cache_offset_in_bytes());
    __ br(Assembler::notEqual, false, Assembler::pn, *stub->entry());
    __ delayed()->nop();
    // check for self
    __ cmp(klass_RInfo, k_RInfo);
    __ br(Assembler::equal, false, Assembler::pn, done);
    __ delayed()->nop();

    // assert(sub.is_same(FrameMap::_G3_RInfo) && super.is_same(FrameMap::_G1_RInfo), "incorrect call setup");
    __ call(Runtime1::entry_for(Runtime1::slow_subtype_check_id), relocInfo::runtime_call_type);
    __ delayed()->nop();
    __ cmp(G3, 0);
    __ br(Assembler::equal, false, Assembler::pn, *stub->entry());
    __ delayed()->nop();
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
    if (k->is_loaded()) {
      ciObject2reg(k, k_RInfo);
    } else {
      jobject2reg_with_patching(k_RInfo, op->info_for_patch());
    }
    assert(obj != k_RInfo, "must be different");
    __ cmp(obj, 0);
    __ br(Assembler::equal, false, Assembler::pt, done);
    __ delayed()->nop();

    // get object class
    // not a safepoint as obj null check happens earlier
    load(obj, oopDesc::klass_offset_in_bytes(), klass_RInfo, T_OBJECT, NULL);
    if (op->fast_check()) {
      __ cmp(k_RInfo, klass_RInfo);
      __ br(Assembler::notEqual, false, Assembler::pt, *stub->entry());
      __ delayed()->nop();
      __ bind(done);
    } else {
      if (k->is_loaded()) {
        assert(dst != obj, "need different registers so we have a temporary");
        load(klass_RInfo, k->super_check_offset(), dst, T_OBJECT, NULL);
        
        if (sizeof(oopDesc) + Klass::secondary_super_cache_offset_in_bytes() != k->super_check_offset()) {
          // See if we get an immediate positive hit
          __ cmp(dst, k_RInfo );
          __ br(Assembler::notEqual, false, Assembler::pn, *stub->entry());
          __ delayed()->nop();
        } else {
          // See if we get an immediate positive hit
          __ cmp(dst, k_RInfo );
          __ br(Assembler::equal, false, Assembler::pn, done);
          // check for self
          __ delayed()->cmp(klass_RInfo, k_RInfo);
          __ br(Assembler::equal, false, Assembler::pn, done);
          __ delayed()->nop();
          
          // assert(sub.is_same(FrameMap::_G3_RInfo) && super.is_same(FrameMap::_G1_RInfo), "incorrect call setup");
          __ call(Runtime1::entry_for(Runtime1::slow_subtype_check_id), relocInfo::runtime_call_type);
          __ delayed()->nop();
          __ cmp(G3, 0);
          __ br(Assembler::equal, false, Assembler::pn, *stub->entry());
          __ delayed()->nop();
        }
        __ bind(done);
      } else {
        assert(dst != obj, "need different registers so we have a temporary");
        assert(dst != klass_RInfo && dst != k_RInfo, "need 3 registers");
        
        load(k_RInfo, sizeof(oopDesc) + Klass::super_check_offset_offset_in_bytes(), dst, T_INT, NULL);
        // See if we get an immediate positive hit
        load(klass_RInfo, dst, as_RInfo(O7), T_OBJECT);
        __ cmp(k_RInfo, O7);
        __ br(Assembler::equal, false, Assembler::pn, done);
        __ delayed()->nop();
        // check for immediate negative hit
        __ cmp(dst, sizeof(oopDesc) + Klass::secondary_super_cache_offset_in_bytes());
        __ br(Assembler::notEqual, false, Assembler::pn, *stub->entry());
        // check for self
        __ delayed()->cmp(klass_RInfo, k_RInfo);
        __ br(Assembler::equal, false, Assembler::pn, done);
        __ delayed()->nop();
        
        // assert(sub.is_same(FrameMap::_G3_RInfo) && super.is_same(FrameMap::_G1_RInfo), "incorrect call setup");
        __ call(Runtime1::entry_for(Runtime1::slow_subtype_check_id), relocInfo::runtime_call_type);
        __ delayed()->nop();
        __ cmp(G3, 0);
        __ br(Assembler::equal, false, Assembler::pn, *stub->entry());
        __ delayed()->nop();
        __ bind(done);
      }
      
    }
    __ mov(obj, dst);
  } else if (code == lir_instanceof) {
    Register obj = op->object()->as_register();
    Register k_RInfo = op->tmp1()->as_register();
    Register klass_RInfo = op->tmp2()->as_register();
    Register dst = op->result_opr()->as_register();
    ciKlass* k = op->klass();
    
    Label done;
    // patching may screw with our temporaries on sparc,
    // so let's do it before loading the class
    if (k->is_loaded()) {
      ciObject2reg(k, k_RInfo);
    } else {
      jobject2reg_with_patching(k_RInfo, op->info_for_patch());
    }
    assert(obj != k_RInfo, "must be different");
    __ cmp(obj, 0);
    __ br(Assembler::equal, true, Assembler::pn, done);
    __ delayed()->set(0, dst);

    // get object class
    // not a safepoint as obj null check happens earlier
    load(obj, oopDesc::klass_offset_in_bytes(), klass_RInfo, T_OBJECT, NULL);
    if (op->fast_check()) {
      __ cmp(k_RInfo, klass_RInfo);
      __ br(Assembler::equal, true, Assembler::pt, done);
      __ delayed()->set(1, dst);
      __ set(0, dst);
      __ bind(done);
    } else {
      if (k->is_loaded()) {
        assert(dst != obj, "need different registers so we have a temporary");
        load(klass_RInfo, k->super_check_offset(), dst, T_OBJECT, NULL);
        
        if (sizeof(oopDesc) + Klass::secondary_super_cache_offset_in_bytes() != k->super_check_offset()) {
          // See if we get an immediate positive hit
          __ cmp(dst, k_RInfo );
          __ br(Assembler::equal, true, Assembler::pt, done);
          __ delayed()->set(1, dst);
          __ set(0, dst);
          __ bind(done);
        } else {
          // See if we get an immediate positive hit
          __ cmp(dst, k_RInfo );
          __ br(Assembler::equal, true, Assembler::pt, done);
          __ delayed()->set(1, dst);
          // check for self
          __ cmp(klass_RInfo, k_RInfo);
          __ br(Assembler::equal, true, Assembler::pt, done);
          __ delayed()->set(1, dst);
          
          // assert(sub.is_same(FrameMap::_G3_RInfo) && super.is_same(FrameMap::_G1_RInfo), "incorrect call setup");
          __ call(Runtime1::entry_for(Runtime1::slow_subtype_check_id), relocInfo::runtime_call_type);
          __ delayed()->nop();
          __ mov(G3, dst);
          __ bind(done);
        }
      } else {
        assert(dst != klass_RInfo && dst != k_RInfo, "need 3 registers");
        
        load(k_RInfo, sizeof(oopDesc) + Klass::super_check_offset_offset_in_bytes(), dst, T_INT, NULL);
        // See if we get an immediate positive hit
        load(klass_RInfo, dst, as_RInfo(O7), T_OBJECT);
        __ cmp(k_RInfo, O7);
        __ br(Assembler::equal, true, Assembler::pt, done);
        __ delayed()->set(1, dst);
        // check for immediate negative hit
        __ cmp(dst, sizeof(oopDesc) + Klass::secondary_super_cache_offset_in_bytes());
        __ br(Assembler::notEqual, true, Assembler::pt, done);
        __ delayed()->set(0, dst);
        // check for self
        __ cmp(klass_RInfo, k_RInfo);
        __ br(Assembler::equal, true, Assembler::pt, done);
        __ delayed()->set(1, dst);
        
        // assert(sub.is_same(FrameMap::_G3_RInfo) && super.is_same(FrameMap::_G1_RInfo), "incorrect call setup");
        __ call(Runtime1::entry_for(Runtime1::slow_subtype_check_id), relocInfo::runtime_call_type);
        __ delayed()->nop();
        __ mov(G3, dst);
        __ bind(done);
      }
    }
  } else {
    ShouldNotReachHere();
  }
  
}


void LIR_Assembler::emit_compare_and_swap(LIR_OpCompareAndSwap* op) {
  if (op->code() == lir_cas_long) {
    assert(VM_Version::supports_cx8(), "wrong machine");
    Register addr = op->addr()->as_register();
    Register cmp_value_lo = op->cmp_value()->as_register_lo();
    Register cmp_value_hi = op->cmp_value()->as_register_hi();
    Register new_value_lo = op->new_value()->as_register_lo();
    Register new_value_hi = op->new_value()->as_register_hi();
    Register t1 = op->tmp1()->as_register();
    Register t2 = op->tmp2()->as_register();
    // move high and low halves of long values into single registers
    __ sllx(cmp_value_hi, 32, t1);         // shift high half into temp reg
    __ srl(cmp_value_lo, 0, cmp_value_lo); // clear upper 32 bits of low half
    __ or3(t1, cmp_value_lo, t1);          // t1 holds 64-bit compare value
    __ sllx(new_value_hi, 32, t2);
    __ srl(new_value_lo, 0, new_value_lo);
    __ or3(t2, new_value_lo, t2);          // t2 holds 64-bit value to swap
    // perform the compare and swap operation
    __ casx(addr, t1, t2);
    // generate condition code - if the swap succeeded, t2 ("new value" reg) was
    // overwritten with the original value in "addr" and will be equal to t1.
    __ cmp(t1, t2);
    
  } else if (op->code() == lir_cas_int || op->code() == lir_cas_obj) {
    Register addr = op->addr()->as_register();
    Register cmp_value = op->cmp_value()->as_register();
    Register new_value = op->new_value()->as_register();
    Register t1 = op->tmp1()->as_register();
    Register t2 = op->tmp2()->as_register();
    __ mov(cmp_value, t1); 
    __ mov(new_value, t2);
    __ cas(addr, t1, t2);
    __ cmp(t1, t2);
  } else {
    Unimplemented();
  }
}


void LIR_Assembler::set_24bit_FPU() {
  Unimplemented();
}


void LIR_Assembler::reset_FPU() {
  Unimplemented();
}


void LIR_Assembler::remove_fpu_result(RInfo reg) {

}


void LIR_Assembler::breakpoint() {
  __ breakpoint_trap();
}


void LIR_Assembler::push(LIR_Opr opr) {
  Unimplemented();
}


void LIR_Assembler::pop(LIR_Opr opr) {
  Unimplemented();
}


void LIR_Assembler::monitor_address(int monitor_no, RInfo dst) {
  Address mon_addr = frame_map()->address_for_monitor_lock_index(monitor_no);
  Register reg = mon_addr.base();
  int offset = mon_addr.disp();
  // compute pointer to BasicLock
  if (mon_addr.is_simm13()) {
    __ add(reg, offset, dst.as_register());
  } else {
    __ set(offset, dst.as_register());
    __ add(reg, dst.as_register(), dst.as_register());
  }
}


void LIR_Assembler::emit_lock(LIR_OpLock* op) {
  Register obj = op->obj_opr()->as_register();
  Register hdr = op->hdr_opr()->as_register();
  Register lock = op->lock_opr()->as_register();

  // obj may not be an oop
  if (op->code() == lir_lock) {
    if (UseFastLocking) {
      assert(BasicLock::displaced_header_offset_in_bytes() == 0, "lock_reg must point to the displaced header");
      // add debug info for NullPointerException only if one is possible
      if (op->info() != NULL) {
        add_debug_info_for_null_check_here(op->info());
      }
      __ lock_object(hdr, obj, lock, op->scratch_opr()->as_register(), *op->stub()->entry());
    } else {
      // always do slow locking
      // note: the slow locking code could be inlined here, however if we use
      //       slow locking, speed doesn't matter anyway and this solution is
      //       simpler and requires less duplicated code - additionally, the
      //       slow locking code is the same in either case which simplifies
      //       debugging
      __ br(Assembler::always, false, Assembler::pt, *op->stub()->entry());
      __ delayed()->nop();
    }
  } else {
    assert (op->code() == lir_unlock, "Invalid code, expected lir_unlock");
    if (UseFastLocking) {
      assert(BasicLock::displaced_header_offset_in_bytes() == 0, "lock_reg must point to the displaced header");
      __ unlock_object(hdr, obj, lock, *op->stub()->entry());
    } else {
      // always do slow unlocking
      // note: the slow unlocking code could be inlined here, however if we use
      //       slow unlocking, speed doesn't matter anyway and this solution is
      //       simpler and requires less duplicated code - additionally, the
      //       slow unlocking code is the same in either case which simplifies
      //       debugging
      __ br(Assembler::always, false, Assembler::pt, *op->stub()->entry());
      __ delayed()->nop();
    }
  }
  __ bind(*op->stub()->continuation());
}

void LIR_Assembler::reg2array(RInfo from_rinfo, LIR_Address* addr, BasicType type, CodeEmitInfo* info) {
  assert(addr->base()->is_register(), "not in a register");  
 
  Register rarr = addr->base()->as_register();
  __ verify_oop(rarr);

  int shift = shift_amount(type);
  if (addr->index()->is_constant()) {
    int index = addr->index()->as_constant_ptr()->as_jint();
    int elem_size = type2aelembytes[type];
    int array_offset = index * elem_size;
    if (Assembler::is_simm13(array_offset)) {
      __ add(rarr, array_offset, O7);
    } else {
      __ mov(index, O7);
      __ sll(O7, shift, O7);
      __ add(O7, rarr, O7);
    }
  } else {
    assert (addr->index()->is_register(), "Must be register");
    Register rind = addr->index()->as_register();
    if (shift > 0) {
      __ sll(rind, shift, O7);
      __ add(O7, rarr, O7);  
    } else {
      __ add(rind, rarr, O7);
    }  
  }

  int base_offset =  arrayOopDesc::base_offset_in_bytes(type);
  switch(type) {
    case T_ARRAY:
    case T_OBJECT:
      __ verify_oop(from_rinfo.as_register());
      // fall through
    case T_ADDRESS:
    case T_BYTE: 
    case T_CHAR: 
    case T_SHORT: 
    case T_INT: {
      assert(from_rinfo.is_word(), "check");
      store(from_rinfo.as_register(), O7, base_offset, type, info);  
      break;
    }  
    case T_FLOAT: {
      assert(from_rinfo.is_float(), "check");
      store(from_rinfo.as_float_reg(), O7, base_offset, type, info);  
      break;
    }
    case T_DOUBLE: {
      assert(from_rinfo.is_double(), "check");
      store(from_rinfo.as_double_reg(), O7, base_offset, type, info);  
      break;
    }
    case T_LONG: { 
      assert(from_rinfo.is_long(), "check");
      Register rlo = from_rinfo.as_register_lo();
      Register rhi = from_rinfo.as_register_hi();
      assert(rlo != rhi, "error in regalloc");

      store(rlo, heap_AddressLO(type, O7, base_offset), T_INT, info);
      store(rhi, heap_AddressHI(type, O7, base_offset), T_INT);
      break;
    } 
    default: ShouldNotReachHere();
  }

  if (type == T_ARRAY || type == T_OBJECT) {
    __ add(O7, base_offset, O7);
    __ store_check(G1, O7);
  }
}


void LIR_Assembler::const2array(LIR_Const* c, LIR_Address* addr, BasicType type, CodeEmitInfo* info) {
  Unimplemented();
}


void LIR_Assembler::align_backward_branch_target() {
}


void LIR_Assembler::emit_delay(LIR_OpDelay* op) {
  // make sure we are expecting a delay
  // this has the side effect of clearing the delay state
  // so we can use _masm instead of _masm->delayed() to do the
  // code generation.
  __ delayed();

  // make sure we only emit one instruction
  int offset = code_offset();
  op->delay_op()->emit_code(this);
#ifdef ASSERT
  if (code_offset() - offset != NativeInstruction::nop_instruction_size) {
    op->delay_op()->print();
  }
  assert(code_offset() - offset == NativeInstruction::nop_instruction_size,
         "only one instruction can go in a delay slot");
#endif

  // we may also be emitting the call info for the instruction
  // which we are the delay slot of.
  CodeEmitInfo * call_info = op->call_info();
  if (call_info) {
    add_call_info(code_offset(), call_info);
  }
}


void LIR_Assembler::negate(LIR_Opr left, LIR_Opr dest) {
  assert(left->is_register(), "can only handle registers");

  if (left->is_single_cpu()) {
    __ neg(left->as_register(), dest->as_register());
  } else if (left->is_single_fpu()) {
    __ fneg(FloatRegisterImpl::S, left->rinfo().as_float_reg(), dest->rinfo().as_float_reg());
  } else if (left->is_double_fpu()) {
    __ fneg(FloatRegisterImpl::D, left->rinfo().as_double_reg(), dest->rinfo().as_double_reg());
  } else {
    assert (left->is_double_cpu(), "Must be a long");
    Register Rlow = left->rinfo().as_register_lo();
    Register Rhi = left->rinfo().as_register_hi();
    __ subcc(G0, Rlow, dest->rinfo().as_register_lo());
    __ subc (G0, Rhi,  dest->rinfo().as_register_hi());
  }
}


void LIR_Assembler::fpu_pop(RInfo reg) {
  // do nothing
}


void LIR_Assembler::dup_fpu(RInfo from_reg, RInfo to_reg) {
  Unimplemented();
}


void LIR_Assembler::fpu_push(RInfo reg) {
  // do nothing
}


void LIR_Assembler::set_fpu_stack_empty() {
  // do nothing
}


void LIR_Assembler::rt_call(address dest, RInfo tmp, int size_in_words, int args) {
#ifdef _LP64
  if ( args & LIR_Emitter::native_arg0_is_long )
    pack64( O0, O0 );
  if ( args & LIR_Emitter::native_arg1_is_long )
    pack64( O2, O1 );
#endif // _LP64

  // if tmp is invalid, then the function being called doesn't destroy the thread
  if (tmp.is_valid()) {
    __ call_VM_leaf(tmp.as_register(), dest);
  } else {
    __ call(dest, relocInfo::runtime_call_type);
    __ delayed()->nop();
#ifdef ASSERT    
    __ verify_thread();
#endif // ASSERT
  }

#ifdef _LP64
  if ( args & LIR_Emitter::native_return_is_long )
    unpack64 ( O0 );
#endif // _LP64
}


void LIR_Assembler::new_multi_array(int rank, RInfo dest, CodeEmitInfo* info) {
  const Register obj = O0;
  __ set(rank, FrameMap::_O1_RInfo.as_register());
  int offset_from_sp = (frame::memory_parameter_word_sp_offset * wordSize) + STACK_BIAS;
  // move Address dst_addr(SP, 0, offset_from_sp) --> O2
  Register dim_reg = O2;
  __ mov(SP, O2);
  __ add(O2, offset_from_sp, O2);
  __ call(Runtime1::entry_for(Runtime1::new_multi_array_id), relocInfo::runtime_call_type);
  __ delayed()->nop();
  add_call_info(code_offset(), info);
  __ mov(obj, dest.as_register());
  __ verify_oop(obj);
}


void LIR_Assembler::volatile_move_op(LIR_Opr src, LIR_Opr dest, BasicType type, LIR_Op1::LIR_PatchCode patch_code, CodeEmitInfo* info) {
  NEEDS_CLEANUP
  // The code for volatiles has not been optimized yet
  if (type == T_LONG) {
    PatchingStub* patch = NULL;
    if (patch_code != LIR_Op1::patch_none) {
      patch = new PatchingStub(_masm, PatchingStub::access_field_id);
    }

    LIR_Address* mem_addr = dest->is_address() ? dest->as_address_ptr() : src->as_address_ptr();

    // (extended to allow indexed as well as constant displaced for JSR-166)
    Register idx; // contains either constant offset or index 

    int disp = offset_of(mem_addr);
    if (mem_addr->index() == LIR_OprFact::illegalOpr) {
      idx = O7;     
      __ sethi(disp & ~0x3ff, idx, true);
      __ add(O7, disp & 0x3ff, idx);
    }
    else {
      assert(disp == 0, "not both indexed and disp");
      idx = mem_addr->index()->as_register();
    }

    int offset = code_offset();

    Register base = base_of(mem_addr).as_register();
    if (src->is_register() && dest->is_address()) {
      __ mov (src->rinfo().as_register_hi(), G4);
      __ mov (src->rinfo().as_register_lo(), G5);
      // G4 is high half, G5 is low half
      if (VM_Version::v9_instructions_work()) {
        // clear the top bits of G5, and scale up G4
        __ clruw(G5, G5);
        __ sllx(G4, 32, G4);
        // combine the two halves into the 64 bits of O0
        __ or3(G4, G5, G4);
        __ stx(G4, base, idx);
      } else {
        __ std(G4, base, idx);
      }
    } else if (src->is_address() && dest->is_register()) {
      if (VM_Version::v9_instructions_work()) {
        __ ldx(base, idx, G5);
        __ srax(G5, 32, G4); // fetch the high half into G4, low into G5
      } else {
        __ ldd(base, idx, G4);
      }
      // G4 is high half, G5 is low half
      __ mov (G4, dest->rinfo().as_register_hi());
      __ mov (G5, dest->rinfo().as_register_lo());
    } else {
      Unimplemented();
    }
    if (patch != NULL) {
      patching_epilog(patch, patch_code, base, info);
    }
    if (info != NULL) add_debug_info_for_null_check(offset, info);

  } else {
    move_op(src, dest, type, patch_code, info); // volatiles handled correctly??
  }
}

void LIR_Assembler::membar() {
  // only StoreLoad membars are ever explicitly needed on sparcs in TSO mode
  __ membar( Assembler::Membar_mask_bits(Assembler::StoreLoad) );
}

void LIR_Assembler::membar_acquire() {
  // no-op on TSO
}

void LIR_Assembler::membar_release() {
  // no-op on TSO
}

// Macro to Pack two sequential registers containing 32 bit values 
// into a single 64 bit register.
// rs and rs->successor() are packed into rd
// rd and rs may be the same register.
// Note: rs and rs->successor() are destroyed.
void LIR_Assembler::pack64( Register rs, Register rd ) {
#ifdef _LP64
  __ sllx(rs, 32, rs);
  __ srl(rs->successor(), 0, rs->successor());
  __ or3(rs, rs->successor(), rd);
#endif
}

// Macro to unpack a 64 bit value in a register into 
// two sequential registers.
// rd is unpacked into rd and rd->successor()
void LIR_Assembler::unpack64( Register rd ) {
#ifdef _LP64
  __ mov(rd, rd->successor());
  __ srax(rd, 32, rd);
  __ sra(rd->successor(), 0, rd->successor());
#endif
}


void LIR_Assembler::leal(LIR_Opr addr, LIR_Opr dest) {
  ShouldNotReachHere();
}


void LIR_Assembler::get_thread(LIR_Opr result_reg) {
  assert(result_reg->is_register(), "check");
  __ mov(G2_thread, result_reg->as_register());
}


#undef __ 

