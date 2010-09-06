#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_LIRAssembler.cpp	1.108 04/04/20 15:56:14 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

# include "incls/_precompiled.incl"
# include "incls/_c1_LIRAssembler.cpp.incl"


//------------------ConstantTable----------------------------------


ConstantTable::ConstantTable() {
  _entries = new FPUConstEntryList();
}


void ConstantTable::append_float(jfloat f) {
  _entries->append(new FPUConstEntry(f));
}


void ConstantTable::append_double(jdouble d) {
  _entries->append(new FPUConstEntry(d));
}

// Note: use JCK test to verify if this can be simplified to use only test
//       of values produced by jint_cast and jlong_cast.

address ConstantTable::address_of_float_constant(jfloat f) {
  for (int i = 0; i < _entries->length(); i++) {
    FPUConstEntry* fp = _entries->at(i);;
    // searching for floats
    if (fp->is_float()) {
      if ( g_isnan(fp->float_value()) || g_isnan(f) ) {
        if ( g_isnan(fp->float_value()) && g_isnan(f) ) {
          assert(fp->pc() != NULL, "entry not emitted");
          return fp->pc();
        }
      } else {
        if (jint_cast(fp->float_value())==jint_cast(f))   { // bitwise check
          assert(fp->pc() != NULL, "entry not emitted");
          return fp->pc(); 
        }
      }
    }
  }
  return NULL;
}


address ConstantTable::address_of_double_constant(jdouble d) {
  for (int i = 0; i < _entries->length(); i++) {
    FPUConstEntry* fp = _entries->at(i);;
    // searching for doubles
    if (fp->is_double()) {
      if ( g_isnan(fp->double_value()) || g_isnan(d) ) {
        if ( g_isnan(fp->double_value()) && g_isnan(d) ) {
          assert(fp->pc() != NULL, "entry not emitted");
          return fp->pc();
        }
      } else {
        if (jlong_cast(fp->double_value())==jlong_cast(d))  {  // bitwise check
          assert(fp->pc() != NULL, "entry not emitted");
          return fp->pc(); 
        }
      }
    }
  }
  return NULL;
}


void ConstantTable::emit_entries(MacroAssembler* masm, bool littleEndian) {
  for (int i = 0; i < _entries->length(); i++) {
    FPUConstEntry* fp = _entries->at(i);;
    if (fp->is_double()) {
      masm->align(wordSize*2); // double word alignment
      jlong temp = jlong_cast(fp->double_value());
      fp->set_pc((address)masm->pc());
      if (littleEndian) {
        masm->a_long(low(temp));
        masm->a_long(high(temp));
      } else {
        masm->a_long(high(temp));
        masm->a_long(low(temp));
      }
    } else {
      masm->align(wordSize); // probably always is
      assert(fp->is_float(), "invalid entry");
      jint temp = jint_cast(fp->float_value());
      fp->set_pc((address)masm->pc());
      masm->a_long(temp);
    }
  }
}


#ifndef PRODUCT

void ConstantTable::print() {
  tty->print_cr("Constant table (%d entries)", _entries->length());
  for (int i = 0; i < _entries->length(); i++) {
    FPUConstEntry* fp = _entries->at(i);;
    if (fp->is_double()) {
      tty->print_cr("  double: %f ", *((jdouble*)fp->pc()));
    } else {
      tty->print_cr("  float : %f ", *((jfloat*)fp->pc()));
    }
  }
}

#endif





void LIR_Assembler::patching_epilog(PatchingStub* patch, LIR_Op1::LIR_PatchCode patch_code, Register obj, CodeEmitInfo* info) {
  // we must have enough patching space so that call can be inserted
  while ((intx) _masm->pc() - (intx) patch->pc_start() < NativeCall::instruction_size) {
    _masm->nop(); 
  }
  patch->install(_masm, patch_code, obj, info);
  append_patching_stub(patch);

#ifdef ASSERT
  Bytecodes::Code code = info->scope()->method()->java_code_at_bci(info->bci());
  if (patch->id() == PatchingStub::access_field_id) {
    switch (code) {
      case Bytecodes::_putstatic:
      case Bytecodes::_getstatic:
      case Bytecodes::_putfield:
      case Bytecodes::_getfield:
        break;
      default:
        ShouldNotReachHere();
    }
  } else if (patch->id() == PatchingStub::load_klass_id) {
    switch (code) {
      case Bytecodes::_putstatic:
      case Bytecodes::_getstatic:
      case Bytecodes::_new:
      case Bytecodes::_anewarray:
      case Bytecodes::_multianewarray:
      case Bytecodes::_instanceof:
      case Bytecodes::_checkcast:
      case Bytecodes::_ldc:
      case Bytecodes::_ldc_w:
        break;
      default:
        ShouldNotReachHere();
    }
  } else {
    ShouldNotReachHere();
  }
#endif
}


//---------------------------------------------------------------


LIR_Assembler::LIR_Assembler(Compilation* c, CodeOffsets* offsets): 
   _compilation(c)
 , _masm(c->masm())
 , _frame_map(c->frame_map())
 , _offsets(offsets)
 , _bailout(false)
 , _current_block(NULL)
 , _last_debug_info_pc_offset(-1)
{
  _call_stubs = new CodeStubList();
  _slow_case_stubs = new CodeStubList();
}


LIR_Assembler::~LIR_Assembler() {
#ifdef ASSERT
  if (compilation()->bailed_out()) {
    // it's possible that these labels have been referenced but not bound because we are
    // bailing out.  Reset them to the uninialized state so we don't get assertion failures.
    Label l;
    _throw_entry_label = l;
    _unwind_entry_label = l;
    _vep_label = l;
  }
#endif
}


void LIR_Assembler::append_patching_stub(PatchingStub* stub) {
  _slow_case_stubs->append(stub);
}


void LIR_Assembler::check_codespace() {
  if (_masm->code()->code_end() + 1*K >= _masm->code()->code_limit()) {
    set_bailout("CodeBuffer overflow");
  }
  if (_masm->code()->locs_end() + 10 >= _masm->code()->locs_limit()) {
    set_bailout("CodeBuffer relocations overflow");
  }
}


// void LIR_Assembler::emit_code_stub(CodeStub* stub, CodeEmitInfo* info) {
void LIR_Assembler::emit_code_stub(CodeStub* stub) {
  CodeEmitInfo* info = stub->info();

  stub->set_code_pc(pc());
  if (stub->is_call_stub()) {
    _call_stubs->append(stub);
  } else {
    _slow_case_stubs->append(stub);
  }
}

void LIR_Assembler::emit_stubs(CodeStubList* stub_list) {
  if (must_bailout()) return;

  for (int m = 0; m < stub_list->length(); m++) {
    CodeStub* s = (*stub_list)[m];
    check_codespace();
    if (must_bailout()) break;
    s->emit_code(this);
#ifdef ASSERT
    s->assert_no_unbound_labels();
#endif
  }
}


void LIR_Assembler::emit_slow_case_stubs() { 
  emit_stubs(_slow_case_stubs);
}


bool LIR_Assembler::needs_icache(ciMethod* method) const {
  return !method->is_static();
}


int LIR_Assembler::code_offset() const {
  return _masm->offset();
}


address LIR_Assembler::pc() const {
  return _masm->pc();
}


//--------------------------iterators----------------------------------
// LIR_CodeGen is used to iterate over all basic blocks
// and emit code for each of the LIR instructions

class CollectConstants: public BlockClosure {
 private:
  ConstantTable* _table;

 public:
  CollectConstants(ConstantTable* table): _table(table) {}

  void block_do(BlockBegin* b) {
    for (Instruction* n = b; n != NULL; n = n->next()) {
      Constant* c = n->as_Constant();
      if (c != NULL) {
        if (c->type()->is_double()) {
          _table->append_double(c->type()->as_DoubleConstant()->value());
        } else if (c->type()->is_float() ) {
          _table->append_float(c->type()->as_FloatConstant()->value());
        }
      }
    }
  }
};


class LIR_CodeGen: public BlockClosure {
 private:
  LIR_Assembler* _assm;
 public:
  LIR_CodeGen(LIR_Assembler* assm): _assm(assm)  {}
  virtual void block_do(BlockBegin* block);
};


//---------------------------------------------------
void LIR_CodeGen::block_do(BlockBegin* block) {
  // the backward_branch_target_flag is set during the first ValueGen pass
  if (block->is_set(BlockBegin::backward_branch_target_flag)) {
    _assm->align_backward_branch_target(); 
  }

  if (block->lir() != NULL) {
    // if this block is the start of an exception handler, record the
    // PC offset of the first instruction for later construction of
    // the ExceptionRangeTable
    if (block->is_set(BlockBegin::exception_entry_flag)) {
      block->set_exception_handler_pco(_assm->code_offset());
    }

#ifndef SPARC
  assert(_assm->masm()->esp_offset() == 0, "frame size should be fixed");
#endif // SPARC

#ifndef PRODUCT
    if (PrintLIRWithAssembly) {
      block->print();
    }
#endif /* PRODUCT */
    _assm->set_current_block(block);
#ifdef ASSERT
    _assm->compilation()->set_cur_assembled_block(block);
#endif
    LIR_OpList* list = block->lir()->instructions_list();
    assert(_assm->is_empty_fpu_stack(), "fpu stack must be empty across blocks");
    int lng = list->length();
    for (int i = 0; i < lng; i++) {
      _assm->check_codespace();
      if (_assm->must_bailout()) break;
      LIR_Op* op = list->at(i);
      op->emit_code(_assm);
#ifndef PRODUCT
      if (PrintLIRWithAssembly) {
        // print out the LIR operation followed by they resulting assembly
        list->at(i)->print();
        _assm->masm()->code()->decode();
      }
#endif /* PRODUCT */
    }
    assert(_assm->is_empty_fpu_stack(), "fpu stack must be empty across blocks");
    _assm->set_current_block(NULL);
#ifdef ASSERT
    _assm->compilation()->set_cur_assembled_block(NULL);
#endif
#ifndef SPARC
    assert(_assm->masm()->esp_offset() == 0, "frame size should be fixed");
#endif // SPARC
  }
}


void LIR_Assembler::emit_code(BlockList* hir) {
  // collect float and double constants
  CollectConstants cc(&_const_table);

  if (UseFPConstTables) {
    hir->iterate_forward(&cc);
  }

  emit_constants();

  if (PrintLIR) {
    print_LIR(hir);
  }

  LIR_CodeGen gen(this);
  hir->iterate_forward(&gen);
}


//----------------------------------debug info--------------------------------


void LIR_Assembler::add_debug_info_for_branch(CodeEmitInfo* info) {
  if (UseCompilerSafepoints)  {
    _masm->code()->relocate(pc(), SafepointPolling ? relocInfo::poll_type : relocInfo::safepoint_type);
    int pc_offset = code_offset();
    _last_debug_info_pc_offset = pc_offset;
    info->record_debug_info(compilation()->debug_info_recorder(), pc_offset, false);
    if (info->exception_scope() != NULL) {
      compilation()->add_exception_handlers_for_pco(pc_offset, false, info->exception_scope());
    }
  }
}


void LIR_Assembler::add_call_info(int pc_offset, CodeEmitInfo* cinfo) {
  _last_debug_info_pc_offset = pc_offset;
  cinfo->record_debug_info(compilation()->debug_info_recorder(), pc_offset, true);
  if (cinfo->exception_scope() != NULL) {
    compilation()->add_exception_handlers_for_pco(pc_offset, true, cinfo->exception_scope());
  }
}

void LIR_Assembler::add_debug_info_for_null_check_here(CodeEmitInfo* cinfo) {
  add_debug_info_for_null_check(code_offset(), cinfo);
}

void LIR_Assembler::add_debug_info_for_null_check(int pc_offset, CodeEmitInfo* cinfo) {
  ImplicitNullCheckStub* stub = new ImplicitNullCheckStub(pc_offset, new CodeEmitInfo(cinfo, true));
  emit_code_stub(stub);
}

void LIR_Assembler::add_debug_info_for_div0_here(CodeEmitInfo* info) {
  add_debug_info_for_div0(code_offset(), info);
}

void LIR_Assembler::add_debug_info_for_div0(int pc_offset, CodeEmitInfo* cinfo) {
  DivByZeroStub* stub = new DivByZeroStub(pc_offset, cinfo);
  emit_code_stub(stub);
}

void LIR_Assembler::emit_rtcall(LIR_OpRTCall* op) {
  rt_call(op->addr(), op->tmp()->rinfo(), op->size(), op->args());
}


void LIR_Assembler::emit_call(LIR_OpJavaCall* op) {
  if (os::is_MP()) {
    // must align calls sites, otherwise they can't be updated atomically on MP hardware
    align_call(op->code());
  }

  StaticCallStub* stub = op->stub();
  if (stub) {
    assert(stub->info() == NULL, "why not?");
    emit_code_stub(stub);
  }

  switch (op->code()) {
  case lir_static_call:  
    call(op->addr(), relocInfo::static_call_type, op->info());
    break;
  case lir_optvirtual_call: 
    call(op->addr(), relocInfo::opt_virtual_call_type, op->info());
    break;
  case lir_icvirtual_call:
    ic_call(op->addr(), op->info());
    break;
  case lir_virtual_call:
    vtable_call(op->vtable_offset(), op->info());
    break;
  default: ShouldNotReachHere();
  }
}


void LIR_Assembler::emit_opLabel(LIR_OpLabel* op) {
  _masm->bind (*(op->label()));
}


void LIR_Assembler::emit_op1(LIR_Op1* op) {
  switch (op->code()) {
    case lir_volatile_move:
      volatile_move_op(op->in_opr(), op->result_opr(), op->type(), op->patch_code(), op->info());
      break;

    case lir_move:   
      move_op(op->in_opr(), op->result_opr(), op->type(), op->patch_code(), op->info()); 
      break;

    case lir_round32:
      // float type only
      round32_op(op->in_opr(), op->result_opr());
      break;

    case lir_array_move:
      array_move_op(op->in_opr(), op->result_opr(), op->type(), op->info());
      break;

    case lir_return:
      return_op(op->in_opr()->rinfo(), op->in_opr()->rinfo().is_valid() ? op->in_opr()->is_oop() : false); 
      break;
    
    case lir_safepoint:
      if (_last_debug_info_pc_offset == code_offset()) {
        _masm->nop();
      }
      safepoint_poll(op->in_opr()->rinfo(), op->info());
      break;

    case lir_fpu_push:
      fpu_push(op->in_opr()->rinfo());
      break;

    case lir_fpu_dup:
      dup_fpu(op->in_opr()->rinfo(), op->result_opr()->rinfo());
      break;

    case lir_branch:
      break;

    case lir_fpu_pop:
      remove_fpu_result(op->in_opr()->rinfo());
      break;

    case lir_push:
      push(op->in_opr());
      break;

    case lir_pop:
      pop(op->in_opr());
      break;

    case lir_neg:
      negate(op->in_opr(), op->result_opr());
      break;
    
    case lir_leal:
      leal(op->in_opr(), op->result_opr());
      break;
    
    case lir_null_check:
      if (GenerateCompilerNullChecks) {
        add_debug_info_for_null_check_here(op->info());

        if (op->in_opr()->is_single_cpu()) {
          _masm->null_check(op->in_opr()->rinfo().as_register());
        } else {
          Unimplemented();
        }
      }
      break;

    case lir_monaddr:
      monitor_address(op->in_opr()->as_constant_ptr()->as_jint(), op->result_opr()->rinfo());
      break;

    case lir_new_multi:
      new_multi_array(op->in_opr()->as_constant_ptr()->as_jint(), op->result_opr()->rinfo(), op->info());
      break;
    
    default:
      Unimplemented();
      break;
  }
}


void LIR_Assembler::emit_op0(LIR_Op0* op) {
  switch (op->code()) {
    case lir_word_align: {
      while (code_offset() % BytesPerWord != 0) {
        _masm->nop();
      }
      break;
    }

    case lir_empty_fpu:
      set_fpu_stack_empty();
      break;

    case lir_nop:
      if (op->info() != NULL) {
        // this nop is safepoint
        add_debug_info_for_branch(op->info());
        safepoint_nop();
      } else {
        _masm->nop();
      }
      break;

    case lir_label:
      Unimplemented();
      break;

    case lir_align_entry:
      // init offsets
      offsets()->_ep_offset   = _masm->offset();
      offsets()->_vep_offset  = _masm->offset();
      offsets()->_iep_offset  = _masm->offset();
      offsets()->_code_offset = _masm->offset();
      offsets()->_osr_offset  = _masm->offset();
      _masm->align(CodeEntryAlignment);
      break;

    case lir_verified_entry:
      offsets()->_vep_offset  = _masm->offset();
      offsets()->_iep_offset  = _masm->offset(); // Note: on SPARC, this entry point will be redefined later
      offsets()->_code_offset = _masm->offset();
      _masm->bind(_vep_label);
      _masm->verified_entry();
      break;

    case lir_build_frame:
      build_frame();
      setup_locals_at_entry();
      break;

    case lir_24bit_FPU:
      set_24bit_FPU();
      break;

    case lir_reset_FPU:
      reset_FPU();
      break;

    case lir_breakpoint:
      breakpoint();
      break;

    case lir_fpop_raw:
      fpop();
      break;

    case lir_jvmpi_method_enter:
      jvmpi_method_enter(op->info());
      break;

    case lir_jvmpi_method_exit:
      ShouldNotReachHere();
      break;

    case lir_membar:
      membar();
      break;

    case lir_membar_acquire:
      membar_acquire();
      break;

    case lir_membar_release:
      membar_release();
      break;

    case lir_get_thread:
      get_thread(op->result_opr());
      break;

    default: 
      ShouldNotReachHere();
      break;
  }
}


void LIR_Assembler::emit_op2(LIR_Op2* op) {
  switch (op->code()) {
    case lir_cmp:
      if (op->info() != NULL) {
        if (op->info()->is_compiled_safepoint()) {
          assert(!op->in_opr1()->is_address() && !op->in_opr2()->is_address(),
                 "shouldn't be compiled safepoint");
          add_debug_info_for_branch(op->info());  // compiled safepoint
        } else {
          assert(op->in_opr1()->is_address() || op->in_opr2()->is_address(),
                 "shouldn't be codeemitinfo for non-address operands");
          add_debug_info_for_null_check_here(op->info()); // exception possible
        }
      }
      comp_op(op->condition(), op->in_opr1(), op->in_opr2(), T_ILLEGAL);
      break;
    
    case lir_cmp_l2i:
    case lir_cmp_fd2i:
    case lir_ucmp_fd2i:
      comp_fl2i(op->code(), op->in_opr1(), op->in_opr2(), op->result_opr());
      break;

    case lir_shl:
    case lir_shlx:
    case lir_shr:
    case lir_ushr:
      if (op->in_opr2()->is_constant()) {
        shift_op(op->code(), op->in_opr1()->rinfo(), op->in_opr2()->as_constant_ptr()->as_jint(), op->result_opr()->rinfo());
      } else {
        shift_op(op->code(), op->in_opr1()->rinfo(), op->in_opr2()->rinfo(), op->result_opr()->rinfo(), op->tmp_opr()->rinfo());
      }
      break;

    case lir_add:
    case lir_sub:
    case lir_mul:
    case lir_mul_strictfp:
    case lir_div:
    case lir_div_strictfp:
    case lir_rem:
      arith_op(
        op->code(),
        op->in_opr1(),
        op->in_opr2(),
        op->result_opr(),
        op->info());
      break;
    
    case lir_sqrt:
    case lir_sin:
    case lir_cos:
      intrinsic_op(op->code(), op->in_opr1(), op->in_opr2(), op->result_opr());
      break;

    case lir_unverified_entry:
      offsets()->_ep_offset = _masm->offset();
      if (needs_icache(compilation()->method())) {
        offsets()->_ep_offset = check_icache(op->in_opr1()->as_register(),
                                             op->in_opr2()->as_register());
      }
      break;
    
    case lir_logic_and:
    case lir_logic_or:
    case lir_logic_orcc:
    case lir_logic_xor:
      logic_op(
        op->code(),
        op->in_opr1(),
        op->in_opr2(),
        op->result_opr());
      break;

    case lir_throw:
    case lir_unwind:
      throw_op(op->in_opr1()->rinfo(), op->in_opr2()->rinfo(), op->info(), op->code() == lir_unwind);
      break;

    default:
      Unimplemented();
      break;
  }
}


void LIR_Assembler::build_frame() {
  _masm->build_frame(initial_frame_size_in_bytes());
}


void LIR_Assembler::array_move_op(LIR_Opr src, LIR_Opr dest, BasicType type, CodeEmitInfo* info) {
  if (src->is_pointer()) {
    if (src->as_address_ptr() != NULL) {
      // array load
      array2reg(src->as_address_ptr(), dest->rinfo(), type, info);
    } else if (src->as_constant_ptr() != NULL) {
      // array store
      const2array(src->as_constant_ptr(), dest->as_address_ptr(), type, info);
    } else {
      Unimplemented();
    }
  } else if (src->is_single_cpu() || src->is_double_cpu() 
          || src->is_single_fpu() || src->is_double_fpu()) {
    // array store
    assert(dest->is_address(), "destination must be heap");
    reg2array(src->rinfo(), dest->as_address_ptr(), type, info);
  } else {
    Unimplemented();
  }
}


void LIR_Assembler::round32_op(LIR_Opr src, LIR_Opr dest) {
  assert(src->is_single_fpu() && dest->is_single_stack(), "rounds register -> stack location");
  reg2stack (src->rinfo(), dest->single_stack_ix(), T_FLOAT);
}


void LIR_Assembler::move_op(LIR_Opr src, LIR_Opr dest, BasicType type, LIR_Op1::LIR_PatchCode patch_code, CodeEmitInfo* info) {
  if (src->is_register() && dest->is_register()) {
    assert(patch_code == LIR_Op1::patch_none, "no patching allowed here");
    reg2reg(src->rinfo(), dest->rinfo());
  } else if (src->is_pointer()) {
    if (src->as_constant_ptr() != NULL) {
      if (dest->is_stack()) {
        if (dest->is_double_word()) {
          const2stack(src->as_constant_ptr(), dest->double_stack_ix());
        } else {
          const2stack(src->as_constant_ptr(), dest->single_stack_ix());
        }
      } else if (dest->is_pointer()) {
        if (dest->is_address()) {
          const2mem(src->as_constant_ptr(), dest->as_address_ptr(), type, info);
        } else {
          Unimplemented();
        }
      } else {
        const2reg(src->as_constant_ptr(), dest->rinfo(), patch_code, info); // patching is possible
      } 
    } else if (src->as_address_ptr() != NULL) {
      mem2reg(src->as_address_ptr(), dest->rinfo(), type, patch_code, info); 
    } else {
      Unimplemented();
    }
  } else if (src->is_stack()) {
    stack2reg(src, dest, type);
  } else if (src->is_single_cpu()) {
    if (dest->is_stack()) {
      assert(patch_code == LIR_Op1::patch_none, "no patching allowed here");
      reg2stack (src->rinfo(), dest->single_stack_ix(), type);
    } else if (dest->as_address_ptr() != NULL) {
      reg2mem(src->rinfo(), dest->as_address_ptr(), type, patch_code, info);
    } else {
      Unimplemented();
    }
  } else if (src->is_single_fpu()) {
    if (dest->is_stack()) {
      assert(patch_code == LIR_Op1::patch_none, "no patching allowed here");
      reg2stack (src->rinfo(), dest->single_stack_ix(), type);
    } else if (dest->is_address()) {
      reg2mem(src->rinfo(), dest->as_address_ptr(), type, patch_code, info);
    } else {
      Unimplemented();
    }
  // Double or Long
  } else if (src->is_double_word() ) {
    if (dest->is_stack()) {
      assert(patch_code == LIR_Op1::patch_none, "no patching allowed here");
      reg2stack (src->rinfo(), dest->double_stack_ix(), type);
    } else if (dest->is_address()) {
      reg2mem(src->rinfo(), dest->as_address_ptr(), type, patch_code, info);
    } else {
      Unimplemented();
    }
  } else {
    Unimplemented();
  }
}


void LIR_Assembler::set_bailout(const char* msg) {
  if (PrintBailouts && !_bailout) { // print only first time
    tty->print_cr("LIR_Assembler bailout reason: %s", msg);
  }
  _bailout = true;
}
